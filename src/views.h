// VIEWS.H : base classes for "views", to be used with the "project" class.

// Copyright (C) 1998 Tommi Hassinen, Jarno Huuskonen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef VIEWS_H
#define VIEWS_H

#include "appconfig.h"

class view;

class project_view;

class ogl_view;
class graphics_view;

class plotting_view;
class plot1d_view;
class plot2d_view;

class rcp_view;		// reaction coordinate plot

class eld_view;		// energy level diagram

class graphics_class_factory;

/*################################################################################################*/

class camera;		// camera.h

class color_mode;	// model.h
class project;		// project.h

#include "mtools.h"
#include "project.h"

// do not include any gtk/etc stuff here ; this is platform-independent, OpenGL only!!!
// do not include any gtk/etc stuff here ; this is platform-independent, OpenGL only!!!
// do not include any gtk/etc stuff here ; this is platform-independent, OpenGL only!!!

#include <vector>
using namespace std;

/*################################################################################################*/

/**	The "##view" is a base class for all views. All things dependent on operating 
	system and/or windowing environment are to be wrapped in ctor/dtor or in a set of 
	pure virtual functions.
	
	The "##view"-objects should not be created directly, but a "##view_factory" object 
	should create them instead. Different platforms/operating systems might have different 
	and tricky requirements on how windows should be created and initialized there. It's 
	up to "##view_factory" and/or "##view" itself to meet these requirements, using 
	specific callbacks or whatever is necessary...
*/

class view
{
	protected:
	
	// no relevant common data!??!!?
	// no relevant common data!??!!?
	// no relevant common data!??!!?
	
	public:
	
	view(void);
	virtual ~view(void);
	
	virtual project * GetProject(void) = 0;
	
/**	This should update the visible contents of this view. 
	The parameter is a "mode" of update, either "##immediate" (true) or 
	"##as soon as possible" (false, is default).
*/
	virtual void Update(bool = false) = 0;
	
/// This should set the window title text for this view.
	virtual void SetTitle(const char *) = 0;
};

/*################################################################################################*/

/// "##project_view" represents the project using tree and list views.

class project_view : virtual public view
{
	public:
	
	project_view(void);
	virtual ~project_view(void);
	
	virtual void UpdateAllWindowTitles(void) = 0;
	
	virtual void CameraAdded(camera *) = 0;
	virtual void CameraRemoved(camera *) = 0;
	
	virtual void LightAdded(light *) = 0;
	virtual void LightRemoved(light *) = 0;
	
	virtual void GraphicsViewAdded(graphics_view *) = 0;
	virtual void GraphicsViewRemoved(graphics_view *) = 0;
	
	virtual void PlottingViewAdded(plotting_view *) = 0;
	virtual void PlottingViewRemoved(plotting_view *) = 0;
	
	virtual void ObjectAdded(smart_object *) = 0;
	virtual void ObjectRemoved(smart_object *) = 0;
	
	virtual void BuildChainsView() = 0;
	virtual void ClearChainsView() = 0;
	
	virtual void AtomAdded(atom *) = 0;
	virtual void AtomUpdateItem(atom *) = 0;	///< updates the properties of an existing object.
	virtual void AtomRemoved(atom *) = 0;
	
	virtual void BondAdded(bond *) = 0;
	virtual void BondUpdateItem(bond *) = 0;	///< updates the properties of an existing object.
	virtual void BondRemoved(bond *) = 0;
};

/*################################################################################################*/

/**	The "##ogl_view" is a view that will use OpenGL graphics to present it's contents, 
	and is (optionally) connected to the standard set of mouse tool classes.
*/

class ogl_view : virtual public view
{
	protected:
	
/// Size of the viewing window, in pixels.
	i32s size[2];
	
	// static data about mouse/keyboard state...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	static mouse_tool::state state;
	static mouse_tool::button button;
	
	static bool shift_down;
	static bool ctrl_down;
	
	// all available mouse tools are here...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	static draw_tool tool_draw;
	static erase_tool tool_erase;
	
	static select_tool tool_select;
	static select_tool::select_mode select_mode;
	
	static zoom_tool tool_zoom;
	static clipping_tool tool_clipping;
	
	static translate_xy_tool tool_translate_xy;
	static translate_z_tool tool_translate_z;
	
	static orbit_xy_tool tool_orbit_xy;
	static orbit_z_tool tool_orbit_z;
	
	static rotate_xy_tool tool_rotate_xy;
	static rotate_z_tool tool_rotate_z;

	static measure_tool tool_measure;
	static measure_tool::measure_mode measure_mode;
	
	friend class win_app;
	friend class project;
	friend class select_tool;
	friend class zoom_tool;
	friend class clipping_tool;
	friend class translate_xy_tool;
	friend class translate_z_tool;
	friend class orbit_xy_tool;
	friend class orbit_z_tool;
	friend class rotate_xy_tool;
	friend class rotate_z_tool;
	friend class measure_tool;
	friend class win_project;
	
