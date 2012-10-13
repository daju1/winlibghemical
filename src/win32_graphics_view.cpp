// Win32_GRAPHICS_VIEW.CPP

// Copyright (C) 2005 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"
#include "../consol/resource.h"

#include "win32_graphics_view.h"

#include "../libghemical/src/utility.h"
#include "../libghemical/src/libconfig.h"		// ENABLE_OPENBABEL
#include "../consol/winghemical.h"

/*################################################################################################*/
#if 0
GtkActionEntry gtk_graphics_view::entries[] =
{
	{ "AttachDetachGV", NULL, "Attach/Detach This View", NULL, "Select whether this view is attached or in an independent window", (GCallback) gtk_graphics_view::popup_ViewsAttachDetach },
	
	{ "FileMenu", NULL, "_File" },
	// ^^^^^^^^
	{ "Open", GTK_STOCK_OPEN, "_Open", "<control>O", "Open a file", (GCallback) gtk_project::popup_FileOpen },
#ifdef ENABLE_OPENBABEL
	{ "Import", GTK_STOCK_OPEN, "_Import...", "<control>I", "Import other file formats using OpenBabel", (GCallback) gtk_project::popup_FileImport },
#endif	// ENABLE_OPENBABEL
	{ "SaveAs", GTK_STOCK_SAVE, "_Save as...", "<control>S", "Save a file", (GCallback) gtk_project::popup_FileSaveAs },
#ifdef ENABLE_OPENBABEL
	{ "Export", GTK_STOCK_SAVE, "_Export", "<control>E", "Export other file formats using OpenBabel", (GCallback) gtk_project::popup_FileExport },
#endif	// ENABLE_OPENBABEL
	{ "Close", GTK_STOCK_QUIT, "_Close", "<control>C", "Close the file", (GCallback) gtk_app::mainmenu_FileClose },
	
	{ "FileExtraMenu", NULL, "_Extra Tools" },
	// ^^^^^^^^^^^^^
	{ "ImportPDB", NULL, "_Import PDB", NULL, "Import a PDB file", (GCallback) gtk_project::popup_FileExtra1 },
	{ "ConvertAA2SF", NULL, "_Convert AA2SF", NULL, NULL, (GCallback) gtk_project::popup_FileExtra2 },
	
	{ "SelectMenu", NULL, "_Select" },
	// ^^^^^^^^^^
	{ "SelectAll", NULL, "Select _All", NULL, "Select all atoms", (GCallback) gtk_project::popup_SelectAll },
	{ "SelectNone", NULL, "Select _None", NULL, "Select nothing", (GCallback) gtk_project::popup_SelectNone },
	{ "InvertSelection", NULL, "_Invert Selection", NULL, "Invert the selection", (GCallback) gtk_project::popup_InvertSelection },
	{ "DeleteSelected", NULL, "_Delete Selected", NULL, "Delete all selected atoms", (GCallback) gtk_project::popup_DeleteSelection },
	
	{ "SelectModeMenu", NULL, "_Selection Mode" },
	// ^^^^^^^^^^^^^^
// todo : convert these into toggle items so that the setting from project is show in each of the graphics views...
	{ "SelectAtoms", NULL, "Select _Atoms", NULL, "Select single atoms", (GCallback) gtk_project::popup_SelectModeAtom },
	{ "SelectResidues", NULL, "Select _Residues", NULL, "Select residues", (GCallback) gtk_project::popup_SelectModeResidue },
	{ "SelectChains", NULL, "Select _Chains", NULL, "Select chains", (GCallback) gtk_project::popup_SelectModeChain },
	{ "SelectMolecules", NULL, "Select _Molecules", NULL, "Select molecules", (GCallback) gtk_project::popup_SelectModeMolecule },
	
	{ "RenderMenu", NULL, "_Render" },
	// ^^^^^^^^^^
// todo : convert these into toggle items so that the local setting is show in each of the graphics views...
	{ "RenderQuickUpdate", NULL, "Quick Update Mode", NULL, "Quick Update Mode switches into Wireframe rendering mode when doing interactive work", (GCallback) gtk_graphics_view::popup_RenderQuickUpdate },
	
	{ "RendViewsMenu", NULL, "_Views" },
	// ^^^^^^^^^^^^^
	{ "RendViewsDeleteView", NULL, "Delete This View", NULL, "Delete this graphics view", (GCallback) gtk_graphics_view::popup_ViewsDeleteView },
	{ "RendViewsPushCRDSet", NULL, "Save a CRD-set (experimental)", NULL, "Save a coordinate set", (GCallback) gtk_graphics_view::popup_ViewsPushCRDSet },
	{ "RendViewsSuperimpose", NULL, "Superimpose CRD-sets (experimental)", NULL, "Superimpose coordinate sets", (GCallback) gtk_graphics_view::popup_ViewsSuperimpose },
	
	{ "RendViewsCreateMenu", NULL, "_Create" },
	// ^^^^^^^^^^^^^^^^^^^
	{ "RendViewsCreateCam", NULL, "New Camera", NULL, "Create a new view with a new camera", (GCallback) gtk_graphics_view::popup_ViewsNewCam },
	{ "RendViewsCreateView", NULL, "New View", NULL, "Create a new view using the current camera", (GCallback) gtk_graphics_view::popup_ViewsNewView },
	{ "RendViewsCreateELD", NULL, "Energy-level Diagram", NULL, "Create a new energy-level diagram view", (GCallback) gtk_project::popup_ViewsNewELD },
	
	{ "RendLightsMenu", NULL, "_Lights" },
	// ^^^^^^^^^^^^^^
	{ "RendLightsNewLight", NULL, "Create New Light", NULL, "Create a new light object", (GCallback) gtk_graphics_view::popup_LightsNewLight },
// todo : convert these into toggle items so that the local setting is show in each of the graphics views...
	{ "RendLightsEnableLocLights", NULL, "Enable Local Lights", NULL, "Switch on/off the local light objects", (GCallback) gtk_graphics_view::popup_LightsSwitchLoc },
	{ "RendLightsEnableGlobLights", NULL, "Enable Global Lights", NULL, "Switch on/off the global light objects", (GCallback) gtk_graphics_view::popup_LightsSwitchGlob },
	
	{ "RendRenderMenu", NULL, "_Render Mode" },
	// ^^^^^^^^^^^^^^
// todo : convert these into toggle items so that the local setting is show in each of the graphics views...
	{ "RendRenderBallStick", NULL, "Ball & Stick", NULL, "Display atoms and bonds using Ball & Stick mode", (GCallback) gtk_project::popup_RModeBallStick },
	{ "RendRenderVanDerWaals", NULL, "van der Waals", NULL, "Display atoms and bonds using van der Waals mode", (GCallback) gtk_project::popup_RModeVanDerWaals },
	{ "RendRenderCylinders", NULL, "Cylinders", NULL, "Display atoms and bonds using Cylinders mode", (GCallback) gtk_project::popup_RModeCylinders },
	{ "RendRenderWireframe", NULL, "Wireframe", NULL, "Display atoms and bonds using Wireframe mode", (GCallback) gtk_project::popup_RModeWireframe },
	{ "RendRenderNothing", NULL, "Nothing", NULL, "Do not display atoms and bonds", (GCallback) gtk_project::popup_RModeNothing },
	
	{ "RendColorMenu", NULL, "_Color Mode" },
	// ^^^^^^^^^^^^^
// todo : convert these into toggle items so that the local setting is show in each of the graphics views...
	{ "RendColorElement", NULL, "Element", NULL, "Color atoms and bonds by element", (GCallback) gtk_project::popup_CModeElement },
	{ "RendColorSecStruct", NULL, "Sec-Struct", NULL, "Color atoms and bonds by secondary-structure state", (GCallback) gtk_project::popup_CModeSecStruct },
	{ "RendColorHydPhob", NULL, "Hydrophobicity", NULL, "Color atoms and bonds by hydrophobicity (experimental)", (GCallback) gtk_project::popup_CModeHydPhob },
	
	{ "RendLabelMenu", NULL, "_Label Mode" },
	// ^^^^^^^^^^^^^
// todo : convert these into toggle items so that the local setting is show in each of the graphics views...
	{ "RendLabelIndex", NULL, "Index", NULL, "Label atoms by atom index", (GCallback) gtk_project::popup_LModeIndex },
	{ "RendLabelElement", NULL, "Element", NULL, "Label atoms by element", (GCallback) gtk_project::popup_LModeElement },
	{ "RendLabelFCharge", NULL, "Formal Charge", NULL, "Label atoms by formal charge", (GCallback) gtk_project::popup_LModeFCharge },
	{ "RendLabelPCharge", NULL, "Partial Charge", NULL, "Label atoms by partial charge", (GCallback) gtk_project::popup_LModePCharge },
	{ "RendLabelAtomType", NULL, "Atom Type", NULL, "Label atoms by atom type", (GCallback) gtk_project::popup_LModeAtomType },
	{ "RendLabelBuilderID", NULL, "Builder ID", NULL, "Label atoms by sequence builder ID", (GCallback) gtk_project::popup_LModeBuilderID },
	{ "RendLabelBondType", NULL, "Bond Type", NULL, "Label bonds by bond type", (GCallback) gtk_project::popup_LModeBondType },
	{ "RendLabelResidue", NULL, "Residue", NULL, "Label by residue", (GCallback) gtk_project::popup_LModeResidue },
	{ "RendLabelSecStruct", NULL, "Sec-Struct", NULL, "Label by secondary structure state", (GCallback) gtk_project::popup_LModeSecStruct },
	{ "RendLabelNothing", NULL, "Nothing", NULL, "Do not show labels", (GCallback) gtk_project::popup_LModeNothing },
	
	{ "RendProjectionMenu", NULL, "_Projection" },
	// ^^^^^^^^^^^^^^^^^^
// todo : convert these into toggle items so that the local setting is show in each of the graphics views...
	{ "RendProjOrthographic", NULL, "Orthographic", NULL, "Use orthographic projection", (GCallback) gtk_graphics_view::popup_ProjOrthographic },
	{ "RendProjPerspective", NULL, "Perspective", NULL, "Use perspective projection", (GCallback) gtk_graphics_view::popup_ProjPerspective },
// todo : convert these into toggle items so that the local setting is show in each of the graphics views...
// todo : add more color combinations ; red/green red/blue ; any others needed???
	{ "RendProjSterRedBlue", NULL, "Red-Blue Stereo", NULL, "Use red-blue stereo mode", (GCallback) gtk_graphics_view::popup_ProjSterRedBlue },
	{ "RendProjSterRelaxed", NULL, "Relaxed-Eye Stereo", NULL, "Use relaxed-eye stereo mode", (GCallback) gtk_graphics_view::popup_ProjSterRelaxed },
	
	{ "ObjectsMenu", NULL, "_Objects" },
	// ^^^^^^^^^^^
	{ "ObjectsRibbon", NULL, "Ribbon", NULL, "Create a Ribbon object for displaying peptides/proteins", (GCallback) gtk_project::popup_ObjRibbon },
	{ "ObjectsEPlane", NULL, "ESP-plane", NULL, "Create an ESP-plane object", (GCallback) gtk_project::popup_ObjEPlane },
	{ "ObjectsEVolume", NULL, "volume-rendered ESP", NULL, "Create a volume-rendered ESP object", (GCallback) gtk_project::popup_ObjEVolume },
	{ "ObjectsESurface", NULL, "ESP-surface", NULL, "Create an ESP-surface object", (GCallback) gtk_project::popup_ObjESurface },
	{ "ObjectsEVDWSurface", NULL, "ESP-colored vdW-surface", NULL, "Create an ESP-colored vdW-surface object", (GCallback) gtk_project::popup_ObjEVDWSurface },
	{ "ObjectsEDPlane", NULL, "Electron density plane", NULL, "Create an Electron density plane object", (GCallback) gtk_project::popup_ObjEDPlane },
	{ "ObjectsEDSurface", NULL, "Electron density surface", NULL, "Create an Electron density surface object", (GCallback) gtk_project::popup_ObjEDSurface },
	{ "ObjectsMOPlane", NULL, "Molecular orbital plane", NULL, "Create a Molecular orbital plane object", (GCallback) gtk_project::popup_ObjMOPlane },
	{ "ObjectsMOVolume", NULL, "Molecular orbital volume", NULL, "Create a Molecular orbital volume object", (GCallback) gtk_project::popup_ObjMOVolume },
	{ "ObjectsMOSurface", NULL, "Molecular orbital surface", NULL, "Create a Molecular orbital surface object", (GCallback) gtk_project::popup_ObjMOSurface },
	{ "ObjectsMODPlane", NULL, "MO-density plane", NULL, "Create an MO-density plane object", (GCallback) gtk_project::popup_ObjMODPlane },
	{ "ObjectsMODVolume", NULL, "MO-density volume", NULL, "Create an MO-density volume object", (GCallback) gtk_project::popup_ObjMODVolume },
	{ "ObjectsMODSurface", NULL, "MO-density surface", NULL, "Create an MO-density surface object", (GCallback) gtk_project::popup_ObjMODSurface },
	{ "ObjectsDeleteCurrent", NULL, "Delete Current Object", NULL, "Delete the currently selected object", (GCallback) gtk_project::popup_ObjectsDeleteCurrent },
	
	{ "ComputeMenu", NULL, "_Compute" },
	// ^^^^^^^^^^^
	{ "CompSetup", NULL, "Setup...", NULL, "Setup or change the comp.chem. method in use", (GCallback) gtk_project::popup_CompSetup },
	{ "CompEnergy", NULL, "Energy", NULL, "Compute a single-point energy", (GCallback) gtk_project::popup_CompEnergy },
	{ "CompGeomOpt", NULL, "Geometry Optimization...", NULL, "Do a geometry optimization run", (GCallback) gtk_project::popup_CompGeomOpt },
	{ "CompMolDyn", NULL, "Molecular Dynamics...", NULL, "Do a molecular dynamics run", (GCallback) gtk_project::popup_CompMolDyn },
	{ "CompRandomSearch", NULL, "Random Conformational Search...", NULL, "Do a random conformational search", (GCallback) gtk_project::popup_CompRandomSearch },
	{ "CompSystematicSearch", NULL, "Systematic Conformational Search...", NULL, "Do a random conformational search", (GCallback) gtk_project::popup_CompSystematicSearch },
	{ "CompMonteCarloSearch", NULL, "Monte Carlo Search...", NULL, "Do a Monte Carlo type conformational search", (GCallback) gtk_project::popup_CompMonteCarloSearch },
	{ "CompTorEnePlot1D", NULL, "Plot Energy vs. 1 Torsion Angle...", NULL, "...todo...", (GCallback) gtk_project::popup_CompTorsionEnergyPlot1D },
	{ "CompTorEnePlot2D", NULL, "Plot Energy vs. 2 Torsion Angles...", NULL, "...todo...", (GCallback) gtk_project::popup_CompTorsionEnergyPlot2D },
	{ "CompPopAnaESP", NULL, "Population Analysis (ESP)", NULL, "...todo...", (GCallback) gtk_project::popup_CompPopAnaElectrostatic },
	{ "CompTransitionSS", NULL, "Transition State Search...", NULL, "...todo...", (GCallback) gtk_project::popup_CompTransitionStateSearch },
	{ "CompStationarySS", NULL, "Stationary State Search...", NULL, "...todo...", (GCallback) gtk_project::popup_CompStationaryStateSearch },
	{ "CompFormula", NULL, "Formula", NULL, "...todo...", (GCallback) gtk_project::popup_CompFormula },
{ "CompUC_SetFormalCharge", NULL, "UnderConstruction ; RS ; Set Formal Charge", NULL, "...todo...", (GCallback) gtk_project::popup_CompSetFormalCharge },
{ "CompUC_CreateRS", NULL, "UnderConstruction ; RS ; make RS", NULL, "...todo...", (GCallback) gtk_project::popup_CompCreateRS },
{ "CompUC_CycleRS", NULL, "UnderConstruction ; RS ; cycle RS", NULL, "...todo...", (GCallback) gtk_project::popup_CompCycleRS },
	
	{ "TrajView", NULL, "MD Trajectory Viewer...", NULL, "...todo...", (GCallback) gtk_project::popup_TrajView },
	{ "SetOrbital", NULL, "Set Current Orbital", NULL, "...todo...", (GCallback) gtk_project::popup_SetOrbital },
	
	{ "BuildMenu", NULL, "_Build" },
	// ^^^^^^^^^
	{ "BuildSolvateBox", NULL, "Solvate Box...", NULL, "...todo...", (GCallback) gtk_project::popup_SolvateBox },
	{ "BuildSolvateSphere", NULL, "Solvate Sphere...", NULL, "...todo...", (GCallback) gtk_project::popup_SolvateSphere },
	{ "BuildSeqBuildAmino", NULL, "Sequence Builder (amino)...", NULL, "...todo...", (GCallback) gtk_project::popup_BuilderAmino },
	{ "BuildSeqBuildNucleic", NULL, "Sequence Builder (nucleic)...", NULL, "...todo...", (GCallback) gtk_project::popup_BuilderNucleic },
	{ "BuildCenter", NULL, "Center", NULL, "...todo...", (GCallback) gtk_project::popup_Center },
	{ "BuildClear", NULL, "Zap All", NULL, "...todo...", (GCallback) gtk_project::popup_Clear },
	
	{ "BuildHydrogensMenu", NULL, "_Hydrogens" },
	// ^^^^^^^^^^^^^^^^^^
	{ "BuildHydrogensAdd", NULL, "Add", NULL, "...todo...", (GCallback) gtk_project::popup_HAdd },
	{ "BuildHydrogensRemove", NULL, "Remove", NULL, "...todo...", (GCallback) gtk_project::popup_HRemove },
	
	{ "EnterCommand", NULL, "Enter a Command...", NULL, "...todo...", (GCallback) gtk_project::popup_EnterCommand },
};

