#include <stddef.h>
#include "atm_logic.h"

// ==========================================
// GLOBAL DATA
// ==========================================

int cardNumbers[NUMBER_OF_CUSTOMERS] = {123456, 234567, 345678};
int customerPINs[NUMBER_OF_CUSTOMERS] = {1234, 2345, 3456};
int accountBalances[NUMBER_OF_CUSTOMERS] = {15000, 25000, 10000};

int dailyWithdrawals[NUMBER_OF_CUSTOMERS] = {0};
int cardBlocked[NUMBER_OF_CUSTOMERS] = {0};

int denominations[NUMBER_OF_DENOMINATIONS] = {500, 200, 100, 50};
int availableNotes[NUMBER_OF_DENOMINATIONS] = {20, 20, 20, 20};

char transactionType[NUMBER_OF_CUSTOMERS][MAX_HISTORY];
int transactionAmount[NUMBER_OF_CUSTOMERS][MAX_HISTORY];
int transactionBalance[NUMBER_OF_CUSTOMERS][MAX_HISTORY];

int transactionNotes[NUMBER_OF_CUSTOMERS][MAX_HISTORY]
                    [NUMBER_OF_DENOMINATIONS];

int transactionReversed[NUMBER_OF_CUSTOMERS][MAX_HISTORY] = {0};
int transactionCount[NUMBER_OF_CUSTOMERS] = {0};

int maintenanceMode = 0;
int cashDispenserFailed = 0;
int depositSlotFailed = 0;

// Failed login attempts for each customer.
static int failedLoginAttempts[NUMBER_OF_CUSTOMERS] = {0};


// ==========================================
// AUTHENTICATION
// ==========================================

int atm_find_customer(int cardNumber)
{
    int i;

    for (i = 0; i < NUMBER_OF_CUSTOMERS; i++)
    {
        if (cardNumbers[i] == cardNumber)
            return i;
    }

    return -1;
}

int atm_authenticate_customer(int cardNumber, int pin)
{
    int customerIndex = atm_find_customer(cardNumber);

    if (customerIndex == -1)
        return -1;

    if (cardBlocked[customerIndex])
        return ATM_ERROR_BLOCKED;

    if (customerPINs[customerIndex] == pin)
        return customerIndex;

    return -1;
}

int atm_authenticate_admin(int id, int pin)
{
    return id == ADMIN_ID && pin == ADMIN_PIN;
}

int atm_register_failed_login(int cardNumber)
{
    int customerIndex = atm_find_customer(cardNumber);

    if (customerIndex == -1)
        return -1;

    if (cardBlocked[customerIndex])
        return 0;

    failedLoginAttempts[customerIndex]++;

    if (failedLoginAttempts[customerIndex] >= MAX_ATTEMPTS)
    {
        cardBlocked[customerIndex] = 1;
        return 0;
    }

    return MAX_ATTEMPTS - failedLoginAttempts[customerIndex];
}

void atm_reset_failed_login(int cardNumber)
{
    int customerIndex = atm_find_customer(cardNumber);

    if (customerIndex != -1)
        failedLoginAttempts[customerIndex] = 0;
}

int atm_customer_is_blocked(int customerIndex)
{
    if (customerIndex < 0 || customerIndex >= NUMBER_OF_CUSTOMERS)
        return 0;

    return cardBlocked[customerIndex];
}


// ==========================================
// ATM INFORMATION
// ==========================================

int atm_calculate_total_cash(void)
{
    int i;
    int total = 0;

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
        total += denominations[i] * availableNotes[i];

    return total;
}

int atm_get_remaining_capacity(void)
{
    return ATM_CASH_CAPACITY - atm_calculate_total_cash();
}

int atm_get_remaining_daily_limit(int customerIndex)
{
    if (customerIndex < 0 || customerIndex >= NUMBER_OF_CUSTOMERS)
        return 0;

    return DAILY_WITHDRAWAL_LIMIT - dailyWithdrawals[customerIndex];
}


// ==========================================
// NOTE SELECTION
// ==========================================

static int atm_automatic_selection(int amount, int selectedNotes[])
{
    int a, b, c, d;
    int remaining;
    int maxA, maxB, maxC;
    int remA, remB;
    int i;

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
        selectedNotes[i] = 0;

    maxA = amount / denominations[0];

    if (maxA > availableNotes[0])
        maxA = availableNotes[0];

    for (a = maxA; a >= 0; a--)
    {
        remA = amount - a * denominations[0];

        maxB = remA / denominations[1];

        if (maxB > availableNotes[1])
            maxB = availableNotes[1];

        for (b = maxB; b >= 0; b--)
        {
            remB = remA - b * denominations[1];

            maxC = remB / denominations[2];

            if (maxC > availableNotes[2])
                maxC = availableNotes[2];

            for (c = maxC; c >= 0; c--)
            {
                remaining = remB - c * denominations[2];

                if (remaining < 0 || remaining % denominations[3] != 0)
                    continue;

                d = remaining / denominations[3];

                if (d <= availableNotes[3])
                {
                    selectedNotes[0] = a;
                    selectedNotes[1] = b;
                    selectedNotes[2] = c;
                    selectedNotes[3] = d;

                    return 1;
                }
            }
        }
    }

    return 0;
}

