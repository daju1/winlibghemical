// EXAMPLE1.CPP

// Copyright (C) 2003 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "example1.h"
#include "..\long_stream.h"

#include "..\notice.h"

#include "..\eng1_mm_tripos52.h"
#include "..\eng1_mm_default.h"

#include "..\geomopt.h"
#include "..\moldyn.h"

#include <windows.h>
HINSTANCE hInst;

#include "../../../consol/filedlg.h"

/*################################################################################################*/

int main(int argc, char* argv[])
{
	print_lib_intro_notice(cout);
	print_copyright_notice(cout);
	cout << endl;
	// example #1: open a file and run a geometry optimization.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	char filename[1048];
	DWORD nFilterIndex;
	if (OpenFileDlg(0, "Ghemical Project File (*.gpr)\0*.gpr\0All files \0*.*\0", filename, nFilterIndex) 
		!= S_OK)
	{
		return -1;
	}
	// create a model object, and read in a file.
	
	model * mdl = new model();
	
	cout << "trying to open a file... ";
		


	ifstream ifile;
	ifile.open(filename, ios::in);
	//ifile.open("example_file.gpr", ios::in);
	
	if (ifile.fail())
	{
		cout << "failed!!!" << endl;
		
		ifile.close();
		delete mdl;
		
		return -1;
	}
	
	cout << "ok!!!" << endl;
	
	ReadGPR(* mdl, ifile, false, false);
	ifile.close();
	
	// if everything went fine, then create a computation engine object, initialize it,
	// and create a geometry optimizer object using the computation engine object.
	
	engine * eng = mdl->GetCurrentSetup()->CreateEngineByIDNumber(CURRENT_ENG1_MM);
//	engine * eng = mdl->GetCurrentSetup()->CreateEngineByIDNumber(ENG1_MM_TRIPOS52);
//	engine * eng = mdl->GetCurrentSetup()->CreateEngineByIDNumber(ENG1_MM_DEFAULT);
	
	CopyCRD(mdl, eng, 0);
	CopyLockedCRD(mdl, eng, 0);
	
	geomopt * opt = new geomopt(eng, 100, 0.025, 10.0);
	
	// then take 100 geometry optimization steps, and print out some data about progess.
	
	for (int n1 = 0;n1 < 100;n1++)
	{
		opt->TakeCGStep(conjugate_gradient::Newton2An);
		
		cout << "step = " << (n1 + 1) << "   ";
		cout << "energy = " << opt->optval << "   ";
		cout << "step length = " << opt->optstp << endl;
	}
	
	// finally release all allocated memory and leave.
	
	delete opt; opt = NULL;
	delete eng; eng = NULL;
	
	delete mdl; mdl = NULL;

	return 0;
}

/*################################################################################################*/

// eof
