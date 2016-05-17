// MOLDYN_DIALOG.CPP

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "../stdafx.h"
#include "../../libghemical/src/long_stream.h"

#include "moldyn_dialog.h"
#include "../../consol/filedlg.h"

//#include <gtk/gtk.h>

#include <iostream>
#include <strstream>
using namespace std;

/*################################################################################################*/
extern HINSTANCE hInst;								// current instance
BOOL CALLBACK DlgProcMolDyn(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	static moldyn_dialog * moldyn_dlg;
    switch (message) 
    { 
	case WM_INITDIALOG:
		{
			moldyn_dlg = reinterpret_cast<moldyn_dialog *>(lParam);
			if (moldyn_dlg)
			{
				moldyn_dlg->hDlg = hDlg;
				moldyn_dlg->OnInitDialog();
			}
		}
		break;
    case WM_COMMAND: 
        switch (LOWORD(wParam)) 
        { 
		case IDC_BUTTON_BROWSE_TRAJ:
			{
				if (moldyn_dlg) 
					moldyn_dlg->handler_ButtonBrowse(NULL, moldyn_dlg);
			}
			break;


		case IDOK: 
			{
				if (moldyn_dlg)
					moldyn_dlg->handler_ButtonOK(NULL, moldyn_dlg);
	            EndDialog(hDlg, wParam); 
			}
			break;
        case IDCANCEL: 
			{
				if (moldyn_dlg)
					moldyn_dlg->handler_ButtonCancel(NULL, moldyn_dlg);
				EndDialog(hDlg, wParam); 
			}
        } 
		break;
	case WM_DESTROY:
		{
			if (moldyn_dlg)
				delete moldyn_dlg;
		}
    } 
    return FALSE; 
} 


moldyn_dialog::moldyn_dialog(moldyn_param * p1) : glade_dialog("glade/moldyn_dialog.glade")
{
	param = p1;
	
//	dialog = glade_xml_get_widget(xml, "moldyn_dialog");
//	if (dialog == NULL) { cout << "moldyn_dialog : glade_xml_get_widget() failed!!!" << endl; return; }
	if (DialogBoxParam(hInst, 
         MAKEINTRESOURCE(IDD_DIALOG_MOL_DYN), 
         (HWND)NULL, (DLGPROC)DlgProcMolDyn, (LPARAM)this)==IDOK) 
    {
        // Complete the command; szItemName contains the 
        // name of the item to delete. 
    }

    else 
    {
        // Cancel the command. 
		 cout << "trajview_dialog : glade_xml_get_widget() failed!!!" << endl; 
		 return; 
    } 
	
	// initialize the widgets...
	/*
	entry_nsteps_h = glade_xml_get_widget(xml, "entry_nsteps_h");
	entry_nsteps_e = glade_xml_get_widget(xml, "entry_nsteps_e");
	entry_nsteps_s = glade_xml_get_widget(xml, "entry_nsteps_s");
	entry_temperature = glade_xml_get_widget(xml, "entry_temperature");
	entry_timestep = glade_xml_get_widget(xml, "entry_timestep");
	checkbutton_constant_e = glade_xml_get_widget(xml, "checkbutton_constant_e");
	checkbutton_langevin = glade_xml_get_widget(xml, "checkbutton_langevin");
	
	entry_trajfile = glade_xml_get_widget(xml, "entry_trajfile");
	button_browse = glade_xml_get_widget(xml, "button_browse");
	file_selector = NULL;
	
	
	// connect the handlers...
	
	glade_xml_signal_connect_data(xml, "on_dialog_destroy", (GtkSignalFunc) handler_Destroy, (gpointer) this);

	glade_xml_signal_connect_data(xml, "on_button_ok_clicked", (GtkSignalFunc) handler_ButtonOK, (gpointer) this);
	glade_xml_signal_connect_data(xml, "on_button_cancel_clicked", (GtkSignalFunc) handler_ButtonCancel, (gpointer) this);
	
	glade_xml_signal_connect_data(xml, "on_button_browse_clicked", (GtkSignalFunc) handler_ButtonBrowse, (gpointer) this);
	
 	gtk_dialog_run(GTK_DIALOG(dialog));	// MODAL
	gtk_widget_destroy(dialog);		// MODAL*/
}