	public:
	
/// Pointer to the currently selected mouse tool.
	static mouse_tool * current_tool;
	
	public:
	
	ogl_view(void);
	virtual ~ogl_view(void);
	
	void SetSize(i32s, i32s);
	
	i32s GetGlutStringWidth(const char *, void * = NULL);
	void WriteGlutString2D(const char *, fGL, fGL, void * = NULL);
	void WriteGlutString3D(const char *, fGL, fGL, fGL, const obj_loc_data *, void * = NULL);
	
/**	This should do all what is necessary to make the OpenGL rendering 
	context of this view "##current", so that the following OpenGL 
	commands would affect this view.
*/
	virtual bool SetCurrent(void) = 0;
	
/// This should contain the necessary OpenGL initialization commands.
	virtual void InitGL(void) = 0;
	
/// This should contain the OpenGL commands that create the visible contents of this view.
	virtual void Render(void) = 0;
};

/*################################################################################################*/

/// "##graphics_view" presents the molecular models in OpenGL 3D graphics.

class graphics_view : virtual public ogl_view
{
	protected:
	
	camera * cam;
	i32s my_gv_number;
	
/// Dimensions of the view, in nanometers.
	fGL range[2];
	
	i32s render; i32s label;
	color_mode * colormode;
	
	bool enable_fog;
	bool accumulate;
	
	static bool quick_update;	// some static flags...
	static bool draw_info;		// some static flags...
	
	friend class draw_tool;
	friend class erase_tool;
	friend class select_tool;
	friend class zoom_tool;
	friend class clipping_tool;
	friend class translate_xy_tool;
	friend class translate_z_tool;
	friend class orbit_xy_tool;
	friend class orbit_z_tool;
	friend class rotate_xy_tool;
	friend class rotate_z_tool;
	friend class measure_tool;
	
	friend class camera;
	
	friend class project;
	
	friend class win_project_view;
	friend class win_graphics_view;		// attach/detach needs this...
	
	public:
	
	graphics_view(camera *);
	virtual ~graphics_view(void);
	
	void GetCRD(i32s *, fGL *);
	
	void InitGL(void);	// virtual
	void Render(void);	// virtual
};

/*################################################################################################*/

void * convert_cset_to_plotting_udata(project *, i32s);
void apply_udata_as_cset(project *, void *);

/**	"##plotting_view" is reserved for 1d and 2d plotting...

	It would be also nice to develop some kind of spreadsheet-program interface
	that could be used for plotting. Gnumeric?!?!?! Sci-Graphica?!?!?!
	
	If the "##plot_userdata1" is PLOT_USERDATA_STRUCTURE, the udata is interpreted to contain
	molecule's 3D coordinates (3N fGL values in an array), and the coordinate sets are updated
	using this data when data is selected.
	
	This same class works for all models, and it requires some tricks and it's against the
	principle of making different derived classes for different tasks. But since the mechanism
	of making view classes is quite complicated, it seems to make sense here...
*/

#define PLOT_USERDATA_IGNORE		0x0000
#define PLOT_USERDATA_STRUCTURE		0x0001

class plotting_view : virtual public ogl_view
{
	protected:
	
	project * mdl;
	iGLu * select_buffer;
	
	public:
	
	plotting_view(project *);
	virtual ~plotting_view(void);
	
	void InitGL(void);	// virtual
	
/**	ZoomEvent() is triggered by zoom_tool::MotionEvent(), passing delta-Y as parameter.
	It is used by plot2d_view and enlevdiag_view to scale the plots.
*/
	virtual void ZoomEvent(i32s) = 0;
	
/**	TransEvent() is triggered by translate_xy_tool::MotionEvent(), passing delta-Y as parameter.
	It is used by enlevdiag_view to scroll the diagram in Y-direction.
*/
	virtual void TransEvent(i32s) = 0;
	
/**	UserEvent() is triggered by all other mouse events except draw/erase, passing X and Y as parameters.
	It is used to handle the selection events of plotting views.
*/
	virtual void UserEvent(i32s, i32s) = 0;
};

/*################################################################################################*/

struct plot1d_data
{
	f64 c1;			// coordinate 1 (say, x-coordinate)
	f64 v;			// value
	
	void * udata;		// user data
// TODO:
// make proper operator=() for this;
// MIGHT BE NEEDED IN STL SORT!!!
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	bool operator<(const plot1d_data & p1) const
	{
		return (c1 < p1.c1);
	}
};

/// This class stores a set of 1D data, and is able to display it.

class plot1d_view : public plotting_view
{
	protected:
	
	i32s plot_userdata1;
	vector<plot1d_data> dv;
	
	char * name1;
	char * namev;
	
	f64 min1; f64 max1;
	f64 minv; f64 maxv;
	
	public:
	
