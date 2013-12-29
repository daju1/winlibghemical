// win_PLOT_VIEWS.CPP

// Copyright (C) 2005 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"
#include "../consol/resource.h"
#include "../consol/winghemical.h"

#include "../src/win32_app.h"
#include "../src/win32_graphics_view.h"
LPCTSTR lpszPlotName;
extern HINSTANCE hInst;								// current instance

LRESULT CALLBACK WndProcGhemicalPlot(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	//static	HGLRC		m_hRC;				//  онтекст OpenGL
	//static	HDC			m_hdc; 			//  онтекст Windows


	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
			case ID_FILE_SAVEAS32867:
				{			
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						plot1d_view * p1dv = dynamic_cast<plot1d_view *>(wgrv);
						if (p1dv)
						{
							p1dv->SaveAs();
						}
					}
					//######################################################
				}
				break;
			case ID_VIEW_UPDATEALLVIEWS:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						//wgrv->RealizeHandler(hWnd, NULL);
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							w_prj->UpdateAllViews();
						}
					}
					//######################################################
				}
				break;
			case ID_FILE_INITOG2:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						wgrv->RealizeHandler(hWnd, NULL);
						wgrv->InitGL();
/*
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							w_prj->InitGL();
						}*/
					}
					//######################################################
				}
				break;
			case ID_FILE_INITOG:
				{
					win_ogl_view * wogv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (! wogv)
					{
						MessageBox(hWnd,"! win_ogl_view\nError","On Create",0);
						return 0;
					}
					wogv->InitOG(hWnd);
				}
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
      case WM_CREATE  :
              {
				  //##################################################
				  CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
				  SetWindowLong(hWnd,	GWL_USERDATA,(LONG)pcs->lpCreateParams);
				  //##################################################
//				  printf("WM_CREATE ");
//				  cout << "hWnd = " << hWnd << endl << endl;
//				  SendMessage(hWnd, WM_COMMAND, ID_FILE_INITOG, 0);
				  //##################################################
				  //##################################################
				  win_ogl_view * wogv = 
					  (win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
				  if (wogv)
				  {
					  wogv->SetWindow(hWnd);

					  /*project * prj = wogv->GetProject();
					  win_project * w_prj = dynamic_cast<win_project *>(prj);
					  if (w_prj)
					  {
						  w_prj->paned_widget = hWnd;
						  printf("w_prj->paned_widget = 0x%08x\n", w_prj->paned_widget);
					  }*/
				  }
				  //##################################################


              }
              break;
		case WM_ERASEBKGND:
			{
				return true;
			}
			break;
		case WM_SIZE:
			{
				// ###########################################################
				win_ogl_view * woglv = 
					(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
				if (woglv)
				{
					woglv->ConfigureHandler(hWnd);
				}
				// ###########################################################
				long lrez = DefWindowProc(hWnd, message, wParam, lParam);
				return lrez;
			}
			break;
		case WM_PAINT:
			{
				//printf("WM_PAINT \n\n");
				hdc = BeginPaint(hWnd, &ps);
				// ###########################################################
				win_graphics_view * woglv = 
					(win_graphics_view *)GetWindowLong(hWnd,GWL_USERDATA);
				if (woglv)
				{	
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					
					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();

					woglv->ExposeEvent();

					SwapBuffers(woglv->m_hdc);
				}

				// ###########################################################
				EndPaint(hWnd, &ps);
			}
			break;
		case WM_RBUTTONDOWN :
		case WM_LBUTTONDOWN :
		case WM_LBUTTONUP :
		case WM_RBUTTONUP :
			{
				// ###########################################################
				win_ogl_view * woglv = 
					(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
				if (woglv)
				{
					woglv->ButtonHandler(hWnd, message, wParam, lParam);
				}
				// ###########################################################
				RECT rect;
				GetClientRect(hWnd,&rect);
				InvalidateRect(hWnd,&rect, true);
			}
			break;
		case WM_MOUSEMOVE :
			{
				// ###########################################################
				win_ogl_view * woglv = 
					(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
				if (woglv)
				{
					woglv->MotionNotifyHandler(hWnd, wParam, lParam);	
				}
				// ###########################################################
				RECT rect;
				GetClientRect(hWnd,&rect);
				InvalidateRect(hWnd,&rect, true);
						
			}
			break;

		case WM_DESTROY:
			//PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}


HWND MakeGhemicalPlot(HWND hwnd, void * lParam, char * win_name)
{
	printf("MakeGhemicalPlot lParam = %x\n", lParam);
	WNDCLASSEX wc;
	// –егистраци€ класса главного окна приложени€.
	//............................................
	//........................................................................
	wc.style         	= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   	= (WNDPROC)WndProcGhemicalPlot;
	wc.cbClsExtra    	= 0;
	wc.cbWndExtra    	= 0;
	wc.hInstance     	= hInst;
	wc.hIcon         	= LoadIcon(hInst, (LPCTSTR)IDI_GHEMICAL);
	wc.hCursor       	= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground 	= (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName		= (LPCSTR)IDR_MENU_PLOT;
	wc.lpszClassName	= "Ghemical Plot";
	wc.cbSize        	= sizeof(WNDCLASSEX);
	wc.hIconSm       	= LoadIcon(wc.hInstance, (LPCTSTR)IDI_SMALL);

//printf("MakeGhemicalPlot 2\n");

	//**************************************

	if ( !RegisterClassEx( &wc ) )
	{
		DWORD dwError = GetLastError();
		if(1410 != dwError)// класс уже существует
		{
			MessageBox(0,"!RegisterClassEx","PrintPreview",0);
			LPVOID lpMsgBuf;

			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dwError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);
			char str[100];
			sprintf(str, "RegisterClassEx dwError = %x",dwError);
			::MessageBox(NULL, (LPCTSTR)lpMsgBuf, str, MB_OK);
		}
		else
		{
			//MessageBox(0,"класс уже существует","WinSurf",0);
		}
	}






	RECT r;
	GetWindowRect(hwnd, &r);

	LONG W = r.right - r.left;
	LONG H = r.bottom - r.top;
	LONG w = W/4;
	LONG h = H/2;




	HWND hPlotWnd = CreateWindow( "Ghemical Plot",
		win_name,
		WS_OVERLAPPEDWINDOW,
		r.left+W-w, r.top+H-h,
		w, h,
		hwnd,	
		NULL,
		hInst,
		(void *) lParam
		//(void *) GetWindowLongPtr(hwnd,GWLP_USERDATA)
		);

	if( ! hPlotWnd)
	{
		MessageBox(0,"! hPlotWnd","",0);
	}

	ShowWindow(hPlotWnd, 1);

	return hPlotWnd;
}






#include "win32_plot_views.h"

/*################################################################################################*/

/*GtkActionEntry win_plot1d_view::entries[] =
{
	{ "AttachDetachPlot1D", NULL, "Attach/Detach This View", NULL, "Select whether this view is attached or in an independent window", (GCallback) win_plot1d_view::popup_AttachDetach },
	{ "DeleteViewPlot1D", NULL, "Delete View", NULL, "Delete this view", (GCallback) win_plot1d_view::popup_DeleteView }
};
*/
const char * win_plot1d_view::ui_description =
"<ui>"
"  <popup name='gp1dvMenu'>"
"    <menuitem action='AttachDetachPlot1D'/>"
"    <separator/>"
"    <menuitem action='DeleteViewPlot1D'/>"
"  </popup>"
"</ui>";

win_plot1d_view::win_plot1d_view(win_project * p1, i32s p2, const char * s1, const char * sv)
	: win_ogl_view(p1), plot1d_view(p1, p2, s1, sv), ogl_view()
{
//printf("win_plot1d_view\n");
	//здесь надо создать окно графика !!! наподобие как в конструкторе класса 
	//win_graphics_view::win_graphics_view(win_project * p1, camera * p2) 
	//:	win_ogl_view(p1), graphics_view(p2), ogl_view()




	this->view_widget = MakeGhemicalPlot(prj->GetWidget(), (void*) dynamic_cast<win_ogl_view*>(this), "plot 1D");

	//cout << "view_widget = " << view_widget << endl;
	
	SendMessage(view_widget, WM_COMMAND, ID_FILE_INITOG, 0);
	SendMessage(view_widget, WM_COMMAND, ID_FILE_INITOG2, 0);

	ConfigureHandler(view_widget);


//	win_label_set_text(GTK_LABEL(label_widget), "1D plot view");	// fixme!!!
	
//	GtkActionGroup * action_group = win_action_group_new("gp1dvActions");
//	win_action_group_add_actions(action_group, entries, G_N_ELEMENTS(entries), win_WIDGET(view_widget));
	
//	win_ui_manager_insert_action_group(win_app::GetUIManager(), action_group, 0);
	
//	GError * error = NULL;
//	if (!win_ui_manager_add_ui_from_string(win_app::GetUIManager(), ui_description, -1, & error))
//	{
//		g_message("Building gp1dv menu failed : %s", error->message);
//		g_error_free(error); exit(EXIT_FAILURE);
//	}
	
	// set win_view::popupmenu so that win_ogl_view::ButtonHandler() will display it...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
//	popupmenu = win_ui_manager_get_widget(win_app::GetUIManager(), "/gp1dvMenu");
}

win_plot1d_view::~win_plot1d_view(void)
{
}

void win_plot1d_view::ExposeEvent(void)
{
//	GdkGLContext * glcontext = win_widget_get_gl_context(view_widget);
//	GdkGLDrawable * gldrawable = win_widget_get_gl_drawable(view_widget);

	SetCurrent(); Render(); glFlush();
	
//	if (gdk_gl_drawable_is_double_buffered(gldrawable)) gdk_gl_drawable_swap_buffers(gldrawable);
//	else glFlush();
	
//win_gl_area_swapbuffers(win_GL_AREA(view_widget));	// older way??? see win_graphics_view::ExposeEvent()
}

void win_plot1d_view::popup_AttachDetach(HWND widget, void *)
{
	win_ogl_view * oglv = GetOGLV(widget);
	plot1d_view * old_p1dv = dynamic_cast<plot1d_view *>(oglv);
	if (old_p1dv == NULL)
	{
		cout << "cast failed at win_plot1d_view::popup_AttachDetach()." << endl;
		return;
	}
	
	// first we create a new view using the requested mode...
	
	i32s ud1 = old_p1dv->getUD1();
	const char * s1 = old_p1dv->getN1();
	const char * sv = old_p1dv->getNV();
	bool flag = (oglv->detached == NULL);
	
	plot1d_view * new_p1dv = oglv->prj->AddPlot1DView(ud1, s1, sv, flag);
	
	// try to retain as much data/settings as possible.
	
	new_p1dv->ImportData(old_p1dv);
	
	// ...and then simply destroy the old one; calling popup_ViewsDeleteView()
	// below this is the same as selecting "Delete This View" from the popup-menu.
	
	popup_DeleteView(widget, NULL);
//	new_p1dv->GetProject()->UpdateAllWindowTitles();
}

void win_plot1d_view::popup_DeleteView(HWND widget, void *)
{
	win_ogl_view * oglv = GetOGLV(widget);
	
	if (oglv->detached != NULL)
	{
//		win_widget_destroy(win_WIDGET(oglv->detached));
	}
	else
	{
//		gint page = win_notebook_page_num(win_NOTEBOOK(oglv->prj->notebook_widget), oglv->view_widget);
//		win_notebook_remove_page(win_NOTEBOOK(oglv->prj->notebook_widget), page);
	}
	
	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv == NULL) { cout << "cast failed at win_plot1d_view::popup_DeleteView()!" << endl; exit(EXIT_FAILURE); }
	
	oglv->prj->RemovePlottingView(pv);
	// TODO : delete the object and release memory ; is it done already???
}

/*################################################################################################*/
/*
GtkActionEntry win_plot2d_view::entries[] =
{
	{ "AttachDetachPlot2D", NULL, "Attach/Detach This View", NULL, "Select whether this view is attached or in an independent window", (GCallback) win_plot2d_view::popup_AttachDetach },
	{ "DeleteViewPlot2D", NULL, "Delete View", NULL, "Delete this view", (GCallback) win_plot2d_view::popup_DeleteView }
};
*/
const char * win_plot2d_view::ui_description =
"<ui>"
"  <popup name='gp2dvMenu'>"
"    <menuitem action='AttachDetachPlot2D'/>"
"    <separator/>"
"    <menuitem action='DeleteViewPlot2D'/>"
"  </popup>"
"</ui>";

win_plot2d_view::win_plot2d_view(win_project * p1, i32s p2, const char * s1, const char * s2, const char * sv)
	: win_ogl_view(p1), plot2d_view(p1, p2, s1, s2, sv), ogl_view()
{
	this->view_widget = MakeGhemicalPlot(prj->GetWidget(), (void*) dynamic_cast<win_ogl_view*>(this), "plot 2D");

	//cout << "view_widget = " << view_widget << endl;
	
	SendMessage(view_widget, WM_COMMAND, ID_FILE_INITOG, 0);
	SendMessage(view_widget, WM_COMMAND, ID_FILE_INITOG2, 0);

	ConfigureHandler(view_widget);

	/*	win_label_set_text(win_LABEL(label_widget), "2D plot view");	// fixme!!!
	
	GtkActionGroup * action_group = win_action_group_new("gp2dvActions");
	win_action_group_add_actions(action_group, entries, G_N_ELEMENTS(entries), win_WIDGET(view_widget));
	
	win_ui_manager_insert_action_group(win_app::GetUIManager(), action_group, 0);
	
	GError * error = NULL;
	if (!win_ui_manager_add_ui_from_string(win_app::GetUIManager(), ui_description, -1, & error))
	{
		g_message("Building gp2dv menu failed : %s", error->message);
		g_error_free(error); exit(EXIT_FAILURE);
	}
	
	// set win_view::popupmenu so that win_ogl_view::ButtonHandler() will display it...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	popupmenu = win_ui_manager_get_widget(win_app::GetUIManager(), "/gp2dvMenu");*/
}

win_plot2d_view::~win_plot2d_view(void)
{
}

void win_plot2d_view::ExposeEvent(void)
{
//	GdkGLContext * glcontext = win_widget_get_gl_context(view_widget);
//	GdkGLDrawable * gldrawable = win_widget_get_gl_drawable(view_widget);

	SetCurrent(); Render(); glFlush();

//	if (gdk_gl_drawable_is_double_buffered(gldrawable)) gdk_gl_drawable_swap_buffers(gldrawable);
//	else glFlush();
	
//win_gl_area_swapbuffers(win_GL_AREA(view_widget));	// older way??? see win_graphics_view::ExposeEvent()
}

void win_plot2d_view::popup_AttachDetach(HWND widget, void *)
{
	win_ogl_view * oglv = GetOGLV(widget);
	plot2d_view * old_p2dv = dynamic_cast<plot2d_view *>(oglv);
	if (old_p2dv == NULL)
	{
		cout << "cast failed at win_plot2d_view::popup_AttachDetach()." << endl;
		return;
	}
	
	// first we create a new view using the requested mode...
	
	i32s ud2 = old_p2dv->getUD2();
	const char * s1 = old_p2dv->getN1();
	const char * s2 = old_p2dv->getN2();
	const char * sv = old_p2dv->getNV();
	bool flag = (oglv->detached == NULL);
	
	plot2d_view * new_p2dv = oglv->prj->AddPlot2DView(ud2, s1, s2, sv, flag);
	
	// try to retain as much data/settings as possible.
	
	new_p2dv->ImportData(old_p2dv);
	
	// ...and then simply destroy the old one; calling popup_ViewsDeleteView()
	// below this is the same as selecting "Delete This View" from the popup-menu.
	
	popup_DeleteView(widget, NULL);
//	new_p1dv->GetProject()->UpdateAllWindowTitles();
}

void win_plot2d_view::popup_DeleteView(HWND widget, void *)
{
	win_ogl_view * oglv = GetOGLV(widget);
	
	if (oglv->detached != NULL)
	{
//		win_widget_destroy(win_WIDGET(oglv->detached));
	}
	else
	{
//		gint page = win_notebook_page_num(win_NOTEBOOK(oglv->prj->notebook_widget), oglv->view_widget);
//		win_notebook_remove_page(win_NOTEBOOK(oglv->prj->notebook_widget), page);
	}
	
	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv == NULL) { cout << "cast failed at win_plot2d_view::popup_DeleteView()!" << endl; exit(EXIT_FAILURE); }
	
	oglv->prj->RemovePlottingView(pv);
	// TODO : delete the object and release memory ; is it done already???
}

/*################################################################################################*/
/*
GtkActionEntry win_rcp_view::entries[] =
{
	{ "DeleteViewRCP", NULL, "Delete View", NULL, "Delete this view", (GCallback) win_rcp_view::popup_DeleteView }
};
*/
const char * win_rcp_view::ui_description =
"<ui>"
"  <popup name='grcpvMenu'>"
"    <menuitem action='DeleteViewRCP'/>"
"  </popup>"
"</ui>";

win_rcp_view::win_rcp_view(win_project * p1, i32s p2, const char * s1, const char * sv)
	: win_ogl_view(p1), rcp_view(p1, p2, s1, sv), ogl_view()
{
/*	win_label_set_text(win_LABEL(label_widget), "RC plot view");	// fixme!!!
	
	GtkActionGroup * action_group = win_action_group_new("grcpvActions");
	win_action_group_add_actions(action_group, entries, G_N_ELEMENTS(entries), win_WIDGET(view_widget));
	
	win_ui_manager_insert_action_group(win_app::GetUIManager(), action_group, 0);
	
	GError * error = NULL;
	if (!win_ui_manager_add_ui_from_string(win_app::GetUIManager(), ui_description, -1, & error))
	{
		g_message("Building grcpv menu failed : %s", error->message);
		g_error_free(error); exit(EXIT_FAILURE);
	}
	
	// set win_view::popupmenu so that win_ogl_view::ButtonHandler() will display it...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	popupmenu = win_ui_manager_get_widget(win_app::GetUIManager(), "/grcpvMenu");*/
}

win_rcp_view::~win_rcp_view(void)
{
}

void win_rcp_view::ExposeEvent(void)
{
//	GdkGLContext * glcontext = win_widget_get_gl_context(view_widget);
//	GdkGLDrawable * gldrawable = win_widget_get_gl_drawable(view_widget);
	
	SetCurrent(); Render(); glFlush();

//	if (gdk_gl_drawable_is_double_buffered(gldrawable)) gdk_gl_drawable_swap_buffers(gldrawable);
//	else glFlush();
	
//win_gl_area_swapbuffers(win_GL_AREA(view_widget));	// older way??? see win_graphics_view::ExposeEvent()
}

void win_rcp_view::popup_DeleteView(HWND widget, void *)
{
	win_ogl_view * oglv = GetOGLV(widget);
	
	if (oglv->detached != NULL)
	{
//		win_widget_destroy(win_WIDGET(oglv->detached));
	}
	else
	{
//		gint page = win_notebook_page_num(win_NOTEBOOK(oglv->prj->notebook_widget), oglv->view_widget);
//		win_notebook_remove_page(win_NOTEBOOK(oglv->prj->notebook_widget), page);
	}
	
	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv == NULL) { cout << "cast failed at win_rcp_view::popup_DeleteView()!" << endl; exit(EXIT_FAILURE); }
	
	oglv->prj->RemovePlottingView(pv);
	// TODO : delete the object and release memory ; is it done already???
}

/*################################################################################################*/
/*
GtkActionEntry win_eld_view::entries[] =
{
	{ "DeleteViewELD", NULL, "Delete View", NULL, "Delete this view", (GCallback) win_eld_view::popup_DeleteView }
};
*/
const char * win_eld_view::ui_description =
"<ui>"
"  <popup name='geldvMenu'>"
"    <menuitem action='DeleteViewELD'/>"
"  </popup>"
"</ui>";

win_eld_view::win_eld_view(win_project * p1)
	: win_ogl_view(p1), eld_view(p1), ogl_view()
{
/*	win_label_set_text(win_LABEL(label_widget), "energy-level diagram view");	// fixme!!!
	
	GtkActionGroup * action_group = win_action_group_new("geldvActions");
	win_action_group_add_actions(action_group, entries, G_N_ELEMENTS(entries), win_WIDGET(view_widget));
	
	win_ui_manager_insert_action_group(win_app::GetUIManager(), action_group, 0);
	
	GError * error = NULL;
	if (!win_ui_manager_add_ui_from_string(win_app::GetUIManager(), ui_description, -1, & error))
	{
		g_message("Building geldv menu failed : %s", error->message);
		g_error_free(error); exit(EXIT_FAILURE);
	}
	
	// set win_view::popupmenu so that win_ogl_view::ButtonHandler() will display it...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	popupmenu = win_ui_manager_get_widget(win_app::GetUIManager(), "/geldvMenu");*/
}

win_eld_view::~win_eld_view(void)
{
}

void win_eld_view::ExposeEvent(void)
{
//	GdkGLContext * glcontext = win_widget_get_gl_context(view_widget);
//	GdkGLDrawable * gldrawable = win_widget_get_gl_drawable(view_widget);

	SetCurrent(); Render(); glFlush();

//	if (gdk_gl_drawable_is_double_buffered(gldrawable)) gdk_gl_drawable_swap_buffers(gldrawable);
//	else glFlush();
	
//win_gl_area_swapbuffers(win_GL_AREA(view_widget));	// older way??? see win_graphics_view::ExposeEvent()
}

void win_eld_view::popup_DeleteView(HWND widget, void *)
{
	win_ogl_view * oglv = GetOGLV(widget);
	
	if (oglv->detached != NULL)
	{
//		win_widget_destroy(win_WIDGET(oglv->detached));
	}
	else
	{
//		gint page = win_notebook_page_num(win_NOTEBOOK(oglv->prj->notebook_widget), oglv->view_widget);
//		win_notebook_remove_page(win_NOTEBOOK(oglv->prj->notebook_widget), page);
	}
	
	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv == NULL) { cout << "cast failed at win_eld_view::popup_DeleteView()!" << endl; exit(EXIT_FAILURE); }
	
	oglv->prj->RemovePlottingView(pv);
	// TODO : delete the object and release memory ; is it done already???
}

/*################################################################################################*/

// eof