moldyn_dialog::~moldyn_dialog(void)
{
}

void moldyn_dialog::OnInitDialog()
{
	char buffer[64];
	
	ostrstream str_nsteps_h(buffer, sizeof(buffer));
	str_nsteps_h << param->nsteps_h << ends;
	// gtk_entry_set_text(GTK_ENTRY(entry_nsteps_h), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_NSTEPS_H, str_nsteps_h.str());

	ostrstream str_nsteps_e(buffer, sizeof(buffer));
	str_nsteps_e << param->nsteps_e << ends;
	// gtk_entry_set_text(GTK_ENTRY(entry_nsteps_e), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_NSTEPS_E, str_nsteps_e.str());

	ostrstream str_nsteps_s(buffer, sizeof(buffer));
	str_nsteps_s << param->nsteps_s << ends;
	// gtk_entry_set_text(GTK_ENTRY(entry_nsteps_s), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_NSTEPS_S, str_nsteps_s.str());
	
	ostrstream str_grad(buffer, sizeof(buffer));
	str_grad << param->temperature << ends;
	// gtk_entry_set_text(GTK_ENTRY(entry_temperature), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_TEMPERATURE, str_grad.str());
	
	ostrstream str_delta_e(buffer, sizeof(buffer));
	str_delta_e << param->timestep << ends;
	// gtk_entry_set_text(GTK_ENTRY(entry_timestep), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_TIMESTEP, str_delta_e.str());

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_constant_e), (param->constant_e ? TRUE : FALSE));
	CheckDlgButton( hDlg, IDC_CHECK_CONSTANT_E,
		param->constant_e ? BST_CHECKED : BST_UNCHECKED );
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_langevin), (param->langevin ? TRUE : FALSE));
	CheckDlgButton( hDlg, IDC_CHECK_LANGEVIN,
		param->langevin ? BST_CHECKED : BST_UNCHECKED );
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	
	// gtk_entry_set_text(GTK_ENTRY(entry_trajfile), param->filename);
	SetDlgItemText(hDlg,IDC_EDIT_TRAJFILE, param->filename);
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	SendDlgItemMessage(this->hDlg,IDC_MOL_DYN_ATOM_LIST, LB_RESETCONTENT,0,0);
	SendDlgItemMessage(this->hDlg,IDC_MOL_DYN_ATOM_LIST,
		(UINT) LB_SETCOLUMNWIDTH,      // message ID  
		(WPARAM) 20,      // = (WPARAM) () wParam;   
		(LPARAM) 0      // = (LPARAM) () lParam; 
		);  

}

void moldyn_dialog::handler_Destroy(HWND, void * data)			// not really needed...
{
	moldyn_dialog * ref = (moldyn_dialog *) data;
	//cout << "handler_Destroy() : ref = " << ref << endl;
}

