// GTK_DIALOG.CPP

// Copyright (C) 1999 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"
#include "../consol/resource.h"

#include "win32_dialog.h"

#include "../libghemical/src/atom.h"
#include "../libghemical/src/bond.h"

#include "win32_project.h"
#include "glade/trajview_dialog.h"

/*################################################################################################*/
extern HINSTANCE hInst;								// current instance

element_dialog * element_dialog::current_object = NULL;
HWND element_dialog::dialog = NULL;
HWND element_dialog::current_element_widget = NULL;

BOOL CALLBACK DlgProcElement(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	static element_dialog * elem_dlg;
    switch (message) 
    { 
	case WM_INITDIALOG:
		{
			elem_dlg = reinterpret_cast<element_dialog *>(lParam);
			if (elem_dlg) elem_dlg->dialog = hDlg;
			if (elem_dlg) elem_dlg->OnInitDialog();
		}
		break;
    case WM_COMMAND: 
        switch (LOWORD(wParam)) 
        { 
            case IDOK: 
            case IDCANCEL: 
				{
					EndDialog(hDlg, wParam); 
					return TRUE; 
				}
				break;
			default:
				{
					if (elem_dlg) elem_dlg->SignalHandler(hDlg, LOWORD(wParam));
				}
		} 
		break;
	case WM_DESTROY:
		{
			if (elem_dlg)
			{
				delete elem_dlg;
				elem_dlg = NULL;
			}
		}
    } 
    return FALSE; 
} 
element_dialog::element_dialog(void)
{
	current_object = this;

	if (DialogBoxParam(hInst, 
         MAKEINTRESOURCE(IDD_DIALOG_ELEMENT), 
         (HWND)NULL, (DLGPROC)DlgProcElement, (LPARAM)this)==IDOK) 
    {
        // Complete the command; szItemName contains the 
        // name of the item to delete. 
    }

    else 
    {
        // Cancel the command. 
    } 
}

void element_dialog::OnInitDialog()
{
//	dialog = gtk_dialog_new();
	
//	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);		// request a modal window.
	
//	gtk_window_set_title(GTK_WINDOW(dialog), "Set Current Element");
//	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	
//	HWND table = gtk_table_new(21, 38, TRUE);
//	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table);
	
	int attachdata[ELEMENT_SYMBOLS][2] =
	{
		{ 1, 1 }, { 35, 1 }, { 1, 3 }, { 3, 3 }, { 25, 3 },		// B
		{ 27, 3 }, { 29, 3 }, { 31, 3 }, { 33, 3 }, { 35, 3 },		// Ne
		{ 1, 5 }, { 3, 5 }, { 25, 5 }, { 27, 5 }, { 29, 5 },		// P
		{ 31, 5 }, { 33, 5 }, { 35, 5 }, { 1, 7 }, { 3, 7 },		// Ca
		{ 5, 7 }, { 7, 7 }, { 9, 7 }, { 11, 7 }, { 13, 7 },		// Mn
		{ 15, 7 }, { 17, 7 }, { 19, 7 }, { 21, 7 }, { 23, 7 },		// Zn
		{ 25, 7 }, { 27, 7 }, { 29, 7 }, { 31, 7 }, { 33, 7 },		// Br
		{ 35, 7 }, { 1, 9 }, { 3, 9 }, { 5, 9 }, { 7, 9 },		// Zr
		{ 9, 9 }, { 11, 9 }, { 13, 9 }, { 15, 9 }, { 17, 9 },		// Rh
		{ 19, 9 }, { 21, 9 }, { 23, 9 }, { 25, 9 }, { 27, 9 },		// Sn
		{ 29, 9 }, { 31, 9 }, { 33, 9 }, { 35, 9 }, { 1, 11 },		// Cs
		{ 3, 11 }, { 5, 11 }, { 9, 16 }, { 11, 16 }, { 13, 16 },	// Nd
		{ 15, 16 }, { 17, 16 }, { 19, 16 }, { 21, 16 }, { 23, 16 },	// Tb
		{ 25, 16 }, { 27, 16 }, { 29, 16 }, { 31, 16 }, { 33, 16 },	// Yb
		{ 35, 16 }, { 7, 11 }, { 9, 11 }, { 11, 11 }, { 13, 11 },	// Re
		{ 15, 11 }, { 17, 11 }, { 19, 11 }, { 21, 11 }, { 23, 11 },	// Hg
		{ 25, 11 }, { 27, 11 }, { 29, 11 }, { 31, 11 }, { 33, 11 },	// At
		{ 35, 11 }, { 1, 13 }, { 3, 13 }, { 5, 13 }, { 9, 18 },		// Th
		{ 11, 18 }, { 13, 18 }, { 15, 18 }, { 17, 18 }, { 19, 18 },	// Am
		{ 21, 18 }, { 23, 18 }, { 25, 18 }, { 27, 18 }, { 29, 18 },	// Fm
		{ 31, 18 }, { 33, 18 }, { 35, 18 }, { 7, 13 }, { 9, 13 },	// Db
		{ 11, 13 }, { 13, 13 }, { 15, 13 }, { 17, 13 }, { 19, 13 }
	};
	
//	GtkSignalFunc sf = (GtkSignalFunc) element_dialog::SignalHandler;
	HWND button;
	
	for (int n1 = 0; n1 < ELEMENT_SYMBOLS; n1++)
	{
		element tmp_element(n1 + 1);
//		button = gtk_toggle_button_new_with_label(tmp_element.GetSymbol());
		
		
//		gtk_signal_connect(GTK_OBJECT(button), "clicked", sf, (void *) (n1 + 1));
//		gtk_widget_show(button);

		int x = attachdata[n1][0]*15; int y = attachdata[n1][1]*10;
//		gtk_table_attach_defaults(GTK_TABLE(table), button, x, x + 2, y, y + 2);
		button = CreateWindow("BUTTON",  tmp_element.GetSymbol(), 
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			x, y, 24, 16, dialog,
			(HMENU) (n1 + 1 + 2), hInst, NULL);


		ShowWindow(button, 1);

		if (n1 + 1 == element::current_element.GetAtomicNumber())
		{
//			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), true);
			current_element_widget = button;	// do we need this???
			SetWindowLongPtr(current_element_widget, GWL_STYLE,  
				BS_PUSHLIKE | GetWindowLongPtr(current_element_widget, GWL_STYLE));

		}
	}
	
//	gtk_widget_show(table);
//	gtk_widget_show(dialog);
}

