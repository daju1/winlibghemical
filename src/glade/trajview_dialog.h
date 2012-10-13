// TRAJVIEW_DIALOG.H : write a short description here...

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "../appconfig.h"

#ifndef TRAJVIEW_DIALOG_H
#define TRAJVIEW_DIALOG_H

#include "../glade_dialog.h"
#include "../win32_project.h"

/*################################################################################################*/

class trajview_dialog : public glade_dialog
{
	protected:
	
	win_project * prj;
	HWND dialog;
	
	public:
	
	trajview_dialog(win_project *);
	~trajview_dialog(void);

	static void handler_Destroy(HWND, void *);
	
	static void handler_ButtonBegin(HWND, void *);
	static void handler_ButtonPrev(HWND, void *);
	static DWORD WINAPI handler_ButtonPlay(void *);
	static void handler_ButtonNext(HWND, void *);
	static void handler_ButtonEnd(HWND, void *);
	
	static void handler_ButtonClose(HWND, void *);
};

/*################################################################################################*/

#endif	// TRAJVIEW_DIALOG_H

// eof
