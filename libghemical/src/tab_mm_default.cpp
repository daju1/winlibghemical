// TAB_MM_DEFAULT.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "tab_mm_default.h"

#include "typerule.h"
#include "utility.h"

#include <fstream>
#include <strstream>
#include <iomanip>
using namespace std;

/*################################################################################################*/

default_tables * default_tables::instance = NULL;
singleton_cleaner<default_tables> deftab_cleaner(default_tables::GetInstance());

default_tables::default_tables(void)
{
	ostr = NULL;		// do not print output.
//	ostr = & cout;		// print output.
	
secondary_types_depth = NOT_DEFINED;	// DANGEROUS!!! keep this NOT_DEFINED in normal use!
use_strict_query = false;		// DANGEROUS!!! keep this false in normal use!
def_params_only = false;		// DANGEROUS!!! keep this false in normal use!
	
	ifstream file;
	file.unsetf(ios::dec | ios::oct | ios::hex);
	
	const char * fn;
	char buffer[1024];
	
/*##############################################*/
/*##############################################*/
	
	fn = "param_mm/default/atomtypes.txt";
	model::OpenLibDataFile(file, false, fn);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			default_at newat;
			file >> newat.atomtype[0];
			file >> newat.atomtype[1];
			
			file >> newat.vdw_R >> newat.vdw_E;
			file >> newat.formal_charge;
			file >> newat.flags;
			
			while (file.peek() != '(') file.get();
			newat.tr = new typerule(& file, ostr);
			
			while (file.get() != '\"');
			file.getline(buffer, sizeof(buffer), '\"');
			newat.description = new char[strlen(buffer) + 1];
			strcpy(newat.description, buffer);
			
			at_vector.push_back(newat);
		}
		
		file.getline(buffer, sizeof(buffer));
	}
	
	file.close();
	
	if (ostr != NULL) (* ostr) << "found " << at_vector.size() << " atomtypes." << endl;
	
/*##############################################*/
/*##############################################*/
	
	fn = "param_mm/default/parameters1.txt";
	model::OpenLibDataFile(file, false, fn);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			default_bs tmp; char bt[16];
			file >> tmp.atmtp[0] >> tmp.atmtp[1] >> bt;
			file >> tmp.opt >> tmp.fc >> tmp.ci;
			
		float fixme; file >> fixme;
			
			tmp.bndtp = bondtype(bt[0]);
			bs_vector.push_back(tmp);
		}
		
		file.getline(buffer, sizeof(buffer));
	}
	
	file.close();
	
	if (ostr != NULL) (* ostr) << "found " << bs_vector.size() << " bs-terms." << endl;
	
/*##############################################*/
/*##############################################*/
	
	fn = "param_mm/default/parameters2.txt";
	model::OpenLibDataFile(file, false, fn);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			default_ab tmp; char bt[16];
			file >> tmp.atmtp[0] >> tmp.atmtp[1] >> tmp.atmtp[2] >> bt;
			file >> tmp.opt >> tmp.fc;
			
			for (i32s n1 = 0;n1 < 2;n1++) tmp.bndtp[n1] = bondtype(bt[n1]);
			ab_vector.push_back(tmp);
		}
		
		file.getline(buffer, sizeof(buffer));
	}
	
	file.close();
	
	if (ostr != NULL) (* ostr) << "found " << ab_vector.size() << " ab-terms." << endl;
	
/*##############################################*/
/*##############################################*/
	
	fn = "param_mm/default/parameters3.txt";
	model::OpenLibDataFile(file, false, fn);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			default_tr tmp; char bt[16];
			file >> tmp.atmtp[0] >> tmp.atmtp[1] >> tmp.atmtp[2] >> tmp.atmtp[3] >> bt;
			file >> tmp.fc1 >> tmp.fc2 >> tmp.fc3;
			
			for (i32s n1 = 0;n1 < 3;n1++) tmp.bndtp[n1] = bondtype(bt[n1]);
			tr_vector.push_back(tmp);
		}
		
		file.getline(buffer, sizeof(buffer));
	}
	
	file.close();
	
	if (ostr != NULL) (* ostr) << "found " << tr_vector.size() << " tr-terms." << endl;
	
/*##############################################*/
/*##############################################*/
	
	fn = "param_mm/default/parameters4.txt";
	model::OpenLibDataFile(file, false, fn);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			default_op tmp; char bt[16];
			file >> tmp.atmtp[0] >> tmp.atmtp[1] >> tmp.atmtp[2] >> tmp.atmtp[3] >> bt;
			file >> tmp.opt >> tmp.fc;
			
			for (i32s n1 = 0;n1 < 3;n1++) tmp.bndtp[n1] = bondtype(bt[n1]);
			op_vector.push_back(tmp);
		}
		
		file.getline(buffer, sizeof(buffer));
	}
	
	file.close();
	
	if (ostr != NULL) (* ostr) << "found " << op_vector.size() << " op-terms." << endl;
	
/*##############################################*/
/*##############################################*/

}

default_tables::~default_tables(void)
{
	for (i32u n1 = 0;n1 < at_vector.size();n1++)
	{
		delete at_vector[n1].tr;
		delete[] at_vector[n1].description;
	}
}

default_tables * default_tables::GetInstance(void)
{
	if (instance != NULL) return instance;
	else return (instance = new default_tables());
}

void default_tables::PrintAllTypeRules(ostream & p1)
{
	for (i32u n1 = 0;n1 < at_vector.size();n1++)
	{
		p1 << (n1 + 1) << ": 0x" << hex << setw(4) << setfill('0') << at_vector[n1].atomtype << dec;
		p1 << " (" << (* at_vector[n1].tr) << ") \"" << at_vector[n1].description << "\"" << endl;
	}
	
	p1 << at_vector.size() << " entries." << endl;
}

i32u default_tables::UpdateTypes(setup * su)
{
	if (ostr != NULL) (* ostr) << "setting up atom types and formal charges..." << endl;
	
	i32u errors = 0;
	model * mdl = su->GetModel();
	
// determine the atomtypes for all atoms, not just MM atoms (might need to skip the virtual atoms of SF if eng2???).
// determine the atomtypes for all atoms, not just MM atoms (might need to skip the virtual atoms of SF if eng2???).
// determine the atomtypes for all atoms, not just MM atoms (might need to skip the virtual atoms of SF if eng2???).
	
	for (iter_al it1 = mdl->GetAtomsBegin();it1 != mdl->GetAtomsEnd();it1++)
	{
		if (ostr != NULL && !((* it1).index % 10)) (* ostr) << "*" << flush;
		
		i32u at_range[2];
		
		at_range[0] = 0;
		while (true)
		{
			if (at_range[0] == at_vector.size()) break;
			if ((at_vector[at_range[0]].atomtype[0] >> 8) == (* it1).el.GetAtomicNumber()) break;
			
			at_range[0]++;
		}
		
		at_range[1] = at_range[0];
		while (true)
		{
			if (at_range[1] == at_vector.size()) break;
			if ((at_vector[at_range[1]].atomtype[0] >> 8) != (* it1).el.GetAtomicNumber()) break;
			
			at_range[1]++;
		}
		
		i32s index = NOT_DEFINED;
		for (i32u n1 = at_range[0];n1 < at_range[1];n1++)
		{
			bool flag = at_vector[n1].tr->Check(mdl, & (* it1), 0);
			if (flag) index = n1;
			
			// above, the LAST matching type will be selected???
		}
		
		if (index != NOT_DEFINED)
		{
			if (secondary_types_depth == NOT_DEFINED)
			{
				(* it1).atmtp = at_vector[index].atomtype[0];
				(* it1).charge = at_vector[index].formal_charge;
			}
			else
			{
				if (ostr != NULL) (* ostr) << "using secondary_types_depth = " << secondary_types_depth << endl;
				
				if (secondary_types_depth == 0)
				{
					(* it1).atmtp = at_vector[index].atomtype[0];
					(* it1).charge = at_vector[index].formal_charge;
				}
				else
				{
					i32s tmptp = at_vector[index].atomtype[0];
					for (i32s n1 = 0;n1 < secondary_types_depth;n1++)
					{
						const default_at * at = GetAtomType(tmptp);
						if (!at) { cout << "BUG: GetAtomType() failed!!! (sec types)" << endl; exit(EXIT_FAILURE); }
						tmptp = at->atomtype[1];	// loop the secondary types...
					}
					
					const default_at * at = GetAtomType(tmptp);
					if (!at) { cout << "BUG: GetAtomType() failed!!! (sec types)" << endl; exit(EXIT_FAILURE); }
					
					(* it1).atmtp = at->atomtype[0];
					(* it1).charge = at->formal_charge;
				}
			}
		}
		else
		{
			char mbuff1[256];
			ostrstream str1(mbuff1, sizeof(mbuff1));
			str1 << "WARNING : could not determine atomtype (atom index = " << (* it1).index << ")." << endl << ends;
			mdl->PrintToLog(mbuff1);
			
			(* it1).atmtp = NOT_DEFINED;
			(* it1).charge = 0.0;
			
			(* it1).flags |= ATOMFLAG_SELECTED;
			errors++;
		}
	}
	
	if (ostr != NULL) (* ostr) << endl;
	
	// exceptions...
	// exceptions...
	// exceptions...
	
	if (dynamic_cast<setup1_mm *>(su)->exceptions)
	{
		if (ostr != NULL) (* ostr) << "setting up atom type exceptions..." << endl;
		errors += e_UpdateTypes(su);
	}
	
	return errors;
}

i32u default_tables::UpdateCharges(setup * su)
{
	if (ostr != NULL) (* ostr) << "setting up partial charges..." << endl;
	
	i32u errors = 0;
//	atom ** atmtab = su->GetMMAtoms();
	bond ** bndtab = su->GetMMBonds();
	
	for (i32s n1 = 0;n1 < su->GetMMBondCount();n1++)
	{
		default_bs_query query; query.strict = false;
		query.atmtp[0] = bndtab[n1]->atmr[0]->atmtp;
		query.atmtp[1] = bndtab[n1]->atmr[1]->atmtp;
		query.bndtp = bndtab[n1]->bt.GetValue();
		
		DoParamSearch(& query);
		if (query.index == NOT_DEFINED) errors++;
		
		f64 delta = query.ci;			// here we also determine...
		if (query.dir) delta = -delta;		// ...the effect of direction!!!
		
		bndtab[n1]->atmr[0]->charge -= delta;
		bndtab[n1]->atmr[1]->charge += delta;
	}
	
	// exceptions...
	// exceptions...
	// exceptions...
	
	if (dynamic_cast<setup1_mm *>(su)->exceptions)
	{
		if (ostr != NULL) (* ostr) << "setting up AMBER partial charges..." << endl;
		errors += e_UpdateCharges(su);
	}
	
	return errors;
}

const default_at * default_tables::GetAtomType(i32s p1)
{
	i32u index = 0;
	while (index < at_vector.size())
	{
		if (at_vector[index].atomtype[0] == p1) return (& at_vector[index]);
		else index++;
	}
	
	// could not find the requested type -> return a NULL pointer instead...
	
	return NULL;
}

void default_tables::DoParamSearch(default_bs_query * query)
{
	for (i32u n1 = 0;n1 < bs_vector.size();n1++)
	{
		if (bs_vector[n1].bndtp.GetValue() != query->bndtp.GetValue()) continue;
		
		bool flag = false; i32s dir;
		for (dir = 0;dir < 2;dir++)
		{
			i32s index[2];
			index[0] = (!dir ? 0 : 1);
			index[1] = (!dir ? 1 : 0);
			
			bool test1 = (bs_vector[n1].atmtp[0] == query->atmtp[index[0]]);
			bool test2 = (bs_vector[n1].atmtp[1] == query->atmtp[index[1]]);
			
			if (test1 && test2) flag = true;
			
			if (flag) break;
		}
		
		if (flag)
		{
			query->index = n1;
			query->dir = dir;
			
			query->opt = bs_vector[n1].opt;
			query->fc = bs_vector[n1].fc;
			
			query->ci = bs_vector[n1].ci;
			// fixme
			
			return;		// success, return the parameters...
		}
	}
	
// recursive search?!?!?! DO IT BY CALLING AGAIN USING SECONDARY TYPES!!! NOT RECURSIVELY!!!
// recursive search?!?!?! DO IT BY CALLING AGAIN USING SECONDARY TYPES!!! NOT RECURSIVELY!!!
// recursive search?!?!?! DO IT BY CALLING AGAIN USING SECONDARY TYPES!!! NOT RECURSIVELY!!!
	
	if (ostr != NULL)
	{
		(* ostr) << "unknown bs: ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << query->atmtp[0] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << query->atmtp[1] << dec << " ";
		(* ostr) << query->bndtp.GetValue() << " ";
		(* ostr) << endl;
	}
	
	// the search failed, return default parameters...
	
	query->index = NOT_DEFINED;
	query->dir = false;
	
	query->opt = 0.140;
	query->fc = 60.0e+03;
	
	query->ci = 0.0;
	// fixme
}

void default_tables::DoParamSearch(default_ab_query * query)
{
if (use_strict_query) query->strict = true;
if (def_params_only) query->atmtp[0] = query->atmtp[2] = 0xffff;
if (!use_strict_query && def_params_only) { cout << "bad flags set!" << endl; exit(EXIT_FAILURE); }
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
	for (i32u n1 = 0;n1 < ab_vector.size();n1++)
	{
		if (ab_vector[n1].atmtp[1] != query->atmtp[1]) continue;
		
		bool flag = false; i32s dir;
		for (dir = 0;dir < 2;dir++)
		{
			i32s btind[2];
			btind[0] = (!dir ? 0 : 1);
			btind[1] = (!dir ? 1 : 0);
			
			bool bttest1 = (ab_vector[n1].bndtp[0].GetValue() != query->bndtp[btind[0]].GetValue());
			bool bttest2 = (ab_vector[n1].bndtp[1].GetValue() != query->bndtp[btind[1]].GetValue());
			if (bttest1 || bttest2) continue;	// bond type mismatch detected...
			
			i32s index[2];
			index[0] = (!dir ? 0 : 2);
			index[1] = (!dir ? 2 : 0);
			
			bool test1 = (ab_vector[n1].atmtp[0] == query->atmtp[index[0]]);
			bool test2 = (ab_vector[n1].atmtp[2] == query->atmtp[index[1]]);
			
			if (test1 && test2) flag = true;
			
			if (query->strict == false)
			{
				bool wc1 = (ab_vector[n1].atmtp[0] == 0xffff);
				bool wc2 = (ab_vector[n1].atmtp[2] == 0xffff);
				
				if (wc1 && test2) flag = true;
				if (test1 && wc2) flag = true;
				if (wc1 && wc2) flag = true;
			}
			
			if (flag) break;
		}
		
		if (flag)
		{
			query->index = n1;
			query->dir = dir;
			
			query->opt = ab_vector[n1].opt;
			query->fc = ab_vector[n1].fc;
			
			return;		// success, return the parameters...
		}
	}

// recursive search?!?!?! DO IT BY CALLING AGAIN USING SECONDARY TYPES!!! NOT RECURSIVELY!!!
// recursive search?!?!?! DO IT BY CALLING AGAIN USING SECONDARY TYPES!!! NOT RECURSIVELY!!!
// recursive search?!?!?! DO IT BY CALLING AGAIN USING SECONDARY TYPES!!! NOT RECURSIVELY!!!
	
	if (ostr != NULL)
	{
		(* ostr) << "unknown ab: " << hex;
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << query->atmtp[0] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << query->atmtp[1] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << query->atmtp[2] << dec << " ";
		(* ostr) << query->bndtp[0].GetValue() << " ";
		(* ostr) << query->bndtp[1].GetValue() << " ";
		(* ostr) << endl;
	}

	// the search failed, return default parameters...
if (use_strict_query) { cout << "search failed with use_strict_query set!" << endl; exit(EXIT_FAILURE); }
	
	query->index = NOT_DEFINED;
	query->dir = false;
	
	query->opt = 2.10;
	query->fc = 250.0;
}

