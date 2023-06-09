// WIN32_VIEWS.CPP

// Copyright (C) 1998 Tommi Hassinen, Jarno Huuskonen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"
#include "../consol/resource.h"

#include "win32_views.h"	// config.h is here -> we get ENABLE-macros here...

#include "../libghemical/src/engine.h"
#include "../libghemical/src/utility.h"
#include "../consol/winghemical.h"

#include "win32_project.h"
#include "win32_project_view.h"	// temporary ; attach/detach
#include "win32_graphics_view.h"	// temporary ; attach/detach
#include "win32_plot_views.h"	// temporary ; attach/detach

//#include <gnome.h>
//#include <gdk/gdkglconfig.h>

//#include <gtk/gtk.h>

#include <strstream>
#include <algorithm>
using namespace std;


#define g_print printf
/*################################################################################################*/

win_view::win_view(win_project * p1) : view()
{
//printf("win_view::win_view(win_project * p1) : view()\n\n");
	prj = p1;
	
//	label_widget = win_label_new("this_view_has_no_title");
//	popupmenu = NULL;
}

win_view::~win_view(void)
{
}

project * win_view::GetProject(void)
{
	return prj;
}

void win_view::SetTitle(const char * p1)
{
	// if this a "detached" view, we should do the title setting differently than usual "attached" cases.
	// only gtk_graphics_view objects can be "detached". ok, this is a bit ugly solution, but see
	// gtk_graphics_view::popup_ViewsAttachDetach() for more comments...
	
// THIS IS BAD!!!!!!!!!!
// THIS IS BAD!!!!!!!!!!
// THIS IS BAD!!!!!!!!!!
// THIS IS BAD!!!!!!!!!!

/*	gtk_graphics_view * gv = dynamic_cast<gtk_graphics_view *>(this);
	if (gv != NULL && gv->detached != NULL)
	{
		char projfilename[256];
		prj->GetProjectFileName(projfilename, 256, true);
		
		char extended_title[256];
		ostrstream str(extended_title, sizeof(extended_title));
		str << projfilename << " : " << p1 << ends;
		
		gtk_window_set_title(GTK_WINDOW(gv->detached), extended_title);
		return;
	}	*/
	
//	gtk_label_set_text(GTK_LABEL(label_widget), p1);
}

/*################################################################################################*/

vector<win_ogl_view *> win_ogl_view::oglv_vector;
HGLRC * win_ogl_view::m_CurrentOGContext = NULL;

win_ogl_view::win_ogl_view(win_project * p1) : ogl_view(), win_view(p1)
{
//printf("win_ogl_view::win_ogl_view(win_project * p1) : ogl_view(), win_view(p1)\n\n");


	detached = NULL;//pointer to window?
	
	is_realized = false;
	oglv_vector.push_back(this);

	/*
	HWND hWnd = p1->GetWindow();

	SendMessage(hWnd, WM_COMMAND, ID_FILE_INITOG, 0);

	printf("win_ogl_view ");
	cout << "hWnd = " << hWnd << endl << endl;*/


#if 0
	
	gint major; gint minor;
	gdk_gl_query_version(& major, & minor);
	g_print("OpenGL extension version - %d.%d\n", major, minor);
	
	GdkGLConfig * glconfig; GdkGLConfigMode mode;
	
	mode = (GdkGLConfigMode) (GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);
	glconfig = gdk_gl_config_new_by_mode(mode);	// try double-buffered visual...
	
	if (glconfig == NULL)
	{
		g_print("*** Cannot find the double-buffered visual.\n");
		g_print("*** Trying single-buffered visual.\n");
		
		mode = (GdkGLConfigMode) (GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH);
		glconfig = gdk_gl_config_new_by_mode(mode);	// try single-buffered visual...
		
		if (glconfig == NULL)
		{
			g_print("*** No appropriate OpenGL-capable visual found.\n");
			exit (1);
		}
	}
	
	view_widget = gtk_drawing_area_new();
	gtk_widget_set_size_request(view_widget, 400, 400);	// minimum size...
	
	gtk_widget_set_gl_capability(view_widget, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE);
	
	int events = GDK_EXPOSURE_MASK;
	events |= GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK;
	events |= GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK;
	gtk_widget_set_events(GTK_WIDGET(view_widget), events);
	
	gtk_signal_connect_after(GTK_OBJECT(view_widget), "realize", GTK_SIGNAL_FUNC(RealizeHandler), NULL);	// after!!!
	
	gtk_signal_connect(GTK_OBJECT(view_widget), "expose_event", GTK_SIGNAL_FUNC(ExposeHandler), NULL);
	gtk_signal_connect(GTK_OBJECT(view_widget), "motion_notify_event", GTK_SIGNAL_FUNC(MotionNotifyHandler), NULL);
	gtk_signal_connect(GTK_OBJECT(view_widget), "button_press_event", GTK_SIGNAL_FUNC(ButtonHandler), NULL);
	gtk_signal_connect(GTK_OBJECT(view_widget), "button_release_event", GTK_SIGNAL_FUNC(ButtonHandler), NULL);
	gtk_signal_connect(GTK_OBJECT(view_widget), "configure_event", GTK_SIGNAL_FUNC(ConfigureHandler), NULL);
	
	gtk_widget_show(GTK_WIDGET(view_widget));
#endif
}

