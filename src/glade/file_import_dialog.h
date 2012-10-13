// FILE_IMPORT_DIALOG.H : write a short description here...

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "appconfig.h"

#ifndef FILE_IMPORT_DIALOG_H
#define FILE_IMPORT_DIALOG_H

#include "glade_dialog.h"
#include "gtk_project.h"

/*################################################################################################*/

class file_import_dialog : public glade_dialog
{
	protected:
	
	gtk_project * prj;
	GtkWidget * dialog;
	
	GtkWidget * entry_trajfile;
	GtkWidget * button_browse;
	GtkWidget * file_selector;
	
	public:
	
	file_import_dialog(gtk_project *);
	~file_import_dialog(void);
	
	static void handler_Destroy(GtkWidget *, gpointer);
	
	static void handler_ButtonOK(GtkWidget *, gpointer);
	static void handler_ButtonCancel(GtkWidget *, gpointer);
	
	static void handler_ButtonBrowse(GtkWidget *, gpointer);
	static void handler_FileChooser(GtkWidget *, gpointer);
};

/*################################################################################################*/

#endif	// FILE_IMPORT_DIALOG_H

// eof
