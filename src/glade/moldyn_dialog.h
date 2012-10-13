// MOLDYN_DIALOG.H : write a short description here...

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "../appconfig.h"

#ifndef MOLDYN_DIALOG_H
#define MOLDYN_DIALOG_H

#include "../glade_dialog.h"

#include "../../libghemical/src/moldyn.h"

/*################################################################################################*/

class moldyn_dialog : public glade_dialog
{
	friend BOOL CALLBACK DlgProcMolDyn(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) ;
	HWND hDlg;
	protected:
	
	moldyn_param * param;
/*	
	GtkWidget * dialog;
	
	GtkWidget * entry_nsteps_h;
	GtkWidget * entry_nsteps_e;
	GtkWidget * entry_nsteps_s;
	GtkWidget * entry_temperature;
	GtkWidget * entry_timestep;
	GtkWidget * checkbutton_constant_e;
	GtkWidget * checkbutton_langevin;
	
	GtkWidget * entry_trajfile;
	GtkWidget * button_browse;
	GtkWidget * file_selector;
*/	
	public:
		void OnInitDialog();
	
	moldyn_dialog(moldyn_param *);
	~moldyn_dialog(void);

	static void handler_Destroy(HWND, void *);
	
	static void handler_ButtonOK(HWND, void *);
	static void handler_ButtonCancel(HWND, void *);
	
	static void handler_ButtonBrowse(HWND, void *);
	static void handler_FileChooser(HWND, void *);
};

/*################################################################################################*/

#endif	// MOLDYN_DIALOG_H

// eof