void default_tables::DoParamSearch(default_tr_query * query)
{
if (use_strict_query) query->strict = true;
if (def_params_only) query->atmtp[0] = query->atmtp[3] = 0xffff;
if (!use_strict_query && def_params_only) { cout << "bad flags set!" << endl; exit(EXIT_FAILURE); }
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
	for (i32u n1 = 0;n1 < tr_vector.size();n1++)
	{
		if (tr_vector[n1].bndtp[1].GetValue() != query->bndtp[1].GetValue()) continue;

		bool flag = false; i32s dir;
		for (dir = 0;dir < 2;dir++)
		{
			i32s btind[2];
			btind[0] = (!dir ? 0 : 2);
			btind[1] = (!dir ? 2 : 0);
			
			bool bttest1 = (tr_vector[n1].bndtp[0].GetValue() != query->bndtp[btind[0]].GetValue());
			bool bttest2 = (tr_vector[n1].bndtp[2].GetValue() != query->bndtp[btind[1]].GetValue());
			if (bttest1 || bttest2) continue;	// bond type mismatch detected...
			
			i32s index[4];
			index[0] = (!dir ? 0 : 3);
			index[1] = (!dir ? 1 : 2);
			index[2] = (!dir ? 2 : 1);
			index[3] = (!dir ? 3 : 0);
			
			bool test1 = (tr_vector[n1].atmtp[0] == query->atmtp[index[0]]);
			bool test2 = (tr_vector[n1].atmtp[1] == query->atmtp[index[1]]);
			bool test3 = (tr_vector[n1].atmtp[2] == query->atmtp[index[2]]);
			bool test4 = (tr_vector[n1].atmtp[3] == query->atmtp[index[3]]);
			
			if (test1 && test2 && test3 && test4) flag = true;
			
			if (query->strict == false)
			{
				bool wc1 = (tr_vector[n1].atmtp[0] == 0xffff);
				bool wc2 = (tr_vector[n1].atmtp[3] == 0xffff);
				
				if (wc1 && test2 && test3 && test4) flag = true;
				if (test1 && test2 && test3 && wc2) flag = true;
				if (wc1 && test2 && test3 && wc2) flag = true;
			}
			
			if (flag) break;
		}
		
		if (flag)
		{
			query->index = n1;
			query->dir = dir;
			
			query->fc1 = tr_vector[n1].fc1;
			query->fc2 = tr_vector[n1].fc2;
			query->fc3 = tr_vector[n1].fc3;
			
			return;		// success, return the parameters...
		}
	}

// recursive search?!?!?! DO IT BY CALLING AGAIN USING SECONDARY TYPES!!! NOT RECURSIVELY!!!
// recursive search?!?!?! DO IT BY CALLING AGAIN USING SECONDARY TYPES!!! NOT RECURSIVELY!!!
// recursive search?!?!?! DO IT BY CALLING AGAIN USING SECONDARY TYPES!!! NOT RECURSIVELY!!!
	
	if (ostr != NULL)
	{
		(* ostr) << "unknown tr: " << hex;
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << query->atmtp[0] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << query->atmtp[1] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << query->atmtp[2] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << query->atmtp[3] << dec << " ";
		(* ostr) << query->bndtp[0].GetValue() << " ";
		(* ostr) << query->bndtp[1].GetValue() << " ";
		(* ostr) << query->bndtp[2].GetValue() << " ";
		(* ostr) << endl;
	}
	
	// the search failed, return default parameters...
if (use_strict_query) { cout << "search failed with use_strict_query set!" << endl; exit(EXIT_FAILURE); }
	
	query->index = NOT_DEFINED;
	query->dir = false;
	
	query->fc1 = 0.0;
	query->fc2 = 0.0;
	query->fc3 = 0.0;
}

void default_tables::DoParamSearch(default_op_query * query)
{
if (use_strict_query) query->strict = true;
if (def_params_only) query->atmtp[0] = query->atmtp[2] = 0xffff;
if (!use_strict_query && def_params_only) { cout << "bad flags set!" << endl; exit(EXIT_FAILURE); }
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
	for (i32u n1 = 0;n1 < op_vector.size();n1++)
	{
		// the atomtypes are defined in the following way:
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		//
		//   3  	the idea is to measure how much atom 3
		//   |  	is bend from the plane defined by atoms
		//   1  	0, 1 and 2.
		//  / \ 
		// 0   2	order of atoms 0 and 2 is not relevant,
		//		but atoms 1 and 3 must match exactly.
		
		if (op_vector[n1].atmtp[1] != query->atmtp[1]) continue;
		if (op_vector[n1].atmtp[3] != query->atmtp[3]) continue;
		if (op_vector[n1].bndtp[2].GetValue() != query->bndtp[2].GetValue()) continue;
		
		// proper bondtype checking not yet implemented....
		// it will be basically similar to the above, but must be moved into the dir-loop!!!
		// it will be basically similar to the above, but must be moved into the dir-loop!!!
		// it will be basically similar to the above, but must be moved into the dir-loop!!!
		
		bool flag = false; i32s dir;
		for (dir = 0;dir < 2;dir++)
		{
			i32s btind[2];
			btind[0] = (!dir ? 0 : 1);
			btind[1] = (!dir ? 1 : 0);
			
			bool bttest1 = (op_vector[n1].bndtp[0].GetValue() != query->bndtp[btind[0]].GetValue());
			bool bttest2 = (op_vector[n1].bndtp[1].GetValue() != query->bndtp[btind[1]].GetValue());
			if (bttest1 || bttest2) continue;	// bond type mismatch detected...
			
			i32s index[2];
			index[0] = (!dir ? 0 : 2);
			index[1] = (!dir ? 2 : 0);
			
			bool test1 = (op_vector[n1].atmtp[0] == query->atmtp[index[0]]);
			bool test2 = (op_vector[n1].atmtp[2] == query->atmtp[index[1]]);
			
			if (test1 && test2) flag = true;
			
			if (query->strict == false)
			{
				bool wc1 = (op_vector[n1].atmtp[0] == 0xffff);
				bool wc2 = (op_vector[n1].atmtp[2] == 0xffff);
				
				if (wc1 && test2) flag = true;
				if (test1 && wc2) flag = true;
				if (wc1 && wc2) flag = true;
			}
			
			if (flag) break;
		}
		
		if (flag)
		{
			query->index = n1;
			
			query->opt = op_vector[n1].opt;
			query->fc = op_vector[n1].fc;
			
			return;		// success, return the parameters...
		}
	}

// recursive search?!?!?! DO IT BY CALLING AGAIN USING SECONDARY TYPES!!! NOT RECURSIVELY!!!
// recursive search?!?!?! DO IT BY CALLING AGAIN USING SECONDARY TYPES!!! NOT RECURSIVELY!!!
// recursive search?!?!?! DO IT BY CALLING AGAIN USING SECONDARY TYPES!!! NOT RECURSIVELY!!!
	
	if (ostr != NULL)
	{
		(* ostr) << "unknown op: " << hex;
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << query->atmtp[0] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << query->atmtp[1] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << query->atmtp[2] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << query->atmtp[3] << dec << " ";
		(* ostr) << query->bndtp[0].GetValue() << " ";
		(* ostr) << query->bndtp[1].GetValue() << " ";
		(* ostr) << query->bndtp[2].GetValue() << " ";
		(* ostr) << endl;
	}
	
	// the search failed, return default parameters...
if (use_strict_query) { cout << "search failed with use_strict_query set!" << endl; exit(EXIT_FAILURE); }
	
	query->index = NOT_DEFINED;
	
	query->opt = 0.0;
	query->fc = 0.0;
}

/*################################################################################################*/
/*################################################################################################*/
/*################################################################################################*/

// in AMBER parameter files, amino acids have constant names but nucleic acids have variable names...

const e_atom_name default_tables::res_name_tab[RES_NAME_TAB_SIZE] =
{
	{ ATE_type_AA | ATE_resn_aa_ALA, "ALA" },		// all_amino02.in
	{ ATE_type_AA | ATE_resn_aa_ARG, "ARG" },
	{ ATE_type_AA | ATE_resn_aa_ASN, "ASN" },
	{ ATE_type_AA | ATE_resn_aa_ASP, "ASP" },
	{ ATE_type_AA | ATE_resn_aa_ASH, "ASH" },
	{ ATE_type_AA | ATE_resn_aa_CYS, "CYS" },
	{ ATE_type_AA | ATE_resn_aa_CYM, "CYM" },
	{ ATE_type_AA | ATE_resn_aa_CYX, "CYX" },
	{ ATE_type_AA | ATE_resn_aa_GLN, "GLN" },
	{ ATE_type_AA | ATE_resn_aa_GLU, "GLU" },
	{ ATE_type_AA | ATE_resn_aa_GLH, "GLH" },
	{ ATE_type_AA | ATE_resn_aa_GLY, "GLY" },
	{ ATE_type_AA | ATE_resn_aa_HID, "HID" },
	{ ATE_type_AA | ATE_resn_aa_HIE, "HIE" },
	{ ATE_type_AA | ATE_resn_aa_HIP, "HIP" },
	{ ATE_type_AA | ATE_resn_aa_ILE, "ILE" },
	{ ATE_type_AA | ATE_resn_aa_LEU, "LEU" },
	{ ATE_type_AA | ATE_resn_aa_LYS, "LYS" },
	{ ATE_type_AA | ATE_resn_aa_LYN, "LYN" },
	{ ATE_type_AA | ATE_resn_aa_MET, "MET" },
	{ ATE_type_AA | ATE_resn_aa_PHE, "PHE" },
	{ ATE_type_AA | ATE_resn_aa_PRO, "PRO" },
	{ ATE_type_AA | ATE_resn_aa_SER, "SER" },
	{ ATE_type_AA | ATE_resn_aa_THR, "THR" },
	{ ATE_type_AA | ATE_resn_aa_TRP, "TRP" },
	{ ATE_type_AA | ATE_resn_aa_TYR, "TYR" },
	{ ATE_type_AA | ATE_resn_aa_VAL, "VAL" },
	
	{ ATE_type_AANT | ATE_resn_aa_ALA, "ALA" },		// all_aminont02.in
	{ ATE_type_AANT | ATE_resn_aa_ARG, "ARG" },
	{ ATE_type_AANT | ATE_resn_aa_ASN, "ASN" },
	{ ATE_type_AANT | ATE_resn_aa_ASP, "ASP" },
	{ ATE_type_AANT | ATE_resn_aa_ASH, "ASH" },
	{ ATE_type_AANT | ATE_resn_aa_CYS, "CYS" },
	{ ATE_type_AANT | ATE_resn_aa_CYM, "CYM" },
	{ ATE_type_AANT | ATE_resn_aa_CYX, "CYX" },
	{ ATE_type_AANT | ATE_resn_aa_GLN, "GLN" },
	{ ATE_type_AANT | ATE_resn_aa_GLU, "GLU" },
	{ ATE_type_AANT | ATE_resn_aa_GLH, "GLH" },
	{ ATE_type_AANT | ATE_resn_aa_GLY, "GLY" },
	{ ATE_type_AANT | ATE_resn_aa_HID, "HID" },
	{ ATE_type_AANT | ATE_resn_aa_HIE, "HIE" },
	{ ATE_type_AANT | ATE_resn_aa_HIP, "HIP" },
	{ ATE_type_AANT | ATE_resn_aa_ILE, "ILE" },
	{ ATE_type_AANT | ATE_resn_aa_LEU, "LEU" },
	{ ATE_type_AANT | ATE_resn_aa_LYS, "LYS" },
	{ ATE_type_AANT | ATE_resn_aa_LYN, "LYN" },
	{ ATE_type_AANT | ATE_resn_aa_MET, "MET" },
	{ ATE_type_AANT | ATE_resn_aa_PHE, "PHE" },
	{ ATE_type_AANT | ATE_resn_aa_PRO, "PRO" },
	{ ATE_type_AANT | ATE_resn_aa_SER, "SER" },
	{ ATE_type_AANT | ATE_resn_aa_THR, "THR" },
	{ ATE_type_AANT | ATE_resn_aa_TRP, "TRP" },
	{ ATE_type_AANT | ATE_resn_aa_TYR, "TYR" },
	{ ATE_type_AANT | ATE_resn_aa_VAL, "VAL" },
	
	{ ATE_type_AACT | ATE_resn_aa_ALA, "ALA" },		// all_aminoct02.in
	{ ATE_type_AACT | ATE_resn_aa_ARG, "ARG" },
	{ ATE_type_AACT | ATE_resn_aa_ASN, "ASN" },
	{ ATE_type_AACT | ATE_resn_aa_ASP, "ASP" },
	{ ATE_type_AACT | ATE_resn_aa_ASH, "ASH" },
	{ ATE_type_AACT | ATE_resn_aa_CYS, "CYS" },
	{ ATE_type_AACT | ATE_resn_aa_CYM, "CYM" },
	{ ATE_type_AACT | ATE_resn_aa_CYX, "CYX" },
	{ ATE_type_AACT | ATE_resn_aa_GLN, "GLN" },
	{ ATE_type_AACT | ATE_resn_aa_GLU, "GLU" },
	{ ATE_type_AACT | ATE_resn_aa_GLH, "GLH" },
	{ ATE_type_AACT | ATE_resn_aa_GLY, "GLY" },
	{ ATE_type_AACT | ATE_resn_aa_HID, "HID" },
	{ ATE_type_AACT | ATE_resn_aa_HIE, "HIE" },
	{ ATE_type_AACT | ATE_resn_aa_HIP, "HIP" },
	{ ATE_type_AACT | ATE_resn_aa_ILE, "ILE" },
	{ ATE_type_AACT | ATE_resn_aa_LEU, "LEU" },
	{ ATE_type_AACT | ATE_resn_aa_LYS, "LYS" },
	{ ATE_type_AACT | ATE_resn_aa_LYN, "LYN" },
	{ ATE_type_AACT | ATE_resn_aa_MET, "MET" },
	{ ATE_type_AACT | ATE_resn_aa_PHE, "PHE" },
	{ ATE_type_AACT | ATE_resn_aa_PRO, "PRO" },
	{ ATE_type_AACT | ATE_resn_aa_SER, "SER" },
	{ ATE_type_AACT | ATE_resn_aa_THR, "THR" },
	{ ATE_type_AACT | ATE_resn_aa_TRP, "TRP" },
	{ ATE_type_AACT | ATE_resn_aa_TYR, "TYR" },
	{ ATE_type_AACT | ATE_resn_aa_VAL, "VAL" },
	
	{ ATE_type_NA | ATE_resn_na_DA, "DA" },			// all_nuc02.in
	{ ATE_type_NA | ATE_resn_na_DC, "DC" },
	{ ATE_type_NA | ATE_resn_na_DG, "DG" },
	{ ATE_type_NA | ATE_resn_na_DT, "DT" },
	{ ATE_type_NA | ATE_resn_na_RA, "RA" },
	{ ATE_type_NA | ATE_resn_na_RC, "RC" },
	{ ATE_type_NA | ATE_resn_na_RG, "RG" },
	{ ATE_type_NA | ATE_resn_na_RU, "RU" },
	
	{ ATE_type_NA5T | ATE_resn_na_DA, "DA5" },		// all_nuc02.in
	{ ATE_type_NA5T | ATE_resn_na_DC, "DC5" },
	{ ATE_type_NA5T | ATE_resn_na_DG, "DG5" },
	{ ATE_type_NA5T | ATE_resn_na_DT, "DT5" },
	{ ATE_type_NA5T | ATE_resn_na_RA, "RA5" },
	{ ATE_type_NA5T | ATE_resn_na_RC, "RC5" },
	{ ATE_type_NA5T | ATE_resn_na_RG, "RG5" },
	{ ATE_type_NA5T | ATE_resn_na_RU, "RU5" },
	
	{ ATE_type_NA3T | ATE_resn_na_DA, "DA3" },		// all_nuc02.in
	{ ATE_type_NA3T | ATE_resn_na_DC, "DC3" },
	{ ATE_type_NA3T | ATE_resn_na_DG, "DG3" },
	{ ATE_type_NA3T | ATE_resn_na_DT, "DT3" },
	{ ATE_type_NA3T | ATE_resn_na_RA, "RA3" },
	{ ATE_type_NA3T | ATE_resn_na_RC, "RC3" },
	{ ATE_type_NA3T | ATE_resn_na_RG, "RG3" },
	{ ATE_type_NA3T | ATE_resn_na_RU, "RU3" }
};

