// Win32_PROJECT.H : classes for gtk/bonobo user interface.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef WIN_PROJECT_H
#define WIN_PROJECT_H

#include "appconfig.h"

class win_project;

/*################################################################################################*/

#include "../libghemical/src/model.h"

#include "project.h"
#include "views.h"

#include "win32_views.h"
//#include "gtk_dialog.h"
//#include "glade/progress_dialog.h"

#include <windows.h>

#ifdef ENABLE_BONOBO
#include <bonobo.h>
#endif	// ENABLE_BONOBO

#include <vector>
using namespace std;

#define FILENAME_FILTER	"*.gpr"

/*################################################################################################*/

/**	This will contain gtk-dependent aspects of the "##project" class.

	BONOBO-docs???
*/

class win_project : public project
{
	win_app * app;
	public:
	win_app * GetApp(){return app;}
	void SetApp(win_app * p){app = p;}
//	win_project::win_project(win_class_factory & p2) : project(p2);
	
	win_project(win_class_factory &);
	virtual ~win_project(void);

#ifdef ENABLE_BONOBO
	BonoboControl * control;
#endif	// ENABLE_BONOBO
	HWND paned_widget;
	HWND GetWidget(){return paned_widget;}
	
//	GtkWidget * notebook_widget;
	
//	GtkTextBuffer * txt_buffer;
//	GtkTextMark * end_mark;
	
//	GtkWidget * scroll_widget;
//	GtkWidget * txt_widget;
	
	readpdb_mdata * importpdb_mdata;		// temporary?!?!?!
	
	friend class win_ogl_view;
	friend class win_view;
	
	public:
	
	friend class win_project_view;
	friend class win_graphics_view;
friend class win_plot1d_view;
friend class win_plot2d_view;
friend class win_rcp_view;
friend class win_eld_view;
	
	friend class win_class_factory;
	
	friend class file_open_dialog;
	friend class file_save_dialog;
	
	friend class importpdb_dialog;		// importpdb_mdata
	public:
//	win_project::win_project(win_class_factory & p2) : project(p2);
	
//	win_project(win_class_factory &);
//	virtual ~win_project(void);
#ifdef ENABLE_BONOBO
	BonoboControl * GetControl(void);
#endif	// ENABLE_BONOBO
	
	
	void ThreadLock(void);			// virtual
	void ThreadUnlock(void);		// virtual
	bool SetProgress(double, double*);	// virtual
	
#ifdef ENABLE_THREADS
	
	protected:
	progress_dialog * pd;
	
	public:

	
	void CreateProgressDialog(const char *, bool, int, int);	// virtual
	void DestroyProgressDialog(void);				// virtual
	
#endif	// ENABLE_THREADS
	
	void Message(const char *);		// virtual
	void WarningMessage(const char *);	// virtual
	void ErrorMessage(const char *);	// virtual
	bool Question(const char *);		// virtual
	
	void PrintToLog(const char *);			// virtual
	void UpdateAllWindowTitles(void);		// virtual
/*	
	void MakeLibraryWidgets(GtkMenuItem *, const char *, int);
	*/
	void GeomOptGetParam(geomopt_param &);	// virtual
	void MolDynGetParam(moldyn_param &);	// virtual
	
	// the popup-menu callbacks are here...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	static void popup_FileLoadBox(HWND, void*);
	static void popup_FileLoadSelect(HWND, void*);
	static void popup_FileOpen(HWND, void*);
	static void popup_FileClose(HWND, void*);
	static void popup_FileImport(HWND, void*);
	static void popup_FileExport(HWND, void*);
	static void popup_FileExportGraphic(HWND, void*);
	static void popup_FileSaveBox(HWND, void*);
	static void popup_FileSaveSelected(HWND, void*);
	static void popup_FileSaveAs(HWND, void*);
	static void popup_FileExtra1(HWND, void*);
	static void popup_FileExtra2(HWND, void*);
	
	static void popup_SelectAll(HWND, void*);
	static void popup_SelectNone(HWND, void*);
	static void popup_InvertSelection(HWND, void*);
	static void popup_DeleteSelection(HWND, void*);
	
	static void popup_SelectModeAtom(HWND, void* data);
	static void popup_SelectModeResidue(HWND, void* data);
	static void popup_SelectModeChain(HWND, void* data);
	static void popup_SelectModeMolecule(HWND, void* data);
	