void moldyn_dialog::handler_ButtonOK(HWND, void * data)
{
	moldyn_dialog * ref = (moldyn_dialog *) data;
	//cout << "handler_ButtonOK() : ref = " << ref << endl;
	
	// read the user's settings from widgets...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	char buffer[1025];
	
	//buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_nsteps_h));
	GetDlgItemText(ref->hDlg,IDC_EDIT_NSTEPS_H, buffer, 63);
	istrstream istr1a(buffer, strlen(buffer) + 1);
	istr1a >> ref->param->nsteps_h;
	
	//buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_nsteps_e));
	GetDlgItemText(ref->hDlg,IDC_EDIT_NSTEPS_E, buffer, 63);
	istrstream istr1b(buffer, strlen(buffer) + 1);
	istr1b >> ref->param->nsteps_e;
	
	//buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_nsteps_s));
	GetDlgItemText(ref->hDlg,IDC_EDIT_NSTEPS_S, buffer, 63);
	istrstream istr1c(buffer, strlen(buffer) + 1);
	istr1c >> ref->param->nsteps_s;
	
	//buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_temperature));
	GetDlgItemText(ref->hDlg,IDC_EDIT_TEMPERATURE, buffer, 63);
	istrstream istr2(buffer, strlen(buffer) + 1);
	istr2 >> ref->param->temperature;
	
	//buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_timestep));
	GetDlgItemText(ref->hDlg,IDC_EDIT_TIMESTEP, buffer, 63);
	istrstream istr3(buffer, strlen(buffer) + 1);
	istr3 >> ref->param->timestep;

	
	//buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_trajfile));
	GetDlgItemText(ref->hDlg,IDC_EDIT_TRAJFILE, buffer, 1023);
	strcpy(ref->param->filename, buffer);


	//ref->param->constant_e = (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ref->checkbutton_constant_e)) == TRUE ? true : false);
	ref->param->constant_e = IsDlgButtonChecked( ref->hDlg, IDC_CHECK_CONSTANT_E) 
				== BST_CHECKED;
	//ref->param->langevin = (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ref->checkbutton_langevin)) == TRUE ? true : false);
	ref->param->langevin = IsDlgButtonChecked( ref->hDlg, IDC_CHECK_LANGEVIN) 
				== BST_CHECKED;
	
	ref->param->confirm = true;
}


void moldyn_dialog::handler_ButtonCancel(HWND, void * data)		// not really needed...
{
	moldyn_dialog * ref = (moldyn_dialog *) data;
	//cout << "handler_ButtonCancel() : ref = " << ref << endl;
	ref->param->confirm = false;
}

void moldyn_dialog::handler_ButtonBrowse(HWND, void * data)
{
	moldyn_dialog * ref = (moldyn_dialog *) data;
	//cout << "handler_ButtonBrowse() : ref = " << ref << endl;
	
	// handle the file selection...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	TCHAR filter[] =     TEXT("Ghemical Trajectory File (*.traj)\0*.traj\0")
						 TEXT("All Files (*.*)\0*.*\0");
	TCHAR filename[1024];

	sprintf(filename, "\0");
	DWORD nFilterIndex;
	if (SaveFileDlg(0, filename, filter, nFilterIndex) == S_OK)
	{
		SetDlgItemText(ref->hDlg,IDC_EDIT_TRAJFILE, filename);
	}	
/*	ref->file_selector = gtk_file_selection_new("Select a Trajectory File.");
	g_signal_connect(GTK_FILE_SELECTION(ref->file_selector)->ok_button, "clicked", G_CALLBACK(handler_FileChooser), data);
	
	g_signal_connect_swapped(GTK_FILE_SELECTION(ref->file_selector)->ok_button, "clicked", G_CALLBACK(gtk_widget_destroy), ref->file_selector);
	g_signal_connect_swapped(GTK_FILE_SELECTION(ref->file_selector)->cancel_button, "clicked", G_CALLBACK(gtk_widget_destroy), ref->file_selector);
	
	gtk_dialog_run(GTK_DIALOG(ref->file_selector)); ref->file_selector = NULL;*/
}