// amino/nucleic names are from AMBER parameter files, solvent/ion names are not.

const e_atom_name default_tables::atm_name_tab[ATM_NAME_TAB_SIZE] =
{
	{ ATE_atmn_xt_Hh2o, "H(H2O)" },		// the corresponding type string is "Hw".
	{ ATE_atmn_xt_Oh2o, "O(H2O)" },		// the corresponding type string is "Ow".
	{ ATE_atmn_xt_Na, "Na+" },		// ???
	{ ATE_atmn_xt_Cl, "Cl-" },		// ???
	
	{ ATE_atmn_aa_H, "H" },			// all_amino02.in all_aminont02.in all_aminoct02.in
	{ ATE_atmn_aa_H1, "H1" },
	{ ATE_atmn_aa_H2, "H2" },
	{ ATE_atmn_aa_H3, "H3" },
	{ ATE_atmn_aa_HA, "HA" },
	{ ATE_atmn_aa_HA2, "HA2" },
	{ ATE_atmn_aa_HA3, "HA3" },
	{ ATE_atmn_aa_HB, "HB" },
	{ ATE_atmn_aa_HB1, "HB1" },
	{ ATE_atmn_aa_HB2, "HB2" },
	{ ATE_atmn_aa_HB3, "HB3" },
	{ ATE_atmn_aa_HG, "HG" },
	{ ATE_atmn_aa_HG1, "HG1" },
	{ ATE_atmn_aa_HG11, "HG11" },
	{ ATE_atmn_aa_HG12, "HG12" },
	{ ATE_atmn_aa_HG13, "HG13" },
	{ ATE_atmn_aa_HG2, "HG2" },
	{ ATE_atmn_aa_HG21, "HG21" },
	{ ATE_atmn_aa_HG22, "HG22" },
	{ ATE_atmn_aa_HG23, "HG23" },
	{ ATE_atmn_aa_HG3, "HG3" },
	{ ATE_atmn_aa_HD1, "HD1" },
	{ ATE_atmn_aa_HD11, "HD11" },
	{ ATE_atmn_aa_HD12, "HD12" },
	{ ATE_atmn_aa_HD13, "HD13" },
	{ ATE_atmn_aa_HD2, "HD2" },
	{ ATE_atmn_aa_HD21, "HD21" },
	{ ATE_atmn_aa_HD22, "HD22" },
	{ ATE_atmn_aa_HD23, "HD23" },
	{ ATE_atmn_aa_HD3, "HD3" },
	{ ATE_atmn_aa_HE, "HE" },
	{ ATE_atmn_aa_HE1, "HE1" },
	{ ATE_atmn_aa_HE2, "HE2" },
	{ ATE_atmn_aa_HE21, "HE21" },
	{ ATE_atmn_aa_HE22, "HE22" },
	{ ATE_atmn_aa_HE3, "HE3" },
	{ ATE_atmn_aa_HZ, "HZ" },
	{ ATE_atmn_aa_HZ1, "HZ1" },
	{ ATE_atmn_aa_HZ2, "HZ2" },
	{ ATE_atmn_aa_HZ3, "HZ3" },
	{ ATE_atmn_aa_HH, "HH" },
	{ ATE_atmn_aa_HH11, "HH11" },
	{ ATE_atmn_aa_HH12, "HH12" },
	{ ATE_atmn_aa_HH2, "HH2" },
	{ ATE_atmn_aa_HH21, "HH21" },
	{ ATE_atmn_aa_HH22, "HH22" },
	{ ATE_atmn_aa_C, "C" },
	{ ATE_atmn_aa_CA, "CA" },
	{ ATE_atmn_aa_CB, "CB" },
	{ ATE_atmn_aa_CG, "CG" },
	{ ATE_atmn_aa_CG1, "CG1" },
	{ ATE_atmn_aa_CG2, "CG2" },
	{ ATE_atmn_aa_CD, "CD" },
	{ ATE_atmn_aa_CD1, "CD1" },
	{ ATE_atmn_aa_CD2, "CD2" },
	{ ATE_atmn_aa_CE, "CE" },
	{ ATE_atmn_aa_CE1, "CE1" },
	{ ATE_atmn_aa_CE2, "CE2" },
	{ ATE_atmn_aa_CE3, "CE3" },
	{ ATE_atmn_aa_CZ, "CZ" },
	{ ATE_atmn_aa_CZ2, "CZ2" },
	{ ATE_atmn_aa_CZ3, "CZ3" },
	{ ATE_atmn_aa_CH2, "CH2" },
	{ ATE_atmn_aa_N, "N" },
	{ ATE_atmn_aa_ND1, "ND1" },
	{ ATE_atmn_aa_ND2, "ND2" },
	{ ATE_atmn_aa_NE, "NE" },
	{ ATE_atmn_aa_NE1, "NE1" },
	{ ATE_atmn_aa_NE2, "NE2" },
	{ ATE_atmn_aa_NZ, "NZ" },
	{ ATE_atmn_aa_NH1, "NH1" },
	{ ATE_atmn_aa_NH2, "NH2" },
	{ ATE_atmn_aa_O, "O" },
	{ ATE_atmn_aa_OG, "OG" },
	{ ATE_atmn_aa_OG1, "OG1" },
	{ ATE_atmn_aa_OD1, "OD1" },
	{ ATE_atmn_aa_OD2, "OD2" },
	{ ATE_atmn_aa_OE1, "OE1" },
	{ ATE_atmn_aa_OE2, "OE2" },
	{ ATE_atmn_aa_OH, "OH" },
	{ ATE_atmn_aa_OXT, "OXT" },
	{ ATE_atmn_aa_SG, "SG" },
	{ ATE_atmn_aa_SD, "SD" },
	
	{ ATE_atmn_na_H1, "H1" },		// all_nuc02.in
	{ ATE_atmn_na_H1p, "H1'" },
	{ ATE_atmn_na_H2, "H2" },
	{ ATE_atmn_na_H21, "H21" },
	{ ATE_atmn_na_H22, "H22" },
	{ ATE_atmn_na_H2p1, "H2'1" },
	{ ATE_atmn_na_H2p2, "H2'2" },
	{ ATE_atmn_na_H3, "H3" },
	{ ATE_atmn_na_H3T, "H3T" },
	{ ATE_atmn_na_H3p, "H3'" },
	{ ATE_atmn_na_H41, "H41" },
	{ ATE_atmn_na_H42, "H42" },
	{ ATE_atmn_na_H4p, "H4'" },
	{ ATE_atmn_na_H5, "H5" },
	{ ATE_atmn_na_H5T, "H5T" },
	{ ATE_atmn_na_H5p1, "H5'1" },
	{ ATE_atmn_na_H5p2, "H5'2" },
	{ ATE_atmn_na_H6, "H6" },
	{ ATE_atmn_na_H61, "H61" },
	{ ATE_atmn_na_H62, "H62" },
	{ ATE_atmn_na_H71, "H71" },
	{ ATE_atmn_na_H72, "H72" },
	{ ATE_atmn_na_H73, "H73" },
	{ ATE_atmn_na_H8, "H8" },
	{ ATE_atmn_na_HOp2, "HO'2" },
	{ ATE_atmn_na_C1p, "C1'" },
	{ ATE_atmn_na_C2, "C2" },
	{ ATE_atmn_na_C2p, "C2'" },
	{ ATE_atmn_na_C3p, "C3'" },
	{ ATE_atmn_na_C4, "C4" },
	{ ATE_atmn_na_C4p, "C4'" },
	{ ATE_atmn_na_C5, "C5" },
	{ ATE_atmn_na_C5p, "C5'" },
	{ ATE_atmn_na_C6, "C6" },
	{ ATE_atmn_na_C7, "C7" },
	{ ATE_atmn_na_C8, "C8" },
	{ ATE_atmn_na_N1, "N1" },
	{ ATE_atmn_na_N2, "N2" },
	{ ATE_atmn_na_N3, "N3" },
	{ ATE_atmn_na_N4, "N4" },
	{ ATE_atmn_na_N6, "N6" },
	{ ATE_atmn_na_N7, "N7" },
	{ ATE_atmn_na_N9, "N9" },
	{ ATE_atmn_na_O1P, "O1P" },
	{ ATE_atmn_na_O2, "O2" },
	{ ATE_atmn_na_O2P, "O2P" },
	{ ATE_atmn_na_O2p, "O2'" },
	{ ATE_atmn_na_O3p, "O3'" },
	{ ATE_atmn_na_O4, "O4" },
	{ ATE_atmn_na_O4p, "O4'" },
	{ ATE_atmn_na_O5p, "O5'" },
	{ ATE_atmn_na_O6, "O6" },
	{ ATE_atmn_na_P, "P" }
};

atom * default_tables::e_UT_FindAtom(iter_al * res_rng, i32s id)
{
	iter_al it1 = res_rng[0];
	while (it1 != res_rng[1] && ((* it1).builder_res_id & 0xFF) != id) it1++;
	if (it1 == res_rng[1]) return NULL; else return & (* it1);
}

void default_tables::e_UT_FindHydrogens(iter_al * res_rng, i32s id, vector<atom *> & htab)
{
	for (iter_al it1 = res_rng[0];it1 != res_rng[1];it1++)
	{
		if ((* it1).el.GetAtomicNumber() != 1) continue;
		
		bool flag = false;
		for (iter_cl it2 = (* it1).cr_list.begin();it2 != (* it1).cr_list.end();it2++)
		{
			if (((* it2).atmr->builder_res_id & 0xFF) != id) continue;
			else { flag = true; break; }
		}
		
		if (flag) htab.push_back(& (* it1));
	}
}

