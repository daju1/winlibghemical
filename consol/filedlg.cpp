#include "stdafx.h"
#include "filedlg.h"

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

    strcpy( file, lpstrFile);
    strcpy( fileTitle, TEXT(""));
    strcpy( fileTitle, lpstrFile);

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
        char s[40];
        DWORD dwErr = CommDlgExtendedError();
		switch(dwErr)
		{
		case CDERR_DIALOGFAILURE :
			MessageBox(0,"The dialog box could not be created. ","CommDlgExtendedError",0);
			break;
		case CDERR_FINDRESFAILURE :
			MessageBox(0,"The common dialog box function failed to find a specified resource.","CommDlgExtendedError",0);
			break;
		case CDERR_INITIALIZATION :
			MessageBox(0,"The common dialog box function failed during initialization. This error often occurs when sufficient memory is not available","CommDlgExtendedError",0);
			break;
		case CDERR_LOADRESFAILURE :
			MessageBox(0,"The common dialog box function failed to load a specified resource.","CommDlgExtendedError",0);
			break;
		case CDERR_LOADSTRFAILURE :
			MessageBox(0,"The common dialog box function failed to load a specified string.","CommDlgExtendedError",0);
			break;
		case CDERR_LOCKRESFAILURE :
			MessageBox(0,"The common dialog box function failed to lock a specified resource.","CommDlgExtendedError",0);
			break;
		case CDERR_MEMALLOCFAILURE :
			MessageBox(0,"The common dialog box function was unable to allocate memory for internal structures.","CommDlgExtendedError",0);
			break;
		case CDERR_MEMLOCKFAILURE :
			MessageBox(0,"The common dialog box function was unable to lock the memory associated with a handle.","CommDlgExtendedError",0);
			break;
		case CDERR_NOHINSTANCE :
			MessageBox(0,"The ENABLETEMPLATE flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a corresponding instance handle.","CommDlgExtendedError",0);
			break;
		case CDERR_NOHOOK :
			MessageBox(0,"The ENABLEHOOK flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a pointer to a corresponding hook procedure.","CommDlgExtendedError",0);
			break;
		case CDERR_NOTEMPLATE :
			MessageBox(0,"The ENABLETEMPLATE flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a corresponding template.","CommDlgExtendedError",0);
			break;
		case CDERR_REGISTERMSGFAIL :
			MessageBox(0,"The RegisterWindowMessage function returned an error code when it was called by the common dialog box function.","CommDlgExtendedError",0);
			break;
		case CDERR_STRUCTSIZE :
			MessageBox(0,"The lStructSize member of the initialization structure for the corresponding common dialog box is invalid.","CommDlgExtendedError",0);
			break;
		case FNERR_BUFFERTOOSMALL :
			MessageBox(0,"The buffer pointed to by the lpstrFile member of the OPENFILENAME structure is too small for the file name specified by the user. The first two bytes of the lpstrFile buffer contain an integer value specifying the size, in TCHARs, required to receive the full name. ","CommDlgExtendedError",0);
			break;
		case FNERR_INVALIDFILENAME :
			{
				char err_str[255];
				sprintf(err_str, "A file name is invalid.\n%s", ofn.lpstrFile );
				MessageBox(0,err_str,"CommDlgExtendedError",0);
			}
			break;
		case FNERR_SUBCLASSFAILURE :
			MessageBox(0,"An attempt to subclass a list box failed because sufficient memory was not available.","CommDlgExtendedError",0);
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
                    sprintf( s, "GetOpenFileName failed with %x", dwErr );
                    MessageBox( hWnd, s, "TexWin", MB_OK | MB_SYSTEMMODAL );
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
	char *p, ext[16];
	p = filter;
	for (iext = 0; iext < 2*(ofn.nFilterIndex-1)+1; iext++)
	{
		p = strchr(p,'\0');
		p = p+1;
	}
	p = p+1;
	if (p && strcmp(p,".*") != 0)
		strcpy(ext,p);
	else
		strcpy(ext,"\0");
	//===============================================================
	//===============================================================
	//===============================================================
	p = strrchr(file,'.');
	if (strcmp(ext, "\0") != 0)
	{
		if (p)
		{
			if (p && strcmp(ext, p) == 0)
			{
			}
			else
			{
				strcpy(p, ext);
			}
		}
		else
		{
			strcat(file, ext);
		}
	}
	strcpy(lpstrFile, file);
	//===============================================================
	//===============================================================
	//===============================================================
e_Exit:
    if (FAILED(hr))
    {
        MessageBox( hWnd, "Unabled to save the specified file!", "Save failed!", MB_OK);
    }

    return hr;
}


