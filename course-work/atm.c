// Когато искате да изтеглите пари от банкомат, се прави трансакция от този банкомат
// към вашата банкова сметка. Реализирайте две програми, като едната е самият
// банкомат (потребителски интерфейс) - изпълнява всички операции: избор на език,
// сума, дали да има касов бон и PIN-код (проверката за него става в банкомата). Скрито
// за потребителя остава вземането на номера на картата, но той все пак се взима
// предвид. След като операцията е разрешена, се изпраща заявка до банковата сметка
// (сума и номер на сметката), където се прави проверка. Ако проверката върне грешка,
// се извежда съобщение на екрана (с цел улеснение - на банката). Информацията за
// клиентите се пази във файл и се зарежда в контейнер при всяко стартиране на
// програмата.

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "transaction.h"


typedef enum Language {
    BG,
    EN,
    ERR
} Language;

typedef enum UIString {
    ADMIN_OR_CLIENT,
    AMOUNT,
    CARD_NUMBER,
    WITHDRAW,
    CASH_RECEIPT,
    PIN,
    CARD,
    EXIT
} UIString;


void ui();
void client_ui(Language);
void admin_ui(Language);
Language choose_language();
char* get_ui_string(Language, UIString);
bool choose_yes_no(Language, UIString);
char* read_card_number(Language);

int initializeTCPConnection(unsigned short);
void closeTCPConnection();
void sendClientTCPRequest(Transaction);
void sendAdminTCPRequest(Client);

int osocket_client;
int osocket_admin;


int main() {
    osocket_client = initializeTCPConnection(CLIENT_PORT);
    osocket_admin = initializeTCPConnection(ADMIN_PORT);

    ui();
    // TODO: add basic atm operations as show how money I have

    closeTCPConnection();

    return 0;
}


void ui() {
    Language lang;
    do {
        lang = choose_language();
    } while(lang == ERR);

    while (1) {
        if (choose_yes_no(lang, EXIT)) {
            break;
        }

        bool admin = choose_yes_no(lang, ADMIN_OR_CLIENT);
        if (admin) {
            admin_ui(lang);
        } else {
            client_ui(lang);
        }
    }
}

void client_ui(Language lang) {
    Transaction trans;

    strcpy(trans.card_number, read_card_number(lang));

    printf("%s: ", get_ui_string(lang, PIN));
    scanf("%s", trans.pin);

    printf("%s: ", get_ui_string(lang, WITHDRAW));
    scanf("%d", &trans.withdraw_amount);

    // TODO: create cash receipt
    bool cash_receipt = choose_yes_no(lang, CASH_RECEIPT);
    trans.cash_receipt = cash_receipt;

    // TODO: add multi client support
    sendClientTCPRequest(trans);
}

void admin_ui(Language lang) {
    Client client;

    printf("%s: ", get_ui_string(lang, CARD_NUMBER));
    scanf("%s", client.card_number);
    FILE *fp;
    fp = fopen("card", "w");
    fwrite(client.card_number, sizeof(client.card_number), 1, fp);
    fclose(fp);

    printf("%s: ", get_ui_string(lang, PIN));
    scanf("%s", client.pin);

    printf("%s: ", get_ui_string(lang, WITHDRAW));
    scanf("%d", &client.amount);

    client.lock = false;

    sendAdminTCPRequest(client);
}

Language choose_language() {
    printf("Choose Language: ");
    char lang[2];
    scanf("%s", lang);

    if (strcmp(lang, "BG") == 0) {
        return BG;
    } else if (strcmp(lang, "EN") == 0) {
        return EN;
    }
    return ERR;
}

bool choose_yes_no(Language lang, UIString key) {
    printf("%s /y, n/: ", get_ui_string(lang, key));
    char cr;
    scanf(" %c", &cr);

    if (cr == 'y') {
        return true;
    } 
    return false;
}

char* read_card_number(Language lang) {
    printf("%s: ", get_ui_string(lang, CARD));

    char card[100];
    char *card_number = malloc(17 * sizeof(char));

    FILE *fp;
    fp = fopen(card, "r");
    fscanf(fp, "%s", card_number);
    fclose(fp);

    return card_number;
}

char* get_ui_string(Language lang, UIString key) {
    if (lang == BG) {
        switch (key) {
            case ADMIN_OR_CLIENT:
                return "Администратор ли сте";
            case AMOUNT:
                return "Наличност по сметка";
            case CARD_NUMBER:
                return "Номер на картата";
            case WITHDRAW:
                return "Размер на теглене";
            case CASH_RECEIPT:
                return "Изкаш ли касов бон";
            case PIN:
                return "Пин";
            case CARD:
                return "Карта";
            case EXIT:
                return "Изход";
        }
    } else if (lang == EN) {
        switch (key) {
            case ADMIN_OR_CLIENT:
                return "Are you administator";
            case AMOUNT:
                return "Account amount";
            case CARD_NUMBER:
                return "Card number";
            case WITHDRAW:
                return "Withdraw Amount";
            case CASH_RECEIPT:
                return "Do you want cash receipt";
            case PIN:
                return "PIN";
            case CARD:
                return "Card";
            case EXIT:
                return "Exit";
        }
    }
    
    return "";
}

int initializeTCPConnection(unsigned short port) {
    struct sockaddr_in server;
    int osocket;

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    if ((osocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation");
        exit(2);
    }

    if (connect(osocket, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("Socket connection");
        exit(2);
    }

    return osocket;
}

void closeTCPConnection() {
    close(osocket_client);
    close(osocket_admin);
}

void sendClientTCPRequest(Transaction transaction) {
    if (send(osocket_client, &transaction, sizeof(Transaction), 0) < 0) {
        perror("Socket send");
        exit(4);
    }

    Response response;
    if (recv(osocket_client, &response, sizeof(Response), 0) < 0) {
        perror("Socket recieve");
        exit(4);
    }

    perror("end");
    fprintf(stdout, "Code - %u | Message: %s", response.code, response.message);
}

void sendAdminTCPRequest(Client client) {
    if (send(osocket_admin, &client, sizeof(Client), 0) < 0) {
        perror("Socket send");
        exit(4);
    }

    Response response;
    if (recv(osocket_admin, &response, sizeof(Response), 0) < 0) {
        perror("Socket recieve");
        exit(4);
    }

    perror("end");
    fprintf(stdout, "Code - %u | Message: %s", response.code, response.message);
}
