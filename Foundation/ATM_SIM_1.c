#include <stdio.h>

#define NUMBER_OF_CUSTOMERS 3
#define NUMBER_OF_DENOMINATIONS 4
#define MAX_ATTEMPTS 3

// Customer information
int cardNumbers[NUMBER_OF_CUSTOMERS] = {123456, 234567, 345678};
int customerPINs[NUMBER_OF_CUSTOMERS] = {1234, 2345, 3456};
int accountBalances[NUMBER_OF_CUSTOMERS] = {15000, 25000, 10000};

// ATM cash inventory
int denominations[NUMBER_OF_DENOMINATIONS] = {500, 200, 100, 50};
int availableNotes[NUMBER_OF_DENOMINATIONS] = {20, 20, 20, 20};

// Clears the remaining characters in the input line
void clearInput()
{
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF)
    {
    }
}

// Reads an integer and handles invalid input
int readInteger(char prompt[], int *value)
{
    int result;

    while (1)
    {
        printf("%s", prompt);
        result = scanf("%d", value);

        if (result == 1)
        {
            clearInput();
            return 1;
        }

        if (result == EOF)
            return 0;

        printf("Invalid input! Enter a number.\n");
        clearInput();
    }
}

// Finds the customer using card number and PIN
int authenticateCustomer(int cardNumber, int pin)
{
    int i;

    for (i = 0; i < NUMBER_OF_CUSTOMERS; i++)
    {
        if (cardNumber == cardNumbers[i] && pin == customerPINs[i])
            return i;
    }

    return -1;
}

// Calculates the total physical cash in the ATM
int calculateTotalCash()
{
    int i, total = 0;

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
        total += denominations[i] * availableNotes[i];

    return total;
}

// Displays the customer's account balance
void checkBalance(int customerIndex)
{
    printf("\n========== ACCOUNT BALANCE ==========\n");
    printf("Available Balance: Rs. %d\n", accountBalances[customerIndex]);
    printf("=====================================\n");
}

// Displays the ATM's available cash
void displayCash()
{
    int i;

    printf("\n========== ATM CASH INVENTORY ==========\n");

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
        printf("Rs. %d notes: %d\n", denominations[i], availableNotes[i]);

    printf("----------------------------------------\n");
    printf("Total ATM Cash: Rs. %d\n", calculateTotalCash());
    printf("========================================\n");
}

