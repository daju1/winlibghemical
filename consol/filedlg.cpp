#include "stdafx.h"
#include "filedlg.h"
#include "tchar.h"

HRESULT SaveFileDlg(HWND hWnd, LPTSTR lpstrFile, TCHAR filter[], DWORD& nFilterIndex)
{
/*  Example:
 *			TCHAR filter[] =
 *                          TEXT("Bitmap files (*.bmp)\0*.x\0")
 *                          TEXT("Metafiles (*.emf)\0*.m\0")
 *                          TEXT("All Files (*.*)\0*.*\0");
 */
    HRESULT hr = S_OK;
	UINT iext;
//    SSavePlotData spdData;
    OPENFILENAME ofn;

    static TCHAR file[256];
    static TCHAR szFilepath[256];
    static TCHAR fileTitle[256];

#ifdef  UNICODE
	wcscpy( file, lpstrFile);
	wcscpy( fileTitle, TEXT(""));
	wcscpy( fileTitle, lpstrFile);
#else
    strcpy( file, lpstrFile);
    strcpy( fileTitle, TEXT(""));
    strcpy( fileTitle, lpstrFile);
#endif

    memset( &ofn, 0, sizeof(ofn) );
    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hWnd;
//    ofn.hInstance         = hInst;
    ofn.lpstrFilter       = filter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0L;
    ofn.nFilterIndex      = 0L;
    ofn.lpstrFile         = file;
    ofn.nMaxFile          = sizeof(file);
    ofn.lpstrFileTitle    = fileTitle;
    ofn.nMaxFileTitle     = sizeof(fileTitle);
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrDefExt       = NULL;
//    ofn.lCustData         = NULL;
//    ofn.lpfnHook          = NULL;
//    ofn.lCustData         = (LPARAM)&spdData;
//    ofn.lpfnHook          = DlgProcSaveMesh;
//    ofn.lpTemplateName    = MAKEINTRESOURCE(IDD_SAVEPLOT);
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER/* | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK*/;

//    spdData.bSaveSelectedOnly = FALSE;
 //   spdData.bSaveHierarchy = TRUE;
 //   spdData.bSaveAnimation = TRUE;
 //   spdData.xFormat = 0;//D3DXF_FILEFORMAT_TEXT;


    if ( ! GetSaveFileName( &ofn) )
    {
        TCHAR s[40];
        DWORD dwErr = CommDlgExtendedError();
		switch(dwErr)
		{
		case CDERR_DIALOGFAILURE :
			MessageBox(0,_T("The dialog box could not be created. "), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_FINDRESFAILURE :
			MessageBox(0, _T("The common dialog box function failed to find a specified resource."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_INITIALIZATION :
			MessageBox(0, _T("The common dialog box function failed during initialization. This error often occurs when sufficient memory is not available"), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_LOADRESFAILURE :
			MessageBox(0, _T("The common dialog box function failed to load a specified resource."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_LOADSTRFAILURE :
			MessageBox(0, _T("The common dialog box function failed to load a specified string."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_LOCKRESFAILURE :
			MessageBox(0, _T("The common dialog box function failed to lock a specified resource."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_MEMALLOCFAILURE :
			MessageBox(0,_T("The common dialog box function was unable to allocate memory for internal structures."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_MEMLOCKFAILURE :
			MessageBox(0, _T("The common dialog box function was unable to lock the memory associated with a handle."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_NOHINSTANCE :
			MessageBox(0, _T("The ENABLETEMPLATE flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a corresponding instance handle."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_NOHOOK :
			MessageBox(0, _T("The ENABLEHOOK flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a pointer to a corresponding hook procedure."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_NOTEMPLATE :
			MessageBox(0, _T("The ENABLETEMPLATE flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a corresponding template."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_REGISTERMSGFAIL :
			MessageBox(0, _T("The RegisterWindowMessage function returned an error code when it was called by the common dialog box function."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_STRUCTSIZE :
			MessageBox(0, _T("The lStructSize member of the initialization structure for the corresponding common dialog box is invalid."), _T("CommDlgExtendedError"),0);
			break;
		case FNERR_BUFFERTOOSMALL :
			MessageBox(0, _T("The buffer pointed to by the lpstrFile member of the OPENFILENAME structure is too small for the file name specified by the user. The first two bytes of the lpstrFile buffer contain an integer value specifying the size, in TCHARs, required to receive the full name."), _T("CommDlgExtendedError"),0);
			break;
		case FNERR_INVALIDFILENAME :
			{
				TCHAR err_str[255];
				wsprintf(err_str, _T("A file name is invalid.\n%s"), ofn.lpstrFile );
				MessageBox(0, err_str, _T("CommDlgExtendedError"), 0);
			}
			break;
		case FNERR_SUBCLASSFAILURE :
			MessageBox(0, _T("An attempt to subclass a list box failed because sufficient memory was not available."), _T("CommDlgExtendedError"),0);
			break;

		}
        if ( 0 != dwErr )
        {
            memset( &ofn, 0, sizeof(OPENFILENAME) );
            ofn.lStructSize       = 1024;//sizeof(OPENFILENAME_NT4W);
            ofn.hwndOwner         = hWnd;
//            ofn.hInstance         = hInst;
            ofn.lpstrFilter       = filter;
            ofn.lpstrCustomFilter = NULL;
            ofn.nMaxCustFilter    = 0L;
            ofn.nFilterIndex      = 1L;
            ofn.lpstrFile         = file;
            ofn.nMaxFile          = sizeof(file);
            ofn.lpstrFileTitle    = fileTitle;
            ofn.nMaxFileTitle     = sizeof(fileTitle);
            ofn.lpstrInitialDir   = NULL;
            ofn.lpstrDefExt       = NULL;
//            ofn.lCustData         = (LPARAM)&spdData;
//            ofn.lpfnHook          = DlgProcSaveMesh;
//            ofn.lpTemplateName    = MAKEINTRESOURCE(IDD_SAVEPLOT);
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER /*| OFN_ENABLETEMPLATE | OFN_ENABLEHOOK*/;

            if ( ! GetSaveFileName( &ofn) )
            {
                dwErr = CommDlgExtendedError();
                if ( 0 != dwErr )
                {
                    wsprintf( s, _T("GetOpenFileName failed with %x"), dwErr );
                    MessageBox( hWnd, s, _T("TexWin"), MB_OK | MB_SYSTEMMODAL );
                }
				hr = S_FALSE;
                goto e_Exit;
            }
        }
        else  // just a cancel, return
        {
			hr = S_FALSE;
            goto e_Exit;
        }
    }
	//===============================================================
	//===============================================================
	//===============================================================
	nFilterIndex = ofn.nFilterIndex;
	//===============================================================
	//===============================================================
	//===============================================================
	TCHAR *p, ext[16];
	p = filter;
	for (iext = 0; iext < 2*(ofn.nFilterIndex-1)+1; iext++)
	{
#ifdef UNICODE
		p = wcschr(p,'\0');
#else
		p = strchr(p,'\0');
#endif
		p = p+1;
	}
	p = p+1;
#ifdef UNICODE
	if (p && wcscmp(p,_T(".*")) != 0)
		wcscpy(ext,p);
	else
		wcscpy(ext,_T("\0"));
#else
	if (p && strcmp(p,".*") != 0)
		strcpy(ext,p);
	else
		strcpy(ext,"\0");
#endif
	//===============================================================
	//===============================================================
	//===============================================================
#ifdef UNICODE
	p = wcsrchr(file,'.');
#else
	p = strrchr(file,'.');
#endif
#ifdef UNICODE
	if (wcscmp(ext, _T("\0")) != 0)
#else
	if (strcmp(ext, "\0") != 0)
#endif
	{
		if (p)
		{
#ifdef UNICODE
			if (p && wcscmp(ext, p) == 0)
#else
			if (p && strcmp(ext, p) == 0)
#endif
			{
			}
			else
			{
#ifdef UNICODE
				wcscpy(p, ext);
#else
				strcpy(p, ext);
#endif
			}
		}
		else
		{
#ifdef UNICODE
			wcscat(file, ext);
#else
			strcat(file, ext);
#endif
		}
	}
#ifdef UNICODE
	wcscpy(lpstrFile, file);
#else
	strcpy(lpstrFile, file);
#endif
	//===============================================================
	//===============================================================
	//===============================================================
e_Exit:
    if (FAILED(hr))
    {
        MessageBox( hWnd, _T("Unabled to save the specified file!"), _T("Save failed!"), MB_OK);
    }

    return hr;
}


HRESULT OpenFileDlg(HWND hWnd, LPCTSTR lpstrFilter, LPTSTR lpstrFile, DWORD& nFilterIndex)
{
    HRESULT hr = S_OK;
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[4098];       // buffer for file name


	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	//
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	//
	ofn.lpstrFile[0] = '\0';
//	if (lpstrFile)
//		strcpy(ofn.lpstrFile, lpstrFile);
	ofn.lpstrFilter = lpstrFilter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 

	if (GetOpenFileName(&ofn)==TRUE)
	{
		if (ofn.lpstrFile != NULL)
		{
//			strcpy(szPath, ofn.lpstrFile);
			if (lpstrFile)				
#ifdef UNICODE
				wcscpy(lpstrFile, ofn.lpstrFile);
#else
				strcpy(lpstrFile, ofn.lpstrFile);
#endif


/*			strcpy(directory, ofn.lpstrFile);
			char *p1, *p2;
			p1 = strrchr(directory,'\\');
			p2 = strrchr(directory,'/');
			if(p1)
				*p1 = '\0';
			if(p2)
				*p2 = '\0';*/
		}

		/*FILE *stream;
		if ((stream = fopen(ofn.lpstrFile,"rt")) == NULL)
		{
			MessageBox(0, "Cannot open input file.\n", "Open Plot", 
				MB_OK | MB_ICONINFORMATION);
			return 0;
		}

		fclose(stream);*/

		nFilterIndex = ofn.nFilterIndex;
	}
	else
	{
//        char s[40];
        DWORD dwErr = CommDlgExtendedError();
		switch(dwErr)
		{
		case CDERR_DIALOGFAILURE :
			MessageBox(0, _T("The dialog box could not be created."), _T("CommDlgExtendedError"), 0);
			break;
		case CDERR_FINDRESFAILURE :
			MessageBox(0, _T("The common dialog box function failed to find a specified resource."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_INITIALIZATION :
			MessageBox(0, _T("The common dialog box function failed during initialization. This error often occurs when sufficient memory is not available"), _T("CommDlgExtendedError"), 0);
			break;
		case CDERR_LOADRESFAILURE :
			MessageBox(0, _T("The common dialog box function failed to load a specified resource."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_LOADSTRFAILURE :
			MessageBox(0, _T("The common dialog box function failed to load a specified string."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_LOCKRESFAILURE :
			MessageBox(0, _T("The common dialog box function failed to lock a specified resource."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_MEMALLOCFAILURE :
			MessageBox(0, _T("The common dialog box function was unable to allocate memory for internal structures."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_MEMLOCKFAILURE :
			MessageBox(0, _T("The common dialog box function was unable to lock the memory associated with a handle."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_NOHINSTANCE :
			MessageBox(0, _T("The ENABLETEMPLATE flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a corresponding instance handle."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_NOHOOK :
			MessageBox(0, _T("The ENABLEHOOK flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a pointer to a corresponding hook procedure."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_NOTEMPLATE :
			MessageBox(0, _T("The ENABLETEMPLATE flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a corresponding template."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_REGISTERMSGFAIL :
			MessageBox(0, _T("The RegisterWindowMessage function returned an error code when it was called by the common dialog box function."), _T("CommDlgExtendedError"),0);
			break;
		case CDERR_STRUCTSIZE :
			MessageBox(0, _T("The lStructSize member of the initialization structure for the corresponding common dialog box is invalid."), _T("CommDlgExtendedError"),0);
			break;
		case FNERR_BUFFERTOOSMALL :
			short size;
			memcpy(&size, ofn.lpstrFile, 2);
			TCHAR str[1024];
			wsprintf(str, _T("The buffer pointed to by the lpstrFile member of the OPENFILENAME structure is too small for the file name specified by the user. The first two bytes of the lpstrFile buffer contain an integer value specifying the size = %d, in TCHARs, required to receive the full name. "), size);
			MessageBox(0, str, _T("CommDlgExtendedError"),0);
			break;
		case FNERR_INVALIDFILENAME :
			{
				TCHAR err_str[255];
				wsprintf(err_str, _T("A file name is invalid.\n%s"), ofn.lpstrFile );
				MessageBox(0,err_str, _T("CommDlgExtendedError"),0);
			}
			break;
		case FNERR_SUBCLASSFAILURE :
			MessageBox(0, _T("An attempt to subclass a list box failed because sufficient memory was not available."), _T("CommDlgExtendedError"),0);
			break;

		}
		hr = S_FALSE ;
	}

    if (FAILED(hr))
    {
        MessageBox( hWnd, _T("Unabled to open the specified file!"), _T("Open failed!"), MB_OK);
    }

    return hr;

}