/*################################################################################################*/
BOOL CALLBACK DlgProcMolDyn_tst(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	static moldyn_tst_dialog * moldyn_tst_dlg;
    switch (message) 
    { 
	case WM_INITDIALOG:
		{
			moldyn_tst_dlg = reinterpret_cast<moldyn_tst_dialog *>(lParam);
			if (moldyn_tst_dlg)
			{
				moldyn_tst_dlg->hDlg = hDlg;
				moldyn_tst_dlg->OnInitDialog();
			}
		}
		break;
    case WM_COMMAND: 
        switch (LOWORD(wParam)) 
        { 
		case IDC_BUTTON_BROWSE_TRAJ:
			{
				if (moldyn_tst_dlg) 
					moldyn_tst_dlg->handler_ButtonBrowseTraj(NULL, moldyn_tst_dlg);
			}
			break;
		case IDC_BUTTON_BROWSE_INPUT_FRAME:
			{
				if (moldyn_tst_dlg) 
					moldyn_tst_dlg->handler_ButtonBrowseInputFrame(NULL, moldyn_tst_dlg);
			}
			break;
		case IDC_BUTTON_BROWSE_OUTPUT_FRAME:
			{
				if (moldyn_tst_dlg) 
					moldyn_tst_dlg->handler_ButtonBrowseOutputFrame(NULL, moldyn_tst_dlg);
			}
			break;
		case IDC_CHECK_LOAD_LAST_FRAME:
			{
				if (moldyn_tst_dlg) 
					moldyn_tst_dlg->handler_CheckLoadLastFrame(NULL, moldyn_tst_dlg);
			}
			break;
		case IDC_CHECK_EXTENDED_TRAJECTORY:
			{
				if (moldyn_tst_dlg) 
					moldyn_tst_dlg->handler_CheckExtendedTrajectory(NULL, moldyn_tst_dlg);
			}
			break;
		case IDOK: 
			{
				if (moldyn_tst_dlg)
					moldyn_tst_dlg->handler_ButtonOK(NULL, moldyn_tst_dlg);
	            EndDialog(hDlg, wParam); 
			}
			break;
        case IDCANCEL: 
			{
				if (moldyn_tst_dlg)
					moldyn_tst_dlg->handler_ButtonCancel(NULL, moldyn_tst_dlg);
				EndDialog(hDlg, wParam); 
			}
        } 
		break;
	case WM_DESTROY:
		{
			if (moldyn_tst_dlg)
				delete moldyn_tst_dlg;
		}
    } 
    return FALSE; 
} 


moldyn_tst_dialog::moldyn_tst_dialog(moldyn_tst_param * p1) : glade_dialog("glade/moldyn_dialog.glade")
{
	param = p1;
	
//	dialog = glade_xml_get_widget(xml, "moldyn_dialog");
//	if (dialog == NULL) { cout << "moldyn_dialog : glade_xml_get_widget() failed!!!" << endl; return; }
	if (DialogBoxParam(hInst, 
         MAKEINTRESOURCE(IDD_DIALOG_MOL_DYN_TST), 
         (HWND)NULL, (DLGPROC)DlgProcMolDyn_tst, (LPARAM)this)==IDOK) 
    {
        // Complete the command; szItemName contains the 
        // name of the item to delete. 
    }

    else 
    {
        // Cancel the command. 
		 cout << "trajview_dialog : glade_xml_get_widget() failed!!!" << endl; 
		 return; 
    } 
	
	// initialize the widgets...
	/*
	entry_nsteps_h = glade_xml_get_widget(xml, "entry_nsteps_h");
	entry_nsteps_e = glade_xml_get_widget(xml, "entry_nsteps_e");
	entry_nsteps_s = glade_xml_get_widget(xml, "entry_nsteps_s");
	entry_temperature = glade_xml_get_widget(xml, "entry_temperature");
	entry_timestep = glade_xml_get_widget(xml, "entry_timestep");
	checkbutton_constant_e = glade_xml_get_widget(xml, "checkbutton_constant_e");
	checkbutton_langevin = glade_xml_get_widget(xml, "checkbutton_langevin");
	
	entry_trajfile = glade_xml_get_widget(xml, "entry_trajfile");
	button_browse = glade_xml_get_widget(xml, "button_browse");
	file_selector = NULL;
	
	
	// connect the handlers...
	
	glade_xml_signal_connect_data(xml, "on_dialog_destroy", (GtkSignalFunc) handler_Destroy, (gpointer) this);

	glade_xml_signal_connect_data(xml, "on_button_ok_clicked", (GtkSignalFunc) handler_ButtonOK, (gpointer) this);
	glade_xml_signal_connect_data(xml, "on_button_cancel_clicked", (GtkSignalFunc) handler_ButtonCancel, (gpointer) this);
	
	glade_xml_signal_connect_data(xml, "on_button_browse_clicked", (GtkSignalFunc) handler_ButtonBrowse, (gpointer) this);
	
 	gtk_dialog_run(GTK_DIALOG(dialog));	// MODAL
	gtk_widget_destroy(dialog);		// MODAL*/
}

