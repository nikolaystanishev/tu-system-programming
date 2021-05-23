#ifndef TRANSACTION_H
#define TRANSACTION_H

typedef enum {false, true} bool;

typedef struct Transaction {
    int withdraw_amount;
    bool cash_receipt;
    char pin[5];
    char card_number[17];
} Transaction;

typedef struct Client {
    int amount;
    char pin[5];
    char card_number[17];
    bool lock;
} Client;

unsigned short CLIENT_PORT = 33333;
unsigned short ADMIN_PORT = 44444;

typedef enum ResponseCode {
    MISSING_USER,
    NOT_ENOUGH_MONEY,
    SUCCESSFULL_WITHDRAW,
    CLIENT_ALREADY_EXISTS,
    SUCCESSFULL_CLIENT_CREATION
} ResponseCode;

typedef struct Response {
    ResponseCode code;
    char message[1000];
} Response;

#endif
