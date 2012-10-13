// EXAMPLE2.CPP

// Copyright (C) 2004 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "example2.h"

#include "..\notice.h"

#include "..\eng1_mm_tripos52.h"
#include "..\eng1_mm_default.h"

#include "..\geomopt.h"
#include "..\moldyn.h"

#include <windows.h>

#include "../../../consol/filedlg.h"

#define _USE_WINGHEMICAL_ 0
#if _USE_WINGHEMICAL_
#include "../../../src/win32_app.h"
#include "../../src/utility.h"
#include "../../../consol/winghemical.h"
#endif

extern HINSTANCE hInst;


/*################################################################################################*/

int main(int argc, char ** argv)
{
	print_lib_intro_notice(cout);
	print_copyright_notice(cout);
	cout << endl;
	
	// example #2: open a file, run a geometry optimization, and save a file.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	char trgtlst_name[1048];
	char infile_name[1048];
	char outfile_name[1048];
	DWORD nFilterIndex;
	if (OpenFileDlg(0, "Ghemical Project File (*.gpr)\0*.gpr\0All files \0*.*\0", 
		infile_name, nFilterIndex) 
		!= S_OK)
	{
		return -1;
	}	
	
	if (OpenFileDlg(0, "Target List File (*.dat)\0*.dat\0All files \0*.*\0", 
		trgtlst_name, nFilterIndex) 
		!= S_OK)
	{
		return -1;
	}

	// check that we have got both file names from the command line.
	
	/*if (argc != 3)
	{
		cout << "usage : example2 infile.gpr outfile.gpr" << endl;
		exit(1);
	}
	
	char * infile_name = argv[1];
	char * outfile_name = argv[2];
	*/
	// create a model object, and read in a file.
	
	
	cout << "trying to open a file " << infile_name << " ; ";
	
	ifstream ifile;
	ifile.open(infile_name, ios::in);
	
	if (ifile.fail())
	{
		cout << "failed!!!" << endl;
		
		ifile.close();
		return -1;
	}
	
	cout << "ok!!!" << endl;
	

#if _USE_WINGHEMICAL_

	//готовим классы для 3D визуализации
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
	//app->work(argc, argv); //
	ReadGPR(* app->GetWinProject(), ifile, false, false);
	ifile.close();

	{
		HANDLE hThread = NULL;
		{
		setup * su = app->GetWinProject()->GetCurrentSetup();
		static jobinfo_GeomOpt ji;
		
		ji.prj = app->GetWinProject();
		ji.go = geomopt_param(su);
		ji.go.show_dialog = false;
		
		hThread = app->GetWinProject()->start_job_GeomOpt(& ji);		
		}
		// !!!!!!!! надо дождаться пока заончится этот процесс а потом запускать следющий
		WaitForSingleObject(hThread, INFINITE);
		{
		setup * su = app->GetWinProject()->GetCurrentSetup();
		static jobinfo_MolDyn ji;
		
		ji.prj = app->GetWinProject();
		ji.md = moldyn_param(su);
		ji.md.show_dialog = false;
		ReadTargetListFile(trgtlst_name, ji.md.target_list);
		ji.md.start_snarjad_vel[0] = 0.0;
		ji.md.start_snarjad_vel[1] = 0.0;
		ji.md.start_snarjad_vel[2] = -20.0;

		
		app->GetWinProject()->start_job_MolDyn(& ji);
		}
	}

	StartWinGhemical(NULL);

#else

	model * mdl = new model();
	ReadGPR(* mdl, ifile, false, false);
	ifile.close();

	//mdl->DoMolDyn()
	{
		const bool updt = true;
		setup * su = mdl->GetCurrentSetup();

printf("periodic_box_HALFdim[0] = %f\n", mdl->periodic_box_HALFdim[0]);
printf("periodic_box_HALFdim[1] = %f\n", mdl->periodic_box_HALFdim[1]);
printf("periodic_box_HALFdim[2] = %f\n", mdl->periodic_box_HALFdim[2]);

		geomopt_param go = geomopt_param(su);
		mdl->DoGeomOpt(go, updt);

printf("periodic_box_HALFdim[0] = %f\n", mdl->periodic_box_HALFdim[0]);
printf("periodic_box_HALFdim[1] = %f\n", mdl->periodic_box_HALFdim[1]);
printf("periodic_box_HALFdim[2] = %f\n", mdl->periodic_box_HALFdim[2]);

		moldyn_param md = moldyn_param(su);
		ReadTargetListFile(trgtlst_name, md.target_list);
		for(int i = 0; i < md.target_list.size(); i++)
		{
			cout << "md.target_list[" << i << "] = " << md.target_list[i] << endl;
		}
		md.start_snarjad_vel[0] = 0.0;
		md.start_snarjad_vel[1] = 0.0;
		md.start_snarjad_vel[2] = -20.0;

		//strcpy(md.filename, infile_name);
		mdl->DoMolDyn(md,updt);
	}
#endif

	//------------------------------------------------------------------
	char filter[] =     TEXT("Ghemical Project File (*.gpr)\0*.gpr\0")
						 TEXT("All Files (*.*)\0*.*\0");

	sprintf(outfile_name, "\0");
	if (SaveFileDlg(0, outfile_name, filter, nFilterIndex) == S_OK)
	{
		cout << "now saving file " << outfile_name << endl;
		ofstream ofile;
		ofile.open(outfile_name, ios::out);
		
#if _USE_WINGHEMICAL_
		WriteGPR_v100(* app->GetWinProject(), ofile);
#else
		WriteGPR_v100(* mdl, ofile);
#endif
		ofile.close();
	}
	
	// finally release all allocated memory and leave.
	
#if !_USE_WINGHEMICAL_
	delete mdl; mdl = NULL;
#endif
	
	return 0;
}

/*################################################################################################*/

// eof
