// FILEIO.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "project.h"

#include <ghemical/engine.h>
#include <ghemical/eng1_qm.h>
#include <ghemical/eng1_mm.h>
#include <ghemical/eng1_sf.h>
#include <ghemical/eng2_qm_mm.h>
#include <ghemical/eng2_mm_sf.h>

#include <iomanip>
#include <strstream>
using namespace std;

//#define VERBOSE	// un-comment this to enable debug-messages...

static char * my_num_locale = NULL;	// the temporary locale strings returned by setlocale() will be stored here...

/*################################################################################################*/

bool ReadGPR_OLD(project & prj, istream & file, bool, bool)
{
	static const char message[] = "Hmmm. It seems that we are trying to read a VERY OLD version of this file.\nIt's not possible to verify the type and version of the file, so this\noperation may fail. Are you sure you wish to try (Y/N) ??? ";
	bool result = prj.Question(message); if (!result) return false;
	
	vector<atom *> tmp1; i32s tmp2[2]; char tmp3; fGL tmp4[3];
	
	char buffer[1024];
	
#ifdef VERBOSE
cout << "model::ReadStream_OLD() : starting file input." << endl;
#endif	// VERBOSE

	while (file.peek() != 'E')
	{
		file >> tmp2[0] >> tmp2[1];
		file >> tmp4[0] >> tmp4[1] >> tmp4[2] >> tmp2[0]; file.get();
		
		element el = element(tmp2[1]);
		atom newatom(el, tmp4, prj.GetCRDSetCount());
		prj.Add_Atom(newatom); tmp1.push_back(& prj.GetLastAtom());
		
		for (i32s n1 = 0;n1 < tmp2[0];n1++)
		{
			file >> tmp2[1] >> tmp3;
			if (tmp2[1] < (i32s) tmp1.size())
			{
				bond newbond(tmp1[tmp2[1]], tmp1[tmp1.size() - 1], bondtype(tmp3));
				prj.AddBond(newbond);
			}
		}
		
		file.getline(buffer, sizeof(buffer));
	}
	
	return true;
}

bool ReadGPR_v100(project & prj, istream & file, bool selected, bool skipheader)
{
	char buffer[1024];
	
	if (!skipheader)
	{
		// first we have to identify the file version, and then call the matching function,
		// if necessary. the very first file format did not use the "!Section" idea, so
		// we must skip the test if no such traces are seen...
		
		int version;
		if (file.peek() != '!') version = -1;
		else
		{
			// now we expect to see "!Header" section.
			// next we will check the file type and version information...
			
			file >> buffer;		// read "!Header".
			file >> buffer;		// read the file type.
			file >> version;	// read the version number;
			
			file.getline(buffer, sizeof(buffer));
		}
		
		// now look at the version number, and pick a correct function if needed.
		
	/*	if (version < 100) return ReadGPR_OLD(prj, file, selected, true);
		else if (version > 100)
		{
			static const char message[] = "Ooops. It seems that we are trying to read a FUTURE version of this file.\nIt's not sure if we can handle that. Are you sure you wish to try (Y/N) ??? ";
			bool result = prj.Question(message); if (!result) return false;
		}	*/
	}
	
	// the "!Header" section is now processed,
	// and the actual file input begins!!!
	
#ifdef VERBOSE
cout << "model::ReadStream() : starting file input." << endl;
#endif	// VERBOSE

	i32u crd_set_counter = 0;
	vector<atom *> ar_vector;
	
	// now we just read the sections, until we hit into "!End"-section and leave the loop...
	
	while (true)
	{
		file >> buffer;		// read the section string.
		
		if (!strcmp(buffer, "!End")) break;		// handle "!End" section.
		else if (!strcmp(buffer, "!Info"))		// handle "!Info" section.
		{
			int n1; file >> n1;
			
			int n2 = n1 - prj.GetCRDSetCount();
			if (n2 > 0)
			{
				i32u n3 = prj.GetCRDSetCount(); prj.PushCRDSets((i32u) n2);
				for (i32u n4 = n3;n4 < prj.GetCRDSetCount();n4++) prj.SetCRDSetVisible(n4, true);
			}
			
#ifdef VERBOSE
cout << "handled \"!Info\" section, added " << n2 << " coordinate sets." << endl;
#endif	// VERBOSE

			file.getline(buffer, sizeof(buffer));
			
			// TODO : read coordinate set descriptions (if there are some)...
			// TODO : read coordinate set descriptions (if there are some)...
			// TODO : read coordinate set descriptions (if there are some)...
		}
		else if (!strcmp(buffer, "!Atoms"))		// handle "!Atoms" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "found \"!Atoms\" section..." << endl;
#endif	// VERBOSE

			while (file.peek() != '!')
			{
				i32u n1; file >> n1;	// id number
				i32s n2; file >> n2;	// atomic number
				
#ifdef VERBOSE
if (n1 != ar_vector.size()) cout << "WARNING : bad atom id numbers!!!" << endl;
#endif	// VERBOSE

				element tmp_element(n2);
				atom newatom(tmp_element, NULL, prj.GetCRDSetCount());
				
				if (selected) newatom.flags |= ATOMFLAG_SELECTED;
				
				prj.Add_Atom(newatom);

				ar_vector.push_back(& prj.GetLastAtom());
				
				file.getline(buffer, sizeof(buffer));
			}
		}
		else if (!strcmp(buffer, "!Bonds"))		// handle "!Bonds" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "found \"!Bonds\" section..." << endl;
#endif	// VERBOSE

			while (file.peek() != '!')
			{
				i32u id1; file >> id1;	// 1st id number
				i32u id2; file >> id2;	// 2nd id number
				
				char bt; file >> bt;	// bond type
				
#ifdef VERBOSE
if (id1 >= ar_vector.size()) cout << "WARNING : bad atom id numbers!!!" << endl;
if (id2 >= ar_vector.size()) cout << "WARNING : bad atom id numbers!!!" << endl;
#endif	// VERBOSE

				bondtype tmp_bondtype(bt);
				bond newbond(ar_vector[id1], ar_vector[id2], tmp_bondtype);
				prj.AddBond(newbond);
				
				file.getline(buffer, sizeof(buffer));
			}
		}
		else if (!strcmp(buffer, "!Coord"))		// handle "!Coord" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "found \"!Coord\" section..." << endl;
