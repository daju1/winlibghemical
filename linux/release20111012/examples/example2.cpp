// EXAMPLE2.CPP

// Copyright (C) 2004 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "example2.h"
//#include "long_stream.h"

#include "notice.h"

#include "eng1_mm_tripos52.h"
#include "eng1_mm_default.h"

#include "geomopt.h"
#include "moldyn.h"

/*################################################################################################*/

int main(int argc, char ** argv)
{
	print_lib_intro_notice(cout);
	print_copyright_notice(cout);
	cout << endl;
	
	// example #2: open a file, run a geometry optimization, and save a file.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	// check that we have got both file names from the command line.
	
	if (argc != 4)
	{
		cout << "usage : example2 infile.gpr infile.box outfile.gpr" << endl;
		exit(1);
	}
	
	char * infile_name = argv[1];
	char * infile_box = argv[2];
	char * outfile_name = argv[3];

	// create a model object, and read in a file.
	
	model * mdl = new model();
	
	cout << "trying to open a file " << infile_name << " ; ";
	
	ifstream ifile;
	ifile.open(infile_name, ios::in);
	
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

#if 0
	printf(
		"mdl->use_periodic_boundary_conditions = %d\n", 
		mdl->use_periodic_boundary_conditions);

	printf(
		"mdl->saved_periodic_box_HALFdim[0] = %f\n", 
		mdl->saved_periodic_box_HALFdim[0]);
	
	printf(
		"mdl->saved_periodic_box_HALFdim[1] = %f\n", 
		mdl->saved_periodic_box_HALFdim[1]);
	
	printf(
		"mdl->saved_periodic_box_HALFdim[2] = %f\n", 
		mdl->saved_periodic_box_HALFdim[2]);
#endif
	// if everything went fine, then create a computation engine object, initialize it,
	// and create a geometry optimizer object using the computation engine object.
	
	engine * eng = mdl->GetCurrentSetup()->CreateEngineByIDNumber(ENG1_MM_PERIODIC);
//	engine * eng = mdl->GetCurrentSetup()->CreateEngineByIDNumber(ENG1_MM_TRIPOS52);
//	engine * eng = mdl->GetCurrentSetup()->CreateEngineByIDNumber(ENG1_MM_DEFAULT);

	// eng_pbc != NULL if we will use a system with periodic boundary conditions...
//	engine_pbc * eng_pbc = dynamic_cast<engine_pbc *>(eng);
#if USE_ENGINE_PBC_TST
	engine_pbc_tst * eng_pbc = dynamic_cast<engine_pbc_tst *>(eng);
#else
	engine_pbc * eng_pbc = dynamic_cast<engine_pbc *>(eng);
#endif /*USE_ENGINE_PBC_TST*/
	
	CopyCRD(mdl, eng, 0);
	//CopyLockedCRD(mdl, eng, 0);

	mdl->LoadBox(infile_box);
	
	geomopt * opt = new geomopt(eng, 100, 0.025, 10.0);
	
	// then take 100 geometry optimization steps, and print out some data about progess.
	
	for (int n1 = 0;n1 < 100;n1++)
	{
		opt->TakeCGStep(conjugate_gradient::Newton2An);
		eng_pbc->CheckAtomLocations();
		
		cout << "step = " << (n1 + 1) << "   ";
		cout << "energy = " << opt->optval << "   ";
		cout << "step length = " << opt->optstp << endl;
	}
	
	// now copy the optimized coordinates back to the model object, and save the file.
	
	CopyCRD(eng, mdl, 0);
	
	delete opt; opt = NULL;
	delete eng; eng = NULL;
	
	//------------------------------------------------------------------
	{
		cout << "now saving file " << outfile_name << endl;
		ofstream ofile;
		ofile.open(outfile_name, ios::out);
		
		WriteGPR_v100(* mdl, ofile);
		ofile.close();
	}
	
	// finally release all allocated memory and leave.
	
	delete mdl; mdl = NULL;
	
	return 0;
}

/*################################################################################################*/

// eof
