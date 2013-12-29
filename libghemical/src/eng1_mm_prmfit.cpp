// ENG1_MM_PRMFIT.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "eng1_mm_prmfit.h"

#include <iomanip>
#include <fstream>
#include <strstream>
#include <algorithm>
using namespace std;

/*################################################################################################*/

eng1_mm_prmfit::eng1_mm_prmfit(setup * p1, i32u p2, prmfit_tables & tab) : engine(p1, p2), eng1_mm(p1, p2)
{
	tab.UpdateTypes(GetSetup());
	tab.UpdateCharges(GetSetup());
	
	ostream * ostr = tab.ostr;
	
	// set default sizes to containers. does it have any practical effects???
	
	bt1_vector.reserve(250);
	bt2_vector.reserve(500);
	bt3_vector.reserve(500);
	
	atom ** atmtab = GetSetup()->GetMMAtoms();
	bond ** bndtab = GetSetup()->GetMMBonds();
	
	atom ** glob_atmtab = GetSetup()->GetAtoms();
	
	i32s total_err;
	
/*##############################################*/
/*##############################################*/

	// create bt1-terms...
	
	if (ostr != NULL) (* ostr) << "creating bt1-terms: ";
	i32s bt1_err = 0;
	
if (!GetSetup()->GetModel()->IsIndexClean())	// obsolete???
	{
		cout << "BUG: is_index_clean == false at eng1_mm_prmfit" << endl;
		exit(EXIT_FAILURE);
	}
	
	for (i32s n1 = 0;n1 < GetSetup()->GetMMBondCount();n1++)
	{
		i32s local_ind1 = 0;
		while (local_ind1 < GetSetup()->GetMMAtomCount())
		{
			if (atmtab[local_ind1] == bndtab[n1]->atmr[0]) break;
			else local_ind1++;
		}
		if (local_ind1 >= GetSetup()->GetMMAtomCount())
		{
			cout << "BUG: local_ind1 search failed!" << endl;
			exit(EXIT_FAILURE);
		}
		
		i32s local_ind2 = 0;
		while (local_ind2 < GetSetup()->GetMMAtomCount())
		{
			if (atmtab[local_ind2] == bndtab[n1]->atmr[1]) break;
			else local_ind2++;
		}
		if (local_ind2 >= GetSetup()->GetMMAtomCount())
		{
			cout << "BUG: local_ind2 search failed!" << endl;
			exit(EXIT_FAILURE);
		}
		
		mm_prmfit_bt1 newbt1;
		newbt1.atmi[0] = local_ind1;		// this is a local index...
		newbt1.atmi[1] = local_ind2;		// this is a local index...
		
	prmfit_bs_query query; query.strict = false;
	query.atmtp[0] = atmtab[newbt1.atmi[0]]->atmtp;
	query.atmtp[1] = atmtab[newbt1.atmi[1]]->atmtp;
	query.bndtp = bndtab[n1]->bt.GetValue();
	
	tab.DoParamSearch(& query);
	if (query.index == NOT_DEFINED) bt1_err++;
	
		newbt1.opt = query.opt;
		newbt1.fc = query.fc;
		
		newbt1.bt = bndtab[n1]->bt.GetValue();		// save also bondtype information...
		
	// write temporary index information into the bond objects; this is needed below and is just for convenience.
	// write temporary index information into the bond objects; this is needed below and is just for convenience.
	// write temporary index information into the bond objects; this is needed below and is just for convenience.
	
		bndtab[n1]->tmp_bt1_index = bt1_vector.size();	// the bond objects are modified here!!!
		
		bt1_vector.push_back(newbt1);
	}
	
	if (ostr != NULL) (* ostr) << bt1_vector.size() << " terms, " << bt1_err << " errors." << endl;
	
	bt1data = new mm_bt1_data[bt1_vector.size()];
	
/*##############################################*/
/*##############################################*/
	
	// create bt2-terms...
	
	if (ostr != NULL) (* ostr) << "creating bt2-terms: ";
	i32s bt2_err = 0;
	
	for (i32s n1 = 0;n1 < GetSetup()->GetMMAtomCount();n1++)
	{
		if (atmtab[n1]->cr_list.size() < 2) continue;
		
		// central atom is known, now find all possible combinations of bonds...
		
		bool dir[2];
		iter_cl ita; iter_cl rnga[2];
		iter_cl itb; iter_cl rngb[2];
		
		rngb[1] = atmtab[n1]->cr_list.end();
		
		rnga[0] = atmtab[n1]->cr_list.begin();
		rnga[1] = rngb[1]; rnga[1]--;
		
		for (ita = rnga[0];ita != rnga[1];ita++)
		{
			rngb[0] = ita; rngb[0]++;
			dir[0] = (atmtab[n1] == (* ita).bndr->atmr[0]);
			
			for (itb = rngb[0];itb != rngb[1];itb++)
			{
				dir[1] = (atmtab[n1] == (* itb).bndr->atmr[0]);
				
				mm_prmfit_bt2 newbt2;
				newbt2.index1[0] = (* ita).bndr->tmp_bt1_index; newbt2.dir1[0] = dir[0];
				newbt2.index1[1] = (* itb).bndr->tmp_bt1_index; newbt2.dir1[1] = dir[1];
				
				newbt2.atmi[0] = bt1_vector[(* ita).bndr->tmp_bt1_index].get_atmi(1, dir[0]);
				newbt2.atmi[2] = bt1_vector[(* itb).bndr->tmp_bt1_index].get_atmi(1, dir[1]);
				
				newbt2.atmi[1] = bt1_vector[(* ita).bndr->tmp_bt1_index].get_atmi(0, dir[0]);
				
	prmfit_ab_query query; query.strict = false;
	query.atmtp[0] = atmtab[newbt2.atmi[0]]->atmtp;
	query.atmtp[1] = atmtab[newbt2.atmi[1]]->atmtp;
	query.atmtp[2] = atmtab[newbt2.atmi[2]]->atmtp;
	query.bndtp[0] = (* ita).bndr->bt.GetValue();
	query.bndtp[1] = (* itb).bndr->bt.GetValue();
	
	tab.DoParamSearch(& query);
	if (query.index == NOT_DEFINED) bt2_err++;
	
				newbt2.opt = query.opt;
				newbt2.fc = query.fc;
				
				newbt2.bt[0] = (* ita).bndr->bt.GetValue();	// save also bondtype information...
				newbt2.bt[1] = (* itb).bndr->bt.GetValue();	// save also bondtype information...
				
				bt2_vector.push_back(newbt2);
			}
		}
	}
	
	if (ostr != NULL) (* ostr) << bt2_vector.size() << " terms, " << bt2_err << " errors." << endl;
	
	bt2data = new mm_bt2_data[bt2_vector.size()];
	
/*##############################################*/
/*##############################################*/
	
	// create bt3-terms...

	if (ostr != NULL) (* ostr) << "creating bt3-terms: ";
	i32s bt3_err = 0;
	
	for (i32s n1 = 0;n1 < GetSetup()->GetMMAtomCount();n1++)
	{
		if (atmtab[n1]->cr_list.size() < 2) continue;
		
		// skip if default geometry is marked linear for this atomtype...
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		const prmfit_at * tp1 = tab.GetAtomType(atmtab[n1]->atmtp);
	//	if (tp1 == NULL) { cout << "GetAtomType() failed!" << endl; exit(EXIT_FAILURE); }
		if (tp1 != NULL && (tp1->flags & 3) == 1) continue;	// linear geometry -> undefined torsion.
		
		for (iter_cl it2 = atmtab[n1]->cr_list.begin();it2 != atmtab[n1]->cr_list.end();it2++)
		{
			bool another = (atmtab[n1] == (* it2).bndr->atmr[0]);
			atom * atmr = (* it2).bndr->atmr[another];
			
			if (atmr->cr_list.size() < 2) continue;			// no torsion...
			if (atmr->varind > atmtab[n1]->varind) continue;	// skip duplicates...
			
			// skip if default geometry is marked linear for this atomtype...
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			const prmfit_at * tp2 = tab.GetAtomType(atmr->atmtp);
		//	if (tp2 == NULL) { cout << "GetAtomType() failed!" << endl; exit(EXIT_FAILURE); }
			if (tp2 != NULL && (tp2->flags & 3) == 1) continue;	// linear geometry -> undefined torsion.
			
			// central atoms are known, now find all possible combinations of bonds...
			
			vector<i32s> ind1a; vector<bool> dir1a;		// search for the 1st group...
			for (iter_cl it3 = atmtab[n1]->cr_list.begin();it3 != atmtab[n1]->cr_list.end();it3++)
			{
				if ((* it3) == (* it2)) continue;
				
				ind1a.push_back((* it3).bndr->tmp_bt1_index);
				dir1a.push_back(atmtab[n1] == (* it3).bndr->atmr[0]);
			}
			
			vector<i32s> ind2a; vector<bool> dir2a;
			for (i32u n2 = 0;n2 < ind1a.size();n2++)
			{
				i32s tmp1 = 0; i32s tmp2 = NOT_DEFINED;
				while (true)
				{
					i32s bt1[2] = { bt2_vector[tmp1].index1[0], bt2_vector[tmp1].index1[1] };
					i32s bt2[2] = { bt2_vector[tmp1].dir1[0], bt2_vector[tmp1].dir1[1] };
					
	if (bt1[0] == ind1a[n2] && bt2[0] == dir1a[n2] && bt1[1] == (* it2).bndr->tmp_bt1_index && bt2[1] == another) tmp2 = true;
	if (bt1[1] == ind1a[n2] && bt2[1] == dir1a[n2] && bt1[0] == (* it2).bndr->tmp_bt1_index && bt2[0] == another) tmp2 = false;
					
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
			for (i32u n2 = 0;n2 < ind1b.size();n2++)
			{
				i32s tmp1 = 0; i32s tmp2 = NOT_DEFINED;
				while (true)
				{
					i32s bt1[2] = { bt2_vector[tmp1].index1[0], bt2_vector[tmp1].index1[1] };
					i32s bt2[2] = { bt2_vector[tmp1].dir1[0], bt2_vector[tmp1].dir1[1] };
					
	if (bt1[0] == ind1b[n2] && bt2[0] == dir1b[n2] && bt1[1] == (* it2).bndr->tmp_bt1_index && bt2[1] != another) tmp2 = false;
	if (bt1[1] == ind1b[n2] && bt2[1] == dir1b[n2] && bt1[0] == (* it2).bndr->tmp_bt1_index && bt2[0] != another) tmp2 = true;
					
					if (tmp2 < 0) tmp1++; else break;
				}
				
				ind2b.push_back(tmp1);
				dir2b.push_back(tmp2);
			}
			
			// now finally create the terms!!!!!
			
			for (i32u n2 = 0;n2 < ind2a.size();n2++)
			{
				for (i32u n3 = 0;n3 < ind2b.size();n3++)
				{
					mm_prmfit_bt3 newbt3;
					newbt3.index2[0] = ind2a[n2];
					newbt3.index2[1] = ind2b[n3];
					
					newbt3.index1[0] = bt2_vector[newbt3.index2[0]].get_index(0, dir2a[n2]);
					newbt3.dir1[0] = bt2_vector[newbt3.index2[0]].get_dir(0, dir2a[n2]);
					
					newbt3.index1[1] = bt2_vector[newbt3.index2[0]].get_index(1, dir2a[n2]);
					newbt3.dir1[1] = bt2_vector[newbt3.index2[0]].get_dir(1, dir2a[n2]);
					
					newbt3.index1[2] = bt2_vector[newbt3.index2[1]].get_index(0, dir2b[n3]);
					newbt3.dir1[2] = bt2_vector[newbt3.index2[1]].get_dir(0, dir2b[n3]);
					
					newbt3.index1[3] = bt2_vector[newbt3.index2[1]].get_index(1, dir2b[n3]);
					newbt3.dir1[3] = bt2_vector[newbt3.index2[1]].get_dir(1, dir2b[n3]);
					
					newbt3.atmi[0] = bt1_vector[newbt3.index1[0]].get_atmi(1, newbt3.dir1[0]);
					newbt3.atmi[1] = bt1_vector[newbt3.index1[0]].get_atmi(0, newbt3.dir1[0]);
					newbt3.atmi[2] = bt1_vector[newbt3.index1[3]].get_atmi(0, newbt3.dir1[3]);
					newbt3.atmi[3] = bt1_vector[newbt3.index1[3]].get_atmi(1, newbt3.dir1[3]);
					
	prmfit_tr_query query; query.strict = false;
	query.atmtp[0] = atmtab[newbt3.atmi[0]]->atmtp;
	query.atmtp[1] = atmtab[newbt3.atmi[1]]->atmtp;
	query.atmtp[2] = atmtab[newbt3.atmi[2]]->atmtp;
	query.atmtp[3] = atmtab[newbt3.atmi[3]]->atmtp;
	
	// the easiest way to get bondtypes is to find the corresponding bonds...
	// the easiest way to get bondtypes is to find the corresponding bonds...
	// the easiest way to get bondtypes is to find the corresponding bonds...
	
	bond tmpb1 = bond(atmtab[newbt3.atmi[0]], atmtab[newbt3.atmi[1]], bondtype());
	iter_bl itb1 = find(GetSetup()->GetModel()->GetBondsBegin(), GetSetup()->GetModel()->GetBondsEnd(), tmpb1);
	
	bond tmpb2 = bond(atmtab[newbt3.atmi[1]], atmtab[newbt3.atmi[2]], bondtype());
	iter_bl itb2 = find(GetSetup()->GetModel()->GetBondsBegin(), GetSetup()->GetModel()->GetBondsEnd(), tmpb2);
	
	bond tmpb3 = bond(atmtab[newbt3.atmi[2]], atmtab[newbt3.atmi[3]], bondtype());
	iter_bl itb3 = find(GetSetup()->GetModel()->GetBondsBegin(), GetSetup()->GetModel()->GetBondsEnd(), tmpb3);
	
	query.bndtp[0] = (* itb1).bt.GetValue();
	query.bndtp[1] = (* itb2).bt.GetValue();
	query.bndtp[2] = (* itb3).bt.GetValue();
	
	tab.DoParamSearch(& query);
	if (query.index == NOT_DEFINED) bt3_err++;
					
					newbt3.fc1 = query.fc1;
					newbt3.fc2 = query.fc2;
					newbt3.fc3 = query.fc3;
					
					newbt3.bt[0] = (* itb1).bt.GetValue();		// save also bondtype information...
					newbt3.bt[1] = (* itb2).bt.GetValue();		// save also bondtype information...
					newbt3.bt[2] = (* itb3).bt.GetValue();		// save also bondtype information...
					
					newbt3.constraint = false;
					bt3_vector.push_back(newbt3);
				}
			}
		}
	}
	
	if (ostr != NULL) (* ostr) << bt3_vector.size() << " terms, " << bt3_err << " errors." << endl;
	
/*##############################################*/
/*##############################################*/

	// create bt4-terms...
	
	if (ostr != NULL) (* ostr) << "creating bt4-terms: ";
	i32s bt4_err = 0;
	
	for (i32s n1 = 0;n1 < GetSetup()->GetMMAtomCount();n1++)
	{
		const prmfit_at * tp = tab.GetAtomType(atmtab[n1]->atmtp);
		if (!tp || (tp != NULL && !(tp->flags & 16))) continue;
		
		if (atmtab[n1]->cr_list.size() != 3) continue;
		
		crec * crtab[3];
		iter_cl iter = atmtab[n1]->cr_list.begin();
		crtab[0] = & (* iter); iter++;
		crtab[1] = & (* iter); iter++;
		crtab[2] = & (* iter);
		
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			mm_prmfit_bt4 newbt4;
			
			newbt4.index1[0] = crtab[(n2 + 0) % 3]->bndr->tmp_bt1_index;
			newbt4.dir1[0] = (bt1_vector[newbt4.index1[0]].get_atmi(0, true) == n1);
			
			newbt4.index1[1] = crtab[(n2 + 1) % 3]->bndr->tmp_bt1_index;
			newbt4.dir1[1] = (bt1_vector[newbt4.index1[1]].get_atmi(0, true) == n1);
			
			newbt4.index1[2] = crtab[(n2 + 2) % 3]->bndr->tmp_bt1_index;
			newbt4.dir1[2] = (bt1_vector[newbt4.index1[2]].get_atmi(0, true) == n1);
			
			newbt4.index2 = 0;
			while (newbt4.index2 < (i32s) bt2_vector.size())
			{
				bool test[4];
				
				newbt4.dir2 = true;
				test[0] = (bt2_vector[newbt4.index2].get_index(0, newbt4.dir2) == newbt4.index1[0]);
				test[1] = (bt2_vector[newbt4.index2].get_dir(0, newbt4.dir2) == newbt4.dir1[0]);
				test[2] = (bt2_vector[newbt4.index2].get_index(1, newbt4.dir2) == newbt4.index1[1]);
				test[3] = (bt2_vector[newbt4.index2].get_dir(1, newbt4.dir2) == newbt4.dir1[1]);
				if (test[0] && test[1] && test[2] && test[3]) break;
				
				newbt4.dir2 = false;
				test[0] = (bt2_vector[newbt4.index2].get_index(0, newbt4.dir2) == newbt4.index1[0]);
				test[1] = (bt2_vector[newbt4.index2].get_dir(0, newbt4.dir2) == newbt4.dir1[0]);
				test[2] = (bt2_vector[newbt4.index2].get_index(1, newbt4.dir2) == newbt4.index1[1]);
				test[3] = (bt2_vector[newbt4.index2].get_dir(1, newbt4.dir2) == newbt4.dir1[1]);
				if (test[0] && test[1] && test[2] && test[3]) break;
				
				newbt4.index2++;
			}
			
			if (newbt4.index2 == (i32s) bt2_vector.size())
			{
				cout << "BUG: bt2 missing for bt4!!!" << endl;
				exit(EXIT_FAILURE);
			}
			
			i32s iglob[4] =
			{
				crtab[(n2 + 0) % 3]->atmr->varind,
				atmtab[n1]->varind,
				crtab[(n2 + 1) % 3]->atmr->varind,
				crtab[(n2 + 2) % 3]->atmr->varind
			};
			
			for (i32s n9 = 0;n9 < 4;n9++)
			{
				i32s index = 0;
				while (index < GetSetup()->GetMMAtomCount())
				{
					if (glob_atmtab[iglob[n9]] == atmtab[index]) break;
					else index++;
				}
				
				if (index >= GetSetup()->GetMMAtomCount())
				{
					cout << "ERROR : iloc search failed!!!" << endl;
					exit(EXIT_FAILURE);
				}
				
				newbt4.atmi[n9] = index;
			}
if (newbt4.atmi[1] != n1) { cout << "OOPS THIS IS NOT WORKING!!!" << endl; exit(EXIT_FAILURE); }
			
	prmfit_op_query query; query.strict = false;
	query.atmtp[0] = atmtab[newbt4.atmi[0]]->atmtp;
	query.atmtp[1] = atmtab[newbt4.atmi[1]]->atmtp;
	query.atmtp[2] = atmtab[newbt4.atmi[2]]->atmtp;
	query.atmtp[3] = atmtab[newbt4.atmi[3]]->atmtp;
	
	query.bndtp[0] = crtab[(n2 + 0) % 3]->bndr->bt.GetValue();
	query.bndtp[1] = crtab[(n2 + 1) % 3]->bndr->bt.GetValue();
	query.bndtp[2] = crtab[(n2 + 2) % 3]->bndr->bt.GetValue();
	
	tab.DoParamSearch(& query);
	if (query.index == NOT_DEFINED) bt4_err++;
	
			newbt4.opt = query.opt;
			newbt4.fc = query.fc;
			
			newbt4.bt[0] = crtab[(n2 + 0) % 3]->bndr->bt.GetValue();	// save also bondtype information...
			newbt4.bt[1] = crtab[(n2 + 1) % 3]->bndr->bt.GetValue();	// save also bondtype information...
			newbt4.bt[2] = crtab[(n2 + 2) % 3]->bndr->bt.GetValue();	// save also bondtype information...
			
			bt4_vector.push_back(newbt4);
		}
	}
	
	if (ostr != NULL) (* ostr) << bt4_vector.size() << " terms, " << bt4_err << " errors." << endl;
	
/*##############################################*/
/*##############################################*/

	// report possible errors...
	
	total_err = bt1_err + bt2_err + bt3_err + bt4_err;
	if (total_err)
	{
		char mbuff1[256];
		ostrstream str1(mbuff1, sizeof(mbuff1));
		str1 << "WARNING : there were " << total_err << " missing parameters in the bonded terms." << endl << ends;
		GetSetup()->GetModel()->PrintToLog(mbuff1);
	}
	
/*##############################################*/
/*##############################################*/

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
			
			if (test == range_cr1[ind1 + 1])
			{
				test = range_cr2[ind1];
				while (test < range_cr2[ind1 + 1])
				{
					if (cr2[test] == atmtab[ind2]) break;
					else test++;
				}
				
				bool is14 = (test != range_cr2[ind1 + 1]);
				
				mm_prmfit_nbt1 newnbt1;
				newnbt1.atmi[0] = ind1;		// this is a local index...
				newnbt1.atmi[1] = ind2;		// this is a local index...
				
	const prmfit_at * at;
	
	f64 r1 = 0.150; f64 e1 = 0.175;			// default...
	at = tab.GetAtomType(atmtab[ind1]->atmtp);
	if (at != NULL) { r1 = at->vdw_R; e1 = at->vdw_E; }
	else nbt1_err++;

	f64 r2 = 0.150; f64 e2 = 0.175;			// default...
	at = tab.GetAtomType(atmtab[ind2]->atmtp);
	if (at != NULL) { r2 = at->vdw_R; e2 = at->vdw_E; }
	else nbt1_err++;
				
				f64 optdist = r1 + r2;
				f64 energy = sqrt(e1 * e2);
				
				f64 charge1 = atmtab[ind1]->charge;
				f64 charge2 = atmtab[ind2]->charge;
				newnbt1.qq = 138.9354518 * charge1 * charge2;
				
				if (is14)
				{
					energy *= 0.5;
					newnbt1.qq *= 0.75;
				}
				
				f64 tmp1 = optdist * pow(1.0 * energy, 1.0 / 12.0);
				f64 tmp2 = optdist * pow(2.0 * energy, 1.0 / 6.0);
				
				// ???
				
				newnbt1.kr = tmp1;
				newnbt1.kd = tmp2;
				
				nbt1_vector.push_back(newnbt1);
			}
		}
	}
	
	if (ostr != NULL) (* ostr) << nbt1_vector.size() << " terms, " << nbt1_err << " errors." << endl;
	
