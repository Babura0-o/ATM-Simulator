#include "raylib.h"
#include "atm_logic.h"
#include "window_icon.h"
#include "gui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==========================================
// APPLICATION SETTINGS
// ==========================================

Color BG_COLOR       = {15, 23, 42, 255};
Color SIDEBAR_COLOR  = {20, 30, 52, 255};
Color PANEL_COLOR    = {30, 43, 68, 255};
Color BLUE_COLOR     = {55, 125, 235, 255};
Color WHITE_COLOR    = {240, 245, 255, 255};
Color GREY_COLOR     = {155, 170, 195, 255};
Color GREEN_COLOR    = {55, 190, 130, 255};
Color RED_COLOR      = {230, 85, 85, 255};

Page currentPage = PAGE_LOGIN;

int currentCustomer = -1;
int manualWithdrawal = 0;

/* Admin login protection: three failed attempts lock admin login
   until the application is restarted. */
int adminFailedAttempts = 0;
int adminLocked = 0;

char cardInput[20] = "";
char pinInput[20] = "";
char amountInput[20] = "";
char noteInputs[NUMBER_OF_DENOMINATIONS][10] = {{0}};

char message[180] = "Welcome to ATM SIMULATOR.";
Color messageColor = {55, 190, 130, 255};

char *activeInput = NULL;
int activeInputLimit = 0;


// ==========================================
// INPUT HELPERS
// ==========================================


// ==========================================
// DRAWING HELPERS
// ==========================================


int ButtonClicked(Rectangle bounds, Vector2 mouse)
{
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
           CheckCollisionPointRec(mouse, bounds);
}


// ==========================================
// LOGIN SCREEN
// ==========================================


// ==========================================
// SIDEBAR
// ==========================================


// ==========================================
// ADMIN SIDEBAR AND DASHBOARD
// ==========================================


// ==========================================
// DASHBOARD
// ==========================================


// ==========================================
// DEPOSIT SCREEN
// ==========================================


// ==========================================
// WITHDRAWAL SCREEN
// ==========================================


// ==========================================
// TRANSACTION HISTORY
// ==========================================


// ==========================================
// ACCOUNT DETAILS
// ==========================================


