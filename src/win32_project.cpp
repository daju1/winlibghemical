// WIN32_PROJECT.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"
#include <windows.h>
#include "win32_project.h"	// config.h is here -> we get ENABLE-macros here...

#include "project.h"
#include "appdefine.h"

#include "../libghemical/src/geomopt.h"
#include "../libghemical/src/moldyn.h"

#include "../libghemical/src/eng1_qm.h"
#include "../libghemical/src/eng1_mm.h"
#include "../libghemical/src/eng1_sf.h"

#include "color.h"

#include "plane.h"
#include "surface.h"
#include "ribbon.h"
#include "win32_app.h"
#include "../consol/filedlg.h"
#include "win32_dialog.h"

#include "glade/trajview_dialog.h"

/*
#include "file_import_dialog.h"
#include "file_export_dialog.h"

#include "geomopt_dialog.h"
#include "moldyn_dialog.h"
*/
#include "glade/setup_dialog.h"
/*
#include "progress_dialog.h"
*/
#include "win32_graphics_view.h"


//#include <sys/stat.h>	// for stat
//#include <dirent.h>	// for scandir

//#include <glade/glade.h>

#include <strstream>
using namespace std;

/*################################################################################################*/

win_project::win_project(win_class_factory & p2) : project(p2)
{
//printf("win_project::win_project(win_class_factory & p2) : project(p2)\n\n");
	// only the control_factory() function from main.cpp should call this?!?!?!
	// only the control_factory() function from main.cpp should call this?!?!?!
	// only the control_factory() function from main.cpp should call this?!?!?!
	
	importpdb_mdata = NULL;		// temporary?!?!?!

	this->app = NULL;
	
	char buffer[1024];
	GetFullProjectFileName(buffer, 1024);

#if 0
	
	// the viewing widget is a vertically paned widget; it contains the notebook and the text log.
	paned_widget = gtk_vpaned_new();
	
	notebook_widget = gtk_notebook_new();
	
	txt_widget = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(txt_widget), false);
	
	txt_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(txt_widget));
	GtkTextIter txt_iter; gtk_text_buffer_get_end_iter(txt_buffer, & txt_iter);
	end_mark = gtk_text_buffer_create_mark(txt_buffer, NULL, & txt_iter, FALSE);	// right_gravity!
	
	scroll_widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scroll_widget), GTK_WIDGET(txt_widget));
	
	gtk_widget_show(txt_widget);
	
	gtk_paned_add1(GTK_PANED(paned_widget), GTK_WIDGET(notebook_widget));
	gtk_paned_add2(GTK_PANED(paned_widget), GTK_WIDGET(scroll_widget));

	gtk_widget_show(notebook_widget);
	gtk_widget_show(scroll_widget);
	
	gtk_widget_show(paned_widget);
#endif	
	// always add a project view and a graphics view by default.
	// add the project view first so that the graphics view remains on top...
	
	CreateProjectView();
	AddGraphicsView(NULL, false);
	
	PrintToLog("A new project created.\n");
	
#ifdef ENABLE_BONOBO
	control = NULL;
#endif	// ENABLE_BONOBO
	
#ifdef ENABLE_THREADS
	pd = NULL;
#endif	// ENABLE_THREADS
	
/*	GtkMenuItem * mi = GTK_MENU_ITEM(build_submenu[1].widget);
	MakeLibraryWidgets(mi, "library/", 1);	*/
}

win_project::~win_project(void)
{
	// check the graphics_view_vector for any "detached" views, and call gtk_widget_destroy for them.
	// the "attached" views, the ones that are linked to the GtkNotebook, will be destroyed with the notebook.
	// ok, this is a bit ugly solution, but see gtk_graphics_view::popup_ViewsAttachDetach() for more comments...
	
// THIS IS BAD!!!!!!!!!!!!!!!!!!
// THIS IS BAD!!!!!!!!!!!!!!!!!!
// THIS IS BAD!!!!!!!!!!!!!!!!!!

	for (i32u n1 = 0;n1 < graphics_view_vector.size();n1++)
	{
		win_graphics_view * gv = dynamic_cast<win_graphics_view *>(graphics_view_vector[n1]);
//		if (gv != NULL && gv->detached != NULL) gtk_widget_destroy(GTK_WIDGET(gv->detached));
	}

#ifdef ENABLE_THREADS
	if (pd != NULL) cout << "MEM-LEAK pd" << endl;
#endif	// ENABLE_THREADS

}

#ifdef ENABLE_BONOBO

BonoboControl * gtk_project::GetControl(void)
{
	if (!control) control = bonobo_control_new(paned_widget);
	return control;
}

#endif	// ENABLE_BONOBO
/*
HWND win_project::GetWindow(void)
{
	return paned_widget;
}
*/
void win_project::ThreadLock(void)
{
#ifdef ENABLE_THREADS
	gdk_threads_enter();
#endif	// ENABLE_THREADS
}

void win_project::ThreadUnlock(void)
{
#ifdef ENABLE_THREADS
	gdk_threads_leave();
#endif	// ENABLE_THREADS
}

bool win_project::SetProgress(double progress, double * graphdata)
{
#ifdef ENABLE_THREADS
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(pd->progressbar_job), progress);
	
	if (graphdata != NULL)
	{
		int f = (pd->g_fill % pd->g_sz);
		for (int i = 0;i < pd->g_n;i++)
		{
			pd->g_data[i * pd->g_sz + f] = graphdata[i];
		}
		
		pd->g_fill++;
		
		gtk_widget_queue_draw_area(pd->drawingarea_job, 0, 0, pd->da_w, pd->da_h);
	}
	
	return pd->cancel;
#else	// ENABLE_THREADS
	return false;
#endif	// ENABLE_THREADS
}

#ifdef ENABLE_THREADS

void win_project::CreateProgressDialog(const char * title, bool show_pbar, int graphs_n, int graphs_s)
{
	pd = new progress_dialog(title, show_pbar, graphs_n, graphs_s);
}

void win_project::DestroyProgressDialog(void)
{
	gtk_widget_destroy(pd->dialog);
	delete pd; pd = NULL;
}

#endif	// ENABLE_THREADS

// Print the message (no problems).
void win_project::Message(const char * msg)
{
	MessageBox(0,msg, "win_project::Message", 0);
/*
	GtkWidget * message_dialog = gtk_message_dialog_new(NULL,
	GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
	
	gtk_dialog_run(GTK_DIALOG(message_dialog));
	gtk_widget_destroy(message_dialog);*/
}

// Print the message (lower severity).
void win_project::WarningMessage(const char * msg)
{
	MessageBox(0,msg, "win_project::WarningMessage", 0);
/*	GtkWidget * message_dialog = gtk_message_dialog_new(NULL,
	GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", msg);
	
	gtk_dialog_run(GTK_DIALOG(message_dialog));
	gtk_widget_destroy(message_dialog);*/
}

