#ifndef GUI_H
#define GUI_H

#include "raylib.h"
#include "atm_logic.h"

/* Application window dimensions. */
#define SCREEN_WIDTH 1100
#define SCREEN_HEIGHT 700

/* Screens managed by the application. */
typedef enum
{
    PAGE_LOGIN,
    PAGE_DASHBOARD,
    PAGE_DEPOSIT,
    PAGE_WITHDRAW,
    PAGE_HISTORY,
    PAGE_ACCOUNT,
    PAGE_ADMIN
} Page;

/*
 * Shared application/UI state.
 * Definitions are in main.c; gui.c accesses them through these declarations.
 */
extern Color BG_COLOR;
extern Color SIDEBAR_COLOR;
extern Color PANEL_COLOR;
extern Color BLUE_COLOR;
extern Color WHITE_COLOR;
extern Color GREY_COLOR;
extern Color GREEN_COLOR;
extern Color RED_COLOR;

extern Page currentPage;
extern int currentCustomer;
extern int manualWithdrawal;
extern int adminFailedAttempts;
extern int adminLocked;

extern char cardInput[20];
extern char pinInput[20];
extern char amountInput[20];
extern char noteInputs[NUMBER_OF_DENOMINATIONS][10];

extern char message[180];
extern Color messageColor;

extern char *activeInput;
extern int activeInputLimit;

/* Input and transaction-display helpers used by main.c. */
void UpdateTextInput(void);
void ClearTransactionInputs(void);
int GetNoteQuantities(int selectedNotes[]);
void SetMessage(const char *text, Color color);
void SetResultMessage(int result);

/* Screen drawing functions called by main.c. */
void DrawLoginScreen(void);
void DrawSidebar(void);
void DrawAdminSidebar(void);
void DrawAdminScreen(void);
void DrawDashboard(void);
void DrawDepositScreen(void);
void DrawWithdrawScreen(void);
void DrawHistoryScreen(void);
void DrawAccountScreen(void);

#endif /* GUI_H */
