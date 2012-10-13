#ifndef FILEGLG_H
#define FILEGLG_H

#include <windows.h>
#include <cderr.h>
#include <Commdlg.h>

HRESULT SaveFileDlg(HWND hWnd, LPTSTR lpstrFile, TCHAR filter[], DWORD& nFilterIndex);
HRESULT OpenFileDlg(HWND hWnd, LPCTSTR lpstrFilter, LPTSTR lpstrFile, DWORD& nFilterIndex);

#endif