// Print the message (higher severity).
void win_project::ErrorMessage(const char * msg)
{
	MessageBox(0,msg, "win_project::ErrorMessage", 0);
/*	GtkWidget * message_dialog = gtk_message_dialog_new(NULL,
	GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", msg);
	
	gtk_dialog_run(GTK_DIALOG(message_dialog));
	gtk_widget_destroy(message_dialog);*/
}

// Print the message and wait for a yes/no response.
bool win_project::Question(const char * msg)
{
	return 
		MessageBox(0, msg, "win_project::Question", 
		MB_OKCANCEL | MB_ICONQUESTION) == IDOK;
/*	GtkWidget * question_dialog = gtk_message_dialog_new(NULL,
	GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s", msg);
	
	gint response = gtk_dialog_run(GTK_DIALOG(question_dialog));
	gtk_widget_destroy(question_dialog);
	
	switch (response)
	{
		case GTK_RESPONSE_YES:
		return true;
		
		default:
		return false;
	}*/
}

void win_project::PrintToLog(const char * message)
{
	printf (" PrintToLog: %s", message);
/*	GtkTextIter txt_iter;
	gtk_text_buffer_get_iter_at_mark(txt_buffer, & txt_iter, end_mark);
	
	gtk_text_buffer_insert(txt_buffer, & txt_iter, message, -1);
	
	gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(txt_widget), end_mark);*/
}

void win_project::UpdateAllWindowTitles()
{
	// mdi_child_name and view titles are somewhat different issues now; check this!!!
	// we should change mdi_child_name when the project name changes, but this function is for setting the view titles.
	
	char buffer[1024];
	GetProjectFileName(buffer, 1024, true);
	
	//gnome_mdi_child_set_name(GNOME_MDI_CHILD(child), buffer);
	project::UpdateAllWindowTitles();
}
#if 0

void gtk_project::MakeLibraryWidgets(GtkMenuItem * menu, const char * directory_path, int recursion)
{
  struct stat stat_buf;

  if ( stat(directory_path, &stat_buf) == 0 && S_ISDIR(stat_buf.st_mode) )
    {
      struct dirent *namelist;
      DIR *dirList;
      GtkWidget *library_item;

       if (( dirList = opendir(directory_path) ))
	 {
//	   char *filename = new char[1024];		// OLD
	   while (( namelist = readdir(dirList) ))
	     {
	       char *filename = new char[1024];		// a new string for each item???
	       
	       strncpy(filename, directory_path, 512);
	       strncat(filename, namelist->d_name, 511);
	       
	       if ( namelist->d_name[0] != '.' &&
		    stat(filename, &stat_buf) == 0 &&
		    strstr(namelist->d_name, ".gpr") != NULL)
		 {
		   library_item = gtk_menu_item_new_with_label(namelist->d_name);
		   
		   // we pass the string "filename" here as user data.
		   // "filename" must be unique for each item, and it must stay in memory.
		   
		   gtk_signal_connect(GTK_OBJECT(library_item), "activate",
				      GTK_SIGNAL_FUNC(popup_Library),
				      filename);
		   
		   gtk_object_set_user_data(GTK_OBJECT(library_item), 
					    (project *) this);

		   gtk_widget_show(library_item);

		   gtk_menu_append(GTK_MENU(menu->submenu),
				   library_item);
		 }
	       else if ( namelist->d_name[0] != '.' &&
			 stat(filename, &stat_buf) == 0 &&
			 S_ISDIR(stat_buf.st_mode) &&
			 recursion != 0)
		 {
		   library_item = gtk_menu_item_new_with_label(namelist->d_name);
		   gtk_menu_item_set_submenu(GTK_MENU_ITEM(library_item),
					     gtk_menu_new());

		   gtk_widget_show(library_item);
		   
		   gtk_menu_append(GTK_MENU(menu->submenu),
				   library_item);
		   
		   strcat(filename, "/");
		   MakeLibraryWidgets(GTK_MENU_ITEM(library_item), filename, recursion-1);
		 }
	// this would be the correct place to delete the string, if it were temporary.
	// however, we must retain the strings in memory because we will use them later, so this will create a (small) memory leak.
	// we should either store the strings to a static block of memory, or someway arrange the memory release when program stops.
//	   delete [] filename;		// can't do this, or we might use freed (and possibly overwritten) memory later...
	     }
//	   delete [] filename;		// OLD
	   closedir(dirList);
	 }

     }
}
#endif

#include "./glade/geomopt_dialog.h"

