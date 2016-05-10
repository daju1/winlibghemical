// TAB_MM_TRIPOS52.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "tab_mm_tripos52.h"

#include "typerule.h"
#include "utility.h"

#include <fstream>
#include <strstream>
#include <iomanip>
using namespace std;

/*################################################################################################*/

tripos52_tables * tripos52_tables::instance = NULL;
singleton_cleaner<tripos52_tables> tripos52_cleaner(tripos52_tables::GetInstance());

tripos52_tables::tripos52_tables(void)
{
	ostr = NULL;		// do not print output.
//	ostr = & cout;		// print output.
	
	const char * fn;
	
	ifstream file;
	file.unsetf(ios::dec | ios::oct | ios::hex);
	
	char buffer[1024];
	
/*##############################################*/
/*##############################################*/
	
	fn = "param_mm/tripos52/atomtypes.txt";
	model::OpenLibDataFile(file, false, fn);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			tripos52_at newat;
			file >> newat.atomtype;
			
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
	
	fn = "param_mm/tripos52/parameters1.txt";
	model::OpenLibDataFile(file, false, fn);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			tripos52_bs tmp; char bt[16];
			file >> tmp.atmtp[0] >> tmp.atmtp[1] >> bt;
			file >> tmp.param[0] >> tmp.param[1];
			
			tmp.bndtp = bondtype(bt[0]);
			bs_vector.push_back(tmp);
		}
		
		file.getline(buffer, sizeof(buffer));
	}
	
	file.close();
	
	if (ostr != NULL) (* ostr) << "found " << bs_vector.size() << " bs-terms." << endl;
	
/*##############################################*/
/*##############################################*/
	
	fn = "param_mm/tripos52/parameters2.txt";
	model::OpenLibDataFile(file, false, fn);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			tripos52_ab tmp; char bt[16];
			file >> tmp.atmtp[0] >> tmp.atmtp[1] >> tmp.atmtp[2] >> bt;
			file >> tmp.param[0] >> tmp.param[1];
			
			for (i32s n1 = 0;n1 < 2;n1++) tmp.bndtp[n1] = bondtype(bt[n1]);
			ab_vector.push_back(tmp);
		}
		
		file.getline(buffer, sizeof(buffer));
	}
	
	file.close();
	
	if (ostr != NULL) (* ostr) << "found " << ab_vector.size() << " ab-terms." << endl;
	
/*##############################################*/
/*##############################################*/
	
	fn = "param_mm/tripos52/parameters3.txt";
	model::OpenLibDataFile(file, false, fn);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			tripos52_tr tmp; char bt[16];
			file >> tmp.atmtp[0] >> tmp.atmtp[1] >> tmp.atmtp[2] >> tmp.atmtp[3] >> bt;
			file >> tmp.k >> tmp.s;
			
			for (i32s n1 = 0;n1 < 3;n1++) tmp.bndtp[n1] = bondtype(bt[n1]);
			tr_vector.push_back(tmp);
		}
		
		file.getline(buffer, sizeof(buffer));
	}
	
	file.close();
	
	if (ostr != NULL) (* ostr) << "found " << tr_vector.size() << " tr-terms." << endl;
	
/*##############################################*/
/*##############################################*/
	
	fn = "param_mm/tripos52/parameters4.txt";
	model::OpenLibDataFile(file, false, fn);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			tripos52_lj tmp;
			file >> tmp.type;
			file >> tmp.r >> tmp.k;
			
			lj_vector.push_back(tmp);
		}
		
		file.getline(buffer, sizeof(buffer));
	}
	
	file.close();
	
	if (ostr != NULL) (* ostr) << "found " << lj_vector.size() << " lj-datasets." << endl;
	
/*##############################################*/
/*##############################################*/
	
	fn = "param_mm/tripos52/parameters5.txt";
	model::OpenLibDataFile(file, false, fn);
	
	if (ostr != NULL) (* ostr) << "reading file \"" << fn << "\": ";
	
	while (file.peek() != '#')		// #end
	{
		if (file.peek() == '0')		// 0x????
		{
			tripos52_ci tmp; char bt[16];
			file >> tmp.atmtp[0] >> tmp.atmtp[1] >> bt;
			file >> tmp.delta;
			
			tmp.bndtp = bondtype(bt[0]);
			ci_vector.push_back(tmp);
		}
		
		file.getline(buffer, sizeof(buffer));
	}
	
	file.close();
	
	if (ostr != NULL) (* ostr) << "found " << ci_vector.size() << " ci-datasets." << endl;
}

