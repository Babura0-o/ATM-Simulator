#include <stdio.h>

#define NUMBER_OF_CUSTOMERS 3
#define NUMBER_OF_DENOMINATIONS 4
#define MAX_ATTEMPTS 3
#define MAX_WITHDRAWAL_PER_TRANSACTION 10000
#define DAILY_WITHDRAWAL_LIMIT 20000
#define ATM_CASH_CAPACITY 50000
#define MAX_HISTORY 10

#define ADMIN_ID 999999
#define ADMIN_PIN 9999

// Global data
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
int transactionNotes[NUMBER_OF_CUSTOMERS][MAX_HISTORY][NUMBER_OF_DENOMINATIONS];
int transactionReversed[NUMBER_OF_CUSTOMERS][MAX_HISTORY] = {0};
int transactionCount[NUMBER_OF_CUSTOMERS] = {0};

int maintenanceMode = 0;
int cashDispenserFailed = 0;
int depositSlotFailed = 0;

// Input handling
void clearInput() {
  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF) {
  }
}

int readInteger(char prompt[], int *value) {
  int result;

  while (1) {
    printf("%s", prompt);
    result = scanf("%d", value);

    if (result == 1) {
      clearInput();
      return 1;
    }

    if (result == EOF)
      return 0;

    printf("Invalid input! Enter a number.\n");
    clearInput();
  }
}

// Authentication helpers
int findCustomer(int cardNumber) {
  int i;
  for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
    if (cardNumber == cardNumbers[i])
      return i;
  }
  return -1;
}

int authenticateCustomer(int cardNumber, int pin) {
  int i;

  for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
    if (cardNumber == cardNumbers[i] && pin == customerPINs[i])
      return i;
  }

  return -1;
}

int authenticateAdmin(int id, int pin) {
  return id == ADMIN_ID && pin == ADMIN_PIN;
}

// ATM cash management
int calculateTotalCash() {
  int i, total = 0;

  for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    total += denominations[i] * availableNotes[i];

  return total;
}

void displayCash() {
  int i;

  printf("\n========== ATM CASH INVENTORY ==========\n");

  for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    printf("Rs. %d notes: %d\n", denominations[i], availableNotes[i]);

  printf("Total Cash: Rs. %d\n", calculateTotalCash());
  printf("Remaining Capacity: Rs. %d\n", ATM_CASH_CAPACITY - calculateTotalCash());
}

