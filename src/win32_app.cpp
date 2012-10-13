// MAIN.CPP

// Copyright (C) 2003 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"
#include "win32_app.h"
#include "../consol/winghemical.h"

#include "../libghemical/src/notice.h"

//#include "glade/setup_dialog.h"

#include "../libghemical/src/utility.h"

#include <string.h>
//#include <X11/Xlib.h>	// DisplayString()

#include "pixmaps/draw.xpm"
#include "pixmaps/erase.xpm"
#include "pixmaps/select.xpm"
#include "pixmaps/zoom.xpm"
#include "pixmaps/clipping.xpm"
#include "pixmaps/transl_xy.xpm"
#include "pixmaps/transl_z.xpm"
#include "pixmaps/orbit_xy.xpm"
#include "pixmaps/orbit_z.xpm"
#include "pixmaps/rotate_xy.xpm"
#include "pixmaps/rotate_z.xpm"
#include "pixmaps/measure.xpm"

#include "pixmaps/element.xpm"
#include "pixmaps/bondtype.xpm"
#include "pixmaps/setup.xpm"

#include <strstream>
using namespace std;

/*################################################################################################*/

// this part is related to the GTK+ application.
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

win_app * win_app::instance = NULL;
//HWND win_app::m_hMainWnd = NULL;
win_project * win_app::prj = NULL;
/*
GtkActionEntry win_app::entries1[] =
{
	{ "FileMenu", NULL, "_File" },
	{ "HelpMenu", NULL, "_Help" },
	
	{ "New", GTK_STOCK_NEW, "_New", "<control>N", "New file", (GCallback) gtk_app::mainmenu_FileNew },
	{ "Open", GTK_STOCK_OPEN, "_Open", "<control>O", "Open a file", (GCallback) gtk_app::mainmenu_FileOpen },
	{ "Close", GTK_STOCK_QUIT, "_Close", "<control>C", "Quit the program", (GCallback) gtk_app::mainmenu_FileClose },
	
	{ "Help", NULL, "_Help", "<control>H", "View the User's Manual", (GCallback) gtk_app::mainmenu_HelpHelp },
	{ "About", NULL, "_About", "<control>A", "View the copyright notices", (GCallback) gtk_app::mainmenu_HelpAbout },
	
	// the rest are toolbar actions...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	{ "Element", "MY_ELEMENT_ICON", "el", NULL, "Set the current element", (GCallback) gtk_app::maintb_dial_Element },
	{ "BondType", "MY_BONDTYPE_ICON", "bt", NULL, "Set the current bondtype", (GCallback) gtk_app::maintb_dial_BondType },
	{ "Setup", "MY_SETUP_ICON", "su", NULL, "Setup or change the comp.chem. method in use", (GCallback) gtk_app::maintb_dial_Setup }
};

GtkToggleActionEntry gtk_app::entries2[] =
{
	{ "Draw", "MY_DRAW_ICON", "d", NULL, "Draw ; add atoms and bonds to the model.", (GCallback) gtk_app::maintb_tool_Draw, FALSE },
	{ "Erase", "MY_ERASE_ICON", "e", NULL, "Erase ; remove atoms and bonds from the model.", (GCallback) gtk_app::maintb_tool_Erase, FALSE },
	{ "Select", "MY_SELECT_ICON", "s", NULL, "Select ; make selections in the model, and also select objects.", (GCallback) gtk_app::maintb_tool_Select, FALSE },
	{ "Zoom", "MY_ZOOM_ICON", "z", NULL, "Zoom ; zoom the view.", (GCallback) gtk_app::maintb_tool_Zoom, FALSE },
	{ "Clip", "MY_CLIPPING_ICON", "c", NULL, "Clipping ; set the near and far clipping planes for graphics rendering.", (GCallback) gtk_app::maintb_tool_Clipping, FALSE },
	{ "TransXY", "MY_TRANSL_XY_ICON", "t", NULL, "TranslateXY ; translate the camera or a selected object in XY-direction.", (GCallback) gtk_app::maintb_tool_TranslXY, FALSE },
	{ "TransZ", "MY_TRANSL_Z_ICON", "tz", NULL, "TranslateZ ; translate the camera or a selected object in Z-direction.", (GCallback) gtk_app::maintb_tool_TranslZ, FALSE },
	{ "OrbXY", "MY_ORBIT_XY_ICON", "o", NULL, "OrbitXY ; orbit the camera or a selected object around the focus point in XY-direction.", (GCallback) gtk_app::maintb_tool_OrbitXY, TRUE },
	{ "OrbZ", "MY_ORBIT_Z_ICON", "oz", NULL, "OrbitZ ; orbit the camera or a selected object around the focus point in Z-direction.", (GCallback) gtk_app::maintb_tool_OrbitZ, FALSE },
	{ "RotXY", "MY_ROTATE_XY_ICON", "r", NULL, "RotateXY ; turn the camera or a selected object in XY-direction.", (GCallback) gtk_app::maintb_tool_RotateXY, FALSE },
	{ "RotZ", "MY_ROTATE_Z_ICON", "rz", NULL, "RotateZ ; turn the camera or a selected object in Z-direction.", (GCallback) gtk_app::maintb_tool_RotateZ, FALSE },
	{ "Measure", "MY_MEASURE_ICON", "m", NULL, "Measure ; measure distances, angles and torsions.", (GCallback) gtk_app::maintb_tool_Measure, FALSE },
};
*/
const char * win_app::ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='New'/>"
"      <menuitem action='Open'/>"
"      <separator/>"
"      <menuitem action='Close'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Help'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"  <toolbar name='MainTB'>"
"    <placeholder name='MainTools'>"
"      <separator/>"
"      <toolitem name='dr' action='Draw'/>"
"      <toolitem name='er' action='Erase'/>"
"      <toolitem name='se' action='Select'/>"
"      <toolitem name='zm' action='Zoom'/>"
"      <toolitem name='cp' action='Clip'/>"
"      <toolitem name='tt' action='TransXY'/>"
"      <toolitem name='tz' action='TransZ'/>"
"      <toolitem name='oo' action='OrbXY'/>"
"      <toolitem name='oz' action='OrbZ'/>"
"      <toolitem name='rr' action='RotXY'/>"
"      <toolitem name='rz' action='RotZ'/>"
"      <toolitem name='ms' action='Measure'/>"
"      <separator/>"
"      <toolitem name='el' action='Element'/>"
"      <toolitem name='bt' action='BondType'/>"
"      <separator/>"
"      <toolitem name='su' action='Setup'/>"
"      <separator/>"
"    </placeholder>"
"  </toolbar>"
"</ui>";
/*
GtkUIManager * gtk_app::ui_manager = NULL;

GtkWidget * gtk_app::main_window = NULL;
GtkWidget * gtk_app::main_vbox = NULL;

GtkWidget * gtk_app::main_menubar = NULL;
GtkWidget * gtk_app::main_toolbar = NULL;

GtkWidget * gtk_app::mtb_mtool_draw = NULL;
GtkWidget * gtk_app::mtb_mtool_erase = NULL;
GtkWidget * gtk_app::mtb_mtool_select = NULL;
GtkWidget * gtk_app::mtb_mtool_zoom = NULL;
GtkWidget * gtk_app::mtb_mtool_clipping = NULL;
GtkWidget * gtk_app::mtb_mtool_transl_xy = NULL;
GtkWidget * gtk_app::mtb_mtool_transl_z = NULL;
GtkWidget * gtk_app::mtb_mtool_orbit_xy = NULL;
GtkWidget * gtk_app::mtb_mtool_orbit_z = NULL;
GtkWidget * gtk_app::mtb_mtool_rotate_xy = NULL;
GtkWidget * gtk_app::mtb_mtool_rotate_z = NULL;
GtkWidget * gtk_app::mtb_mtool_measure = NULL;
//GtkWidget * gtk_app::mtb_dial_element = NULL;
//GtkWidget * gtk_app::mtb_dial_bondtype = NULL;
//GtkWidget * gtk_app::mtb_dial_setup = NULL;
*/
win_app::win_app(void)
{
//printf("win_app::win_app(void)\n");
	if (instance != NULL)	// allow only a single initialization...
	{
		cout << "win_app::instance != NULL" << endl;
		exit(EXIT_FAILURE);
	}
	
	instance = this;
//printf("win_app::register some new stock icons\n")	;
	// register some new stock icons...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
//	GtkIconFactory * icon_factory = gtk_icon_factory_new();
//	GtkIconSet * icon_set; GdkPixbuf * pixbuf; GtkIconSource * icon_source;
	const int num_icons = 12 + 3;
	
	const char ** icondata[num_icons] =
	{
		(const char **) draw_xpm,
		(const char **) erase_xpm,
		(const char **) select_xpm,
		(const char **) zoom_xpm,
		(const char **) clipping_xpm,
		(const char **) transl_xy_xpm,
		(const char **) transl_z_xpm,
		(const char **) orbit_xy_xpm,
		(const char **) orbit_z_xpm,
		(const char **) rotate_xy_xpm,
		(const char **) rotate_z_xpm,
		(const char **) measure_xpm,
		
		(const char **) element_xpm,
		(const char **) bondtype_xpm,
		(const char **) setup_xpm
	};
	
	const char * icon_id[num_icons] =
	{
		"MY_DRAW_ICON",
		"MY_ERASE_ICON",
		"MY_SELECT_ICON",
		"MY_ZOOM_ICON",
		"MY_CLIPPING_ICON",
		"MY_TRANSL_XY_ICON",
		"MY_TRANSL_Z_ICON",
		"MY_ORBIT_XY_ICON",
		"MY_ORBIT_Z_ICON",
		"MY_ROTATE_XY_ICON",
		"MY_ROTATE_Z_ICON",
		"MY_MEASURE_ICON",
		
		"MY_ELEMENT_ICON",
		"MY_BONDTYPE_ICON",
		"MY_SETUP_ICON"
	};
/*	
	for (int ii = 0;ii < num_icons;ii++)
	{
		pixbuf = gdk_pixbuf_new_from_xpm_data(icondata[ii]);
		icon_set = gtk_icon_set_new_from_pixbuf(pixbuf);
		
		icon_source = gtk_icon_source_new();
		gtk_icon_source_set_pixbuf(icon_source, pixbuf);
		gtk_icon_set_add_source(icon_set, icon_source);
		gtk_icon_source_free (icon_source);
		
		gtk_icon_factory_add(icon_factory, icon_id[ii], icon_set);
		gtk_icon_set_unref (icon_set);
	}
	
	gtk_icon_factory_add_default(icon_factory);
*/	
//	printf("win_app::create the main window\n");
	// create the main window...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^
//	BeginWinGhemical();
//	m_hMainWnd = CreateWinGhemical(this);

/*	DWORD dwChildID;
	hWinThread = ::CreateThread(NULL, 0, 
		StartWinGhemical, 
		reinterpret_cast<LPVOID>(NULL), 
		0,
		&dwChildID );*/

/*	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(main_window), 1);
	
	g_signal_connect(G_OBJECT(main_window), "delete_event", G_CALLBACK(gtk_app::DeleteEventHandler), NULL);
	g_signal_connect(G_OBJECT(main_window), "destroy", G_CALLBACK(gtk_app::DestroyHandler), NULL);
	
	main_vbox = gtk_vbox_new(FALSE, 1);
	gtk_container_set_border_width(GTK_CONTAINER(main_vbox), 1);
	gtk_container_add(GTK_CONTAINER(main_window), main_vbox);
	
	// add the user interface elements...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	GtkActionGroup * action_group1 = gtk_action_group_new("MainWindowActions");
	gtk_action_group_add_actions(action_group1, entries1, G_N_ELEMENTS(entries1), NULL);
	
	GtkActionGroup * action_group2 = gtk_action_group_new("MouseToolToggleActions");
	gtk_action_group_add_toggle_actions(action_group2, entries2, G_N_ELEMENTS(entries2), NULL);
	
	ui_manager = gtk_ui_manager_new();
	gtk_ui_manager_insert_action_group(ui_manager, action_group1, 0);
	gtk_ui_manager_insert_action_group(ui_manager, action_group2, 0);
	
	GError * error = NULL;
	if (!gtk_ui_manager_add_ui_from_string(ui_manager, ui_description, -1, & error))
	{
		g_message("Building main menu failed : %s", error->message);
		g_error_free(error); exit(EXIT_FAILURE);
	}
	
	main_menubar = gtk_ui_manager_get_widget(ui_manager, "/MainMenu");
	gtk_container_add(GTK_CONTAINER(main_vbox), main_menubar);

	main_toolbar = gtk_ui_manager_get_widget(ui_manager, "/MainTB");
	gtk_container_add(GTK_CONTAINER(main_vbox), main_toolbar);
	
mtb_mtool_draw = gtk_ui_manager_get_widget(ui_manager, "/MainTB/MainTools/dr");
mtb_mtool_erase = gtk_ui_manager_get_widget(ui_manager, "/MainTB/MainTools/er");
mtb_mtool_select = gtk_ui_manager_get_widget(ui_manager, "/MainTB/MainTools/se");
mtb_mtool_zoom = gtk_ui_manager_get_widget(ui_manager, "/MainTB/MainTools/zm");
mtb_mtool_clipping = gtk_ui_manager_get_widget(ui_manager, "/MainTB/MainTools/cp");
mtb_mtool_transl_xy = gtk_ui_manager_get_widget(ui_manager, "/MainTB/MainTools/tt");
mtb_mtool_transl_z = gtk_ui_manager_get_widget(ui_manager, "/MainTB/MainTools/tz");
mtb_mtool_orbit_xy = gtk_ui_manager_get_widget(ui_manager, "/MainTB/MainTools/oo");
mtb_mtool_orbit_z = gtk_ui_manager_get_widget(ui_manager, "/MainTB/MainTools/oz");
mtb_mtool_rotate_xy = gtk_ui_manager_get_widget(ui_manager, "/MainTB/MainTools/rr");
mtb_mtool_rotate_z = gtk_ui_manager_get_widget(ui_manager, "/MainTB/MainTools/rz");
mtb_mtool_measure = gtk_ui_manager_get_widget(ui_manager, "/MainTB/MainTools/ms");
*/	
	
//printf("win_app::set a new default project\n\n");	
	// set a new default project.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	SetNewProject();
	
	// show the widgets and enter in the main loop.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/*	
	gtk_widget_show(main_menubar);
	gtk_widget_show(main_toolbar);
	
	gtk_widget_show(main_vbox);
	gtk_widget_show(main_window);*/

}