const char * gtk_graphics_view::ui_description =
"<ui>"
"  <popup name='ggvMenu'>"
"    <menuitem action='AttachDetachGV'/>"
"    <separator/>"
"    <menu action='FileMenu'>"
"      <menuitem action='Open'/>"
#ifdef ENABLE_OPENBABEL
"      <menuitem action='Import'/>"
#endif	// ENABLE_OPENBABEL
"      <separator/>"
"      <menuitem action='SaveAs'/>"
#ifdef ENABLE_OPENBABEL
"      <menuitem action='Export'/>"
#endif	// ENABLE_OPENBABEL
"      <separator/>"
"      <menu action='FileExtraMenu'>"
"        <menuitem action='ImportPDB'/>"
"        <menuitem action='ConvertAA2SF'/>"
"      </menu>"
"      <separator/>"
"      <menuitem action='Close'/>"
"    </menu>"
"    <menu action='SelectMenu'>"
"      <menuitem action='SelectAll'/>"
"      <menuitem action='SelectNone'/>"
"      <separator/>"
"      <menuitem action='InvertSelection'/>"
"      <separator/>"
"      <menu action='SelectModeMenu'>"
"        <menuitem action='SelectAtoms'/>"
"        <menuitem action='SelectResidues'/>"
"        <menuitem action='SelectChains'/>"
"        <menuitem action='SelectMolecules'/>"
"      </menu>"
"      <separator/>"
"      <menuitem action='DeleteSelected'/>"
"    </menu>"
"    <separator/>"
"    <menu action='RenderMenu'>"
"      <menu action='RendViewsMenu'>"
"        <menu action='RendViewsCreateMenu'>"
"          <menuitem action='RendViewsCreateCam'/>"
"          <menuitem action='RendViewsCreateView'/>"
"          <separator/>"
"          <menuitem action='RendViewsCreateELD'/>"
"        </menu>"
"        <menuitem action='RendViewsDeleteView'/>"
"        <separator/>"
"        <menuitem action='RendViewsPushCRDSet'/>"
"        <menuitem action='RendViewsSuperimpose'/>"
"      </menu>"
"      <menu action='RendLightsMenu'>"
"        <menuitem action='RendLightsNewLight'/>"
"        <separator/>"
"        <menuitem action='RendLightsEnableLocLights'/>"
"        <menuitem action='RendLightsEnableGlobLights'/>"
"      </menu>"
"      <separator/>"
"      <menu action='RendRenderMenu'>"
"        <menuitem action='RendRenderBallStick'/>"
"        <menuitem action='RendRenderVanDerWaals'/>"
"        <menuitem action='RendRenderCylinders'/>"
"        <menuitem action='RendRenderWireframe'/>"
"        <menuitem action='RendRenderNothing'/>"
"      </menu>"
"      <menu action='RendColorMenu'>"
"        <menuitem action='RendColorElement'/>"
"        <menuitem action='RendColorSecStruct'/>"
"        <menuitem action='RendColorHydPhob'/>"
"      </menu>"
"      <menu action='RendLabelMenu'>"
"        <menuitem action='RendLabelIndex'/>"
"        <menuitem action='RendLabelElement'/>"
"        <menuitem action='RendLabelFCharge'/>"
"        <menuitem action='RendLabelPCharge'/>"
"        <menuitem action='RendLabelAtomType'/>"
"        <menuitem action='RendLabelBuilderID'/>"
"        <separator/>"
"        <menuitem action='RendLabelBondType'/>"
"        <separator/>"
"        <menuitem action='RendLabelResidue'/>"
"        <menuitem action='RendLabelSecStruct'/>"
"        <separator/>"
"        <menuitem action='RendLabelNothing'/>"
"      </menu>"
"      <separator/>"
"      <menu action='RendProjectionMenu'>"
"        <menuitem action='RendProjOrthographic'/>"
"        <menuitem action='RendProjPerspective'/>"
"        <separator/>"
"        <menuitem action='RendProjSterRedBlue'/>"
"        <menuitem action='RendProjSterRelaxed'/>"
"      </menu>"
"      <menuitem action='RenderQuickUpdate'/>"
"    </menu>"
"    <menu action='ObjectsMenu'>"
"      <menuitem action='ObjectsRibbon'/>"
"      <separator/>"
"      <menuitem action='ObjectsEPlane'/>"
"      <menuitem action='ObjectsEVolume'/>"
"      <menuitem action='ObjectsESurface'/>"
"      <menuitem action='ObjectsEVDWSurface'/>"
"      <separator/>"
"      <menuitem action='ObjectsEDPlane'/>"
"      <menuitem action='ObjectsEDSurface'/>"
"      <menuitem action='ObjectsMOPlane'/>"
"      <menuitem action='ObjectsMOVolume'/>"
"      <menuitem action='ObjectsMOSurface'/>"
"      <menuitem action='ObjectsMODPlane'/>"
"      <menuitem action='ObjectsMODVolume'/>"
"      <menuitem action='ObjectsMODSurface'/>"
"      <separator/>"
"      <menuitem action='ObjectsDeleteCurrent'/>"
"    </menu>"
"    <separator/>"
"    <menu action='ComputeMenu'>"
"      <menuitem action='CompSetup'/>"
"      <separator/>"
"      <menuitem action='CompEnergy'/>"
"      <menuitem action='CompGeomOpt'/>"
"      <menuitem action='CompMolDyn'/>"
"      <menuitem action='CompRandomSearch'/>"
"      <menuitem action='CompSystematicSearch'/>"
"      <menuitem action='CompMonteCarloSearch'/>"
"      <separator/>"
"      <menuitem action='CompTorEnePlot1D'/>"
"      <menuitem action='CompTorEnePlot2D'/>"
"      <separator/>"
"      <menuitem action='CompPopAnaESP'/>"
"      <menuitem action='CompTransitionSS'/>"
"      <menuitem action='CompStationarySS'/>"
"      <separator/>"
"      <menuitem action='CompFormula'/>"
"<menuitem action='CompUC_SetFormalCharge'/>"
"<menuitem action='CompUC_CreateRS'/>"
"<menuitem action='CompUC_CycleRS'/>"
"    </menu>"
"    <menuitem action='TrajView'/>"
"    <menuitem action='SetOrbital'/>"
"    <separator/>"
"    <menu action='BuildMenu'>"
"      <menu action='BuildHydrogensMenu'>"
"        <menuitem action='BuildHydrogensAdd'/>"
"        <menuitem action='BuildHydrogensRemove'/>"
"      </menu>"
"      <separator/>"
"      <menuitem action='BuildSolvateBox'/>"
"      <menuitem action='BuildSolvateSphere'/>"
"      <separator/>"
"      <menuitem action='BuildSeqBuildAmino'/>"
"      <menuitem action='BuildSeqBuildNucleic'/>"
"      <separator/>"
"      <menuitem action='BuildCenter'/>"
"      <menuitem action='BuildClear'/>"
"    </menu>"
"    <separator/>"
"    <menuitem action='EnterCommand'/>"
"  </popup>"
"</ui>";
#endif
win_graphics_view::win_graphics_view(win_project * p1, camera * p2) :
	win_ogl_view(p1), graphics_view(p2), ogl_view()
{
//printf("win_graphics_view::win_graphics_view(win_project * p1, camera * p2) :\n"
//	"win_ogl_view(p1), graphics_view(p2), ogl_view()\n\n");
	BeginWinGhemical();
//	cout << "&win_graphics_view = " << this << endl;
	view_widget = CreateWinGhemical(dynamic_cast<win_ogl_view*>(this));
//	cout << "view_widget = " << view_widget << endl;

	SendMessage(view_widget, WM_COMMAND, ID_FILE_INITOG2, 0);

	ConfigureHandler(view_widget);

		//	GtkActionGroup * action_group = gtk_action_group_new("ggvActions");
//	gtk_action_group_add_actions(action_group, entries, G_N_ELEMENTS(entries), GTK_WIDGET(view_widget));
	
//	GtkUIManager * ui_manager = gtk_app::GetUIManager();	// does not give us independent menu widgets...
//	GtkUIManager * ui_manager = gtk_ui_manager_new();
	
//	gtk_ui_manager_insert_action_group(ui_manager, action_group, 0);
	
//	GError * error = NULL;
//	if (!gtk_ui_manager_add_ui_from_string(ui_manager, ui_description, -1, & error))
//	{
//		g_message("Building ggv menu failed : %s", error->message);
//		g_error_free(error); exit(EXIT_FAILURE);
//	}
	
//	popupmenu = gtk_ui_manager_get_widget(ui_manager, "/ggvMenu");
	
//	my_stereo_dialog = NULL;
}

