// Win32_VIEWS.H : gtk/bonobo implementations of "view" classes.

// Copyright (C) 1998 Tommi Hassinen, Jarno Huuskonen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef WIN_VIEWS_H
#define WIN_VIEWS_H

#include "appconfig.h"

class win_view;
class win_ogl_view;

class win_class_factory;

/*################################################################################################*/

class camera;		// camera.h

class color_mode;	// model.h
class project;		// project.h

#include "mtools.h"
#include "win32_project.h"

#include <windows.h>
//#include <gtk/gtkgl.h>

#include <vector>
using namespace std;

/*################################################################################################*/

/// GTK-dependent part of "##view" class.

class win_view : virtual public view
{
	protected:
	
	win_project * prj;
	
	HWND view_widget;
//	HWND label_widget;
	
//	HWND popupmenu;
	
	public:
	HWND GetWindow(void){return view_widget;}
	
	win_view(win_project *);
	virtual ~win_view(void);
	
	project * GetProject(void);		// virtual
	void SetTitle(const char *);		// virtual
};

/*################################################################################################*/

/// A GTK-version of "##ogl_view".

class win_ogl_view : public win_view, virtual public ogl_view
{
	private:
	
	static vector<win_ogl_view *> oglv_vector;
	
	protected:
	
	bool is_realized;
	
/**	If the "##detached" pointer is different from NULL, it means that the view is
	displayed on a separate window (and not as a part of the GtkNotebook).
*/
	HWND detached;

	
	friend class win_plot1d_view;
	friend class win_plot2d_view;
	friend class win_rcp_view;
	friend class win_eld_view;
	friend class win_graphics_view;
	
public:
	// my insertions ########################################
	SetWindow (HWND hWnd){detached = hWnd;}

	HGLRC		m_hRC;
	HDC			m_hdc; 

	static HGLRC * m_CurrentOGContext;

	bool InitOG(HWND hWnd);
	// my insertions ########################################
	
	win_ogl_view(win_project *);
	virtual ~win_ogl_view(void);
	
	void Update(bool);			// virtual
	bool SetCurrent(void);			// virtual
	
	int MotionNotifyHandler(HWND widget, WPARAM wParam, LPARAM lParam);	// EVENT_HANDLER
	int ButtonHandler(HWND widget, UINT message, WPARAM wParam, LPARAM lParam);

	static int ConfigureHandler(HWND /*, GdkEventConfigure * */);
	
	virtual void ExposeEvent(void) = 0;
	static void RealizeHandler(HWND, void *);
	private:
	
	static int ExposeHandler(HWND /*,GdkEventExpose * */);
	
	protected:
	
	static int DetachedDeleteHandler(HWND/*GtkWidget *, GdkEvent */);
	
	static win_ogl_view * GetOGLV(HWND);
};

/*################################################################################################*/

/// This class has a private constructor, so you can't use this in the usual way; use the GetInstance() instead.

class win_class_factory : public graphics_class_factory
{
	private:
	public: //test this
	
	static win_class_factory * instance;
	win_class_factory(void);
	
	public:
	
	~win_class_factory(void);
	static win_class_factory * GetInstance(void);
	
	project_view * ProduceProjectView(project *);				// virtual
	
	graphics_view * ProduceGraphicsView(project *, camera *, bool);		// virtual
	
	plot1d_view * ProducePlot1DView(project *, i32s, const char *, const char *, bool);			// virtual
	plot2d_view * ProducePlot2DView(project *, i32s, const char *, const char *, const char *, bool);	// virtual
	
	rcp_view * ProduceRCPView(project *, i32s, const char *, const char *, bool);		// virtual
	
	eld_view * ProduceELDView(project *, bool);						// virtual
};

/*################################################################################################*/

#endif	// WIN_VIEWS_H

// eof