	plot1d_view(project *, i32s, const char *, const char *);
	virtual ~plot1d_view(void);

	void SaveAs();
	
/**	AddData() is used to add data points.
	The points should be added in correct (increasing) order.
	See project::DoEnergyPlot1D() as an example case...
*/
	void AddData(f64, f64, void * = NULL);
	void SetCenterAndScale(void);
	
	void Render(void);		// virtual
	
	void ZoomEvent(i32s);			// virtual
	void TransEvent(i32s);			// virtual
	virtual void UserEvent(i32s, i32s);	// virtual
	
	// these are access methods that make it easier to duplicate an object...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	i32s getUD1(void) { return plot_userdata1; }
	const char * getN1(void) { return name1; }
	const char * getNV(void) { return namev; }
	
	void ImportData(plot1d_view *);		///< this is for duplicating a view for attach/detach...
};

/*################################################################################################*/

struct plot2d_data
{
	f64 c1;			// coordinate 1 (say, x-coordinate)
	f64 c2;			// coordinate 2 (say, y-coordinate)
	f64 v;			// value
	
	void * udata;		// user data
};

/// This class stores a set of 2D data, and is able to display it.

class plot2d_view : public plotting_view
{
	protected:
	
	i32s plot_userdata2;
	vector<plot2d_data> dv;
	
	char * name1;
	char * name2;
	char * namev;
	
	f64 min1; f64 max1;
	f64 min2; f64 max2;
	f64 minv; f64 maxv;
	
	void SetColor(f64);
	
	public:
	
	plot2d_view(project *, i32s, const char *, const char *, const char *);
	virtual ~plot2d_view(void);
	
/**	AddData() is used to add data points.
	Only "##square" data sets can be used, with equal number of points in both axes (x points per axis -> x*x total points).
	The points should be added in correct (increasing) order, using the following rule of ordering : index = (1st * x) + 2nd
	See project::DoEnergyPlot2D() as an example case...
*/
	void AddData(f64, f64, f64, void * = NULL);
	void SetCenterAndScale(void);
	
	void Render(void);		// virtual
	
	void ZoomEvent(i32s);			// virtual
	void TransEvent(i32s);			// virtual
	void UserEvent(i32s, i32s);		// virtual
	
	// these are access methods that make it easier to duplicate an object...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	i32s getUD2(void) { return plot_userdata2; }
	const char * getN1(void) { return name1; }
	const char * getN2(void) { return name2; }
	const char * getNV(void) { return namev; }
	
	void ImportData(plot2d_view *);		///< this is for duplicating a view for attach/detach...
};

/*################################################################################################*/

/// This is a plot1d_view that also shows approximately how bonds break/form in a reaction; see TSS!

class rcp_view : public plot1d_view
{
	protected:
	
	bool ready;
	
	vector<i32u> patoms;
	vector<bond *> rbonds;
	vector<bond *> pbonds;
	
	fGL r_treshold;
	fGL p_treshold;
	
	public:
	
	rcp_view(project *, i32s, const char *, const char *);
	~rcp_view(void);
	
	void AddPAtom(i32u p1) { patoms.push_back(p1); }
	void AddRBond(bond * p1) { rbonds.push_back(p1); }
	void AddPBond(bond * p1) { pbonds.push_back(p1); }
	
	void SortDataAndCalcRC(void);
	
	void UserEvent(i32s, i32s);		// virtual
};

/*################################################################################################*/

/// eld_view is an energy level diagram for QM models.

class eld_view : public plotting_view
{
	protected:
	
	fGL center;
	fGL scale;
	
	public:
	
	eld_view(project *);
	~eld_view(void);
	
	void SetCenterAndScale(void);
	
	void Render(void);	// virtual
	
	void ZoomEvent(i32s);			// virtual
	void TransEvent(i32s);			// virtual
	void UserEvent(i32s, i32s);		// virtual
};

/*################################################################################################*/

/**	"##graphics_class_factory" is a graphics_class capable to create also "view" objects.

	For each different platform (just like a "##view") a "##graphics_class_factory" should
	be made which produces new "view" objects for this particular platform...
*/

class graphics_class_factory
{
	protected:
	
	public:
	
	graphics_class_factory(void) { };
	virtual ~graphics_class_factory(void) { };
	
	virtual project_view * ProduceProjectView(project *) = 0;
	
	virtual graphics_view * ProduceGraphicsView(project *, camera *, bool) = 0;
	
	virtual plot1d_view * ProducePlot1DView(project *, i32s, const char *, const char *, bool) = 0;
	virtual plot2d_view * ProducePlot2DView(project *, i32s, const char *, const char *, const char *, bool) = 0;
	
	virtual rcp_view * ProduceRCPView(project *, i32s, const char *, const char *, bool) = 0;
	
	virtual eld_view * ProduceELDView(project *, bool) = 0;
};

/*################################################################################################*/

#endif	// VIEWS_H

// eof