if (crd_set_counter >= prj.GetCRDSetCount()) cout << "ERROR : too many crd-sets!!!" << endl;
#endif	// VERBOSE

			while (file.peek() != '!')
			{
				i32u n1; file >> n1;	// id number
				
				fGL xcrd; file >> xcrd;	// x-coordinate
				fGL ycrd; file >> ycrd;	// y-coordinate
				fGL zcrd; file >> zcrd;	// z-coordinate
				
#ifdef VERBOSE
if (n1 >= ar_vector.size()) cout << "ERROR : bad atom id numbers!!!" << endl;
#endif	// VERBOSE

				atom * ar = ar_vector[n1];
				ar->SetCRD(crd_set_counter, xcrd, ycrd, zcrd);
				
				file.getline(buffer, sizeof(buffer));
			}
			
			crd_set_counter++;
		}
		else if (!strcmp(buffer, "!Charges"))		// handle "!Charges" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "found \"!Charges\" section..." << endl;
#endif	// VERBOSE

			while (file.peek() != '!')
			{
				i32u n1; file >> n1;	// id number
				f64 n2; file >> n2;	// charge
				
#ifdef VERBOSE
if (n1 >= ar_vector.size()) cout << "WARNING : bad atom id numbers!!!" << endl;
#endif	// VERBOSE

				atom * ar = ar_vector[n1];
				ar->charge = n2;
				
				file.getline(buffer, sizeof(buffer));
			}
		}
		else						// handle an unknown section...
		{
			file.getline(buffer, sizeof(buffer));
			
			while (file.peek() != '!')
			{
				file.getline(buffer, sizeof(buffer));
			}
		}
	}
	
	return true;
}