win_ogl_view::~win_ogl_view(void)
{
	vector<win_ogl_view *>::iterator it1;
	it1 = find(oglv_vector.begin(), oglv_vector.end(), this);
	
	if (it1 != oglv_vector.end())
	{
		oglv_vector.erase(it1);
	}
	else
	{
		cout << "WARNING : unknown view_widget at ~win_ogl_view()!!!" << endl;
	}
}

void win_ogl_view::Update(bool directly)
{
	if (!is_realized) g_print("WARNING : Update() called before widget was realized ; skipping...\n");
	else
	{
#if 0
//printf("InvalidateRect = directly = %d\n", directly);
			RECT rect;
			GetClientRect(view_widget,&rect);
			InvalidateRect(view_widget,&rect, true);
#else
		if (directly) ExposeEvent();
//		else win_widget_queue_draw(view_widget);
		else
		{
			RECT rect;
			GetClientRect(view_widget,&rect);
			InvalidateRect(view_widget,&rect, true);
		}
#endif
	}
	// here we update always directly. should emit an update request for indirect rendering!?!?!?
	// here we update always directly. should emit an update request for indirect rendering!?!?!?
	// here we update always directly. should emit an update request for indirect rendering!?!?!?
}
bool win_ogl_view::InitOG(HWND hWnd)
{
	PIXELFORMATDESCRIPTOR pfd =	// Описатель формата
	{
		sizeof(PIXELFORMATDESCRIPTOR),// Размер структуры
		1,							// Номер версии
		PFD_DRAW_TO_WINDOW |	// Поддержка GDI
		PFD_SUPPORT_OPENGL |	// Поддержка OpenGL
		PFD_DOUBLEBUFFER,		// Двойная буферизация
		PFD_TYPE_RGBA,			// Формат RGBA, не палитра
		24, 						// Количество плоскостей
		 							// в каждом буфере цвета
		24,	0,						// Для компоненты Red
		24,	0,						// Для компоненты Green
		24,	0,						// Для компоненты Blue
		24,	0,						// Для компоненты Alpha
		0,							// Количество плоскостей
									// буфера Accumulation
		0,			 				// То же для компоненты Red
		0,			 				// для компоненты Green
		0,							// для компоненты Blue
		0,							// для компоненты Alpha
		32, 						// Глубина Z-буфера
		0,							// Глубина буфера Stencil
		0,							// Глубина буфера Auxiliary
		0,				 			// Теперь игнорируется
		0,							// Количество плоскостей
		0,							// Теперь игнорируется
		0,							// Цвет прозрачной маски
		0							// Теперь игнорируется
	};



	//====== Добываем дежурный контекст
	this->m_hdc = ::GetDC(hWnd);
//MessageBox(hWnd,"2","On Create",0);

	//====== Просим выбрать ближайший совместимый формат
	int iD = ChoosePixelFormat(this->m_hdc, &pfd);
	if ( !iD )
	{
		MessageBox(hWnd,"ChoosePixelFormat::Error","On Create",0);
		return 0;
	}

	//GL_RGBA

	//====== Пытаемся установить этот формат
	if ( !SetPixelFormat (this->m_hdc, iD, &pfd) )
	{
		MessageBox(hWnd,"SetPixelFormat::Error","On Create winsuf",0);
		return 0;
	}


	//====== Пытаемся создать контекст передачи OpenGL
	if ( !(this->m_hRC = wglCreateContext (this->m_hdc)))
	{
		MessageBox(hWnd,"wglCreateContext::Error","On Create",0);
		return 0;
	}

	//====== Пытаемся выбрать его в качестве текущего
	if ( !wglMakeCurrent (this->m_hdc, this->m_hRC))
	{
		MessageBox(hWnd,"wglMakeCurrent::Error","On Create",0);
		return 0;
	}
	else
		win_ogl_view::m_CurrentOGContext = &this->m_hRC;


	/*//====== Пытаемся выбрать его в качестве текущего
	if ( !wglMakeCurrent (m_hdc, m_hRC))
	{
		MessageBox(hWnd,"wglMakeCurrent::Error","On Create",0);
		return 0;
	}*/




//int pixelFormat = ChoosePixelFormat (hDC, &pfd);
int pixelFormat = iD;

PIXELFORMATDESCRIPTOR pfd_new;
DescribePixelFormat (m_hdc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd_new);


int generic_format = pfd_new.dwFlags & PFD_GENERIC_FORMAT;
int generic_accelerated = pfd_new.dwFlags & PFD_GENERIC_ACCELERATED;

if (generic_format && ! generic_accelerated)
{
// режим программной эмуляции; всю работу выполняет центральный процессор
// MessageBox(0,"режим программной эмуляции; всю работу выполняет центральный процессор","",0);
}
else if (generic_format && generic_accelerated)
{
// MCD-драйвер; аппаратно реализуется только часть функций ускорения
// MessageBox(0,"MCD-драйвер; аппаратно реализуется только часть функций ускорения","",0);
}
else if (! generic_format && ! generic_accelerated)
{
// полноценный ICD-драйвер с функциями ускорения
// MessageBox(0,"полноценный ICD-драйвер с функциями ускорения","",0);
}

}

