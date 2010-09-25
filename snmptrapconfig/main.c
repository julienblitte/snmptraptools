#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "mainDialog.h"
#include "resource.h"

HINSTANCE hInst;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    hInst = hInstance;

    // The user interface is a modal dialog box
    if (DialogBox(hInstance, MAKEINTRESOURCE(ID_DIALOG_MAIN), NULL, (DLGPROC)dlgMainMessageHandler) != 0)
    {
        printf("error: %lu\n", GetLastError());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