win_app::~win_app(void)
{
}

win_app * win_app::GetInstance(void)
{
//printf("win_app * win_app::GetInstance(void)\n\n");
	if (instance != NULL) return instance;
	else return (instance = new win_app());
}
/*
gboolean gtk_app::DeleteEventHandler(GtkWidget * widget, GdkEvent * event, gpointer data)
{
	bool quit = prj->Question("Are you sure that you\nwant to quit the program?");
	if (quit) return FALSE; else return TRUE;
}

void gtk_app::DestroyHandler(GtkWidget * widget, gpointer data)
{
	delete prj;
	prj = NULL;
	
	gtk_main_quit();
}
*/
void win_app::SetNewProject(void)
{
//printf("win_app::SetNewProject(void)\n\n");
	if (prj != NULL)
	{
//		gtk_container_remove(GTK_CONTAINER(main_vbox), prj->GetWidget());
		
		delete prj;
		prj = NULL;
	}
	
	prj = new win_project(* win_class_factory::GetInstance());

//	prj->paned_widget = m_hMainWnd;
//	printf("prj->paned_widget = m_hMainWnd ");
//	cout << prj->paned_widget << "  " << m_hMainWnd << endl << endl;
//	GtkWidget * widget = prj->GetWidget();
	
//	gtk_container_add(GTK_CONTAINER(main_vbox), widget);
}