i32u default_tables::e_UpdateTypes(setup * su)
{
	i32u errors = 0;
	model * mdl = su->GetModel();
	
	// first just clean all atmtp_E settings...
	// first just clean all atmtp_E settings...
	// first just clean all atmtp_E settings...
	
	for (iter_al it1 = mdl->GetAtomsBegin();it1 != mdl->GetAtomsEnd();it1++)
	{
		(* it1).atmtp_E = NOT_DEFINED;
	}
	
	// check H2O (and other solvents/ions?), by molecules...
	// check H2O (and other solvents/ions?), by molecules...
	// check H2O (and other solvents/ions?), by molecules...
	
	if (!mdl->IsGroupsClean()) mdl->UpdateGroups();
	if (!mdl->IsGroupsSorted()) mdl->SortGroups(true);
	
	for (i32s n1 = 0;n1 < mdl->GetMoleculeCount();n1++)
	{
		iter_al r_mol[2]; mdl->GetRange(0, n1, r_mol);
		
		i32s atom_count = 0;
		for (iter_al it1 = r_mol[0];it1 != r_mol[1];it1++) atom_count++;
		
		// test for H2O... IGNORE BONDS!!!
		// test for H2O... IGNORE BONDS!!!
		// test for H2O... IGNORE BONDS!!!
		
		if (atom_count == 3)
		{
			i32s H_count = 0; i32s O_count = 0;
			for (iter_al it1 = r_mol[0];it1 != r_mol[1];it1++)
			{
				if ((* it1).el.GetAtomicNumber() == 1) H_count++;
				if ((* it1).el.GetAtomicNumber() == 8) O_count++;
			}
			
			if (H_count == 2 && O_count == 1)
			{
				for (iter_al it1 = r_mol[0];it1 != r_mol[1];it1++)
				{
					if ((* it1).el.GetAtomicNumber() == 1) (* it1).atmtp_E = (ATE_type_XT | ATE_atmn_xt_Hh2o);
					if ((* it1).el.GetAtomicNumber() == 8) (* it1).atmtp_E = (ATE_type_XT | ATE_atmn_xt_Oh2o);
				}
			}
		}
		
		// other tests???
		// other tests???
		// other tests???
	}
	
	// check amino/nucleic acids, by chains...
	// check amino/nucleic acids, by chains...
	// check amino/nucleic acids, by chains...
	
//	if (!mdl->ref_civ) { cout << "e_UpdateTypes : ref_civ == NULL!" << endl; exit(EXIT_FAILURE); }
	if (!mdl->ref_civ) mdl->UpdateChains();
	
	vector<chn_info> & ci_vector = (* mdl->ref_civ);
	{
	for (i32u n1 = 0;n1 < ci_vector.size();n1++)
	{
		iter_al chnR[2]; mdl->GetRange(1, n1, chnR);
		
		// amino...
		// amino...
		// amino...
		
		if (ci_vector[n1].GetType() == chn_info::amino_acid)
		{
			const char * sequence = ci_vector[n1].GetSequence();
			for (i32s n2 = 0;n2 < ci_vector[n1].GetLength();n2++)
			{
				iter_al resR[2]; mdl->GetRange(2, chnR, n2, resR);
				
				atom * atmr_00 = e_UT_FindAtom(resR, 0x00);	// N
				atom * atmr_01 = e_UT_FindAtom(resR, 0x01);	// alpha-C
				atom * atmr_02 = e_UT_FindAtom(resR, 0x02);	// carbonyl-C
				atom * atmr_10 = e_UT_FindAtom(resR, 0x10);	// carbonyl-O
				
				if (!atmr_00 || !atmr_01 || !atmr_02 || !atmr_10) continue;
				
				i32s flags_chn = ATE_type_AA;
				i32s flags_res = 0;
				
				atmr_00->atmtp_E = ATE_atmn_aa_N;
				atmr_01->atmtp_E = ATE_atmn_aa_CA;
				atmr_02->atmtp_E = ATE_atmn_aa_C;
				atmr_10->atmtp_E = ATE_atmn_aa_O;
				
				vector<atom *> htab_00; e_UT_FindHydrogens(resR, 0x00, htab_00);
				vector<atom *> htab_01; e_UT_FindHydrogens(resR, 0x01, htab_01);
				
				if (htab_01.size() != 1 && !(sequence[n2] == 'G' && htab_01.size() == 2))
				{
					cout << "eUT: incorrect alpha-C hydrogen count!" << endl;
					exit(EXIT_FAILURE);
				}
				
				htab_01.front()->atmtp_E = ATE_atmn_aa_HA;
				
				bool tN_processed = false;
				if (n2 == 0)					// N-terminal residue???
				{
					i32u num_h_expected = 3;
					if (sequence[n2] == 'P') num_h_expected -= 1;
					
				// TODO : allow also -NH2???
					if (htab_00.size() != num_h_expected)
					{
						cout << "eUT: unknown N-terminus!!!" << endl;
						exit(EXIT_FAILURE);
					}
					
					if (num_h_expected == 3)
					{
						htab_00[0]->atmtp_E = ATE_atmn_aa_H1;
						htab_00[1]->atmtp_E = ATE_atmn_aa_H2;
						htab_00[2]->atmtp_E = ATE_atmn_aa_H3;
					}
					else
					{
						htab_00[0]->atmtp_E = ATE_atmn_aa_H2;
						htab_00[1]->atmtp_E = ATE_atmn_aa_H3;
					}
					
					flags_chn = ATE_type_AANT;
					tN_processed = true;
				}
				
				if (!tN_processed)
				{
					i32u num_h_expected = 1;
					if (sequence[n2] == 'P') num_h_expected -= 1;
					
					if (htab_00.size() != num_h_expected)
					{
						cout << "eUT: incorrect N hydrogen count!" << endl;
						exit(EXIT_FAILURE);
					}
					
					if (num_h_expected == 1)
					{
						htab_00.front()->atmtp_E = ATE_atmn_aa_H;
					}
				}
				
				bool tC_processed = false;
				if (n2 == ci_vector[n1].GetLength() - 1)	// C-terminal residue???
				{
				//	atom * atmr_11 = e_UT_FindAtom(resR, 0x11);
					atom * atmr_11 = NULL;	// search OXT directly since seq-builder misses it...
					for (iter_cl it1 = atmr_02->cr_list.begin();it1 != atmr_02->cr_list.end();it1++)
					{
						if ((* it1).atmr->el.GetAtomicNumber() != 8) continue;
						if (((* it1).atmr->builder_res_id & 0xFF) == 0x10) continue;
						
						atmr_11 = (* it1).atmr;
					}
					
				// TODO : allow also -COOH???
					if (!atmr_11)
					{
						cout << "eUT: unknown C-terminus!!!" << endl;
						exit(EXIT_FAILURE);
					}
					
					atmr_11->atmtp_E = ATE_atmn_aa_OXT;
					
					flags_chn = ATE_type_AACT;
					tC_processed = true;
				}
				
				if (!tC_processed)
				{
					// nothing needed here...
				}
				
				// start handling the side chains!!!
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
				
				if (sequence[n2] == 'A')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					
					bool valid_residue = true;
					if (!atmr_20) valid_residue = false;
					if (htab_20.size() != 3) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue A." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_ALA;
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB1;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[2]->atmtp_E = ATE_atmn_aa_HB3;
				}
				
				if (sequence[n2] == 'R')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					atom * atmr_25 = e_UT_FindAtom(resR, 0x25);
					atom * atmr_26 = e_UT_FindAtom(resR, 0x26);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					vector<atom *> htab_22; e_UT_FindHydrogens(resR, 0x22, htab_22);
					vector<atom *> htab_23; e_UT_FindHydrogens(resR, 0x23, htab_23);
					vector<atom *> htab_25; e_UT_FindHydrogens(resR, 0x25, htab_25);
					vector<atom *> htab_26; e_UT_FindHydrogens(resR, 0x26, htab_26);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24 || !atmr_25 || !atmr_26) valid_residue = false;
					if (htab_20.size() != 2 || htab_21.size() != 2 || htab_22.size() != 2 || htab_23.size() != 1 || htab_25.size() != 2 || htab_26.size() != 2) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue R." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_ARG;
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_CG;
					htab_21[0]->atmtp_E = ATE_atmn_aa_HG2;
					htab_21[1]->atmtp_E = ATE_atmn_aa_HG3;
					atmr_22->atmtp_E = ATE_atmn_aa_CD;
					htab_22[0]->atmtp_E = ATE_atmn_aa_HD2;
					htab_22[1]->atmtp_E = ATE_atmn_aa_HD3;
					atmr_23->atmtp_E = ATE_atmn_aa_NE;
					htab_23[0]->atmtp_E = ATE_atmn_aa_HE;
					atmr_24->atmtp_E = ATE_atmn_aa_CZ;
					atmr_25->atmtp_E = ATE_atmn_aa_NH1;
					htab_25[0]->atmtp_E = ATE_atmn_aa_HH11;
					htab_25[1]->atmtp_E = ATE_atmn_aa_HH12;
					atmr_26->atmtp_E = ATE_atmn_aa_NH2;
					htab_26[0]->atmtp_E = ATE_atmn_aa_HH21;
					htab_26[1]->atmtp_E = ATE_atmn_aa_HH22;
				}
				
				if (sequence[n2] == 'N')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_23; e_UT_FindHydrogens(resR, 0x23, htab_23);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23) valid_residue = false;
					if (htab_20.size() != 2 || htab_23.size() != 2) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue N." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_ASN;
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_CG;
					atmr_22->atmtp_E = ATE_atmn_aa_OD1;
					atmr_23->atmtp_E = ATE_atmn_aa_ND2;
					htab_23[0]->atmtp_E = ATE_atmn_aa_HD21;
					htab_23[1]->atmtp_E = ATE_atmn_aa_HD22;
				}
				
				if (sequence[n2] == 'D')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_22; e_UT_FindHydrogens(resR, 0x22, htab_22);
					vector<atom *> htab_23; e_UT_FindHydrogens(resR, 0x23, htab_23);
					
					i32u sumh = htab_22.size() + htab_23.size();
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23) valid_residue = false;
					if (htab_20.size() != 2 || (sumh != 0 && sumh != 1)) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue D." << endl;
						exit(EXIT_FAILURE);
					}
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_CG;
					if (sumh == 0)
					{
						flags_res = ATE_resn_aa_ASP;
						
						atmr_22->atmtp_E = ATE_atmn_aa_OD1;
						atmr_23->atmtp_E = ATE_atmn_aa_OD2;
					}
					else if (sumh == 1)
					{
						flags_res = ATE_resn_aa_ASH;
						
						atom * atmr[2] = { atmr_22, atmr_23 };
						vector<atom *> * htab[2] = { & htab_22, & htab_23 };
						
						bool dir = (htab_22.size() != 0);
						
						atmr[dir]->atmtp_E = ATE_atmn_aa_OD1;
						atmr[!dir]->atmtp_E = ATE_atmn_aa_OD2;
						htab[!dir]->operator[](0)->atmtp_E = ATE_atmn_aa_HD2;
					}
					else
					{
						cout << "eUT: unknown error (residue D)." << endl;
						exit(EXIT_FAILURE);
					}
				}
				
				if (sequence[n2] == 'C')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21) valid_residue = false;
					if (htab_20.size() != 2 || htab_21.size() > 1) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue C." << endl;
						exit(EXIT_FAILURE);
					}
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_SG;
					
					if (htab_21.size() == 1)
					{
						flags_res = ATE_resn_aa_CYS;
						
						htab_21[0]->atmtp_E = ATE_atmn_aa_HG;
					}
					else
					{
						if (atmr_21->cr_list.size() > 1)
						{
							flags_res = ATE_resn_aa_CYX;
						}
						else
						{
							flags_res = ATE_resn_aa_CYM;
						}
					}
				}
				
				if (sequence[n2] == 'Q')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					vector<atom *> htab_24; e_UT_FindHydrogens(resR, 0x24, htab_24);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24) valid_residue = false;
					if (htab_20.size() != 2 || htab_21.size() != 2 || htab_24.size() != 2) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue Q." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_GLN;
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_CG;
					htab_21[0]->atmtp_E = ATE_atmn_aa_HG2;
					htab_21[1]->atmtp_E = ATE_atmn_aa_HG3;
					atmr_22->atmtp_E = ATE_atmn_aa_CD;
					atmr_23->atmtp_E = ATE_atmn_aa_OE1;
					atmr_24->atmtp_E = ATE_atmn_aa_NE2;
					htab_24[0]->atmtp_E = ATE_atmn_aa_HE21;
					htab_24[1]->atmtp_E = ATE_atmn_aa_HE22;
				}
				
				if (sequence[n2] == 'E')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					vector<atom *> htab_23; e_UT_FindHydrogens(resR, 0x23, htab_23);
					vector<atom *> htab_24; e_UT_FindHydrogens(resR, 0x24, htab_24);
					
					i32u sumh = htab_23.size() + htab_24.size();
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24) valid_residue = false;
					if (htab_20.size() != 2 || htab_21.size() != 2 || (sumh != 0 && sumh != 1)) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue E." << endl;
						exit(EXIT_FAILURE);
					}
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_CG;
					htab_21[0]->atmtp_E = ATE_atmn_aa_HG2;
					htab_21[1]->atmtp_E = ATE_atmn_aa_HG3;
					atmr_22->atmtp_E = ATE_atmn_aa_CD;
					if (sumh == 0)
					{
						flags_res = ATE_resn_aa_GLU;
						
						atmr_23->atmtp_E = ATE_atmn_aa_OE1;
						atmr_24->atmtp_E = ATE_atmn_aa_OE2;
					}
					else if (sumh == 1)
					{
						flags_res = ATE_resn_aa_GLH;
						
						atom * atmr[2] = { atmr_23, atmr_24 };
						vector<atom *> * htab[2] = { & htab_23, & htab_24 };
						
						bool dir = (htab_23.size() != 0);
						
						atmr[dir]->atmtp_E = ATE_atmn_aa_OE1;
						atmr[!dir]->atmtp_E = ATE_atmn_aa_OE2;
						htab[!dir]->operator[](0)->atmtp_E = ATE_atmn_aa_HE2;
					}
					else
					{
						cout << "eUT: unknown error (residue E)." << endl;
						exit(EXIT_FAILURE);
					}
				}
				
				if (sequence[n2] == 'G')
				{
					bool valid_residue = true;
					if (htab_01.size() != 2) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue G." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_GLY;
					
					htab_01[0]->atmtp_E = ATE_atmn_aa_HA2;		// this overrides the default name "HA" set above...
					htab_01[1]->atmtp_E = ATE_atmn_aa_HA3;		// this overrides the default name "HA" set above...
				}
				
				if (sequence[n2] == 'H')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					atom * atmr_25 = e_UT_FindAtom(resR, 0x25);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_22; e_UT_FindHydrogens(resR, 0x22, htab_22);
					vector<atom *> htab_23; e_UT_FindHydrogens(resR, 0x23, htab_23);
					vector<atom *> htab_24; e_UT_FindHydrogens(resR, 0x24, htab_24);
					vector<atom *> htab_25; e_UT_FindHydrogens(resR, 0x25, htab_25);
					
					i32u sumh = htab_22.size() + htab_24.size();
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24 || !atmr_25) valid_residue = false;
					if (htab_20.size() != 2 || htab_23.size() != 1 || htab_25.size() != 1 || (sumh != 1 && sumh != 2)) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue H." << endl;
						exit(EXIT_FAILURE);
					}
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_CG;
					atmr_22->atmtp_E = ATE_atmn_aa_ND1;
					atmr_23->atmtp_E = ATE_atmn_aa_CE1;
					htab_23[0]->atmtp_E = ATE_atmn_aa_HE1;
					atmr_24->atmtp_E = ATE_atmn_aa_NE2;
					atmr_25->atmtp_E = ATE_atmn_aa_CD2;
					htab_25[0]->atmtp_E = ATE_atmn_aa_HD2;
					
					if (htab_22.size() == 1) htab_22[0]->atmtp_E = ATE_atmn_aa_HD1;
					if (htab_24.size() == 1) htab_24[0]->atmtp_E = ATE_atmn_aa_HE2;
					
					if (sumh == 2) flags_res = ATE_resn_aa_HIP;
					else if (htab_22.size() == 1) flags_res = ATE_resn_aa_HID;
					else if (htab_24.size() == 1) flags_res = ATE_resn_aa_HIE;
					else
					{
						cout << "eUT: unknown error (residue H)." << endl;
						exit(EXIT_FAILURE);
					}
				}
				
				if (sequence[n2] == 'I')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					vector<atom *> htab_22; e_UT_FindHydrogens(resR, 0x22, htab_22);
					vector<atom *> htab_23; e_UT_FindHydrogens(resR, 0x23, htab_23);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23) valid_residue = false;
					if (htab_20.size() != 1 || htab_21.size() != 3 || htab_22.size() != 2 || htab_23.size() != 3) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue I." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_ILE;
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB;
					atmr_21->atmtp_E = ATE_atmn_aa_CG2;
					htab_21[0]->atmtp_E = ATE_atmn_aa_HG21;
					htab_21[1]->atmtp_E = ATE_atmn_aa_HG22;
					htab_21[2]->atmtp_E = ATE_atmn_aa_HG23;
					atmr_22->atmtp_E = ATE_atmn_aa_CG1;
					htab_22[0]->atmtp_E = ATE_atmn_aa_HG12;
					htab_22[1]->atmtp_E = ATE_atmn_aa_HG13;
					atmr_23->atmtp_E = ATE_atmn_aa_CD1;
					htab_23[0]->atmtp_E = ATE_atmn_aa_HD11;
					htab_23[1]->atmtp_E = ATE_atmn_aa_HD12;
					htab_23[2]->atmtp_E = ATE_atmn_aa_HD13;
				}
				
				if (sequence[n2] == 'L')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					vector<atom *> htab_22; e_UT_FindHydrogens(resR, 0x22, htab_22);
					vector<atom *> htab_23; e_UT_FindHydrogens(resR, 0x23, htab_23);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23) valid_residue = false;
					if (htab_20.size() != 2 || htab_21.size() != 1 || htab_22.size() != 3 || htab_23.size() != 3) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue L." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_LEU;
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_CG;
					htab_21[0]->atmtp_E = ATE_atmn_aa_HG;
					atmr_22->atmtp_E = ATE_atmn_aa_CD1;
					htab_22[0]->atmtp_E = ATE_atmn_aa_HD11;
					htab_22[1]->atmtp_E = ATE_atmn_aa_HD12;
					htab_22[2]->atmtp_E = ATE_atmn_aa_HD13;
					atmr_23->atmtp_E = ATE_atmn_aa_CD2;
					htab_23[0]->atmtp_E = ATE_atmn_aa_HD21;
					htab_23[1]->atmtp_E = ATE_atmn_aa_HD22;
					htab_23[2]->atmtp_E = ATE_atmn_aa_HD23;
				}
				
				if (sequence[n2] == 'K')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					vector<atom *> htab_22; e_UT_FindHydrogens(resR, 0x22, htab_22);
					vector<atom *> htab_23; e_UT_FindHydrogens(resR, 0x23, htab_23);
					vector<atom *> htab_24; e_UT_FindHydrogens(resR, 0x24, htab_24);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24) valid_residue = false;
					if (htab_20.size() != 2 || htab_21.size() != 2 || htab_22.size() != 2 || htab_23.size() != 2 || (htab_24.size() != 2 && htab_24.size() != 3)) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue K." << endl;
						exit(EXIT_FAILURE);
					}
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_CG;
					htab_21[0]->atmtp_E = ATE_atmn_aa_HG2;
					htab_21[1]->atmtp_E = ATE_atmn_aa_HG3;
					atmr_22->atmtp_E = ATE_atmn_aa_CD;
					htab_22[0]->atmtp_E = ATE_atmn_aa_HD2;
					htab_22[1]->atmtp_E = ATE_atmn_aa_HD3;
					atmr_23->atmtp_E = ATE_atmn_aa_CE;
					htab_23[0]->atmtp_E = ATE_atmn_aa_HE2;
					htab_23[1]->atmtp_E = ATE_atmn_aa_HE3;
					atmr_24->atmtp_E = ATE_atmn_aa_NZ;
					htab_24[0]->atmtp_E = ATE_atmn_aa_HZ2;
					htab_24[1]->atmtp_E = ATE_atmn_aa_HZ3;
					
					if (htab_24.size() == 2)
					{
						flags_res = ATE_resn_aa_LYN;
					}
					else if (htab_24.size() == 3)
					{
						flags_res = ATE_resn_aa_LYS;
						
						htab_24[2]->atmtp_E = ATE_atmn_aa_HZ1;
					}
					else
					{
						cout << "eUT: unknown error (residue K)." << endl;
						exit(EXIT_FAILURE);
					}
				}
				
				if (sequence[n2] == 'M')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					vector<atom *> htab_23; e_UT_FindHydrogens(resR, 0x23, htab_23);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23) valid_residue = false;
					if (htab_20.size() != 2 || htab_21.size() != 2 || htab_23.size() != 3) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue M." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_MET;
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_CG;
					htab_21[0]->atmtp_E = ATE_atmn_aa_HG2;
					htab_21[1]->atmtp_E = ATE_atmn_aa_HG3;
					atmr_22->atmtp_E = ATE_atmn_aa_SD;
					atmr_23->atmtp_E = ATE_atmn_aa_CE;
					htab_23[0]->atmtp_E = ATE_atmn_aa_HE1;
					htab_23[1]->atmtp_E = ATE_atmn_aa_HE2;
					htab_23[2]->atmtp_E = ATE_atmn_aa_HE3;
				}
				
				if (sequence[n2] == 'F')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					atom * atmr_25 = e_UT_FindAtom(resR, 0x25);
					atom * atmr_26 = e_UT_FindAtom(resR, 0x26);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_22; e_UT_FindHydrogens(resR, 0x22, htab_22);
					vector<atom *> htab_23; e_UT_FindHydrogens(resR, 0x23, htab_23);
					vector<atom *> htab_24; e_UT_FindHydrogens(resR, 0x24, htab_24);
					vector<atom *> htab_25; e_UT_FindHydrogens(resR, 0x25, htab_25);
					vector<atom *> htab_26; e_UT_FindHydrogens(resR, 0x26, htab_26);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24 || !atmr_25 || !atmr_26) valid_residue = false;
					if (htab_20.size() != 2 || htab_22.size() != 1 || htab_23.size() != 1 || htab_24.size() != 1 || htab_25.size() != 1 || htab_26.size() != 1) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue F." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_PHE;
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_CG;
					atmr_22->atmtp_E = ATE_atmn_aa_CD1;
					htab_22[0]->atmtp_E = ATE_atmn_aa_HD1;
					atmr_23->atmtp_E = ATE_atmn_aa_CE1;
					htab_23[0]->atmtp_E = ATE_atmn_aa_HE1;
					atmr_24->atmtp_E = ATE_atmn_aa_CZ;
					htab_24[0]->atmtp_E = ATE_atmn_aa_HZ;
					atmr_25->atmtp_E = ATE_atmn_aa_CE2;
					htab_25[0]->atmtp_E = ATE_atmn_aa_HE2;
					atmr_26->atmtp_E = ATE_atmn_aa_CD2;
					htab_26[0]->atmtp_E = ATE_atmn_aa_HD2;
				}
				
				if (sequence[n2] == 'P')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					vector<atom *> htab_22; e_UT_FindHydrogens(resR, 0x22, htab_22);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22) valid_residue = false;
					if (htab_20.size() != 2 || htab_20.size() != 2 || htab_20.size() != 2) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue P." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_PRO;
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_CG;
					htab_21[0]->atmtp_E = ATE_atmn_aa_HG2;
					htab_21[1]->atmtp_E = ATE_atmn_aa_HG3;
					atmr_22->atmtp_E = ATE_atmn_aa_CD;
					htab_22[0]->atmtp_E = ATE_atmn_aa_HD2;
					htab_22[1]->atmtp_E = ATE_atmn_aa_HD3;
				}
				
				if (sequence[n2] == 'S')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21) valid_residue = false;
					if (htab_20.size() != 2 || htab_21.size() != 1) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue S." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_SER;
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_OG;
					htab_21[0]->atmtp_E = ATE_atmn_aa_HG;
				}
				
				if (sequence[n2] == 'T')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					vector<atom *> htab_22; e_UT_FindHydrogens(resR, 0x22, htab_22);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22) valid_residue = false;
					if (htab_20.size() != 1 || htab_21.size() != 3 || htab_22.size() != 1) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue T." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_THR;
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB;
					atmr_21->atmtp_E = ATE_atmn_aa_CG2;
					htab_21[0]->atmtp_E = ATE_atmn_aa_HG21;
					htab_21[1]->atmtp_E = ATE_atmn_aa_HG22;
					htab_21[2]->atmtp_E = ATE_atmn_aa_HG23;
					atmr_22->atmtp_E = ATE_atmn_aa_OG1;
					htab_22[0]->atmtp_E = ATE_atmn_aa_HG1;
				}
				
				if (sequence[n2] == 'W')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					atom * atmr_25 = e_UT_FindAtom(resR, 0x25);
					atom * atmr_26 = e_UT_FindAtom(resR, 0x26);
					atom * atmr_27 = e_UT_FindAtom(resR, 0x27);
					atom * atmr_28 = e_UT_FindAtom(resR, 0x28);
					atom * atmr_29 = e_UT_FindAtom(resR, 0x29);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_22; e_UT_FindHydrogens(resR, 0x22, htab_22);
					vector<atom *> htab_23; e_UT_FindHydrogens(resR, 0x23, htab_23);
					vector<atom *> htab_25; e_UT_FindHydrogens(resR, 0x25, htab_25);
					vector<atom *> htab_26; e_UT_FindHydrogens(resR, 0x26, htab_26);
					vector<atom *> htab_27; e_UT_FindHydrogens(resR, 0x27, htab_27);
					vector<atom *> htab_28; e_UT_FindHydrogens(resR, 0x28, htab_28);
					