bool ReadGPR_v110(project & prj, istream & file, bool selected, bool skipheader)
{
	char buffer[1024];
	
	if (!skipheader)
	{
		// first we have to identify the file version, and then call the matching function,
		// if necessary. the very first file format did not use the "!Section" idea, so
		// we must skip the test if no such traces are seen...
		
		int version;
		if (file.peek() != '!') version = -1;
		else
		{
			// now we expect to see "!Header" section.
			// next we will check the file type and version information...
			
			file >> buffer;		// read "!Header".
			file >> buffer;		// read the file type.
			file >> version;	// read the version number;
			
			file.getline(buffer, sizeof(buffer));
		}
		
		// now look at the version number, and pick a correct function if needed.
		
	/*	if (version < 110 && version >= 100) return ReadGPR_v100(prj, file, selected, true);
		else if (version < 100) return ReadGPR_OLD(prj, file, selected, true);
		else if (version > current_version_number)
		{
			static const char message[] = "Ooops. It seems that we are trying to read a FUTURE version of this file.\nIt's not sure if we can handle that. Are you sure you wish to try (Y/N) ??? ";
			bool result = prj.Question(message); if (!result) return false;
		}	*/
	}
	
	// the "!Header" section is now processed,
	// and the actual file input begins!!!
	
#ifdef VERBOSE
cout << "model::ReadStream() : starting file input." << endl;
#endif	// VERBOSE
	
	delete prj.current_setup;			// get rid of the old setup...
	prj.current_setup = new setup1_mm(& prj);	// ...and make a new temporary one.
	
	char su_cn[16] = "unknown";	// the setup class classname.
	i32s engid = NOT_DEFINED;	// the engine class ID.
	
	i32u crd_set_counter = 0;
	vector<atom *> ar_vector;
	vector<bond *> br_vector;
	
	// now we just read the sections, until we hit into "!End"-section and leave the loop...
	
	while (true)
	{
		file >> buffer;		// read the section string.
		
		if (!strcmp(buffer, "!End")) break;		// handle "!End" section.
		else if (!strcmp(buffer, "!Info"))		// handle "!Info" section.
		{
			i32s n1; file >> n1;		// number of crd-sets
			file >> su_cn;			// the setup class classname.
			file >> engid;			// the engine class ID.
			
			int n2 = n1 - prj.GetCRDSetCount();
			if (n2 > 0)
			{
				i32u n3 = prj.GetCRDSetCount(); prj.PushCRDSets((i32u) n2);
				for (i32u n4 = n3;n4 < prj.GetCRDSetCount();n4++) prj.SetCRDSetVisible(n4, true);
			}
			
#ifdef VERBOSE
cout << "handled \"!Info\" section, added " << n2 << " coordinate sets." << endl;
cout << "classname = " << su_cn << " engid = " << engid << " - changing setup to these later." << endl;
#endif	// VERBOSE

			file.getline(buffer, sizeof(buffer));
			
			// TODO : read coordinate set descriptions (if there are some)...
			// TODO : read coordinate set descriptions (if there are some)...
			// TODO : read coordinate set descriptions (if there are some)...
		}
		else if (!strcmp(buffer, "!Atoms"))		// handle "!Atoms" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "found \"!Atoms\" section..." << endl;
#endif	// VERBOSE

			while (file.peek() != '!')
			{
				i32u n1; file >> n1;	// id number
				i32s n2; file >> n2;	// atomic number
				i32u n3; file >> n3;	// the flags
				
if (n1 != ar_vector.size()) cout << "WARNING : atom id number mismatch!!!" << endl;

				element tmp_element(n2);
				atom newatom(tmp_element, NULL, prj.GetCRDSetCount());
				
				newatom.flags = n3;
				if (selected) newatom.flags |= ATOMFLAG_SELECTED;
				
				prj.Add_Atom(newatom);

				ar_vector.push_back(& prj.GetLastAtom());
				
				file.getline(buffer, sizeof(buffer));
			}
		}
		else if (!strcmp(buffer, "!Bonds"))		// handle "!Bonds" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "found \"!Bonds\" section..." << endl;
#endif	// VERBOSE

			while (file.peek() != '!')
			{
				i32u id1; file >> id1;	// 1st id number
				i32u id2; file >> id2;	// 2nd id number
				
				char bt; file >> bt;	// bond type
				
if (id1 >= ar_vector.size()) { cout << "ERROR : invalid atom id number!!!" << endl; exit(EXIT_FAILURE); }
if (id2 >= ar_vector.size()) { cout << "ERROR : invalid atom id number!!!" << endl; exit(EXIT_FAILURE); }

				bondtype tmp_bondtype(bt);
				bond newbond(ar_vector[id1], ar_vector[id2], tmp_bondtype);
				prj.AddBond(newbond);
				
				br_vector.push_back(& prj.GetLastBond());
				
				file.getline(buffer, sizeof(buffer));
			}
		}
		else if (!strcmp(buffer, "!Coord"))		// handle "!Coord" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "found \"!Coord\" section..." << endl;
