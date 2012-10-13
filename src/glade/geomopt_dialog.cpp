// GEOMOPT_DIALOG.CPP

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "geomopt_dialog.h"

#include <gtk/gtk.h>

#include <iostream>
#include <strstream>
using namespace std;

/*################################################################################################*/

geomopt_dialog::geomopt_dialog(geomopt_param * p1) : glade_dialog("glade/geomopt_dialog.glade")
{
	param = p1;
	
	dialog = glade_xml_get_widget(xml, "geomopt_dialog");
	if (dialog == NULL) { cout << "geomopt_dialog : glade_xml_get_widget() failed!!!" << endl; return; }
	
	// initialize the widgets...
	
	entry_nsteps = glade_xml_get_widget(xml, "entry_nsteps");
	entry_grad = glade_xml_get_widget(xml, "entry_grad");
	entry_delta_e = glade_xml_get_widget(xml, "entry_delta_e");
	
	char buffer[64];
	
	ostrstream str_nsteps(buffer, sizeof(buffer));
	str_nsteps << param->treshold_nsteps << ends;
	gtk_entry_set_text(GTK_ENTRY(entry_nsteps), buffer);
	
	ostrstream str_grad(buffer, sizeof(buffer));
	str_grad << param->treshold_grad << ends;
	gtk_entry_set_text(GTK_ENTRY(entry_grad), buffer);
	
	ostrstream str_delta_e(buffer, sizeof(buffer));
	str_delta_e << param->treshold_delta_e << ends;
	gtk_entry_set_text(GTK_ENTRY(entry_delta_e), buffer);
	
	// connect the handlers...
	
	glade_xml_signal_connect_data(xml, "on_dialog_destroy", (GtkSignalFunc) handler_Destroy, (gpointer) this);
	
	glade_xml_signal_connect_data(xml, "on_button_ok_clicked", (GtkSignalFunc) handler_ButtonOK, (gpointer) this);
	glade_xml_signal_connect_data(xml, "on_button_cancel_clicked", (GtkSignalFunc) handler_ButtonCancel, (gpointer) this);
	
	gtk_dialog_run(GTK_DIALOG(dialog));	// MODAL
	gtk_widget_destroy(dialog);		// MODAL
}

geomopt_dialog::~geomopt_dialog(void)
{
}

void geomopt_dialog::handler_Destroy(GtkWidget *, gpointer data)		// not really needed...
{
	geomopt_dialog * ref = (geomopt_dialog *) data;
	//cout << "handler_Destroy() : ref = " << ref << endl;
}

void geomopt_dialog::handler_ButtonOK(GtkWidget *, gpointer data)
{
	geomopt_dialog * ref = (geomopt_dialog *) data;
	//cout << "handler_ButtonOK() : ref = " << ref << endl;
	
	// read the user's settings from widgets...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	GtkWidget * cb_nsteps = glade_xml_get_widget(ref->xml, "checkbutton_nsteps");
	GtkWidget * cb_grad = glade_xml_get_widget(ref->xml, "checkbutton_grad");
	GtkWidget * cb_delta_e = glade_xml_get_widget(ref->xml, "checkbutton_delta_e");
	
	const gchar * buffer;
	
	ref->param->enable_nsteps = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cb_nsteps));
	buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_nsteps));
	istrstream istr1(buffer, strlen(buffer) + 1);
	istr1 >> ref->param->treshold_nsteps;
	
	ref->param->enable_grad = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cb_grad));
	buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_grad));
	istrstream istr2(buffer, strlen(buffer) + 1);
	istr2 >> ref->param->treshold_grad;
	
	ref->param->enable_delta_e = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cb_delta_e));
	buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_delta_e));
	istrstream istr3(buffer, strlen(buffer) + 1);
	istr3 >> ref->param->treshold_delta_e;
	
	if (!ref->param->enable_nsteps && !ref->param->enable_grad && !ref->param->enable_delta_e)
	{
		ref->param->enable_nsteps = true;
		cout << "WARNING: all termination tests were disabled!!! the nsteps test is forced." << endl;
	}
	
	ref->param->confirm = true;
}

void geomopt_dialog::handler_ButtonCancel(GtkWidget *, gpointer data)		// not really needed...
{
	geomopt_dialog * ref = (geomopt_dialog *) data;
	//cout << "handler_ButtonCancel() : ref = " << ref << endl;
}

/*################################################################################################*/

// eof