element_dialog::~element_dialog(void)
{
}

void element_dialog::SignalHandler(HWND, int signal)
{
	element::current_element = element(signal-2);
	cout << "current element is now " << element::current_element.GetSymbol() << endl;	// which prj???
	
//	gtk_widget_destroy(dialog);
	//delete current_object;		// the object was created with new!!!
}

/*################################################################################################*/

bondtype_dialog * bondtype_dialog::current_object = NULL;
HWND bondtype_dialog::dialog = NULL;
HWND bondtype_dialog::current_bond_widget = NULL;

BOOL CALLBACK DlgProcBondType(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	static bondtype_dialog * bndtp_dlg;
    switch (message) 
    { 
	case WM_INITDIALOG:
		{
			bndtp_dlg = reinterpret_cast<bondtype_dialog *>(lParam);
			if (bndtp_dlg) bndtp_dlg->dialog = hDlg;
			if (bndtp_dlg) bndtp_dlg->OnInitDialog();
		}
		break;
    case WM_COMMAND: 
        switch (LOWORD(wParam)) 
        { 
            case IDOK: 
            case IDCANCEL: 
				{
					EndDialog(hDlg, wParam); 
					return TRUE; 
				}
				break;
			default:
				{
					if (bndtp_dlg) bndtp_dlg->SignalHandler(hDlg, LOWORD(wParam));
				}
		} 
		break;
	case WM_DESTROY:
		{
			if (bndtp_dlg)
			{
				delete bndtp_dlg;
				bndtp_dlg = NULL;
			}
		}
    } 
    return FALSE; 
} 


bondtype_dialog::bondtype_dialog(void)
{
	current_object = this;
	if (DialogBoxParam(hInst, 
         MAKEINTRESOURCE(IDD_DIALOG_BOND), 
         (HWND)NULL, (DLGPROC)DlgProcBondType, (LPARAM)this)==IDOK) 
    {
        // Complete the command; szItemName contains the 
        // name of the item to delete. 
    }

    else 
    {
        // Cancel the command. 
    } 
}