win_graphics_view::~win_graphics_view(void)
{
/*	if (my_stereo_dialog != NULL)
	{
		gtk_widget_destroy(my_stereo_dialog);
		my_stereo_dialog = NULL;
	}*/
}
win_graphics_view * win_graphics_view::GetGV(HWND widget)
{
	win_ogl_view * oglv = win_ogl_view::GetOGLV(widget);
	if (!oglv) cout << "Unknown ID in win_graphics_view::GetGV() !!!" << endl;
	
	win_graphics_view * gv = dynamic_cast<win_graphics_view *>(oglv);
	if (gv != NULL) return gv;
	else
	{
		cout << "bad cast in win_graphics_view::GetGV() !!!!!" << endl;
		return NULL;
	}
}
void win_graphics_view::ExposeEvent(void)
{
//	printf("win_graphics_view::ExposeEvent(void)\nWM_PAINT !!!\n\n");
	// WM_PAINT !!!!!

//	GdkGLContext * glcontext = gtk_widget_get_gl_context(view_widget);
//	GdkGLDrawable * gldrawable = gtk_widget_get_gl_drawable(view_widget);
	
//	gdk_gl_drawable_gl_begin(gldrawable, glcontext);
	
	SetCurrent(); Render(); glFlush();
	
//	if (gdk_gl_drawable_is_double_buffered(gldrawable)) gdk_gl_drawable_swap_buffers(gldrawable);
//	else glFlush();
	
//	gdk_gl_drawable_gl_end(gldrawable);
}

