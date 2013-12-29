// ENG1_MM_TRIPOS52.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "eng1_mm_tripos52.h"

#include "eng1_mm.h"
#include "tab_mm_tripos52.h"

#include <algorithm>
#include <strstream>
using namespace std;

/*################################################################################################*/

eng1_mm_tripos52_bt::eng1_mm_tripos52_bt(setup * p1, i32u p2) : engine(p1, p2), eng1_mm(p1, p2)
{
	tripos52_tables::GetInstance()->UpdateTypes(GetSetup());
	tripos52_tables::GetInstance()->UpdateCharges(GetSetup());
	
	// set default sizes to containers. how optimal/useful this is after all?!?!?!?!?!
	
	bt1_vector.reserve(250);
	bt2_vector.reserve(500);
	bt3_vector.reserve(500);
	
	model * mdl = GetSetup()->GetModel();
	atom ** atmtab = GetSetup()->GetMMAtoms();
	
//	ostream * ostr = tripos52_tables::GetInstance()->ostr;
	ostream * ostr = & cout;
	
/*##############################################*/
/*##############################################*/

	// create bt1-terms...
	
	if (ostr != NULL) (* ostr) << "creating bt1-terms: ";
	i32s bt1_err = 0;
	
	for (iter_bl it1 = mdl->GetBondsBegin();it1 != mdl->GetBondsEnd();it1++)
	{
		mm_tripos52_bt1 newbt1;
		newbt1.atmi[0] = (* it1).atmr[0]->index;
		newbt1.atmi[1] = (* it1).atmr[1]->index;
		
		i32s bt = (* it1).bt.GetValue();
		
		(* it1).tmp_bt1_index = bt1_vector.size();	// the bond objects are modified here!!!!!
		
		bt1_err += !tripos52_tables::GetInstance()->Init(this, & newbt1, bt);
		bt1_vector.push_back(newbt1);
	}
	
	if (ostr != NULL)
	{
		(* ostr) << bt1_vector.size() << " terms, ";
		(* ostr) << bt1_err << " errors." << endl;
	}
	
	bt1data = new mm_bt1_data[bt1_vector.size()];
	
/*##############################################*/
/*##############################################*/
	
	// create bt2-terms...
	
	if (ostr != NULL) (* ostr) << "creating bt2-terms: ";
	i32s bt2_err = 0;
	{
	for (iter_al it1 = mdl->GetAtomsBegin();it1 != mdl->GetAtomsEnd();it1++)
	{
		if ((* it1).cr_list.size() < 2) continue;
		
		// central atom is known, now find all possible combinations of bonds...
		
		bool dir[2];
		iter_cl ita; iter_cl rnga[2];
		iter_cl itb; iter_cl rngb[2];
		
		rngb[1] = (* it1).cr_list.end();
		
		rnga[0] = (* it1).cr_list.begin();
		rnga[1] = rngb[1]; rnga[1]--;
		
		for (ita = rnga[0];ita != rnga[1];ita++)
		{
			rngb[0] = ita; rngb[0]++;
			dir[0] = (& (* it1) == (* ita).bndr->atmr[0]);
			
			for (itb = rngb[0];itb != rngb[1];itb++)
			{
				dir[1] = (& (* it1) == (* itb).bndr->atmr[0]);
				
				mm_tripos52_bt2 newbt2;
				newbt2.index1[0] = (* ita).bndr->tmp_bt1_index; newbt2.dir1[0] = dir[0];
				newbt2.index1[1] = (* itb).bndr->tmp_bt1_index; newbt2.dir1[1] = dir[1];
				
				newbt2.atmi[0] = bt1_vector[(* ita).bndr->tmp_bt1_index].get_atmi(1, dir[0]);
				newbt2.atmi[2] = bt1_vector[(* itb).bndr->tmp_bt1_index].get_atmi(1, dir[1]);
				
				newbt2.atmi[1] = bt1_vector[(* ita).bndr->tmp_bt1_index].get_atmi(0, dir[0]);
				
				i32s bt[2];
				bt[0] = (* ita).bndr->bt.GetValue();
				bt[1] = (* itb).bndr->bt.GetValue();
				
				bt2_err += !tripos52_tables::GetInstance()->Init(this, & newbt2, bt);
				bt2_vector.push_back(newbt2);
			}
		}
	}
	}
	
	if (ostr != NULL)
	{
		(* ostr) << bt2_vector.size() << " terms, ";
		(* ostr) << bt2_err << " errors." << endl;
	}
	
	bt2data = new mm_bt2_data[bt2_vector.size()];
	
/*##############################################*/
/*##############################################*/
	
	// create bt3-terms...

	if (ostr != NULL) (* ostr) << "creating bt3-terms: ";
	i32s bt3_err = 0;
	{
	for (iter_al it1 = mdl->GetAtomsBegin();it1 != mdl->GetAtomsEnd();it1++)
	{
		if ((* it1).cr_list.size() < 2) continue;
		
		for (iter_cl it2 = (* it1).cr_list.begin();it2 != (* it1).cr_list.end();it2++)
		{
			bool another = (& (* it1) == (* it2).bndr->atmr[0]);
			atom * atmr = (* it2).bndr->atmr[another];
			
			if (atmr->cr_list.size() < 2) continue;
			if (atmr->index > (* it1).index) continue;
			
			// central atoms are known, now find all possible combinations of bonds...
			
			vector<i32s> ind1a; vector<bool> dir1a;		// search for the 1st group...
			for (iter_cl it3 = (* it1).cr_list.begin();it3 != (* it1).cr_list.end();it3++)
			{
				if ((* it3) == (* it2)) continue;
				
				ind1a.push_back((* it3).bndr->tmp_bt1_index);
				dir1a.push_back(& (* it1) == (* it3).bndr->atmr[0]);
			}
			
			vector<i32s> ind2a; vector<bool> dir2a;
			for (i32u n1 = 0;n1 < ind1a.size();n1++)
			{
				i32s tmp1 = 0; i32s tmp2 = NOT_DEFINED;
				while (true)
				{
					i32s bt1[2] = { bt2_vector[tmp1].index1[0], bt2_vector[tmp1].index1[1] };
					i32s bt2[2] = { bt2_vector[tmp1].dir1[0], bt2_vector[tmp1].dir1[1] };
					
					if (bt1[0] == ind1a[n1] && bt2[0] == dir1a[n1] &&
						bt1[1] == (* it2).bndr->tmp_bt1_index && bt2[1] == another) tmp2 = true;
					if (bt1[1] == ind1a[n1] && bt2[1] == dir1a[n1] &&
						bt1[0] == (* it2).bndr->tmp_bt1_index && bt2[0] == another) tmp2 = false;
					
					if (tmp2 < 0) tmp1++; else break;
				}
				
				ind2a.push_back(tmp1);
				dir2a.push_back(tmp2);
			}
			
			vector<i32s> ind1b; vector<bool> dir1b;		// search for the 2nd group...
			{
			for (iter_cl it3 = atmr->cr_list.begin();it3 != atmr->cr_list.end();it3++)
			{
				if ((* it3) == (* it2)) continue;
				
				ind1b.push_back((* it3).bndr->tmp_bt1_index);
				dir1b.push_back(atmr == (* it3).bndr->atmr[0]);
			}
			}
			
			vector<i32s> ind2b; vector<bool> dir2b;
			for (i32u n1 = 0;n1 < ind1b.size();n1++)
			{
				i32s tmp1 = 0; i32s tmp2 = NOT_DEFINED;
				while (true)
				{
					i32s bt1[2] = { bt2_vector[tmp1].index1[0], bt2_vector[tmp1].index1[1] };
					i32s bt2[2] = { bt2_vector[tmp1].dir1[0], bt2_vector[tmp1].dir1[1] };
					
					if (bt1[0] == ind1b[n1] && bt2[0] == dir1b[n1] &&
						bt1[1] == (* it2).bndr->tmp_bt1_index && bt2[1] != another) tmp2 = false;
					if (bt1[1] == ind1b[n1] && bt2[1] == dir1b[n1] &&
						bt1[0] == (* it2).bndr->tmp_bt1_index && bt2[0] != another) tmp2 = true;
					
					if (tmp2 < 0) tmp1++; else break;
				}
				
				ind2b.push_back(tmp1);
				dir2b.push_back(tmp2);
			}
			
			// now finally create the terms!!!!!
			
			for (i32u n1 = 0;n1 < ind2a.size();n1++)
			{
				for (i32u n2 = 0;n2 < ind2b.size();n2++)
				{
					mm_tripos52_bt3 newbt3;
					newbt3.index2[0] = ind2a[n1];
					newbt3.index2[1] = ind2b[n2];
					
					newbt3.index1[0] = bt2_vector[newbt3.index2[0]].get_index(0, dir2a[n1]);
					newbt3.dir1[0] = bt2_vector[newbt3.index2[0]].get_dir(0, dir2a[n1]);
					
					newbt3.index1[1] = bt2_vector[newbt3.index2[0]].get_index(1, dir2a[n1]);
					newbt3.dir1[1] = bt2_vector[newbt3.index2[0]].get_dir(1, dir2a[n1]);
					
					newbt3.index1[2] = bt2_vector[newbt3.index2[1]].get_index(0, dir2b[n2]);
					newbt3.dir1[2] = bt2_vector[newbt3.index2[1]].get_dir(0, dir2b[n2]);
					
					newbt3.index1[3] = bt2_vector[newbt3.index2[1]].get_index(1, dir2b[n2]);
					newbt3.dir1[3] = bt2_vector[newbt3.index2[1]].get_dir(1, dir2b[n2]);
					
					newbt3.atmi[0] = bt1_vector[newbt3.index1[0]].get_atmi(1, newbt3.dir1[0]);
					newbt3.atmi[1] = bt1_vector[newbt3.index1[0]].get_atmi(0, newbt3.dir1[0]);
					newbt3.atmi[2] = bt1_vector[newbt3.index1[3]].get_atmi(0, newbt3.dir1[3]);
					newbt3.atmi[3] = bt1_vector[newbt3.index1[3]].get_atmi(1, newbt3.dir1[3]);
					
					// easiest way to get bondtypes is to find the bonds...
					// easiest way to get bondtypes is to find the bonds...
					// easiest way to get bondtypes is to find the bonds...
					
/*	bond tmpb1 = bond(index[newbt3.atmi[0]], index[newbt3.atmi[1]], bondtype());
	iter_bl itb1 = find(mdl->GetBondsBegin(), mdl->GetBondsEnd(), tmpb1);
	
	bond tmpb2 = bond(index[newbt3.atmi[1]], index[newbt3.atmi[2]], bondtype());
	iter_bl itb2 = find(mdl->GetBondsBegin(), mdl->GetBondsEnd(), tmpb2);
	
	bond tmpb3 = bond(index[newbt3.atmi[2]], index[newbt3.atmi[3]], bondtype());
	iter_bl itb3 = find(mdl->GetBondsBegin(), mdl->GetBondsEnd(), tmpb3);	*/
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
	bond tmpb1 = bond(atmtab[newbt3.atmi[0]], atmtab[newbt3.atmi[1]], bondtype());
	iter_bl itb1 = find(GetSetup()->GetModel()->GetBondsBegin(), GetSetup()->GetModel()->GetBondsEnd(), tmpb1);
	
	bond tmpb2 = bond(atmtab[newbt3.atmi[1]], atmtab[newbt3.atmi[2]], bondtype());
	iter_bl itb2 = find(GetSetup()->GetModel()->GetBondsBegin(), GetSetup()->GetModel()->GetBondsEnd(), tmpb2);
	
	bond tmpb3 = bond(atmtab[newbt3.atmi[2]], atmtab[newbt3.atmi[3]], bondtype());
	iter_bl itb3 = find(GetSetup()->GetModel()->GetBondsBegin(), GetSetup()->GetModel()->GetBondsEnd(), tmpb3);
	
					i32s bt[3];
					bt[0] = (* itb1).bt.GetValue();
					bt[1] = (* itb2).bt.GetValue();
					bt[2] = (* itb3).bt.GetValue();
					
	bt3_err += !tripos52_tables::GetInstance()->Init(this, & newbt3, bt);
//	if (newbt3.k == 0.0) continue;		// these have bad optimal geometries -> SKIP!!! constraints???

					newbt3.constraint = false;
					bt3_vector.push_back(newbt3);
				}
			}
		}
	}
	}
	if (ostr != NULL)
	{
		(* ostr) << bt3_vector.size() << " terms, ";
		(* ostr) << bt3_err << " errors." << endl;
	}
	
/*##############################################*/
/*##############################################*/

	// report possible errors...
	
	i32s total_err = bt1_err + bt2_err + bt3_err;
	if (total_err)
	{
		char mbuff1[256];
		ostrstream str1(mbuff1, sizeof(mbuff1));
		str1 << "WARNING : there were " << total_err << " missing parameters in the bonded terms." << endl << ends;
		mdl->PrintToLog(mbuff1);
	}
}