void win_project::GeomOptGetParam(geomopt_param & param)
{

	//}
	//else
	//	param.confirm = false;	

	new geomopt_dialog(& param);		// the object will call delete itself...
	// the above dialog is a modal one; we will return from the function only after the dialog is closed.

	if (param.confirm)
	{
		win_app * app = this->GetApp();
		if (app) 
		{
			app->maintb_tool_OrbitXY(NULL, NULL, NULL);

			CheckMenuItem(GetMenu( this->paned_widget ), ID_MOUSE_DRAW,		MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetMenu( this->paned_widget ), ID_MOUSE_ERASE,		MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetMenu( this->paned_widget ), ID_MOUSE_SELECT,		MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetMenu( this->paned_widget ), ID_MOUSE_ZOOM,		MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetMenu( this->paned_widget ), ID_MOUSE_CLIPPING,	MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetMenu( this->paned_widget ), ID_MOUSE_TRANSLATEXY,MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetMenu( this->paned_widget ), ID_MOUSE_TRANSLATEZ, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetMenu( this->paned_widget ), ID_MOUSE_ORBITXY,	MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetMenu( this->paned_widget ), ID_MOUSE_ORBITZ,		MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetMenu( this->paned_widget ), ID_MOUSE_ROTATEXY,	MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetMenu( this->paned_widget ), ID_MOUSE_ROTATEZ,	MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem(GetMenu( this->paned_widget ), ID_MOUSE_MEASURE,	MF_BYCOMMAND | MF_UNCHECKED);
			
			CheckMenuItem(GetMenu( this->paned_widget ), ID_MOUSE_ORBITXY, MF_BYCOMMAND | MF_CHECKED);

		}
	}
}

#include "./glade/moldyn_dialog.h"

void win_project::MolDynGetParam(moldyn_param & param)
{
	new moldyn_dialog(& param);		// the object will call delete itself...
	
	// the above dialog is a modal one; we will return from the function only after the dialog is closed.

}

void win_project::MolDynGetParam(moldyn_tst_param & param)
{
	new moldyn_tst_dialog(& param);		// the object will call delete itself...
	
	// the above dialog is a modal one; we will return from the function only after the dialog is closed.

}
// the popup-menu callbacks are here...
// the popup-menu callbacks are here...
// the popup-menu callbacks are here...
void win_project::popup_FileLoadBox(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
//	if (prj) new file_open_dialog(prj);	// will call delete itself...
	char filename[1048];
	DWORD nFilterIndex;
	if (OpenFileDlg(widget, "Box File (*.box)\0*.box\0All files \0*.*\0", filename, nFilterIndex) == S_OK)
	{			
		if (prj == NULL)
		{
			// set a the new project object ; this is only for a stand-alone app ; FIXME bonobo.
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			
//			gtk_container_remove(GTK_CONTAINER(gtk_app::main_vbox), gtk_app::prj->GetWidget());
			
			delete win_app::prj;
			win_app::prj = NULL;
			
			win_app::prj = new win_project(* win_class_factory::GetInstance());
//			GtkWidget * widget = prj->GetWidget();
			
//			gtk_container_add(GTK_CONTAINER(gtk_app::main_vbox), widget);
		}

		prj->LoadBox(filename);
		

		
		prj->UpdateAllGraphicsViews();
	}
//	return true;
}

void win_project::popup_FileLoadSelect(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
//	if (prj) new file_open_dialog(prj);	// will call delete itself...
	char filename[1048];
	DWORD nFilterIndex;
	if (OpenFileDlg(widget, "Dat File (*.dat)\0*.dat\0All files \0*.*\0", filename, nFilterIndex) == S_OK)
	{			
		if (prj == NULL)
		{
			// set a the new project object ; this is only for a stand-alone app ; FIXME bonobo.
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			
//			gtk_container_remove(GTK_CONTAINER(gtk_app::main_vbox), gtk_app::prj->GetWidget());
			
			delete win_app::prj;
			win_app::prj = NULL;
			
			win_app::prj = new win_project(* win_class_factory::GetInstance());
//			GtkWidget * widget = prj->GetWidget();
			
//			gtk_container_add(GTK_CONTAINER(gtk_app::main_vbox), widget);
		}

		prj->LoadSelected(filename);
		

		
		prj->UpdateAllGraphicsViews();
	}
//	return true;
}
void win_project::popup_ClearMolecularAxises(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	{
		prj->ClearMolecularAxises();
		prj->UpdateAllGraphicsViews();
	}
}
void win_project::popup_FileLoadMolecularAxises(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
//	if (prj) new file_open_dialog(prj);	// will call delete itself...
	char filename[1048];
	DWORD nFilterIndex;
	if (OpenFileDlg(widget, "Molecular Axises File (*.mlax)\0*.mlax\0All files \0*.*\0", filename, nFilterIndex) == S_OK)
	{			
		if (prj == NULL)
		{
			// set a the new project object ; this is only for a stand-alone app ; FIXME bonobo.
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			
//			gtk_container_remove(GTK_CONTAINER(gtk_app::main_vbox), gtk_app::prj->GetWidget());
			
			delete win_app::prj;
			win_app::prj = NULL;
			
			win_app::prj = new win_project(* win_class_factory::GetInstance());
//			GtkWidget * widget = prj->GetWidget();
			
//			gtk_container_add(GTK_CONTAINER(gtk_app::main_vbox), widget);
		}

		prj->LoadMolecularAxises(filename);
		

		
		prj->UpdateAllGraphicsViews();
	}
//	return true;
}


void win_project::popup_FileOpen(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
//	if (prj) new file_open_dialog(prj);	// will call delete itself...
	char filename[1048];
	DWORD nFilterIndex;
	if (OpenFileDlg(widget, "Ghemical Project File (*.gpr)\0*.gpr\0All files \0*.*\0", filename, nFilterIndex) == S_OK)
	{			
		if (prj == NULL)
		{
			// set a the new project object ; this is only for a stand-alone app ; FIXME bonobo.
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			
//			gtk_container_remove(GTK_CONTAINER(gtk_app::main_vbox), gtk_app::prj->GetWidget());
			
			delete win_app::prj;
			win_app::prj = NULL;
			
			win_app::prj = new win_project(* win_class_factory::GetInstance());
//			GtkWidget * widget = prj->GetWidget();
			
//			gtk_container_add(GTK_CONTAINER(gtk_app::main_vbox), widget);
		}
		
		bool open = prj->IsEmpty();	// open, not insert
		
		ifstream ifile;
		ifile.open(filename, ios::in);
		if (ifile.bad())
		{
			printf("file \"%s\" did not opened\n", filename);
			return;
		}
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
	}
//	return true;
}
void win_project::popup_FileClose(HWND widget, void * data)
{
	// win_graphics_view * gv = GetGV(widget);
	
	// apparently, it is not necessary to call gnome_mdi_remove_view() for the view here...
	// apparently, it is not necessary to call gnome_mdi_remove_view() for the view here...
	// apparently, it is not necessary to call gnome_mdi_remove_view() for the view here...
	
	//gnome_mdi_remove_child(gnome_mdi_app::mdi, GNOME_MDI_CHILD(gv->prj->child), FALSE);
}

void win_project::popup_FileImport(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	//if (prj) new file_import_dialog(prj);		// will call delete itself...
#ifdef ENABLE_OPENBABEL

	const size_t N = 1024*1024;
	size_t n = 0;
	char * filter = new char[N];
	char *p1, *p2, *p;
	p = filter;
	file_trans ft;
	char tmp[1024];
	char tmp2[1024];

	for (unsigned int i = 0;i < ft.NumImports();i++)			// insert other items...
	{
		string temp = ft.GetImportDescription(i);

		strcpy(tmp,temp.c_str());
		strcpy(tmp2,temp.c_str());

		p1 = strrchr(tmp2,'.');
		p2 = p1-1;
		*p2 = '*';

		size_t len = strlen(tmp);
		size_t len2 = strlen(p2);

		strncpy(p, tmp, len);
		p[len] = '\0'; p += len+1;
		n+=len+1;

		strncpy(p, p2, len2);
		p[len2] = '\0'; p += len2+1;
		n+=len2+1;

	}

	strcpy(tmp,"All files(*.*)\0");
	strcpy(tmp2,"*.*\0");

	size_t len = strlen(tmp);
	size_t len2 = strlen(tmp2);

	strncpy(p, tmp, len);
	p[len] = '\0'; p += len+1;
		n+=len+1;

	strncpy(p, tmp2, len2);
	p[len2] = '\0'; 
	p += len2+1;
		n+=len2+1;
		
	

#endif	// ENABLE_OPENBABEL

	char filename[1048];
	DWORD nFilterIndex;
	if (OpenFileDlg(widget, 
		filter, 
		filename, nFilterIndex) == S_OK)
	{			
		if (prj == NULL)
		{
			// set a the new project object ; this is only for a stand-alone app ; FIXME bonobo.
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			
//			gtk_container_remove(GTK_CONTAINER(gtk_app::main_vbox), gtk_app::prj->GetWidget());
			
			delete win_app::prj;
			win_app::prj = NULL;
			
			win_app::prj = new win_project(* win_class_factory::GetInstance());
//			GtkWidget * widget = prj->GetWidget();
			
//			gtk_container_add(GTK_CONTAINER(gtk_app::main_vbox), widget);
		}
		
#ifdef ENABLE_OPENBABEL

		int index = nFilterIndex-1;

		if ( nFilterIndex == ft.NumImports()+1)
			index = 0;

		printf("nFilterIndex = %u index %d N = %u n = %u ft.NumImports() %u\n", nFilterIndex, index, N, n, ft.NumImports());
	
		prj->ImportFile(filename, index);
		prj->ParseProjectFileNameAndPath(filename);
		prj->UpdateAllWindowTitles();
	
#endif	// ENABLE_OPENBABEL
	
		prj->CenterCRDSet(0, true);
		prj->UpdateAllGraphicsViews();
	}



	delete [] filter;
  printf("popup_FileImport end\n");
}

void win_project::popup_FileExport(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	//if (prj) new file_export_dialog(prj);		// will call delete itself...
#ifdef ENABLE_OPENBABEL
	const size_t N = 1024*1024;
	size_t n = 0;
	char * filter = new char[N];
	char *p1, *p2, *p;
	p = filter;
	char tmp[1024];
	char tmp2[1024];
	
	file_trans ft;
	for (unsigned int i = 0;i < ft.NumExports();i++)			// insert other items...
	{
		string temp = ft.GetExportDescription(i);
		strcpy(tmp,temp.c_str());
		strcpy(tmp2,temp.c_str());

		p1 = strrchr(tmp2,'.');
		p2 = p1-1;
		*p2 = '*';

		size_t len = strlen(tmp);
		size_t len2 = strlen(p2);

		strncpy(p, tmp, len);
		p[len] = '\0'; p += len+1;
		n+=len+1;

		strncpy(p, p2, len2);
		p[len2] = '\0'; p += len2+1;
		n+=len2+1;
	}
	
#endif	// ENABLE_OPENBABEL
	char filename[1048];filename[0] = '\0';
	DWORD nFilterIndex;
	if (SaveFileDlg(widget, 
		filename, 
		filter, 
		nFilterIndex) == S_OK)
	{			
#ifdef ENABLE_OPENBABEL
		int index = nFilterIndex-1;
		prj->ExportFile(filename, index);
		prj->ParseProjectFileNameAndPath(filename);
		prj->UpdateAllWindowTitles();	
#endif	// ENABLE_OPENBABEL
	}
}


void win_project::popup_FileExportGraphic(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj) new file_save_graphics_dialog(prj);	// will call delete itself...
}