// the popup-menu callbacks related to graphics_view are here...
// the popup-menu callbacks related to graphics_view are here...
// the popup-menu callbacks related to graphics_view are here...

void win_graphics_view::popup_ProjOrthographic(HWND widget, void * data)
{
	win_graphics_view * gv = GetGV(widget);
	gv->cam->ortho = true; gv->cam->stereo_mode = false; gv->cam->stereo_relaxed = false; 
	gv->prj->UpdateGraphicsViews(gv->cam);
}

void win_graphics_view::popup_ProjPerspective(HWND widget, void * data)
{
	win_graphics_view * gv = GetGV(widget);
	gv->cam->ortho = false; gv->cam->stereo_mode = false; gv->cam->stereo_relaxed = false; 
	gv->prj->UpdateGraphicsViews(gv->cam);
}
#if 0
void gtk_graphics_view::popup_ProjSterRedBlue(GtkWidget *, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	gv->cam->ortho = false; gv->cam->stereo_mode = true; gv->cam->stereo_relaxed = false;
	
	create_stereo_adj(data, 2.5);
	
	gv->prj->UpdateGraphicsViews(gv->cam);
}

void gtk_graphics_view::popup_ProjSterRelaxed(GtkWidget *, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	gv->cam->ortho = false; gv->cam->stereo_mode = true; gv->cam->stereo_relaxed = true;
	
	create_stereo_adj(data, 5.0);
	
	gv->prj->UpdateGraphicsViews(gv->cam);
}