/*##############################################*/
/*##############################################*/

	// report possible errors...
	
	total_err = nbt1_err;
	if (total_err)
	{
		char mbuff1[256];
		ostrstream str1(mbuff1, sizeof(mbuff1));
		str1 << "WARNING : there were " << total_err << " missing parameters in the nonbonded terms." << endl << ends;
		GetSetup()->GetModel()->PrintToLog(mbuff1);
	}
}

eng1_mm_prmfit::~eng1_mm_prmfit(void)
{
	delete[] bt1data;
	delete[] bt2data;
}

i32s eng1_mm_prmfit::FindTorsion(atom * a1, atom * a2, atom * a3, atom * a4)
{
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

bool eng1_mm_prmfit::SetTorsionConstraint(i32s ind_bt3, f64 opt, f64 fc, bool lock_local_structure)
{
	if (ind_bt3 < 0) return false;
	if (ind_bt3 >= (i32s) bt3_vector.size()) return false;
	
	// check that opt is in valid range [-pi,+pi]!!!
	
	while (opt > +M_PI) opt -= 2.0 * M_PI;
	while (opt < -M_PI) opt += 2.0 * M_PI;
	
	// measure the current torsion and set constraints for the other torsions, if requested...
	
	if (lock_local_structure)
	{
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
				v3d<f64> v2a(& crd[l2g_mm[bt3_vector[n1].atmi[1]] * 3], & crd[l2g_mm[bt3_vector[n1].atmi[0]] * 3]);
				v3d<f64> v2b(& crd[l2g_mm[bt3_vector[n1].atmi[1]] * 3], & crd[l2g_mm[bt3_vector[n1].atmi[2]] * 3]);
				v3d<f64> v2c(& crd[l2g_mm[bt3_vector[n1].atmi[2]] * 3], & crd[l2g_mm[bt3_vector[n1].atmi[3]] * 3]);
				f64 local = v2a.tor(v2b, v2c) + delta;
				
				while (local > +M_PI) local -= 2.0 * M_PI;
				while (local < -M_PI) local += 2.0 * M_PI;
				
				bt3_vector[n1].constraint = true;
				bt3_vector[n1].fc1 = local;		// fc1 = opt
				bt3_vector[n1].fc2 = fc;		// fc2 = fc
			}
		}
	}
	
	// then set the requested constraint... if lock_local_structure was true, then
	// this operation is in fact redundant, but perhaps a bit more accurate than above.
	
	bt3_vector[ind_bt3].constraint = true;
	bt3_vector[ind_bt3].fc1 = opt;			// fc1 = opt
	bt3_vector[ind_bt3].fc2 = fc;			// fc2 = fc
	
	return true;
}