if (crd_set_counter >= prj.GetCRDSetCount()) { cout << "ERROR : too many crd-sets!!!" << endl; exit(EXIT_FAILURE); }
#endif	// VERBOSE

			while (file.peek() != '!')
			{
				i32u n1; file >> n1;	// id number
				
				fGL xcrd; file >> xcrd;	// x-coordinate
				fGL ycrd; file >> ycrd;	// y-coordinate
				fGL zcrd; file >> zcrd;	// z-coordinate
				
if (n1 >= ar_vector.size()) { cout << "ERROR : invalid atom id number!!!" << endl; exit(EXIT_FAILURE); }

				atom * ar = ar_vector[n1];
				ar->SetCRD(crd_set_counter, xcrd, ycrd, zcrd);
				
				file.getline(buffer, sizeof(buffer));
			}
			
			crd_set_counter++;
		}
		else if (!strcmp(buffer, "!Charges"))		// handle "!Charges" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "found \"!Charges\" section..." << endl;
#endif	// VERBOSE

			while (file.peek() != '!')
			{
				i32u n1; file >> n1;	// id number
				f64 n2; file >> n2;	// charge
				
if (n1 >= ar_vector.size()) { cout << "ERROR : invalid atom id number!!!" << endl; exit(EXIT_FAILURE); }

				atom * ar = ar_vector[n1];
				ar->charge = n2;
				
				file.getline(buffer, sizeof(buffer));
			}
		}
		else						// handle an unknown section...
		{
			file.getline(buffer, sizeof(buffer));
			
			while (file.peek() != '!')
			{
				file.getline(buffer, sizeof(buffer));
			}
		}
	}
	
	// change the current_setup and current_eng using the settings from file.
	// try to find a new setup class; if no success use the setup1_mm as a default.
	
	delete prj.current_setup;	// get rid of the temporary setup...
	prj.current_setup = NULL;	// ...and mark it invalid.
	
	if (!prj.current_setup && !strcmp(setup1_qm::static_GetClassName(), su_cn)) prj.current_setup = new setup1_qm(& prj);
	if (!prj.current_setup && !strcmp(setup1_mm::static_GetClassName(), su_cn)) prj.current_setup = new setup1_mm(& prj);
	if (!prj.current_setup && !strcmp(setup1_sf::static_GetClassName(), su_cn)) prj.current_setup = new setup1_sf(& prj, false);
	if (!prj.current_setup && !strcmp(setup2_qm_mm::static_GetClassName(), su_cn)) prj.current_setup = new setup2_qm_mm(& prj);
	if (!prj.current_setup && !strcmp(setup2_mm_sf::static_GetClassName(), su_cn)) prj.current_setup = new setup2_mm_sf(& prj, false);
	
	if (!prj.current_setup)
	{
		prj.WarningMessage("did not find a matching setup");
		prj.current_setup = new setup1_mm(& prj);
	}
	
	// now try to set a correct value for current_eng_index
	
	i32u index = 0;
	while (index < prj.GetCurrentSetup()->GetEngineCount())
	{
		i32u id = prj.GetCurrentSetup()->GetEngineIDNumber(index);
		if (id == (i32u) engid) break;
		
		index++;
	}
	
	if (index == prj.GetCurrentSetup()->GetEngineCount())
	{
		prj.WarningMessage("did not find a matching engid");
		index = 0;
	}
	
	prj.GetCurrentSetup()->SetCurrentEngIndex(index);
	
	// ok, we are ready now!
	
	return true;
}

/*################################################################################################*/