void gtk_graphics_view::create_stereo_adj(gpointer data, float angrng)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	
	// 20050422 ; the initial value of gv->cam->rang???
	// should be neg for red/blue and pos for relaxed???
	
	if (!gv->my_stereo_dialog)
	{
		GtkWidget * vbox;
		GtkWidget * hbox1, * label1, * hscale1; GtkObject * adj1;
		GtkWidget * hbox2, * label2, * hscale2; GtkObject * adj2;
		
		gv->my_stereo_dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(gv->my_stereo_dialog), "Adjust the Separation");
		gtk_window_set_transient_for(GTK_WINDOW(gv->my_stereo_dialog), GTK_WINDOW(gtk_app::GetMainWindow()));
		gtk_window_set_resizable(GTK_WINDOW(gv->my_stereo_dialog), FALSE);
		
		gtk_container_set_border_width(GTK_CONTAINER(gv->my_stereo_dialog), 10);
		
		vbox = gtk_vbox_new(FALSE, 10);
		gtk_container_set_border_width(GTK_CONTAINER (vbox), 10);
		gtk_container_add(GTK_CONTAINER(gv->my_stereo_dialog), vbox);
		
		hbox1 = gtk_hbox_new(TRUE, 10);
		label1 = gtk_label_new("Separation Distance:");
		gtk_box_pack_start(GTK_BOX(hbox1), label1, FALSE, FALSE, 0);
		adj1 = gtk_adjustment_new(gv->cam->slider, -5.0, +5.0, 0.005, 0.050, 0.0);
		hscale1 = gtk_hscale_new(GTK_ADJUSTMENT(adj1));
		scale_set_default_values(GTK_SCALE(hscale1));
		gtk_box_pack_start(GTK_BOX(hbox1), hscale1, TRUE, TRUE, 0);
		gtk_widget_show(label1); gtk_widget_show(hscale1); gtk_widget_show(hbox1);
		gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, FALSE, 0);
		
		hbox2 = gtk_hbox_new(TRUE, 10);
		label2 = gtk_label_new("Second Molecule Angle:");
		gtk_box_pack_start(GTK_BOX(hbox2), label2, FALSE, FALSE, 0);
		adj2 = gtk_adjustment_new(gv->cam->rang, -angrng, +angrng, 0.005, 0.050, 0.0);
		hscale2 = gtk_hscale_new(GTK_ADJUSTMENT(adj2));
		scale_set_default_values(GTK_SCALE(hscale2));
		gtk_box_pack_start(GTK_BOX(hbox2), hscale2, TRUE, TRUE, 0);
		gtk_widget_show(label2); gtk_widget_show(hscale2); gtk_widget_show(hbox2);
		gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, FALSE, 0);
		
		gtk_widget_show(vbox);
		gtk_widget_show_all(gv->my_stereo_dialog);
		
		gtk_signal_connect(GTK_OBJECT(adj1), "value_changed", GTK_SIGNAL_FUNC(callback_distance), data);
		gtk_signal_connect(GTK_OBJECT(adj2), "value_changed", GTK_SIGNAL_FUNC(callback_angle), data);
		
		gtk_signal_connect(GTK_OBJECT(gv->my_stereo_dialog), "destroy", GTK_SIGNAL_FUNC(callback_destroy), data);
	}
}