#include "..\libghemical\src\notice.h"

#include "..\libghemical\src\eng1_mm_tripos52.h"
#include "..\libghemical\src\eng1_mm_default.h"

#include "..\libghemical\src\geomopt.h"
#include "..\libghemical\src\moldyn.h"

void win_app::work(int argc, char* argv[])
{
	// example #2: open a file, run a geometry optimization, and save a file.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	// check that we have got both file names from the command line.
	
	if (argc != 3)
	{
		cout << "usage : example2 infile.gpr outfile.gpr" << endl;
//		StartWinGhemical(NULL);
		return;
	}
	
	char * infile_name = argv[1];
	char * outfile_name = argv[2];
	
	// create a model object, and read in a file.
	
//	model * mdl = new model();
	win_project * mdl = prj;


	
	cout << "trying to open a file " << infile_name << " ; ";
	
	ifstream ifile;
	ifile.open(infile_name, ios::in);
	
	if (ifile.fail())
	{
		cout << "failed!!!" << endl;
		
		ifile.close();
		return;
	}
	
	cout << "ok!!!" << endl;
	ReadGPR(* mdl, ifile, true);	
//	ReadGPR(* mdl, ifile, false, false);
	ifile.close();

	printf(
		"mdl->use_periodic_boundary_conditions = %d\n", 
		mdl->use_periodic_boundary_conditions);

	printf(
		"mdl->periodic_box_HALFdim[0] = %f\n", 
		mdl->periodic_box_HALFdim[0]);
	
	printf(
		"mdl->periodic_box_HALFdim[1] = %f\n", 
		mdl->periodic_box_HALFdim[1]);
	
	printf(
		"mdl->periodic_box_HALFdim[2] = %f\n", 
		mdl->periodic_box_HALFdim[2]);
	
	// if everything went fine, then create a computation engine object, initialize it,
	// and create a geometry optimizer object using the computation engine object.
	
	engine * eng = mdl->GetCurrentSetup()->CreateEngineByIDNumber(CURRENT_ENG1_MM);
//	engine * eng = mdl->GetCurrentSetup()->CreateEngineByIDNumber(ENG1_MM_TRIPOS52);
//	engine * eng = mdl->GetCurrentSetup()->CreateEngineByIDNumber(ENG1_MM_DEFAULT);

	// eng_pbc != NULL if we will use a system with periodic boundary conditions...
#if USE_ENGINE_PBC2
	engine_pbc2 * eng_pbc = dynamic_cast<engine_pbc2 *>(eng);
#else
	engine_pbc * eng_pbc = dynamic_cast<engine_pbc *>(eng);
#endif /*USE_ENGINE_PBC2*/
	
	CopyCRD(mdl, eng, 0);
	CalcMaxMinCoordinates(mdl, eng, 0);

	printf("\n%f %f %f %f %f %f\n",
		mdl->maxCRD[0],
		mdl->minCRD[0],
		mdl->maxCRD[1],
		mdl->minCRD[1],
		mdl->maxCRD[2],
		mdl->minCRD[2]);

	mdl->periodic_box_HALFdim[0] = (mdl->maxCRD[0] - mdl->minCRD[0]+0.14) / 2.;
	mdl->periodic_box_HALFdim[1] = (mdl->maxCRD[1] - mdl->minCRD[1]+0.12) / 2.;

	mdl->periodic_box_HALFdim[0] = (mdl->maxCRD[0] - mdl->minCRD[0]) / 2. + 0.14;
	mdl->periodic_box_HALFdim[1] = (mdl->maxCRD[1] - mdl->minCRD[1]) / 2. + 0.12;

	printf(
		"mdl->use_periodic_boundary_conditions = %d\n", 
		mdl->use_periodic_boundary_conditions);

	printf(
		"mdl->periodic_box_HALFdim[0] = %f\n", 
		mdl->periodic_box_HALFdim[0]);
	
	printf(
		"mdl->periodic_box_HALFdim[1] = %f\n", 
		mdl->periodic_box_HALFdim[1]);
	
	printf(
		"mdl->periodic_box_HALFdim[2] = %f\n", 
		mdl->periodic_box_HALFdim[2]);
	
	
	geomopt * opt = new geomopt(eng, 100, 0.025, 10.0);
	
	// then take 100 geometry optimization steps, and print out some data about progess.
	
	for (int n1 = 0;n1 < 100;n1++)
	{
		opt->TakeCGStep(conjugate_gradient::Newton2An);
//		eng_pbc->CheckLocations();
		
		cout << "step = " << (n1 + 1) << "   ";
		cout << "energy = " << opt->optval << "   ";
		cout << "step length = " << opt->optstp << endl;
	}

	
	// now copy the optimized coordinates back to the model object, and save the file.
	
	CopyCRD(eng, mdl, 0);
	
	delete opt; opt = NULL;
	delete eng; eng = NULL;
	
	cout << "now saving file " << outfile_name << endl;
	
	ofstream ofile;
	ofile.open(outfile_name, ios::out);
	
	WriteGPR_v100(* mdl, ofile);
	ofile.close();
	
//	StartWinGhemical(NULL);
	// finally release all allocated memory and leave.
	
}
/*
void gtk_app::mainmenu_FileNew(gpointer, guint, GtkWidget *)
{
	SetNewProject();
}

void gtk_app::mainmenu_FileOpen(gpointer, guint, GtkWidget *)
{
	new file_open_dialog(prj);	// will call delete itself...
}

void gtk_app::mainmenu_FileClose(gpointer, guint, GtkWidget *)
{
// gtk_signal_emit_by_name(GTK_OBJECT(main_window), "delete_event");
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	// the above is an "elegant" solution but it segfaults ; why???
	// in below the same stuff is done "manually".
	
	if (!DeleteEventHandler(NULL, NULL, NULL)) gtk_widget_destroy(main_window);
}

void gtk_app::mainmenu_HelpHelp(gpointer, guint, GtkWidget *)
{
	cout << "help not yet implemented!" << endl;
}

void gtk_app::mainmenu_HelpAbout(gpointer, guint, GtkWidget *)
{
	static char about[2048];
	ostrstream about_str(about, sizeof(about));
	
	about_str << "ghemical-" << APPVERSION << " released on " << APPRELEASEDATE << endl;
	about_str << " " << endl;
	about_str << "For more information please visit:" << endl;
	about_str << WEBSITE << endl;
	about_str << " " << endl;
	
	// leave some lines out to keep the dialog size smaller...
	for (i32s n1 = 0;n1 < 16;n1++) about_str << get_copyright_notice_line(n1) << endl;
	
	about_str << " " << endl;
	about_str << "Authors:" << endl;
	about_str << "\t\t" << "Tommi Hassinen" << endl;
	about_str << "\t\t" << "Geoff Hutchison" << endl;
	about_str << "\t\t" << "Mike Cruz" << endl;
	about_str << "\t\t" << "Michael Banck" << endl;
	about_str << "\t\t" << "Christopher Rowley" << endl;
	
	about_str << ends;
	prj->Message(about);
}
*/
static int toggle_event_ignore_counter = 0;
void win_app::HandleToggleButtons(mouse_tool * old_tool)
{
	// a static event counter is set up and used so that a cascade of button events is prevented.
	// the same thing could be done by disabling/enabling the callbacks in GTK, but this is simpler.
	
	bool state = TRUE;
	if (old_tool != ogl_view::current_tool) state = FALSE;
	
	if (old_tool == & ogl_view::tool_draw)
	{
		toggle_event_ignore_counter++;
//		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(mtb_mtool_draw), state);
	}
	
	if (old_tool == & ogl_view::tool_erase)
	{
		toggle_event_ignore_counter++;
//		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(mtb_mtool_erase), state);
	}
	
	if (old_tool == & ogl_view::tool_select)
	{
		toggle_event_ignore_counter++;
//		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(mtb_mtool_select), state);
	}
	
	if (old_tool == & ogl_view::tool_zoom)
	{
		toggle_event_ignore_counter++;
//		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(mtb_mtool_zoom), state);
	}
	
	if (old_tool == & ogl_view::tool_clipping)
	{
		toggle_event_ignore_counter++;
//		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(mtb_mtool_clipping), state);
	}
	
	if (old_tool == & ogl_view::tool_translate_xy)
	{
		toggle_event_ignore_counter++;
//		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(mtb_mtool_transl_xy), state);
	}
	
	if (old_tool == & ogl_view::tool_translate_z)
	{
		toggle_event_ignore_counter++;
//		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(mtb_mtool_transl_z), state);
	}
	
	if (old_tool == & ogl_view::tool_orbit_xy)
	{
		toggle_event_ignore_counter++;
//		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(mtb_mtool_orbit_xy), state);
	}
	
	if (old_tool == & ogl_view::tool_orbit_z)
	{
		toggle_event_ignore_counter++;
//		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(mtb_mtool_orbit_z), state);
	}
	
	if (old_tool == & ogl_view::tool_rotate_xy)
	{
		toggle_event_ignore_counter++;
//		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(mtb_mtool_rotate_xy), state);
	}
	
	if (old_tool == & ogl_view::tool_rotate_z)
	{
		toggle_event_ignore_counter++;
//		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(mtb_mtool_rotate_z), state);
	}
	
	if (old_tool == & ogl_view::tool_measure)
	{
		toggle_event_ignore_counter++;
//		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(mtb_mtool_measure), state);
	}
}

