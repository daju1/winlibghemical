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

WPARAM StartWinLoop()
{
	MSG msg;
	while( GetMessage( &msg, NULL, 0, 0) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	} 
	return( msg.wParam );
}

/*################################################################################################*/

int main(int argc, char ** argv)
{
	print_lib_intro_notice(cout);
	print_copyright_notice(cout);
	cout << endl;
#if SNARJAD_TARGET_WORKING

	double start = 0.01;
	double step = 0.01;
	double fin = 2.75;

	switch (argc)
	{
	case 2:
		fin = atof(argv[1]);
		break;
	case 3:
		start = atof(argv[1]);
		fin = atof(argv[2]);
		break;
	case 4:
		start = atof(argv[1]);
		step = atof(argv[2]);
		fin = atof(argv[3]);
		break;
	}
	printf("start = %f step = %f fin = %f\n", start, step, fin);
#endif
	// example #2: open a file, run a geometry optimization, and save a file.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	char trgtlst_name[1048];
	char infile_name[1048];
	char outfile_name[1048];
	char box_name[1048];
	char fixed_name[1048];

	DWORD nFilterIndex;
	if (OpenFileDlg(0, "Ghemical Project File (*.gpr)\0*.gpr\0All files \0*.*\0", 
		infile_name, nFilterIndex) 
		!= S_OK)
	{
		return -1;
	}	

	if (OpenFileDlg(0, "Box File (*.box)\0*.box\0All files \0*.*\0", 
		box_name, nFilterIndex) 
		!= S_OK)
	{
		return -1;
	}

#if SNARJAD_TARGET_WORKING || PROBNIY_ATOM_WORKING || PROBNIY_ATOM_GEOMOPT
	if (OpenFileDlg(0, "Target List File (*.dat)\0*.dat\0All files \0*.*\0", 
		trgtlst_name, nFilterIndex) 
		!= S_OK)
	{
		return -1;
	}
#endif

	if (OpenFileDlg(0, "Fixed List File (*.dat)\0*.dat\0All files \0*.*\0", 
		fixed_name, nFilterIndex) 
		!= S_OK)
	{
		return -1;
	}



#if _USE_WINGHEMICAL_
	//готовим классы для 3D визуализации
	singleton_cleaner<win_app> app_cleaner;
	win_app * app = win_app::GetInstance();  
	app_cleaner.SetInstance(app);
	app->GetWinProject()->SetApp(app);
//	doc = (SurfDoc *)app->GetWinProject();
#else
	model * mdl = new model();
#endif

#if DIFFUSE_WORKING
	mdl->work_diffuse(infile_name);
#endif

#if SNARJAD_TARGET_WORKING
	mdl->work(start, step, fin, infile_name, trgtlst_name);
#endif

#if PROBNIY_ATOM_WORKING
	mdl->work_prob_atom(infile_name, trgtlst_name);
#endif

#if PROBNIY_ATOM_GEOMOPT
#if _USE_WINGHEMICAL_
	app->GetWinProject()->popup_Comp_work_prob_atom_GeomOpt(infile_name, trgtlst_name, box_name, fixed_name);
#else
	mdl->work_prob_atom_GeomOpt(infile_name, trgtlst_name, box_name, fixed_name);
#endif
#endif

#if DIFFUSE_WORKING

	// finally release all allocated memory and leave.
	//------------------------------------------------------------------
	TCHAR filter[] =     TEXT("Ghemical Project File (*.gpr)\0*.gpr\0")
						 TEXT("All Files (*.*)\0*.*\0");

	sprintf(outfile_name, "\0");
	if (SaveFileDlg(0, outfile_name, filter, nFilterIndex) == S_OK)
	{
		cout << "now saving file " << outfile_name << endl;
		ofstream ofile;
		ofile.open(outfile_name, ios::out);
		
		WriteGPR_v100(* mdl, ofile);
		ofile.close();
	}
#endif
	// finally release all allocated memory and leave.
#if _USE_WINGHEMICAL_
	//запуск 3D визуализации
	return StartWinLoop();
#else
	delete mdl; mdl = NULL;
	return 0;
#endif
	
}

/*################################################################################################*/

// eof