eng1_mm_tripos52_bt::~eng1_mm_tripos52_bt(void)
{
	delete[] bt1data;
	delete[] bt2data;
}

i32s eng1_mm_tripos52_bt::FindTorsion(atom * a1, atom * a2, atom * a3, atom * a4)
{
/*	i32s tmp[4] = { a1->index, a2->index, a3->index, a4->index };
	
	for (i32s n1 = 0;n1 < (i32s) bt3_vector.size();n1++)
	{
		bool test;	// since torsion is the same in both directions, we can ignore direction...
		
		test = true;
		if (bt3_vector[n1].atmi[0] != tmp[0]) test = false;
		if (bt3_vector[n1].atmi[1] != tmp[1]) test = false;
		if (bt3_vector[n1].atmi[2] != tmp[2]) test = false;
		if (bt3_vector[n1].atmi[3] != tmp[3]) test = false;
		if (test) return n1;
		
		test = true;
		if (bt3_vector[n1].atmi[0] != tmp[3]) test = false;
		if (bt3_vector[n1].atmi[1] != tmp[2]) test = false;
		if (bt3_vector[n1].atmi[2] != tmp[1]) test = false;
		if (bt3_vector[n1].atmi[3] != tmp[0]) test = false;
		if (test) return n1;
	}
	
	return NOT_DEFINED;	*/
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
	i32s iglob[4] = { a1->varind, a2->varind, a3->varind, a4->varind };
	i32s iloc[4];
	
	atom ** atmtab = GetSetup()->GetMMAtoms();
	atom ** glob_atmtab = GetSetup()->GetAtoms();
	
	for (i32s n1 = 0;n1 < 4;n1++)
	{
		i32s index = 0;
		while (index < GetSetup()->GetMMAtomCount())
		{
			if (glob_atmtab[iglob[n1]] == atmtab[index]) break;
			else index++;
		}
		
		if (index >= GetSetup()->GetMMAtomCount())
		{
			cout << "ERROR : iloc search failed!!!" << endl;
			exit(EXIT_FAILURE);
		}
		
		iloc[n1] = index;
	}
	
	for (i32s n1 = 0;n1 < (i32s) bt3_vector.size();n1++)
	{
		bool test;	// since torsion is the same in both directions, we can ignore direction...
		
		test = true;
		if (bt3_vector[n1].atmi[0] != iloc[0]) test = false;
		if (bt3_vector[n1].atmi[1] != iloc[1]) test = false;
		if (bt3_vector[n1].atmi[2] != iloc[2]) test = false;
		if (bt3_vector[n1].atmi[3] != iloc[3]) test = false;
		if (test) return n1;
		
		test = true;
		if (bt3_vector[n1].atmi[0] != iloc[3]) test = false;
		if (bt3_vector[n1].atmi[1] != iloc[2]) test = false;
		if (bt3_vector[n1].atmi[2] != iloc[1]) test = false;
		if (bt3_vector[n1].atmi[3] != iloc[0]) test = false;
		if (test) return n1;
	}
	
	return NOT_DEFINED;
}