static int atm_validate_notes(int amount, int selectedNotes[],
                              int checkAvailable)
{
    int i;
    int total = 0;

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    {
        if (selectedNotes[i] < 0)
            return 0;

        if (checkAvailable && selectedNotes[i] > availableNotes[i])
            return 0;

        total += selectedNotes[i] * denominations[i];
    }

    return total == amount;
}


// ==========================================
// TRANSACTION HISTORY
// ==========================================

void atm_record_transaction(int customerIndex, char type,
                            int amount, int selectedNotes[])
{
    int i, j;
    int position;

    if (customerIndex < 0 || customerIndex >= NUMBER_OF_CUSTOMERS)
        return;

    if (transactionCount[customerIndex] == MAX_HISTORY)
    {
        for (i = 1; i < MAX_HISTORY; i++)
        {
            transactionType[customerIndex][i - 1] =
                transactionType[customerIndex][i];

            transactionAmount[customerIndex][i - 1] =
                transactionAmount[customerIndex][i];

            transactionBalance[customerIndex][i - 1] =
                transactionBalance[customerIndex][i];

            transactionReversed[customerIndex][i - 1] =
                transactionReversed[customerIndex][i];

            for (j = 0; j < NUMBER_OF_DENOMINATIONS; j++)
            {
                transactionNotes[customerIndex][i - 1][j] =
                    transactionNotes[customerIndex][i][j];
            }
        }

        position = MAX_HISTORY - 1;
    }
    else
    {
        position = transactionCount[customerIndex];
        transactionCount[customerIndex]++;
    }

    transactionType[customerIndex][position] = type;
    transactionAmount[customerIndex][position] = amount;
    transactionBalance[customerIndex][position] =
        accountBalances[customerIndex];

    transactionReversed[customerIndex][position] = 0;

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    {
        transactionNotes[customerIndex][position][i] =
            selectedNotes[i];
    }
}


// ==========================================
// WITHDRAWAL
// ==========================================

int atm_withdraw(int customerIndex, int amount,
                 int manualSelection, int selectedNotes[])
{
    int i;

    if (customerIndex < 0 || customerIndex >= NUMBER_OF_CUSTOMERS)
        return ATM_ERROR_INVALID;

    if (maintenanceMode)
        return ATM_ERROR_MAINTENANCE;

    if (cashDispenserFailed)
        return ATM_ERROR_HARDWARE;

    if (amount <= 0 || amount % 50 != 0)
        return ATM_ERROR_INVALID;

    if (amount > MAX_WITHDRAWAL_PER_TRANSACTION ||
        amount > atm_get_remaining_daily_limit(customerIndex))
        return ATM_ERROR_LIMIT;

    if (amount > accountBalances[customerIndex])
        return ATM_ERROR_BALANCE;

    if (amount > atm_calculate_total_cash())
        return ATM_ERROR_CASH;

    if (selectedNotes == NULL)
        return ATM_ERROR_INVALID;

    if (!manualSelection)
    {
        if (!atm_automatic_selection(amount, selectedNotes))
            return ATM_ERROR_NOTES;
    }
    else
    {
        if (!atm_validate_notes(amount, selectedNotes, 1))
            return ATM_ERROR_NOTES;
    }

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
        availableNotes[i] -= selectedNotes[i];

    accountBalances[customerIndex] -= amount;
    dailyWithdrawals[customerIndex] += amount;

    atm_record_transaction(customerIndex, 'W', amount, selectedNotes);

    return ATM_SUCCESS;
}


// ==========================================
// DEPOSIT
// ==========================================

int atm_deposit(int customerIndex, int amount, int selectedNotes[])
{
    int i;

    if (customerIndex < 0 || customerIndex >= NUMBER_OF_CUSTOMERS)
        return ATM_ERROR_INVALID;

    if (maintenanceMode)
        return ATM_ERROR_MAINTENANCE;

    if (depositSlotFailed)
        return ATM_ERROR_HARDWARE;

    if (amount <= 0 || amount % 50 != 0)
        return ATM_ERROR_INVALID;

    if (selectedNotes == NULL)
        return ATM_ERROR_INVALID;

    if (amount > atm_get_remaining_capacity())
        return ATM_ERROR_CAPACITY;

    if (!atm_validate_notes(amount, selectedNotes, 0))
        return ATM_ERROR_NOTES;

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
        availableNotes[i] += selectedNotes[i];

    accountBalances[customerIndex] += amount;

    atm_record_transaction(customerIndex, 'D', amount, selectedNotes);

    return ATM_SUCCESS;
}


