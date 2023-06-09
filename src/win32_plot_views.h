// GTK_PLOT_VIEWS.H : write a short description here...

// Copyright (C) 2005 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef GTK_PLOT_VIEWS_H
#define GTK_PLOT_VIEWS_H

#include "appconfig.h"

class win_plot1d_view;
class win_plot2d_view;

class win_rcp_view;		// reaction coordinate plot

class win_eld_view;		// energy level diagram

#include "win32_views.h"

/*################################################################################################*/

class win_plot1d_view : public win_ogl_view, public plot1d_view
{
	protected:
	
//	static GtkActionEntry entries[];
	static const char * ui_description;
	
	private:
	
	win_plot1d_view(win_project *, i32s, const char *, const char *, const char *);
	
	friend class win_class_factory;
	
	public:
	
	~win_plot1d_view(void);
	
	private:
	
	void ExposeEvent(void);		// virtual
	
	public:
	
	static void popup_AttachDetach(HWND, void *);
	static void popup_DeleteView(HWND, void *);
};

/*################################################################################################*/

class win_plot2d_view : public win_ogl_view, public plot2d_view
{
	protected:
	
//	static GtkActionEntry entries[];
	static const char * ui_description;
	
	private:
	
	win_plot2d_view(win_project *, i32s, const char *, const char *, const char *, const char *);
	
	friend class win_class_factory;
	
	public:
	
	~win_plot2d_view(void);
	
	private:
	
	void ExposeEvent(void);		// virtual
	
	public:
	
	static void popup_AttachDetach(HWND, void *);
	static void popup_DeleteView(HWND, void *);
};

/*################################################################################################*/

class win_rcp_view : public win_ogl_view, public rcp_view
{
	protected:
	
//	static GtkActionEntry entries[];
	static const char * ui_description;
	
	private:
	
	win_rcp_view(win_project *, i32s, const char *, const char *, const char *);
	
	friend class win_class_factory;
	
	public:
	
	~win_rcp_view(void);
	
	private:
	
	void ExposeEvent(void);		// virtual
	
	public:
	
	static void popup_DeleteView(HWND, void *);
};

/*################################################################################################*/

class win_eld_view : public win_ogl_view, public eld_view
{
	protected:
	
//	static GtkActionEntry entries[];
	static const char * ui_description;
	
	private:
	
	win_eld_view(win_project *);
	
	friend class win_class_factory;
	
	public:
	
	~win_eld_view(void);
	
	private:
	
	void ExposeEvent(void);		// virtual
	
	public:
	
	static void popup_DeleteView(HWND, void *);
};

/*################################################################################################*/

#endif	// GTK_PLOT_VIEWS_H

// eof
