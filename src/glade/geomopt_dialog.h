// GEOMOPT_DIALOG.H : write a short description here...

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "../appconfig.h"

#ifndef GEOMOPT_DIALOG_H
#define GEOMOPT_DIALOG_H

#include <windows.h>

#include "../glade_dialog.h"

#include "../../libghemical/src//geomopt.h"

/*################################################################################################*/

class geomopt_dialog : public glade_dialog
{
	friend BOOL CALLBACK DlgProcGeomOpt(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) ;
	HWND hDlg;
	protected:
	
	geomopt_param * param;
	
	/*GtkWidget * dialog;
	
	GtkWidget * entry_nsteps;
	GtkWidget * entry_grad;
	GtkWidget * entry_delta_e;
	*/
	public:
		void OnInitDialog();
	geomopt_dialog(geomopt_param *);
	~geomopt_dialog(void);
	
	static void handler_Destroy(HWND, void *);
	
	static void handler_ButtonOK(HWND, void *);
	static void handler_ButtonCancel(HWND, void *);
};

/*################################################################################################*/

#endif	// GEOMOPT_DIALOG_H

// eof