void bondtype_dialog::OnInitDialog()
{
	//	dialog = gtk_dialog_new();
	
//	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);		// request a modal window.
	
//	gtk_window_set_title(GTK_WINDOW(dialog), "Set Current Bondtype");
//	gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	
//	HWND table = gtk_table_new((2 + 4 + 1), 6, TRUE);
//	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), table);
	
	char bt_symbols[4] = { 'S', 'D', 'T', 'C' };
	
//	GtkSignalFunc sf = (GtkSignalFunc) bondtype_dialog::SignalHandler;
	HWND button;
	
	int x = 1; int y = 0;
	for (int n1 = 0;n1 < 4;n1++)
	{
		bondtype tmp_bondtype(bt_symbols[n1]);
//		button = gtk_toggle_button_new_with_label(tmp_bondtype.GetString());
		


		i32s long_bt_symbol = bt_symbols[n1];	// convert 8bit -> 32bit
//		gtk_signal_connect(GTK_OBJECT(button), "clicked", sf, (void *) long_bt_symbol);
		
		y += 1;
//		gtk_table_attach_defaults(GTK_TABLE(table), button, x, x + 4, y, y + 1);
//		gtk_widget_show(button);
		
		if (n1 == 2)	// add a separator ; 20050415
		{
//			button = gtk_hseparator_new();
			
			y += 1;
//			gtk_table_attach_defaults(GTK_TABLE(table), button, x, x + 4, y, y + 1);
//			gtk_widget_show(button);
		}
		button = CreateWindow("BUTTON",  tmp_bondtype.GetString(), 
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			x*30, y*30, 100, 25, dialog,
			(HMENU) (long_bt_symbol), hInst, NULL);

		if (bt_symbols[n1] == bondtype::current_bondtype.GetSymbol1())
		{
//			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), true);
			current_bond_widget = button;	// do we need this??
		}
	}
	
//	gtk_widget_show(table);
//	gtk_widget_show(dialog);
}

bondtype_dialog::~bondtype_dialog(void)
{
}

void bondtype_dialog::SignalHandler(HWND, int signal)
{
	bondtype::current_bondtype = bondtype((char) signal);
	cout << "current bondtype is now " << bondtype::current_bondtype.GetString() << endl;	// which prj???
	
//	gtk_widget_destroy(dialog);
//	delete current_object;		// the object was created with new!!!
}

/*################################################################################################*/

const char * make_tmp_filename(win_project * prj)
{
	static char tmp_filename[256];
	prj->GetFullProjectFileName(tmp_filename, 256);
	return tmp_filename;
}

HWND file_dialog::dialog;

file_dialog::file_dialog(const char * title, const char * filter, const char * def_fn)
{
//	dialog = gtk_file_selection_new(title);
//	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);		// request a modal window.
	
//	if (filter != NULL) gtk_file_selection_complete(GTK_FILE_SELECTION(dialog), filter);
//	if (def_fn != NULL) gtk_file_selection_set_filename(GTK_FILE_SELECTION(dialog), def_fn);
	
//	gtk_signal_connect(GTK_OBJECT(dialog),
//		"destroy", GTK_SIGNAL_FUNC(DestroyHandler), this);
		
//	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(dialog)->ok_button),
//		"clicked", GTK_SIGNAL_FUNC(OkButtonHandler), this);
		
//	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(dialog)->cancel_button),
//		"clicked", GTK_SIGNAL_FUNC(CancelButtonHandler), this);

//	gtk_widget_show(dialog);
}

file_dialog::~file_dialog(void)
{
}

void file_dialog::DestroyHandler(HWND, void * data)
{
	file_dialog * ref = (file_dialog *) data;
	
	delete ref;	// the object was created using new!!!
}

void file_dialog::OkButtonHandler(HWND, void * data)
{
	file_dialog * ref = (file_dialog *) data;
	
	// do not close the dialog if we had for example wrong file type...
	
//	if (ref->OkEvent(gtk_file_selection_get_filename(GTK_FILE_SELECTION(dialog))))
//	{
//		gtk_widget_destroy(dialog);
//	}
}

