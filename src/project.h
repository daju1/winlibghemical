// PROJECT.H : base classes for all graphical user interfaces.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef PROJECT_H
#define PROJECT_H

#include "appconfig.h"

class project;
class dummy_project;

class color_mode;
class color_mode_element;		// atom *
class color_mode_secstruct;		// atom *
class color_mode_hydphob;		// atom *

class transparent_primitive;
class transparent_primitive_data;

class tpd_tri_3c;
class tpd_quad_4c;

class project_view;		// views.h
class graphics_view;		// views.h
class view_factory;		// views.h

class dummy_object;		// objects.h
class smart_object;		// objects.h

class camera;			// camera.h
class light;			// camera.h

class transformer;		// camera.h

#include "../libghemical/src/model.h"
#include "../libghemical/src/geomopt.h"
#include "../libghemical/src/moldyn.h"

//#include <glib.h>

#include "project.h"
#include "views.h"

#include "filetrans.h"

// do not include any gtk/etc stuff here ; this is platform-independent, OpenGL only!!!
// do not include any gtk/etc stuff here ; this is platform-independent, OpenGL only!!!
// do not include any gtk/etc stuff here ; this is platform-independent, OpenGL only!!!

#include <vector>
using namespace std;

#define FILENAME_FILTER	"*.gpr"

/*################################################################################################*/
#if PROBNIY_ATOM_GEOMOPT
class jobinfo_work_prob_atom_GeomOpt
{
	public:
	
	project * prj;
	geomopt_param * go;	// this is a class...

	char * infile_name; 
	char * trgtlst_name;
	char * box_name;
	char * fixed_name;
	int total_frames;
	
	jobinfo_work_prob_atom_GeomOpt(void){go = NULL; prj = NULL; }
};
#endif

class jobinfo_GeomOpt
{
	public:
	
	project * prj;
	geomopt_param go;	// this is a class...
	
	jobinfo_GeomOpt(void) : go(NULL) { prj = NULL; }
};

class jobinfo_MolDyn
{
	public:
	
	project * prj;
	moldyn_param md;	// this is a class...
	
	jobinfo_MolDyn(void) : md(NULL) { prj = NULL; }
};

class jobinfo_MolDyn_tst
{
	public:
	
	project * prj;
	moldyn_tst_param md;	// this is a class...
	
	jobinfo_MolDyn_tst(void) : md(NULL) { prj = NULL; }
};

struct jobinfo_RandomSearch
{
	project * prj;
	
	int cycles;
	int optsteps;
};


/*################################################################################################*/

#define SB_SIZE 512		// the OGL selection buffer size.

/**	The "##project" class is an enhanced version of "##model" class, containing
	graphical user interface.
*/

class project : public model
{
	private:
	
	graphics_class_factory * graphics_factory;
	
	protected:
	
	char * project_path;		// may or may not exist...
	char * project_filename;	// should always exist...
	
	vector<camera *> camera_vector;
	vector<graphics_view *> graphics_view_vector;
	vector<plotting_view *> plotting_view_vector;
	
	i32s camera_counter;
	i32s object_counter;
	
/**	A static counter of display list IDs.
	In some targets there can be several GUI's running simultaneously,
	so we must make sure that the display list ID's are unique -> static.
*/
	static iGLu list_counter;
	
	vector<transparent_primitive> tp_vector;
	
// at the moment, we have only a single project_view object.
// this could be always open?!?!? is it needed??? improve MDI first...
	project_view * pv;
	
	ifstream * trajfile;
	i32s traj_num_atoms;
	i32s total_traj_frames;
	i32s current_traj_frame;
	
	atom * mt_a1;	// measure_tool
	atom * mt_a2;	// measure_tool
	atom * mt_a3;	// measure_tool
	
	public:
	
	enum rmode { Normal = 0, Transform1 = 1, Transform2 = 2 };
	
	vector<light *> light_vector;
	vector<smart_object *> object_vector;
	dummy_object * selected_object;
	
	iGLu * select_buffer;

	public:
	
	static const char appversion[16];
	static const char appdata_path[256];
	
	static color_mode_element cm_element;
	static color_mode_secstruct cm_secstruct;
	static color_mode_hydphob cm_hydphob;
	
	atom * draw_data[2];
	
	public:
	
	project(graphics_class_factory &);
	virtual ~project(void);
	
	virtual bool Question(const char *) = 0;
	
	i32s GetNewCameraID(void) { return camera_counter++; }
	i32s GetNewObjectID(void) { return object_counter++; }
	
	graphics_class_factory * GetGraphicsFactory(void) { return graphics_factory; }
	
/**	Some functions to support transparent primitives... 
	External objects (planes, surfaces, etc) should use these.
*/
	bool AddTP(void *, transparent_primitive &);
	void UpdateMPsForAllTPs(void *);
	void RemoveAllTPs(void *);
	