bool win_ogl_view::SetCurrent(void)
{
//printf("win_ogl_view::SetCurrent(void)\n\n");
	if (!is_realized)
	{
		g_print("WARNING : SetCurrent() called before widget was realized ; skipping...\n");
		return false;
	}
	else
	{
//		GdkGLContext * glcontext = win_widget_get_gl_context(view_widget);
//		GdkGLDrawable * gldrawable = win_widget_get_gl_drawable(view_widget);
		
	//	gdk_gl_drawable_wait_gl()	what are these???
	//	gdk_gl_drawable_wait_gdk()	what are these???

		if (win_ogl_view::m_CurrentOGContext != &this->m_hRC)
		{
			//====== Пытаемся выбрать его в качестве текущего
			if ( wglMakeCurrent (this->m_hdc, this->m_hRC))
			{
				win_ogl_view::m_CurrentOGContext = &this->m_hRC;
				return true;
			}
			else
			{
				MessageBox(this->detached,"wglMakeCurrent::Error","On Create",0);
				g_print("ERROR : wglMakeCurrent() failed in SetCurrent().\n");
				return false;
			}
		}
		else 
			return true;
		/*	if (!gdk_gl_drawable_make_current(gldrawable, glcontext))
		{
			g_print("ERROR : gdk_gl_drawable_make_current() failed in SetCurrent().\n");
			return false;
		}
		else
		{
			return true;
		}*/
	}
	return false;
}

void win_ogl_view::RealizeHandler(HWND widget, void *)
{
	win_ogl_view * oglv = GetOGLV(widget);
	if (!oglv) cout << "Unknown ID in RealizeHandler !!!" << endl;
	else
	{
		oglv->is_realized = true;
		oglv->InitGL();
	}
}

int win_ogl_view::ExposeHandler(HWND widget/*, GdkEventExpose * */)		// EVENT_HANDLER
{
	win_ogl_view * oglv = GetOGLV(widget);
	if (!oglv) cout << "Unknown ID in ExposeHandler !!!" << endl;
	else
	{
#ifdef ENABLE_THREADS
//gdk_threads_enter();
#endif	// ENABLE_THREADS
		
		oglv->ExposeEvent();
		
#ifdef ENABLE_THREADS
//gdk_threads_leave();
#endif	// ENABLE_THREADS
	}
	
//	return FALSE;	// why is that???
	return TRUE;	// why is that???
}