bool eng1_mm_tripos52_bt::SetTorsionConstraint(i32s ind_bt3, f64 opt, f64 fc, bool lock_local_structure)
{
	if (ind_bt3 < 0) return false;
	if (ind_bt3 >= (i32s) bt3_vector.size()) return false;
	
	// check that opt is in valid range [-pi,+pi]!!!
	
	while (opt > +M_PI) opt -= 2.0 * M_PI;
	while (opt < -M_PI) opt += 2.0 * M_PI;
	
	// measure the current torsion and set constraints for the other torsions, if requested...
	
	if (lock_local_structure)
	{
	/*	v3d<f64> v1a(crd[bt3_vector[ind_bt3].atmi[1]], crd[bt3_vector[ind_bt3].atmi[0]]);
		v3d<f64> v1b(crd[bt3_vector[ind_bt3].atmi[1]], crd[bt3_vector[ind_bt3].atmi[2]]);
		v3d<f64> v1c(crd[bt3_vector[ind_bt3].atmi[2]], crd[bt3_vector[ind_bt3].atmi[3]]);
		f64 delta = opt - v1a.tor(v1b, v1c);	*/
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
		v3d<f64> v1a(& crd[l2g_mm[bt3_vector[ind_bt3].atmi[1]] * 3], & crd[l2g_mm[bt3_vector[ind_bt3].atmi[0]] * 3]);
		v3d<f64> v1b(& crd[l2g_mm[bt3_vector[ind_bt3].atmi[1]] * 3], & crd[l2g_mm[bt3_vector[ind_bt3].atmi[2]] * 3]);
		v3d<f64> v1c(& crd[l2g_mm[bt3_vector[ind_bt3].atmi[2]] * 3], & crd[l2g_mm[bt3_vector[ind_bt3].atmi[3]] * 3]);
		f64 delta = opt - v1a.tor(v1b, v1c);
		
		while (delta > +M_PI) delta -= 2.0 * M_PI;
		while (delta < -M_PI) delta += 2.0 * M_PI;
		
		i32s tmp1 = bt3_vector[ind_bt3].atmi[1];
		i32s tmp2 = bt3_vector[ind_bt3].atmi[2];
		
		for (i32s n1 = 0;n1 < (i32s) bt3_vector.size();n1++)
		{
			bool test1 = true;
			if (bt3_vector[n1].atmi[1] != tmp1) test1 = false;
			if (bt3_vector[n1].atmi[2] != tmp2) test1 = false;
			
			bool test2 = true;
			if (bt3_vector[n1].atmi[1] != tmp2) test2 = false;
			if (bt3_vector[n1].atmi[2] != tmp1) test2 = false;
			
			if (test1 || test2)
			{
			/*	v3d<f64> v2a(crd[bt3_vector[n1].atmi[1]], crd[bt3_vector[n1].atmi[0]]);
				v3d<f64> v2b(crd[bt3_vector[n1].atmi[1]], crd[bt3_vector[n1].atmi[2]]);
				v3d<f64> v2c(crd[bt3_vector[n1].atmi[2]], crd[bt3_vector[n1].atmi[3]]);
				f64 local = v2a.tor(v2b, v2c) + delta;	*/
	////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////
				v3d<f64> v2a(& crd[l2g_mm[bt3_vector[n1].atmi[1]] * 3], & crd[l2g_mm[bt3_vector[n1].atmi[0]] * 3]);
				v3d<f64> v2b(& crd[l2g_mm[bt3_vector[n1].atmi[1]] * 3], & crd[l2g_mm[bt3_vector[n1].atmi[2]] * 3]);
				v3d<f64> v2c(& crd[l2g_mm[bt3_vector[n1].atmi[2]] * 3], & crd[l2g_mm[bt3_vector[n1].atmi[3]] * 3]);
				f64 local = v2a.tor(v2b, v2c) + delta;
				
				while (local > +M_PI) local -= 2.0 * M_PI;
				while (local < -M_PI) local += 2.0 * M_PI;
				
				bt3_vector[n1].constraint = true;
				bt3_vector[n1].k = local; bt3_vector[n1].s = fc;
			}
		}
	}
	
	// then set the requested constraint... if lock_local_structure was true, then
	// this operation is in fact redundant, but perhaps a bit more accurate than above.
	
	bt3_vector[ind_bt3].constraint = true;
	bt3_vector[ind_bt3].k = opt; bt3_vector[ind_bt3].s = fc;
	return true;
}