// ==========================================
// MAIN APPLICATION
// ==========================================

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "ATM SIMULATOR");

    // Set the custom Windows application icon.
    SetApplicationIcon(GetWindowHandle());

    while (!WindowShouldClose())
    {
        Vector2 mouse = GetMousePosition();

        UpdateTextInput();

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            if (currentPage == PAGE_LOGIN)
            {
                if (CheckCollisionPointRec(mouse, (Rectangle){400, 320, 300, 50}))
                {
                    activeInput = cardInput;
                    activeInputLimit = 19;
                }
                else if (CheckCollisionPointRec(mouse, (Rectangle){400, 415, 300, 50}))
                {
                    activeInput = pinInput;
                    activeInputLimit = 19;
                }
                else if (CheckCollisionPointRec(mouse, (Rectangle){400, 490, 300, 48}))
                {
                    int card = atoi(cardInput);
                    int pin = atoi(pinInput);

                    /* Admin authentication is checked separately from customers. */
                    if (adminLocked && card == 999999)
                    {
                        SetMessage("Admin login locked. Restart the application to retry.", RED_COLOR);
                    }
                    else if (atm_authenticate_admin(card, pin))
                    {
                        currentCustomer = -1;
                        currentPage = PAGE_ADMIN;
                        adminFailedAttempts = 0;
                        activeInput = NULL;

                        cardInput[0] = '\0';
                        pinInput[0] = '\0';

                        SetMessage("Administrator login successful.", GREEN_COLOR);
                    }
                    else if (card == 999999)
                    {
                        adminFailedAttempts++;

                        if (adminFailedAttempts >= 3)
                        {
                            adminLocked = 1;
                            SetMessage("Admin login locked after 3 failed attempts.", RED_COLOR);
                        }
                        else
                        {
                            snprintf(message, sizeof(message),
                                     "Incorrect admin PIN. Attempts remaining: %d",
                                     3 - adminFailedAttempts);
                            messageColor = RED_COLOR;
                        }
                    }
                    else
                    {
                        int result = atm_authenticate_customer(card, pin);

                        if (result >= 0)
                        {
                            currentCustomer = result;
                            atm_reset_failed_login(result);

                            currentPage = PAGE_DASHBOARD;
                            activeInput = NULL;

                            cardInput[0] = '\0';
                            pinInput[0] = '\0';

                            SetMessage("Login successful. Welcome!", GREEN_COLOR);
                        }
                        else
                        {
                            int remaining = atm_register_failed_login(card);

                            if (remaining == ATM_ERROR_BLOCKED)
                            {
                                SetMessage("Card blocked after failed login attempts.", RED_COLOR);
                            }
                            else if (remaining > 0)
                            {
                                snprintf(message, sizeof(message),
                                         "Incorrect credentials. Attempts remaining: %d",
                                         remaining);
                                messageColor = RED_COLOR;
                            }
                            else
                            {
                                SetMessage("Invalid card number or PIN.", RED_COLOR);
                            }
                        }
                    }

                    if (currentPage == PAGE_LOGIN)
                    {
                        pinInput[0] = '\0';
                        activeInput = pinInput;
                        activeInputLimit = 19;
                    }
                }
            }
            else if (currentPage == PAGE_ADMIN)
            {
                // Admin has a separate, restricted navigation area.
                if (CheckCollisionPointRec(mouse, (Rectangle){15, 650, 195, 40}))
                {
                    currentPage = PAGE_LOGIN;
                    currentCustomer = -1;
                    activeInput = NULL;
                    cardInput[0] = '\0';
                    pinInput[0] = '\0';
                    SetMessage("Administrator logged out.", GREEN_COLOR);
                }
            }
            else
            {
                // Sidebar navigation
                if (mouse.x < 230)
                {
                    if (CheckCollisionPointRec(mouse, (Rectangle){15, 170, 195, 43}))
                        currentPage = PAGE_DASHBOARD;

                    else if (CheckCollisionPointRec(mouse, (Rectangle){15, 225, 195, 43}))
                        currentPage = PAGE_DEPOSIT;

                    else if (CheckCollisionPointRec(mouse, (Rectangle){15, 280, 195, 43}))
                        currentPage = PAGE_WITHDRAW;

                    else if (CheckCollisionPointRec(mouse, (Rectangle){15, 335, 195, 43}))
                        currentPage = PAGE_HISTORY;

                    else if (CheckCollisionPointRec(mouse, (Rectangle){15, 390, 195, 43}))
                        currentPage = PAGE_ACCOUNT;

                    else if (CheckCollisionPointRec(mouse, (Rectangle){15, 650, 195, 40}))
                    {
                        currentPage = PAGE_LOGIN;
                        currentCustomer = -1;
                        SetMessage("You have logged out.", GREEN_COLOR);
                    }

                    activeInput = NULL;
                }
                else if (currentPage == PAGE_DASHBOARD)
                {
                    if (CheckCollisionPointRec(mouse, (Rectangle){275, 390, 235, 115}))
                        currentPage = PAGE_DEPOSIT;

                    else if (CheckCollisionPointRec(mouse, (Rectangle){535, 390, 235, 115}))
                        currentPage = PAGE_WITHDRAW;

                    else if (CheckCollisionPointRec(mouse, (Rectangle){795, 390, 250, 115}))
                        currentPage = PAGE_HISTORY;
                }
                else if (currentPage == PAGE_DEPOSIT)
                {
                    for (int i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
                    {
                        int y = 210 + i * 65;

                        if (CheckCollisionPointRec(mouse, (Rectangle){600, y, 180, 45}))
                        {
                            activeInput = noteInputs[i];
                            activeInputLimit = 10;
                        }
                    }

                    if (CheckCollisionPointRec(mouse, (Rectangle){305, 555, 220, 50}))
                    {
                        int selectedNotes[NUMBER_OF_DENOMINATIONS];
                        int total = GetNoteQuantities(selectedNotes);

                        if (total <= 0)
                        {
                            SetMessage("Enter valid deposit note quantities.", RED_COLOR);
                        }
                        else
                        {
                            int result = atm_deposit(
                                currentCustomer, total, selectedNotes
                            );

                            SetResultMessage(result);

                            if (result == ATM_SUCCESS)
                                ClearTransactionInputs();
                        }

                        activeInput = NULL;
                    }
                }
                else if (currentPage == PAGE_WITHDRAW)
                {
                    if (CheckCollisionPointRec(mouse, (Rectangle){305, 220, 300, 50}))
                    {
                        activeInput = amountInput;
                        activeInputLimit = 20;
                    }
                    else if (CheckCollisionPointRec(mouse, (Rectangle){650, 220, 250, 50}))
                    {
                        manualWithdrawal = !manualWithdrawal;
                        activeInput = NULL;
                        ClearTransactionInputs();
                    }
                    else if (manualWithdrawal)
                    {
                        for (int i = 0; i < NUMBER_OF_DENOMINATIONS; i++)
                        {
                            int x = 305 + (i % 2) * 330;
                            int y = 365 + (i / 2) * 65;

                            if (CheckCollisionPointRec(mouse, (Rectangle){x, y, 200, 42}))
                            {
                                activeInput = noteInputs[i];
                                activeInputLimit = 10;
                            }
                        }
                    }

                    if (CheckCollisionPointRec(mouse, (Rectangle){305, 555, 220, 50}))
                    {
                        int amount = atoi(amountInput);
                        int selectedNotes[NUMBER_OF_DENOMINATIONS] = {0};

                        if (manualWithdrawal)
                            GetNoteQuantities(selectedNotes);

                        int result = atm_withdraw(
                            currentCustomer,
                            amount,
                            manualWithdrawal,
                            selectedNotes
                        );

                        SetResultMessage(result);

                        if (result == ATM_SUCCESS)
                            ClearTransactionInputs();

                        activeInput = NULL;
                    }
                }
            }
        }

        BeginDrawing();
        ClearBackground(BG_COLOR);

        if (currentPage == PAGE_LOGIN)
        {
            DrawLoginScreen();
        }
        else if (currentPage == PAGE_ADMIN)
        {
            DrawAdminSidebar();
            DrawAdminScreen();
        }
        else
        {
            DrawSidebar();

            switch (currentPage)
            {
                case PAGE_DASHBOARD:
                    DrawDashboard();
                    break;

                case PAGE_DEPOSIT:
                    DrawDepositScreen();
                    break;

                case PAGE_WITHDRAW:
                    DrawWithdrawScreen();
                    break;

                case PAGE_HISTORY:
                    DrawHistoryScreen();
                    break;

                case PAGE_ACCOUNT:
                    DrawAccountScreen();
                    break;

                default:
                    break;
            }
        }

        EndDrawing();
    }

    // Release the Windows application icon.
    CleanupApplicationIcon();

    CloseWindow();

    return 0;
}