moldyn_tst_dialog::~moldyn_tst_dialog(void)
{
}

void moldyn_tst_dialog::OnInitDialog()
{
	char buffer[64];
	
	ostrstream str_nsteps_h(buffer, sizeof(buffer));
	str_nsteps_h << param->nsteps_h << ends;
	// gtk_entry_set_text(GTK_ENTRY(entry_nsteps_h), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_NSTEPS_H, str_nsteps_h.str());

	ostrstream str_nsteps_e(buffer, sizeof(buffer));
	str_nsteps_e << param->nsteps_e << ends;
	// gtk_entry_set_text(GTK_ENTRY(entry_nsteps_e), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_NSTEPS_E, str_nsteps_e.str());

	ostrstream str_nsteps_s(buffer, sizeof(buffer));
	str_nsteps_s << param->nsteps_s << ends;
	// gtk_entry_set_text(GTK_ENTRY(entry_nsteps_s), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_NSTEPS_S, str_nsteps_s.str());
	
	ostrstream str_grad(buffer, sizeof(buffer));
	str_grad << param->temperature << ends;
	// gtk_entry_set_text(GTK_ENTRY(entry_temperature), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_TEMPERATURE, str_grad.str());
	
	ostrstream str_grada(buffer, sizeof(buffer));
	str_grada << param->maxwell_init_temperature << ends;
	// gtk_entry_set_text(GTK_ENTRY(entry_temperature), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_MAXWELL_INIT_TEMPERATURE, str_grad.str());
	
	ostrstream str_delta_e(buffer, sizeof(buffer));
	str_delta_e << param->timestep << ends;
	// gtk_entry_set_text(GTK_ENTRY(entry_timestep), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_TIMESTEP, str_delta_e.str());

	ostrstream str_gravi_gx(buffer, sizeof(buffer));
	str_gravi_gx << param->g[0] << ends;
	// gtk_entry_set_text(GTK_ENTRY(entry_timestep), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_GRAVI_GX, str_gravi_gx.str());

	ostrstream str_gravi_gy(buffer, sizeof(buffer));
	str_gravi_gy << param->g[1] << ends;
	// gtk_entry_set_text(GTK_ENTRY(entry_timestep), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_GRAVI_GY, str_gravi_gy.str());

	ostrstream str_gravi_gz(buffer, sizeof(buffer));
	str_gravi_gz << param->g[2] << ends;
	// gtk_entry_set_text(GTK_ENTRY(entry_timestep), buffer);
	SetDlgItemText(hDlg,IDC_EDIT_GRAVI_GZ, str_gravi_gz.str());

	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_constant_e), (param->constant_e ? TRUE : FALSE));
	CheckDlgButton( hDlg, IDC_CHECK_CONSTANT_E,
		param->constant_e ? BST_CHECKED : BST_UNCHECKED );
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	// gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_langevin), (param->langevin ? TRUE : FALSE));
	CheckDlgButton( hDlg, IDC_CHECK_LANGEVIN,
		param->langevin ? BST_CHECKED : BST_UNCHECKED );
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	
	CheckDlgButton( hDlg, IDC_CHECK_CALC_MIN_MAX_COORDINATES_2,
		param->recalc_box ? BST_CHECKED : BST_UNCHECKED );
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	CheckDlgButton( hDlg, IDC_CHECK_BOX_OPTIMIZATION_2,
		param->box_optimization ? BST_CHECKED : BST_UNCHECKED );
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	CheckDlgButton( hDlg, IDC_CHECK_MAXWELL_DISTRIBUTION_INIT,
		param->maxwell_distribution_init ? BST_CHECKED : BST_UNCHECKED );
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	CheckDlgButton( hDlg, IDC_CHECK_LOAD_LAST_FRAME,
		param->load_last_frame ? BST_CHECKED : BST_UNCHECKED );
	CheckDlgButton( hDlg, IDC_CHECK_EXTENDED_TRAJECTORY,
		param->extended_trajectory ? BST_CHECKED : BST_UNCHECKED );
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	// gtk_entry_set_text(GTK_ENTRY(entry_trajfile), param->filename);
	SetDlgItemText(hDlg,IDC_EDIT_TRAJFILE, param->filename_traj);
	SetDlgItemText(hDlg,IDC_EDIT_INPUT_FRAME_FILE, param->filename_input_frame);
	SetDlgItemText(hDlg,IDC_EDIT_OUTPUT_FRAME_FILE, param->filename_output_frame);
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	SendDlgItemMessage(this->hDlg,IDC_MOL_DYN_ATOM_LIST, LB_RESETCONTENT,0,0);
	SendDlgItemMessage(this->hDlg,IDC_MOL_DYN_ATOM_LIST,
		(UINT) LB_SETCOLUMNWIDTH,      // message ID  
		(WPARAM) 20,      // = (WPARAM) () wParam;   
		(LPARAM) 0      // = (LPARAM) () lParam; 
		);  

}