void eng1_mm_tripos52_bt::ComputeBT1(i32u p1)
{
//printf("eng1_mm_tripos52_bt::ComputeBT1 = %d\n", p1);
	energy_bt1 = 0.0;
	
	atom ** atmtab = GetSetup()->GetMMAtoms();
	
	// len -> length of the bond vector, in nanometers [nm].
	
	// the bond is a vector, since it has unique begin and end points.
	// if a bond vector needs to be reversed, it's begin and end points are swapped.
	// all data for both forward and reverse vectors are calculated and stored...
	
	// dlen[0] -> grad[0-2]: for atom 1 when direction = 0, for atom 0 when direction = 1
	// dlen[1] -> grad[0-2]: for atom 0 when direction = 0, for atom 1 when direction = 1
	
	// the end point gradient of a vector is always the same as components of the unit vector!!!
	
	for (i32s n1 = 0;n1 < (i32s) bt1_vector.size();n1++)
	{
		i32s * atmi = bt1_vector[n1].atmi;
		
		f64 t1a[3]; f64 t1b = 0.0;
		for (i32s n2 = 0;n2 < 3;n2++)
		{
	//		f64 t9a = crd[atmi[0]][n2];
	//		f64 t9b = crd[atmi[1]][n2];
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
			f64 t9a = crd[l2g_mm[atmi[0]] * 3 + n2];
			f64 t9b = crd[l2g_mm[atmi[1]] * 3 + n2];
			
			t1a[n2] = t9a - t9b;
			t1b += t1a[n2] * t1a[n2];
		}
		
		f64 t1c = sqrt(t1b);
		bt1data[n1].len = t1c;
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			f64 t9a = t1a[n2] / t1c;
			
			bt1data[n1].dlen[0][n2] = +t9a;
			bt1data[n1].dlen[1][n2] = -t9a;
		}
		
		// f = a(x-b)^2
		// df/dx = 2a(x-b)
		
		f64 t2a = t1c - bt1_vector[n1].opt;
		f64 t2b = bt1_vector[n1].fc * t2a * t2a;
		
		energy_bt1 += t2b;
if (atmtab[atmi[0]]->flags & ATOMFLAG_IS_SOLVENT_ATOM) E_solvent += t2b; else E_solute += t2b;
		
		if (p1 > 0)
		{
			f64 t2c = 2.0 * bt1_vector[n1].fc * t2a;
			
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t2d = bt1data[n1].dlen[0][n2] * t2c;
				
		//		d1[atmi[0]][n2] += t2d;
		//		d1[atmi[1]][n2] -= t2d;
		//////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////
				d1[l2g_mm[atmi[0]] * 3 + n2] += t2d;
				d1[l2g_mm[atmi[1]] * 3 + n2] -= t2d;
			}
		}
	}
}

void eng1_mm_tripos52_bt::ComputeBT2(i32u p1)
{
//printf("eng1_mm_tripos52_bt::ComputeBT2 = %d\n", p1);
	energy_bt2 = 0.0;
	
	atom ** atmtab = GetSetup()->GetMMAtoms();
	
	// ang -> cosine of the bond angle, in the usual range [-1.0, +1.0]
	
	// we need directions also here... the angle consists of three points, say A-B-C.
	// when we reverse the angle, we will swap the end points: now they will be C-B-A.
	
	// dang[0] -> grad[0-2]: for atom 2 when direction = 0, for atom 0 when direction = 1
	// dang[1] -> grad[0-2]: for atom 1 when direction = 0, for atom 1 when direction = 1
	// dang[2] -> grad[0-2]: for atom 0 when direction = 0, for atom 2 when direction = 1
	
	for (i32s n1 = 0;n1 < (i32s) bt2_vector.size();n1++)
	{
		i32s * atmi = bt2_vector[n1].atmi;
		
		i32s * index1 = bt2_vector[n1].index1;
		bool * dir1 = bt2_vector[n1].dir1;
		
		f64 * t1a = bt1data[index1[0]].dlen[dir1[0]];
		f64 * t1b = bt1data[index1[1]].dlen[dir1[1]];
		
		f64 t1c = t1a[0] * t1b[0] + t1a[1] * t1b[1] + t1a[2] * t1b[2];
		
		if (t1c < -1.0) t1c = -1.0;		// domain check...
		if (t1c > +1.0) t1c = +1.0;		// domain check...
		
		bt2data[n1].csa = t1c;
		
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			f64 t9a = (t1b[n2] - t1c * t1a[n2]) / bt1data[index1[0]].len;
			f64 t9b = (t1a[n2] - t1c * t1b[n2]) / bt1data[index1[1]].len;
			
			bt2data[n1].dcsa[0][n2] = t9a;
			bt2data[n1].dcsa[1][n2] = -(t9a + t9b);
			bt2data[n1].dcsa[2][n2] = t9b;
		}
		
		f64 t2a; f64 t2b;	// t2a = f ; t2b = df/dx
		if (bt2_vector[n1].opt > M_PI * 170.0 / 180.0)		// later changed to NEAR_LINEAR_LIMIT.
		{
			// f = a(1 + x)
			// df/dx = a
			
			f64 t3b = 1.0 * bt2_vector[n1].fc;	// CHECK THE VALUE!!!
			t2a = t3b * (1.0 + t1c);
			
			t2b = t3b;
		}
		else
		{
			// f = a(acos(x)-b)^2
			// df/dx = -2a(x-b)/sqrt(1-x*x)
			
			f64 t3b = acos(t1c) - bt2_vector[n1].opt;
			t2a = bt2_vector[n1].fc * t3b * t3b;
			
			t2b = -2.0 * bt2_vector[n1].fc * t3b / sqrt(1.0 - t1c * t1c);
		}
		
		energy_bt2 += t2a;
if (atmtab[atmi[0]]->flags & ATOMFLAG_IS_SOLVENT_ATOM) E_solvent += t2a; else E_solute += t2a;
		
		if (p1 > 0)
		{
			for (i32s n2 = 0;n2 < 3;n2++)
			{
			//	d1[atmi[0]][n2] += bt2data[n1].dcsa[0][n2] * t2b;
			//	d1[atmi[1]][n2] += bt2data[n1].dcsa[1][n2] * t2b;
			//	d1[atmi[2]][n2] += bt2data[n1].dcsa[2][n2] * t2b;
			//////////////////////////////////////////////////////////////////////7
			//////////////////////////////////////////////////////////////////////7
				d1[l2g_mm[atmi[0]] * 3 + n2] += bt2data[n1].dcsa[0][n2] * t2b;
				d1[l2g_mm[atmi[1]] * 3 + n2] += bt2data[n1].dcsa[1][n2] * t2b;
				d1[l2g_mm[atmi[2]] * 3 + n2] += bt2data[n1].dcsa[2][n2] * t2b;
			}
		}
	}
}

