#include "gui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * GUI implementation.
 *
 * The application state is currently owned by main.c and exposed through
 * gui.h. ATM data is provided by atm_logic.h.
 */

void UpdateTextInput(void)
{
    if (activeInput == NULL)
        return;

    int key = GetCharPressed();

    while (key > 0)
    {
        int length = (int)strlen(activeInput);

        if (key >= '0' && key <= '9' &&
            length < activeInputLimit - 1)
        {
            activeInput[length] = (char)key;
            activeInput[length + 1] = '\0';
        }

        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE))
    {
        int length = (int)strlen(activeInput);

        if (length > 0)
            activeInput[length - 1] = '\0';
    }
}


void ClearTransactionInputs(void)
{
    amountInput[0] = '\0';

    for (int i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
        noteInputs[i][0] = '\0';
}


int GetNoteQuantities(int selectedNotes[])
{
    int total = 0;

    for (int i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    {
        selectedNotes[i] = atoi(noteInputs[i]);

        if (selectedNotes[i] < 0)
            return -1;

        total += selectedNotes[i] * denominations[i];
    }

    return total;
}


void SetMessage(const char *text, Color color)
{
    snprintf(message, sizeof(message), "%s", text);
    messageColor = color;
}


void SetResultMessage(int result)
{
    switch (result)
    {
        case ATM_SUCCESS:
            SetMessage("Transaction completed successfully!", GREEN_COLOR);
            break;

        case ATM_ERROR_INVALID:
            SetMessage("Invalid amount or input.", RED_COLOR);
            break;

        case ATM_ERROR_MAINTENANCE:
            SetMessage("ATM is currently under maintenance.", RED_COLOR);
            break;

        case ATM_ERROR_HARDWARE:
            SetMessage("Required ATM hardware is unavailable.", RED_COLOR);
            break;

        case ATM_ERROR_LIMIT:
            SetMessage("Withdrawal limit exceeded.", RED_COLOR);
            break;

        case ATM_ERROR_BALANCE:
            SetMessage("Insufficient account balance.", RED_COLOR);
            break;

        case ATM_ERROR_CASH:
            SetMessage("Insufficient ATM cash.", RED_COLOR);
            break;

        case ATM_ERROR_CAPACITY:
            SetMessage("ATM cash capacity exceeded.", RED_COLOR);
            break;

        case ATM_ERROR_NOTES:
            SetMessage("Invalid note selection or notes unavailable.", RED_COLOR);
            break;

        case ATM_ERROR_BLOCKED:
            SetMessage("This card is blocked. Contact the administrator.", RED_COLOR);
            break;

        default:
            SetMessage("Transaction could not be completed.", RED_COLOR);
            break;
    }
}


static void DrawPanel(int x, int y, int width, int height)
{
    DrawRectangleRounded(
        (Rectangle){x, y, width, height},
        0.08f, 10, PANEL_COLOR
    );
}


static void DrawButton(int x, int y, int width, int height,
                const char *text, Color color)
{
    DrawRectangleRounded(
        (Rectangle){x, y, width, height},
        0.18f, 10, color
    );

    int textWidth = MeasureText(text, 18);

    DrawText(
        text,
        x + (width - textWidth) / 2,
        y + (height - 18) / 2,
        18,
        WHITE_COLOR
    );
}


static void DrawInputBox(const char *label, char *value,
                  int x, int y, int width, int height,
                  int isActive, int isPassword)
{
    DrawText(label, x, y - 27, 16, GREY_COLOR);

    DrawRectangleRounded(
        (Rectangle){x, y, width, height},
        0.12f, 8,
        isActive ? BLUE_COLOR : PANEL_COLOR
    );

    char displayText[100];

    if (isPassword)
    {
        int length = (int)strlen(value);

        for (int i = 0; i < length; i++)
            displayText[i] = '*';

        displayText[length] = '\0';
    }
    else
    {
        snprintf(displayText, sizeof(displayText), "%s", value);
    }

    DrawText(
        displayText,
        x + 15, y + 13, 20,
        WHITE_COLOR
    );

    if (isActive && ((int)(GetTime() * 2) % 2 == 0))
    {
        int textWidth = MeasureText(displayText, 20);

        DrawLine(
            x + 15 + textWidth + 2,
            y + 10,
            x + 15 + textWidth + 2,
            y + 35,
            WHITE_COLOR
        );
    }
}


void DrawLoginScreen(void)
{
    DrawText("ATM", 440, 105, 48, BLUE_COLOR);
    DrawText("SIMULATOR", 520, 115, 35, WHITE_COLOR);

    DrawPanel(350, 190, 400, 365);

    DrawText("CUSTOMER / ADMIN LOGIN", 405, 220, 23, WHITE_COLOR);
    DrawText("Enter your banking credentials", 405, 255, 16, GREY_COLOR);

    DrawInputBox(
        "CARD NUMBER",
        cardInput,
        400, 320, 300, 50,
        activeInput == cardInput, 0
    );

    DrawInputBox(
        "PIN",
        pinInput,
        400, 415, 300, 50,
        activeInput == pinInput, 1
    );

    DrawButton(400, 490, 300, 48, "LOGIN", BLUE_COLOR);

    DrawText(message, 350, 590, 16, messageColor);

    DrawText("Demo customer: 123456 / 1234",
             405, 625, 15, GREY_COLOR);
    DrawText("Demo admin: 999999 / 9999",
             405, 647, 15, GREY_COLOR);
}


void DrawSidebar(void)
{
    DrawRectangle(0, 0, 230, SCREEN_HEIGHT, SIDEBAR_COLOR);

    DrawText("ATM", 35, 35, 35, BLUE_COLOR);
    DrawText("SIMULATOR", 35, 75, 23, WHITE_COLOR);

    DrawText("MAIN MENU", 30, 135, 14, GREY_COLOR);

    const char *labels[] =
    {
        "Dashboard",
        "Deposit",
        "Withdraw",
        "Transactions",
        "Account Details"
    };

    int positions[] = {170, 225, 280, 335, 390};

    for (int i = 0; i < 5; i++)
    {
        int selected = (int)currentPage == i + 1;

        if (selected)
        {
            DrawRectangleRounded(
                (Rectangle){15, positions[i], 195, 43},
                0.15f, 8, BLUE_COLOR
            );
        }

        DrawText(
            labels[i],
            32, positions[i] + 12,
            17,
            selected ? WHITE_COLOR : GREY_COLOR
        );
    }

    DrawLine(20, 590, 210, 590, GREY_COLOR);

    DrawText("ATM SYSTEM", 30, 610, 14, GREY_COLOR);
    DrawText(
        maintenanceMode ? "Maintenance Mode" : "ATM Online",
        30, 635, 16,
        maintenanceMode ? RED_COLOR : GREEN_COLOR
    );

    DrawText("Logout", 30, 665, 16, RED_COLOR);
}


void DrawAdminSidebar(void)
{
    DrawRectangle(0, 0, 230, SCREEN_HEIGHT, SIDEBAR_COLOR);

    DrawText("ATM", 35, 35, 35, BLUE_COLOR);
    DrawText("SIMULATOR", 35, 75, 23, WHITE_COLOR);

    DrawText("ADMINISTRATION", 30, 145, 14, GREY_COLOR);

    DrawRectangleRounded(
        (Rectangle){15, 180, 195, 43},
        0.15f, 8, BLUE_COLOR
    );

    DrawText("Overview", 32, 192, 17, WHITE_COLOR);

    DrawLine(20, 590, 210, 590, GREY_COLOR);

    DrawText("ADMIN SESSION", 30, 610, 14, GREY_COLOR);
    DrawText("Administrator", 30, 635, 16, GREEN_COLOR);
    DrawText("Logout", 30, 665, 16, RED_COLOR);
}


void DrawAdminScreen(void)
{
    int totalCash = 0;

    for (int i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
        totalCash += denominations[i] * availableNotes[i];

    DrawText("Administrator Dashboard", 275, 35, 32, WHITE_COLOR);
    DrawText("ATM overview and customer account summary",
             275, 80, 16, GREY_COLOR);

    // ATM cash summary
    DrawPanel(275, 120, 770, 115);
    DrawText("TOTAL ATM CASH", 305, 145, 16, GREY_COLOR);
    DrawText(TextFormat("Rs. %d", totalCash),
             305, 180, 35, WHITE_COLOR);

    DrawText("CASH INVENTORY", 275, 270, 20, WHITE_COLOR);
    DrawPanel(275, 310, 770, 125);

    for (int i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    {
        int x = 310 + (i % 2) * 350;
        int y = 330 + (i / 2) * 48;

        DrawText(
            TextFormat("Rs. %d notes: %d",
                       denominations[i], availableNotes[i]),
            x, y, 19, WHITE_COLOR
        );
    }

    DrawText("CUSTOMER ACCOUNTS", 275, 465, 20, WHITE_COLOR);
    DrawPanel(275, 505, 770, 125);

    DrawText("CUSTOMER", 305, 520, 14, GREY_COLOR);
    DrawText("CARD NUMBER", 535, 520, 14, GREY_COLOR);
    DrawText("BALANCE", 820, 520, 14, GREY_COLOR);

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
    {
        int y = 550 + i * 24;

        DrawText(TextFormat("Customer %d", i + 1),
                 305, y, 16, WHITE_COLOR);
        DrawText(TextFormat("%d", cardNumbers[i]),
                 535, y, 16, WHITE_COLOR);
        DrawText(TextFormat("Rs. %d", accountBalances[i]),
                 820, y, 16, GREEN_COLOR);
    }

    DrawText(message, 275, 655, 16, messageColor);
}


void DrawDashboard(void)
{
    DrawText("Dashboard", 275, 35, 32, WHITE_COLOR);
    DrawText("Your banking overview", 275, 78, 16, GREY_COLOR);

    DrawPanel(275, 125, 770, 185);

    DrawText("AVAILABLE BALANCE", 305, 155, 16, GREY_COLOR);

    DrawText(
        TextFormat("Rs. %d", accountBalances[currentCustomer]),
        305, 195, 42, WHITE_COLOR
    );

    DrawText(
        TextFormat("Daily withdrawal limit remaining: Rs. %d",
                   atm_get_remaining_daily_limit(currentCustomer)),
        305, 265, 17, GREY_COLOR
    );

    DrawText("QUICK ACTIONS", 275, 345, 20, WHITE_COLOR);

    DrawPanel(275, 390, 235, 115);
    DrawText("Deposit", 300, 415, 22, GREEN_COLOR);
    DrawText("Add money to account", 300, 455, 15, GREY_COLOR);

    DrawPanel(535, 390, 235, 115);
    DrawText("Withdraw", 560, 415, 22, BLUE_COLOR);
    DrawText("Withdraw cash", 560, 455, 15, GREY_COLOR);

    DrawPanel(795, 390, 250, 115);
    DrawText("Transactions", 820, 415, 22, WHITE_COLOR);
    DrawText("View recent activity", 820, 455, 15, GREY_COLOR);

    DrawText("ACCOUNT STATUS", 275, 555, 17, GREY_COLOR);
    DrawText("Active", 275, 585, 22, GREEN_COLOR);

    DrawText(message, 275, 640, 16, messageColor);
}


void DrawDepositScreen(void)
{
    DrawText("Cash Deposit", 275, 35, 32, WHITE_COLOR);
    DrawText("Enter the number of notes being deposited",
             275, 80, 16, GREY_COLOR);

    DrawPanel(275, 125, 770, 405);

    DrawText("NOTE DENOMINATIONS", 305, 155, 19, WHITE_COLOR);

    for (int i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
    {
        int y = 210 + i * 65;

        DrawText(
            TextFormat("Rs. %d notes", denominations[i]),
            315, y + 10, 18, WHITE_COLOR
        );

        DrawInputBox(
            "QUANTITY",
            noteInputs[i],
            600, y, 180, 45,
            activeInput == noteInputs[i], 0
        );
    }

    int selectedNotes[NUMBER_OF_DENOMINATIONS] = {0};
    int total = GetNoteQuantities(selectedNotes);

    DrawText(
        TextFormat("Deposit total: Rs. %d", total < 0 ? 0 : total),
        305, 480, 20, GREEN_COLOR
    );

    DrawButton(305, 555, 220, 50, "CONFIRM DEPOSIT", GREEN_COLOR);

    DrawText(message, 275, 630, 16, messageColor);
}


void DrawWithdrawScreen(void)
{
    DrawText("Cash Withdrawal", 275, 35, 32, WHITE_COLOR);
    DrawText("Withdraw money from your account",
             275, 80, 16, GREY_COLOR);

    DrawPanel(275, 125, 770, 405);

    DrawText(
        TextFormat("Available balance: Rs. %d",
                   accountBalances[currentCustomer]),
        305, 155, 19, WHITE_COLOR
    );

    DrawInputBox(
        "WITHDRAWAL AMOUNT",
        amountInput,
        305, 220, 300, 50,
        activeInput == amountInput, 0
    );

    DrawButton(
        650, 220, 250, 50,
        manualWithdrawal ? "MANUAL SELECTION" : "AUTOMATIC SELECTION",
        BLUE_COLOR
    );

    if (manualWithdrawal)
    {
        DrawText("Enter the notes you want", 305, 315, 17, GREY_COLOR);

        for (int i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
        {
            int x = 305 + (i % 2) * 330;
            int y = 365 + (i / 2) * 65;

            DrawInputBox(
                TextFormat("Rs. %d notes", denominations[i]),
                noteInputs[i],
                x, y, 200, 42,
                activeInput == noteInputs[i], 0
            );
        }
    }
    else
    {
        DrawText(
            "The ATM will automatically select available notes.",
            305, 350, 17, GREY_COLOR
        );
    }

    DrawText(
        TextFormat("Daily withdrawal limit remaining: Rs. %d",
                   atm_get_remaining_daily_limit(currentCustomer)),
        305, 475, 17, GREY_COLOR
    );

    DrawButton(305, 555, 220, 50, "CONFIRM WITHDRAWAL", BLUE_COLOR);

    DrawText(message, 275, 630, 16, messageColor);
}


void DrawHistoryScreen(void)
{
    DrawText("Transaction History", 275, 35, 32, WHITE_COLOR);
    DrawText("Your latest account transactions",
             275, 80, 16, GREY_COLOR);

    DrawPanel(275, 125, 770, 490);

    if (transactionCount[currentCustomer] == 0)
    {
        DrawText("No transactions recorded yet.",
                 315, 180, 20, GREY_COLOR);
        return;
    }

    DrawText("TYPE", 310, 155, 15, GREY_COLOR);
    DrawText("AMOUNT", 500, 155, 15, GREY_COLOR);
    DrawText("BALANCE", 710, 155, 15, GREY_COLOR);
    DrawText("STATUS", 900, 155, 15, GREY_COLOR);

    for (int i = transactionCount[currentCustomer] - 1; i >= 0; i--)
    {
        int y = 200 + (transactionCount[currentCustomer] - 1 - i) * 38;

        if (y > 575)
            break;

        char *type = "Unknown";
        Color typeColor = WHITE_COLOR;

        if (transactionType[currentCustomer][i] == 'W')
        {
            type = "Withdrawal";
            typeColor = BLUE_COLOR;
        }
        else if (transactionType[currentCustomer][i] == 'D')
        {
            type = "Deposit";
            typeColor = GREEN_COLOR;
        }
        else if (transactionType[currentCustomer][i] == 'R')
        {
            type = "Reversal";
            typeColor = RED_COLOR;
        }

        DrawText(type, 310, y, 16, typeColor);

        DrawText(
            TextFormat("Rs. %d", transactionAmount[currentCustomer][i]),
            500, y, 16, WHITE_COLOR
        );

        DrawText(
            TextFormat("Rs. %d", transactionBalance[currentCustomer][i]),
            710, y, 16, WHITE_COLOR
        );

        DrawText(
            transactionReversed[currentCustomer][i] ? "Reversed" : "Completed",
            900, y, 15,
            transactionReversed[currentCustomer][i] ? RED_COLOR : GREEN_COLOR
        );
    }
}


void DrawAccountScreen(void)
{
    DrawText("Account Details", 275, 35, 32, WHITE_COLOR);
    DrawText("Your registered banking information",
             275, 80, 16, GREY_COLOR);

    DrawPanel(275, 125, 770, 400);

    DrawText("ACCOUNT INFORMATION", 310, 160, 19, BLUE_COLOR);

    DrawText(
        TextFormat("Customer number: %d", currentCustomer + 1),
        310, 225, 20, WHITE_COLOR
    );

    DrawText(
        TextFormat("Card number: %d", cardNumbers[currentCustomer]),
        310, 275, 20, WHITE_COLOR
    );

    DrawText("Account type: Savings Account",
             310, 325, 20, WHITE_COLOR);

    DrawText(
        TextFormat("Current balance: Rs. %d",
                   accountBalances[currentCustomer]),
        310, 375, 20, WHITE_COLOR
    );

    DrawText("Account status: ACTIVE",
             310, 425, 20, GREEN_COLOR);
}