void win_app::maintb_tool_Draw(void * p1, UINT p2, HWND p3)
{
/*	if (toggle_event_ignore_counter > 0)
	{
		cout << "draw skipped ; counter = " << toggle_event_ignore_counter << endl;
		toggle_event_ignore_counter--;
	}
	else
	{*/
		cout << "draw state is " <<  "mtb_mtool_draw" << endl;
		//win_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(mtb_mtool_draw)) <<
		
		mouse_tool * old_tool = ogl_view::current_tool;
		ogl_view::current_tool = & ogl_view::tool_draw;
		HandleToggleButtons(old_tool);
//	}
}

void win_app::maintb_tool_Erase(void * p1, UINT p2, HWND p3)
{
/*	if (toggle_event_ignore_counter > 0)
	{
		cout << "erase skipped ; counter = " << toggle_event_ignore_counter << endl;
		toggle_event_ignore_counter--;
	}
	else
	{*/
		cout << "erase state is mtb_mtool_erase" <<  endl;
//			win_toggle_tool_button_get_active(win_TOGGLE_TOOL_BUTTON(mtb_mtool_erase)) <<
		
		mouse_tool * old_tool = ogl_view::current_tool;
		ogl_view::current_tool = & ogl_view::tool_erase;
		HandleToggleButtons(old_tool);
//	}
}

