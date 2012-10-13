#ifndef WINGHEMICAL_H
#define WINGHEMICAL_H
#include <windows.h>
#define WM_MOUSEWHEEL                   0x020A

int WinGhemical();
ATOM MyRegisterClass(HINSTANCE hInstance);
int BeginWinGhemical();
HWND CreateWinGhemical( void * p);
DWORD WINAPI StartWinGhemical(void * p);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
#endif /*WINGHEMICAL_H*/