// a temporary bugfix : might need to swap atoms 25<->29 and 26<->28 due to a bug in seq-builder!!!
// a temporary bugfix : might need to swap atoms 25<->29 and 26<->28 due to a bug in seq-builder!!!
// a temporary bugfix : might need to swap atoms 25<->29 and 26<->28 due to a bug in seq-builder!!!
vector<atom *> htab_29; e_UT_FindHydrogens(resR, 0x29, htab_29);
if (htab_29.size() != 0 && htab_25.size() == 0)
{
	atmr_25 = e_UT_FindAtom(resR, 0x29);
	atmr_26 = e_UT_FindAtom(resR, 0x28);
	atmr_28 = e_UT_FindAtom(resR, 0x26);
	atmr_29 = e_UT_FindAtom(resR, 0x25);
	htab_25.resize(0); e_UT_FindHydrogens(resR, 0x29, htab_25);
	htab_26.resize(0); e_UT_FindHydrogens(resR, 0x28, htab_26);
	htab_28.resize(0); e_UT_FindHydrogens(resR, 0x26, htab_28);
	htab_29.resize(0); e_UT_FindHydrogens(resR, 0x25, htab_29);
}
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24 || !atmr_25 || !atmr_26 || !atmr_27 || !atmr_28 || !atmr_29) valid_residue = false;
					if (htab_20.size() != 2 || htab_22.size() != 1 || htab_23.size() != 1 || htab_25.size() != 1 || htab_26.size() != 1 || htab_27.size() != 1 || htab_28.size() != 1) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue W." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_TRP;
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_CG;
					atmr_22->atmtp_E = ATE_atmn_aa_CD1;
					htab_22[0]->atmtp_E = ATE_atmn_aa_HD1;
					atmr_23->atmtp_E = ATE_atmn_aa_NE1;
					htab_23[0]->atmtp_E = ATE_atmn_aa_HE1;
					atmr_24->atmtp_E = ATE_atmn_aa_CD2;
					atmr_25->atmtp_E = ATE_atmn_aa_CE3;
					htab_25[0]->atmtp_E = ATE_atmn_aa_HE3;
					atmr_26->atmtp_E = ATE_atmn_aa_CZ3;
					htab_26[0]->atmtp_E = ATE_atmn_aa_HZ3;
					atmr_27->atmtp_E = ATE_atmn_aa_CH2;
					htab_27[0]->atmtp_E = ATE_atmn_aa_HH2;
					atmr_28->atmtp_E = ATE_atmn_aa_CZ2;
					htab_28[0]->atmtp_E = ATE_atmn_aa_HZ2;
					atmr_29->atmtp_E = ATE_atmn_aa_CE2;
				}
				
				if (sequence[n2] == 'Y')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					atom * atmr_25 = e_UT_FindAtom(resR, 0x25);
					atom * atmr_26 = e_UT_FindAtom(resR, 0x26);
					atom * atmr_27 = e_UT_FindAtom(resR, 0x27);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_22; e_UT_FindHydrogens(resR, 0x22, htab_22);
					vector<atom *> htab_23; e_UT_FindHydrogens(resR, 0x23, htab_23);
					vector<atom *> htab_25; e_UT_FindHydrogens(resR, 0x25, htab_25);
					vector<atom *> htab_26; e_UT_FindHydrogens(resR, 0x26, htab_26);
					vector<atom *> htab_27; e_UT_FindHydrogens(resR, 0x27, htab_27);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24 || !atmr_25 || !atmr_26 || !atmr_27) valid_residue = false;
					if (htab_20.size() != 2 || htab_22.size() != 1 || htab_23.size() != 1 || htab_25.size() != 1 || htab_26.size() != 1 || htab_27.size() != 1) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue Y." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_TYR;
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB2;
					htab_20[1]->atmtp_E = ATE_atmn_aa_HB3;
					atmr_21->atmtp_E = ATE_atmn_aa_CG;
					atmr_22->atmtp_E = ATE_atmn_aa_CD1;
					htab_22[0]->atmtp_E = ATE_atmn_aa_HD1;
					atmr_23->atmtp_E = ATE_atmn_aa_CE1;
					htab_23[0]->atmtp_E = ATE_atmn_aa_HE1;
					atmr_24->atmtp_E = ATE_atmn_aa_CZ;
					atmr_25->atmtp_E = ATE_atmn_aa_CE2;
					htab_25[0]->atmtp_E = ATE_atmn_aa_HE2;
					atmr_26->atmtp_E = ATE_atmn_aa_CD2;
					htab_26[0]->atmtp_E = ATE_atmn_aa_HD2;
					atmr_27->atmtp_E = ATE_atmn_aa_OH;
					htab_27[0]->atmtp_E = ATE_atmn_aa_HH;
				}
				
				if (sequence[n2] == 'V')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					vector<atom *> htab_20; e_UT_FindHydrogens(resR, 0x20, htab_20);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					vector<atom *> htab_22; e_UT_FindHydrogens(resR, 0x22, htab_22);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22) valid_residue = false;
					if (htab_20.size() != 1 || htab_21.size() != 3 || htab_22.size() != 3) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue V." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_aa_VAL;
					
					atmr_20->atmtp_E = ATE_atmn_aa_CB;
					htab_20[0]->atmtp_E = ATE_atmn_aa_HB;
					atmr_21->atmtp_E = ATE_atmn_aa_CG1;
					htab_21[0]->atmtp_E = ATE_atmn_aa_HG11;
					htab_21[1]->atmtp_E = ATE_atmn_aa_HG12;
					htab_21[2]->atmtp_E = ATE_atmn_aa_HG13;
					atmr_22->atmtp_E = ATE_atmn_aa_CG2;
					htab_22[0]->atmtp_E = ATE_atmn_aa_HG21;
					htab_22[1]->atmtp_E = ATE_atmn_aa_HG22;
					htab_22[2]->atmtp_E = ATE_atmn_aa_HG23;
				}
				
				// finally, set also the chn/res parts of the type information...
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
				
				for (iter_al it1 = resR[0];it1 != resR[1];it1++)
				{
					if ((* it1).atmtp_E != NOT_DEFINED)
					{
						(* it1).atmtp_E |= (flags_chn | flags_res);
					}
				}
			}
		}
		
		// nucleic...
		// nucleic...
		// nucleic...
		
		if (ci_vector[n1].GetType() == chn_info::nucleic_acid)
		{
			const char * sequence = ci_vector[n1].GetSequence();
			for (i32s n2 = 0;n2 < ci_vector[n1].GetLength();n2++)
			{
				iter_al resR[2]; mdl->GetRange(2, chnR, n2, resR);
				
				atom * atmr_01 = e_UT_FindAtom(resR, 0x01);
				atom * atmr_02 = e_UT_FindAtom(resR, 0x02);
				atom * atmr_03 = e_UT_FindAtom(resR, 0x03);
				atom * atmr_04 = e_UT_FindAtom(resR, 0x04);
				atom * atmr_05 = e_UT_FindAtom(resR, 0x05);
				atom * atmr_06 = e_UT_FindAtom(resR, 0x06);
				atom * atmr_07 = e_UT_FindAtom(resR, 0x07);
				atom * atmr_08 = e_UT_FindAtom(resR, 0x08);
				
				if (!atmr_01 || !atmr_02 || !atmr_03 || !atmr_04) continue;
				if (!atmr_05 || !atmr_06 || !atmr_07 || !atmr_08) continue;
				
				i32s flags_chn = ATE_type_NA;
				i32s flags_res = 0;
				
				atmr_01->atmtp_E = ATE_atmn_na_O5p;
				atmr_02->atmtp_E = ATE_atmn_na_C5p;
				atmr_03->atmtp_E = ATE_atmn_na_C4p;
				atmr_04->atmtp_E = ATE_atmn_na_O4p;
				atmr_05->atmtp_E = ATE_atmn_na_C1p;
				atmr_06->atmtp_E = ATE_atmn_na_C2p;
				atmr_07->atmtp_E = ATE_atmn_na_C3p;
				atmr_08->atmtp_E = ATE_atmn_na_O3p;
				
				vector<atom *> htab_02; e_UT_FindHydrogens(resR, 0x02, htab_02);
				vector<atom *> htab_03; e_UT_FindHydrogens(resR, 0x03, htab_03);
				vector<atom *> htab_05; e_UT_FindHydrogens(resR, 0x05, htab_05);
				vector<atom *> htab_06; e_UT_FindHydrogens(resR, 0x06, htab_06);
				vector<atom *> htab_07; e_UT_FindHydrogens(resR, 0x07, htab_07);
				
				if (htab_02.size() != 2)
				{
					cout << "eUT: incorrect C5' hydrogen count!" << endl;
					exit(EXIT_FAILURE);
				}
				
				htab_02[0]->atmtp_E = ATE_atmn_na_H5p1;
				htab_02[0]->atmtp_E = ATE_atmn_na_H5p2;
				
				if (htab_03.size() != 1)
				{
					cout << "eUT: incorrect C4' hydrogen count!" << endl;
					exit(EXIT_FAILURE);
				}
				
				htab_03[0]->atmtp_E = ATE_atmn_na_H4p;
				
				if (htab_05.size() != 1)
				{
					cout << "eUT: incorrect C1' hydrogen count!" << endl;
					exit(EXIT_FAILURE);
				}
				
				htab_05[0]->atmtp_E = ATE_atmn_na_H1p;
				
				i32u num_h_expected = 2;			// for DNA.
				if (sequence[n2] > 'Z') num_h_expected -= 1;	// for RNA (lower case symbols).
				
				if (htab_06.size() != num_h_expected)
				{
					cout << "eUT: incorrect C2' hydrogen count!" << endl;
					exit(EXIT_FAILURE);
				}
				
				htab_06[0]->atmtp_E = ATE_atmn_na_H2p1;
				if (htab_06.size() > 1)
				{
					htab_06[1]->atmtp_E = ATE_atmn_na_H2p2;
				}
				else
				{
					atom * atmr_30 = e_UT_FindAtom(resR, 0x30);
					
					if (!atmr_30)
					{
						cout << "eUT: 2' -OH lacks oxygen atom!" << endl;
						exit(EXIT_FAILURE);
					}
					
					vector<atom *> htab_30; e_UT_FindHydrogens(resR, 0x30, htab_30);
					
					if (htab_30.size() != 1)
					{
						cout << "eUT: 2' -OH has incorrect hydrogen count!" << endl;
						exit(EXIT_FAILURE);
					}
					
					atmr_07->atmtp_E = ATE_atmn_na_O2p;
					htab_30.front()->atmtp_E = ATE_atmn_na_HOp2;
				}
				
				if (htab_07.size() != 1)
				{
					cout << "eUT: incorrect C3' hydrogen count!" << endl;
					exit(EXIT_FAILURE);
				}
				
				htab_07[0]->atmtp_E = ATE_atmn_na_H3p;
				
				bool t5_processed = false;
				if (n2 == 0)					// 5'-terminal residue???
				{
					// at 5'-terminal, there really should be an -OH group.
					// however, the current sequencebuilder does not support that.
					
					// therefore, leave the 5'-terminus without typing information,
					// so that the default parameterization will be used...
					
					flags_chn = ATE_type_NA5T;
					t5_processed = true;
				}
				
				if (!t5_processed)
				{
					atom * atmr_00 = e_UT_FindAtom(resR, 0x00);
					atom * atmr_10 = e_UT_FindAtom(resR, 0x10);
					atom * atmr_11 = e_UT_FindAtom(resR, 0x11);
					
					if (!atmr_00 || !atmr_10 || !atmr_11)
					{
						cout << "eUT: unknown 5'-terminus!!!" << endl;
						exit(EXIT_FAILURE);
					}
					
					atmr_00->atmtp_E = ATE_atmn_na_P;
					atmr_10->atmtp_E = ATE_atmn_na_O1P;
					atmr_11->atmtp_E = ATE_atmn_na_O2P;
				}
				
				bool t3_processed = false;
				if (n2 == ci_vector[n1].GetLength() - 1)	// 3'-terminal residue???
				{
					vector<atom *> htab_08; e_UT_FindHydrogens(resR, 0x08, htab_08);
					
					if (htab_08.size() != 1)
					{
						cout << "eUT: 3' -OH has incorrect hydrogen count!" << endl;
						exit(EXIT_FAILURE);
					}
					
					htab_08.front()->atmtp_E = ATE_atmn_na_H3T;
					
					flags_chn = ATE_type_NA3T;
					t3_processed = true;
				}
				
				if (!t3_processed)
				{
					// nothing needed here...
				}
				
				// start handling the side chains!!!
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
				
				if (sequence[n2] == 'A')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					atom * atmr_25 = e_UT_FindAtom(resR, 0x25);
					atom * atmr_26 = e_UT_FindAtom(resR, 0x26);
					atom * atmr_27 = e_UT_FindAtom(resR, 0x27);
					atom * atmr_28 = e_UT_FindAtom(resR, 0x28);
					atom * atmr_40 = e_UT_FindAtom(resR, 0x40);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					vector<atom *> htab_25; e_UT_FindHydrogens(resR, 0x25, htab_25);
					vector<atom *> htab_40; e_UT_FindHydrogens(resR, 0x40, htab_40);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24 || !atmr_25 || !atmr_26 || !atmr_27 || !atmr_28 || !atmr_40) valid_residue = false;
					if (htab_21.size() != 1 || htab_25.size() != 1 || htab_40.size() != 2) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue dA." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_na_DA;
					
					atmr_20->atmtp_E = ATE_atmn_na_N9;
					atmr_21->atmtp_E = ATE_atmn_na_C8;
					htab_21[0]->atmtp_E = ATE_atmn_na_H8;
					atmr_22->atmtp_E = ATE_atmn_na_N7;
					atmr_23->atmtp_E = ATE_atmn_na_C4;
					atmr_24->atmtp_E = ATE_atmn_na_N3;
					atmr_25->atmtp_E = ATE_atmn_na_C2;
					htab_25[0]->atmtp_E = ATE_atmn_na_H2;
					atmr_26->atmtp_E = ATE_atmn_na_N1;
					atmr_27->atmtp_E = ATE_atmn_na_C6;
					atmr_28->atmtp_E = ATE_atmn_na_C5;
					atmr_40->atmtp_E = ATE_atmn_na_N6;
					htab_40[0]->atmtp_E = ATE_atmn_na_H61;
					htab_40[1]->atmtp_E = ATE_atmn_na_H62;
				}
				
				if (sequence[n2] == 'C')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					atom * atmr_25 = e_UT_FindAtom(resR, 0x25);
					atom * atmr_40 = e_UT_FindAtom(resR, 0x40);
					atom * atmr_41 = e_UT_FindAtom(resR, 0x41);
					vector<atom *> htab_24; e_UT_FindHydrogens(resR, 0x24, htab_24);
					vector<atom *> htab_25; e_UT_FindHydrogens(resR, 0x25, htab_25);
					vector<atom *> htab_41; e_UT_FindHydrogens(resR, 0x41, htab_41);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24 || !atmr_25 || !atmr_40 || !atmr_41) valid_residue = false;
					if (htab_24.size() != 1 || htab_25.size() != 1 || htab_41.size() != 2) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue dC." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_na_DC;
					
					atmr_20->atmtp_E = ATE_atmn_na_N1;
					atmr_21->atmtp_E = ATE_atmn_na_C2;
					atmr_22->atmtp_E = ATE_atmn_na_N3;
					atmr_23->atmtp_E = ATE_atmn_na_C4;
					atmr_24->atmtp_E = ATE_atmn_na_C5;
					htab_24[0]->atmtp_E = ATE_atmn_na_H5;
					atmr_25->atmtp_E = ATE_atmn_na_C6;
					htab_25[0]->atmtp_E = ATE_atmn_na_H6;
					atmr_40->atmtp_E = ATE_atmn_na_O2;
					atmr_41->atmtp_E = ATE_atmn_na_N4;
					htab_41[0]->atmtp_E = ATE_atmn_na_H41;
					htab_41[1]->atmtp_E = ATE_atmn_na_H42;
				}
				
				if (sequence[n2] == 'G')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					atom * atmr_25 = e_UT_FindAtom(resR, 0x25);
					atom * atmr_26 = e_UT_FindAtom(resR, 0x26);
					atom * atmr_27 = e_UT_FindAtom(resR, 0x27);
					atom * atmr_28 = e_UT_FindAtom(resR, 0x28);
					atom * atmr_40 = e_UT_FindAtom(resR, 0x40);
					atom * atmr_41 = e_UT_FindAtom(resR, 0x41);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					vector<atom *> htab_26; e_UT_FindHydrogens(resR, 0x26, htab_26);
					vector<atom *> htab_40; e_UT_FindHydrogens(resR, 0x40, htab_40);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24 || !atmr_25 || !atmr_26 || !atmr_27 || !atmr_28 || !atmr_40 || !atmr_41) valid_residue = false;
					if (htab_21.size() != 1 || htab_26.size() != 1 || htab_40.size() != 2) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue dG." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_na_DG;
					
					atmr_20->atmtp_E = ATE_atmn_na_N9;
					atmr_21->atmtp_E = ATE_atmn_na_C8;
					htab_21[0]->atmtp_E = ATE_atmn_na_H8;
					atmr_22->atmtp_E = ATE_atmn_na_N7;
					atmr_23->atmtp_E = ATE_atmn_na_C4;
					atmr_24->atmtp_E = ATE_atmn_na_N3;
					atmr_25->atmtp_E = ATE_atmn_na_C2;
					atmr_26->atmtp_E = ATE_atmn_na_N1;
					htab_26[0]->atmtp_E = ATE_atmn_na_H1;
					atmr_27->atmtp_E = ATE_atmn_na_C6;
					atmr_28->atmtp_E = ATE_atmn_na_C5;
					atmr_40->atmtp_E = ATE_atmn_na_N2;
					htab_40[0]->atmtp_E = ATE_atmn_na_H21;
					htab_40[1]->atmtp_E = ATE_atmn_na_H22;
					atmr_41->atmtp_E = ATE_atmn_na_O6;
				}
				
				if (sequence[n2] == 'T')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					atom * atmr_25 = e_UT_FindAtom(resR, 0x25);
					atom * atmr_40 = e_UT_FindAtom(resR, 0x40);
					atom * atmr_41 = e_UT_FindAtom(resR, 0x41);
					atom * atmr_42 = e_UT_FindAtom(resR, 0x42);
					vector<atom *> htab_22; e_UT_FindHydrogens(resR, 0x22, htab_22);
					vector<atom *> htab_25; e_UT_FindHydrogens(resR, 0x25, htab_25);
					vector<atom *> htab_42; e_UT_FindHydrogens(resR, 0x42, htab_42);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24 || !atmr_25 || !atmr_40 || !atmr_41 || !atmr_42) valid_residue = false;
					if (htab_22.size() != 1 || htab_25.size() != 1 || htab_42.size() != 3) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue dT." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_na_DT;
					
					atmr_20->atmtp_E = ATE_atmn_na_N1;
					atmr_21->atmtp_E = ATE_atmn_na_C2;
					atmr_22->atmtp_E = ATE_atmn_na_N3;
					htab_22[0]->atmtp_E = ATE_atmn_na_H3;
					atmr_23->atmtp_E = ATE_atmn_na_C4;
					atmr_24->atmtp_E = ATE_atmn_na_C5;
					atmr_25->atmtp_E = ATE_atmn_na_C6;
					htab_25[0]->atmtp_E = ATE_atmn_na_H6;
					atmr_40->atmtp_E = ATE_atmn_na_O2;
					atmr_41->atmtp_E = ATE_atmn_na_O4;
					atmr_42->atmtp_E = ATE_atmn_na_C7;
					htab_42[0]->atmtp_E = ATE_atmn_na_H71;
					htab_42[1]->atmtp_E = ATE_atmn_na_H72;
					htab_42[2]->atmtp_E = ATE_atmn_na_H73;
				}
				
				if (sequence[n2] == 'a')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					atom * atmr_25 = e_UT_FindAtom(resR, 0x25);
					atom * atmr_26 = e_UT_FindAtom(resR, 0x26);
					atom * atmr_27 = e_UT_FindAtom(resR, 0x27);
					atom * atmr_28 = e_UT_FindAtom(resR, 0x28);
					atom * atmr_40 = e_UT_FindAtom(resR, 0x40);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					vector<atom *> htab_25; e_UT_FindHydrogens(resR, 0x25, htab_25);
					vector<atom *> htab_40; e_UT_FindHydrogens(resR, 0x40, htab_40);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24 || !atmr_25 || !atmr_26 || !atmr_27 || !atmr_28 || !atmr_40) valid_residue = false;
					if (htab_21.size() != 1 || htab_25.size() != 1 || htab_40.size() != 2) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue A." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_na_RA;
					
					atmr_20->atmtp_E = ATE_atmn_na_N9;
					atmr_21->atmtp_E = ATE_atmn_na_C8;
					htab_21[0]->atmtp_E = ATE_atmn_na_H8;
					atmr_22->atmtp_E = ATE_atmn_na_N7;
					atmr_23->atmtp_E = ATE_atmn_na_C4;
					atmr_24->atmtp_E = ATE_atmn_na_N3;
					atmr_25->atmtp_E = ATE_atmn_na_C2;
					htab_25[0]->atmtp_E = ATE_atmn_na_H2;
					atmr_26->atmtp_E = ATE_atmn_na_N1;
					atmr_27->atmtp_E = ATE_atmn_na_C6;
					atmr_28->atmtp_E = ATE_atmn_na_C5;
					atmr_40->atmtp_E = ATE_atmn_na_N6;
					htab_40[0]->atmtp_E = ATE_atmn_na_H61;
					htab_40[1]->atmtp_E = ATE_atmn_na_H62;
				}
				
				if (sequence[n2] == 'c')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					atom * atmr_25 = e_UT_FindAtom(resR, 0x25);
					atom * atmr_40 = e_UT_FindAtom(resR, 0x40);
					atom * atmr_41 = e_UT_FindAtom(resR, 0x41);
					vector<atom *> htab_24; e_UT_FindHydrogens(resR, 0x24, htab_24);
					vector<atom *> htab_25; e_UT_FindHydrogens(resR, 0x25, htab_25);
					vector<atom *> htab_41; e_UT_FindHydrogens(resR, 0x41, htab_41);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24 || !atmr_25 || !atmr_40 || !atmr_41) valid_residue = false;
					if (htab_24.size() != 1 || htab_25.size() != 1 || htab_41.size() != 2) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue C." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_na_RC;
					
					atmr_20->atmtp_E = ATE_atmn_na_N1;
					atmr_21->atmtp_E = ATE_atmn_na_C2;
					atmr_22->atmtp_E = ATE_atmn_na_N3;
					atmr_23->atmtp_E = ATE_atmn_na_C4;
					atmr_24->atmtp_E = ATE_atmn_na_C5;
					htab_24[0]->atmtp_E = ATE_atmn_na_H5;
					atmr_25->atmtp_E = ATE_atmn_na_C6;
					htab_25[0]->atmtp_E = ATE_atmn_na_H6;
					atmr_40->atmtp_E = ATE_atmn_na_O2;
					atmr_41->atmtp_E = ATE_atmn_na_N4;
					htab_41[0]->atmtp_E = ATE_atmn_na_H41;
					htab_41[1]->atmtp_E = ATE_atmn_na_H42;
				}
				
				if (sequence[n2] == 'g')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					atom * atmr_25 = e_UT_FindAtom(resR, 0x25);
					atom * atmr_26 = e_UT_FindAtom(resR, 0x26);
					atom * atmr_27 = e_UT_FindAtom(resR, 0x27);
					atom * atmr_28 = e_UT_FindAtom(resR, 0x28);
					atom * atmr_40 = e_UT_FindAtom(resR, 0x40);
					atom * atmr_41 = e_UT_FindAtom(resR, 0x41);
					vector<atom *> htab_21; e_UT_FindHydrogens(resR, 0x21, htab_21);
					vector<atom *> htab_26; e_UT_FindHydrogens(resR, 0x26, htab_26);
					vector<atom *> htab_40; e_UT_FindHydrogens(resR, 0x40, htab_40);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24 || !atmr_25 || !atmr_26 || !atmr_27 || !atmr_28 || !atmr_40 || !atmr_41) valid_residue = false;
					if (htab_21.size() != 1 || htab_26.size() != 1 || htab_40.size() != 2) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue G." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_na_RG;
					
					atmr_20->atmtp_E = ATE_atmn_na_N9;
					atmr_21->atmtp_E = ATE_atmn_na_C8;
					htab_21[0]->atmtp_E = ATE_atmn_na_H8;
					atmr_22->atmtp_E = ATE_atmn_na_N7;
					atmr_23->atmtp_E = ATE_atmn_na_C4;
					atmr_24->atmtp_E = ATE_atmn_na_N3;
					atmr_25->atmtp_E = ATE_atmn_na_C2;
					atmr_26->atmtp_E = ATE_atmn_na_N1;
					htab_26[0]->atmtp_E = ATE_atmn_na_H1;
					atmr_27->atmtp_E = ATE_atmn_na_C6;
					atmr_28->atmtp_E = ATE_atmn_na_C5;
					atmr_40->atmtp_E = ATE_atmn_na_N2;
					htab_40[0]->atmtp_E = ATE_atmn_na_H21;
					htab_40[1]->atmtp_E = ATE_atmn_na_H22;
					atmr_41->atmtp_E = ATE_atmn_na_O6;
				}
				
				if (sequence[n2] == 'u')
				{
					atom * atmr_20 = e_UT_FindAtom(resR, 0x20);
					atom * atmr_21 = e_UT_FindAtom(resR, 0x21);
					atom * atmr_22 = e_UT_FindAtom(resR, 0x22);
					atom * atmr_23 = e_UT_FindAtom(resR, 0x23);
					atom * atmr_24 = e_UT_FindAtom(resR, 0x24);
					atom * atmr_25 = e_UT_FindAtom(resR, 0x25);
					atom * atmr_40 = e_UT_FindAtom(resR, 0x40);
					atom * atmr_41 = e_UT_FindAtom(resR, 0x41);
					vector<atom *> htab_22; e_UT_FindHydrogens(resR, 0x22, htab_22);
					vector<atom *> htab_24; e_UT_FindHydrogens(resR, 0x24, htab_24);
					vector<atom *> htab_25; e_UT_FindHydrogens(resR, 0x25, htab_25);
					
					bool valid_residue = true;
					if (!atmr_20 || !atmr_21 || !atmr_22 || !atmr_23 || !atmr_24 || !atmr_25 || !atmr_40 || !atmr_41) valid_residue = false;
					if (htab_22.size() != 1 || htab_24.size() != 1 || htab_25.size() != 1) valid_residue = false;
					
					if (!valid_residue)
					{
						cout << "eUT: found malformed residue U." << endl;
						exit(EXIT_FAILURE);
					}
					
					flags_res = ATE_resn_na_RU;
					
					atmr_20->atmtp_E = ATE_atmn_na_N1;
					atmr_21->atmtp_E = ATE_atmn_na_C2;
					atmr_22->atmtp_E = ATE_atmn_na_N3;
					htab_22[0]->atmtp_E = ATE_atmn_na_H3;
					atmr_23->atmtp_E = ATE_atmn_na_C4;
					atmr_24->atmtp_E = ATE_atmn_na_C5;
					htab_24[0]->atmtp_E = ATE_atmn_na_H5;
					atmr_25->atmtp_E = ATE_atmn_na_C6;
					htab_25[0]->atmtp_E = ATE_atmn_na_H6;
					atmr_40->atmtp_E = ATE_atmn_na_O2;
					atmr_41->atmtp_E = ATE_atmn_na_O4;
				}
				
				// finally, set also the chn/res parts of the type information...
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
				
				for (iter_al it1 = resR[0];it1 != resR[1];it1++)
				{
					if ((* it1).atmtp_E != NOT_DEFINED)
					{
						(* it1).atmtp_E |= (flags_chn | flags_res);
					}
				}
			}
		}
	}
	}
	// ready...
	// ready...
	// ready...
	
	return errors;
}