void win_app::maintb_tool_Select(void * p1, UINT p2, HWND p3)
{
/*	if (toggle_event_ignore_counter > 0)
	{
		cout << "select skipped ; counter = " << toggle_event_ignore_counter << endl;
		toggle_event_ignore_counter--;
	}
	else
	{*/
		cout << "select state is " << endl;
			//win_toggle_tool_button_get_active(win_TOGGLE_TOOL_BUTTON(mtb_mtool_select)) 
		
		mouse_tool * old_tool = ogl_view::current_tool;
		ogl_view::current_tool = & ogl_view::tool_select;
		HandleToggleButtons(old_tool);
//	}
}

void win_app::maintb_tool_Zoom(void * p1, UINT p2, HWND p3)
{
/*	if (toggle_event_ignore_counter > 0)
	{
		cout << "zoom skipped ; counter = " << toggle_event_ignore_counter << endl;
		toggle_event_ignore_counter--;
	}
	else
	{*/
		cout << "zoom state is " << endl;
			//win_toggle_tool_button_get_active(win_TOGGLE_TOOL_BUTTON(mtb_mtool_zoom)) << 
		
		mouse_tool * old_tool = ogl_view::current_tool;
		ogl_view::current_tool = & ogl_view::tool_zoom;
		HandleToggleButtons(old_tool);
//	}
}