HRESULT OpenFileDlg(HWND hWnd, LPCTSTR lpstrFilter, LPTSTR lpstrFile, DWORD& nFilterIndex)
{
    HRESULT hr = S_OK;
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[4098];       // buffer for file name


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
				strcpy(lpstrFile, ofn.lpstrFile);


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
			MessageBox(0,"The dialog box could not be created. ","CommDlgExtendedError",0);
			break;
		case CDERR_FINDRESFAILURE :
			MessageBox(0,"The common dialog box function failed to find a specified resource.","CommDlgExtendedError",0);
			break;
		case CDERR_INITIALIZATION :
			MessageBox(0,"The common dialog box function failed during initialization. This error often occurs when sufficient memory is not available","CommDlgExtendedError",0);
			break;
		case CDERR_LOADRESFAILURE :
			MessageBox(0,"The common dialog box function failed to load a specified resource.","CommDlgExtendedError",0);
			break;
		case CDERR_LOADSTRFAILURE :
			MessageBox(0,"The common dialog box function failed to load a specified string.","CommDlgExtendedError",0);
			break;
		case CDERR_LOCKRESFAILURE :
			MessageBox(0,"The common dialog box function failed to lock a specified resource.","CommDlgExtendedError",0);
			break;
		case CDERR_MEMALLOCFAILURE :
			MessageBox(0,"The common dialog box function was unable to allocate memory for internal structures.","CommDlgExtendedError",0);
			break;
		case CDERR_MEMLOCKFAILURE :
			MessageBox(0,"The common dialog box function was unable to lock the memory associated with a handle.","CommDlgExtendedError",0);
			break;
		case CDERR_NOHINSTANCE :
			MessageBox(0,"The ENABLETEMPLATE flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a corresponding instance handle.","CommDlgExtendedError",0);
			break;
		case CDERR_NOHOOK :
			MessageBox(0,"The ENABLEHOOK flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a pointer to a corresponding hook procedure.","CommDlgExtendedError",0);
			break;
		case CDERR_NOTEMPLATE :
			MessageBox(0,"The ENABLETEMPLATE flag was set in the Flags member of the initialization structure for the corresponding common dialog box, but you failed to provide a corresponding template.","CommDlgExtendedError",0);
			break;
		case CDERR_REGISTERMSGFAIL :
			MessageBox(0,"The RegisterWindowMessage function returned an error code when it was called by the common dialog box function.","CommDlgExtendedError",0);
			break;
		case CDERR_STRUCTSIZE :
			MessageBox(0,"The lStructSize member of the initialization structure for the corresponding common dialog box is invalid.","CommDlgExtendedError",0);
			break;
		case FNERR_BUFFERTOOSMALL :
			short size;
			memcpy(&size, ofn.lpstrFile, 2);
			char str[1024];
			sprintf(str, "The buffer pointed to by the lpstrFile member of the OPENFILENAME structure is too small for the file name specified by the user. The first two bytes of the lpstrFile buffer contain an integer value specifying the size = %d, in TCHARs, required to receive the full name. ", size);
			MessageBox(0, str,"CommDlgExtendedError",0);
			break;
		case FNERR_INVALIDFILENAME :
			{
				char err_str[255];
				sprintf(err_str, "A file name is invalid.\n%s", ofn.lpstrFile );
				MessageBox(0,err_str,"CommDlgExtendedError",0);
			}
			break;
		case FNERR_SUBCLASSFAILURE :
			MessageBox(0,"An attempt to subclass a list box failed because sufficient memory was not available.","CommDlgExtendedError",0);
			break;

		}
		hr = S_FALSE ;
	}

    if (FAILED(hr))
    {
        MessageBox( hWnd, "Unabled to open the specified file!", "Open failed!", MB_OK);
    }

    return hr;

}