void eng1_mm_tripos52_bt::ComputeBT3(i32u p1)
{
//printf("eng1_mm_tripos52_bt::ComputeBT3 = %d\n", p1);
	energy_bt3 = 0.0;
	
	atom ** atmtab = GetSetup()->GetMMAtoms();
	
	for (i32s n1 = 0;n1 < (i32s) bt3_vector.size();n1++)
	{
		i32s * atmi = bt3_vector[n1].atmi;
		
		i32s * index2 = bt3_vector[n1].index2;
		i32s * index1 = bt3_vector[n1].index1;
		bool * dir1 = bt3_vector[n1].dir1;
		
		f64 t1a[2] = { bt2data[index2[0]].csa, bt2data[index2[1]].csa };
		f64 t1b[2] = { 1.0 - t1a[0] * t1a[0], 1.0 - t1a[1] * t1a[1] };
		
		f64 t1c[2][3];
		t1c[0][0] = bt1data[index1[0]].dlen[dir1[0]][0] - t1a[0] * bt1data[index1[1]].dlen[dir1[1]][0];
		t1c[0][1] = bt1data[index1[0]].dlen[dir1[0]][1] - t1a[0] * bt1data[index1[1]].dlen[dir1[1]][1];
		t1c[0][2] = bt1data[index1[0]].dlen[dir1[0]][2] - t1a[0] * bt1data[index1[1]].dlen[dir1[1]][2];
		t1c[1][0] = bt1data[index1[3]].dlen[dir1[3]][0] - t1a[1] * bt1data[index1[2]].dlen[dir1[2]][0];
		t1c[1][1] = bt1data[index1[3]].dlen[dir1[3]][1] - t1a[1] * bt1data[index1[2]].dlen[dir1[2]][1];
		t1c[1][2] = bt1data[index1[3]].dlen[dir1[3]][2] - t1a[1] * bt1data[index1[2]].dlen[dir1[2]][2];
		
		f64 t1d = t1c[0][0] * t1c[1][0] + t1c[0][1] * t1c[1][1] + t1c[0][2] * t1c[1][2];
		f64 t1e = t1d / sqrt(t1b[0] * t1b[1]);
		
		if (t1e < -1.0) t1e = -1.0;		// domain check...
		if (t1e > +1.0) t1e = +1.0;		// domain check...
		
		f64 t1f[3];
		t1f[0] = acos(t1e);
		
		// now we still have to determine the sign of the result...
		// now we still have to determine the sign of the result...
		// now we still have to determine the sign of the result...
		
		f64 t1g[3];
		t1g[0] = bt1data[index1[2]].dlen[dir1[2]][1] * bt1data[index1[3]].dlen[dir1[3]][2] -
			bt1data[index1[2]].dlen[dir1[2]][2] * bt1data[index1[3]].dlen[dir1[3]][1];
		t1g[1] = bt1data[index1[2]].dlen[dir1[2]][2] * bt1data[index1[3]].dlen[dir1[3]][0] -
			bt1data[index1[2]].dlen[dir1[2]][0] * bt1data[index1[3]].dlen[dir1[3]][2];
		t1g[2] = bt1data[index1[2]].dlen[dir1[2]][0] * bt1data[index1[3]].dlen[dir1[3]][1] -
			bt1data[index1[2]].dlen[dir1[2]][1] * bt1data[index1[3]].dlen[dir1[3]][0];
		
		f64 t1h = t1c[0][0] * t1g[0] + t1c[0][1] * t1g[1] + t1c[0][2] * t1g[2];
		if (t1h < 0.0) t1f[0] = -t1f[0];
		
		t1f[1] = t1f[0] + t1f[0];
		t1f[2] = t1f[1] + t1f[0];
		
		f64 t9a; f64 t9b;
		if (bt3_vector[n1].constraint)
		{
			// Dx = x-b			| for the following formulas...
			
			// f = a(2PI-Dx)^4		| if Dx > +PI
			// df/fx = -4a(2PI-Dx)^3
			
			// f = a(2PI+Dx)^4		| if Dx < -PI
			// df/fx = +4a(2PI+Dx)^3
			
			// f = a(Dx)^4			| otherwise
			// df/fx = 4a(Dx)^3
			
			f64 t8a = t1f[0] - bt3_vector[n1].k;
			if (t8a > +M_PI)
			{
				t8a = 2.0 * M_PI - t8a;
				f64 t8b = t8a * t8a;
				
				t9a = bt3_vector[n1].s * t8b * t8b;
				t9b = -4.0 * bt3_vector[n1].s * t8b * t8a;
			}
			else if (t8a < -M_PI)
			{
				t8a = 2.0 * M_PI + t8a;
				f64 t8b = t8a * t8a;
				
				t9a = bt3_vector[n1].s * t8b * t8b;
				t9b = +4.0 * bt3_vector[n1].s * t8b * t8a;
			}
			else
			{
				f64 t8b = t8a * t8a;
				
				t9a = bt3_vector[n1].s * t8b * t8b;
				t9b = 4.0 * bt3_vector[n1].s * t8b * t8a;
			}
		}
		else
		{
			// f = a(1+cos(x))+b(1-cos(2x))+c(1+cos(3x))
			// df/dx = -a*sin(x)+2b*sin(2x)-3c*sin(3x)
			
		// here we make a quick conversion into a fourier series...
		// here we make a quick conversion into a fourier series...
		// here we make a quick conversion into a fourier series...
		
		f64 fc[3] = { 0.0, 0.0, 0.0 }; i32s zzz1 = (i32s) bt3_vector[n1].s;
		i32u zzz2 = abs(zzz1); if (zzz2 < 1) zzz2 = 1; zzz2 -= 1; if (zzz2 == 1) zzz1 = -zzz1;
		fc[zzz2] = bt3_vector[n1].k; if (zzz1 < 0) fc[zzz2] = -fc[zzz2];
		
			f64 t8a = fc[0] * (1.0 + cos(t1f[0]));
			f64 t8b = fc[1] * (1.0 - cos(t1f[1]));
			f64 t8c = fc[2] * (1.0 + cos(t1f[2]));
			t9a = t8a + t8b + t8c;
			
			f64 t8r = fc[0] * sin(t1f[0]);
			f64 t8s = fc[1] * sin(t1f[1]) * 2.0;
			f64 t8t = fc[2] * sin(t1f[2]) * 3.0;
			t9b = t8s - (t8r + t8t);
		}
		
		energy_bt3 += t9a;
if (atmtab[atmi[0]]->flags & ATOMFLAG_IS_SOLVENT_ATOM) E_solvent += t9a; else E_solute += t9a;
		
		if (p1 > 0)
		{
			f64 t2a = bt1data[index1[0]].len * t1b[0];
			f64 t2b = bt1data[index1[0]].len * t1a[0] / bt1data[index1[1]].len;
			
			f64 t3a = bt1data[index1[3]].len * t1b[1];
			f64 t3b = bt1data[index1[3]].len * t1a[1] / bt1data[index1[2]].len;
			
			f64 t4c[3]; f64 t5c[3]; f64 t6a[3]; f64 t7a[3];
			const i32s cp[3][3] = { { 0, 1, 2 }, { 1, 2, 0 }, { 2, 0, 1 } };
			
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t4a = bt1data[index1[0]].dlen[dir1[0]][cp[n2][1]] * bt1data[index1[1]].dlen[dir1[1]][cp[n2][2]];
				f64 t4b = bt1data[index1[0]].dlen[dir1[0]][cp[n2][2]] * bt1data[index1[1]].dlen[dir1[1]][cp[n2][1]];
				t4c[n2] = (t4a - t4b) / t2a;
				
				f64 t5a = bt1data[index1[2]].dlen[dir1[2]][cp[n2][2]] * bt1data[index1[3]].dlen[dir1[3]][cp[n2][1]];
				f64 t5b = bt1data[index1[2]].dlen[dir1[2]][cp[n2][1]] * bt1data[index1[3]].dlen[dir1[3]][cp[n2][2]];
				t5c[n2] = (t5a - t5b) / t3a;
				
			//	d1[atmi[0]][n2] += t4c[n2] * t9b;
			//	d1[atmi[3]][n2] += t5c[n2] * t9b;
			//////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////
				d1[l2g_mm[atmi[0]] * 3 + n2] += t4c[n2] * t9b;
				d1[l2g_mm[atmi[3]] * 3 + n2] += t5c[n2] * t9b;
				
				t6a[n2] = (t2b - 1.0) * t4c[n2] - t3b * t5c[n2];
				t7a[n2] = (t3b - 1.0) * t5c[n2] - t2b * t4c[n2];
				
			//	d1[atmi[1]][n2] += t6a[n2] * t9b;
			//	d1[atmi[2]][n2] += t7a[n2] * t9b;
			//////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////
				d1[l2g_mm[atmi[1]] * 3 + n2] += t6a[n2] * t9b;
				d1[l2g_mm[atmi[2]] * 3 + n2] += t7a[n2] * t9b;
			}
		}
	}
}