int button_event_lost_counter = 0;

int win_ogl_view::ButtonHandler(HWND widget, UINT message, WPARAM wParam, LPARAM lParam)

// EVENT_HANDLER

{
	int x = LOWORD(lParam); // horizontal position of pointer 
	int y = HIWORD(lParam); // vertical position of pointer 		}
	if (x > 32768) x -= 65536;
	if (y > 32768) y -= 65536;
/*
	if( wParam &  MK_LBUTTON)
	{
	}
	//====== Если одновременно была нажата Ctrl,
	if (wParam & MK_CONTROL)
	{
	}
	if (wParam & MK_SHIFT )
	{
	}
	if	(wParam &  MK_RBUTTON )
	{
	}
*/	



	win_ogl_view * oglv = GetOGLV(widget);
	if (!oglv) cout << "Unknown ID in ButtonHandler !!!" << endl;
	else
	{
		mouse_tool::button tmpb; i32u tmps1;
		if( wParam &  MK_LBUTTON)
		{
			tmpb = mouse_tool::Left;
			tmps1 = MK_LBUTTON;
		}
		if( wParam &  MK_RBUTTON)
		{			
			tmpb = mouse_tool::Right;
			tmps1 = MK_RBUTTON;
		}
		if( wParam &  MK_MBUTTON)
		{			
			tmpb = mouse_tool::Middle;
			tmps1 = MK_MBUTTON;
		}
		
		mouse_tool::state tmps2;// = (eb->state & tmps1) ? mouse_tool::Up : mouse_tool::Down;
		switch(message)
		{
		case WM_LBUTTONDOWN:
			tmpb = mouse_tool::Left;
			tmps2 = mouse_tool::Down;
			break;
		case WM_LBUTTONUP:
			tmpb = mouse_tool::Left;
			tmps2 = mouse_tool::Up;
			break;
		case WM_RBUTTONDOWN:
			tmpb = mouse_tool::Right;
			tmps2 = mouse_tool::Down;
			break;
		case WM_RBUTTONUP:
			tmpb = mouse_tool::Right;
			tmps2 = mouse_tool::Up;
			break;
		case WM_MBUTTONDOWN:
			tmpb = mouse_tool::Middle;
			tmps2 = mouse_tool::Down;
			break;
		case WM_MBUTTONUP:
			tmpb = mouse_tool::Middle;
			tmps2 = mouse_tool::Up;
			break;
		}
		
		if (tmps2 == mouse_tool::Down)
		{
			if (button == mouse_tool::None)
			{
				if (tmpb == mouse_tool::Right)
				{
// the popup menu is created here. pointer to the win_drawing_area
// widget is given as "user_data", and it is also passed to the popup
// hander callback function (instead of the original value).
// 2001-08-22 quick fix : check that we open the popup for
// a graphics view only (not for energy level diagrams).
//					graphics_view * gv = dynamic_cast<graphics_view *>(window);
//					if (gv == NULL) return TRUE;	// was not a graphics_view!!!
//					win_graphics_view * ggv = dynamic_cast<win_graphics_view *>(gv);
//					if (ggv == NULL) return TRUE;	// FIXME so that all views have their own popups!!!
//				//	win_popup_menu_do_popup(,NULL, NULL, NULL, window->view_widget, NULL);
//				//	win_menu_popup(win_MENU(GetWindow(widget)->prj->popupmenu), NULL, NULL, NULL, NULL, eb->button, eb->time);
//					win_menu_popup(win_MENU(ggv->popupmenu), NULL, NULL, NULL, NULL, eb->button, eb->time);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					win_ogl_view * oglv = GetOGLV(widget);
/*					if (oglv != NULL && oglv->popupmenu != NULL)
					{
						win_menu_popup(win_MENU(oglv->popupmenu), NULL, NULL, NULL, NULL, eb->button, eb->time);
					}
*/					
					return TRUE;
				}
				
				button = tmpb;
				
				shift_down = (wParam & MK_SHIFT) ? true : false;
				ctrl_down = (wParam & MK_CONTROL) ? true : false;
				
				state = mouse_tool::Down;
				
			//	cout << "button_event_D " << button << " " << state << " " << (ogl_view *) window << endl;
				current_tool->ButtonEvent((ogl_view *) oglv, (i32s) x, (i32s) y);
				button_event_lost_counter = 0;	// this is for exceptions, see below...
			}
		}
		else
		{
			if (button == mouse_tool::Left && tmpb != mouse_tool::Left) return TRUE;
			if (button == mouse_tool::Middle && tmpb != mouse_tool::Middle) return TRUE;
			if (button == mouse_tool::Right && tmpb != mouse_tool::Right) return TRUE;
			
			state = mouse_tool::Up;
			
		//	cout << "button_event_U " << button << " " << state << " " << (ogl_view *) window << endl;
			current_tool->ButtonEvent((ogl_view *) oglv, (i32s) x, (i32s) y);
			
			button = mouse_tool::None;
		}
	}
	
	return TRUE;
}