bool ReadGPR(project & prj, istream & file, bool selected, bool skipheader)
{
	const int current_version_number = 111;		// version number multiplied by 100 -> int.
	char buffer[1024];
	
	if (my_num_locale != NULL) cout << "WARNING : my_num_locale != NULL" << endl;	// todo : change into a thread-safe lock???
	// store the current (numeric) locale into my_num_locale, and switch into "C" numeric locale...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	my_num_locale = setlocale(LC_NUMERIC, NULL);
	setlocale(LC_NUMERIC, "C");
	
	if (!skipheader)
	{
		// first we have to identify the file version, and then call the matching function,
		// if necessary. the very first file format did not use the "!Section" idea, so
		// we must skip the test if no such traces are seen...
		
		int version;
		if (file.peek() != '!') version = -1;
		else
		{
			// now we expect to see "!Header" section.
			// next we will check the file type and version information...
			
			file >> buffer;		// read "!Header".
			file >> buffer;		// read the file type.
			file >> version;	// read the version number;
			
			file.getline(buffer, sizeof(buffer));
		}
		
		// now look at the version number, and pick a correct function if needed.
		
		if (version == 110)
		{
			bool result = ReadGPR_v110(prj, file, selected, true);
			
			// change the original locale back...
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			setlocale(LC_NUMERIC, my_num_locale);
			my_num_locale = NULL;
			
			return result;
		}
		if (version < 110 && version >= 100)
		{
			bool result = ReadGPR_v100(prj, file, selected, true);
			
			// change the original locale back...
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			setlocale(LC_NUMERIC, my_num_locale);
			my_num_locale = NULL;
			
			return result;
		}
		else if (version < 100)
		{
			bool result = ReadGPR_OLD(prj, file, selected, true);
			
			// change the original locale back...
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			setlocale(LC_NUMERIC, my_num_locale);
			my_num_locale = NULL;
			
			return result;
		}
		else if (version > current_version_number)
		{
			static const char message[] = "Ooops. It seems that we are trying to read a FUTURE version of this file.\nIt's not sure if we can handle that. Are you sure you wish to try (Y/N) ??? ";
			bool result = prj.Question(message);
			if (!result)
			{
				// change the original locale back...
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
				setlocale(LC_NUMERIC, my_num_locale);
				my_num_locale = NULL;
				
				return false;
			}
		}
	}
	
	// the "!Header" section is now processed,
	// and the actual file input begins!!!
	
#ifdef VERBOSE
cout << "model::ReadStream() : starting file input." << endl;
#endif	// VERBOSE
	
	delete prj.current_setup;			// get rid of the old setup...
	prj.current_setup = new setup1_mm(& prj);	// ...and make a new temporary one.
	
	char su_cn[16] = "unknown";	// the setup class classname.
	i32s engid = NOT_DEFINED;	// the engine class ID.
	
	i32u crd_set_counter = 0;
	vector<atom *> ar_vector;
	vector<bond *> br_vector;
	
	// now we just read the sections, until we hit into "!End"-section and leave the loop...
	
	while (true)
	{
		file >> buffer;		// read the section string.
		
		if (!strcmp(buffer, "!End")) break;		// handle "!End" section.
		else if (!strcmp(buffer, "!Info"))		// handle "!Info" section.
		{
			i32s n1; file >> n1;		// number of crd-sets
			file >> su_cn;			// the setup class classname.
			file >> engid;			// the engine class ID.
			
			int n2 = n1 - prj.GetCRDSetCount();
			if (n2 > 0)
			{
				i32u n3 = prj.GetCRDSetCount(); prj.PushCRDSets((i32u) n2);
				for (i32u n4 = n3;n4 < prj.GetCRDSetCount();n4++) prj.SetCRDSetVisible(n4, true);
			}
			
#ifdef VERBOSE
cout << "handled \"!Info\" section, added " << n2 << " coordinate sets." << endl;
cout << "classname = " << su_cn << " engid = " << engid << " - changing setup to these later." << endl;
#endif	// VERBOSE

			file.getline(buffer, sizeof(buffer));
			
			// TODO : read coordinate set descriptions (if there are some)...
			// TODO : read coordinate set descriptions (if there are some)...
			// TODO : read coordinate set descriptions (if there are some)...
		}
		else if (!strcmp(buffer, "!Atoms"))		// handle "!Atoms" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "found \"!Atoms\" section..." << endl;
#endif	// VERBOSE

			while (file.peek() != '!')
			{
				i32u n1; file >> n1;	// id number
				i32s n2; file >> n2;	// atomic number
				i32s n3; file >> n3;	// formal charge
				i32u n4; file >> n4;	// the flags
				
if (n1 != ar_vector.size()) cout << "WARNING : atom id number mismatch!!!" << endl;

				element tmp_element(n2);
				atom newatom(tmp_element, NULL, prj.GetCRDSetCount());
				newatom.formal_charge = n3;
				
				newatom.flags = n4;
				if (selected) newatom.flags |= ATOMFLAG_SELECTED;
				
				prj.Add_Atom(newatom);

				ar_vector.push_back(& prj.GetLastAtom());
				
				file.getline(buffer, sizeof(buffer));
			}
		}
		else if (!strcmp(buffer, "!Bonds"))		// handle "!Bonds" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "found \"!Bonds\" section..." << endl;
#endif	// VERBOSE

			while (file.peek() != '!')
			{
				i32u id1; file >> id1;	// 1st id number
				i32u id2; file >> id2;	// 2nd id number
				
				char bt; file >> bt;	// bond type
				
if (id1 >= ar_vector.size()) { cout << "ERROR : invalid atom id number!!!" << endl; exit(EXIT_FAILURE); }
if (id2 >= ar_vector.size()) { cout << "ERROR : invalid atom id number!!!" << endl; exit(EXIT_FAILURE); }

				bondtype tmp_bondtype(bt);
				bond newbond(ar_vector[id1], ar_vector[id2], tmp_bondtype);
				prj.AddBond(newbond);
				
				br_vector.push_back(& prj.GetLastBond());
				
				file.getline(buffer, sizeof(buffer));
			}
		}
		else if (!strcmp(buffer, "!Coord"))		// handle "!Coord" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "found \"!Coord\" section..." << endl;
