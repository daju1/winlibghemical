// MAIN.H : a bonobo control factory and related stuff for the control(s).

// Copyright (C) 2003 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef WIN_APP_H
#define WIN_APP_H

#include "appconfig.h"

#include "win32_project.h"	// include this first or libintl.h will cause trouble?!? :(
#include <windows.h>

//#include <glib.h>

#ifdef ENABLE_BONOBO
#include <bonobo.h>
#endif	// ENABLE_BONOBO

//#include <unistd.h>

/*################################################################################################*/

// this part is related to the GTK+ application.
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

class win_app
{
	private:
	
	static win_app * instance;
	static win_project * prj;



//	static HWND m_hMainWnd;
	public:
	win_project * GetWinProject(){ return prj;}
	protected:
	
//	static GtkActionEntry entries1[];
//	static GtkToggleActionEntry entries2[];
	static const char * ui_description;
	
/*	static GtkUIManager * ui_manager;
	
	static GtkWidget * main_window;
	static GtkWidget * main_vbox;
	
	static GtkWidget * main_menubar;
	static GtkWidget * main_toolbar;
	
static GtkWidget * mtb_mtool_draw;
static GtkWidget * mtb_mtool_erase;
static GtkWidget * mtb_mtool_select;
static GtkWidget * mtb_mtool_zoom;
static GtkWidget * mtb_mtool_clipping;
static GtkWidget * mtb_mtool_transl_xy;
static GtkWidget * mtb_mtool_transl_z;
static GtkWidget * mtb_mtool_orbit_xy;
static GtkWidget * mtb_mtool_orbit_z;
static GtkWidget * mtb_mtool_rotate_xy;
static GtkWidget * mtb_mtool_rotate_z;
static GtkWidget * mtb_mtool_measure;*/
//static GtkWidget * mtb_dial_element;
//static GtkWidget * mtb_dial_bondtype;
//static GtkWidget * mtb_dial_setup;
	
	friend class win_project;
	friend class file_open_dialog;
	
	private:
	
// the constructor is private so that only a single instance
// of the class can be created (by calling GetInstance()).
	win_app(void);
	
	public:
	
	~win_app(void);
	
	static win_app * GetInstance(void);
	
	protected:
	
//	static gboolean DeleteEventHandler(GtkWidget *, GdkEvent *, gpointer);
//	static void DestroyHandler(GtkWidget *, gpointer);
	
	/*static*/ void SetNewProject(void);
	
	public:
		void work(int argc, char* argv[]);
	
/*	static void mainmenu_FileNew(gpointer, guint, GtkWidget *);
	static void mainmenu_FileOpen(gpointer, guint, GtkWidget *);
	static void mainmenu_FileClose(gpointer, guint, GtkWidget *);

	static void mainmenu_HelpHelp(gpointer, guint, GtkWidget *);
	static void mainmenu_HelpAbout(gpointer, guint, GtkWidget *);
*/	
	static void HandleToggleButtons(mouse_tool *);

	static void maintb_tool_Draw(void * p1, UINT p2, HWND p3);
	static void maintb_tool_Erase(void * p1, UINT p2, HWND p3);
	static void maintb_tool_Select(void * p1, UINT p2, HWND p3);
	static void maintb_tool_Zoom(void * p1, UINT p2, HWND p3);
	static void maintb_tool_Clipping(void * p1, UINT p2, HWND p3);
	static void maintb_tool_TranslXY(void * p1, UINT p2, HWND p3);
	static void maintb_tool_TranslZ(void * p1, UINT p2, HWND p3);
	static void maintb_tool_OrbitXY(void * p1, UINT p2, HWND p3);
	static void maintb_tool_OrbitZ(void * p1, UINT p2, HWND p3);
	static void maintb_tool_RotateXY(void * p1, UINT p2, HWND p3);
	static void maintb_tool_RotateZ(void * p1, UINT p2, HWND p3);
	static void maintb_tool_Measure(void * p1, UINT p2, HWND p3);
	static void maintb_tool_MolAxis(void * p1, UINT p2, HWND p3);
	
/*	static void maintb_dial_Element(gpointer, guint, GtkWidget *);
	static void maintb_dial_BondType(gpointer, guint, GtkWidget *);
	static void maintb_dial_Setup(gpointer, guint, GtkWidget *);
	
	static GtkUIManager * GetUIManager(void) { return ui_manager; }
	static GtkWidget * GetMainWindow(void) { return main_window; }*/
};

// this part is related to the bonobo server.
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#ifdef ENABLE_BONOBO

static BonoboObject * control_factory(BonoboGenericFactory *, const char *, void *);

#endif	// ENABLE_BONOBO

// the main function.
// ^^^^^^^^^^^^^^^^^^

int main(int, char **);

/*################################################################################################*/

#endif	// MAIN_H

// eof