void win_project::popup_FileSaveAs(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	//if (prj) new file_save_dialog(prj);		// will call delete itself...
	TCHAR filter[] =     TEXT("Ghemical Project File (*.gpr)\0*.gpr\0")
						 TEXT("All Files (*.*)\0*.*\0");
	TCHAR filename[256];

	sprintf(filename, "\0");
	DWORD nFilterIndex;
	if (SaveFileDlg(0, filename, filter, nFilterIndex) == S_OK)
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

//		return true;
	}
}

void win_project::popup_FileSaveBox(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	//if (prj) new file_save_dialog(prj);		// will call delete itself...
	TCHAR filter[] =     TEXT("Box File (*.box)\0*.box\0")
						 TEXT("All Files (*.*)\0*.*\0");
	TCHAR filename[256];

	sprintf(filename, "\0");
	DWORD nFilterIndex;
	if (SaveFileDlg(0, filename, filter, nFilterIndex) == S_OK)
	{
		if (prj == NULL)
		{
			prj->ErrorMessage("BUG: file_save_dialog::OkEvent() failed.");
			exit(EXIT_FAILURE);
		}		
		prj->SaveBox(filename);

	}
}


void win_project::popup_FileSaveSelected(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	//if (prj) new file_save_dialog(prj);		// will call delete itself...
	TCHAR filter[] =     TEXT("Dat File (*.dat)\0*.dat\0")
						 TEXT("All Files (*.*)\0*.*\0");
	TCHAR filename[256];

	sprintf(filename, "\0");
	DWORD nFilterIndex;
	if (SaveFileDlg(0, filename, filter, nFilterIndex) == S_OK)
	{
		if (prj == NULL)
		{
			prj->ErrorMessage("BUG: file_save_dialog::OkEvent() failed.");
			exit(EXIT_FAILURE);
		}		
		prj->SaveSelected(filename);

	}
}




void win_project::popup_FileSaveMolecularAxises(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	//if (prj) new file_save_dialog(prj);		// will call delete itself...
	TCHAR filter[] =     TEXT("Molecular Axises File (*.mlax)\0*.mlax\0")
						 TEXT("All Files (*.*)\0*.*\0");
	TCHAR filename[256];

	sprintf(filename, "\0");
	DWORD nFilterIndex;
	if (SaveFileDlg(0, filename, filter, nFilterIndex) == S_OK)
	{
		if (prj == NULL)
		{
			prj->ErrorMessage("BUG: file_save_dialog::OkEvent() failed.");
			exit(EXIT_FAILURE);
		}		
		prj->SaveMolecularAxises(filename);
	}
}




#if 0
void win_project::popup_FileExtra1(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj) new importpdb_dialog(prj);		// will call delete itself???
}

void win_project::popup_FileExtra2(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (prj->importpdb_mdata == NULL)
		{
			prj->Message("Please import a PDB file first;\nSee File/ExtraTools/ImportPDB...");
			return;
		}
		
prj->Message("sorry, this is under construction now...");
//		prj->aa2sf_ConvertAll(prj->importpdb_mdata);
//		prj->UpdateAllGraphicsViews();
	}
}
#endif
void win_project::popup_SelectAll(HWND widget, void * data)
{
	win_graphics_view::GetGV(widget)->prj->SelectAll();
}

void win_project::popup_SelectNone(HWND widget, void * data)
{
	win_graphics_view::GetGV(widget)->prj->SelectAll();		// should call the base class function to prevent the flash!!!
	win_graphics_view::GetGV(widget)->prj->InvertSelection();
}

void win_project::popup_InvertSelection(HWND widget, void * data)
{
	win_graphics_view::GetGV(widget)->prj->InvertSelection();
}

void win_project::popup_DeleteSelection(HWND widget, void * data)
{
	win_graphics_view::GetGV(widget)->prj->DeleteSelection();
}

void win_project::popup_SelectModeAtom(HWND widget, void * data)
{
	ogl_view::select_mode = select_tool::Atom;
cout << "select_mode = atm" << endl;
}

void win_project::popup_SelectModeResidue(HWND widget, void * data)
{
	ogl_view::select_mode = select_tool::Residue;
cout << "select_mode = res" << endl;
}

void win_project::popup_SelectModeChain(HWND widget, void * data)
{
	ogl_view::select_mode = select_tool::Chain;
cout << "select_mode = chn" << endl;
}

void win_project::popup_SelectModeMolecule(HWND widget, void * data)
{
	ogl_view::select_mode = select_tool::Molecule;
cout << "select_mode = mol" << endl;
}

void win_project::popup_ViewsNewELD(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		prj->AddEnergyLevelDiagramView(true);
	}
}