	// some file name functions...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^

	const char * GetProjectFileNameExtension(void);
	
	void SetProjectPath(const char *);
	void SetProjectFileName(const char *);
	
	void SetDefaultProjectFileName(void);
	
	void ParseProjectFileNameAndPath(const char *);
	
	void GetProjectFileName(char *, int, bool);
	void GetFullProjectFileName(char *, int);

	// methods for file I/O : ReadGPR and WriteGPR are friend functions so that it would be easier to "borrow" them elsewhere...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	friend bool ReadGPR_OLD(project &, istream &, bool, bool = false);		///< this is for the very old version.
	friend bool ReadGPR_v100(project &, istream &, bool, bool = false);		///< this is for the version 1.00.
	friend bool ReadGPR_v110(project &, istream &, bool, bool = false);		///< this is for the version 1.10.
/// This is an input function for the v1.11 ghemical file format.
	friend bool ReadGPR(project &, istream &, bool, bool = false);
	
	friend void WriteGPR_v100(project &, ostream &);		///< this is for the version 1.00.
/// This is an output function for the v1.11 ghemical file format.
	friend void WriteGPR(project &, ostream &);
	
#ifdef ENABLE_OPENBABEL
	
	bool ImportFile(const char * filename, int index = 0);
	bool ExportFile(const char * filename, int index = 0);
	
#endif	// ENABLE_OPENBABEL
	
	// methods related to MD trajectories...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	void TrajectorySetTotalFrames(const char * fn, i32s _total_traj_frames);
	void OpenTrajectory(const char *);
	void CloseTrajectory(void);
	void ReadFrame(void);
	// WriteFrame???
	void EvaluateBFact(void);
	void EvaluateDiffConst(double);
	
	i32s GetTotalFrames(void);
	ifstream * GetTrajectoryFile(void);
	
	i32s GetCurrentFrame(void);
	void SetCurrentFrame(i32s);
	
	// add or remove hydrogen atoms...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	void AddH(void);
	void RemoveH(void);
	
	// some utility functions.
	// ^^^^^^^^^^^^^^^^^^^^^^^

	const char * GetType(void);
	color_mode * GetDefaultColorMode(void);
	
	void SelectAll(void);
	void InvertSelection(void);
	void DeleteSelection(void);
	
	iGLu GetDisplayListIDs(iGLu);
	void DeleteDisplayLists(iGLu, iGLu);
	
	// we need to intercept these events from the model class for keeping project_view up-to-date!
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	void Add_Atom(atom &);		// virtual
	void RemoveAtom(iter_al);	// virtual
	
	void AddBond(bond &);		// virtual
	void RemoveBond(iter_bl);	// virtual
	
	void InvalidateGroups(void);	// virtual
	void UpdateChains(void);	// virtual
	
	// some light-related functions:
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	i32s IsLight(const dummy_object *);
	bool SelectLight(const dummy_object *);
	
	bool AddGlobalLight(light *);
	bool AddLocalLight(light *, camera *);
	bool RemoveLight(dummy_object *);
	
	iGLs CountGlobalLights(void);
	iGLs CountLocalLights(camera *);
	
	bool SetupLights(camera *);
	
	void SetGlobalLightNumbers(void);
	void SetLocalLightNumbers(camera *);
	
	// some object-related functions:
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	i32s IsObject(const dummy_object *);
	bool SelectObject(const dummy_object *);
	
	void AddObject(smart_object *);
	bool RemoveObject(dummy_object *);
	
	// some camera/window functions:
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	fGL GetDefaultFocus(void);
	
	project_view * CreateProjectView(void);
	bool DestroyProjectView(void);
	
	graphics_view * AddGraphicsView(camera *, bool);	///< if (camera *) is NULL, a new one is created...
	bool RemoveGraphicsView(graphics_view *, bool);
	bool IsThisLastGraphicsView(graphics_view *);
	
	plot1d_view * AddPlot1DView(i32s, const char *, const char *, bool);
	plot2d_view * AddPlot2DView(i32s, const char *, const char *, const char *, bool);
	rcp_view * AddReactionCoordinatePlotView(i32s, const char *, const char *, bool);
	eld_view * AddEnergyLevelDiagramView(bool);
	bool RemovePlottingView(plotting_view *);
	
	void UpdateAllViews(void);
	virtual void UpdateAllWindowTitles(void);
	
	void UpdateAllGraphicsViews(bool = false);			// virtual
	void UpdateGraphicsViews(camera *, bool = false);
	void UpdateGraphicsView(graphics_view *, bool = false);
	
	void UpdatePlottingViews(void);
	
