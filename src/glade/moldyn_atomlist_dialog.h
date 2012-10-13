// MOLDYN_ATOMLIST_DIALOG.H : write a short description here...

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "../appconfig.h"

#ifndef MOLDYN_ATOMLIST_DIALOG_H
#define MOLDYN_ATOMLIST_DIALOG_H

#include "../glade_dialog.h"

#include "../../libghemical/src/moldyn.h"

/*################################################################################################*/

class moldyn_atomlist_dialog : public glade_dialog
{
	friend BOOL CALLBACK DlgProcAtomlistMolDyn(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) ;
	HWND hDlg;
	protected:
	
//	moldyn_param	* param;
	moldyn			* mld;

	HWND hList;
	HWND hWndEdit;

	vector<string> * headers;
	vector<vector<string> > * items;

	vector<vector<int> >		* checkings;
	vector<vector<int> >		* editings;

	int rows, cols;
	int edit_r, edit_c;
	
	public:
	static void OnLVN_ENDLABELEDIT(WPARAM wParam, LPARAM lParam, void *);
	static void OnLVN_GETDISPINFO(WPARAM wParam, LPARAM lParam, void *);
	static void OnWM_DRAWITEM(WPARAM wParam, LPARAM lParam, void * data);
	static void OnWM_USER_1(WPARAM wParam, LPARAM lParam, void * data);
	static void OnNM_CLICK(WPARAM wParam, LPARAM lParam, void * data);
	static void OnInitDialog(void *);
	
	moldyn_atomlist_dialog(moldyn *);
	~moldyn_atomlist_dialog(void);

	static void handler_Destroy(HWND, void *);
	
	static void handler_ButtonOK(HWND, void *);
	static void handler_ButtonCancel(HWND, void *);
	
};

/*################################################################################################*/

#endif	// MOLDYN_DIALOG_H

// eof