void win_project::popup_RModeBallStick(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->render = RENDER_BALL_AND_STICK; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_RModeVanDerWaals(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->render = RENDER_VAN_DER_WAALS; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_RModeCylinders(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->render = RENDER_CYLINDERS; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_RModeWireframe(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->render = RENDER_WIREFRAME; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_RModeNothing(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->render = RENDER_NOTHING; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_CModeElement(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->colormode = & project::cm_element; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_CModeSecStruct(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->colormode = & project::cm_secstruct; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_CModeHydPhob(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->colormode = & project::cm_hydphob; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_LModeIndex(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->label = LABEL_INDEX; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_LModeElement(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->label = LABEL_ELEMENT; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_LModeFCharge(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->label = LABEL_F_CHARGE; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_LModePCharge(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->label = LABEL_P_CHARGE; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_LModeAtomType(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->label = LABEL_ATOMTYPE; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_LModeBuilderID(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->label = LABEL_BUILDER_ID; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_LModeBondType(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->label = LABEL_BONDTYPE; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_LModeResidue(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->label = LABEL_RESIDUE; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_LModeSecStruct(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->label = LABEL_SEC_STRUCT; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_LModeNothing(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->label = LABEL_NOTHING; gv->prj->UpdateAllGraphicsViews();
}

void win_project::popup_ObjRibbon(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (!prj->ref_civ) prj->UpdateChains();
		vector<chn_info> & ci_vector = (* prj->ref_civ);
		for (i32u n1 = 0;n1 < ci_vector.size();n1++)
		{
			if (ci_vector[n1].GetType() != chn_info::amino_acid) continue;
			if (ci_vector[n1].GetLength() < 3) continue;
			
			if (ci_vector[n1].GetSecStrStates() == NULL) DefineSecondaryStructure(prj);
			
			prj->AddObject(new ribbon(prj, gv->colormode, n1, 4));		// min. order is 2!!!
		}
		
		prj->UpdateAllGraphicsViews();
	}
}

void win_project::popup_ObjEPlane(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (!prj->GetCurrentSetup()->GetCurrentEngine())
		{
			prj->Message("Please calculate energy first!");
		}
		else
		{
			static const char command[] = "add plane esp rb1 138.0 AUTO 1.0 50 1 0.75";
			new command_dialog(prj, gv, command);
		}
	}
}

void win_project::popup_ObjEVolume(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (!prj->GetCurrentSetup()->GetCurrentEngine())
		{
			prj->Message("Please calculate energy first!");
		}
		else
		{
			static const char command[] = "add volrend esp rb2 138.0 0.0 1.0 25 0.50";
			new command_dialog(prj, gv, command);
		}
	}
}

void win_project::popup_ObjESurface(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (!prj->GetCurrentSetup()->GetCurrentEngine())
		{
			prj->Message("Please calculate energy first!");
		}
		else
		{
			static const char command[] = "add surf2 esp unity red blue +35.0 -35.0 1.0 0.0 2.0 50 0 0 0.50";
			new command_dialog(prj, gv, command);
		}
	}
}

void win_project::popup_ObjEVDWSurface(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (!prj->GetCurrentSetup()->GetCurrentEngine())
		{
			prj->Message("Please calculate energy first!");
		}
		else
		{
			static const char command[] = "add surf1 vdws esp rb1 1.0 70.0 AUTO 2.0 50 1 1 0.65";
			new command_dialog(prj, gv, command);
		}
	}
}

void win_project::popup_ObjEDPlane(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (!prj->GetCurrentSetup()->GetCurrentEngine())
		{
			prj->Message("Please calculate energy first!");
		}
		else
		{
			static const char command[] = "add plane eldens rb1 0.05 0.0 0.75 50 1 0.75";
			new command_dialog(prj, gv, command);
		}
	}
}

void win_project::popup_ObjEDSurface(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (!prj->GetCurrentSetup()->GetCurrentEngine())
		{
			prj->Message("Please calculate energy first!");
		}
		else
		{
			static const char command[] = "add surf1 eldens unity red 0.01 1.0 0.0 1.5 50 0 0 0.65";
			new command_dialog(prj, gv, command);
		}
	}
}

void win_project::popup_ObjMOPlane(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (!prj->GetCurrentSetup()->GetCurrentEngine())
		{
			prj->Message("Please calculate energy first!");
		}
		else
		{
			static const char command[] = "add plane mo rb1 0.05 0.0 0.75 50 1 0.75";
			new command_dialog(prj, gv, command);
		}
	}
}

void win_project::popup_ObjMOVolume(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (!prj->GetCurrentSetup()->GetCurrentEngine())
		{
			prj->Message("Please calculate energy first!");
		}
		else
		{
			static const char command[] = "add volrend mo rb2 0.025 0.0 1.5 25 0.50";
			new command_dialog(prj, gv, command);
		}
	}
}

void win_project::popup_ObjMOSurface(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (!prj->GetCurrentSetup()->GetCurrentEngine())
		{
			prj->Message("Please calculate energy first!");
		}
		else
		{
			static const char command[] = "add surf2 mo unity red blue +0.025 -0.025 1.0 0.0 1.5 50 0 0 0.50";
			new command_dialog(prj, gv, command);
		}
	}
}

void win_project::popup_ObjMODPlane(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (!prj->GetCurrentSetup()->GetCurrentEngine())
		{
			prj->Message("Please calculate energy first!");
		}
		else
		{
			static const char command[] = "add plane mod rb1 0.005 0.0 0.75 50 1 0.75";
			new command_dialog(prj, gv, command);
		}
	}
}

void win_project::popup_ObjMODVolume(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (!prj->GetCurrentSetup()->GetCurrentEngine())
		{
			prj->Message("Please calculate energy first!");
		}
		else
		{
			static const char command[] = "add volrend mod rb2 0.0025 0.0 1.5 25 0.35";
			new command_dialog(prj, gv, command);
		}
	}
}

void win_project::popup_ObjMODSurface(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (!prj->GetCurrentSetup()->GetCurrentEngine())
		{
			prj->Message("Please calculate energy first!");
		}
		else
		{
			static const char command[] = "add surf1 mod unity red 0.0025 1.0 0.0 1.5 50 0 0 0.65";
			new command_dialog(prj, gv, command);
		}
	}
}

void win_project::popup_ObjectsDeleteCurrent(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	gv->prj->DoDeleteCurrentObject();
}

void win_project::popup_CompSetup(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	
	new setup_dialog(prj);		// will call delete itself...
}

void win_project::popup_CompEnergy(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj) prj->DoEnergy();
}
#if PROBNIY_ATOM_GEOMOPT
void win_project::popup_Comp_work_prob_atom_GeomOpt(geomopt_param & param, char * infile_name, char * trgtlst_name, char * box_name, char * fixed_name, int total_frames)
{
	win_project * prj = this;
	if (prj)
	{
		setup * su = prj->GetCurrentSetup();
		static jobinfo_work_prob_atom_GeomOpt ji;
		
		ji.prj = prj;
		//ji.go = geomopt_param(su);
		ji.go = &param;
		ji.total_frames		= total_frames;

		ji.infile_name		= infile_name;
		ji.trgtlst_name		= trgtlst_name;
		ji.box_name			= box_name;
		ji.fixed_name		= fixed_name;
		
		prj->start_job_work_prob_atom_GeomOpt(& ji);
	}
}
#endif
void win_project::popup_CompGeomOpt(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		setup * su = prj->GetCurrentSetup();
		static jobinfo_GeomOpt ji;
		
		ji.prj = prj;
		ji.go = geomopt_param(su);
		
		prj->start_job_GeomOpt(& ji);
	}
}

void win_project::popup_CompMolDyn(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		setup * su = prj->GetCurrentSetup();
		static jobinfo_MolDyn ji;
		
		ji.prj = prj;
		ji.md = moldyn_param(su);
		
		prj->start_job_MolDyn(& ji);
	}
}

void win_project::popup_CompMolDyn_tst(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		setup * su = prj->GetCurrentSetup();
		static jobinfo_MolDyn_tst ji;
		
		ji.prj = prj;
		ji.md = moldyn_tst_param(su);
		
		prj->start_job_MolDyn_tst(& ji);
	}
}