int win_ogl_view::ConfigureHandler(HWND widget/*, GdkEventConfigure * */)		// EVENT_HANDLER
{
	win_ogl_view * oglv = GetOGLV(widget);
	if (!oglv) cout << "Unknown ID in ConfigureHandler !!!" << endl;
	else
	{
#ifdef ENABLE_THREADS
//gdk_threads_enter();
#endif	// ENABLE_THREADS
		
		oglv->SetCurrent();
		RECT rect;
		GetClientRect(widget, &rect);
		oglv->SetSize(rect.right - rect.left, rect.bottom - rect.top);
		
#ifdef ENABLE_THREADS
//gdk_threads_leave();
#endif	// ENABLE_THREADS
	}
	
	return TRUE;
}

int win_ogl_view::MotionNotifyHandler(HWND widget, WPARAM wParam, LPARAM lParam)	// EVENT_HANDLER
{
#ifdef ENABLE_THREADS
//gdk_threads_enter();
#endif	// ENABLE_THREADS

	int x = LOWORD(lParam); // horizontal position of pointer 
	int y = HIWORD(lParam); // vertical position of pointer 		}
	if (x > 32768) x -= 65536;
	if (y > 32768) y -= 65536;
/*
	if( wParam &  MK_LBUTTON)
	{
	}
	//====== Если одновременно была нажата Ctrl,
	if (wParam & MK_CONTROL)
	{
	}
	if (wParam & MK_SHIFT )
	{
	}
	if	(wParam &  MK_RBUTTON )
	{
	}
*/	
//	int x; int y; GdkModifierType mbstate;
//	if (event->is_hint) gdk_window_get_pointer(event->window, & x, & y, & mbstate);
//	else { x = (int) event->x; y = (int) event->y; mbstate = (GdkModifierType) event->state; }
	
	// here it is good to check if we have lost a "mouse button up" message.
	// it can happen if a user moves the mouse outside to the graphics window,
	// and then changes the mousebutton state.
	
	// if we think that a mouse button should be down, but GTK+ says it's not,
	// then immediately send a "mouse button down" message...
	
	bool no_buttons_down = !(wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON));
	if (no_buttons_down && button != mouse_tool::None)
	{
		button_event_lost_counter++;
		if (button_event_lost_counter > 1)
		{
			win_ogl_view * oglv = GetOGLV(widget);
			if (!oglv) cout << "Unknown ID in MotionNotifyHandler !!!" << endl;
			else
			{
				cout << "WARNING ; a mouse-button-up event was lost!" << endl;
				
				state = mouse_tool::Up;
				
			//	cout << "button_event_U " << button << " " << state << " " << (ogl_view *) window << endl;
				current_tool->ButtonEvent((ogl_view *) oglv, (i32s) x, (i32s) y);
				
				button = mouse_tool::None;
			}
		}
	}
	
	// the normal operation starts here...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	if (button != mouse_tool::None)
	{
		win_ogl_view * oglv = GetOGLV(widget);
		if (!oglv) cout << "Unknown ID in MotionNotifyHandler !!!" << endl;
		else
		{
		//	cout << "motion_event " << button << " " << state << endl;
			current_tool->MotionEvent((ogl_view *) oglv, x, y);
		}
	}
	
