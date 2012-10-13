// GTK_DIALOG.H : dialog boxes etc...

// Copyright (C) 1999 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef GTK_DIALOG_H
#define GTK_DIALOG_H

#include "appconfig.h"

class win_project;	// win_project.h

//#include <gtk/gtk.h>

#include "win32_app.h"
#include "win32_views.h"

// the dialogs here are made modal, so that only one dialog of it's kind can exist at any
// moment. therefore GtkWidgets are made static, because there is no need to store several
// GtkWidgets. if modal dialogs are needed, an array/container of GtkWidgets are needed
// in a similar way as in window classes...

/*################################################################################################*/

class element_dialog
{
	friend BOOL CALLBACK DlgProcElement(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) ;
	private:
	
	static element_dialog * current_object;
	static HWND dialog;
	static HWND current_element_widget;
	
	public:
	
	element_dialog(void);
	~element_dialog(void);
	
	static void SignalHandler(HWND, int);
	static void OnInitDialog();

};

/*################################################################################################*/

class bondtype_dialog
{
	friend BOOL CALLBACK DlgProcBondType(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) ;
	private:
	
	static bondtype_dialog * current_object;
	static HWND dialog;
	static HWND current_bond_widget;	

	public:
	
	bondtype_dialog(void);
	~bondtype_dialog(void);
	
	static void SignalHandler(HWND, int);
	static void OnInitDialog();
};

/*################################################################################################*/

const char * make_tmp_filename(win_project *);

class file_dialog
{
	protected:
	
	static HWND dialog;
	
	public:
	
	file_dialog(const char *, const char *, const char *);
	virtual ~file_dialog(void);
	
	static void DestroyHandler(HWND, void *);
	
	static void OkButtonHandler(HWND, void *);
	static void CancelButtonHandler(HWND, void *);
	
	virtual bool OkEvent(const char *) = 0;
	virtual void CancelEvent(void) = 0;
};

/*################################################################################################*/

class file_open_dialog : public file_dialog
{
	private:
	
	static const char title[];
	static win_project * prj;
	bool insert;
	
	public:
	
	file_open_dialog(win_project *);
	~file_open_dialog(void);
	
	bool OkEvent(const char *);	// virtual
	void CancelEvent(void);		// virtual
};

/*################################################################################################*/

class file_save_dialog : public file_dialog
{
	private:
	
	static const char title[];
	static win_project * prj;
	
	public:
	
	file_save_dialog(win_project *);
	~file_save_dialog(void);
	
	bool OkEvent(const char *);	// virtual
	void CancelEvent(void);		// virtual
};

/*################################################################################################*/

class file_save_graphics_dialog : public file_dialog
{
	private:
	
	static const char title[];
	static win_project * prj;
	
	public:
	
	file_save_graphics_dialog(win_project *);
	~file_save_graphics_dialog(void);
	
	bool OkEvent(const char *);	// virtual
	void CancelEvent(void);		// virtual
};

/*################################################################################################*/

class trajfile_dialog : public file_dialog
{
	private:
	
	static const char title[];
	static win_project * prj;
	
	public:
	
	trajfile_dialog(win_project *);
	~trajfile_dialog(void);
	
	bool OkEvent(const char *);	// virtual
	void CancelEvent(void);		// virtual
};

/*################################################################################################*/

class importpdb_dialog : public file_dialog
{
	private:
	
	static const char title[];
	static win_project * prj;
	
	public:
	
	importpdb_dialog(win_project *);
	~importpdb_dialog(void);
	
	bool OkEvent(const char *);	// virtual
	void CancelEvent(void);		// virtual
};

/*################################################################################################*/

class command_dialog
{
	private:
	
	static win_project * prj;
	static graphics_view * gv;
	
	static HWND dialog;
	static HWND entry;
	
	public:
	char command[512];
	command_dialog(win_project *, graphics_view *, const char *);
	virtual ~command_dialog(void);
	
	
	void OkButtonHandler(HWND);
};

/*################################################################################################*/

#endif	// GTK_DIALOG_H

// eof