// ==========================================
// ADMIN CASH MANAGEMENT
// ==========================================

int atm_replenish_cash(int amount, int selectedNotes[])
{
    int i;

    if (amount <= 0 || amount % 50 != 0)
        return ATM_ERROR_INVALID;

    if (selectedNotes == NULL)
        return ATM_ERROR_INVALID;

    if (amount > atm_get_remaining_capacity())
        return ATM_ERROR_CAPACITY;

    if (!atm_validate_notes(amount, selectedNotes, 0))
        return ATM_ERROR_NOTES;

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
        availableNotes[i] += selectedNotes[i];

    return ATM_SUCCESS;
}

int atm_remove_cash(int amount, int selectedNotes[])
{
    int i;

    if (amount <= 0 || amount % 50 != 0)
        return ATM_ERROR_INVALID;

    if (selectedNotes == NULL)
        return ATM_ERROR_INVALID;

    if (amount > atm_calculate_total_cash())
        return ATM_ERROR_CASH;

    if (!atm_validate_notes(amount, selectedNotes, 1))
        return ATM_ERROR_NOTES;

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
        availableNotes[i] -= selectedNotes[i];

    return ATM_SUCCESS;
}


// ==========================================
// TRANSACTION REVERSAL
// ==========================================

int atm_reverse_transaction(int customerIndex, int transactionNumber)
{
    int position;
    int amount;
    int i;
    int selectedNotes[NUMBER_OF_DENOMINATIONS];

    if (customerIndex < 0 || customerIndex >= NUMBER_OF_CUSTOMERS)
        return ATM_ERROR_INVALID;

    if (transactionNumber < 1 ||
        transactionNumber > transactionCount[customerIndex])
        return ATM_ERROR_TRANSACTION;

    position = transactionNumber - 1;

    if (transactionType[customerIndex][position] == 'R' ||
        transactionReversed[customerIndex][position])
        return ATM_ERROR_REVERSED;

    amount = transactionAmount[customerIndex][position];

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    {
        selectedNotes[i] =
            transactionNotes[customerIndex][position][i];
    }

    if (transactionType[customerIndex][position] == 'W')
    {
        if (amount > atm_get_remaining_capacity())
            return ATM_ERROR_CAPACITY;

        accountBalances[customerIndex] += amount;
        dailyWithdrawals[customerIndex] -= amount;

        if (dailyWithdrawals[customerIndex] < 0)
            dailyWithdrawals[customerIndex] = 0;

        for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
            availableNotes[i] += selectedNotes[i];
    }
    else if (transactionType[customerIndex][position] == 'D')
    {
        if (amount > accountBalances[customerIndex])
            return ATM_ERROR_BALANCE;

        if (!atm_validate_notes(amount, selectedNotes, 1))
            return ATM_ERROR_NOTES;

        accountBalances[customerIndex] -= amount;

        for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
            availableNotes[i] -= selectedNotes[i];
    }
    else
    {
        return ATM_ERROR_TRANSACTION;
    }

    transactionReversed[customerIndex][position] = 1;

    atm_record_transaction(customerIndex, 'R', amount, selectedNotes);

    return ATM_SUCCESS;
}


// ==========================================
// ADMIN CONTROLS
// ==========================================

void atm_reset_daily_limits(void)
{
    int i;

    for (i = 0; i < NUMBER_OF_CUSTOMERS; i++)
    {
        dailyWithdrawals[i] = 0;
        cardBlocked[i] = 0;
        failedLoginAttempts[i] = 0;
    }
}

void atm_toggle_maintenance(void)
{
    maintenanceMode = !maintenanceMode;
}

void atm_toggle_cash_dispenser(void)
{
    cashDispenserFailed = !cashDispenserFailed;
}

void atm_toggle_deposit_slot(void)
{
    depositSlotFailed = !depositSlotFailed;
}


// ==========================================
// ADMIN REPORTS
// ==========================================

void atm_generate_report(int *activeTransactions,
                         int *totalWithdrawals,
                         int *totalDeposits)
{
    int i, j;

    if (activeTransactions == NULL ||
        totalWithdrawals == NULL ||
        totalDeposits == NULL)
        return;

    *activeTransactions = 0;
    *totalWithdrawals = 0;
    *totalDeposits = 0;

    for (i = 0; i < NUMBER_OF_CUSTOMERS; i++)
    {
        for (j = 0; j < transactionCount[i]; j++)
        {
            if (transactionReversed[i][j])
                continue;

            if (transactionType[i][j] == 'W')
            {
                *totalWithdrawals +=
                    transactionAmount[i][j];

                (*activeTransactions)++;
            }
            else if (transactionType[i][j] == 'D')
            {
                *totalDeposits +=
                    transactionAmount[i][j];

                (*activeTransactions)++;
            }
        }
    }
}
