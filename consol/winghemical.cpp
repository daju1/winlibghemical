// ghemical.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "resource.h"
#include "winghemical.h"
#include "../src/win32_app.h"
#include "../src/win32_graphics_view.h"
// Windows Header Files:
#include "../src/win32_dialog.h"


#include <math.h>
#define PI		3.14159265358979323846
#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glut.h>

#define MAX_LOADSTRING 256
#define IDT_TIMER_1    1

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];								// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];								// The title bar text


#include "../src/project.h"	// config.h is here -> we get ENABLE-macros here...

int BeginWinGhemical( )
{
	// Initialize global strings
	LoadString(hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInst, IDC_GHEMICAL, szWindowClass, MAX_LOADSTRING);

	return MyRegisterClass(hInst);

}
HWND CreateWinGhemical( void * p)
{
	HWND hWnd = NULL;

	hWnd = CreateWindow(
	   szWindowClass, 
	   szTitle, 
	   WS_OVERLAPPEDWINDOW,
	   CW_USEDEFAULT, 0, 
	   CW_USEDEFAULT, 0, 
	   NULL, NULL, 
	   hInst, 
	   (void *)p);

	if (!hWnd)
	{
		return NULL;
	}

	ShowWindow(hWnd, 1);
//	UpdateWindow(hWnd);

	return hWnd;
}
DWORD WINAPI StartWinGhemical(void * p)
{
	MSG msg;
	HACCEL hAccelTable;

	hAccelTable = LoadAccelerators(hInst, (LPCTSTR)IDC_GHEMICAL);

	while( GetMessage( &msg, NULL, 0, 0) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	} 
	return( msg.wParam );
}
int WinGhemical()
{
	BeginWinGhemical();
	void * p = NULL;
	HWND hWnd = CreateWinGhemical(p);
	return StartWinGhemical(NULL);
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_GHEMICAL);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_GHEMICAL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	static bool m_bPerspective;

	//static	HGLRC		m_hRC;				// Контекст OpenGL
	//static	HDC			m_hdc; 			// Контекст Windows


	switch (message) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
			case ID_MOUSE_DRAW:
			case ID_MOUSE_ERASE:
			case ID_MOUSE_SELECT:
			case ID_MOUSE_ZOOM:
			case ID_MOUSE_CLIPPING:
			case ID_MOUSE_TRANSLATEXY:
			case ID_MOUSE_TRANSLATEZ:
			case ID_MOUSE_ORBITXY:
			case ID_MOUSE_ORBITZ:
			case ID_MOUSE_ROTATEXY:
			case ID_MOUSE_ROTATEZ:
			case ID_MOUSE_MEASURE:
				{
					CheckMenuItem(GetMenu( hWnd ), ID_MOUSE_DRAW,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_MOUSE_ERASE,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_MOUSE_SELECT,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_MOUSE_ZOOM,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_MOUSE_CLIPPING,	MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_MOUSE_TRANSLATEXY,MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_MOUSE_TRANSLATEZ, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_MOUSE_ORBITXY,	MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_MOUSE_ORBITZ,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_MOUSE_ROTATEXY,	MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_MOUSE_ROTATEZ,	MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_MOUSE_MEASURE,	MF_BYCOMMAND | MF_UNCHECKED);
					
					CheckMenuItem(GetMenu( hWnd ), wmId, MF_BYCOMMAND | MF_CHECKED);
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						///wgrv->RealizeHandler(hWnd, NULL);
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							win_app * app = w_prj->GetApp();
							if (app)
							{
								switch (wmId)
								{
								case ID_MOUSE_DRAW:
									app->maintb_tool_Draw(NULL, NULL, NULL);
									break;
								case ID_MOUSE_ERASE:
									app->maintb_tool_Erase(NULL, NULL, NULL);
									break;
								case ID_MOUSE_SELECT:
									app->maintb_tool_Select(NULL, NULL, NULL);
									break;
								case ID_MOUSE_ZOOM:
									app->maintb_tool_Zoom(NULL, NULL, NULL);
									break;
								case ID_MOUSE_CLIPPING:
									app->maintb_tool_Clipping(NULL, NULL, NULL);
									break;
								case ID_MOUSE_TRANSLATEXY:
									app->maintb_tool_TranslXY(NULL, NULL, NULL);
									break;
								case ID_MOUSE_TRANSLATEZ:
									app->maintb_tool_TranslZ(NULL, NULL, NULL);
									break;
								case ID_MOUSE_ORBITXY:
									app->maintb_tool_OrbitXY(NULL, NULL, NULL);
									break;
								case ID_MOUSE_ORBITZ:
									app->maintb_tool_OrbitZ(NULL, NULL, NULL);
									break;
								case ID_MOUSE_ROTATEXY:
									app->maintb_tool_RotateXY(NULL, NULL, NULL);
									break;
								case ID_MOUSE_ROTATEZ:
									app->maintb_tool_RotateZ(NULL, NULL, NULL);
									break;
								case ID_MOUSE_MEASURE:
									app->maintb_tool_Measure(NULL, NULL, NULL);
									break;
								}

							}
						}
					}
					//######################################################
				}
				break;

			case ID_RENDER_BALLANDSTICK:
			case ID_RENDER_VANDERVAALS:
			case ID_RENDER_CYLINDERS:
			case ID_RENDER_WIREFRAME:
			case ID_RENDER_NOTHING:
				{
					CheckMenuItem(GetMenu( hWnd ), ID_RENDER_BALLANDSTICK,	MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_RENDER_VANDERVAALS,	MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_RENDER_CYLINDERS,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_RENDER_WIREFRAME,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_RENDER_NOTHING,		MF_BYCOMMAND | MF_UNCHECKED);
					
					CheckMenuItem(GetMenu( hWnd ), wmId, MF_BYCOMMAND | MF_CHECKED);
					//######################################################
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							switch (wmId)
							{
							case ID_RENDER_BALLANDSTICK:
								w_prj->popup_RModeBallStick(hWnd, NULL);
								break;
							case ID_RENDER_VANDERVAALS:
								w_prj->popup_RModeVanDerWaals(hWnd, NULL);
								break;
							case ID_RENDER_CYLINDERS:
								w_prj->popup_RModeCylinders(hWnd, NULL);
								break;
							case ID_RENDER_WIREFRAME:
								w_prj->popup_RModeWireframe(hWnd, NULL);
								break;
							case ID_RENDER_NOTHING:
								w_prj->popup_RModeNothing(hWnd, NULL);
								break;
							}
						}
					}
				}
				break;

			case ID_COLOR_ELEMENT:
			case ID_COLOR_SEC:
			case ID_COLOR_HYDROPHOBICITY:
				{
					CheckMenuItem(GetMenu( hWnd ), ID_COLOR_ELEMENT,			MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_COLOR_SEC,				MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_COLOR_HYDROPHOBICITY,		MF_BYCOMMAND | MF_UNCHECKED);
					
					CheckMenuItem(GetMenu( hWnd ), wmId, MF_BYCOMMAND | MF_CHECKED);
					//######################################################
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							switch (wmId)
							{
							case ID_COLOR_ELEMENT:
								w_prj->popup_CModeElement(hWnd, NULL);
								break;
							case ID_COLOR_SEC:
								w_prj->popup_CModeSecStruct(hWnd, NULL);
								break;
							case ID_COLOR_HYDROPHOBICITY:
								w_prj->popup_CModeHydPhob(hWnd, NULL);
								break;
							}
						}
					}
				}
				break;

			case ID_LABEL_INDEX:
			case ID_LABEL_ELEMENT:
			case ID_LABEL_FORMALCHARGE:
			case ID_LABEL_PARTIALCHARGE:
			case ID_LABEL_ATOMTYPE:
			case ID_LABEL_BUILDERID:
			case ID_LABEL_BONDTYPE:
			case ID_LABEL_RESIDUE:
			case ID_LABEL_SEC:
			case ID_LABEL_NOTHING:
				{
					CheckMenuItem(GetMenu( hWnd ), ID_LABEL_INDEX,	MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_LABEL_ELEMENT,	MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_LABEL_FORMALCHARGE,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_LABEL_PARTIALCHARGE,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_LABEL_ATOMTYPE,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_LABEL_BUILDERID,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_LABEL_BONDTYPE,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_LABEL_RESIDUE,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_LABEL_SEC,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_LABEL_NOTHING,		MF_BYCOMMAND | MF_UNCHECKED);
					
					CheckMenuItem(GetMenu( hWnd ), wmId, MF_BYCOMMAND | MF_CHECKED);
					//######################################################
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							switch (wmId)
							{
							case ID_LABEL_INDEX:
								w_prj->popup_LModeIndex(hWnd, NULL);
								break;
							case ID_LABEL_ELEMENT:
								w_prj->popup_LModeElement(hWnd, NULL);
								break;
							case ID_LABEL_FORMALCHARGE:
								w_prj->popup_LModeFCharge(hWnd, NULL);
								break;
							case ID_LABEL_PARTIALCHARGE:
								w_prj->popup_LModePCharge(hWnd, NULL);
								break;
							case ID_LABEL_ATOMTYPE:
								w_prj->popup_LModeAtomType(hWnd, NULL);
								break;
							case ID_LABEL_BUILDERID:
								w_prj->popup_LModeBuilderID(hWnd, NULL);
								break;
							case ID_LABEL_BONDTYPE:
								w_prj->popup_LModeBondType(hWnd, NULL);
								break;
							case ID_LABEL_RESIDUE:
								w_prj->popup_LModeResidue(hWnd, NULL);
								break;
							case ID_LABEL_SEC:
								w_prj->popup_LModeSecStruct(hWnd, NULL);
								break;
							case ID_LABEL_NOTHING:
								w_prj->popup_LModeNothing(hWnd, NULL);
								break;
							}
						}
					}
				}
				break;

			case ID_SELECT_ATOM:
			case ID_SELECT_RESIDUE:
			case ID_SELECT_CHAIN:
			case ID_SELECT_MOLECULE:
				{
					CheckMenuItem(GetMenu( hWnd ), ID_SELECT_ATOM,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_SELECT_RESIDUE,	MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_SELECT_CHAIN,		MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu( hWnd ), ID_SELECT_MOLECULE,	MF_BYCOMMAND | MF_UNCHECKED);
					
					CheckMenuItem(GetMenu( hWnd ), wmId, MF_BYCOMMAND | MF_CHECKED);
					//######################################################
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							switch (wmId)
							{
							case ID_SELECT_ATOM:
								w_prj->popup_SelectModeAtom(hWnd, NULL);
								break;
							case ID_SELECT_RESIDUE:
								w_prj->popup_SelectModeResidue(hWnd, NULL);
								break;
							case ID_SELECT_CHAIN:
								w_prj->popup_SelectModeChain(hWnd, NULL);
								break;
							case ID_SELECT_MOLECULE:
								w_prj->popup_SelectModeMolecule(hWnd, NULL);
								break;
							}
						}
					}
				}
				break;

			case ID_SELECT_SELECTALL:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							w_prj->popup_SelectAll(hWnd,NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_SELECT_SELECTNONE:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							w_prj->popup_SelectNone(hWnd,NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_SELECT_INVERTSELECTION:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							w_prj->popup_InvertSelection(hWnd,NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_SELECT_DELETESELECTION:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							w_prj->popup_DeleteSelection(hWnd,NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_BUILD_CLEAR:
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
							w_prj->popup_Clear(hWnd,NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_BUILD_DELETEHYDROGENS:
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
							w_prj->popup_HRemove(hWnd,NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_BUILD_ADDHYDROGENS:
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
							w_prj->popup_HAdd(hWnd,NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_BUILD_SOLVATEBOX:
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
							w_prj->popup_SolvateBox(hWnd,NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_BUILD_SOLVATESPHERE:
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
							w_prj->popup_SolvateSphere(hWnd,NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_BUILD_AMINO:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							w_prj->popup_BuilderAmino(hWnd,NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_BUILD_NUCLEIC:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							w_prj->popup_BuilderNucleic(hWnd,NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_ENTERCOMMAND:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							w_prj->popup_EnterCommand(hWnd,NULL);
						}
					}
					//######################################################
				}
				break;				

			case ID_DO_SETUP:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							w_prj->popup_CompSetup(hWnd,NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_VIEW_PERSPECTIVE:
				{
					//######################################################
					win_ogl_view * wogv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);

					win_graphics_view * wgrv = dynamic_cast<win_graphics_view *>(wogv);
					if (wgrv)
					{
						if (m_bPerspective)
						{
							m_bPerspective = false;

							wgrv->popup_ProjOrthographic(hWnd, NULL);

							CheckMenuItem(GetMenu( GetParent( hWnd ) ), 
								ID_VIEW_PERSPECTIVE,
								MF_BYCOMMAND | MF_UNCHECKED);
						}
						else
						{
							m_bPerspective = true;

							wgrv->popup_ProjPerspective(hWnd, NULL);

							CheckMenuItem(GetMenu( GetParent( hWnd ) ), 
								ID_VIEW_PERSPECTIVE,
								MF_BYCOMMAND | MF_CHECKED);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_ELEMENTSTABLE:
				{
					new element_dialog;
				}
				break;

			case ID_DO_BONDTYPE:
				{
					new bondtype_dialog;
				}
				break;









			case ID_FILE_OPEN:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							w_prj->popup_FileOpen(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_FILE_SAVEAS:
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
							w_prj->popup_FileSaveAs(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_FILE_SAVEBOX:
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
							w_prj->popup_FileSaveBox(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_FILE_LOADBOX:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							w_prj->popup_FileLoadBox(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_FILE_SAVESELECTED:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							w_prj->popup_FileSaveSelected(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_FILE_LOADSELECT:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						win_project * w_prj = dynamic_cast<win_project *>(prj);
						if (w_prj)
						{
							w_prj->popup_FileLoadSelect(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_FILE_IMPORT:
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
							w_prj->popup_FileImport(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_FILE_EXPORT:
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
							w_prj->popup_FileExport(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_DO_GEOMETRYOPTIMIZATION:
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
							w_prj->popup_CompGeomOpt(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_DO_MOLECULARDYNAMICS:
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
							w_prj->popup_CompMolDyn(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_RANDOMSEARCH:
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
							w_prj->popup_CompRandomSearch(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_SYSTEMATICSEARCH:
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
							w_prj->popup_CompSystematicSearch(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_MONTECARLOSEARCH:
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
							w_prj->popup_CompMonteCarloSearch(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_STATIONARYSTATESEARCH:
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
							w_prj->popup_CompStationaryStateSearch(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_DO_COMPFORMULA:
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
							w_prj->popup_CompFormula(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

				

			case ID_DO_TRANSITIONSTATESEARCH:
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
							w_prj->popup_CompTransitionStateSearch(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;


				

			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->popup_CompPopAnaElectrostatic(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_SETORBITAL:
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
							w_prj->popup_SetOrbital(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_SETFORMALCHARGE:
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
							w_prj->popup_CompSetFormalCharge(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_VIEW_ADDENERGYLEVELDIAGRAMVIEW:
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
							w_prj->popup_ViewsNewELD(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
				/*
			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->popup_ObjRibbon(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->popup_ObjEPlane(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->popup_ObjEVolume(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->popup_ObjESurface(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->popup_ObjEVDWSurface(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
				

			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->popup_ObjEDPlane(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->popup_ObjEDSurface(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->ObjMOPlane(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->ObjMOVolume(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->popup_ObjMOSurface(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->popup_ObjMODPlane(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->popup_ObjMODVolume(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->popup_ObjMODSurface(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->popup_(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_POPANAELECTROSTATIC:
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
							w_prj->popup_(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;


				///*/


			case ID_DO_COMPUTEENERGY:
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
							w_prj->popup_CompEnergy(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_SORTGROUPS:
				{
					//######################################################
					win_ogl_view * wgrv = 
						(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
					if (wgrv)
					{
						project * prj = wgrv->GetProject();
						if (prj)
						{
							//if (!prj->IsGroupsClean()) 
								prj->UpdateGroups();		// for internal coordinates...
							//	if (!tmpmdl->IsGroupsSorted()) 
								prj->SortGroups(true);	// for internal coordinates...
						}
					}
					//######################################################
				}
				break;

			case ID_TRAJECTORY_VIEW_POTENCIAL_ENERGY_PLOT:
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
							w_prj->popup_TrajView_EnergyPlot(hWnd, NULL, true);
						}
					}
					//######################################################
				}
				break;		

			case ID_SELECTEDATOMS_LOCK:
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
							w_prj->popup_LockSelectedAtoms(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;		

			case ID_SELECTEDATOMS_UNLOCK:
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
							w_prj->popup_UnLockSelectedAtoms(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;		

			case ID_SELECTEDATOMS_SETASSOLVENT:
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
							w_prj->popup_SetSelectedAtomsAsSolvent(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;		

			case ID_SELECTEDATOMS_UNSETASSOLVENT:
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
							w_prj->popup_UnSetSelectedAtomsAsSolvent(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;		

			case ID_SELECTEDATOMS_SETUNDERGRAVI:
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
							w_prj->popup_SetSelectedAtomsUnderGravi(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;	

			case ID_SELECTEDATOMS_UNSETUNDERGRAVI:
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
							w_prj->popup_UnSetSelectedAtomsUnderGravi(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;	

			case ID_DO_TRAJECTORYVIEWKINETICENERGYPLOT:
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
							w_prj->popup_TrajView_EnergyPlot(hWnd, NULL, false);
						}
					}
					//######################################################
				}
				break;		


			case ID_TRAJECTORY_SETTOTALFRAMES:
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
							w_prj->popup_TrajSetTotalFrames(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
				
				
			case ID_DO_TRAJECTORYVIEW:
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
							w_prj->popup_TrajView(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;				
				
			case ID_DO_COMPUTEANGLEENERGYPLOT1D:
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
							w_prj->popup_CompAngleEnergyPlot1D(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;
				
			case ID_TRAJECTORY_VIEWANGLEPLOT:
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
							w_prj->popup_CompAngleTrajPlot1D(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_TRAJECTORY_VIEWDISTANCEPLOT:
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
							w_prj->popup_CompDistanceTrajPlot1D(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_TRAJECTORY_VIEWCOORDINATEPLOT:
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
							w_prj->popup_CompCoordinateTrajPlot1D(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_COMPTORSIONENERGYPLOT1D:
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
							w_prj->popup_CompTorsionEnergyPlot1D(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

			case ID_DO_COMPUTETORSIONENERGYPLOT2D:
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
							w_prj->popup_CompTorsionEnergyPlot2D(hWnd, NULL);
						}
					}
					//######################################################
				}
				break;

				
				
			case ID_VIEW_CENTER:
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
							w_prj->popup_Center(hWnd, NULL);
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
			case IDM_ABOUT:
				DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
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
				  //##################################################
				  CheckMenuItem(GetMenu( hWnd ), 
					  ID_MOUSE_ORBITXY,
					  MF_BYCOMMAND | MF_CHECKED);
				  //##################################################
				  win_ogl_view * wogv = 
					  (win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
				  if (wogv)
				  {
					  wogv->SetWindow(hWnd);

					  project * prj = wogv->GetProject();
					  win_project * w_prj = dynamic_cast<win_project *>(prj);
					  if (w_prj)
					  {
						  w_prj->paned_widget = hWnd;
						  printf("w_prj->paned_widget = 0x%08x\n", w_prj->paned_widget);
					  }
				  }
				  //##################################################
 				  SendMessage(hWnd, WM_COMMAND, ID_FILE_INITOG, 0);
				  
				  //##################################################
				  m_bPerspective = true;
				  
				  if (m_bPerspective)
				  {
					  CheckMenuItem(GetMenu( GetParent( hWnd ) ),
						  ID_VIEW_PERSPECTIVE,
						  MF_BYCOMMAND | MF_CHECKED);
				  }
				  else
				  {
					  CheckMenuItem(GetMenu( GetParent( hWnd ) ),
						  ID_VIEW_PERSPECTIVE,
						  MF_BYCOMMAND | MF_UNCHECKED);
				  }
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
					/*if (win_ogl_view::m_CurrentOGContext != &woglv->m_hRC)
					{
						//====== Пытаемся выбрать его в качестве текущего
						if ( wglMakeCurrent (m_hdc, woglv->m_hRC))
						{
							win_ogl_view::m_CurrentOGContext = &woglv->m_hRC;
						}
						else
						{
							MessageBox(hWnd,"wglMakeCurrent::Error","On Create",0);
							return 0;
						}
					}*/



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
				win_ogl_view * woglv = 
					(win_ogl_view *)GetWindowLong(hWnd,GWL_USERDATA);
				if (woglv)
				{	
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
					
					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
					/*if (win_ogl_view::m_CurrentOGContext != &woglv->m_hRC)
					{
						//====== Пытаемся выбрать его в качестве текущего
						if ( wglMakeCurrent (m_hdc, woglv->m_hRC))
						{
							win_ogl_view::m_CurrentOGContext = &woglv->m_hRC;
						}
						else
						{
							MessageBox(hWnd,"wglMakeCurrent::Error","On Create",0);
							return 0;
						}
					}*/
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
		case WM_MOUSEWHEEL :
			{
				int zDelta = (int) wParam; // wheel rotation 
				//vView[iWnd].VerticalZoom(hWnd, zDelta, false);
				//unsigned short xPos = LOWORD(lParam); // horizontal position of pointer 
				//unsigned short yPos = HIWORD(lParam); // vertical position of pointer 		}
/*				m_dz = float(zDelta) / 3000000.f;
				//====== Если одновременно была нажата Ctrl,
				if (wParam & MK_CONTROL)
				{
					//=== Изменяем коэффициенты сдвига изображения
					m_AngleX += m_dz;
				}
				else
				{
					if (wParam & MK_SHIFT )
					{
						m_AngleY += m_dz;
					}
					else
					{
						m_AngleZ += m_dz;
					}
				}

				RECT rect;
				GetClientRect(hWnd,&rect);
				InvalidateRect(hWnd,&rect, true);*/
			}
			break;
/*		case WM_KEYDOWN :
			{
				//char str[255];
				//sprintf(str,"wParam = %d lParam = %d", wParam, lParam);
				//MessageBox(0,str,"WM_KEYDOWN",0);
				switch (wParam)
				{
				case 16:// Shift
					{
						m_bShiftPressed = true;
					}
					break;
				case 17:// Ctrl
					{
						m_bCtrlPressed = true;
					}
					break;
				case 38:// forword
					{
						m_zTrans += 0.05f;
					}
					break;
				case 40:// back
					{
						m_zTrans -= 0.05f;
					}
					break;
				case 37:// left
					{
						if(m_bShiftPressed)
							m_xTrans += 0.02f;
						else
						{
							if (m_bCtrlPressed)//поворот влево
								m_AngleZ -= 0.5f;
							else
							{
								float r = (float)sqrt(m_xTrans*m_xTrans + m_zTrans*m_zTrans);
								float alpha = (float)atan2(m_xTrans, m_zTrans);
								alpha -= 0.01f;
								m_AngleZ -= (float)(0.01f*180.f/PI);
								m_zTrans = (float)(r*cos(alpha));
								m_xTrans = (float)(r*sin(alpha));
							}
						}
					}
					break;
				case 39:// right
					{
						if(m_bShiftPressed)
							m_xTrans -= 0.02f;
						else
						{
							if (m_bCtrlPressed)// поворот вправо
								m_AngleZ += 0.5f;
							else
							{
								float r = (float)sqrt(m_xTrans*m_xTrans + m_zTrans*m_zTrans);
								float alpha = (float)atan2(m_xTrans, m_zTrans);
								alpha += 0.01f;
								m_AngleZ +=(float)(0.01f*180.f/PI);
								m_zTrans = (float)(r*cos(alpha));
								m_xTrans = (float)(r*sin(alpha));
							}
						}
					}
					break;
				case 33:// page up
					{
						if(m_bShiftPressed)
							m_AngleX += 0.5f;
						else
						{
							if (m_bCtrlPressed)// поворот вправо
								m_AngleY += 0.5f;
							else
								m_yTrans -= 0.02f;
						}
					}
					break;
				case 34:// page down
					{
						if(m_bShiftPressed)
							m_AngleX -= 0.5f;
						else
						{
							if (m_bCtrlPressed)// поворот вправо
								m_AngleY -= 0.5f;
							else
								m_yTrans += 0.02f;
						}
					}
					break;
				}

				RECT rect;
				GetClientRect(hWnd,&rect);
				InvalidateRect(hWnd,&rect, true);
			}
			break;

		case WM_KEYUP :
			{
				//char str[255];
				//sprintf(str,"wParam = %d lParam = %d", wParam, lParam);
				//MessageBox(0,str,"WM_KEYUP",0);
				switch (wParam)
				{
				case 16:// Shift
					{
						m_bShiftPressed = false;
					}
					break;
				case 17:// Ctrl
					{
						m_bCtrlPressed = false;
					}
					break;
				}
			}
			break;
*/
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}