	static void popup_ViewsNewELD(HWND, void*);
	
	static void popup_RModeBallStick(HWND, void*);
	static void popup_RModeVanDerWaals(HWND, void*);
	static void popup_RModeCylinders(HWND, void*);
	static void popup_RModeWireframe(HWND, void*);
	static void popup_RModeNothing(HWND, void*);
	
	static void popup_CModeElement(HWND, void*);
	static void popup_CModeSecStruct(HWND, void*);
	static void popup_CModeHydPhob(HWND, void*);
	
	static void popup_LModeIndex(HWND, void*);
	static void popup_LModeElement(HWND, void*);
	static void popup_LModeFCharge(HWND, void*);
	static void popup_LModePCharge(HWND, void*);
	static void popup_LModeAtomType(HWND, void*);
	static void popup_LModeBuilderID(HWND, void*);
	static void popup_LModeBondType(HWND, void*);
	static void popup_LModeResidue(HWND, void*);
	static void popup_LModeSecStruct(HWND, void*);
	static void popup_LModeNothing(HWND, void*);
	
	static void popup_ObjRibbon(HWND, void*);
	static void popup_ObjEPlane(HWND, void*);
	static void popup_ObjEVolume(HWND, void*);
	static void popup_ObjESurface(HWND, void*);
	static void popup_ObjEVDWSurface(HWND, void*);
	static void popup_ObjEDPlane(HWND, void*);
	static void popup_ObjEDSurface(HWND, void*);
	static void popup_ObjMOPlane(HWND, void*);
	static void popup_ObjMOVolume(HWND, void*);
	static void popup_ObjMOSurface(HWND, void*);
	static void popup_ObjMODPlane(HWND, void*);
	static void popup_ObjMODVolume(HWND, void*);
	static void popup_ObjMODSurface(HWND, void*);
	
	static void popup_ObjectsDeleteCurrent(HWND, void*);
	
	static void popup_CompSetup(HWND, void*);
	static void popup_CompEnergy(HWND, void*);
	static void popup_CompGeomOpt(HWND, void*);
	static void popup_CompMolDyn(HWND, void*);

#if PROBNIY_ATOM_GEOMOPT
	void popup_Comp_work_prob_atom_GeomOpt(char * infile_name,	char * trgtlst_name, char * box_name, char * fixed_name);
#endif
	static void popup_CompRandomSearch(HWND, void*);
	static void popup_CompSystematicSearch(HWND, void*);
	static void popup_CompMonteCarloSearch(HWND, void*);

	static void popup_CompDistanceTrajPlot1D(HWND widget, void * data);
	static void popup_CompAngleTrajPlot1D(HWND widget, void * data);
	static void popup_TrajSetTotalFrames(HWND widget, void * data);


	static void popup_CompAngleEnergyPlot1D(HWND, void*);
	static void popup_CompTorsionEnergyPlot1D(HWND, void*);
	static void popup_CompTorsionEnergyPlot2D(HWND, void*);
	static void popup_CompPopAnaElectrostatic(HWND, void*);
	static void popup_CompTransitionStateSearch(HWND, void*);
	static void popup_CompStationaryStateSearch(HWND, void*);
	static void popup_CompFormula(HWND, void*);

static void popup_CompSetFormalCharge(HWND, void*);
static void popup_CompCreateRS(HWND, void*);
static void popup_CompCycleRS(HWND, void*);

	void popup_TrajView_EnergyPlot(HWND widget, void * data, bool do_potencial);

	static void popup_TrajView(HWND, void*);
	static void popup_SetOrbital(HWND, void*);	
	static void popup_HAdd(HWND, void*);
	static void popup_HRemove(HWND, void*);
	static void popup_SolvateBox(HWND, void*);
	static void popup_SolvateSphere(HWND, void*);
	static void popup_Clear(HWND, void*);
	//static void popup_Library(HWND, void*);	
	static void popup_BuilderAmino(HWND, void*);
	static void popup_BuilderNucleic(HWND, void*);
	static void popup_Center(HWND, void*);
	static void popup_EnterCommand(HWND, void*);
};

/*################################################################################################*/

#endif	// GTK_PROJECT_H

// eof