void gtk_graphics_view::callback_distance(GtkAdjustment * adj, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	
	gv->cam->slider = adj->value;
	gv->prj->UpdateGraphicsViews(gv->cam);
}

void gtk_graphics_view::callback_angle(GtkAdjustment * adj, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	
	gv->cam->rang = adj->value;
	gv->prj->UpdateGraphicsViews(gv->cam);
}

void gtk_graphics_view::callback_destroy(GtkWidget *, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	
	if (gv->my_stereo_dialog != NULL)
	{
		gtk_widget_destroy(gv->my_stereo_dialog);
		gv->my_stereo_dialog = NULL;
	}
}

void gtk_graphics_view::scale_set_default_values(GtkScale *scale)
{
	gtk_scale_set_digits(scale, 3);
	gtk_scale_set_value_pos(scale, GTK_POS_RIGHT);
	gtk_scale_set_draw_value (scale, TRUE);
}

void gtk_graphics_view::popup_RenderQuickUpdate(GtkWidget *, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	gv->quick_update = !gv->quick_update;
}

void gtk_graphics_view::popup_ViewsAttachDetach(GtkWidget *, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	
// this function offers attaching/detaching to/from the prj notebook. it perhaps would be nice
// if ALL views had this possibility, but that turned out to be complicated. the graphics views benefit
// most from this; also the project view should remain in notebook (or the notebook might become empty).
// anyways, more elegant way would be to just take the widget out of the notebook page, and then connect
// it to a new GtkWindow. however, it seems not be that easy (at least in GTK v1.2) because a call to
// the gtk_notebook_remove_page() seems to destroy the page widget.

	// first we create a new view using the requested mode...
	
	bool flag = (gv->detached == NULL);
	graphics_view * new_gv = gv->prj->AddGraphicsView(gv->cam, flag);
	
	new_gv->render = gv->render;	// try to retain as much graphics settings as possible.
	new_gv->label = gv->label;
	
	new_gv->my_gv_number = gv->my_gv_number;		// also retain the old numbering...
	new_gv->cam->graphics_view_counter -= 1;		// also retain the old numbering...
	
	// ...and then simply destroy the old one; calling popup_ViewsDeleteView()
	// below this is the same as selecting "Delete This View" from the popup-menu.
	
	popup_ViewsDeleteView(NULL, data);
	new_gv->GetProject()->UpdateAllWindowTitles();		// also retain the old numbering...
}