void eng1_mm_prmfit::ComputeBT1(i32u p1)
{
//printf("eng1_mm_prmfit::ComputeBT1 = %d\n", p1);
	energy_bt1 = 0.0;
	
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
		
		if (p1 > 0)
		{
			f64 t2c = 2.0 * bt1_vector[n1].fc * t2a;
			
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t2d = bt1data[n1].dlen[0][n2] * t2c;
				
				d1[l2g_mm[atmi[0]] * 3 + n2] += t2d;
				d1[l2g_mm[atmi[1]] * 3 + n2] -= t2d;
			}
		}
	}
}

void eng1_mm_prmfit::ComputeBT2(i32u p1)
{
//printf("eng1_mm_prmfit::ComputeBT2 = %d\n", p1);
	energy_bt2 = 0.0;
	
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
		
		f64 t2a;	// df/dx
		
		if (bt2_vector[n1].opt > NEAR_LINEAR_LIMIT)
		{
			// f = a(1 + x)
			// df/dx = a
			
			f64 t3b = bt2_vector[n1].fc;
			energy_bt2 += t3b * (1.0 + t1c);
			
			t2a = t3b;
		}
		else
		{
			// f = a(acos(x)-b)^2
			// df/dx = -2a(x-b)/sqrt(1-x*x)
			
			f64 t3b = acos(t1c) - bt2_vector[n1].opt;
			f64 t3c = bt2_vector[n1].fc * t3b * t3b;
			
			energy_bt2 += t3c;
			
			t2a = -2.0 * bt2_vector[n1].fc * t3b / sqrt(1.0 - t1c * t1c);
		}
		
		if (p1 > 0)
		{
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				d1[l2g_mm[atmi[0]] * 3 + n2] += bt2data[n1].dcsa[0][n2] * t2a;
				d1[l2g_mm[atmi[1]] * 3 + n2] += bt2data[n1].dcsa[1][n2] * t2a;
				d1[l2g_mm[atmi[2]] * 3 + n2] += bt2data[n1].dcsa[2][n2] * t2a;
			}
		}
	}
}