tripos52_tables::~tripos52_tables(void)
{
	for (i32u n1 = 0;n1 < at_vector.size();n1++)
	{
		delete at_vector[n1].tr;
		delete[] at_vector[n1].description;
	}
}

tripos52_tables * tripos52_tables::GetInstance(void)
{
	if (instance != NULL) return instance;
	else return (instance = new tripos52_tables());
}

void tripos52_tables::PrintAllTypeRules(ostream & p1)
{
	for (i32u n1 = 0;n1 < at_vector.size();n1++)
	{
		p1 << (n1 + 1) << ": 0x" << hex << setw(4) << setfill('0') << at_vector[n1].atomtype << dec;
		p1 << " (" << (* at_vector[n1].tr) << ") \"" << at_vector[n1].description << "\"" << endl;
	}
	
	p1 << at_vector.size() << " entries." << endl;
}

i32u tripos52_tables::UpdateTypes(setup * su)
{
	if (ostr != NULL) (* ostr) << "setting up atom types and formal charges..." << endl;
	
	i32u errors = 0;
	model * mdl = su->GetModel();
	
	for (iter_al it1 = mdl->GetAtomsBegin();it1 != mdl->GetAtomsEnd();it1++)
	{
		if (ostr != NULL && !((* it1).index % 10)) (* ostr) << "*" << flush;
		
		i32u at_range[2];
		
		at_range[0] = 0;
		while (true)
		{
			if (at_range[0] == at_vector.size()) break;
			if ((at_vector[at_range[0]].atomtype >> 8) == (* it1).el.GetAtomicNumber()) break;
			
			at_range[0]++;
		}
		
		at_range[1] = at_range[0];
		while (true)
		{
			if (at_range[1] == at_vector.size()) break;
			if ((at_vector[at_range[1]].atomtype >> 8) != (* it1).el.GetAtomicNumber()) break;
			
			at_range[1]++;
		}
		
		i32s index = NOT_DEFINED;
		for (i32u n1 = at_range[0];n1 < at_range[1];n1++)
		{
			bool flag = at_vector[n1].tr->Check(mdl, & (* it1), 0);
			if (flag) index = n1;
		}
		
		if (index != NOT_DEFINED)
		{
			(* it1).atmtp = at_vector[index].atomtype;
			
			// no formal charges defined in tripos5.2???
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			
			(* it1).charge = 0.0;
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

i32u tripos52_tables::UpdateCharges(setup * su)
{
	if (ostr != NULL) (* ostr) << "setting up partial charges..." << endl;
	
	i32u errors = 0;
	model * mdl = su->GetModel();
	
	for (iter_bl it1 = mdl->GetBondsBegin();it1 != mdl->GetBondsEnd();it1++)
	{
		f64 delta = tripos52_tables::GetInstance()->GetChargeInc(& (* it1));
		
		(* it1).atmr[0]->charge -= delta;
		(* it1).atmr[1]->charge += delta;
	}
	
	return errors;
}

f64 tripos52_tables::GetChargeInc(bond * ref)
{
	i32s atmtp[2];
	for (i32s n1 = 0;n1 < 2;n1++)
	{
		atmtp[n1] = ref->atmr[n1]->atmtp;
	}
	
	for (i32u n1 = 0;n1 < ci_vector.size();n1++)
	{
		if (ci_vector[n1].bndtp.GetValue() != ref->bt.GetValue()) continue;
		
		for (i32s dir = 0;dir < 2;dir++)
		{
			bool test1 = (ci_vector[n1].atmtp[0] == atmtp[dir]);
			bool test2 = (ci_vector[n1].atmtp[1] == atmtp[!dir]);
			
			if (test1 && test2)
			{
				if (!dir) return +ci_vector[n1].delta;
				else return -ci_vector[n1].delta;
			}
		}
	}
	
	if (ostr != NULL)
	{
		(* ostr) << "there was no record for the following ci: ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << atmtp[0] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << atmtp[1] << dec << " ";
		(* ostr) << ref->bt.GetValue() << endl;
	}
	
	return 0.0;
}

bool tripos52_tables::Init(eng1_mm * eng, mm_tripos52_bt1 * ref, i32s bt)
{
	atom ** atmtab = eng->GetSetup()->GetMMAtoms();
	
	i32s atmtp[2];
	for (i32s n1 = 0;n1 < 2;n1++)
	{
		atmtp[n1] = atmtab[ref->atmi[n1]]->atmtp;
	}
	
	for (i32u n1 = 0;n1 < bs_vector.size();n1++)
	{
		if (bs_vector[n1].bndtp.GetValue() != bt) continue;
		
		bool flag = false;
		for (i32s dir = 0;dir < 2;dir++)
		{
			bool test1 = (bs_vector[n1].atmtp[0] == atmtp[dir]);
			bool test2 = (bs_vector[n1].atmtp[1] == atmtp[!dir]);
			
			if (test1 && test2) flag = true;
			
			bool wc1 = (bs_vector[n1].atmtp[0] == 0xFFFF);
			bool wc2 = (bs_vector[n1].atmtp[1] == 0xFFFF);
			
			if (wc1 && test2) flag = true;
			if (test1 && wc2) flag = true;
			if (wc1 && wc2) flag = true;
			
			if (flag) break;
		}
		
		// we will convert units to [nm] and [kJ/mol]...
		
		if (flag)
		{
			ref->opt = 0.1 * bs_vector[n1].param[0];
			ref->fc = 418.68 * bs_vector[n1].param[1];
			
			return true;
		}
	}
	
	if (ostr != NULL)
	{
		(* ostr) << "unknown bst: ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << atmtp[0] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << atmtp[1] << dec << " ";
		(* ostr) << bt << endl;
	}
	
	ref->opt = 0.1 * 1.100;
	ref->fc = 418.68 * 500.0;
	
	return false;
}

bool tripos52_tables::Init(eng1_mm * eng, mm_tripos52_bt2 * ref, i32s * bt)
{
	atom ** atmtab = eng->GetSetup()->GetMMAtoms();
	
	i32s atmtp[3];
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		atmtp[n1] = atmtab[ref->atmi[n1]]->atmtp;
	}
	
	for (i32u n1 = 0;n1 < ab_vector.size();n1++)
	{
		if (ab_vector[n1].atmtp[1] != atmtp[1]) continue;
		
		// bondtype checking not yet implemented....
		// it will be basically similar to the above, but must be moved into the dir-loop!!!
		// it will be basically similar to the above, but must be moved into the dir-loop!!!
		// it will be basically similar to the above, but must be moved into the dir-loop!!!
		
		bool flag = false;
		for (i32s dir = 0;dir < 2;dir++)
		{
			i32s index[2];
			index[0] = (!dir ? 0 : 2);
			index[1] = (!dir ? 2 : 0);
			
			bool test1 = (ab_vector[n1].atmtp[0] == atmtp[index[0]]);
			bool test2 = (ab_vector[n1].atmtp[2] == atmtp[index[1]]);
			
			if (test1 && test2) flag = true;
			
			bool wc1 = (ab_vector[n1].atmtp[0] == 0xFFFF);
			bool wc2 = (ab_vector[n1].atmtp[2] == 0xFFFF);
			
			if (wc1 && test2) flag = true;
			if (test1 && wc2) flag = true;
			if (wc1 && wc2) flag = true;
			
			if (flag) break;
		}
		
		// we will convert units to [rad] and [kJ/mol]...
		
		if (flag)
		{
			ref->opt = M_PI * ab_vector[n1].param[0] / 180.0;
			ref->fc = 13744.5 * ab_vector[n1].param[1];
			
			return true;
		}
	}
	
	if (ostr != NULL)
	{
		(* ostr) << "unknown abn: " << hex;
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << atmtp[0] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << atmtp[1] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << atmtp[2] << dec << " ";
		(* ostr) << bt[0] << " " << bt[1] << endl;
	}
	
	ref->opt = M_PI * 120.0 / 180.0;
	ref->fc = 13744.5 * 0.020;
	
	return false;
}

bool tripos52_tables::Init(eng1_mm * eng, mm_tripos52_bt3 * ref, i32s * bt)
{
	atom ** atmtab = eng->GetSetup()->GetMMAtoms();
	
	i32s atmtp[4];
	for (i32s n1 = 0;n1 < 4;n1++)
	{
		atmtp[n1] = atmtab[ref->atmi[n1]]->atmtp;
	}
	
	for (i32u n1 = 0;n1 < tr_vector.size();n1++)
	{
		if (tr_vector[n1].bndtp[1].GetValue() != bt[1]) continue;

		// proper bondtype checking not yet implemented....
		// it will be basically similar to the above, but must be moved into the dir-loop!!!
		// it will be basically similar to the above, but must be moved into the dir-loop!!!
		// it will be basically similar to the above, but must be moved into the dir-loop!!!
		
		bool flag = false;
		for (i32s dir = 0;dir < 2;dir++)
		{
			i32s index[4];
			index[0] = (!dir ? 0 : 3);
			index[1] = (!dir ? 1 : 2);
			index[2] = (!dir ? 2 : 1);
			index[3] = (!dir ? 3 : 0);
			
			bool test1 = (tr_vector[n1].atmtp[0] == atmtp[index[0]]);
			bool test2 = (tr_vector[n1].atmtp[1] == atmtp[index[1]]);
			bool test3 = (tr_vector[n1].atmtp[2] == atmtp[index[2]]);
			bool test4 = (tr_vector[n1].atmtp[3] == atmtp[index[3]]);
			
			if (test1 && test2 && test3 && test4) flag = true;
			
			bool wc1 = (tr_vector[n1].atmtp[0] == 0xFFFF);
			bool wc2 = (tr_vector[n1].atmtp[3] == 0xFFFF);
			
			if (wc1 && test2 && test3 && test4) flag = true;
			if (test1 && test2 && test3 && wc2) flag = true;
			if (wc1 && test2 && test3 && wc2) flag = true;
			
			if (flag) break;
		}
		
		// we will convert units to [kJ/mol]...
		
		if (flag)
		{
			ref->k = 4.1868 * tr_vector[n1].k;
			ref->s = tr_vector[n1].s;
			
			return true;
		}
	}
	
	if (ostr != NULL)
	{
		(* ostr) << "unknown tor: " << hex;
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << atmtp[0] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << atmtp[1] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << atmtp[2] << dec << " ";
		(* ostr) << "0x" << hex << setw(4) << setfill('0') << atmtp[3] << dec << " ";
		(* ostr) << bt[0] << " " << bt[1] << " " << bt[2] << endl;
	}
	
	ref->k = 0.0;
	ref->s = +1.0;
	
	return false;
}

bool tripos52_tables::Init(eng1_mm * eng, mm_tripos52_nbt1 * ref, bool is14)
{
	atom ** atmtab = eng->GetSetup()->GetMMAtoms();
	
	i32s atmtp[2];
	for (i32s n1 = 0;n1 < 2;n1++)
	{
		atmtp[n1] = atmtab[ref->atmi[n1]]->atmtp;
	}
	
	i32u index[2];
	for (i32s n1 = 0;n1 < 2;n1++)
	{
		index[n1] = 0;
		
		while (index[n1] < lj_vector.size())
		{
			if (lj_vector[index[n1]].type == atmtp[n1]) break;
			else index[n1]++;
		}
		
		if (index[n1] == lj_vector.size())
		{
			if (ostr != NULL) (* ostr) << "bad atomtype!!! using hydrogen instead..." << endl;
			index[n1] = 0;
		}
	}
	
	// we will convert units to [nm] and [kJ/mol]...
	
	f64 energy = 4.1868 * sqrt(lj_vector[index[0]].k * lj_vector[index[1]].k);
	f64 optdist = 0.1 * (lj_vector[index[0]].r + lj_vector[index[1]].r);
	
	f64 charge1 = atmtab[ref->atmi[0]]->charge;
	f64 charge2 = atmtab[ref->atmi[1]]->charge;
	ref->qq = 138.9354518 * charge1 * charge2;
	
	if (is14)
	{
		energy *= 0.5;
		ref->qq *= 0.5;
	}
	
	ref->k1 = optdist * pow(1.0 * energy, 1.0 / 12.0);
	ref->k2 = optdist * pow(2.0 * energy, 1.0 / 6.0);
	
	return true;
}

/*################################################################################################*/

// eof