void gtk_graphics_view::popup_ViewsNewCam(GtkWidget *, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	gv->prj->AddGraphicsView(NULL, false);
}

void gtk_graphics_view::popup_ViewsNewView(GtkWidget *, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	gv->prj->AddGraphicsView(gv->cam, false);
}

void gtk_graphics_view::popup_ViewsDeleteView(GtkWidget *, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	
	// before removing the view, we have to remove it from the GtkNotebook.
	// however, not all calls to RemoveGraphicsView() are successful; here we will not
	// force the removal, so if the view to be removed is the last graphics view for
	// the project, RemoveGraphicsView() will just pop up an error message and skip
	// the removal. so, we use IsThisLastGraphicsView() to detect this case...
	
	// now we also have those "detached" windows, and we have to handle them separately.
	
	if (!gv->prj->IsThisLastGraphicsView(gv))
	{
		if (gv->detached != NULL)
		{
			gtk_widget_destroy(GTK_WIDGET(gv->detached));
		}
		else
		{
			gint page = gtk_notebook_page_num(GTK_NOTEBOOK(gv->prj->notebook_widget), gv->view_widget);
			gtk_notebook_remove_page(GTK_NOTEBOOK(gv->prj->notebook_widget), page);
		}
		
		// perhaps a more elegant way would be to change "add/remove view"-style funtions into virtual functions,
		// and then override the default behaviour at, say, here in gtk classes (where we would do add/remove operations
		// for out GtkNotebook). HOWEVER, it seems to have some side-effects; just lost 4 h trying to figure it out...  :(
		
		gv->prj->RemoveGraphicsView(gv, false);
	}
}

