
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "window_icon.h"

static HICON appIcon = NULL;

void SetApplicationIcon(void *windowHandle)
{
    appIcon = (HICON)LoadImageA(
        NULL,
        "assets/Glossy Blue ATM Bank Icon.ico",
        IMAGE_ICON,
        0,
        0,
        LR_LOADFROMFILE | LR_DEFAULTSIZE
    );

    if (appIcon == NULL)
    {
        return;
    }

    if (windowHandle != NULL)
    {
        HWND hwnd = (HWND)windowHandle;

        SendMessageA(
            hwnd,
            WM_SETICON,
            ICON_SMALL,
            (LPARAM)appIcon
        );

        SendMessageA(
            hwnd,
            WM_SETICON,
            ICON_BIG,
            (LPARAM)appIcon
        );
    }
}

void CleanupApplicationIcon(void)
{
    if (appIcon != NULL)
    {
        DestroyIcon(appIcon);
        appIcon = NULL;
    }
}