void win_app::maintb_tool_Clipping(void * p1, UINT p2, HWND p3)
{
/*	if (toggle_event_ignore_counter > 0)
	{
		cout << "clipping skipped ; counter = " << toggle_event_ignore_counter << endl;
		toggle_event_ignore_counter--;
	}
	else
	{*/
		cout << "clipping state is " << endl;
			//win_toggle_tool_button_get_active(win_TOGGLE_TOOL_BUTTON(mtb_mtool_clipping)) << 
		
		mouse_tool * old_tool = ogl_view::current_tool;
		ogl_view::current_tool = & ogl_view::tool_clipping;
		HandleToggleButtons(old_tool);
//	}
}

void win_app::maintb_tool_TranslXY(void * p1, UINT p2, HWND p3)
{
/*	if (toggle_event_ignore_counter > 0)
	{
		cout << "transl_xy skipped ; counter = " << toggle_event_ignore_counter << endl;
		toggle_event_ignore_counter--;
	}
	else
	{*/
		cout << "transl_xy state is " << endl;
			//win_toggle_tool_button_get_active(win_TOGGLE_TOOL_BUTTON(mtb_mtool_transl_xy)) << 
		
		mouse_tool * old_tool = ogl_view::current_tool;
		ogl_view::current_tool = & ogl_view::tool_translate_xy;
		HandleToggleButtons(old_tool);
//	}
}