void gtk_graphics_view::popup_ViewsPushCRDSet(GtkWidget *, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	
	gv->prj->PushCRDSets(1);
	
	i32s tmp1 = ((i32s) gv->prj->cs_vector.size()) - 1;
	while (tmp1 > 0) { gv->prj->CopyCRDSet(tmp1 - 1, tmp1); tmp1 -= 1; }
	
	gv->prj->cs_vector.back()->visible = true;
	gv->prj->UpdateAllGraphicsViews();
}

void gtk_graphics_view::popup_ViewsSuperimpose(GtkWidget *, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	
#ifdef ENABLE_OPENBABEL
gv->prj->ErrorMessage("\
FIXME : superimpose conflicts with openbabel?\n\
g++ (GCC) 3.3.5 (Debian 1:3.3.5-13) 20050701\n\
compiler says superimpose is undeclared???");
#else
	f64 sum = 0.0;
	for (i32s n1 = 1;n1 < (i32s) gv->prj->GetCRDSetCount();n1++)
	{
		superimpose si(gv->prj, 0, n1);
		
		for (i32s n2 = 0;n2 < 100;n2++)
		{
			si.TakeCGStep(conjugate_gradient::Newton2An);
		}
		
		f64 rms = si.GetRMS(); sum += rms;
		cout << "sets 0 <-> " << n1 << " RMS = " << rms << endl;
		
		si.Transform();		// modify the coordinates!!!
	}
	
	f64 average = sum / ((f64) gv->prj->GetCRDSetCount() - 1);
	cout << "average RMS = " << average << endl;
#endif	// ENABLE_OPENBABEL
	
	gv->prj->UpdateAllGraphicsViews();
}

void gtk_graphics_view::popup_LightsNewLight(GtkWidget *, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	
	static const char command[] = "add light";
	new command_dialog(gv->prj, gv, command);
}

void gtk_graphics_view::popup_LightsSwitchLoc(GtkWidget *, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	gv->prj->DoSwitchLocalLights(gv->cam, false);
}

void gtk_graphics_view::popup_LightsSwitchGlob(GtkWidget *, gpointer data)
{
	gtk_graphics_view * gv = GetGV((GtkWidget *) data);
	gv->prj->DoSwitchGlobalLights(gv->cam, false);
}
#endif  //0
/*################################################################################################*/

// eof
