// TRAJVIEW_DIALOG.CPP

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "../stdafx.h"

#include "trajview_dialog.h"

#include <iostream>
#include <strstream>
using namespace std;

/*################################################################################################*/
extern HINSTANCE hInst;								// current instance
BOOL CALLBACK DlgProcTrajView(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	static trajview_dialog * traj_dlg;
    switch (message) 
    { 
	case WM_INITDIALOG:
		{
			traj_dlg = reinterpret_cast<trajview_dialog *>(lParam);
		}
		break;
    case WM_COMMAND: 
        switch (LOWORD(wParam)) 
        { 
		case IDC_BUTTON_TRAJ_VIEW_BEGIN:
			{
				if (traj_dlg) traj_dlg->handler_ButtonBegin(NULL, traj_dlg);
			}
			break;
		case IDC_BUTTON_TRAJ_VIEW_PREV:
			{
				if (traj_dlg) traj_dlg->handler_ButtonPrev(NULL, traj_dlg);
			}
			break;
		case IDC_BUTTON_TRAJ_VIEW_PLAY:
			{
				DWORD dwChildID;
				HANDLE hThread;
				hThread = ::CreateThread(NULL, 0, 
					traj_dlg->handler_ButtonPlay, 
					reinterpret_cast<LPVOID>(traj_dlg), 
					0,
					&dwChildID );
			}
			break;
		case IDC_BUTTON_TRAJ_VIEW_NEXT:
			{
				if (traj_dlg) traj_dlg->handler_ButtonNext(NULL, traj_dlg);
			}
			break;
		case IDC_BUTTON_TRAJ_VIEW_END:
			{
				if (traj_dlg) traj_dlg->handler_ButtonEnd(NULL, traj_dlg);
			}
			break;
		case IDOK: 

        case IDCANCEL: 
            EndDialog(hDlg, wParam); 
            return TRUE; 
        } 
		break;
	case WM_DESTROY:
		{
			if (traj_dlg) delete traj_dlg;
		}
    } 
    return FALSE; 
} 


trajview_dialog::trajview_dialog(win_project * p1) : glade_dialog("glade/trajview_dialog.glade")
{
	prj = p1;
	
//	dialog = glade_xml_get_widget(xml, "trajview_dialog");
	if (DialogBoxParam(hInst, 
         MAKEINTRESOURCE(IDD_DIALOG_TRAJ_VIEW), 
         (HWND)NULL, (DLGPROC)DlgProcTrajView, (LPARAM)this)==IDOK) 
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
	
	// connect the handlers...
	
//	glade_xml_signal_connect_data(xml, "on_dialog_destroy", (GtkSignalFunc) handler_Destroy, (void *) this);
	
//	glade_xml_signal_connect_data(xml, "on_button_begin_clicked", (GtkSignalFunc) handler_ButtonBegin, (void *) this);
//	glade_xml_signal_connect_data(xml, "on_button_prev_clicked", (GtkSignalFunc) handler_ButtonPrev, (void *) this);
//	glade_xml_signal_connect_data(xml, "on_button_play_clicked", (GtkSignalFunc) handler_ButtonPlay, (void *) this);
//	glade_xml_signal_connect_data(xml, "on_button_next_clicked", (GtkSignalFunc) handler_ButtonNext, (void *) this);
//	glade_xml_signal_connect_data(xml, "on_button_end_clicked", (GtkSignalFunc) handler_ButtonEnd, (void *) this);
	
//	glade_xml_signal_connect_data(xml, "on_button_close_clicked", (GtkSignalFunc) handler_ButtonClose, (void *) this);
	
//	gtk_widget_show(dialog);	// MODELESS
}

trajview_dialog::~trajview_dialog(void)
{
	prj->CloseTrajectory();
}

void trajview_dialog::handler_Destroy(HWND, void * data)
{
	trajview_dialog * ref = (trajview_dialog *) data;
	//cout << "handler_Destroy() : ref = " << ref << endl;
}

void trajview_dialog::handler_ButtonBegin(HWND, void * data)
{
	trajview_dialog * ref = (trajview_dialog *) data;
	
	i32s frame = 0;
	ref->prj->SetCurrentFrame(frame);
	ref->prj->ReadFrame();
	
	char mbuff1[128]; ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << "frame = " << ref->prj->GetCurrentFrame() << endl << ends;
	ref->prj->PrintToLog(mbuff1);
	
	ref->prj->UpdateAllGraphicsViews(true);
}

void trajview_dialog::handler_ButtonPrev(HWND, void * data)
{
	trajview_dialog * ref = (trajview_dialog *) data;
	
	i32s frame = ref->prj->GetCurrentFrame() - 1;
	i32s max = ref->prj->GetTotalFrames();
	
	if (frame < 0) frame = 0;
	if (frame >= max) frame = max - 1;
	
	ref->prj->SetCurrentFrame(frame);
	ref->prj->ReadFrame();
	
	char mbuff1[128]; ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << "frame = " << ref->prj->GetCurrentFrame() << endl << ends;
	ref->prj->PrintToLog(mbuff1);
	
	ref->prj->UpdateAllGraphicsViews(true);
}

DWORD WINAPI trajview_dialog::handler_ButtonPlay(void * data)
{
	trajview_dialog * ref = (trajview_dialog *) data;

	i32s max_frames = ref->prj->GetTotalFrames();
	for (i32s loop = 0;loop < max_frames;loop++)
	{
		ref->prj->SetCurrentFrame(loop);
		ref->prj->ReadFrame();
		
		ref->prj->UpdateAllGraphicsViews(true);
		::Sleep(100);
	}
	return 0;
}

void trajview_dialog::handler_ButtonNext(HWND, void * data)
{
	trajview_dialog * ref = (trajview_dialog *) data;

	i32s frame = ref->prj->GetCurrentFrame() + 1;
	i32s max = ref->prj->GetTotalFrames();
	
	if (frame < 0) frame = 0;
	if (frame >= max) frame = max - 1;
	
	ref->prj->SetCurrentFrame(frame);
	ref->prj->ReadFrame();
	
	char mbuff1[128]; ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << "frame = " << ref->prj->GetCurrentFrame() << endl << ends;
	ref->prj->PrintToLog(mbuff1);
	
	ref->prj->UpdateAllGraphicsViews(true);
}

void trajview_dialog::handler_ButtonEnd(HWND, void * data)
{
	trajview_dialog * ref = (trajview_dialog *) data;

	i32s frame = ref->prj->GetTotalFrames() - 1;
	ref->prj->SetCurrentFrame(frame);
	ref->prj->ReadFrame();
	
	char mbuff1[128]; ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << "frame = " << ref->prj->GetCurrentFrame() << endl << ends;
	ref->prj->PrintToLog(mbuff1);
	
	ref->prj->UpdateAllGraphicsViews(true);
}

void trajview_dialog::handler_ButtonClose(HWND, void * data)
{
	trajview_dialog * ref = (trajview_dialog *) data;
	//cout << "handler_ButtonClose() : ref = " << ref << endl;
	
	// close the dialog...
	// ^^^^^^^^^^^^^^^^^^^
	
//	gtk_widget_destroy(ref->dialog);
}

/*################################################################################################*/

// eof