// this will in fact set the atom types (atom::atmtp_s) as well as charges, no matter what the name is...
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

i32u default_tables::e_UpdateCharges(setup * su)
{
	i32u errors = 0;
	model * mdl = su->GetModel();
	
	// do the operation atom-by-atom; this might be slow, but is also flexible,
	// making it easier to adapt in possible changes in parameter files in future.
	
	for (iter_al it1 = mdl->GetAtomsBegin();it1 != mdl->GetAtomsEnd();it1++)
	{
		if ((* it1).atmtp_E == NOT_DEFINED) continue;
		
		// check H2O and other solvents/ions...
		// check H2O and other solvents/ions...
		// check H2O and other solvents/ions...
		
		if (((* it1).atmtp_E & ATE_type_mask) == ATE_type_XT)
		{
			switch ((* it1).atmtp_E & ATE_atmn_mask)
			{
				case ATE_atmn_xt_Hh2o:	(* it1).charge = +0.417; strcpy((* it1).atmtp_s, "Hw"); break;
				case ATE_atmn_xt_Oh2o:	(* it1).charge = -0.834; strcpy((* it1).atmtp_s, "Ow"); break;
			}
		}
		
		// check amino/nucleic acids...
		// check amino/nucleic acids...
		// check amino/nucleic acids...
		
		static const char aafn1[] = "amber/all_amino02.in";
		static const char aafn2[] = "amber/all_aminont02.in";
		static const char aafn3[] = "amber/all_aminoct02.in";
		static const char nafn1[] = "amber/all_nuc02.in";
		
		const char * filename = NULL;
		if (((* it1).atmtp_E & ATE_type_mask) == ATE_type_AA) filename = aafn1;
		if (((* it1).atmtp_E & ATE_type_mask) == ATE_type_AANT) filename = aafn2;
		if (((* it1).atmtp_E & ATE_type_mask) == ATE_type_AACT) filename = aafn3;
		if (((* it1).atmtp_E & ATE_type_mask) == ATE_type_NA) filename = nafn1;
		if (((* it1).atmtp_E & ATE_type_mask) == ATE_type_NA5T) filename = nafn1;
		if (((* it1).atmtp_E & ATE_type_mask) == ATE_type_NA3T) filename = nafn1;
		
		if (filename != NULL)
		{
			const char * resname = NULL;
			for (i32s n1 = 0;n1 < RES_NAME_TAB_SIZE;n1++)
			{
				if (res_name_tab[n1].id != ((* it1).atmtp_E & (ATE_type_mask | ATE_resn_mask))) continue;
				
				resname = res_name_tab[n1].name;
				break;
			}
			
			if (!resname)
			{
				cout << "ERROR : could not determine resname for 0x" << hex << (* it1).atmtp_E << "." << endl;
				exit(EXIT_FAILURE);
			}
			
			const char * atmname = NULL;
			for (i32s n1 = 0;n1 < ATM_NAME_TAB_SIZE;n1++)
			{
				if (atm_name_tab[n1].id != ((* it1).atmtp_E & ATE_atmn_mask)) continue;
				
				atmname = atm_name_tab[n1].name;
				break;
			}
			
			if (!atmname)
			{
				cout << "ERROR : could not determine atmname for 0x" << hex << (* it1).atmtp_E << "." << endl;
				exit(EXIT_FAILURE);
			}
			
			// open the file and find the matching residue...
			
			ifstream in_file; char txtbuff[1024];
			model::OpenLibDataFile(in_file, false, filename);
			
			in_file.getline(txtbuff, sizeof(txtbuff));	// discard the first...
			in_file.getline(txtbuff, sizeof(txtbuff));	// ...two lines!!!
			
			// the records start with a residue name written as text, followed by a blank line.
			// then the first string of the line is the residue symbol that should match the one we are looking for.
			// the records end with a line only containing the "DONE" keyword. keyword "STOP" means end of file.
			
			while (true)
			{
				in_file.getline(txtbuff, sizeof(txtbuff));		// residue name.
			//	cout << "DEBUG: now processing record : " << txtbuff << endl;
				
				in_file.getline(txtbuff, sizeof(txtbuff));		// blank line.
				
				// at next line, we should either get the residue symbol, or "STOP" if the file ends.
				
				in_file >> txtbuff;
				if (!strcmp(txtbuff, "STOP"))
				{
					cout << "fatal error : end of file reached in " << filename << " in search of residue " << resname << "." << endl;
					exit(EXIT_FAILURE);
				}
				
				// exit the loop if the residue name matches.
				
				if (!strcmp(txtbuff, resname)) break;
				
				// otherwise, read in the data until "DONE" keyword is encountered.
				
				while (true)
				{
					in_file.getline(txtbuff, sizeof(txtbuff));
					txtbuff[4] = 0;		// truncate the lines since we only need to find "DONE".
					
					if (!strcmp(txtbuff, "DONE")) break;
					
					if (in_file.eof())
					{
						cout << "fatal error : unexpected end of file reached in " << filename << "." << endl;
						exit(EXIT_FAILURE);
					}
				}
			}
			
			// ok, if we get this far, we are at the beginning of the correct record!!!
			
		//	cout << "DEBUG: found correct record : " << txtbuff << endl;
			
			// now we can erase the next 6 lines; these include the 3 lines for dummy atoms.
			
			in_file.getline(txtbuff, sizeof(txtbuff));
			in_file.getline(txtbuff, sizeof(txtbuff));
			in_file.getline(txtbuff, sizeof(txtbuff));
			in_file.getline(txtbuff, sizeof(txtbuff));
			in_file.getline(txtbuff, sizeof(txtbuff));
			in_file.getline(txtbuff, sizeof(txtbuff));
			
			// the next thing to do, is to look for the correct atom record.
			// we need to keep track of atom record indexing, in order to get the correct charge from charge table.
			
			i32s record_index = 4;
			
			while (true)
			{
				i32s index = NOT_DEFINED;
				in_file >> index;
				
				if (record_index != index)
				{
					cout << "fatal error : record index mismatch : " << record_index << " != " << index << " (" << resname << "/" << atmname << " 0x" << hex << (* it1).atmtp_E << ")." << endl;
					exit(EXIT_FAILURE);
				}
				
				// read in the atomname, and if it matches, exit the loop.
				
				in_file >> txtbuff;
				if (!strcmp(txtbuff, atmname)) break;
				
				// otherwise, just dump the line and cycle record_index.
				
				in_file.getline(txtbuff, sizeof(txtbuff));
				record_index++;
			}
			
			// now read in the correct atomtype, and store it.
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			
			in_file >> txtbuff;
			if (strlen(txtbuff) > 2)
			{
				cout << "fatal error : atomtype string overflow : " << txtbuff << "." << endl;
				exit(EXIT_FAILURE);
			}
			
			strcpy((* it1).atmtp_s, txtbuff);
			cout << "DEBUG: stored atomtype string : " << txtbuff << endl;
			
			// now find the CHARGE keyword, and pick the charge from charge table using stored record_index.
			
			record_index -= 4;
			i32s charge_row = record_index / 5;
			i32s charge_ind = (record_index % 5) + 1;
			
			while (true)
			{
				in_file.getline(txtbuff, sizeof(txtbuff));
				txtbuff[6] = 0;		// truncate the lines since we only need to find "CHARGE".
				
				if (!strcmp(txtbuff, "CHARGE")) break;
				
				if (in_file.eof())
				{
					cout << "fatal error : unexpected end of file reached in " << filename << "." << endl;
					exit(EXIT_FAILURE);
				}
			}
			
			f64 charge = 0.0;
			for (i32s n1 = 0;n1 < charge_row;n1++) in_file.getline(txtbuff, sizeof(txtbuff));
			for (i32s n1 = 0;n1 < charge_ind;n1++) in_file >> charge;
			
			if (charge < -2.0 || charge > +2.0)
			{
				cout << "fatal error : charge overflow : " << charge << "." << endl;
				exit(EXIT_FAILURE);
			}
			
			(* it1).charge = charge;
			in_file.close();
		}
	}
	
	return errors;
}

