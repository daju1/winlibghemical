// FILE_IMPORT_DIALOG.CPP

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "file_import_dialog.h"

#include "filetrans.h"

#include <gtk/gtk.h>

#include <iostream>
#include <strstream>
using namespace std;

/*################################################################################################*/

file_import_dialog::file_import_dialog(gtk_project * p1) : glade_dialog("glade/file_import_dialog.glade")
{
	prj = p1;
	
	dialog = glade_xml_get_widget(xml, "file_import_dialog");
	if (dialog == NULL) { cout << "file_import_dialog : glade_xml_get_widget() failed!!!" << endl; return; }
	
	// initialize the widgets...
	
	entry_trajfile = glade_xml_get_widget(xml, "entry_filename");
	button_browse = glade_xml_get_widget(xml, "button_browse");
	file_selector = NULL;
	
	GtkWidget * optmenu = glade_xml_get_widget(xml, "optionmenu1");
	GtkWidget * submenu = gtk_menu_new(); GtkWidget * item;
	
	item = gtk_menu_item_new_with_label("Automatic");			// insert item "automatic"...
	gtk_widget_show(item); gtk_menu_append(GTK_MENU(submenu), item);
	
#ifdef ENABLE_OPENBABEL
	
	file_trans ft;
	for (unsigned int i = 0;i < ft.NumImports();i++)			// insert other items...
	{
		item = gtk_menu_item_new_with_label((char *) ft.GetImportDescription(i).c_str());
		gtk_widget_show(item); gtk_menu_append(GTK_MENU(submenu), item);
	}
	
#endif	// ENABLE_OPENBABEL
	
	gtk_option_menu_set_menu(GTK_OPTION_MENU(optmenu), submenu);
	
	// connect the handlers...
	
	glade_xml_signal_connect_data(xml, "on_dialog_destroy", (GtkSignalFunc) handler_Destroy, (gpointer) this);
	
	glade_xml_signal_connect_data(xml, "on_button_ok_clicked", (GtkSignalFunc) handler_ButtonOK, (gpointer) this);
	glade_xml_signal_connect_data(xml, "on_button_cancel_clicked", (GtkSignalFunc) handler_ButtonCancel, (gpointer) this);
	
	glade_xml_signal_connect_data(xml, "on_button_browse_clicked", (GtkSignalFunc) handler_ButtonBrowse, (gpointer) this);
	
	gtk_dialog_run(GTK_DIALOG(dialog));	// MODAL
	gtk_widget_destroy(dialog);		// MODAL
}

file_import_dialog::~file_import_dialog(void)
{
}

void file_import_dialog::handler_Destroy(GtkWidget *, gpointer data)		// not really needed...
{
	file_import_dialog * ref = (file_import_dialog *) data;
	//cout << "handler_Destroy() : ref = " << ref << endl;
}

void file_import_dialog::handler_ButtonOK(GtkWidget *, gpointer data)
{
	file_import_dialog * ref = (file_import_dialog *) data;
	//cout << "handler_ButtonOK() : ref = " << ref << endl;
	
	// do the import...
	// ^^^^^^^^^^^^^^^^
	
	GtkWidget * optmenu = glade_xml_get_widget(ref->xml, "optionmenu1");
	GtkWidget * submenu = gtk_option_menu_get_menu(GTK_OPTION_MENU(optmenu));
	GtkWidget * item = gtk_menu_get_active(GTK_MENU(submenu));
	int index = g_list_index(GTK_MENU_SHELL(submenu)->children, item);
	
	GtkWidget * entry_fn = glade_xml_get_widget(ref->xml, "entry_filename");
	const gchar * filename = gtk_entry_get_text(GTK_ENTRY(entry_fn));
	
#ifdef ENABLE_OPENBABEL
	
	ref->prj->ImportFile(filename, index);
	ref->prj->ParseProjectFileNameAndPath(filename);
	ref->prj->UpdateAllWindowTitles();
	
#endif	// ENABLE_OPENBABEL
	
	ref->prj->CenterCRDSet(0, true);
	ref->prj->UpdateAllGraphicsViews();
}

void file_import_dialog::handler_ButtonCancel(GtkWidget *, gpointer data)	// not really needed...
{
	file_import_dialog * ref = (file_import_dialog *) data;
	//cout << "handler_ButtonCancel() : ref = " << ref << endl;
}

void file_import_dialog::handler_ButtonBrowse(GtkWidget *, gpointer data)
{
	file_import_dialog * ref = (file_import_dialog *) data;
	//cout << "handler_ButtonBrowse() : ref = " << ref << endl;
	
	// handle the file selection...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	ref->file_selector = gtk_file_selection_new("Select a Trajectory File.");
	g_signal_connect(GTK_FILE_SELECTION(ref->file_selector)->ok_button, "clicked", G_CALLBACK(handler_FileChooser), data);
	
	g_signal_connect_swapped(GTK_FILE_SELECTION(ref->file_selector)->ok_button, "clicked", G_CALLBACK(gtk_widget_destroy), ref->file_selector);
	g_signal_connect_swapped(GTK_FILE_SELECTION(ref->file_selector)->cancel_button, "clicked", G_CALLBACK(gtk_widget_destroy), ref->file_selector);
	
	gtk_dialog_run(GTK_DIALOG(ref->file_selector)); ref->file_selector = NULL;
}

void file_import_dialog::handler_FileChooser(GtkWidget *, gpointer data)
{
	file_import_dialog * ref = (file_import_dialog *) data;
	//cout << "handler_FileChooser() : ref = " << ref << endl;
	
	// save the selected filename...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	const gchar * selected_filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(ref->file_selector));
	gtk_entry_set_text(GTK_ENTRY(ref->entry_trajfile), selected_filename);
}

/*################################################################################################*/

// eof
