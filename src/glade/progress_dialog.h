// PROGRESS_DIALOG.H : write a short description here...

// Copyright (C) 2005 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "appconfig.h"

#ifndef PROGRESS_DIALOG_H
#define PROGRESS_DIALOG_H

#include "glade_dialog.h"

/*################################################################################################*/

class progress_dialog : public glade_dialog
{
	protected:
	
	GtkWidget * dialog;
	
	GtkWidget * entry_job;
	GtkWidget * drawingarea_job; int da_h; int da_w;
	GtkWidget * progressbar_job;
	
	bool cancel;
	
	int g_n; int g_sz; int g_fill;
	double * g_data;
	
	friend class gtk_project;
	
	public:
	
	progress_dialog(const char *, bool, int, int);
	~progress_dialog(void);
	
	static void handler_Destroy(GtkWidget *, gpointer);
	static void handler_ButtonCancel(GtkWidget *, gpointer);
	
	static gboolean handler_ExposeEvent(GtkWidget *, GdkEventExpose *, gpointer);
};

/*################################################################################################*/

#endif	// PROGRESS_DIALOG_H

// eof