void moldyn_tst_dialog::handler_Destroy(HWND, void * data)			// not really needed...
{
	moldyn_tst_dialog * ref = (moldyn_tst_dialog *) data;
	//cout << "handler_Destroy() : ref = " << ref << endl;
}

void moldyn_tst_dialog::handler_ButtonOK(HWND, void * data)
{
	moldyn_tst_dialog * ref = (moldyn_tst_dialog *) data;
	//cout << "handler_ButtonOK() : ref = " << ref << endl;
	
	// read the user's settings from widgets...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	char buffer[1025];
	
	//buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_nsteps_h));
	GetDlgItemText(ref->hDlg,IDC_EDIT_NSTEPS_H, buffer, 63);
	istrstream istr1a(buffer, strlen(buffer) + 1);
	istr1a >> ref->param->nsteps_h;
	
	//buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_nsteps_e));
	GetDlgItemText(ref->hDlg,IDC_EDIT_NSTEPS_E, buffer, 63);
	istrstream istr1b(buffer, strlen(buffer) + 1);
	istr1b >> ref->param->nsteps_e;
	
	//buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_nsteps_s));
	GetDlgItemText(ref->hDlg,IDC_EDIT_NSTEPS_S, buffer, 63);
	istrstream istr1c(buffer, strlen(buffer) + 1);
	istr1c >> ref->param->nsteps_s;
	
	//buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_temperature));
	GetDlgItemText(ref->hDlg,IDC_EDIT_TEMPERATURE, buffer, 63);
	istrstream istr2(buffer, strlen(buffer) + 1);
	istr2 >> ref->param->temperature;
	
	//buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_temperature));
	GetDlgItemText(ref->hDlg,IDC_EDIT_MAXWELL_INIT_TEMPERATURE, buffer, 63);
	istrstream istr2a(buffer, strlen(buffer) + 1);
	istr2a >> ref->param->maxwell_init_temperature;
	
	//buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_timestep));
	GetDlgItemText(ref->hDlg,IDC_EDIT_TIMESTEP, buffer, 63);
	istrstream istr3(buffer, strlen(buffer) + 1);
	istr3 >> ref->param->timestep;

	GetDlgItemText(ref->hDlg,IDC_EDIT_GRAVI_GX, buffer, 63);
	istrstream istr4x(buffer, strlen(buffer) + 1);
	istr4x >> ref->param->g[0];

	GetDlgItemText(ref->hDlg,IDC_EDIT_GRAVI_GY, buffer, 63);
	istrstream istr4y(buffer, strlen(buffer) + 1);
	istr4y >> ref->param->g[1];

	GetDlgItemText(ref->hDlg,IDC_EDIT_GRAVI_GZ, buffer, 63);
	istrstream istr4z(buffer, strlen(buffer) + 1);
	istr4z >> ref->param->g[2];

	
	//buffer = gtk_entry_get_text(GTK_ENTRY(ref->entry_trajfile));
	GetDlgItemText(ref->hDlg,IDC_EDIT_TRAJFILE, buffer, 1023);
	strcpy(ref->param->filename_traj, buffer);

	GetDlgItemText(ref->hDlg,IDC_EDIT_INPUT_FRAME_FILE, buffer, 1023);
	strcpy(ref->param->filename_input_frame, buffer);

	GetDlgItemText(ref->hDlg,IDC_EDIT_OUTPUT_FRAME_FILE, buffer, 1023);
	strcpy(ref->param->filename_output_frame, buffer);

	//ref->param->constant_e = (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ref->checkbutton_constant_e)) == TRUE ? true : false);
	ref->param->constant_e = IsDlgButtonChecked( ref->hDlg, IDC_CHECK_CONSTANT_E) 
				== BST_CHECKED;
	//ref->param->langevin = (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ref->checkbutton_langevin)) == TRUE ? true : false);
	ref->param->langevin = IsDlgButtonChecked( ref->hDlg, IDC_CHECK_LANGEVIN) 
				== BST_CHECKED;
	ref->param->recalc_box = IsDlgButtonChecked( ref->hDlg, IDC_CHECK_CALC_MIN_MAX_COORDINATES_2) 
				== BST_CHECKED;
	ref->param->box_optimization = IsDlgButtonChecked( ref->hDlg, IDC_CHECK_BOX_OPTIMIZATION_2) 
				== BST_CHECKED;
	ref->param->maxwell_distribution_init = IsDlgButtonChecked( ref->hDlg, IDC_CHECK_MAXWELL_DISTRIBUTION_INIT) 
				== BST_CHECKED;

	ref->param->load_last_frame = IsDlgButtonChecked( ref->hDlg, IDC_CHECK_LOAD_LAST_FRAME) 
				== BST_CHECKED;
	ref->param->extended_trajectory = IsDlgButtonChecked( ref->hDlg, IDC_CHECK_EXTENDED_TRAJECTORY) 
				== BST_CHECKED;

	
	ref->param->confirm = true;
}

