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

#endif