void eng1_mm_tripos52_bt::ComputeBT4(i32u p1)
{
//printf("eng1_mm_tripos52_bt::ComputeBT4 = %d\n", p1);
	energy_bt4 = 0.0;
}

/*################################################################################################*/

eng1_mm_tripos52_nbt_mbp::eng1_mm_tripos52_nbt_mbp(setup * p1, i32u p2) : engine(p1, p2), eng1_mm(p1, p2), engine_mbp(p1, p2)
{
	model * mdl = GetSetup()->GetModel();
	atom ** atmtab = GetSetup()->GetMMAtoms();
//	bond ** bndtab = GetSetup()->GetMMBonds();
	
//	ostream * ostr = tripos52_tables::GetInstance()->ostr;
	ostream * ostr = & cout;
	
	// also check engine::bp_center!!!
	// also check engine::bp_center!!!
	// also check engine::bp_center!!!
	bp_fc_solute = 5000.0;		// 50 kJ/(mol*Å^2) = 5000 kJ/(mol*(nm)^2)
	bp_fc_solvent = 12500.0;	// 125 kJ/(mol*Å^2) = 12500 kJ/(mol*(nm)^2)
	
	if (ostr != NULL && use_simple_bp)
	{
		(* ostr) << "use_simple_bp ; ";
		(* ostr) << bp_radius_solute << " " << bp_fc_solute << " ; ";
		(* ostr) << bp_radius_solvent << " " << bp_fc_solvent << endl;
	}
	
	if (ostr != NULL) (* ostr) << "creating nbt1-terms: ";
	i32s nbt1_err = 0;
	
/*	for (i32s ind1 = 0;ind1 < natm - 1;ind1++)
	{
		for (i32s ind2 = ind1 + 1;ind2 < natm;ind2++)
		{
			i32s test = range_cr1[ind1];
			while (test < range_cr1[ind1 + 1])
			{
				if (cr1[test] == ind2) break;
				else test++;
			}
			
			if (test == range_cr1[ind1 + 1])
			{
				test = range_cr2[ind1];
				while (test < range_cr2[ind1 + 1])
				{
					if (cr2[test] == ind2) break;
					else test++;
				}
				
				bool is14 = (test != range_cr2[ind1 + 1]);
				
				mm1_exp1_nbt1 newnbt1;
				newnbt1.atmi[0] = ind1;
				newnbt1.atmi[1] = ind2;
				
				nbt1_err += !exp1_tables::GetInstance()->Init(this, & newnbt1, is14);
				nbt1_vector.push_back(newnbt1);
			}
		}
	}	*/
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
	for (i32s ind1 = 0;ind1 < GetSetup()->GetMMAtomCount() - 1;ind1++)
	{
		for (i32s ind2 = ind1 + 1;ind2 < GetSetup()->GetMMAtomCount();ind2++)
		{
			i32s test = range_cr1[ind1];
			while (test < range_cr1[ind1 + 1])
			{
				if (cr1[test] == atmtab[ind2]) break;
				else test++;
			}
			
			if (test == range_cr1[ind1 + 1])	// if this is true, the atom are not in 1-2 or 1-3 positions.
			{
				test = range_cr2[ind1];
				while (test < range_cr2[ind1 + 1])
				{
					if (cr2[test] == atmtab[ind2]) break;
					else test++;
				}
				
				bool is14 = (test != range_cr2[ind1 + 1]);	// if this is true, the atoms are 1-4 related.
				
				// see also eng1_mm_tripos52_nbt_mim::UpdateTerms() ; should be the same!!!
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
				
				mm_tripos52_nbt1 newnbt1;
				newnbt1.atmi[0] = ind1;		// this is a local index...
				newnbt1.atmi[1] = ind2;		// this is a local index...
				
				nbt1_err += !tripos52_tables::GetInstance()->Init(this, & newnbt1, is14);
				nbt1_vector.push_back(newnbt1);
			}
		}
	}
	
	// report possible errors...
	
	i32s total_err = nbt1_err;
	if (total_err)
	{
		char mbuff1[256];
		ostrstream str1(mbuff1, sizeof(mbuff1));
		str1 << "WARNING : there were " << total_err << " missing parameters in the nonbonded terms." << endl << ends;
		mdl->PrintToLog(mbuff1);
	}
}

eng1_mm_tripos52_nbt_mbp::~eng1_mm_tripos52_nbt_mbp(void)
{
}

