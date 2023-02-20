// EXAMPLE2.CPP

// Copyright (C) 2004 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "example2.h"

#include "notice.h"

#include "eng1_mm_tripos52.h"
#include "eng1_mm_default.h"

#include "geomopt.h"
#include "moldyn.h"

#include <string.h>

/*################################################################################################*/
static void help(int argc, char ** argv)
{
	fprintf(stderr,
		"Usage: %s [-T target_T] [-t timestep] [-N nsteps_s] [-v traj_version]  [-f frame_save_frq] [-E] [-P] infile.gpr infile.box outfile.traj [outfile.gpr outfile.box]\n",
		argv[0]);
}

int main(int argc, char ** argv)
{
	print_lib_intro_notice(cout);
	print_copyright_notice(cout);
	cout << endl;

	libghemical_init(LIBDATA_PATH);
	
	// example #2: open a file, run a geometry optimization, and save a file.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	char *end;
	int flags = 0;

	char * infile_name;
	char * infile_box;
	char * outfile_traj;

	char * outfile_name = NULL;
	char * outfile_box = NULL;

	f64 temperature = 300.;
	f64 timestep = 0.5;
	int N = 18000;
	int traj_version = 15;
	int frame_save_frq = 10000;
	bool const_E = false;
	bool const_P = false;
	bool inverse_time = false;
	box_optimization_moldyn_mode box_optimization = box_optimization_always;

	/* handle (optional) flags first */
	while (1+flags < argc && argv[1+flags][0] == '-') {
		switch (argv[1+flags][1]) {
		case 'E': const_E = true; break;
		case 'P': const_P = true; break;
		case 'I': inverse_time = true; break;
		case 'B':
			box_optimization = (box_optimization_moldyn_mode) strtol(argv[2+flags], &end, 0);
			if (*end) {
				fprintf(stderr, "Error: B argment not a "
					"number!\n");
				help(argc, argv);
				exit(1);
			}
			flags++;
			break;
		case 'N':
			N = strtol(argv[2+flags], &end, 0);
			if (*end) {
				fprintf(stderr, "Error: N argment not a "
					"number!\n");
				help(argc, argv);
				exit(1);
			}
			flags++;
			break;
		case 'v':
			traj_version = strtol(argv[2+flags], &end, 0);
			if (*end) {
				fprintf(stderr, "Error: v argment not a "
					"number!\n");
				help(argc, argv);
				exit(1);
			}
			flags++;
			break;
		case 'f':
			frame_save_frq = strtol(argv[2+flags], &end, 0);
			if (*end) {
				fprintf(stderr, "Error: f argment not a "
					"number!\n");
				help(argc, argv);
				exit(1);
			}
			flags++;
			break;
		case 'T':
			temperature = strtod_l(argv[2+flags], &end, 0);
			if (*end) {
				fprintf(stderr, "Error: temperature argment not a "
					"number!\n");
				help(argc, argv);
				exit(1);
			}
			flags++;
			break;
		case 't':
			timestep = strtod_l(argv[2+flags], &end, 0);
			if (*end) {
				fprintf(stderr, "Error: timestep argment not a "
					"number!\n");
				help(argc, argv);
				exit(1);
			}
			flags++;
			break;
		default:
			fprintf(stderr, "Error: Unsupported option "
				"\"%s\"!\n", argv[1+flags]);
			help(argc, argv);
			exit(1);
		}
		flags++;
	}

	if (argc < flags + 4) {
		fprintf(stderr, "Error: No infile.gpr infile.box outfile.traj specified!\n");
		help(argc, argv);
		exit(1);
	}

	infile_name  = argv[flags+1];
	infile_box   = argv[flags+2];
	outfile_traj = argv[flags+3];

	/* read address range if present */
	if (argc == flags + 6) {
		outfile_name = argv[flags+4];
		outfile_box = argv[flags+5];
	} else if (argc != flags + 4) {
		help(argc, argv);
		exit(1);
	}

	cout << "trying to open a file " << infile_name << " ; ";
	
	ifstream ifile;
	ifile.open(infile_name, ios::in);
	if (ifile.fail())
	{
		cout << "failed!!!" << endl;
		
		ifile.close();
		return -1;
	}

	model * mdl = new model();
	ReadGPR(* mdl, ifile, false, false);
	ifile.close();

	cout << "trying to apply a box " << infile_box << " ; ";
	mdl->LoadBox(infile_box);

	mdl->SetTrajectoryVersionBeforeMoldyn(traj_version);
	mdl->SetTrajectoryFrameSaveFrq(frame_save_frq);

	//mdl->DoMolDyn()
	{
		const bool updt = true;
		setup * su = mdl->GetCurrentSetup();

		moldyn_param param = moldyn_param(su);

		//param.load_last_frame = true;

		//param.nsteps_h = 0;		// heat
		//param.nsteps_e = 0;		// equilibration
		param.nsteps_s = N;		// simulation
		cout << "nsteps_s =" << param.nsteps_s << endl;

		if (const_E)
		{
			param.constant_T = false;
		}
		else
		{
			param.constant_T = true;
		}
		cout << "constant_T =" << param.constant_T << endl;

		if (const_P)
		{
			param.constant_P = true;
		}
		else
		{
			param.constant_P = false;
		}
		cout << "constant_P = " << param.constant_P << endl;

		param.inverse_time_init = inverse_time;
		cout << "inverse_time_init = " << param.inverse_time_init << endl;

		param.box_optimization = box_optimization;
		cout << "box_optimization = " << param.box_optimization << endl;

		param.target_T = temperature;
		cout << "target_T = " << param.target_T << endl;
		param.timestep = timestep;
		cout << "timestep = " << param.timestep << endl;
		strncpy(param.filename, outfile_traj, 256);
		cout << "outfile_traj = " << param.filename << endl;

		//strcpy(md.filename, infile_name);
		mdl->DoMolDyn(param, updt);
	}

	//------------------------------------------------------------------
	if (outfile_name && outfile_box)
	{
		cout << "now saving file " << outfile_name << endl;
		ofstream ofile;
		ofile.open(outfile_name, ios::out);
		
		WriteGPR_v100(* mdl, ofile);

		ofile.close();

		cout << "now saving box " << outfile_box << endl;
		mdl->SaveBox(outfile_box);
	}
	
	// finally release all allocated memory and leave.
	
	delete mdl; mdl = NULL;
	
	return 0;
}

/*################################################################################################*/

// eof