// Note selection
int automaticSelection(int amount, int selectedNotes[]) {
  int a, b, c, d, remaining, i;
  int maxA, maxB, maxC, remA, remB;

  for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    selectedNotes[i] = 0;

  maxA = amount / denominations[0];
  if (maxA > availableNotes[0])
    maxA = availableNotes[0];

  for (a = maxA; a >= 0; a--) {
    remA = amount - a * denominations[0];
    maxB = remA / denominations[1];
    if (maxB > availableNotes[1])
      maxB = availableNotes[1];

    for (b = maxB; b >= 0; b--) {
      remB = remA - b * denominations[1];
      maxC = remB / denominations[2];
      if (maxC > availableNotes[2])
        maxC = availableNotes[2];

      for (c = maxC; c >= 0; c--) {
        remaining = remB - c * denominations[2];

        if (remaining < 0 || remaining % denominations[3] != 0)
          continue;

        d = remaining / denominations[3];

        if (d <= availableNotes[3]) {
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

int selectNotes(int amount, int selectedNotes[], int isDeposit) {
  int i, quantity, total = 0, maximum, maxPossible;

  printf("\n========== NOTE SELECTION ==========\n");

  for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++) {
    maxPossible = amount / denominations[i];

    if (isDeposit) {
      maximum = maxPossible;
    } else {
      maximum = (availableNotes[i] < maxPossible) ? availableNotes[i] : maxPossible;
      printf("Rs. %d notes available: %d\n", denominations[i],
             availableNotes[i]);
    }

    if (!readInteger("Enter quantity: ", &quantity))
      return 0;

    if (quantity < 0 || quantity > maximum) {
      printf("Invalid quantity! Maximum allowed: %d\n", maximum);
      return 0;
    }

    selectedNotes[i] = quantity;
    total += quantity * denominations[i];
  }

  if (total != amount) {
    printf("Selected total: Rs. %d | Required: Rs. %d\n", total, amount);
    printf("Selected notes do not match the amount.\n");
    return 0;
  }

  return 1;
}

// Transaction history
void recordTransaction(int customerIndex, char type, int amount, int selectedNotes[]) {
  int i, j, position;

  if (transactionCount[customerIndex] == MAX_HISTORY) {
    for (i = 1; i < MAX_HISTORY; i++) {
      transactionType[customerIndex][i - 1] = transactionType[customerIndex][i];

      transactionAmount[customerIndex][i - 1] = transactionAmount[customerIndex][i];

      transactionBalance[customerIndex][i - 1] = transactionBalance[customerIndex][i];

      transactionReversed[customerIndex][i - 1] = transactionReversed[customerIndex][i];

      for (j = 0; j < NUMBER_OF_DENOMINATIONS; j++)
        transactionNotes[customerIndex][i - 1][j] = transactionNotes[customerIndex][i][j];
    }

    position = MAX_HISTORY - 1;
  } else {
    position = transactionCount[customerIndex];
    transactionCount[customerIndex]++;
  }

  transactionType[customerIndex][position] = type;
  transactionAmount[customerIndex][position] = amount;
  transactionBalance[customerIndex][position] = accountBalances[customerIndex];
  transactionReversed[customerIndex][position] = 0;

  for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    transactionNotes[customerIndex][position][i] = selectedNotes[i];
}

void displayTransactionHistory(int customerIndex) {
  int i;

  printf("\n========== TRANSACTION HISTORY ==========\n");

  if (transactionCount[customerIndex] == 0) {
    printf("No transactions recorded.\n");
    return;
  }

  for (i = 0; i < transactionCount[customerIndex]; i++) {
    printf("\nTransaction %d\n", i + 1);

    if (transactionType[customerIndex][i] == 'W')
      printf("Type: Withdrawal\n");
    else if (transactionType[customerIndex][i] == 'D')
      printf("Type: Deposit\n");
    else
      printf("Type: Reversal\n");

    printf("Amount: Rs. %d\n", transactionAmount[customerIndex][i]);
    printf("Balance: Rs. %d\n", transactionBalance[customerIndex][i]);

    if (transactionReversed[customerIndex][i])
      printf("Status: Reversed\n");
  }
}

// Receipts and balance
void printReceipt(int amount, int customerIndex, int selectedNotes[],
                  char type) {
  int i;

  printf("\n========== %s RECEIPT ==========\n",
         type == 'W' ? "WITHDRAWAL" : "DEPOSIT");

  printf("Amount: Rs. %d\n", amount);

  for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++) {
    if (selectedNotes[i] > 0)
      printf("Rs. %d x %d = Rs. %d\n", denominations[i], selectedNotes[i], denominations[i] * selectedNotes[i]);
  }

  printf("Updated Balance: Rs. %d\n", accountBalances[customerIndex]);

  if (type == 'W')
    printf("Remaining Daily Limit: Rs. %d\n", DAILY_WITHDRAWAL_LIMIT - dailyWithdrawals[customerIndex]);
}

void checkBalance(int customerIndex) {
  printf("\n========== ACCOUNT BALANCE ==========\n");
  printf("Available Balance: Rs. %d\n", accountBalances[customerIndex]);
  printf("Daily Withdrawal Limit: Rs. %d\n", DAILY_WITHDRAWAL_LIMIT);
  printf("Withdrawn Today: Rs. %d\n", dailyWithdrawals[customerIndex]);
  printf("Remaining Limit: Rs. %d\n", DAILY_WITHDRAWAL_LIMIT - dailyWithdrawals[customerIndex]);
}

// Withdrawals and deposits
void withdrawCash(int customerIndex) {
  int amount, choice, i;
  int selectedNotes[NUMBER_OF_DENOMINATIONS] = {0};

  if (!readInteger("Enter withdrawal amount: Rs. ", &amount))
    return;

  if (amount <= 0 || amount % 50 != 0) {
    printf("Enter a positive amount in multiples of Rs. 50.\n");
    return;
  }

  if (amount > MAX_WITHDRAWAL_PER_TRANSACTION ||
      amount > DAILY_WITHDRAWAL_LIMIT - dailyWithdrawals[customerIndex] ||
      amount > accountBalances[customerIndex] ||
      amount > calculateTotalCash()) {
    printf("Withdrawal rejected! Check your limits, balance and ATM cash.\n");
    return;
  }

  printf("1. Automatic note selection\n2. Manual note selection\n");

  if (!readInteger("Enter choice: ", &choice))
    return;

  if (choice == 1) {
    if (!automaticSelection(amount, selectedNotes)) {
      printf("ATM cannot dispense this exact amount.\n");
      return;
    }
  } else if (choice == 2) {
    if (!selectNotes(amount, selectedNotes, 0)) {
      printf("Withdrawal cancelled.\n");
      return;
    }
  } else {
    printf("Invalid selection!\n");
    return;
  }

  for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    availableNotes[i] -= selectedNotes[i];

  accountBalances[customerIndex] -= amount;
  dailyWithdrawals[customerIndex] += amount;

  recordTransaction(customerIndex, 'W', amount, selectedNotes);
  printReceipt(amount, customerIndex, selectedNotes, 'W');
}

void depositCash(int customerIndex) {
  int amount, i;
  int selectedNotes[NUMBER_OF_DENOMINATIONS] = {0};

  if (!readInteger("Enter deposit amount: Rs. ", &amount))
    return;

  if (amount <= 0 || amount % 50 != 0) {
    printf("Enter a positive amount in multiples of Rs. 50.\n");
    return;
  }

  if (amount > ATM_CASH_CAPACITY - calculateTotalCash()) {
    printf("ATM capacity exceeded!\n");
    return;
  }

  if (!selectNotes(amount, selectedNotes, 1)) {
    printf("Deposit cancelled.\n");
    return;
  }

  for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    availableNotes[i] += selectedNotes[i];

  accountBalances[customerIndex] += amount;

  recordTransaction(customerIndex, 'D', amount, selectedNotes);
  printReceipt(amount, customerIndex, selectedNotes, 'D');
}

// Customer session
void customerSession() {
  int cardNumber, pin, customerIndex = -1, cardIdx = -1;
  int attempts = 0, choice;

  if (maintenanceMode) {
    printf("\nATM is currently under maintenance. Please try again later.\n");
    return;
  }

  if (!readInteger("Enter Card Number: ", &cardNumber))
    return;

  cardIdx = findCustomer(cardNumber);
  if (cardIdx != -1 && cardBlocked[cardIdx]) {
    printf("This card is blocked due to excessive failed attempts! Contact administrator.\n");
    return;
  }

  while (attempts < MAX_ATTEMPTS) {
    if (!readInteger("Enter PIN: ", &pin))
      return;

    if (cardIdx != -1 && pin == customerPINs[cardIdx]) {
      customerIndex = cardIdx;
      break;
    }

    attempts++;
    printf("Incorrect credentials! Attempts remaining: %d\n",
           MAX_ATTEMPTS - attempts);
  }

  if (customerIndex == -1) {
    if (cardIdx != -1) {
      cardBlocked[cardIdx] = 1;
      printf("Maximum login attempts exceeded. Card has been blocked for security.\n");
    } else {
      printf("Maximum login attempts exceeded.\n");
    }
    return;
  }

  printf("\nCustomer login successful!\n");

  while (1) {
    printf("\n========== CUSTOMER MENU ==========\n");
    printf("1. Balance Enquiry\n");
    printf("2. Cash Withdrawal\n");
    printf("3. Display ATM Cash\n");
    printf("4. Cash Deposit\n");
    printf("5. Transaction History\n");
    printf("6. Logout\n");

    if (!readInteger("Enter choice: ", &choice))
      return;

    if (maintenanceMode && choice != 6) {
      printf("ATM is under maintenance.\n");
      continue;
    }

    switch (choice) {
    case 1:
      checkBalance(customerIndex);
      break;

    case 2:
      if (cashDispenserFailed)
        printf("Cash dispenser unavailable!\n");
      else
        withdrawCash(customerIndex);
      break;

    case 3:
      displayCash();
      break;

    case 4:
      if (depositSlotFailed)
        printf("Deposit slot unavailable!\n");
      else
        depositCash(customerIndex);
      break;

    case 5:
      displayTransactionHistory(customerIndex);
      break;

    case 6:
      printf("Logged out successfully.\n");
      return;

    default:
      printf("Invalid choice!\n");
    }
  }
}

// Admin cash management
void replenishCash() {
  int amount, i;
  int selectedNotes[NUMBER_OF_DENOMINATIONS] = {0};

  if (!readInteger("Enter replenishment amount: Rs. ", &amount))
    return;

  if (amount <= 0 || amount % 50 != 0 ||
      amount > ATM_CASH_CAPACITY - calculateTotalCash()) {
    printf("Invalid amount or ATM capacity exceeded!\n");
    return;
  }

  if (!selectNotes(amount, selectedNotes, 1))
    return;

  for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    availableNotes[i] += selectedNotes[i];
  printf("Cash replenishment successful!\n");
  displayCash();
}

void removeCash() {
  int amount, i;
  int selectedNotes[NUMBER_OF_DENOMINATIONS] = {0};

  if (!readInteger("Enter removal amount: Rs. ", &amount))
    return;

  if (amount <= 0 || amount % 50 != 0 || amount > calculateTotalCash()) {
    printf("Invalid amount or insufficient ATM cash!\n");
    return;
  }

  if (!selectNotes(amount, selectedNotes, 0))
    return;

  for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    availableNotes[i] -= selectedNotes[i];
  printf("Cash removal successful!\n");
  displayCash();
}

// Hardware management
void manageHardware() {
  int choice;

  while (1) {
    printf("\n========== HARDWARE MANAGEMENT ==========\n");
    printf("1. Toggle Cash Dispenser\n");
    printf("2. Toggle Deposit Slot\n");
    printf("3. Display Status\n");
    printf("4. Return\n");

    if (!readInteger("Enter choice: ", &choice))
      return;

    switch (choice) {
    case 1:
      cashDispenserFailed = !cashDispenserFailed;
      break;

    case 2:
      depositSlotFailed = !depositSlotFailed;
      break;

    case 3:
      printf("Cash dispenser: %s\n",
             cashDispenserFailed ? "FAILED" : "OPERATIONAL");
      printf("Deposit slot: %s\n",
             depositSlotFailed ? "FAILED" : "OPERATIONAL");
      continue;

    case 4:
      return;

    default:
      printf("Invalid choice!\n");
      continue;
    }
    printf("Hardware status updated.\n");
  }
}

// Transaction reversal
void reverseTransaction() {
  int customerIndex, transactionNumber, position;
  int amount, i, wasFull;
  int selectedNotes[NUMBER_OF_DENOMINATIONS];

  if (!readInteger("Enter customer number (1-3): ", &customerIndex))
    return;

  if (customerIndex < 1 || customerIndex > NUMBER_OF_CUSTOMERS) {
    printf("Invalid customer number!\n");
    return;
  }

  customerIndex--;

  if (transactionCount[customerIndex] == 0) {
    printf("No transactions recorded for this customer.\n");
    return;
  }

  displayTransactionHistory(customerIndex);

  if (!readInteger("Enter transaction number: ", &transactionNumber))
    return;

  if (transactionNumber < 1 ||
      transactionNumber > transactionCount[customerIndex]) {
    printf("Invalid transaction number!\n");
    return;
  }

  position = transactionNumber - 1;

  if (transactionType[customerIndex][position] == 'R' ||
      transactionReversed[customerIndex][position]) {
    printf("Transaction cannot be reversed again.\n");
    return;
  }

  amount = transactionAmount[customerIndex][position];

  for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    selectedNotes[i] = transactionNotes[customerIndex][position][i];

  if (transactionType[customerIndex][position] == 'W') {
    if (amount > ATM_CASH_CAPACITY - calculateTotalCash()) {
      printf("Insufficient ATM capacity for reversal!\n");
      return;
    }

    accountBalances[customerIndex] += amount;
    dailyWithdrawals[customerIndex] -= amount;
    if (dailyWithdrawals[customerIndex] < 0)
      dailyWithdrawals[customerIndex] = 0;

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
      availableNotes[i] += selectedNotes[i];
  } else if (transactionType[customerIndex][position] == 'D') {
    if (amount > accountBalances[customerIndex]) {
      printf("Insufficient customer balance!\n");
      return;
    }

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++) {
      if (selectedNotes[i] > availableNotes[i]) {
        printf("Insufficient ATM notes for reversal!\n");
        return;
      }
    }

    accountBalances[customerIndex] -= amount;

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
      availableNotes[i] -= selectedNotes[i];
  } else {
    printf("Unsupported transaction type!\n");
    return;
  }

  wasFull = (transactionCount[customerIndex] == MAX_HISTORY);
  recordTransaction(customerIndex, 'R', amount, selectedNotes);

  if (wasFull) {
    if (position > 0)
      transactionReversed[customerIndex][position - 1] = 1;
  } else {
    transactionReversed[customerIndex][position] = 1;
  }
  printf("Transaction reversal successful!\n");
  printf("Updated balance: Rs. %d\n", accountBalances[customerIndex]);
}

// Admin reports
void adminReports() {
  int i, j, resetChoice;
  int deposits = 0, withdrawals = 0, count = 0;

  for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
    for (j = 0; j < transactionCount[i]; j++) {
      if (transactionReversed[i][j])
        continue;

      if (transactionType[i][j] == 'W') {
        withdrawals += transactionAmount[i][j];
        count++;
      } else if (transactionType[i][j] == 'D') {
        deposits += transactionAmount[i][j];
        count++;
      }
    }
  }

  printf("\n========== ATM REPORT ==========\n");
  printf("Active transactions: %d\n", count);
  printf("Total withdrawals: Rs. %d\n", withdrawals);
  printf("Total deposits: Rs. %d\n", deposits);

  for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
    printf("Customer %d (Card %d) | Balance: Rs. %d | Today Withdrawn: Rs. %d | %s\n", i + 1, cardNumbers[i], accountBalances[i], dailyWithdrawals[i],
         cardBlocked[i] ? "BLOCKED" : "ACTIVE");
  }

  displayCash();

  printf("Maintenance: %s\n", maintenanceMode ? "ON" : "OFF");

  printf("\nDo you want to reset daily limits & unblock all cards? (1 = Yes, 0 = No): ");
  if (readInteger("", &resetChoice) && resetChoice == 1) {
    for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
      dailyWithdrawals[i] = 0;
      cardBlocked[i] = 0;
    }
    printf("All daily withdrawal limits reset and cards unblocked successfully.\n");
  }
}