#ifdef ENABLE_THREADS
//gdk_threads_leave();
#endif	// ENABLE_THREADS
	
	return TRUE;
}

int win_ogl_view::DetachedDeleteHandler(HWND)
{
	// when we create detached view windows as win_WINDOW_TOPLEVEL, the window will have the "close" button
	// at titlebar. now if the user presses the "close" button, the window-closing sequence will start.
	// we will grab the resulting delete_event here and return TRUE, that will deny the user's request to
	// close the window. the user should use the stardard popup-way of closing the window...
	
	return TRUE;
}

win_ogl_view * win_ogl_view::GetOGLV(HWND widget)
{
//printf("win_ogl_view * win_ogl_view::GetOGLV(HWND widget)\n");
//cout << "widget = " << widget << endl;
	vector<win_ogl_view *>::iterator it1 = oglv_vector.begin();
	while (it1 != oglv_vector.end())
	{
		vector<win_ogl_view *>::iterator it2 = it1++;
//cout << "(* it2)->view_widget " << (* it2)->view_widget << endl;
		if ((* it2)->view_widget == widget) return (* it2);
	}
	
	// return NULL if the search failed...
	// return NULL if the search failed...
	// return NULL if the search failed...
	
	return NULL;
}
/*################################################################################################*/

win_class_factory * win_class_factory::instance = NULL;
singleton_cleaner<win_class_factory> win_class_factory_cleaner(win_class_factory::GetInstance());

win_class_factory::win_class_factory(void) : graphics_class_factory()
{
}

win_class_factory::~win_class_factory(void)
{
}

win_class_factory * win_class_factory::GetInstance(void)
{
	if (instance != NULL) return instance;
	else return (instance = new win_class_factory());
}

project_view * win_class_factory::ProduceProjectView(project * prj1)
{
//printf("project_view * win_class_factory::ProduceProjectView(project * prj1)\n\n");
	win_project * prj2 = dynamic_cast<win_project *>(prj1);
	
	win_project_view * pv = new win_project_view(prj2);
//	win_notebook_append_page(win_NOTEBOOK(prj2->notebook_widget), pv->view_widget, pv->label_widget);
//	win_notebook_set_page(win_NOTEBOOK(prj2->notebook_widget), -1);		// activate the last page. // currently has no effect...
	
//	win_widget_show(win_WIDGET(prj2->notebook_widget));
	return pv;
}

graphics_view * win_class_factory::ProduceGraphicsView(project * prj1, camera * cam, bool detach)
{
//printf("graphics_view * win_class_factory::ProduceGraphicsView(project * prj1, camera * cam, bool detach)\n\n");

	win_project * prj2 = dynamic_cast<win_project *>(prj1);
	win_graphics_view * gv = new win_graphics_view(prj2, cam);

	if (detach)
	{
//		gv->detached = win_window_new(win_WINDOW_TOPLEVEL);
//		win_window_set_default_size(win_WINDOW(gv->detached), 400, 400);
		
//		win_container_add(win_CONTAINER(gv->detached), gv->view_widget);
//		win_signal_connect(win_OBJECT(gv->detached), "delete_event", win_SIGNAL_FUNC(win_graphics_view::DetachedDeleteHandler), NULL);
		
//		win_widget_show(gv->detached);
	}
	else
	{
//		win_notebook_append_page(win_NOTEBOOK(prj2->notebook_widget), gv->view_widget, gv->label_widget);
//		win_notebook_set_page(win_NOTEBOOK(prj2->notebook_widget), -1);		// activate the last page.
	}
	return gv;
}

