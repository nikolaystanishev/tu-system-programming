#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "transaction.h"


void fillDB();
void loadDB();
void storeDB();

int initializeTCPConnection(unsigned short);
void closeTCPConnection();
void handleTCPConnection();
void *handleClientTCPConnection(void *);
void *handleAdminTCPConnection(void *);

void addClientToDB(Client);
int findClient(Transaction);
int findClientByCard(char*);
bool isClientNull(int);


Client *db;
int db_count = 0;

int osocket[2];
int nsocket_client;
int nsocket_admin;

pthread_mutex_t client_lock;


int main() {
    loadDB();

    nsocket_client = initializeTCPConnection(CLIENT_PORT);
    nsocket_admin = initializeTCPConnection(ADMIN_PORT);

    handleTCPConnection();

    closeTCPConnection();

    return 0;
}


void fillDB() {
    FILE *fp;

    Client db[] = {
        {100, "1234", "1111111111111111", false},
        {200, "1111", "2222222222222222", false},
        {300, "2222", "3333333333333333", false},
        {400, "3333", "4444444444444444", false},
        {500, "4444", "5555555555555555", false},
        {600, "5555", "6666666666666666", false},
        {700, "6666", "7777777777777777", false},
        {800, "7777", "8888888888888888", false},
        {900, "8888", "9999999999999999", false},
        {1000, "9999", "0000000000000000", false},
    };

    fp = fopen("db", "wb");
    fwrite(db, sizeof(db), 1, fp);
    fclose(fp);
}

void loadDB() {
    db_count = 0;
    db = (Client *) calloc(1, sizeof(Client));
    if (db == NULL) {
        perror("Cannot create database");
        closeTCPConnection();
        exit(1);
    }

    FILE *fp;
    fp = fopen("db", "rb");
    int indx = 0;

    while (1) {
        Client client;
        fread(&client, sizeof(Client), 1, fp);
        if(feof(fp)) {
            break;
        }

        addClientToDB(client);
    }

    fclose(fp);

    fprintf(stdout, "DB loaded\n");
}

void storeDB() {
    FILE *fp;
    fp = fopen("db", "wb");

    fwrite(db, sizeof(Client), db_count, fp);
    fclose(fp);

    fprintf(stdout, "DB store\n");
}

int initializeTCPConnection(unsigned short port) {
    struct sockaddr_in client;
    struct sockaddr_in server;
    unsigned int namelen;
    int nsocket;

    if ((osocket[port % 2] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation");
        closeTCPConnection();
        exit(2);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(osocket[port % 2], (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Socket binding");
        closeTCPConnection();
        exit(2);
    }

    if (listen(osocket[port % 2], 1) != 0) {
        perror("Socket listening");
        closeTCPConnection();
        exit(2);
    }

    namelen = sizeof(client);
    if ((nsocket = accept(osocket[port % 2], (struct sockaddr *)&client, &namelen)) == -1) {
        perror("Socket accept");
        closeTCPConnection();
        exit(2);
    }

    if (pthread_mutex_init(&client_lock, NULL) != 0) {
        perror("Mutex initialization");
        closeTCPConnection();
        exit(3);
    }

    return nsocket;
}

void closeTCPConnection() {
    close(nsocket_client);
    close(nsocket_admin);
    close(osocket[0]);
    close(osocket[1]);

    pthread_mutex_destroy(&client_lock);

    storeDB();
}

void handleTCPConnection() {
    fprintf(stdout, "Server ready\n");

    pthread_t threads[2];

    if (pthread_create(&threads[CLIENT_PORT % 2], NULL, handleClientTCPConnection, NULL)) {
        perror("Thread creation");
        closeTCPConnection();
        exit(3);
    }
    if (pthread_create(&threads[ADMIN_PORT % 2], NULL, handleAdminTCPConnection, NULL)) {
        perror("Thread creation");
        closeTCPConnection();
        exit(3);
    }

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
}

void *handleClientTCPConnection(void *thread_id) {
    while (1) {
        Transaction transaction;

        if (recv(nsocket_client, &transaction, sizeof(Transaction), 0) == -1) {
            perror("Socket recieve");
            closeTCPConnection();
            exit(4);
        }

        int client = findClient(transaction);
        if (isClientNull(client)) {
            Response response = {MISSING_USER, "Missing user"};
            if (send(nsocket_client, &response, sizeof(Response), 0) < 0) {
                perror("Socket send");
                closeTCPConnection();
                exit(4);
            }
            continue;
        }

        while (db[client].lock == true) { }
        pthread_mutex_lock(&client_lock);
        db[client].lock = true;
        pthread_mutex_unlock(&client_lock);

        if (db[client].amount < transaction.withdraw_amount) {
            Response response = {NOT_ENOUGH_MONEY, "Not enough money"};
            if (send(nsocket_client, &response, sizeof(Response), 0) < 0) {
                perror("Socket send");
                closeTCPConnection();
                exit(4);
            }
            continue;
        }

        db[client].amount -= transaction.withdraw_amount;

        db[client].lock = false;

        char cash_receipt[1000];
        snprintf(
            cash_receipt, 1000,
            "Money in account - %d | Withdrawen money - %d", db[client].amount, transaction.withdraw_amount
        );

        Response response = {SUCCESSFULL_WITHDRAW, ""};
        strcpy(response.message, cash_receipt);
        if (send(nsocket_client, &response, sizeof(Response), 0) < 0) {
            perror("Socket send");
            closeTCPConnection();
            exit(4);
        }
    }
}

void *handleAdminTCPConnection(void *thread_id) {
    while (1) {
        Client client;

        if (recv(nsocket_admin, &client, sizeof(Client), 0) == -1) {
            perror("Socket recieve");
            closeTCPConnection();
            exit(4);
        }

        int db_client = findClientByCard(client.card_number);
        if (!isClientNull(db_client)) {
            Response response = {CLIENT_ALREADY_EXISTS, "Client alredy exists"};
            if (send(nsocket_admin, &response, sizeof(Response), 0) < 0) {
                perror("Socket send");
                closeTCPConnection();
                exit(4);
            }
            continue;
        }

        pthread_mutex_lock(&client_lock);
        addClientToDB(client);
        pthread_mutex_unlock(&client_lock);

        Response response = {SUCCESSFULL_CLIENT_CREATION, "Successfull client creation"};
        if (send(nsocket_admin, &response, sizeof(Response), 0) < 0) {
            perror("Socket send");
            closeTCPConnection();
            exit(4);
        }
    }
}

void addClientToDB(Client client) {
    Client *temp = (Client*) realloc(db, (db_count++ * sizeof(Client)));

    if (temp == NULL) {
        perror("Cannot add client to database.");
        closeTCPConnection();
        exit(1);
    } else {
        db = temp;
    }

    db[db_count - 1] = client;
}

int findClient(Transaction transaction) {
    int client = findClientByCard(transaction.card_number);
    
    if (!strcmp(db[client].pin, transaction.pin)) {
        return client;
    }

    return -1;
}

int findClientByCard(char* card_number) {
    for (int i = 0; i < db_count; i++) {
        if (!strcmp(db[i].card_number, card_number)) {
            return i;
        }
    }

    return -1;
}

bool isClientNull(int client) {
    return client == -1 ? true : false;
}