void file_dialog::CancelButtonHandler(HWND, void * data)
{
	file_dialog * ref = (file_dialog *) data;
	ref->CancelEvent();
	
//	gtk_widget_destroy(dialog);
}

/*################################################################################################*/

const char file_open_dialog::title[] = "Open Project File";
win_project * file_open_dialog::prj = NULL;

file_open_dialog::file_open_dialog(win_project * p1) : file_dialog(title, NULL, NULL)
{
	prj = p1;
}

file_open_dialog::~file_open_dialog(void)
{
}

bool file_open_dialog::OkEvent(const char * filename)
{
	if (prj == NULL)
	{
		// set a the new project object ; this is only for a stand-alone app ; FIXME bonobo.
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		
//		gtk_container_remove(GTK_CONTAINER(gtk_app::main_vbox), gtk_app::prj->GetWidget());
		
		delete win_app::prj;
		win_app::prj = NULL;
		
		win_app::prj = new win_project(* win_class_factory::GetInstance());
//		HWND widget = prj->GetWidget();
		
//		gtk_container_add(GTK_CONTAINER(gtk_app::main_vbox), widget);
	}
	
	bool open = prj->IsEmpty();	// open, not insert
	
	ifstream ifile;
	ifile.open(filename, ios::in);
	ReadGPR(* prj, ifile, !open);
	ifile.close();
	
	if (open) 	// we're "switching" to this file
	{
		prj->ParseProjectFileNameAndPath(filename);
		
		char buffer[256];
		prj->GetProjectFileName(buffer, 256, true);
		
	// the file name change here is not yet properly working.
	// the project notebook widget should be stored (if not already) in gtk_project and
	// a call of UpdateAllWindowTitles() should change the labels.
		
		prj->UpdateAllWindowTitles();
	}
	else		// we append this file to the current project
	{
		if (prj->selected_object != NULL)		// if an object was selected, remove
		{						// that selection and leave the newly added
			prj->selected_object = NULL;		// atoms as selection; now the mouse tools
		}						// also affect the newly added atoms!
	}
	
	prj->UpdateAllGraphicsViews();
	return true;
}

void file_open_dialog::CancelEvent(void)
{
}

/*################################################################################################*/

const char file_save_dialog::title[] = "Save Project File";
win_project * file_save_dialog::prj = NULL;

file_save_dialog::file_save_dialog(win_project * p1) :
	file_dialog(title, FILENAME_FILTER, make_tmp_filename(p1))
{
	prj = p1;
}

file_save_dialog::~file_save_dialog(void)
{
}

bool file_save_dialog::OkEvent(const char * filename)
{
	if (prj == NULL)
	{
		prj->ErrorMessage("BUG: file_save_dialog::OkEvent() failed.");
		exit(EXIT_FAILURE);
	}
	
	// should check to see if file already exists...
	
	prj->ParseProjectFileNameAndPath(filename);
	
	char buffer[256];
	prj->GetProjectFileName(buffer, 256, true);
	
	//gnome_mdi_child_set_name(GNOME_MDI_CHILD(prj->child), buffer);
	prj->UpdateAllWindowTitles();
	
	char tmp_filename[256];
	prj->GetFullProjectFileName(tmp_filename, 256);
	
	ofstream ofile;
	ofile.open(tmp_filename, ios::out);
	WriteGPR_v100(* prj, ofile);
	ofile.close();

	return true;
}

void file_save_dialog::CancelEvent(void)
{
}

/*################################################################################################*/

const char file_save_graphics_dialog::title[] = "Save Graphics File - ps";
win_project * file_save_graphics_dialog::prj = NULL;

file_save_graphics_dialog::file_save_graphics_dialog(win_project * p1) :
	file_dialog(title, "ps", make_tmp_filename(p1))
{
	prj = p1;
}

file_save_graphics_dialog::~file_save_graphics_dialog(void)
{
}

bool file_save_graphics_dialog::OkEvent(const char * filename)
{
	if (prj == NULL)
	{
		prj->ErrorMessage("BUG: file_save_graphics_dialog::OkEvent() failed.");
		exit(EXIT_FAILURE);
	}
	
	// write graphics event
	
	return true;
}

void file_save_graphics_dialog::CancelEvent(void)
{
}

/*################################################################################################*/