void moldyn_tst_dialog::handler_CheckLoadLastFrame(HWND, void * data)
{
	moldyn_tst_dialog * ref = (moldyn_tst_dialog *) data;
	//cout << "handler_ButtonOK() : ref = " << ref << endl;
	ref->param->load_last_frame = IsDlgButtonChecked( ref->hDlg, IDC_CHECK_LOAD_LAST_FRAME) 
				== BST_CHECKED;	
}

void moldyn_tst_dialog::handler_CheckExtendedTrajectory(HWND, void * data)
{
	moldyn_tst_dialog * ref = (moldyn_tst_dialog *) data;
	ref->param->extended_trajectory = IsDlgButtonChecked( ref->hDlg, IDC_CHECK_EXTENDED_TRAJECTORY) 
				== BST_CHECKED;	
}

void moldyn_tst_dialog::handler_ButtonCancel(HWND, void * data)		// not really needed...
{
	moldyn_tst_dialog * ref = (moldyn_tst_dialog *) data;
	//cout << "handler_ButtonCancel() : ref = " << ref << endl;
	ref->param->confirm = false;
}

void moldyn_tst_dialog::handler_ButtonBrowseTraj(HWND, void * data)
{
	moldyn_tst_dialog * ref = (moldyn_tst_dialog *) data;
	//cout << "handler_ButtonBrowse() : ref = " << ref << endl;
	
	// handle the file selection...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	TCHAR filter[] =     TEXT("Ghemical Trajectory File (*.traj)\0*.traj\0")
						 TEXT("All Files (*.*)\0*.*\0");
	TCHAR filename[1024];

	sprintf(filename, "\0");
	DWORD nFilterIndex;
	if (SaveFileDlg(0, filename, filter, nFilterIndex) == S_OK)
	{
		SetDlgItemText(ref->hDlg,IDC_EDIT_TRAJFILE, filename);
	}	
/*	ref->file_selector = gtk_file_selection_new("Select a Trajectory File.");
	g_signal_connect(GTK_FILE_SELECTION(ref->file_selector)->ok_button, "clicked", G_CALLBACK(handler_FileChooser), data);
	
	g_signal_connect_swapped(GTK_FILE_SELECTION(ref->file_selector)->ok_button, "clicked", G_CALLBACK(gtk_widget_destroy), ref->file_selector);
	g_signal_connect_swapped(GTK_FILE_SELECTION(ref->file_selector)->cancel_button, "clicked", G_CALLBACK(gtk_widget_destroy), ref->file_selector);
	
	gtk_dialog_run(GTK_DIALOG(ref->file_selector)); ref->file_selector = NULL;*/
}