	// update for other views?!?!?!? or all views ?!?!?!?
	// update for other views?!?!?!? or all views ?!?!?!?
	// update for other views?!?!?!? or all views ?!?!?!?
	
/**	Contains some OpenGL initialization commands 
	(color, material, light settings). 
	The OGL selection buffer should also be assigned here!!!
*/
	void InitGL(void);
	
/**	Contains the OpenGL rendering commands that draw the 3D-view.

	Selection is done in a rather crude way by dumping the atom pointers directly to 
	the OGL selection buffer and reading them later there... Works fine as long as 
	both datatypes have the same size (currently it's 32 bits). If we some day need 
	64-bit pointers and have only 32-bit sel-buffer we have to use two names...
*/
	void Render(graphics_view *, rmode);

/**	This is just quickly cut into a different function for clarity... We will call this
	multiple times with translation applied if we render some periodic systems...
*/
	void RenderScene(graphics_view *, rmode, bool);
	
	void RenderAllTPs(graphics_view *, rmode);
	
/**	This should center the transformer object to the center of all
	selected atoms, and shift the center of all selected atoms to the origo.
*/
	void Center(transformer *);
	
/**	This should get the transformation matrix from the transformer
	object and use it to transform all selected atoms/bonds/whatever.
*/
	void Transform(transformer *);
	
	void MolAxisEvent(graphics_view *, vector<iGLu> &);
	void DrawEvent(graphics_view *, vector<iGLu> &);
	void EraseEvent(graphics_view *, vector<iGLu> &);
	
	void SelectEvent(graphics_view *, vector<iGLu> &);
	void MeasureEvent(graphics_view *, vector<iGLu> &);
	
	// some common operations are implemented here:
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
#ifdef ENABLE_THREADS

	virtual void CreateProgressDialog(const char *, bool, int, int) = 0;
	virtual void DestroyProgressDialog(void) = 0;
	
#endif	// ENABLE_THREADS
	
	void ProcessCommandString(graphics_view *, const char *);
	
// the GUI threading model is explained here:
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// the "main thread" of the program handles gtk_main() loop and callbacks.
// in ProcessCommandString() a single "sub-thread" can be started for certain
// operations, if the program is compiled with ENABLE_THREADS set. therefore all
// operations in the pcs_job_XXX() methods below must be made thread-safe in the
// "gdk_threads" sense. THIS IS STILL EXPERIMENTAL STUFF!!!
	
#if PROBNIY_ATOM_GEOMOPT
	HANDLE  start_job_work_prob_atom_GeomOpt(jobinfo_work_prob_atom_GeomOpt * ji);
	static DWORD WINAPI pcs_job_work_prob_atom_GeomOpt(void* p);
#endif


	HANDLE  start_job_GeomOpt(jobinfo_GeomOpt *);	// for this we have both a dialog and a command line interface...
//	static gpointer pcs_job_GeomOpt(gpointer);
	static DWORD WINAPI pcs_job_GeomOpt(void*);
	
	HANDLE  start_job_MolDyn(jobinfo_MolDyn *);	// for this we have both a dialog and a command line interface...
	HANDLE  start_job_MolDyn_tst(jobinfo_MolDyn_tst *);	// for this we have both a dialog and a command line interface...
//	static gpointer pcs_job_MolDyn(gpointer);
	static DWORD WINAPI pcs_job_MolDyn(void*);
	static DWORD WINAPI pcs_job_MolDyn_tst(void*);
	
//	static gpointer pcs_job_RandomSearch(gpointer);
	static void* pcs_job_RandomSearch(void*);
	
	void DoDeleteCurrentObject(void);
	
	void DoSwitchLocalLights(camera *, bool);
	void DoSwitchGlobalLights(camera *, bool);
	
	bool TestAtom(atom *, rmode);
	bool TestBond(bond *, rmode);
	
	inline void SetColor(color_mode *, atom *, bool);
	
	void DrawCylinder1(const fGL **, const fGL **, const fGL *);
	
// here are some Do???() functions similar to those in the model class.
// however, the classes here are "interactive" and are directly dependent on graphics.
// so you don't want to use any of these classes in the console version...

	double DoFormula(bool msg = true);
	void DoDensity(void);

	void SetTheFlagOnSelectedAtoms(i32u flag_number);
	void UnSetTheFlagOnSelectedAtoms(i32u flag_number);
	void SelectAtomsWithTheFlag(i32u flag_number);

	void SelectMolecularAxises();

	void TrajView_NematicCoordinatePlot(i32s type, i32s dim);

	void TrajView_VeloncityDistribution2D(i32s divx, i32s divy, f64 dt);

	void TrajView_CoordinateDifferencePlot(i32s ind1, i32s ind2, i32s dim);
	void TrajView_CoordinatePlot(i32s ind, i32s dim);
	void TrajView_DistancePlot(i32s inda, i32s indb);
	void TrajView_AnglePlot(i32s inda, i32s indb, i32s indc);