const char trajfile_dialog::title[] = "Open Trajectory File - traj";
win_project * trajfile_dialog::prj = NULL;

trajfile_dialog::trajfile_dialog(win_project * p1) : file_dialog(title, NULL, NULL)
{
	prj = p1;
}

trajfile_dialog::~trajfile_dialog(void)
{
}

bool trajfile_dialog::OkEvent(const char * filename)
{
	cout << "trying to open \"" << filename << "\"." << endl;
	prj->OpenTrajectory(filename);
// check if there were problems with OpenTrajectory()?!?!?!
// check if there were problems with OpenTrajectory()?!?!?!
// check if there were problems with OpenTrajectory()?!?!?!
	
	static trajview_dialog * tvd = NULL;
	
	if (tvd != NULL) delete tvd;		// how to safely release the memory...
	tvd = new trajview_dialog(prj);		// ...right after the dialog is closed?
	
	// the dialog will call prj->CloseTrajectory() itself when closed!!!
	// the dialog will call prj->CloseTrajectory() itself when closed!!!
	// the dialog will call prj->CloseTrajectory() itself when closed!!!
	
//	gtk_widget_destroy(dialog);	// close the dialog -> must return false...
	return false;
}

void trajfile_dialog::CancelEvent(void)
{
}

/*################################################################################################*/

const char importpdb_dialog::title[] = "Import File - Brookhaven PDB/ENT";
win_project * importpdb_dialog::prj = NULL;

importpdb_dialog::importpdb_dialog(win_project * p1) : file_dialog(title, NULL, NULL)
{
	prj = p1;
}

importpdb_dialog::~importpdb_dialog(void)
{
}

bool importpdb_dialog::OkEvent(const char * filename)
{
	if (prj != NULL)
	{
		prj->WarningMessage("This alternative PDB reader is designed to remove any gaps that might\nexist in the structure data. Therefore the sequence might be altered!");
		
		prj->ParseProjectFileNameAndPath(filename);
		
		prj->importpdb_mdata = prj->readpdb_ReadMData(filename);
		prj->readpdb_ReadData(filename, prj->importpdb_mdata, -1);
		
		prj->UpdateAllWindowTitles();
		prj->UpdateAllGraphicsViews();
	}
	
	return true;
}

void importpdb_dialog::CancelEvent(void)
{
}

/*################################################################################################*/

win_project * command_dialog::prj;
graphics_view * command_dialog::gv;

HWND command_dialog::dialog;
HWND command_dialog::entry;

BOOL CALLBACK DlgProcCommand(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	static command_dialog * cmnd_dlg;
    switch (message) 
    { 
	case WM_INITDIALOG:
		{
			cmnd_dlg = reinterpret_cast<command_dialog *>(lParam);
			SetDlgItemText(hDlg, IDC_EDIT_COMMAND, cmnd_dlg->command);
		}
		break;
    case WM_COMMAND: 
        switch (LOWORD(wParam)) 
        { 
            case IDOK: 
				if (cmnd_dlg) cmnd_dlg->OkButtonHandler(GetDlgItem(hDlg, IDC_EDIT_COMMAND));

            case IDCANCEL: 
                EndDialog(hDlg, wParam); 
                return TRUE; 
        } 
		break;
	case WM_DESTROY:
		{
			if (cmnd_dlg) delete cmnd_dlg;
		}
    } 
    return FALSE; 
} 

command_dialog::command_dialog(win_project * p1, graphics_view * p2, const char * def_str)
{
	prj = p1; gv = p2;
	command[0] = '\0';
	if (def_str) strcpy(command, def_str);

	if (DialogBoxParam(hInst, 
         MAKEINTRESOURCE(IDD_COMMAND_DIALOG), 
         (HWND)NULL, (DLGPROC)DlgProcCommand, (LPARAM)this)==IDOK) 
    {
        // Complete the command; szItemName contains the 
        // name of the item to delete. 
    }

    else 
    {
        // Cancel the command. 
    } 

}

command_dialog::~command_dialog(void)
{
}
void command_dialog::OkButtonHandler(HWND hEditWnd)
{
	GetWindowText(hEditWnd, command, 511);
	if (strlen(command) > 0) prj->ProcessCommandString(gv, command);
}


/*################################################################################################*/

// eof
