// ghemical.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "..\libghemical\src\libconfig.h"
#include "..\libghemical\src\long_stream.h"

#ifndef EXAMPLE2_H
#define EXAMPLE2_H

#include "..\libghemical\src\model.h"

#include <iostream>
using namespace std;

/*################################################################################################*/

int main(int, char **);

/*################################################################################################*/

bool ReadGPR_OLD(model &, istream &, bool, bool);		// see fileio.cpp!!!
bool ReadGPR_v100(model &, istream &, bool, bool);		// see fileio.cpp!!!
bool ReadGPR(model &, istream &, bool, bool);			// see fileio.cpp!!!
void WriteGPR(model &, ostream &);				// see fileio.cpp!!!
void WriteGPR_v100(model &, ostream &);				// see fileio.cpp!!!

/*################################################################################################*/

#endif	// EXAMPLE2_H
#include "..\libghemical\src\notice.h"

#include "..\libghemical\src\eng1_mm_tripos52.h"
#include "..\libghemical\src\eng1_mm_default.h"

#include "..\libghemical\src\geomopt.h"
#include "..\libghemical\src\moldyn.h"

#include "../src/project.h"	// config.h is here -> we get ENABLE-macros here...
//#include "../src/win32_project.h"
#include "../src/win32_views.h"
#include "winghemical.h"
#include "../src/win32_app.h"
#include "../libghemical/src/utility.h"

void WriteGPR_v100(project & prj, ostream & file);

extern HINSTANCE hInst;								// current instance

int main(int argc, char* argv[])
{
	cout << endl;

	hInst = GetModuleHandle(NULL);


	singleton_cleaner<win_app> app_cleaner;
	win_app * app = win_app::GetInstance();  
	// new win_app() here 
	// -> 	SetNewProject(); 
	// -> new win_project(* win_class_factory::GetInstance());
	// -> new win_class_factory() : graphics_class_factory()
	// 

	// win_project()
	// -> CreateProjectView() -> new win_project_view(prj2) -> LightAdded(); ObjectAdded();

	// win_project()
	// -> AddGraphicsView(NULL, false);

	// project::AddGraphicsView()
	// -> win_class_factory::ProduceGraphicsView() 
	// -> new win_graphics_view(prj2, cam);
	// -> BeginWinGhemical(); view_widget = CreateWinGhemical(this); SendMessage(view_widget, WM_COMMAND, ID_FILE_INITOG2, 0); ConfigureHandler(view_widget);
	app_cleaner.SetInstance(app);

	app->GetWinProject()->SetApp(app);
	//app->work(argc, argv);
	StartWinGhemical(NULL);

	return 0;
}