	void DoEnergyPlot1D(i32s, i32s, i32s, i32s, fGL, fGL, i32s, char *);

	void DoEnergyPlot1D(i32s, i32s, i32s, i32s, i32s, fGL, fGL, i32s);
	void DoEnergyPlot2D(i32s, i32s, i32s, i32s, i32s, fGL, fGL, i32s, i32s, i32s, i32s, i32s, fGL, fGL, i32s);
	
	void DoTransitionStateSearch(f64, f64);
	void DoStationaryStateSearch(i32s);
};

float measure_len(float *, float *);
float measure_ang(float *, float *, float *);
float measure_tor(float *, float *, float *, float *);

/*################################################################################################*/

/// the purpose of this class is just to make file operations available (ReadGPR() wants project object).

class dummy_project : public project
{
	public:
	
	dummy_project(graphics_class_factory *);
	~dummy_project(void);
	
#ifdef ENABLE_THREADS
	
	void CreateProgressDialog(const char *, bool, int, int) { }	// virtual
	void DestroyProgressDialog(void) { }				// virtual
	
#endif	// ENABLE_THREADS
	
	bool Question(const char *) { return false; }
};

/*################################################################################################*/

/// The "##color_mode" class is used to handle the color detemination details.

class color_mode
{
	protected:
	
// this is just an interface definition -> there is no relevant common data?!?!?
// this is just an interface definition -> there is no relevant common data?!?!?
// this is just an interface definition -> there is no relevant common data?!?!?

	public:
	
	color_mode(void) { }
	virtual ~color_mode(void) { }
	
// is this (void *)-method really the only possibility to give a virtual function different
// parameters without the "hiding problem"???????????????????

// should be [void *, i32s CRD_SET, fGL_a4] -> crd-set can be included in coloring??? ALSO NEED TO FIND A BETTER WAY TO INCLUDE PREFS!!!
// should be [void *, i32s CRD_SET, fGL_a4] -> crd-set can be included in coloring??? ALSO NEED TO FIND A BETTER WAY TO INCLUDE PREFS!!!
// should be [void *, i32s CRD_SET, fGL_a4] -> crd-set can be included in coloring??? ALSO NEED TO FIND A BETTER WAY TO INCLUDE PREFS!!!

	virtual void GetColor4(const void *, i32s, fGL *) = 0;
};

class color_mode_element : public color_mode
{
	protected:
	
	public:
	
	color_mode_element(void) : color_mode() { }
	~color_mode_element(void) { }
	
	void GetColor4(const void *, i32s, fGL *);
};

class color_mode_secstruct : public color_mode
{
	protected:
	
	public:
	
	color_mode_secstruct(void) : color_mode() { }
	~color_mode_secstruct(void) { }
	
	void GetColor4(const void *, i32s, fGL *);
};

class color_mode_hydphob : public color_mode
{
	protected:
	
	public:
	
	color_mode_hydphob(void) : color_mode() { }
	~color_mode_hydphob(void) { }
	
	void GetColor4(const void *, i32s, fGL *);
};

/*################################################################################################*/

/// An utility class for transparency effects...

class transparent_primitive
{
	private:
	
	fGL z_distance; void * owner;
	transparent_primitive_data * data;
	
	public:
	
	transparent_primitive(void);
	transparent_primitive(void *, transparent_primitive_data &);
	transparent_primitive(const transparent_primitive &);
	~transparent_primitive(void);
	
	bool TestOwner(void *) const;
	transparent_primitive_data * GetData(void) const;
	
	void UpdateDistance(const fGL *, const fGL *);
	
	bool operator<(const transparent_primitive &) const;
};

class transparent_primitive_data
{
	protected:
	
	fGL midpoint[3];
	
	public:
	
	transparent_primitive_data(void);
	virtual ~transparent_primitive_data(void);
	
	virtual void Render(void) = 0;
	virtual void UpdateMP(void) = 0;
	
	friend class transparent_primitive;
};

/// A triangle with 3 colors.

class tpd_tri_3c : public transparent_primitive_data
{
	protected:
	
	fGL * color[3];
	
	fGL * point[3];
	
	public:
	
	tpd_tri_3c(fGL *, fGL *, fGL *, fGL *, fGL *, fGL *);
	~tpd_tri_3c(void);
	
	void Render(void);		// virtual
	void UpdateMP(void);		// virtual
};

/// A quadrilateral with 4 colors.

class tpd_quad_4c : public transparent_primitive_data
{
	protected:
	
	fGL * color[4];
	
	fGL * point[4];
	
	public:
	
	tpd_quad_4c(fGL *, fGL *, fGL *, fGL *, fGL *, fGL *, fGL *, fGL *);
	~tpd_quad_4c(void);
	
	void Render(void);		// virtual
	void UpdateMP(void);		// virtual
};

/*################################################################################################*/

#endif	// PROJECT_H

// eof