bool default_tables::e_Init(eng1_mm * eng, mm_default_bt1 * ref, i32s bt)
{
	atom ** atmtab = eng->GetSetup()->GetMMAtoms();
	
	const char * tp_string[2] = { NULL, NULL };
	for (i32s n1 = 0;n1 < 2;n1++)
	{
		tp_string[n1] = atmtab[ref->atmi[n1]]->atmtp_s;
		if (tp_string[n1][0] == 0) return false;
	}
	
	// test for built-in parameters (solvents etc)...
	
	bool builtin_H2O = false;
	if (!strcmp(tp_string[0], "Ow") && !strcmp(tp_string[1], "Hw")) builtin_H2O = true;
	if (!strcmp(tp_string[1], "Ow") && !strcmp(tp_string[0], "Hw")) builtin_H2O = true;
	if (builtin_H2O && bt == BONDTYPE_SINGLE)
	{
		ref->opt = 0.957 * 0.1;		// convert Å -> nm
		ref->fc = 553.0 * 418.68;	// convert (cal -> J) / (Å -> nm) ^ 2
		return true;
	}
	
	// open the file and find the matching parameters...
	
	ifstream in_file; char txtbuff[1024];
	model::OpenLibDataFile(in_file, false, "amber/parm99.dat");
	
	// the tables are separated by "empty lines".
	// search the beginning of bond-stretching parameters (need to cross 1 empty lines).
	
	for (i32s empty_lines = 0;empty_lines < 1;empty_lines++)
	{
		while (true)
		{
			in_file.getline(txtbuff, sizeof(txtbuff));
			
			bool is_empty = true;
			for (i32u n1 = 0;n1 < strlen(txtbuff) && is_empty;n1++)
			{
				bool is_whitespace = false;
				if (txtbuff[n1] == ' ') is_whitespace = true;
				if (txtbuff[n1] == '\t') is_whitespace = true;
				if (!is_whitespace) is_empty = false;
			}
			
			if (is_empty) break;
		}
	}
	
	in_file.getline(txtbuff, sizeof(txtbuff));	// remove the line : C   H   HO  N   NA  NB  NC...
	
	while (true)
	{
		char namestring[32]; namestring[5] = 0;
		for (i32s n1 = 0;n1 < 5;n1++) namestring[n1] = in_file.get();
		
		if (namestring[2] != '-')
		{
			cout << "ERROR e_Init(bt1) : no params found for " << tp_string[0] << "-" << tp_string[1] << "." << endl;
			exit(EXIT_FAILURE);
		}
		
		char name1[16];
		name1[0] = namestring[0]; name1[1] = namestring[1];
		name1[2] = 0; if (name1[1] == ' ') name1[1] = 0;
		
		char name2[16];
		name2[0] = namestring[3]; name2[1] = namestring[4];
		name2[2] = 0; if (name2[1] == ' ') name2[1] = 0;
		
		bool match = false;
		for (i32s dd = 0;dd < 2;dd++)
		{
			bool m1 = false;
			if (!strcmp(name1, tp_string[dd == 0 ? 0 : 1])) m1 = true;
			
			bool m2 = false;
			if (!strcmp(name2, tp_string[dd == 0 ? 1 : 0])) m2 = true;
			
			if (m1 && m2)
			{
				match = true;
				break;
			}
		}
		
		if (match) break;
		else in_file.getline(txtbuff, sizeof(txtbuff));
	}
	
	f64 fc; f64 opt;
	in_file >> fc >> opt;
	in_file.close();
	
	ref->opt = opt * 0.1;		// convert Å -> nm
	ref->fc = fc * 418.68;		// convert (cal -> J) / (Å -> nm) ^ 2
	return true;
}