void eng1_mm_tripos52_nbt_mbp::ComputeNBT1(i32u p1)
{
//printf("eng1_mm_tripos52_nbt_mbp::ComputeNBT1 = %d\n", p1);
	energy_nbt1a = 0.0;
	energy_nbt1b = 0.0;
	energy_nbt1c = 0.0;
	energy_nbt1d = 0.0;
	
	atom ** atmtab = GetSetup()->GetMMAtoms();
	
	// an additional pass for the boundary potential (optional).
	// an additional pass for the boundary potential (optional).
	// an additional pass for the boundary potential (optional).
	
	if (use_simple_bp)
	{
		if (nd_eval != NULL) nd_eval->AddCycle();
		for (i32s n1 = 0;n1 < GetSetup()->GetMMAtomCount();n1++)
		{
			f64 radius = bp_radius_solute;
			f64 fc = bp_fc_solute;
			
			if (atmtab[n1]->flags & ATOMFLAG_IS_SOLVENT_ATOM)
			{
				radius = bp_radius_solvent;
				fc = bp_fc_solvent;
			}
			
			f64 t1a[3]; f64 t1b = 0.0;
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t9a = bp_center[n2];
				f64 t9b = crd[l2g_mm[n1] * 3 + n2];
				
				t1a[n2] = t9a - t9b;
				t1b += t1a[n2] * t1a[n2];
			}
			
			f64 t1c = sqrt(t1b);
			
			if (nd_eval != NULL && (atmtab[n1]->flags & ATOMFLAG_MEASURE_ND_RDF)) nd_eval->AddValue(t1c);
			
			if (rdf_eval != NULL && rdf_eval->count_begin > -0.5)
			{
				bool is_in_rdf_counting_window = true;
				if (t1c < rdf_eval->count_begin) is_in_rdf_counting_window = false;
				if (t1c >= rdf_eval->count_end) is_in_rdf_counting_window = false;
				
				if (is_in_rdf_counting_window) atmtab[n1]->flags |= ATOMFLAG_COUNT_IN_RDF;
				else atmtab[n1]->flags &= (~ATOMFLAG_COUNT_IN_RDF);
			}
			
			if (t1c < radius) continue;
			
			// f = a(x-b)^2
			// df/dx = 2a(x-b)
			
			f64 t2a = t1c - radius;
			f64 t2b = fc * t2a * t2a;
			
			energy_bt1 += t2b;
if (atmtab[n1]->flags & ATOMFLAG_IS_SOLVENT_ATOM) E_solvent += t2b; else E_solute += t2b;
			
			if (p1 > 0)
			{
				f64 t2c = 2.0 * fc * t2a;
				
				for (i32s n2 = 0;n2 < 3;n2++)
				{
					f64 t2d = (t1a[n2] / t1c) * t2c;
					
					d1[l2g_mm[n1] * 3 + n2] -= t2d;
				}
			}
		}
	}
	
	// the nonbonded terms begin...
	// the nonbonded terms begin...
	// the nonbonded terms begin...
	
	if (rdf_eval != NULL) rdf_eval->AddCycle();
	for (i32s n1 = 0;n1 < (i32s) nbt1_vector.size();n1++)
	{
		i32s * atmi = nbt1_vector[n1].atmi;
		
		f64 t1a[3]; f64 t1b = 0.0;
		for (i32s n2 = 0;n2 < 3;n2++)
		{
//			f64 t2a = crd[atmi[0]][n2];
//			f64 t2b = crd[atmi[1]][n2];
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
			f64 t2a = crd[l2g_mm[atmi[0]] * 3 + n2];
			f64 t2b = crd[l2g_mm[atmi[1]] * 3 + n2];
			
			t1a[n2] = t2a - t2b;
			t1b += t1a[n2] * t1a[n2];
		}
		
		f64 t1c = sqrt(t1b);
		if (rdf_eval != NULL)
		{
			bool flag = true;
			if (!(atmtab[atmi[0]]->flags & ATOMFLAG_MEASURE_ND_RDF)) flag = false;
			if (!(atmtab[atmi[1]]->flags & ATOMFLAG_MEASURE_ND_RDF)) flag = false;
			
			if (rdf_eval->count_begin > -0.5)	// check the counting window, if needed...
			{
				if (!(atmtab[atmi[0]]->flags & ATOMFLAG_COUNT_IN_RDF)) flag = false;
				if (!(atmtab[atmi[1]]->flags & ATOMFLAG_COUNT_IN_RDF)) flag = false;
			}
			
			if (flag) rdf_eval->AddValue(t1c);
		}
		
		// f1 = (r/a)^-12 - (r/b)^-6
		// df1/dr = -12/a(r/a)^-13 + 6/b(r/b)^-7
		
		f64 t3a = t1c / nbt1_vector[n1].k1;
		f64 t3b = t1c / nbt1_vector[n1].k2;
		
		f64 t4a = t3a * t3a * t3a; f64 t4b = t4a * t4a; f64 t4c = t4b * t4b;	// ^3 ^6 ^12
		f64 t5a = t3b * t3b * t3b; f64 t5b = t5a * t5a;				// ^3 ^6
		
		f64 t6a = 1.0 / (t4c) - 1.0 / (t5b);
		
		// f2 = Q/r
		// df2/dr = -Q/r^2
		
		f64 t6b = nbt1_vector[n1].qq / t1c;
		
		energy_nbt1a += t6a + t6b;
		
		f64 tote = t6a + t6b;
int class1 = (atmtab[atmi[0]]->flags & ATOMFLAG_IS_SOLVENT_ATOM);
int class2 = (atmtab[atmi[1]]->flags & ATOMFLAG_IS_SOLVENT_ATOM);
if (class1 == class2) { if (class1) E_solvent += tote; else E_solute += tote; }
else E_solusolv += tote;
		
		if (p1 > 0)
		{
			f64 t7a = 12.0 / (nbt1_vector[n1].k1 * t4c * t3a);
			f64 t7b = 6.0 / (nbt1_vector[n1].k2 * t5b * t3b);
			
			f64 t8a = nbt1_vector[n1].qq / t1b;
			
			f64 t9a = t7b - t7a - t8a;
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t9b = (t1a[n2] / t1c) * t9a;
				
		//		d1[atmi[0]][n2] += t9b;
		//		d1[atmi[1]][n2] -= t9b;
		///////////////////////////////////////////////////////
		///////////////////////////////////////////////////////
				d1[l2g_mm[atmi[0]] * 3 + n2] += t9b;
				d1[l2g_mm[atmi[1]] * 3 + n2] -= t9b;
			}
		}
	}
}

/*################################################################################################*/

eng1_mm_tripos52_nbt_mim::eng1_mm_tripos52_nbt_mim(setup * p1, i32u p2) : engine(p1, p2), eng1_mm(p1, p2), engine_pbc(p1, p2)
{
	fGL mindim = GetSetup()->GetModel()->periodic_box_HALFdim[0];
	if (GetSetup()->GetModel()->periodic_box_HALFdim[1] < mindim) mindim = GetSetup()->GetModel()->periodic_box_HALFdim[1];
	if (GetSetup()->GetModel()->periodic_box_HALFdim[2] < mindim) mindim = GetSetup()->GetModel()->periodic_box_HALFdim[2];
	
	sw1 = 0.6; if (sw1 < (mindim - 0.4)) sw1 = mindim - 0.4;	// will trigger if boxdim < 2.0 nm!!!
	sw2 = shft1 = mindim - 0.2;
	
	limit = mindim;
	update = true;
	
	// calculate the actual values...
	
	sw1 = sw1 * sw1;
	sw2 = sw2 * sw2;
	
	swA = 3.0 * sw1;
	swB = pow(sw2 - sw1, 3.0);
	
	shft3 = pow(shft1, 3.0);
	
	limit = limit * limit;
	
	nbt1_vector.reserve(250000);
}

eng1_mm_tripos52_nbt_mim::~eng1_mm_tripos52_nbt_mim(void)
{
}

void eng1_mm_tripos52_nbt_mim::ComputeNBT1(i32u p1)
{
//printf("eng1_mm_tripos52_nbt_mim::ComputeNBT1 = %d\n", p1);
	energy_nbt1a = 0.0;
	energy_nbt1b = 0.0;
	energy_nbt1c = 0.0;
	energy_nbt1d = 0.0;
	
	atom ** atmtab = GetSetup()->GetMMAtoms();
	
	if (update) UpdateTerms();
	
	// the nonbonded terms begin...
	// the nonbonded terms begin...
	// the nonbonded terms begin...
	
	for (i32s n1 = 0;n1 < (i32s) nbt1_vector.size();n1++)
	{
		i32s * atmi = nbt1_vector[n1].atmi;
		
		f64 t1a[3]; f64 t1b = 0.0;
		for (i32s n2 = 0;n2 < 3;n2++)
		{
	//		f64 t2a = crd[atmi[0]][n2];
	//		f64 t2b = crd[atmi[1]][n2];
	///////////////////////////////////////////////////////
	///////////////////////////////////////////////////////
			f64 t2a = crd[l2g_mm[atmi[0]] * 3 + n2];
			f64 t2b = crd[l2g_mm[atmi[1]] * 3 + n2];
			
			t1a[n2] = t2a - t2b;
			
			if (t1a[n2] < -GetSetup()->GetModel()->periodic_box_HALFdim[n2])
			{
				t1a[n2] += 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n2];
			}
			else if (t1a[n2] > +GetSetup()->GetModel()->periodic_box_HALFdim[n2])
			{
				t1a[n2] -= 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n2];
			}
			
			t1b += t1a[n2] * t1a[n2];
		}
		
		f64 t1c = sqrt(t1b);
		
		// f1 = (r/a)^-12 - (r/b)^-6
		// df1/dr = -12/a(r/a)^-13 + 6/b(r/b)^-7
		
		f64 t3a = t1c / nbt1_vector[n1].k1;
		f64 t3b = t1c / nbt1_vector[n1].k2;
		
		f64 t4a = t3a * t3a * t3a; f64 t4b = t4a * t4a; f64 t4c = t4b * t4b;	// ^3 ^6 ^12
		f64 t5a = t3b * t3b * t3b; f64 t5b = t5a * t5a;				// ^3 ^6
		
		f64 t6a = 1.0 / (t4c) - 1.0 / (t5b);
		
		// s1 = (rE^2 - r^2)^2 * (rE^2 + 2r^2 - 3rB^2) / (rE^2 - rB^2)^3
		// ds1/dr = [this will yield 2 terms quite easily...]
		
		f64 t3x;		// value
		f64 t3y; f64 t3z;	// derivative
		if (t1b < sw1)
		{
			t3x = 1.0;
			t3y = t3z = 0.0;
		}
		else if (t1b > sw2)
		{
			t3x = 0.0;
			t3y = t3z = 0.0;
		}
		else
		{
			f64 t3c = sw2 - t1b; f64 t3d = t3c * t3c;
			f64 t3e = sw2 + 2.0 * t1b - swA;
			
			t3x = t3d * t3e / swB;
			t3y = 4.0 * t1c * t3d / swB;
			t3z = 4.0 * t1c * t3c * t3e / swB;
		}
		
		energy_nbt1a += t6a * t3x;
		
		// f2 = Q/r
		// df2/dr = -Q/r^2
		
		f64 t6b = nbt1_vector[n1].qq / t1c;
		
		// s2 = (1 - (r/rE)^3)^2
		// ds2/dr = -6r^2 * (1 - (r/rE)^3) / rE^3
		
		f64 t4x;		// value
		f64 t4y;		// derivative
		if (t1c > shft1)
		{
			t4x = 0.0;
			t4y = 0.0;
		}
		else
		{
			f64 t4d = t1b * t1c / shft3;
			f64 t4e = 1.0 - t4d;
			
			t4x = t4e * t4e;
			t4y = 6.0 * t1b * t4e / shft3;
		}
		
		energy_nbt1b += t6b * t4x;
		
		f64 tote = t6a * t3x + t6b * t4x;
