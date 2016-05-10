// STREAMIO.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

// this file is taken from the ghemical source tree (src/fileio.cpp) and slightly modified.
// 1) "project" objects changed to "model" objects. 2) some (less important) sections commented out.
#include "stdafx.h"

#if 0

#include "..\model.h"		// modified!!!
#include "..\engine.h"		// modified!!!
#include "..\eng1_qm.h"		// modified!!!
#include "..\eng1_mm.h"		// modified!!!
#include "..\eng1_sf.h"		// modified!!!
#include "..\eng2_qm_mm.h"		// modified!!!
#include "..\eng2_mm_sf.h"		// modified!!!

#include "example3.h"

#else
#include "model.h"		// modified!!!
#include "engine.h"		// modified!!!
#include "eng1_qm.h"		// modified!!!
#include "eng1_mm.h"		// modified!!!
#include "eng1_sf.h"		// modified!!!
#include "eng2_qm_mm.h"		// modified!!!
#include "eng2_mm_sf.h"		// modified!!!

#include "simple_test/example3.h"

#endif

#include <iomanip>
#include <strstream>
using namespace std;

//#define VERBOSE		// for command-line debugging only!!! BEWARE!!!

/*################################################################################################*/

bool ReadCGI(silent_model & prj, istream & file, bool skipheader)
{
	const int current_version_number = 1000;	// version number multiplied by 1000 -> int.
	char buffer[1024];
	
	if (!skipheader)
	{
		// first we have to identify the file version,
		// and then call the matching function, if necessary.
		
		if (file.peek() != '%')
		{
			cout << "%Error unknown_file_format" << endl;
			exit(EXIT_FAILURE);
		}
		
		// now we expect to see "%Header" section.
		// next we will check the file type and version information...
		
		int version;
		file >> buffer;		// read "%Header".
		file >> buffer;		// read the file type.
		file >> version;	// read the version number;
		
		if (strcmp(buffer, "ghemical-cgi") != 0)
		{
			cout << "%Error unknown_file_type " << buffer << endl;
			exit(EXIT_FAILURE);
		}
		
		file.getline(buffer, sizeof(buffer));
		
		// now look at the version number, and pick a correct function if needed.
		
		if (version != current_version_number)
		{
			cout << "%Error unknown_file_version " << version << endl;
			exit(EXIT_FAILURE);
		}
	}
	
	// the "%Header" section is now processed,
	// and the actual file input begins!!!
	
#ifdef VERBOSE
cout << "%Debug model::ReadStream() : starting file input." << endl;
#endif	// VERBOSE

	i32u crd_set_counter = 0;
	vector<atom *> ar_vector;
	vector<bond *> br_vector;
	
	// now we just read the sections, until we hit into "%End"-section and leave the loop...
	
	while (true)
	{
		file >> buffer;		// read the section string.
		
		if (!strcmp(buffer, "%End")) break;		// handle "%End" section.
		else if (!strcmp(buffer, "%Info"))		// handle "%Info" section.
		{
			i32s n1; file >> n1;		// number of crd-sets
			char cn[16]; file >> cn;	// the setyp classname
			i32u engid; file >> engid;	// the engine class ID
			
			int n2 = n1 - prj.GetCRDSetCount();
			if (n2 > 0)
			{
				i32u n3 = prj.GetCRDSetCount(); prj.PushCRDSets((i32u) n2);
				for (i32u n4 = n3;n4 < prj.GetCRDSetCount();n4++) prj.SetCRDSetVisible(n4, true);
			}
			
#ifdef VERBOSE
cout << "%Debug handled \"%Info\" section, added " << n2 << " coordinate sets." << endl;
cout << "%Debug classname = " << cn << " engid = " << engid << " - changing setup to these." << endl;
#endif	// VERBOSE

			file.getline(buffer, sizeof(buffer));
			
		}
		else if (!strcmp(buffer, "%Atoms"))		// handle "%Atoms" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "%Debug found \"%Atoms\" section..." << endl;
#endif	// VERBOSE

			while (file.peek() != '%')
			{
				i32u n1; file >> n1;	// id number
				i32s n2; file >> n2;	// atomic number
				
#ifdef VERBOSE
if (n1 != ar_vector.size()) cout << "%Debug WARNING : atom id number mismatch!!!" << endl;
#endif	// VERBOSE

				element tmp_element(n2);
				atom newatom(tmp_element, NULL, prj.GetCRDSetCount());
				
//newatom.flags = n3;
//if (selected) newatom.flags |= ATOMFLAG_SELECTED;
				
				prj.AddAtom(newatom);

				ar_vector.push_back(& prj.GetLastAtom());
				
				file.getline(buffer, sizeof(buffer));
			}
		}
		else if (!strcmp(buffer, "%Bonds"))		// handle "%Bonds" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "%Debug found \"%Bonds\" section..." << endl;
#endif	// VERBOSE

			while (file.peek() != '%')
			{
				i32u id1; file >> id1;	// 1st id number
				i32u id2; file >> id2;	// 2nd id number
				
				char bt; file >> bt;	// bond type
				
if (id1 >= ar_vector.size()) { cout << "%Error invalid_atom_id_number" << endl; exit(EXIT_FAILURE); }
if (id2 >= ar_vector.size()) { cout << "%Error invalid_atom_id_number" << endl; exit(EXIT_FAILURE); }

				bondtype tmp_bondtype(bt);
				bond newbond(ar_vector[id1], ar_vector[id2], tmp_bondtype);
				prj.AddBond(newbond);
				
				br_vector.push_back(& prj.GetLastBond());
				
				file.getline(buffer, sizeof(buffer));
			}
		}
		else if (!strcmp(buffer, "%Coord"))		// handle "%Coord" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "%Debug found \"%Coord\" section..." << endl;