bool default_tables::e_Init(eng1_mm * eng, mm_default_bt2 * ref, i32s * bt)
{
	atom ** atmtab = eng->GetSetup()->GetMMAtoms();
	
	const char * tp_string[3] = { NULL, NULL, NULL };
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		tp_string[n1] = atmtab[ref->atmi[n1]]->atmtp_s;
		if (tp_string[n1][0] == 0) return false;
	}
	
	// test for built-in parameters (solvents etc)...
	
	bool builtin_H2O = false;
	if (!strcmp(tp_string[0], "Hw") && !strcmp(tp_string[1], "Ow") && !strcmp(tp_string[2], "Hw")) builtin_H2O = true;
	if (builtin_H2O && bt[0] == BONDTYPE_SINGLE && bt[1] == BONDTYPE_SINGLE)
	{
		ref->opt = 104.5 * M_PI / 180.0;	// convert deg -> rad
		ref->fc = 0.020 * 13744.5;		// convert (cal -> J) / (deg -> rad) ^ 2
		return true;
	}
	
	// open the file and find the matching parameters...
	
	ifstream in_file; char txtbuff[1024];
	model::OpenLibDataFile(in_file, false, "amber/parm99.dat");
	
	// the tables are separated by "empty lines".
	// search the beginning of bond-stretching parameters (need to cross 2 empty lines).
	
	for (i32s empty_lines = 0;empty_lines < 2;empty_lines++)
	{
		while (true)
		{
			in_file.getline(txtbuff, sizeof(txtbuff));
			
			bool is_empty = true;
			for (i32u n1 = 0;n1 < strlen(txtbuff) && is_empty;n1++)
			{
				bool is_whitespace = false;
				if (txtbuff[n1] == ' ') is_whitespace = true;
				if (txtbuff[n1] == '\t') is_whitespace = true;
				if (!is_whitespace) is_empty = false;
			}
			
			if (is_empty) break;
		}
	}
	
	while (true)
	{
		char namestring[32]; namestring[8] = 0;
		for (i32s n1 = 0;n1 < 8;n1++) namestring[n1] = in_file.get();
		
		if (namestring[2] != '-' || namestring[5] != '-')
		{
			cout << "ERROR e_Init(bt2) : no params found for " << tp_string[0] << "-" << tp_string[1] << "-" << tp_string[2] << "." << endl;
			exit(EXIT_FAILURE);
		}
		
		char name1[16];
		name1[0] = namestring[0]; name1[1] = namestring[1];
		name1[2] = 0; if (name1[1] == ' ') name1[1] = 0;
		
		char name2[16];
		name2[0] = namestring[3]; name2[1] = namestring[4];
		name2[2] = 0; if (name2[1] == ' ') name2[1] = 0;
		
		char name3[16];
		name3[0] = namestring[6]; name3[1] = namestring[7];
		name3[2] = 0; if (name3[1] == ' ') name3[1] = 0;
		
		bool match = false;
		for (i32s dd = 0;dd < 2;dd++)
		{
			bool m1 = false;
			if (!strcmp(name1, tp_string[dd == 0 ? 0 : 2])) m1 = true;
			
			bool m2 = false;
			if (!strcmp(name2, tp_string[1])) m2 = true;

			bool m3 = false;
			if (!strcmp(name3, tp_string[dd == 0 ? 2 : 0])) m3 = true;
			
			if (m1 && m2 && m3)
			{
				match = true;
				break;
			}
		}
		
		if (match) break;
		else in_file.getline(txtbuff, sizeof(txtbuff));
	}
	
	f64 fc; f64 opt;
	in_file >> fc >> opt;
	in_file.close();
	
	ref->opt = opt * M_PI / 180.0;	// convert deg -> rad
	ref->fc = fc * 4.1868;		// convert cal -> J
	return true;
}

bool default_tables::e_Init(eng1_mm * eng, mm_default_bt3 * ref, i32s * bt)
{
	atom ** atmtab = eng->GetSetup()->GetMMAtoms();
	
	const char * tp_string[4] = { NULL, NULL, NULL, NULL };
	for (i32s n1 = 0;n1 < 4;n1++)
	{
		tp_string[n1] = atmtab[ref->atmi[n1]]->atmtp_s;
		if (tp_string[n1][0] == 0) return false;
	}
	
	// open the file and find the matching parameters...
	
	ifstream in_file; char txtbuff[1024];
	model::OpenLibDataFile(in_file, false, "amber/parm99.dat");
	
	// the tables are separated by "empty lines".
	// search the beginning of bond-stretching parameters (need to cross 3 empty lines).
	
	for (i32s empty_lines = 0;empty_lines < 3;empty_lines++)
	{
		while (true)
		{
			in_file.getline(txtbuff, sizeof(txtbuff));
			
			bool is_empty = true;
			for (i32u n1 = 0;n1 < strlen(txtbuff) && is_empty;n1++)
			{
				bool is_whitespace = false;
				if (txtbuff[n1] == ' ') is_whitespace = true;
				if (txtbuff[n1] == '\t') is_whitespace = true;
				if (!is_whitespace) is_empty = false;
			}
			
			if (is_empty) break;
		}
	}
	
	char namestring[32];
	while (true)
	{
		namestring[11] = 0;
		for (i32s n1 = 0;n1 < 11;n1++) namestring[n1] = in_file.get();
		
		if (namestring[2] != '-' || namestring[5] != '-' || namestring[8] != '-')
		{
			cout << "ERROR e_Init(bt3) : no params found for " << tp_string[0] << "-" << tp_string[1] << "-" << tp_string[2] << "." << endl;
			exit(EXIT_FAILURE);
		}
		
		char name1[16];
		name1[0] = namestring[0]; name1[1] = namestring[1];
		name1[2] = 0; if (name1[1] == ' ') name1[1] = 0;
		
		char name2[16];
		name2[0] = namestring[3]; name2[1] = namestring[4];
		name2[2] = 0; if (name2[1] == ' ') name2[1] = 0;
		
		char name3[16];
		name3[0] = namestring[6]; name3[1] = namestring[7];
		name3[2] = 0; if (name3[1] == ' ') name3[1] = 0;
		
		char name4[16];
		name4[0] = namestring[9]; name4[1] = namestring[10];
		name4[2] = 0; if (name4[1] == ' ') name4[1] = 0;
		
		bool match = false;
		for (i32s dd = 0;dd < 2;dd++)
		{
			bool m1 = false; if (name1[0] == 'X') m1 = true;
			if (!strcmp(name1, tp_string[dd == 0 ? 0 : 3])) m1 = true;
			
			bool m2 = false;
			if (!strcmp(name2, tp_string[dd == 0 ? 1 : 2])) m2 = true;
			
			bool m3 = false;
			if (!strcmp(name3, tp_string[dd == 0 ? 2 : 1])) m3 = true;
			
			bool m4 = false; if (name4[0] == 'X') m4 = true;
			if (!strcmp(name4, tp_string[dd == 0 ? 3 : 0])) m4 = true;
			
			if (m1 && m2 && m3 && m4)
			{
				match = true;
				break;
			}
		}
		
		if (match) break;
		else in_file.getline(txtbuff, sizeof(txtbuff));
	}
	
	// in parm99.dat many terms for the same torsion are split into several lines;
	// negative value of "n" is a sign for this. NO FURTHER TYPE CHECKS ARE DONE!!!
	// also it is assumed that "gamma" can only have values of either 0 or 180.
	
	f64 fc[4] = { 0.0, 0.0, 0.0, 0.0 };
	while (true)
	{
		i32s nop; f64 v; f64 gamma; f64 n;
		in_file >> nop >> v >> gamma >> n;
		
		i32u z = abs((i32s) n);
		if (z < 1) z = 1; z -= 1;
		if (gamma > 90.0) v = -v;
		fc[z] = v / (f64) nop;
		
		if (n > 0.5) break;
		
		in_file.getline(txtbuff, sizeof(txtbuff));
		for (i32s n1 = 0;n1 < 11;n1++)
		{
			char tc = in_file.get();
			if (tc != namestring[n1])
			{
				cout << "ERROR : namestring does not match!" << endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	
	in_file.close();
	
	ref->fc1 = fc[0] * 4.1868;	// convert cal -> J
	ref->fc2 = fc[1] * 4.1868;	// convert cal -> J
	ref->fc3 = fc[2] * 4.1868;	// convert cal -> J
	ref->fc4 = fc[3] * 4.1868;	// convert cal -> J
	return true;
}

bool default_tables::e_Init(eng1_mm *, mm_default_bt4 * ref, i32s *)
{
	// out-of-plane terms are not used in AMBER...
	// out-of-plane terms are not used in AMBER...
	// out-of-plane terms are not used in AMBER...
	
	ref->opt = 0.0;
	ref->fc = 0.0;
	return true;		// return true here, since all terms are added anyway...
}

bool default_tables::e_Init(eng1_mm * eng, mm_default_nbt1 * ref, bool is14)
{
	// the values in the table below are taken (unless marked otherwise) from Table 14 of:
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	// Cornell WD, Cieplak P, Bayly CI, Gould IR, Merz KM, Ferguson DM, Spellmeyer DC,
	// Fox T, Caldwell JW, Kollman PA : "A Second Generation Force Field for the Simulation
	// of Proteins, Nucleic Acids, and Organic Molecules" J. Am. Chem. Soc. 117, 5179-
	// 5197 (1995)
	
	const int pt_size = 2 + 44;
	const e_vdw_param pt[pt_size] =
	{
		{ "Hw", 0.9000, 0.0157 },		// modified...
		{ "Ow", 1.5500, 0.1520 },		// modified...
		
		{ "C",  1.9080, 0.0860 },
		{ "CA", 1.9080, 0.0860 },
		{ "CB", 1.9080, 0.0860 },	// copy of "CA".
		{ "CC", 1.9080, 0.0860 },	// copy of "CA".
		{ "CD", 1.9080, 0.0860 },	// copy of "CA".
		{ "CK", 1.9080, 0.0860 },	// copy of "CA".
		{ "CM", 1.9080, 0.0860 },
		{ "CN", 1.9080, 0.0860 },	// copy of "CM".
		{ "CQ", 1.9080, 0.0860 },	// copy of "CM".
		{ "CR", 1.9080, 0.0860 },	// copy of "CM".
		{ "CT", 1.9080, 0.1094 },
		{ "CV", 1.9080, 0.0860 },	// copy of "CM".
		{ "CW", 1.9080, 0.0860 },	// copy of "CM".
		{ "C*", 1.9080, 0.0860 },	// copy of "CM".
		{ "CY", 1.9080, 0.0860 },	// copy of "CM".
		{ "CZ", 1.9080, 0.0860 },	// copy of "CM".
		{ "F",  1.7500, 0.0610 },
		{ "H",  0.6000, 0.0157 },
		{ "H1", 1.3870, 0.0157 },
		{ "H2", 1.2870, 0.0157 },
		{ "H3", 1.1870, 0.0157 },
		{ "H4", 1.4090, 0.0150 },
		{ "H5", 1.3590, 0.0150 },
		{ "HA", 1.4590, 0.0150 },
		{ "HC", 1.4870, 0.0157 },
		{ "HO", 0.9000, 0.0157 },		// modified...
		{ "HP", 1.1000, 0.0157 },
		{ "HS", 0.6000, 0.0157 },
		{ "N",  1.8240, 0.1700 },
		{ "NA", 1.8240, 0.1700 },	// copy of "N".
		{ "NB", 1.8240, 0.1700 },	// copy of "N".
		{ "NC", 1.8240, 0.1700 },	// copy of "N".
		{ "N2", 1.8240, 0.1700 },	// copy of "N".
		{ "N3", 1.8750, 0.1700 },
		{ "NT", 1.8750, 0.1700 },	// copy of "N3".
		{ "N*", 1.8750, 0.1700 },	// copy of "N3".
		{ "NY", 1.8750, 0.1700 },	// copy of "N3".
		{ "O",  1.6612, 0.2100 },
		{ "O2", 1.6612, 0.2100 },
		{ "OH", 1.7210, 0.2104 },
		{ "OS", 1.6837, 0.1700 },
		{ "P",  2.1000, 0.2000 },
		{ "S",  2.0000, 0.2500 },
		{ "SH", 2.0000, 0.2500 }
	};
	
	atom ** atmtab = eng->GetSetup()->GetMMAtoms();
	
	const char * tp_string[2] = { NULL, NULL }; bool skip = false;
	for (i32s n1 = 0;n1 < 2;n1++)
	{
		tp_string[n1] = atmtab[ref->atmi[n1]]->atmtp_s;
		if (tp_string[n1][0] == 0)
		{
			atmtab[ref->atmi[n1]]->flags |= ATOMFLAG_SELECTED;	// debug...
			skip = true;
		}
	}
	
	if (skip) return false;
	
	i32s index[2] = { NOT_DEFINED, NOT_DEFINED };
	for (i32s n1 = 0;n1 < 2;n1++)
	{
		for (i32s n2 = 0;n2 < pt_size;n2++)
		{
			if (!strcmp(tp_string[n1], pt[n2].name))
			{
				index[n1] = n2;
				break;
			}
		}
		
		if (index[n1] == NOT_DEFINED)
		{
			cout << "ERROR e_Init(nbt1) : no params found for " << tp_string[n1] << "." << endl;
			exit(EXIT_FAILURE);
		}
	}
	
	// we will convert units to [nm] and [kJ/mol]...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	f64 energy = sqrt(pt[index[0]].param_e * pt[index[1]].param_e) * 4.1868;	// convert cal -> J
	f64 optdist = (pt[index[0]].param_r + pt[index[1]].param_r) * 0.1;		// convert Å -> nm
	
	f64 charge1 = atmtab[ref->atmi[0]]->charge;
	f64 charge2 = atmtab[ref->atmi[1]]->charge;
	ref->qq = 138.9354518 * charge1 * charge2;
// DOKUMENTOI MUUNNOSKERROIN!!!
// DOKUMENTOI MUUNNOSKERROIN!!!
// DOKUMENTOI MUUNNOSKERROIN!!!
	
	if (is14)
	{
		energy *= 0.50;		// see "Methods" of the above reference...
		ref->qq *= 0.83;	// see "Methods" of the above reference...
	}
	
	f64 tmp1 = optdist * pow(1.0 * energy, 1.0 / 12.0);
	f64 tmp2 = optdist * pow(2.0 * energy, 1.0 / 6.0);
	
/*	if (ref->qq < 0.0)
	{
		cout << "ALKUP = " << tmp1 << " " << tmp2 << endl;
		
		f64 tmp9 = 0.75 * ref->qq;
		f64 tmp5 = -6.0 / tmp2 * pow(optdist / tmp2, -7.0);
		f64 tmp6 = +1.0 / tmp9 * pow(optdist / tmp9, -2.0);
		f64 tmp7 = -12.0 * pow(optdist, -13.0) / (tmp5 + tmp6);
		tmp1 = pow(tmp7, -1.0 / 12.0);
		
		cout << "VALMIS = " << tmp1 << " " << tmp2 << endl;
		int qqq; cin >> qqq;
	}	*/
	
	ref->kr = tmp1;
	ref->kd = tmp2;
	
	return true;
}

/*################################################################################################*/

// eof
