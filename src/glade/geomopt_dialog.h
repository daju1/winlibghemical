// GEOMOPT_DIALOG.H : write a short description here...

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "appconfig.h"

#ifndef GEOMOPT_DIALOG_H
#define GEOMOPT_DIALOG_H

#include "glade_dialog.h"

#include <ghemical/geomopt.h>

/*################################################################################################*/

class geomopt_dialog : public glade_dialog
{
	protected:
	
	geomopt_param * param;
	
	GtkWidget * dialog;
	
	GtkWidget * entry_nsteps;
	GtkWidget * entry_grad;
	GtkWidget * entry_delta_e;
	
	public:
	
	geomopt_dialog(geomopt_param *);
	~geomopt_dialog(void);
	
	static void handler_Destroy(GtkWidget *, gpointer);
	
	static void handler_ButtonOK(GtkWidget *, gpointer);
	static void handler_ButtonCancel(GtkWidget *, gpointer);
};

/*################################################################################################*/

#endif	// GEOMOPT_DIALOG_H

// eof