void moldyn_tst_dialog::handler_ButtonBrowseInputFrame(HWND, void * data)
{
	moldyn_tst_dialog * ref = (moldyn_tst_dialog *) data;
	//cout << "handler_ButtonBrowse() : ref = " << ref << endl;
	
	// handle the file selection...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	TCHAR filter[] =     TEXT("Ghemical Last Frame File (*.frame)\0*.frame\0")
						 TEXT("All Files (*.*)\0*.*\0");
	TCHAR filename[1024];

	sprintf(filename, "\0");

	DWORD nFilterIndex;
	if (OpenFileDlg(0, filter, filename, nFilterIndex) == S_OK)
	{
		SetDlgItemText(ref->hDlg,IDC_EDIT_INPUT_FRAME_FILE, filename);
	}
	
	
/*	ref->file_selector = gtk_file_selection_new("Select a Trajectory File.");
	g_signal_connect(GTK_FILE_SELECTION(ref->file_selector)->ok_button, "clicked", G_CALLBACK(handler_FileChooser), data);
	
	g_signal_connect_swapped(GTK_FILE_SELECTION(ref->file_selector)->ok_button, "clicked", G_CALLBACK(gtk_widget_destroy), ref->file_selector);
	g_signal_connect_swapped(GTK_FILE_SELECTION(ref->file_selector)->cancel_button, "clicked", G_CALLBACK(gtk_widget_destroy), ref->file_selector);
	
	gtk_dialog_run(GTK_DIALOG(ref->file_selector)); ref->file_selector = NULL;*/
}


void moldyn_tst_dialog::handler_ButtonBrowseOutputFrame(HWND, void * data)
{
	moldyn_tst_dialog * ref = (moldyn_tst_dialog *) data;
	//cout << "handler_ButtonBrowse() : ref = " << ref << endl;
	
	// handle the file selection...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	TCHAR filter[] =     TEXT("Ghemical Last Frame File (*.frame)\0*.frame\0")
						 TEXT("All Files (*.*)\0*.*\0");
	TCHAR filename[1024];

	sprintf(filename, "\0");

	DWORD nFilterIndex;
	if (SaveFileDlg(0, filename, filter, nFilterIndex) == S_OK)
	{
		SetDlgItemText(ref->hDlg,IDC_EDIT_OUTPUT_FRAME_FILE, filename);
	}	

	
/*	ref->file_selector = gtk_file_selection_new("Select a Trajectory File.");
	g_signal_connect(GTK_FILE_SELECTION(ref->file_selector)->ok_button, "clicked", G_CALLBACK(handler_FileChooser), data);
	
	g_signal_connect_swapped(GTK_FILE_SELECTION(ref->file_selector)->ok_button, "clicked", G_CALLBACK(gtk_widget_destroy), ref->file_selector);
	g_signal_connect_swapped(GTK_FILE_SELECTION(ref->file_selector)->cancel_button, "clicked", G_CALLBACK(gtk_widget_destroy), ref->file_selector);
	
	gtk_dialog_run(GTK_DIALOG(ref->file_selector)); ref->file_selector = NULL;*/
}

void moldyn_tst_dialog::handler_FileChooser(HWND, void * data)
{
	moldyn_tst_dialog * ref = (moldyn_tst_dialog *) data;
	//cout << "handler_FileChooser() : ref = " << ref << endl;
	
	// save the selected filename...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
/*	const gchar * selected_filename = gtk_file_selection_get_filename(GTK_FILE_SELECTION(ref->file_selector));
	gtk_entry_set_text(GTK_ENTRY(ref->entry_trajfile), selected_filename);*/
}

/*################################################################################################*/

// eof