if (crd_set_counter >= prj.GetCRDSetCount()) { cout << "ERROR : too many crd-sets!!!" << endl; exit(EXIT_FAILURE); }
#endif	// VERBOSE

			while (file.peek() != '!')
			{
				i32u n1; file >> n1;	// id number
				
				fGL xcrd; file >> xcrd;	// x-coordinate
				fGL ycrd; file >> ycrd;	// y-coordinate
				fGL zcrd; file >> zcrd;	// z-coordinate
				
if (n1 >= ar_vector.size()) { cout << "ERROR : invalid atom id number!!!" << endl; exit(EXIT_FAILURE); }

				atom * ar = ar_vector[n1];
				ar->SetCRD(crd_set_counter, xcrd, ycrd, zcrd);
				
				file.getline(buffer, sizeof(buffer));
			}
			
			crd_set_counter++;
		}
		else if (!strcmp(buffer, "!PartialCharges"))		// handle "!PartialCharges" section.
		{
			file.getline(buffer, sizeof(buffer));
			
#ifdef VERBOSE
cout << "found \"!PartialCharges\" section..." << endl;
#endif	// VERBOSE

			while (file.peek() != '!')
			{
				i32u n1; file >> n1;	// id number
				f64 n2; file >> n2;	// charge
				
if (n1 >= ar_vector.size()) { cout << "ERROR : invalid atom id number!!!" << endl; exit(EXIT_FAILURE); }

				atom * ar = ar_vector[n1];
				ar->charge = n2;
				
				file.getline(buffer, sizeof(buffer));
			}
		}
		else						// handle an unknown section...
		{
			file.getline(buffer, sizeof(buffer));
			
			while (file.peek() != '!')
			{
				file.getline(buffer, sizeof(buffer));
			}
		}
	}
	
	// change the current_setup and current_eng using the settings from file.
	// try to find a new setup class; if no success use the setup1_mm as a default.
	
	delete prj.current_setup;	// get rid of the temporary setup...
	prj.current_setup = NULL;	// ...and mark it invalid.
	
	if (!prj.current_setup && !strcmp(setup1_qm::static_GetClassName(), su_cn)) prj.current_setup = new setup1_qm(& prj);
	if (!prj.current_setup && !strcmp(setup1_mm::static_GetClassName(), su_cn)) prj.current_setup = new setup1_mm(& prj);
	if (!prj.current_setup && !strcmp(setup1_sf::static_GetClassName(), su_cn)) prj.current_setup = new setup1_sf(& prj, false);
	if (!prj.current_setup && !strcmp(setup2_qm_mm::static_GetClassName(), su_cn)) prj.current_setup = new setup2_qm_mm(& prj);
	if (!prj.current_setup && !strcmp(setup2_mm_sf::static_GetClassName(), su_cn)) prj.current_setup = new setup2_mm_sf(& prj, false);
	
	if (!prj.current_setup)
	{
		prj.WarningMessage("did not find a matching setup");
		prj.current_setup = new setup1_mm(& prj);
	}
	
	// now try to set a correct value for current_eng_index
	
	i32u index = 0;
	while (index < prj.GetCurrentSetup()->GetEngineCount())
	{
		i32u id = prj.GetCurrentSetup()->GetEngineIDNumber(index);
		if (id == (i32u) engid) break;
		
		index++;
	}
	
	if (index == prj.GetCurrentSetup()->GetEngineCount())
	{
		prj.WarningMessage("did not find a matching engid");
		index = 0;
	}
	
	prj.GetCurrentSetup()->SetCurrentEngIndex(index);
	
	// change the original locale back...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	setlocale(LC_NUMERIC, my_num_locale);
	my_num_locale = NULL;
	
	// ok, we are ready now!
	
	return true;
}