void eng1_mm_prmfit::ComputeBT3(i32u p1)
{
//printf("eng1_mm_prmfit::ComputeBT3 = %d\n", p1);
	energy_bt3 = 0.0;
	
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
		
		t1f[1] = t1f[0] + t1f[0];	// 2x
		t1f[2] = t1f[1] + t1f[0];	// 3x
		
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
			
			f64 t8a = t1f[0] - bt3_vector[n1].fc1;
			if (t8a > +M_PI)
			{
				t8a = 2.0 * M_PI - t8a;
				f64 t8b = t8a * t8a;
				
				t9a = bt3_vector[n1].fc2 * t8b * t8b;
				t9b = -4.0 * bt3_vector[n1].fc2 * t8b * t8a;
			}
			else if (t8a < -M_PI)
			{
				t8a = 2.0 * M_PI + t8a;
				f64 t8b = t8a * t8a;
				
				t9a = bt3_vector[n1].fc2 * t8b * t8b;
				t9b = +4.0 * bt3_vector[n1].fc2 * t8b * t8a;
			}
			else
			{
				f64 t8b = t8a * t8a;
				
				t9a = bt3_vector[n1].fc2 * t8b * t8b;
				t9b = 4.0 * bt3_vector[n1].fc2 * t8b * t8a;
			}
		}
		else
		{
			// f = a*cos(x)+b*cos(2x)+c*cos(3x)
			// df/dx = -a*sin(x)-2b*sin(2x)-3c*sin(3x)
			
			f64 t8a = bt3_vector[n1].fc1 * cos(t1f[0]);
			f64 t8b = bt3_vector[n1].fc2 * cos(t1f[1]);
			f64 t8c = bt3_vector[n1].fc3 * cos(t1f[2]);
			t9a = t8a + t8b + t8c;
			
			f64 t8r = bt3_vector[n1].fc1 * sin(t1f[0]);
			f64 t8s = bt3_vector[n1].fc2 * sin(t1f[1]) * 2.0;
			f64 t8t = bt3_vector[n1].fc3 * sin(t1f[2]) * 3.0;
			t9b = -(t8r + t8s + t8t);
		}
		
		energy_bt3 += t9a;
		
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
				
				d1[l2g_mm[atmi[0]] * 3 + n2] += t4c[n2] * t9b;
				d1[l2g_mm[atmi[3]] * 3 + n2] += t5c[n2] * t9b;
				
				t6a[n2] = (t2b - 1.0) * t4c[n2] - t3b * t5c[n2];
				t7a[n2] = (t3b - 1.0) * t5c[n2] - t2b * t4c[n2];
				
				d1[l2g_mm[atmi[1]] * 3 + n2] += t6a[n2] * t9b;
				d1[l2g_mm[atmi[2]] * 3 + n2] += t7a[n2] * t9b;
			}
		}
	}
}

