// TAB_MM_PRMFIT.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "tab_mm_prmfit.h"

#include "typerule.h"
#include "utility.h"

#include <iomanip>
#include <fstream>
#include <strstream>
#include <algorithm>
using namespace std;

#define RAND_f64 ((f64) rand() / (f64) RAND_MAX - 0.5)

/*################################################################################################*/

prmfit_tables::prmfit_tables(const char * p1)
{
	path = new char[strlen(p1) + 1];
	strcpy(path, p1);
	
	ostr = & cout;		// print output.
//	ostr = NULL;		// do not print output.
	
	ifstream file;
	file.unsetf(ios::dec | ios::oct | ios::hex);
	
	char fn[1024];
	char buffer[1024];
	
/*##############################################*/
/*##############################################*/

	ostrstream fn_strt(fn, sizeof(fn));
	fn_strt << path << "/atomtypes.txt" << ends;
	file.open(fn, ios::in);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			prmfit_at newat;
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
			
			at2_vector.push_back(newat);
		}
		
		file.getline(buffer, sizeof(buffer));
	}
	
	file.close();
	
	if (ostr != NULL) (* ostr) << "found " << at2_vector.size() << " atomtypes." << endl;
	
/*##############################################*/
/*##############################################*/
	
	ostrstream fn_str1(fn, sizeof(fn));
	fn_str1 << path << "/parameters1.txt" << ends;
	file.open(fn, ios::in);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			prmfit_bs tmp; char bt[16];
			file >> tmp.atmtp[0] >> tmp.atmtp[1] >> bt;
			file >> tmp.opt >> tmp.fc >> tmp.ci;
			
			tmp.bndtp = bondtype(bt[0]);
			bs_vector.push_back(tmp);
		}
		
		file.getline(buffer, sizeof(buffer));
	}
	
	file.close();
	
	if (ostr != NULL) (* ostr) << "found " << bs_vector.size() << " bs-terms." << endl;
	
/*##############################################*/
/*##############################################*/
	
	ostrstream fn_str2(fn, sizeof(fn));
	fn_str2 << path << "/parameters2.txt" << ends;
	file.open(fn, ios::in);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			prmfit_ab tmp; char bt[16];
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
	
	ostrstream fn_str3(fn, sizeof(fn));
	fn_str3 << path << "/parameters3.txt" << ends;
	file.open(fn, ios::in);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			prmfit_tr tmp; char bt[16];
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

	ostrstream fn_str4(fn, sizeof(fn));
	fn_str4 << path << "/parameters4.txt" << ends;
	file.open(fn, ios::in);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			prmfit_op tmp; char bt[16];
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

prmfit_tables::~prmfit_tables(void)
{
	for (i32u n1 = 0;n1 < at2_vector.size();n1++)
	{
		delete at2_vector[n1].tr;
		delete[] at2_vector[n1].description;
	}
	
	delete[] path;
}

void prmfit_tables::PrintAllTypeRules(ostream & p1)
{
	for (i32u n1 = 0;n1 < at2_vector.size();n1++)
	{
		p1 << (n1 + 1) << ": 0x" << hex << setw(4) << setfill('0') << at2_vector[n1].atomtype << dec;
		p1 << " (" << (* at2_vector[n1].tr) << ") \"" << at2_vector[n1].description << "\"" << endl;
	}
	
	p1 << at2_vector.size() << " entries." << endl;
}

i32u prmfit_tables::UpdateTypes(setup * su)
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
			if (at_range[0] == at2_vector.size()) break;
			if ((at2_vector[at_range[0]].atomtype[0] >> 8) == (* it1).el.GetAtomicNumber()) break;
			
			at_range[0]++;
		}
		
		at_range[1] = at_range[0];
		while (true)
		{
			if (at_range[1] == at2_vector.size()) break;
			if ((at2_vector[at_range[1]].atomtype[0] >> 8) != (* it1).el.GetAtomicNumber()) break;
			
			at_range[1]++;
		}
		
		i32s index = NOT_DEFINED;
		for (i32u n1 = at_range[0];n1 < at_range[1];n1++)
		{
			bool flag = at2_vector[n1].tr->Check(mdl, & (* it1), 0);
			if (flag) index = n1;
			
			// above, the LAST matching type will be selected???
		}
		
		if (index != NOT_DEFINED)
		{
			(* it1).atmtp = at2_vector[index].atomtype[0];
			(* it1).charge = at2_vector[index].formal_charge;
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
	return errors;
}

i32u prmfit_tables::UpdateCharges(setup * su)
{
	if (ostr != NULL) (* ostr) << "setting up partial charges..." << endl;
	
	i32u errors = 0;
//	atom ** atmtab = su->GetMMAtoms();
	bond ** bndtab = su->GetMMBonds();
	
	for (i32s n1 = 0;n1 < su->GetMMBondCount();n1++)
	{
		prmfit_bs_query query; query.strict = false;
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
	
	return errors;
}

const prmfit_at * prmfit_tables::GetAtomType(i32s p1)
{
	i32u index = 0;
	while (index < at2_vector.size())
	{
		if (at2_vector[index].atomtype[0] == p1) return (& at2_vector[index]);
		else index++;
	}
	
	// could not find the requested type -> return a NULL pointer instead...
	
	return NULL;
}

void prmfit_tables::DoParamSearch(prmfit_bs_query * query)
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
}

void prmfit_tables::DoParamSearch(prmfit_ab_query * query)
{
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
	
	query->index = NOT_DEFINED;
	query->dir = false;
	
	query->opt = 2.10;
	query->fc = 250.0;
}

void prmfit_tables::DoParamSearch(prmfit_tr_query * query)
{
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
	
	query->index = NOT_DEFINED;
	query->dir = false;
	
	query->fc1 = 0.0;
	query->fc2 = 0.0;
	query->fc3 = 0.0;
}

void prmfit_tables::DoParamSearch(prmfit_op_query * query)
{
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
	
	query->index = NOT_DEFINED;
	
	query->opt = 0.0;
	query->fc = 0.0;
}

/*################################################################################################*/

// eof
