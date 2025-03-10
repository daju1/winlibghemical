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
		"Usage: %s -t molgrouptype infile.gpr infile.box infile.traj [outfile.gpr outfile.box]\n",
		argv[0]);
}

void * MolTcfPlotHeader(void * p, i32s molgrouptype, i32s ind_mol, i32s dim, i32s crd_type1, i32s crd_type2, i32s show_directed_tcf, i32s show_integrated_tcf, const char * crd_names [][2], size_t crd_names_len)
{
	if (NULL != p)
	{
		return NULL;
	}

	if (crd_type1 >= crd_names_len) crd_type1 = 1;
	if (crd_type2 >= crd_names_len) crd_type2 = 1;
	std::string sv1 = std::string(crd_names[crd_type1][0])
					+ (strlen(crd_names[crd_type1][1]) > 0
					? std::string("(") + std::string(crd_names[crd_type1][1]) + std::string(")")
					: std::string(""));
	std::string sv2 = std::string(crd_names[crd_type2][0])
					+ (strlen(crd_names[crd_type2][1]) > 0
					? std::string("(") + std::string(crd_names[crd_type2][1]) + std::string(")")
					: std::string(""));

	std::string sv = "Time correlation functions for (" + sv1 + ", " + sv2 + ")";

	std::string title =
		std::string(GAS == molgrouptype ? "Gas " : "Membrane ")
		+ sv;

	cout << title << endl;
	const char * s1 = "frame(num), ";
	cout << s1;

	cout << "tcf, ";
	cout << "dtcf, ";
	cout << "sum_tcf, ";
	cout << "sum_dtcf";

	cout << endl;

	return NULL;
}


void MolCrdPrintValue(void * p, i32s loop, f64 value)
{

}

void MolCrdPrintValues(void * p, i32s loop, std::list<f64> & values)
{
	cout << loop;

	std::list<f64>::const_iterator it  = values.begin();
	std::list<f64>::const_iterator end = values.end();

	for (; it != end; ++it)
	{
		cout << "," << (*it);
	}

	cout << endl;
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

	i32s molgrouptype = 1;
	i32s ind_mol = 0;
	i32s dim = 2;
	i32s start_from = 1000;
	i32s tcf_frames = 1000;

	i32s crd_type1 = 0;
	i32s crd_type2 = 0;
	i32s show_directed_tcf = 0;
	i32s show_integrated_tcf = 0;

	char * infile_name;
	char * infile_box;
	char * infile_traj;

	char * outfile_name = NULL;
	char * outfile_box = NULL;


	/* handle (optional) flags first */
	while (1+flags < argc && argv[1+flags][0] == '-') {
		switch (argv[1+flags][1]) {

		case 'i':
			ind_mol = strtol(argv[2+flags], &end, 0);
			if (*end) {
				fprintf(stderr, "Error: i argment not a "
					"number!\n");
				help(argc, argv);
				exit(1);
			}
			flags++;
			break;

		case 'd':
			dim = strtol(argv[2+flags], &end, 0);
			if (*end) {
				fprintf(stderr, "Error: d argment not a "
					"number!\n");
				help(argc, argv);
				exit(1);
			}
			flags++;
			break;

		case 't':
			molgrouptype = strtol(argv[2+flags], &end, 0);
			if (*end) {
				fprintf(stderr, "Error: t argment not a "
					"number!\n");
				help(argc, argv);
				exit(1);
			}
			flags++;
			break;

		case 's':
			start_from = strtol(argv[2+flags], &end, 0);
			if (*end) {
				fprintf(stderr, "Error: start_from argment not a "
					"number!\n");
				help(argc, argv);
				exit(1);
			}
			flags++;
			break;

		case 'N':
		case 'n':
			tcf_frames = strtol(argv[2+flags], &end, 0);
			if (*end) {
				fprintf(stderr, "Error: tcf_frames argment not a "
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

	infile_name = argv[flags+1];
	infile_box  = argv[flags+2];
	infile_traj = argv[flags+3];

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
		help(argc, argv);
		return -1;
	}

	model * mdl = new model();
	ReadGPR(* mdl, ifile, false, false);
	ifile.close();

	cout << "trying to apply a box " << infile_box << " ; ";
	mdl->LoadBox(infile_box);

	mdl->OpenTrajectory(infile_traj);

	mdl->TimeCorrelationFunctionPlot(molgrouptype, ind_mol, dim,
									crd_type1, crd_type2,
									show_directed_tcf, show_integrated_tcf,
									start_from, tcf_frames,
									MolTcfPlotHeader, MolCrdPrintValue, MolCrdPrintValues, mdl);

	mdl->CloseTrajectory();

	// finally release all allocated memory and leave.

	delete mdl; mdl = NULL;

	return 0;
}

/*################################################################################################*/

// eof
