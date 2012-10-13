// SETUP_DIALOG.H : write a short description here...

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "../appconfig.h"

#ifndef SETUP_DIALOG_H
#define SETUP_DIALOG_H

#include "../glade_dialog.h"
class win_project;
#include <windows.h>

/*################################################################################################*/

class setup_dialog : public glade_dialog
{
	friend BOOL CALLBACK DlgProcSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) ;
	protected:
	
	win_project * prj;	
	HWND dialog;
	
	public:
	
	setup_dialog(win_project *);
	~setup_dialog(void);
	
	static void handler_Destroy(HWND, void *);
	
	static void handler_ButtonOK(HWND, void *);
	static void handler_ButtonCancel(HWND, void *);
};

/*################################################################################################*/

#endif	// SETUP_DIALOG_H

// eof