int class1 = (atmtab[atmi[0]]->flags & ATOMFLAG_IS_SOLVENT_ATOM);
int class2 = (atmtab[atmi[1]]->flags & ATOMFLAG_IS_SOLVENT_ATOM);
if (class1 == class2) { if (class1) E_solvent += tote; else E_solute += tote; }
else E_solusolv += tote;
		
		if (p1 > 0)
		{
			f64 t7a = 12.0 / (nbt1_vector[n1].k1 * t4c * t3a);
			f64 t7b = 6.0 / (nbt1_vector[n1].k2 * t5b * t3b);
			
			f64 t8a = nbt1_vector[n1].qq / t1b;
			
			f64 t9a = (t7b - t7a) * t3x + t6a * (t3y - t3z);
			f64 t9b = t8a * t4x + t6b * t4y;
			
			f64 t9c = t9a - t9b;
			
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t9d = (t1a[n2] / t1c) * t9c;
				
	//			d1[atmi[0]][n2] += t9d;
	//			d1[atmi[1]][n2] -= t9d;
	/////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////
				d1[l2g_mm[atmi[0]] * 3 + n2] += t9d;
				d1[l2g_mm[atmi[1]] * 3 + n2] -= t9d;
			}
		}
	}
}

void eng1_mm_tripos52_nbt_mim::UpdateTerms(void)
{
	model * mdl = GetSetup()->GetModel();
	atom ** atmtab = GetSetup()->GetMMAtoms();
//	bond ** bndtab = GetSetup()->GetMMBonds();
	
//	ostream * ostr = tripos52_tables::GetInstance()->ostr;
	ostream * ostr = & cout;
	
	update = false;
	nbt1_vector.resize(0);
	
	if (ostr != NULL) (* ostr) << "creating nbt1-terms: ";
	i32s nbt1_err = 0;
	
	for (i32s ind1 = 0;ind1 < GetSetup()->GetMMAtomCount() - 1;ind1++)
	{
		for (i32s ind2 = ind1 + 1;ind2 < GetSetup()->GetMMAtomCount();ind2++)
		{
			i32s test = range_cr1[ind1];
			while (test < range_cr1[ind1 + 1])
			{
				if (cr1[test] == atmtab[ind2]) break;
				else test++;
			}
			
			if (test == range_cr1[ind1 + 1])	// if this is true, the atom are not in 1-2 or 1-3 positions.
			{
				f64 t1a; f64 t1b = 0.0;
				for (i32s n1 = 0;n1 < 3;n1++)
				{
					f64 t2a = crd[l2g_mm[ind1] * 3 + n1];
					f64 t2b = crd[l2g_mm[ind2] * 3 + n1];
					
					t1a = t2a - t2b;
					
					if (t1a < -GetSetup()->GetModel()->periodic_box_HALFdim[n1])
					{
						t1a += 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n1];
					}
					else if (t1a > +GetSetup()->GetModel()->periodic_box_HALFdim[n1])
					{
						t1a -= 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n1];
					}
					
					t1b += t1a * t1a;
				}
				
				if (t1b > limit) continue;
				
				test = range_cr2[ind1];
				while (test < range_cr2[ind1 + 1])
				{
					if (cr2[test] == atmtab[ind2]) break;
					else test++;
				}
				
				bool is14 = (test != range_cr2[ind1 + 1]);	// if this is true, the atoms are 1-4 related.
				
				// see also eng1_mm_tripos52_nbt_mbp ctor ; should be the same!!!
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
				
				mm_tripos52_nbt1 newnbt1;
				newnbt1.atmi[0] = ind1;		// this is a local index...
				newnbt1.atmi[1] = ind2;		// this is a local index...
				
				nbt1_err += !tripos52_tables::GetInstance()->Init(this, & newnbt1, is14);
				nbt1_vector.push_back(newnbt1);
			}
		}
	}
	
	// report possible errors...
	
	i32s total_err = nbt1_err;
	if (total_err)
	{
		char mbuff1[256];
		ostrstream str1(mbuff1, sizeof(mbuff1));
		str1 << "WARNING : there were " << total_err << " missing parameters in the nonbonded terms." << endl << ends;
		mdl->PrintToLog(mbuff1);
	}
}

/*################################################################################################*/

eng1_mm_tripos52_mbp::eng1_mm_tripos52_mbp(setup * p1, i32u p2) : engine(p1, p2), eng1_mm(p1, p2),
	engine_mbp(p1, p2), eng1_mm_tripos52_bt(p1, p2), eng1_mm_tripos52_nbt_mbp(p1, p2)
{
//	cout << "eng1_mm_tripos52_mbp" << endl;
}

eng1_mm_tripos52_mbp::~eng1_mm_tripos52_mbp(void)
{
//	cout << "~eng1_mm_tripos52_mbp" << endl;
}

/*################################################################################################*/

eng1_mm_tripos52_mim::eng1_mm_tripos52_mim(setup * p1, i32u p2) : engine(p1, p2), eng1_mm(p1, p2),
	eng1_mm_tripos52_bt(p1, p2), eng1_mm_tripos52_nbt_mim(p1, p2)
{
}

eng1_mm_tripos52_mim::~eng1_mm_tripos52_mim(void)
{
}

/*################################################################################################*/

// eof