/*################################################################################################*/
/*################################################################################################*/
/*################################################################################################*/

void WriteGPR_v100(project & prj, ostream & file)
{
	if (my_num_locale != NULL) cout << "WARNING : my_num_locale != NULL" << endl;	// todo : change into a thread-safe lock???
	// store the current (numeric) locale into my_num_locale, and switch into "C" numeric locale...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	my_num_locale = setlocale(LC_NUMERIC, NULL);
	setlocale(LC_NUMERIC, "C");
	
	const int current_version_number = 100;		// version number multiplied by 100 -> int.
	
	file << "!Header ";
	file << prj.GetProjectFileNameExtension() << " ";
	file << current_version_number << endl;
	
	// the "!Header" section is now processed, and the actual file output begins!!!
	// the "!Header" section is now processed, and the actual file output begins!!!
	// the "!Header" section is now processed, and the actual file output begins!!!
	
	// write "!Info" section. write the number of coordinate sets,
	// which should match to the number of "!Coord"-sections later in the file.
	
	file << "!Info ";
	file << prj.GetCRDSetCount() << endl;

	// write "!Atoms" section. write the number of atoms,
	// which should match to the number of records in this section.
	
	file << "!Atoms ";
	file << prj.GetAtomCount() << endl;
	
	prj.UpdateIndex();		// this will update the "index"-records...
	
	iter_al it1 = prj.GetAtomsBegin();
	while (it1 != prj.GetAtomsEnd())
	{
		file << (* it1).index << " ";
		file << (* it1).el.GetAtomicNumber() << " ";
		
		file << endl;
		it1++;
	}
	
	// write "!Bonds" section. write the number of bonds,
	// which should match to the number of records in this section.
	
	file << "!Bonds ";
	file << prj.GetBondCount() << endl;
	
	iter_bl it2 = prj.GetBondsBegin();
	while (it2 != prj.GetBondsEnd())
	{
		file << (* it2).atmr[0]->index << " ";
		file << (* it2).atmr[1]->index << " ";
		
		file << (* it2).bt.GetSymbol1() << " ";
		
		file << endl;
		it2++;
	}
	
	// write the "!Coord" sections...
	
	for (i32u n1 = 0;n1 < prj.GetCRDSetCount();n1++)
	{
		file << "!Coord" << endl;
		
		it1 = prj.GetAtomsBegin();
		while (it1 != prj.GetAtomsEnd())
		{
			const fGL * cdata = (* it1).GetCRD(n1);
			
			file << (* it1).index << " ";
			
			file << cdata[0] << " ";
			file << cdata[1] << " ";
			file << cdata[2] << " ";
			
			file << endl;
			it1++;
		}
	}
	
	// write the !Charges section.
	
	file << "!Charges" << endl;
	
	it1 = prj.GetAtomsBegin();
	while (it1 != prj.GetAtomsEnd())
	{
		file << (* it1).index << " ";
		file << (* it1).charge;
		
		file << endl;
		it1++;
	}
	
	// write "!End" section.
	
	file << "!End" << endl;
	
	// change the original locale back...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	setlocale(LC_NUMERIC, my_num_locale);
	my_num_locale = NULL;
}

/*################################################################################################*/

// WHAT WE NEED TO ADD WHEN WE CHANGE THE FORMAT NEXT TIME:
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// [just add your ideas here]
//
// need to write the mol/chn/res/atm id numbers into files?!?!?!
// see model::UpdateGroups() for more info; most relates to SF models and Identify/Ribbon stuff...