plot1d_view * win_class_factory::ProducePlot1DView(project * prj1, i32s ud1, const char * s1, const char * sv, const char * title, bool detach)
{
	win_project * prj2 = dynamic_cast<win_project *>(prj1);
	win_plot1d_view * p1dv = new win_plot1d_view(prj2, ud1, s1, sv, title);
	
	if (detach)
	{
//		p1dv->detached = win_window_new(win_WINDOW_TOPLEVEL);
//		win_window_set_default_size(win_WINDOW(p1dv->detached), 400, 400);
		
//		win_container_add(win_CONTAINER(p1dv->detached), p1dv->view_widget);
//		win_signal_connect(win_OBJECT(p1dv->detached), "delete_event", win_SIGNAL_FUNC(win_plot1d_view::DetachedDeleteHandler), NULL);
		
//		win_widget_show(p1dv->detached);
	}
	else
	{
//		win_notebook_append_page(win_NOTEBOOK(prj2->notebook_widget), p1dv->view_widget, p1dv->label_widget);
//		win_notebook_set_page(win_NOTEBOOK(prj2->notebook_widget), -1);		// activate the last page.
	}
	
	return p1dv;
}

plot2d_view * win_class_factory::ProducePlot2DView(project * prj1, i32s ud2, const char * s1, const char * s2, const char * sv, const char * title, bool detach)
{
	win_project * prj2 = dynamic_cast<win_project *>(prj1);
	win_plot2d_view * p2dv = new win_plot2d_view(prj2, ud2, s1, s2, sv, title);
	
	if (detach)
	{
//		p2dv->detached = win_window_new(win_WINDOW_TOPLEVEL);
//		win_window_set_default_size(win_WINDOW(p2dv->detached), 400, 400);
		
//		win_container_add(win_CONTAINER(p2dv->detached), p2dv->view_widget);
//		win_signal_connect(win_OBJECT(p2dv->detached), "delete_event", win_SIGNAL_FUNC(win_plot2d_view::DetachedDeleteHandler), NULL);
		
//		win_widget_show(p2dv->detached);
	}
	else
	{
//		win_notebook_append_page(win_NOTEBOOK(prj2->notebook_widget), p2dv->view_widget, p2dv->label_widget);
//		win_notebook_set_page(win_NOTEBOOK(prj2->notebook_widget), -1);		// activate the last page.
	}
	
	return p2dv;
}

rcp_view * win_class_factory::ProduceRCPView(project * prj1, i32s ud1, const char * s1, const char * sv, const char * title, bool detach)
{
	win_project * prj2 = dynamic_cast<win_project *>(prj1);
	win_rcp_view * rcpv = new win_rcp_view(prj2, ud1, s1, sv, title);
	
	if (detach)
	{
//		rcpv->detached = win_window_new(win_WINDOW_TOPLEVEL);
//		win_window_set_default_size(win_WINDOW(rcpv->detached), 400, 400);
		
//		win_container_add(win_CONTAINER(rcpv->detached), rcpv->view_widget);
//		win_signal_connect(win_OBJECT(rcpv->detached), "delete_event", win_SIGNAL_FUNC(win_rcp_view::DetachedDeleteHandler), NULL);
		
//		win_widget_show(rcpv->detached);
	}
	else
	{
//		win_notebook_append_page(win_NOTEBOOK(prj2->notebook_widget), rcpv->view_widget, rcpv->label_widget);
//		win_notebook_set_page(win_NOTEBOOK(prj2->notebook_widget), -1);		// activate the last page.
	}
	
	return rcpv;
}

eld_view * win_class_factory::ProduceELDView(project * mdl, bool detach)
{
	win_project * prj2 = dynamic_cast<win_project *>(mdl);
	win_eld_view * eldv = new win_eld_view(prj2);
	
	if (detach)
	{
//		eldv->detached = win_window_new(win_WINDOW_TOPLEVEL);
//		win_window_set_default_size(win_WINDOW(eldv->detached), 400, 400);
		
//		win_container_add(win_CONTAINER(eldv->detached), eldv->view_widget);
//		win_signal_connect(win_OBJECT(eldv->detached), "delete_event", win_SIGNAL_FUNC(win_rcp_view::DetachedDeleteHandler), NULL);
		
//		win_widget_show(eldv->detached);
	}
	else
	{
//		win_notebook_append_page(win_NOTEBOOK(prj2->notebook_widget), eldv->view_widget, eldv->label_widget);
//		win_notebook_set_page(win_NOTEBOOK(prj2->notebook_widget), -1);		// activate the last page.
	}
	
//	win_widget_show(win_WIDGET(prj2->notebook_widget));
	
	return eldv;
}
/*################################################################################################*/

// eof
