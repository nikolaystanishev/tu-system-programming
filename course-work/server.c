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
void handleClientTCPConnection();
void handleAdminTCPConnection();

Client findClient(Transaction);
Client findClientByCard(char*);
bool isClientNull(Client);


Client db[10];

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

    storeDB();
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
    FILE *fp;
    fp = fopen("db", "rb");
    int indx = 0;

    while (1) {
        fread(&db[indx++], sizeof(Client), 1, fp);
        if(feof(fp)) {
            break;
        }
    }

    fclose(fp);
}

void storeDB() {
    FILE *fp;
    fp = fopen("db", "wb");

    fwrite(db, sizeof(db), 1, fp);
    fclose(fp);
}

int initializeTCPConnection(unsigned short port) {
    struct sockaddr_in client;
    struct sockaddr_in server;
    unsigned int namelen;
    int nsocket;

    if ((osocket[port % 2] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation");
        exit(2);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(osocket[port % 2], (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Socket binding");
        exit(3);
    }

    if (listen(osocket[port % 2], 1) != 0) {
        perror("Socket listening");
        exit(4);
    }

    namelen = sizeof(client);
    if ((nsocket = accept(osocket[port % 2], (struct sockaddr *)&client, &namelen)) == -1) {
        perror("Socket accept");
        exit(5);
    }

    if (pthread_mutex_init(&client_lock, NULL) != 0) {
        perror("Mutex initialization");
        exit(6);
    }

    return nsocket;
}

void closeTCPConnection() {
    close(nsocket_client);
    close(nsocket_admin);
    close(osocket[0]);
    close(osocket[1]);

    pthread_mutex_destroy(&client_lock);
}

void handleTCPConnection() {
    while (1) {
        handleClientTCPConnection();
        handleAdminTCPConnection();
    }
}

void handleClientTCPConnection() {
    Transaction transaction;

    while (1) {
        if (recv(nsocket_client, &transaction, sizeof(Transaction), 0) == -1) {
            perror("Socket recieve");
            exit(6);
        }

        Client client = findClient(transaction);
        if (isClientNull(client)) {
            if (send(nsocket_client, "Missing user", 13, 0) < 0) {
                perror("Socket send");
                exit(7);
            }
            continue;
        }

        while (client.lock == true) { }
        pthread_mutex_lock(&client_lock);
        client.lock = true;
        pthread_mutex_unlock(&client_lock);

        if (client.amount < transaction.withdraw_amount) {
            if (send(nsocket_client, "Not enought money", 18, 0) < 0) {
                perror("Socket send");
                exit(7);
            }
            continue;
        }

        client.amount -= transaction.withdraw_amount;

        client.lock = false;

        if (send(nsocket_client, "Succesful withdraw", 19, 0) < 0) {
            perror("Socket send");
            exit(7);
        }
    }
}

void handleAdminTCPConnection() {
    Client client;

    while (1) {
        if (recv(nsocket_admin, &client, sizeof(Client), 0) == -1) {
            perror("Socket recieve");
            exit(6);
        }

        Client client = findClientByCard(client.card_number);
        if (!isClientNull(client)) {
            if (send(nsocket_admin, "Client already exists", 22, 0) < 0) {
                perror("Socket send");
                exit(7);
            }
            continue;
        }

        pthread_mutex_lock(&client_lock);
        // TODO: Add client to DB
        pthread_mutex_unlock(&client_lock);

        if (send(nsocket_admin, "Succesful client creation", 26, 0) < 0) {
            perror("Socket send");
            exit(7);
        }
    }
}

Client findClient(Transaction transaction) {
    Client client = findClientByCard(transaction.card_number);
    
    if (!strcmp(client.pin, transaction.pin)) {
        return client;
    }

    return (Client) {0, "", "", false};
}

Client findClientByCard(char* card_number) {
    for (int i = 0; i < 10; i++) {
        if (!strcmp(db[i].card_number, card_number)) {
            return db[i];
        }
    }

    return (Client) {0, "", "", false};
}

bool isClientNull(Client client) {
    if (client.amount == 0 && !strcmp(client.card_number, "") && !strcmp(client.pin, "") && client.lock == false) {
        return true;
    }

    return false;
}