void win_app::maintb_tool_TranslZ(void * p1, UINT p2, HWND p3)
{
/*	if (toggle_event_ignore_counter > 0)
	{
		cout << "transl_z skipped ; counter = " << toggle_event_ignore_counter << endl;
		toggle_event_ignore_counter--;
	}
	else
	{*/
		cout << "transl_z state is " << endl;
			//win_toggle_tool_button_get_active(win_TOGGLE_TOOL_BUTTON(mtb_mtool_transl_z)) << 
		
		mouse_tool * old_tool = ogl_view::current_tool;
		ogl_view::current_tool = & ogl_view::tool_translate_z;
		HandleToggleButtons(old_tool);
//	}
}

void win_app::maintb_tool_OrbitXY(void * p1, UINT p2, HWND p3)
{
/*	if (toggle_event_ignore_counter > 0)
	{
		cout << "orbit_xy skipped ; counter = " << toggle_event_ignore_counter << endl;
		toggle_event_ignore_counter--;
	}
	else
	{*/
		cout << "orbit_xy state is " << endl;
			//win_toggle_tool_button_get_active(win_TOGGLE_TOOL_BUTTON(mtb_mtool_orbit_xy)) << 
		
		mouse_tool * old_tool = ogl_view::current_tool;
		ogl_view::current_tool = & ogl_view::tool_orbit_xy;
		HandleToggleButtons(old_tool);
//	}
}

void win_app::maintb_tool_OrbitZ(void * p1, UINT p2, HWND p3)
{
/*	if (toggle_event_ignore_counter > 0)
	{
		cout << "orbit_z skipped ; counter = " << toggle_event_ignore_counter << endl;
		toggle_event_ignore_counter--;
	}
	else
	{*/
		cout << "orbit_z state is " << endl;
			//win_toggle_tool_button_get_active(win_TOGGLE_TOOL_BUTTON(mtb_mtool_orbit_z)) <<
		
		mouse_tool * old_tool = ogl_view::current_tool;
		ogl_view::current_tool = & ogl_view::tool_orbit_z;
		HandleToggleButtons(old_tool);
//	}
}

void win_app::maintb_tool_RotateXY(void * p1, UINT p2, HWND p3)
{
/*	if (toggle_event_ignore_counter > 0)
	{
		cout << "rotate_xy skipped ; counter = " << toggle_event_ignore_counter << endl;
		toggle_event_ignore_counter--;
	}
	else
	{*/
		cout << "rotate_xy state is " << endl;
			//win_toggle_tool_button_get_active(win_TOGGLE_TOOL_BUTTON(mtb_mtool_rotate_xy)) << 
		
		mouse_tool * old_tool = ogl_view::current_tool;
		ogl_view::current_tool = & ogl_view::tool_rotate_xy;
		HandleToggleButtons(old_tool);
//	}
}

