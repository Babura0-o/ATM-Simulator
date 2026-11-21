
#ifndef ATM_LOGIC_H
#define ATM_LOGIC_H

#include <stddef.h>

// ==========================================
// CONSTANTS
// ==========================================

#define NUMBER_OF_CUSTOMERS 3
#define NUMBER_OF_DENOMINATIONS 4

#define MAX_ATTEMPTS 3
#define MAX_WITHDRAWAL_PER_TRANSACTION 10000
#define DAILY_WITHDRAWAL_LIMIT 20000
#define ATM_CASH_CAPACITY 50000
#define MAX_HISTORY 10

#define ADMIN_ID 999999
#define ADMIN_PIN 9999

// ==========================================
// STATUS CODES
// ==========================================

#define ATM_SUCCESS 1

#define ATM_ERROR_INVALID -1
#define ATM_ERROR_MAINTENANCE -2
#define ATM_ERROR_HARDWARE -3
#define ATM_ERROR_LIMIT -4
#define ATM_ERROR_BALANCE -5
#define ATM_ERROR_CASH -6
#define ATM_ERROR_CAPACITY -7
#define ATM_ERROR_NOTES -8
#define ATM_ERROR_REVERSED -9
#define ATM_ERROR_TRANSACTION -10
#define ATM_ERROR_BLOCKED -11

// ==========================================
// CUSTOMER DATA
// ==========================================

extern int cardNumbers[NUMBER_OF_CUSTOMERS];
extern int customerPINs[NUMBER_OF_CUSTOMERS];
extern int accountBalances[NUMBER_OF_CUSTOMERS];
extern int dailyWithdrawals[NUMBER_OF_CUSTOMERS];
extern int cardBlocked[NUMBER_OF_CUSTOMERS];

// ==========================================
// ATM CASH DATA
// ==========================================

extern int denominations[NUMBER_OF_DENOMINATIONS];
extern int availableNotes[NUMBER_OF_DENOMINATIONS];

// ==========================================
// TRANSACTION HISTORY
// ==========================================

extern char transactionType[NUMBER_OF_CUSTOMERS][MAX_HISTORY];
extern int transactionAmount[NUMBER_OF_CUSTOMERS][MAX_HISTORY];
extern int transactionBalance[NUMBER_OF_CUSTOMERS][MAX_HISTORY];

extern int transactionNotes[NUMBER_OF_CUSTOMERS][MAX_HISTORY]
                           [NUMBER_OF_DENOMINATIONS];

extern int transactionReversed[NUMBER_OF_CUSTOMERS][MAX_HISTORY];
extern int transactionCount[NUMBER_OF_CUSTOMERS];

// ==========================================
// ATM STATUS
// ==========================================

extern int maintenanceMode;
extern int cashDispenserFailed;
extern int depositSlotFailed;

// ==========================================
// AUTHENTICATION
// ==========================================

int atm_find_customer(int cardNumber);

int atm_authenticate_customer(int cardNumber, int pin);
int atm_authenticate_admin(int id, int pin);

int atm_register_failed_login(int cardNumber);
void atm_reset_failed_login(int cardNumber);

// ==========================================
// INFORMATION FUNCTIONS
// ==========================================

int atm_calculate_total_cash(void);
int atm_get_remaining_capacity(void);
int atm_get_remaining_daily_limit(int customerIndex);
int atm_customer_is_blocked(int customerIndex);

// ==========================================
// CUSTOMER TRANSACTIONS
// ==========================================

int atm_withdraw(
    int customerIndex,
    int amount,
    int manualSelection,
    int selectedNotes[]
);

int atm_deposit(
    int customerIndex,
    int amount,
    int selectedNotes[]
);

// ==========================================
// CASH MANAGEMENT
// ==========================================

int atm_replenish_cash(int amount, int selectedNotes[]);
int atm_remove_cash(int amount, int selectedNotes[]);

// ==========================================
// TRANSACTION MANAGEMENT
// ==========================================

void atm_record_transaction(
    int customerIndex,
    char type,
    int amount,
    int selectedNotes[]
);

int atm_reverse_transaction(int customerIndex, int transactionNumber);

// ==========================================
// ADMIN FUNCTIONS
// ==========================================

void atm_reset_daily_limits(void);
void atm_toggle_maintenance(void);
void atm_toggle_cash_dispenser(void);
void atm_toggle_deposit_slot(void);

void atm_generate_report(
    int *activeTransactions,
    int *totalWithdrawals,
    int *totalDeposits
);

#endif