if (crd_set_counter >= prj.GetCRDSetCount()) { cout << "%Debug ERROR : too many crd-sets!!!" << endl; exit(EXIT_FAILURE); }
#endif	// VERBOSE

			while (file.peek() != '%')
			{
				i32u n1; file >> n1;	// id number
				
				fGL xcrd; file >> xcrd;	// x-coordinate
				fGL ycrd; file >> ycrd;	// y-coordinate
				fGL zcrd; file >> zcrd;	// z-coordinate
				
if (n1 >= ar_vector.size()) { cout << "%Error invalid_atom_id_number" << endl; exit(EXIT_FAILURE); }

				atom * ar = ar_vector[n1];
				ar->SetCRD(crd_set_counter, xcrd, ycrd, zcrd);
				
				file.getline(buffer, sizeof(buffer));
			}
			
			crd_set_counter++;
		}
		else						// handle an unknown section...
		{
#ifdef VERBOSE
cout << "%Debug WARNING : found an unknown section " << buffer << endl;
#endif	// VERBOSE
			
			file.getline(buffer, sizeof(buffer));
			while (file.peek() != '%')
			{
				file.getline(buffer, sizeof(buffer));
			}
		}
	}
	
	// ok, we are ready now!
	
	return true;
}

/*################################################################################################*/

void WriteCGI_Coord(silent_model & prj, ostream & file, i32u cset)
{
	// write a "%Coord" section.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^
	
	file << "%Coord" << endl;
	
	iter_al it1 = prj.GetAtomsBegin();
	while (it1 != prj.GetAtomsEnd())
	{
		const fGL * cdata = (* it1).GetCRD(cset);
		
		file << (* it1).index << " ";
		file.setf(ios::showpos);
		file << setprecision(15) << cdata[0] << " ";
		file << setprecision(15) << cdata[1] << " ";
		file << setprecision(15) << cdata[2] << " ";
		file.unsetf(ios::showpos);
		
		file << endl;
		it1++;
	}
}

/*################################################################################################*/

// eof