void eng1_mm_prmfit::ComputeBT4(i32u p1)
{
//printf("eng1_mm_prmfit::ComputeBT4 = %d\n", p1);
	energy_bt4 = 0.0;
	
	for (i32s n1 = 0;n1 < (i32s) bt4_vector.size();n1++)
	{
		i32s * atmi = bt4_vector[n1].atmi;
		
		i32s index2 = bt4_vector[n1].index2;
		bool dir2 = bt4_vector[n1].dir2;
		i32s * index1 = bt4_vector[n1].index1;
		bool * dir1 = bt4_vector[n1].dir1;
		
		f64 t1a[3];
		t1a[0] = bt1data[index1[0]].dlen[dir1[0]][1] * bt1data[index1[1]].dlen[dir1[1]][2] -
			bt1data[index1[0]].dlen[dir1[0]][2] * bt1data[index1[1]].dlen[dir1[1]][1];
		t1a[1] = bt1data[index1[0]].dlen[dir1[0]][2] * bt1data[index1[1]].dlen[dir1[1]][0] -
			bt1data[index1[0]].dlen[dir1[0]][0] * bt1data[index1[1]].dlen[dir1[1]][2];
		t1a[2] = bt1data[index1[0]].dlen[dir1[0]][0] * bt1data[index1[1]].dlen[dir1[1]][1] -
			bt1data[index1[0]].dlen[dir1[0]][1] * bt1data[index1[1]].dlen[dir1[1]][0];
		
		f64 t1b = 0.0;
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			t1b += bt1data[index1[2]].dlen[dir1[2]][n2] * t1a[n2];
		}
		
		f64 t1c = 1.0 - bt2data[index2].csa * bt2data[index2].csa;
		f64 t1d = sqrt(t1c);
		
		f64 t1e = t1b / t1d;
		
//cout << "t1e = " << t1e << " " << t1e * t1e << endl;
//v3d<f64> v1(crd[l2g_mm[atmi[1]]], crd[l2g_mm[atmi[0]]]);
//v3d<f64> v2(crd[l2g_mm[atmi[1]]], crd[l2g_mm[atmi[2]]]);
//v3d<f64> v3(crd[l2g_mm[atmi[1]]], crd[l2g_mm[atmi[3]]]);
//v3d<f64> v4 = v1.vpr(v2);
//f64 ang = v3.ang(v4);
//cout << "check = " << cos(ang) << " " << cos(ang) * cos(ang) << endl;
//int zzz; cin >> zzz;

		if (t1e < -1.0) t1e = -1.0;		// domain check...
		if (t1e > +1.0) t1e = +1.0;		// domain check...
		
		// f = a(asin(x)-b)^2
		// df/dx = 2a(asin(x)-b)/sqrt(1-x^2)
		
		f64 t1f = asin(t1e) - bt4_vector[n1].opt;
		energy_bt4 += bt4_vector[n1].fc * t1f * t1f;
		
		if (p1 > 0)
		{
			f64 t1g = 2.0 * bt4_vector[n1].fc * t1f / sqrt(1.0 - t1e * t1e);
	//		f64 t1h = bt2data[index2].csa / t1d;
			
			f64 t9b = 1.0 - bt2data[index2].csa * bt2data[index2].csa;
			f64 t9c = sqrt(t9b);
			
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				// this code is borrowed from eng1_sf.cpp file...
				// might not be optimal, just a quick way to go ahead.
				
	f64 t6a[2];	// epsilon i
	t6a[0] = bt2data[index2].dcsa[dir2 ? 0 : 2][n2] * bt2data[index2].csa / t9b;
	t6a[1] = bt2data[index2].dcsa[dir2 ? 2 : 0][n2] * bt2data[index2].csa / t9b;
	
	f64 t6b[2];	// sigma ii / r2X
	t6b[0] = (1.0 - bt1data[index1[0]].dlen[dir1[0]][n2] * bt1data[index1[0]].dlen[dir1[0]][n2]) / bt1data[index1[0]].len;
	t6b[1] = (1.0 - bt1data[index1[1]].dlen[dir1[1]][n2] * bt1data[index1[1]].dlen[dir1[1]][n2]) / bt1data[index1[1]].len;
	
	i32s n3[2];
	n3[0] = (n2 + 1) % 3;		// index j
	n3[1] = (n2 + 2) % 3;		// index k
	
	f64 t6c[2];	// sigma ij / r2X
	t6c[0] = -bt1data[index1[0]].dlen[dir1[0]][n2] * bt1data[index1[0]].dlen[dir1[0]][n3[0]] / bt1data[index1[0]].len;
	t6c[1] = -bt1data[index1[1]].dlen[dir1[1]][n2] * bt1data[index1[1]].dlen[dir1[1]][n3[0]] / bt1data[index1[1]].len;
	
	f64 t6d[2];	// sigma ik / r2X
	t6d[0] = -bt1data[index1[0]].dlen[dir1[0]][n2] * bt1data[index1[0]].dlen[dir1[0]][n3[1]] / bt1data[index1[0]].len;
	t6d[1] = -bt1data[index1[1]].dlen[dir1[1]][n2] * bt1data[index1[1]].dlen[dir1[1]][n3[1]] / bt1data[index1[1]].len;
	
	f64 t5a[2][3];	// components of dc/di
	t5a[0][n2] = (t6c[0] * bt1data[index1[1]].dlen[dir1[1]][n3[1]] -
		t6d[0] * bt1data[index1[1]].dlen[dir1[1]][n3[0]] + t1a[n2] * t6a[0]) / t9c;
	t5a[0][n3[0]] = (t6d[0] * bt1data[index1[1]].dlen[dir1[1]][n2] -
		t6b[0] * bt1data[index1[1]].dlen[dir1[1]][n3[1]] + t1a[n3[0]] * t6a[0]) / t9c;
	t5a[0][n3[1]] = (t6b[0] * bt1data[index1[1]].dlen[dir1[1]][n3[0]] -
		t6c[0] * bt1data[index1[1]].dlen[dir1[1]][n2] + t1a[n3[1]] * t6a[0]) / t9c;
	t5a[1][n2] = (t6d[1] * bt1data[index1[0]].dlen[dir1[0]][n3[0]] -
		t6c[1] * bt1data[index1[0]].dlen[dir1[0]][n3[1]] + t1a[n2] * t6a[1]) / t9c;
	t5a[1][n3[0]] = (t6b[1] * bt1data[index1[0]].dlen[dir1[0]][n3[1]] -
		t6d[1] * bt1data[index1[0]].dlen[dir1[0]][n2] + t1a[n3[0]] * t6a[1]) / t9c;
	t5a[1][n3[1]] = (t6c[1] * bt1data[index1[0]].dlen[dir1[0]][n2] -
		t6b[1] * bt1data[index1[0]].dlen[dir1[0]][n3[0]] + t1a[n3[1]] * t6a[1]) / t9c;
				
				f64 tmp1a = t5a[0][0] * bt1data[index1[2]].dlen[dir1[2]][0] +
					t5a[0][1] * bt1data[index1[2]].dlen[dir1[2]][1] +
					t5a[0][2] * bt1data[index1[2]].dlen[dir1[2]][2];
				
				f64 tmp3a = t5a[1][0] * bt1data[index1[2]].dlen[dir1[2]][0] +
					t5a[1][1] * bt1data[index1[2]].dlen[dir1[2]][1] +
					t5a[1][2] * bt1data[index1[2]].dlen[dir1[2]][2];
				
	f64 tmp4a = 0.0;
	for (i32s n4 = 0;n4 < 3;n4++)
	{
		f64 tmp4b;
		if (n2 != n4) tmp4b = -bt1data[index1[2]].dlen[!dir1[2]][n2] * bt1data[index1[2]].dlen[!dir1[2]][n4];
		else tmp4b = 1.0 - bt1data[index1[2]].dlen[!dir1[2]][n4] * bt1data[index1[2]].dlen[!dir1[2]][n4];
		tmp4a += (tmp4b / bt1data[index1[2]].len) * (t1a[n4] / t1d);
	}
				
				d1[l2g_mm[atmi[0]] * 3 + n2] += tmp1a * t1g;
				d1[l2g_mm[atmi[1]] * 3 + n2] -= (tmp1a + tmp3a + tmp4a) * t1g;
				d1[l2g_mm[atmi[2]] * 3 + n2] += tmp3a * t1g;
				d1[l2g_mm[atmi[3]] * 3 + n2] += tmp4a * t1g;
			}
		}
	}
}