// Admin session
void adminSession() {
  int id, pin, attempts = 0, choice;

  while (attempts < MAX_ATTEMPTS) {
    if (!readInteger("Enter Admin ID: ", &id) || !readInteger("Enter Admin PIN: ", &pin))
      return;

    if (authenticateAdmin(id, pin))
      break;

    attempts++;
    printf("Incorrect credentials! Attempts remaining: %d\n",
           MAX_ATTEMPTS - attempts);
  }

  if (attempts == MAX_ATTEMPTS) {
    printf("Maximum login attempts exceeded.\n");
    return;
  }

  printf("Administrator login successful!\n");

  while (1) {
    printf("\n========== ADMIN MENU ==========\n");
    printf("1. Display ATM Cash\n");
    printf("2. Replenish Cash\n");
    printf("3. Remove Cash\n");
    printf("4. Toggle Maintenance Mode\n");
    printf("5. Hardware Management\n");
    printf("6. Reverse Transaction\n");
    printf("7. View Reports\n");
    printf("8. Logout\n");

    if (!readInteger("Enter choice: ", &choice))
      return;

    switch (choice) {
    case 1:
      displayCash();
      break;

    case 2:
      replenishCash();
      break;

    case 3:
      removeCash();
      break;

    case 4:
      maintenanceMode = !maintenanceMode;
      printf("Maintenance mode: %s\n", maintenanceMode ? "ON" : "OFF");
      break;

    case 5:
      manageHardware();
      break;

    case 6:
      reverseTransaction();
      break;

    case 7:
      adminReports();
      break;

    case 8:
      printf("Administrator logged out.\n");
      return;

    default:
      printf("Invalid choice!\n");
    }
  }
}

// Main program
int main() {
  int choice;

  printf("====================================\n");
  printf("       ATM SIMULATOR VERSION 3.0    \n");
  printf("====================================\n");

  printf("Starting with default ATM data.\n");

  while (1) {
    printf("\n========== MAIN MENU ==========\n");
    printf("1. Customer Login\n");
    printf("2. Administrator Login\n");
    printf("3. Exit ATM\n");

    if (!readInteger("Enter choice: ", &choice))
      break;

    switch (choice) {
    case 1:
      customerSession();
      break;

    case 2:
      adminSession();
      break;

    case 3:
      printf("Thank you for using ATM Simulator 3.0!\n");
      return 0;

    default:
      printf("Invalid choice!\n");
    }
  }
  printf("ATM program terminated.\n");

  return 0;
}