void WriteGPR(project & prj, ostream & file)
{
prj.Message("PLEASE NOTE : This file format is experimental!\nIt is safer to use the stable v1.0 of this program.");
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (my_num_locale != NULL) cout << "WARNING : my_num_locale != NULL" << endl;	// todo : change into a thread-safe lock???
	// store the current (numeric) locale into my_num_locale, and switch into "C" numeric locale...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	my_num_locale = setlocale(LC_NUMERIC, NULL);
	setlocale(LC_NUMERIC, "C");
	
	const int current_version_number = 111;		// version number multiplied by 100 -> int.
	
	// write "!Header" section. this is for file-format definition purposes.
	
	file << "!Header "; file << "gpr ";
	file << current_version_number << endl;
	
	// write "!Info" section. write the number of coordinate sets,
	// which should match to the number of "!Coord"-sections later in the file.
	// also write the ClassName and current EngineID of the setup object that we use now.
// the above only saves the most primitive setup information; need to add !Setup section later???
// the above only saves the most primitive setup information; need to add !Setup section later???
// the above only saves the most primitive setup information; need to add !Setup section later???
	
	file << "!Info ";
	file << prj.GetCRDSetCount() << " ";
	file << prj.GetCurrentSetup()->GetClassName() << " ";
	file << prj.GetCurrentSetup()->GetEngineIDNumber(prj.GetCurrentSetup()->GetCurrEngIndex()) << endl;	// these values might change in future versions?!?!?!
	
	// write "!Atoms" section. write the number of atoms,
	// which should match to the number of records in this section.
	
	file << "!Atoms ";
	file << prj.GetAtomCount() << endl;
	
	prj.UpdateIndex();		// this will update the "index"-records...
	
	iter_al it1 = prj.GetAtomsBegin();
	while (it1 != prj.GetAtomsEnd())
	{
		file << (* it1).index << " ";			// unsigned ints.
		file << (* it1).el.GetAtomicNumber() << " ";	// signed ints; can be -1 for dummy atoms!!!
		file.setf(ios::showpos);
		file << (* it1).formal_charge << " ";		// signed ints.
		file.unsetf(ios::showpos);
		file << (* it1).flags << " ";			// unsigned ints; can be large numbers depending on the flags used!
		
		// what else should we print here ?!?!?!?
		
		file << endl;
		it1++;
	}
	
	// write "!Bonds" section. write the number of bonds,
	// which should match to the number of records in this section.
	
	file << "!Bonds ";
	file << prj.GetBondCount() << endl;
	
	iter_bl it2 = prj.GetBondsBegin();
	while (it2 != prj.GetBondsEnd())
	{
		file << (* it2).atmr[0]->index << " ";		// unsigned ints.
		file << (* it2).atmr[1]->index << " ";		// unsigned ints.
		file << (* it2).bt.GetSymbol1() << " ";		// single characters.
		
		// what else should we print here ?!?!?!?
		
		file << endl;
		it2++;
	}
	
	// write the "!Coord" sections...
// TODO : write visible-flag and set descriptions (if there are some), for example:
// 0 1 "1st coordinate set is this one, and it's visible"
// 1 1 "2nd set looks like this!, and it's also visible"
// 2 0 "this 3rd set is not visible"
	
	for (i32u n1 = 0;n1 < prj.GetCRDSetCount();n1++)
	{
		file << "!Coord" << endl;
		
		it1 = prj.GetAtomsBegin();
		while (it1 != prj.GetAtomsEnd())
		{
			const fGL * cdata = (* it1).GetCRD(n1);
			
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
	
	// write the !PartialCharges section.
	
	file << "!PartialCharges" << endl;
	
	it1 = prj.GetAtomsBegin();
	while (it1 != prj.GetAtomsEnd())
	{
		file << (* it1).index << " ";
		file.setf(ios::showpos);
		file << setprecision(15) << (* it1).charge;
		file.unsetf(ios::showpos);
		
		file << endl;
		it1++;
	}
	
	// write "!End" section.
	
	file << "!End" << endl;
	
	// change the original locale back...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	setlocale(LC_NUMERIC, my_num_locale);
	my_num_locale = NULL;
}

/*################################################################################################*/

// eof