void win_app::maintb_tool_RotateZ(void * p1, UINT p2, HWND p3)
{
/*	if (toggle_event_ignore_counter > 0)
	{
		cout << "rotate_z skipped ; counter = " << toggle_event_ignore_counter << endl;
		toggle_event_ignore_counter--;
	}
	else
	{*/
		cout << "rotate_z state is " << endl;
			//win_toggle_tool_button_get_active(win_TOGGLE_TOOL_BUTTON(mtb_mtool_rotate_z)) << 
		
		mouse_tool * old_tool = ogl_view::current_tool;
		ogl_view::current_tool = & ogl_view::tool_rotate_z;
		HandleToggleButtons(old_tool);
//	}
}

void win_app::maintb_tool_Measure(void * p1, UINT p2, HWND p3)
{
/*	if (toggle_event_ignore_counter > 0)
	{
		cout << "measure skipped ; counter = " << toggle_event_ignore_counter << endl;
		toggle_event_ignore_counter--;
	}
	else
	{*/
		cout << "measure state is " << endl;
		//	win_toggle_tool_button_get_active(win_TOGGLE_TOOL_BUTTON(mtb_mtool_measure)) << 
		
		mouse_tool * old_tool = ogl_view::current_tool;
		ogl_view::current_tool = & ogl_view::tool_measure;
		HandleToggleButtons(old_tool);
//	}
}

void win_app::maintb_tool_MolAxis(void * p1, UINT p2, HWND p3)
{
/*	if (toggle_event_ignore_counter > 0)
	{
		cout << "measure skipped ; counter = " << toggle_event_ignore_counter << endl;
		toggle_event_ignore_counter--;
	}
	else
	{*/
		cout << "measure state is " << endl;
		//	win_toggle_tool_button_get_active(win_TOGGLE_TOOL_BUTTON(mtb_mtool_measure)) << 
		
		mouse_tool * old_tool = ogl_view::current_tool;
		ogl_view::current_tool = & ogl_view::tool_molaxis;
		HandleToggleButtons(old_tool);
//	}
}
/*

void win_app::maintb_dial_Element(gpointer, guint, GtkWidget *)
{
	new element_dialog();		// will call delete itself...
}

void win_app::maintb_dial_BondType(gpointer, guint, GtkWidget *)
{
	new bondtype_dialog();		// will call delete itself...
}

void win_app::maintb_dial_Setup(gpointer, guint, GtkWidget *)
{
	new setup_dialog(prj);		// will call delete itself...
}

// this part is related to the bonobo server.
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#ifdef ENABLE_BONOBO

static BonoboObject * control_factory(BonoboGenericFactory *, const char * object_id, void *)
{
	BonoboObject * object = NULL;
	
	g_return_val_if_fail(object_id != NULL, NULL);
	
	if (!strcmp(object_id, "OAFIID:GNOME_Ghemical_Control"))
	{
		win_project * prj = new win_project(* win_class_factory::GetInstance());
		object = BONOBO_OBJECT(prj->GetControl());
	}
	
	return object;
}

#endif	// ENABLE_BONOBO

// the main function.
// ^^^^^^^^^^^^^^^^^^

int main(int argc, char ** argv)
{
	bool bonobo_server_flag = false;
	for (int arg = 1;arg < argc;arg++)
	{
		if (!strcmp(argv[arg], "--bonobo-server")) bonobo_server_flag = true;
	}
	
//	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
//	bind_textdomain_codeset (PACKAGE, "UTF-8");
//	textdomain (PACKAGE);
	
	if (bonobo_server_flag)
	{
	
#ifdef ENABLE_BONOBO

		BONOBO_FACTORY_INIT("ghemical", "0.1", & argc, argv);
		win_gl_init(& argc, & argv);
		
		return bonobo_generic_factory_main ("OAFIID:GNOME_Ghemical_ControlFactory", control_factory, NULL);
		
#else	// ENABLE_BONOBO

		cout << "ghemical was compiled without --enable-bonobo option set ;" << endl;
		cout << "therefore the bonobo-server functionality is not available!" << endl;
		return 0;
		
#endif	// ENABLE_BONOBO

	}
	else
	{
		
#ifdef ENABLE_THREADS
		
		g_thread_init(NULL);
		gdk_threads_init();
		
		gdk_threads_enter();
		
#endif	// ENABLE_THREADS
		
		win_init(& argc, & argv);
		gdk_gl_init(& argc, & argv);
		
		singleton_cleaner<win_app> app_cleaner;
		app_cleaner.SetInstance(win_app::GetInstance());
		
		win_main();
		
#ifdef ENABLE_THREADS
		
		gdk_threads_leave();
		
#endif	// ENABLE_THREADS
		
		return 0;
	}
}
*/
//BONOBO_OAF_FACTORY_MULTI ("OAFIID:GNOME_Ghemical_ControlFactory", "ghemical", "0.1", control_factory, NULL)

/*################################################################################################*/

// eof