// Finds an exact combination of notes automatically
int automaticSelection(int amount, int selectedNotes[])
{
    int a, b, c, d, remaining;

    for (a = 0; a < NUMBER_OF_DENOMINATIONS; a++)
        selectedNotes[a] = 0;

    for (a = availableNotes[0]; a >= 0; a--)
    {
        for (b = availableNotes[1]; b >= 0; b--)
        {
            for (c = availableNotes[2]; c >= 0; c--)
            {
                remaining = amount - a * denominations[0] - b * denominations[1] - c * denominations[2];

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

// Lets the customer select the notes manually
int manualSelection(int amount, int selectedNotes[])
{
    int i, quantity, total = 0;

    printf("\n========== MANUAL CASH SELECTION ==========\n");

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    {
        printf("Rs. %d: %d notes available\n",
               denominations[i], availableNotes[i]);
    }

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    {
        printf("\nDenomination: Rs. %d\n", denominations[i]);

        if (!readInteger("Enter quantity: ", &quantity))
            return 0;

        if (quantity < 0 || quantity > availableNotes[i])
        {
            printf("Invalid quantity! Available notes: %d\n",
                   availableNotes[i]);
            return 0;
        }

        selectedNotes[i] = quantity;
        total += quantity * denominations[i];
    }

    if (total != amount)
    {
        printf("\nSelected total: Rs. %d\n", total);
        printf("Required amount: Rs. %d\n", amount);
        printf("The selected notes do not match the withdrawal amount.\n");
        return 0;
    }

    return 1;
}

// Prints the withdrawal receipt
void printReceipt(int amount, int customerIndex, int selectedNotes[])
{
    int i;

    printf("\n========== WITHDRAWAL RECEIPT ==========\n");
    printf("Amount Withdrawn: Rs. %d\n", amount);
    printf("\nCash Dispensed:\n");

    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    {
        if (selectedNotes[i] > 0)
        {
            printf("Rs. %d x %d = Rs. %d\n",
                   denominations[i],
                   selectedNotes[i],
                   denominations[i] * selectedNotes[i]);
        }
    }

    printf("----------------------------------------\n");
    printf("Remaining Balance: Rs. %d\n", accountBalances[customerIndex]);
    printf("========================================\n");
    printf("Please collect your cash.\n");
}

// Handles the complete withdrawal process
void withdrawCash(int customerIndex)
{
    int amount, selection, i;
    int selectedNotes[NUMBER_OF_DENOMINATIONS] = {0};

    printf("\n========== CASH WITHDRAWAL ==========\n");

    if (!readInteger("Enter withdrawal amount: Rs. ", &amount))
        return;

    if (amount <= 0)
    {
        printf("Withdrawal amount must be positive.\n");
        return;
    }

    if (amount % 50 != 0)
    {
        printf("Amount must be a multiple of Rs. 50.\n");
        return;
    }

    if (amount > accountBalances[customerIndex])
    {
        printf("Insufficient account balance!\n");
        return;
    }

    if (amount > calculateTotalCash())
    {
        printf("Insufficient cash in the ATM!\n");
        return;
    }

    printf("\n1. Automatic denomination selection\n");
    printf("2. Manual denomination selection\n");

    if (!readInteger("Enter your choice: ", &selection))
        return;

    if (selection == 1)
    {
        if (!automaticSelection(amount, selectedNotes))
        {
            printf("ATM cannot dispense this exact amount.\n");
            return;
        }
    }
    else if (selection == 2)
    {
        if (!manualSelection(amount, selectedNotes))
        {
            printf("Withdrawal cancelled.\n");
            return;
        }
    }
    else
    {
        printf("Invalid selection!\n");
        return;
    }

    // Update balances and inventory only after validation
    for (i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
        availableNotes[i] -= selectedNotes[i];

    accountBalances[customerIndex] -= amount;

    printReceipt(amount, customerIndex, selectedNotes);
}

// Displays the main ATM menu
void displayMenu()
{
    printf("\n========== ATM MAIN MENU ==========\n");
    printf("1. Balance Enquiry\n");
    printf("2. Cash Withdrawal\n");
    printf("3. Display ATM Cash\n");
    printf("4. Logout\n");
    printf("5. Exit ATM\n");
    printf("===================================\n");
}

// Main program
int main()
{
    int cardNumber, pin, customerIndex;
    int attempts, choice;

    printf("====================================\n");
    printf("       WELCOME TO ATM SIMULATOR      \n");
    printf("====================================\n");

    while (1)
    {
        attempts = 0;
        customerIndex = -1;

        printf("\n========== CUSTOMER LOGIN ==========\n");

        while (attempts < MAX_ATTEMPTS)
        {
            if (!readInteger("Enter Card Number: ", &cardNumber) || !readInteger("Enter PIN: ", &pin))
            {
                printf("\nInput ended. Exiting ATM.\n");
                return 0;
            }

            customerIndex = authenticateCustomer(cardNumber, pin);

            if (customerIndex != -1)
            {
                printf("\nLogin Successful!\n");
                break;
            }

            attempts++;
            printf("\nIncorrect card number or PIN!\n");
            printf("Attempts remaining: %d\n", MAX_ATTEMPTS - attempts);
        }

        if (customerIndex == -1)
        {
            printf("\nMaximum login attempts exceeded.\n");
            printf("Please try again later.\n");
            continue;
        }

        while (1)
        {
            displayMenu();

            if (!readInteger("Enter your choice: ", &choice))
            {
                printf("\nInput ended. Exiting ATM.\n");
                return 0;
            }

            switch (choice)
            {
                case 1:
                    checkBalance(customerIndex);
                    break;

                case 2:
                    withdrawCash(customerIndex);
                    break;

                case 3:
                    displayCash();
                    break;

                case 4:
                    printf("\nYou have been logged out successfully.\n");
                    break;

                case 5:
                    printf("\nThank you for using our ATM Simulator!\n");
                    return 0;

                default:
                    printf("\nInvalid menu choice! Try again.\n");
            }

            if (choice == 4)
                break;
        }
    }

    return 0;
}