void eng1_mm_prmfit::ComputeNBT1(i32u p1)
{
//printf("eng1_mm_prmfit::ComputeNBT1 = %d\n", p1);
	energy_nbt1a = 0.0;
	energy_nbt1b = 0.0;
	energy_nbt1c = 0.0;
	energy_nbt1d = 0.0;
	
	for (i32s n1 = 0;n1 < (i32s) nbt1_vector.size();n1++)
	{
		i32s * atmi = nbt1_vector[n1].atmi;
		
		f64 t1a[3]; f64 t1b = 0.0;
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			f64 t2a = crd[l2g_mm[atmi[0]] * 3 + n2];
			f64 t2b = crd[l2g_mm[atmi[1]] * 3 + n2];
			
			t1a[n2] = t2a - t2b;
			t1b += t1a[n2] * t1a[n2];
		}
		
		f64 t1c = sqrt(t1b);
		
		// f1 = (r/a)^-12 - (r/b)^-6
		// df1/dr = -12/a(r/a)^-13 + 6/b(r/b)^-7
		
		f64 t3a = t1c / nbt1_vector[n1].kr;
		f64 t3b = t1c / nbt1_vector[n1].kd;
		
		f64 t4a = t3a * t3a * t3a; f64 t4b = t4a * t4a; f64 t4c = t4b * t4b;	// ^3 ^6 ^12
		f64 t5a = t3b * t3b * t3b; f64 t5b = t5a * t5a;				// ^3 ^6
		
		f64 t6a = 1.0 / (t4c) - 1.0 / (t5b);
		energy_nbt1a += t6a;
		
		// f2 = Q/r
		// df2/dr = -Q/r^2
		
		f64 t6b = nbt1_vector[n1].qq / t1c;
		energy_nbt1b += t6b;
		
		if (p1 > 0)
		{
			f64 t7a = 12.0 / (nbt1_vector[n1].kr * t4c * t3a);
			f64 t7b = 6.0 / (nbt1_vector[n1].kd * t5b * t3b);
			
			f64 t8a = nbt1_vector[n1].qq / t1b;
			
			f64 t9a = t7b - t7a - t8a;
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t9b = (t1a[n2] / t1c) * t9a;
				
				d1[l2g_mm[atmi[0]] * 3 + n2] += t9b;
				d1[l2g_mm[atmi[1]] * 3 + n2] -= t9b;
			}
		}
	}
}

/*################################################################################################*/

// eof