void win_project::popup_CompRandomSearch(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		static const char command[] = "random_search 100 250";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_CompSystematicSearch(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		static const char command[] = "systematic_search 6 250";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_CompMonteCarloSearch(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		static const char command[] = "montecarlo_search 10 100 250";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_CompTorsionEnergyPlot1D(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
prj->Message("PLEASE NOTE!\nThe command string, which is displayed in the next dialog, is incomplete.\nYou should replace the letters A-D with atom indices that define the torsion.\n\nALSO NOTE: structure refinement is always done using molecular mechanics (optsteps).");
		
		static const char command[] = "make_plot1 A B C D 36 0.0 360.0 250";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_TrajSetTotalFrames(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (prj->GetTrajectoryFile())
			prj->CloseTrajectory();

		DWORD nFilterIndex;
		char filename[512];
		if (S_OK != OpenFileDlg(widget, "Ghemical Trajectory File (*.traj)\0*.traj\0All files \0*.*\0", filename, nFilterIndex))
		{	
			return;
		}

		prj->OpenTrajectory(filename);	

		// CalcTotalFrames
		size_t start_pos = prj->trajfile->tellg();
		prj->trajfile->seekg(0, std::ios::end);
		size_t end_pos = prj->trajfile->tellg();
		prj->trajfile->seekg(start_pos, ios::beg);
		size_t traj_body_size = end_pos - start_pos;

		i32s max_frames = prj->GetTotalFrames();
		size_t real_frames = traj_body_size / prj->GetTrajectoryFrameSize();

		prj->CloseTrajectory();

		//prj->Message("PLEASE NOTE!\nThe command string, which is displayed in the next dialog, is incomplete.\nYou should replace the letter A with nframes.");
		//static const char command[] = "traj_set_total_frames A ";
		//new command_dialog(prj, gv, command);

		prj->TrajectorySetTotalFrames(filename, real_frames);

	}
}
void win_project::popup_UnSetSelectedAtomsUnderGravi(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "unset_flag_on_sel_atoms 16";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_SetSelectedAtomsUnderGravi(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "set_flag_on_sel_atoms 16";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_UnSetSelectedAtomsAsSolvent(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "unset_flag_on_sel_atoms 8";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_SetSelectedAtomsAsSolvent(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "set_flag_on_sel_atoms 8";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_UnSetSelectedAtomsAsWorking(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "unset_flag_on_sel_atoms 7";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_SetSelectedAtomsAsWorking(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "set_flag_on_sel_atoms 7";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_UnSetSelectedAtomsAsSpecial(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "unset_flag_on_sel_atoms 6";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_SetSelectedAtomsAsSpecial(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "set_flag_on_sel_atoms 6";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_UnLockSelectedAtoms(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "unset_flag_on_sel_atoms 15";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_LockSelectedAtoms(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "set_flag_on_sel_atoms 15";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_SelectMolecularAxises(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		prj->SelectMolecularAxises();
	}
}

void win_project::popup_SelectLockedAtoms(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "sel_atoms_with_flag 15";
		new command_dialog(prj, gv, command);
	}
}


void win_project::popup_SelectWorkingAtoms(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "sel_atoms_with_flag 7";
		new command_dialog(prj, gv, command);
	}
}


void win_project::popup_SelectSpecialAtoms(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "sel_atoms_with_flag 6";
		new command_dialog(prj, gv, command);
	}
}


void win_project::popup_SelectSolventAtoms(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "sel_atoms_with_flag 8";
		new command_dialog(prj, gv, command);
	}
}


void win_project::popup_SelectUnderGraviAtoms(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{		
		static const char command[] = "sel_atoms_with_flag 16";
		new command_dialog(prj, gv, command);
	}
}


void win_project::popup_CompNematicOrderParamTrajPlot1D(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
prj->Message("PLEASE NOTE!\n"
			 "The command string, which is displayed in the next dialog, is incomplete.\n"
			 "You should replace the letters TYPE with\n"
			 "0 - ORDER PARAMETER"
			 "1 - DIRECTOR ANGLE"
			 );
		
		static const char command[] = "make_nematic_plot TYPE";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_CompCoordinateDifferenceTrajPlot1D(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		prj->Message("PLEASE NOTE!\nThe command string, which is displayed in the next dialog, is incomplete.\nYou should replace the letters IND1, IND2 and DIM with atom indexes and dimension (0,1 or 2) \nthat define the coordinate difference: coord(IND1)-coord(IND2).");
		
		static const char command[] = "make_plot_crd_diff IND1 IND2 DIM";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_CompVeloncityDistribution2D(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		//prj->Message("PLEASE NOTE!\nThe command string, which is displayed in the next dialog, is incomplete.\nYou should replace the letters IND1, IND2 and DIM with atom indexes and dimension (0,1 or 2) \nthat define the coordinate difference.");
		
		static const char command[] = "vel_dist_2d 100 100 2";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_CompCoordinateTrajPlot1D(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
prj->Message("PLEASE NOTE!\nThe command string, which is displayed in the next dialog, is incomplete.\nYou should replace the letters IND and DIM with atom index and dimension (0,1 or 2) that define the coordinate.");
		
		static const char command[] = "make_plot_crd IND DIM";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_CompDistanceTrajPlot1D(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
prj->Message("PLEASE NOTE!\nThe command string, which is displayed in the next dialog, is incomplete.\nYou should replace the letters A-B with atom indices that define the distance.");
		
		static const char command[] = "make_plot_dist A B";
		new command_dialog(prj, gv, command);
	}
}


void win_project::popup_CompAngleTrajPlot1D(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
prj->Message("PLEASE NOTE!\nThe command string, which is displayed in the next dialog, is incomplete.\nYou should replace the letters A-C with atom indices that define the angle.");
		
		static const char command[] = "make_plot_ang A B C";
		new command_dialog(prj, gv, command);
	}
}


void win_project::popup_CompAngleEnergyPlot1D(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
prj->Message("PLEASE NOTE!\nThe command string, which is displayed in the next dialog, is incomplete.\nYou should replace the letters A-C with atom indices that define the angle.\n\nALSO NOTE: structure refinement is always done using molecular mechanics (optsteps).");
		
		static const char command[] = "make_plot_a A B C 600 -60.0 60.0 250 angle_energy_out.txt";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_CompTorsionEnergyPlot2D(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
prj->Message("PLEASE NOTE!\nThe command string, which is displayed in the next dialog, is incomplete.\nYou should replace the letters A-D and I-L with atom indices that define the torsions.\n\nALSO NOTE: structure refinement is always done using molecular mechanics (optsteps).");
		
		static const char command[] = "make_plot2 A B C D 36 0.0 360.0 I J K L 36 0.0 360.0 250";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_CompPopAnaElectrostatic(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		static const char command[] = "population_analysis_ESP";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_CompTransitionStateSearch(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		static const char command[] = "transition_state_search 10.0 500.0";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_CompStationaryStateSearch(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		static const char command[] = "stationary_state_search 100";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_CompDensity(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj) prj->DoDensity();
}

void win_project::popup_CompFormula(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj) prj->DoFormula();
}

void win_project::popup_CompSetFormalCharge(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		static const char command[] = "set_formal_charge X +0";
		new command_dialog(prj, gv, command);
	}
}
void win_project::popup_CompCreateRS(HWND widget, void * data)	// todo : this is only for testing?!?!?!?
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		if (prj->GetRS() == NULL) prj->CreateRS();
	}
}

void win_project::popup_CompCycleRS(HWND widget, void * data)	// todo : this is only for testing?!?!?!?
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj && prj->GetRS() != NULL)
	{
	//	prj->GetRS()->CycleStructures();
		prj->UpdateAllGraphicsViews();
	}
	else cout << "ERROR" << endl;
}
void win_project::popup_TrajView_EnergyPlot(HWND widget, void * data, bool do_potencial)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)// new trajfile_dialog(prj);	// will call delete itself...
	{
		if (!prj->GetTrajectoryFile())
		{

			char filename[512];
			DWORD nFilterIndex;
			if (OpenFileDlg(widget, "Ghemical Trajectory File (*.traj)\0*.traj\0All files \0*.*\0", filename, nFilterIndex) == S_OK)
			{
				cout << "trying to open \"" << filename << "\"." << endl;
				prj->OpenTrajectory(filename);
				// check if there were problems with OpenTrajectory()?!?!?!
				// check if there were problems with OpenTrajectory()?!?!?!
				// check if there were problems with OpenTrajectory()?!?!?!
					
				const char * s1 = "frame(num)"; const char * sv = "E(kJ/mol)";
				plot1d_view * plot = AddPlot1DView(PLOT_USERDATA_STRUCTURE, s1, sv, true);
			
				float ekin;
				float epot;


				i32s max_frames = prj->GetTotalFrames();
				for (i32s loop = 0;loop < max_frames;loop++)
				{
					prj->SetCurrentFrame(loop);
					//prj->ReadFrame();
					//void project::ReadFrame(void)
					{
						i32s place = 8 + 2 * sizeof(int);						// skip the header...
						place += (2 + 3 * traj_num_atoms) * sizeof(float) * current_traj_frame;		// get the correct frame...
						//place += 2 * sizeof(float);							// skip epot and ekin...
						
						trajfile->seekg(place, ios::beg);

						trajfile->read((char *) & ekin, sizeof(ekin));
						trajfile->read((char *) & epot, sizeof(epot));
						
						for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
						{
						//	if ((* it1).flags & ATOMFLAG_IS_HIDDEN) continue;	// currently all coordinates are written...
							
							fGL cdata[3];
							for (i32s t4 = 0;t4 < 3;t4++)
							{
								float t1a; trajfile->read((char *) & t1a, sizeof(t1a));
								cdata[t4] = t1a;
							}
							
							(* it1).SetCRD(0, cdata[0], cdata[1], cdata[2]);
						}
						//this->UpdateAllGraphicsViews();
					}

					//ref->prj->UpdateAllGraphicsViews(true);
					//::Sleep(100);
					void * udata = convert_cset_to_plotting_udata(this, 0);
					f64 value = do_potencial ? epot : ekin;
					plot->AddData(loop, value, udata);
				}
				
				prj->CloseTrajectory();

//				static trajview_dialog * tvd = NULL;
				
//				if (tvd != NULL) delete tvd;		// how to safely release the memory...
				//tvd =
				//	new trajview_dialog(prj);		// ...right after the dialog is closed?
				
				// the dialog will call prj->CloseTrajectory() itself when closed!!!
				// the dialog will call prj->CloseTrajectory() itself when closed!!!
				// the dialog will call prj->CloseTrajectory() itself when closed!!!
				plot->SetCenterAndScale();
				plot->Update();
			}
		}
		else prj->ErrorMessage("Trajectory already open?!?!?!");
	}
}


void win_project::popup_TrajView(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)// new trajfile_dialog(prj);	// will call delete itself...
	{	
		if (!prj->GetTrajectoryFile())
		{
			char filename[512];
			DWORD nFilterIndex;
			if (OpenFileDlg(widget, "Ghemical Trajectory File (*.traj)\0*.traj\0All files \0*.*\0", filename, nFilterIndex) == S_OK)
			{
				cout << "trying to open \"" << filename << "\"." << endl;
				prj->OpenTrajectory(filename);
				// check if there were problems with OpenTrajectory()?!?!?!
				// check if there were problems with OpenTrajectory()?!?!?!
				// check if there were problems with OpenTrajectory()?!?!?!
				
//				static trajview_dialog * tvd = NULL;
				
//				if (tvd != NULL) delete tvd;		// how to safely release the memory...
				//tvd =
					new trajview_dialog(prj);		// ...right after the dialog is closed?
				
				// the dialog will call prj->CloseTrajectory() itself when closed!!!
				// the dialog will call prj->CloseTrajectory() itself when closed!!!
				// the dialog will call prj->CloseTrajectory() itself when closed!!!
			}
		}
		else prj->ErrorMessage("Trajectory already open?!?!?!");
	}
}

void win_project::popup_ConCatTrajFiles(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)// new trajfile_dialog(prj);	// will call delete itself...
	{	
		if (!prj->GetTrajectoryFile())
		{
			// handle the file selection...
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			TCHAR filter[] =     TEXT("Ghemical Trajectory File (*.traj)\0*.traj\0")
								TEXT("All Files (*.*)\0*.*\0");
			TCHAR outfilename[1024];

			sprintf(outfilename, "\0");
			DWORD nFilterIndex;
			if (SaveFileDlg(0, outfilename, filter, nFilterIndex) == S_OK)
			{
				ofstream ofile;
				//ofile.open(param.filename, ios::out | ios::binary);
				ofile.open(outfilename, ios_base::out | ios_base::trunc | ios_base::binary);
				
				int number_of_atoms = prj->GetAtomCount();
				const char file_id[10] = "traj_v10";
				
				int total_frames = 0;
				
				ofile.write((char *) file_id, 8);					// file id, 8 chars.
				ofile.write((char *) & number_of_atoms, sizeof(number_of_atoms));	// number of atoms, int.
				ofile.write((char *) & total_frames, sizeof(total_frames));		// total number of frames, int.


				char filename[512];
				DWORD nFilterIndex;
				while (OpenFileDlg(widget, "Ghemical Trajectory File (*.traj)\0*.traj\0All files \0*.*\0", filename, nFilterIndex) == S_OK)
				{			

					cout << "trying to open \"" << filename << "\"." << endl;
					prj->OpenTrajectory(filename);	

					// CalcTotalFrames
					size_t start_pos = prj->trajfile->tellg();
					prj->trajfile->seekg(0, std::ios::end);
					size_t end_pos = prj->trajfile->tellg();
					prj->trajfile->seekg(start_pos, ios::beg);
					size_t traj_body_size = end_pos - start_pos;
				
					i32s max_frames = prj->GetTotalFrames();
					size_t real_frames = traj_body_size / prj->GetTrajectoryFrameSize();



					float ekin;
					float epot;

					total_frames += real_frames;
					for (i32s loop = 0;loop < real_frames;loop++)
					{
						//prj->SetCurrentFrame(loop);//current_traj_frame = loop;
						//prj->ReadFrame();
						//void project::ReadFrame(void)
						{
							i32s place = prj->GetTrajectoryHeaderSize();						// skip the header...
							place += prj->GetTrajectoryFrameSize() * loop;		// get the correct frame...
							//place += prj->GetTrajectoryEnergySize();							// skip epot and ekin...
							
							prj->trajfile->seekg(place, ios::beg);

							prj->trajfile->read((char *) & ekin, sizeof(ekin));
							prj->trajfile->read((char *) & epot, sizeof(epot));
							
							ofile.write((char *) & ekin, sizeof(ekin));	// kinetic energy, float.
							ofile.write((char *) & epot, sizeof(epot));	// potential energy, float.
							
							for (iter_al it1 = prj->atom_list.begin();it1 != prj->atom_list.end();it1++)
							{								
								//fGL cdata[3];
								for (i32s t4 = 0;t4 < 3;t4++)
								{
									float t1a; 
									prj->trajfile->read((char *) & t1a, sizeof(t1a));
									//cdata[t4] = t1a;
									//float t1a = cdata[t4];
									ofile.write((char *) & t1a, sizeof(t1a));
								}
							}
						}
					}	

					prj->CloseTrajectory();
				}
				ofile.close();
				prj->TrajectorySetTotalFrames(outfilename, total_frames);
			}	
		}
		else prj->ErrorMessage("Trajectory already open?!?!?!");
	}
}

void win_project::popup_SetOrbital(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		prj->Message("PLEASE NOTE!\nThe command string, which is displayed in the next dialog, is incomplete.\nYou should replace the letter X with the orbital index that will become the current orbital.");
		
		static const char command[] = "set_current_orbital X";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_HAdd(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		prj->AddH();
		prj->UpdateAllGraphicsViews();
	}
}

void win_project::popup_HRemove(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		prj->RemoveH();
		prj->UpdateAllGraphicsViews();
	}
}

void win_project::popup_NematicBox(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		static char command[256];
		sprintf(command, "nematic_box %f %f %f mmba", 
			prj->periodic_box_HALFdim[0],
			prj->periodic_box_HALFdim[1],
			prj->periodic_box_HALFdim[2]);
		sprintf(command, "nematic_box 4.0 2.0 0.5 mmba");
		//= "solvate_box 3.0 3.0 3.0";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_SolvateBox(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		static char command[256];
		sprintf(command, "solvate_box %f %f %f 0.8 2", 
			prj->periodic_box_HALFdim[0],
			prj->periodic_box_HALFdim[1],
			prj->periodic_box_HALFdim[2]);
		//= "solvate_box 3.0 3.0 3.0";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_SolvateSphere(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		static const char command[] = "solvate_sphere 1.2 1.6";
		new command_dialog(prj, gv, command);
	}
}
void win_project::popup_Clear(HWND widget, void* data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj && 
	    prj->Question("Are you sure you want to clear everything?"))
	{
	  prj->ClearModel();
	  prj->UpdateAllGraphicsViews();
	}
}

#if 0
void win_project::popup_Library(GtkWidget *item, void * data)
{
  project * prj1 = (project *) win_object_get_user_data(GTK_OBJECT(item));
  win_project * prj = dynamic_cast<win_project *>(prj1);
  
  static char buffer[256];
  g_snprintf(buffer, 256, "%s", data);
  cout << "preparing to open a library file : " << buffer << endl;	// just for debugging...
  
  bool open = prj->IsEmpty();	// open, not insert
  
  ifstream ifile;
  ifile.open(buffer, ios::in);
  ReadGPR(* prj, ifile, true);
  
  // This should probably go into the prjiew class 
  // that is, the prjiew::ReadGPR() should override 
  // the model::ReadGPR() and the rest
  	
  if (!open)	// we append this file to the current project
    {
      if (prj->selected_object != NULL)
	{
	  prj->selected_object = NULL;
	}
    }
	
  ifile.close();
}
#endif
void win_project::popup_BuilderAmino(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
prj->Message("PLEASE NOTE!\nThe command string, which is displayed in the next dialog, is incomplete.\nYou should replace the default sequence AAA with the sequence to be built.");
		
		static const char command[] = "build_amino AAA";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_BuilderNucleic(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
prj->Message("PLEASE NOTE!\nThe command string, which is displayed in the next dialog, is incomplete.\nYou should replace the default sequence AGTCaguc with the sequence to be built.");
		
		static const char command[] = "build_nucleic AGTCaguc";
		new command_dialog(prj, gv, command);
	}
}

void win_project::popup_Center(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		prj->CenterCRDSet(0, true);
		
		// Which is the current Coord Set?
		// usually it's 0, but it could be any/all. the crdset-system is not yet in GUI...
		
		// 2001-06-18 TH: yes, this is not quite ready. but i guess in the end we will
		// move ALL crd-sets...
		
// sometimes this centering won't work, if the camera won't point to the origo.
// so, here we try to turn it there. ANY EFFECTS TO LIGHTS, ETC??????

	//	for (i32u n1 = 0;n1 < prj->camera_vector.size();n1++)
	//	{
			// rotation or translation???
			// translation would affect less to lights (since it's accurate)????
	//	}
		
		prj->UpdateAllGraphicsViews();
	}
}

void win_project::popup_EnterCommand(HWND widget, void * data)
{
	win_graphics_view * gv = win_graphics_view::GetGV(widget);
	win_project * prj = dynamic_cast<win_project *>(gv->prj);
	if (prj)
	{
		new command_dialog(prj, gv, NULL);
	}
}

/*################################################################################################*/

// eof
