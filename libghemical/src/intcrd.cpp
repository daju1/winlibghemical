// INTCRD.CPP

// Copyright (C) 2001 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "intcrd.h"

#include "v3d.h"
#include "libdefine.h"

#include "atom.h"
#include "bond.h"

#include <iostream>
using namespace std;

/*################################################################################################*/

ic_data::ic_data(void)
{
	previous = NULL;
	atmr = NULL;
	
	is_variable = false;
}

ic_data::~ic_data(void)
{
}

/*################################################################################################*/

intcrd::intcrd(model & p1, i32s p2, i32s p3)
{
	mdl = (& p1);
	molnum = p2; crdset = p3;
	
	fGL tmp1[3] = { 1.0, 0.0, 0.0 };
	fGL tmp2[3] = { 1.0, 1.0, 0.0 };
	fGL tmp3[3] = { 1.0, 1.0, 1.0 };
	
	base[0].atmr = new atom(element(6), tmp1, mdl->GetCRDSetCount()); base[0].previous = NULL;
	base[1].atmr = new atom(element(6), tmp2, mdl->GetCRDSetCount()); base[1].previous = & base[0];
	base[2].atmr = new atom(element(6), tmp3, mdl->GetCRDSetCount()); base[2].previous = & base[1]; base[2].torv = 0.0;
	
	if (mdl->GetMoleculeCount() < 0)	// you must call mdl->GatherGroups before using this class!!!
	{
		cout << "BUG: invalid model at intcrd ctor." << endl;
		exit(EXIT_FAILURE);
	}
	
	// get the atom range for the molecule.
	
	iter_al range[2];
	mdl->GetRange(0, molnum, range);
	
	if (range[0] == mdl->GetAtomsEnd())
	{
		cout << "BUG: invalid molecule at intcrd ctor." << endl;
		exit(EXIT_FAILURE);
	}
	
	// find the best atom (the one with least bonds) to work as an "anchor".
	
	iter_al anchor = range[0];
	i32u nbonds = (* anchor).cr_list.size();
	
	for (iter_al it1 = range[0];it1 != range[1];it1++)
	{
		if ((* it1).cr_list.size() < nbonds)
		{
			anchor = it1;
			nbonds = (* anchor).cr_list.size();
		}
	}
	
	// add it...
	
	vector<atom *> added_atoms;
	
	added_atoms.push_back(& (* anchor));
	AddNewPoint(& (* anchor), & base[2], false);
	
	// ...and also the rest of the atoms.
	
	while (true)
	{
		// first, try to find a not-yet-added atom
		// that is connected to an already-added one.
		
		i32s added_index = NOT_DEFINED;
		crec * matching_cr = NULL;
		
		iter_al it1 = range[0];
		while (it1 != range[1])
		{
			i32u search1 = 0;
			while (search1 < added_atoms.size())
			{
				if (added_atoms[search1] == & (* it1)) break;
				else search1++;
			}
			
			if (search1 != added_atoms.size())
			{
				it1++;
				continue;
			}
			
			i32s search2 = NOT_DEFINED;
			iter_cl it2 = (* it1).cr_list.begin();
			while (it2 != (* it1).cr_list.end())
			{
				search2 = 0;
				while (search2 < (i32s) added_atoms.size())
				{
					if (added_atoms[search2] == (* it2).atmr) break;
					else search2++;
				}
				
				if (search2 != (i32s) added_atoms.size()) break;
				else it2++;
			}
			
			if (it2 != (* it1).cr_list.end())
			{
				added_index = search2;
				matching_cr = & (* it2);
				break;
			}
			else it1++;
		}
		
		// if none was found, all atoms are added,
		// so we can leave the loop.
		
		if (it1 == range[1]) break;
		
		// now determine if we should mark this as variable...
		
		bool flag = true;
		
		if (added_atoms.size() < 2) flag = false;			// skip the first one.
		
		if ((* it1).cr_list.size() < 2) flag = false;			// there are further bonds.
		
		if (matching_cr->bndr->bt.GetSymbol1() == 'D') flag = false;	// double bonds are fixed.
		
		matching_cr->bndr->flags[0] = true;				// ring structures are fixed.
		if (mdl->FindPath(& (* it1), matching_cr->atmr, NOT_DEFINED, 0) != NOT_FOUND) flag = false;
		matching_cr->bndr->flags[0] = false;
		
		// AMIDE GROUPS!??!!?!
		// AMIDE GROUPS!??!!?!
		// AMIDE GROUPS!??!!?!
		
		// ...and add it.
		
		added_atoms.push_back(& (* it1));
		AddNewPoint(& (* it1), ic_vector[added_index], flag);
	}
}

intcrd::~intcrd(void)
{
	for (i32u n1 = 0;n1 < ic_vector.size();n1++)
	{
		delete ic_vector[n1];
	}
	
	delete base[0].atmr;
	delete base[1].atmr;
	delete base[2].atmr;
}

void intcrd::AddNewPoint(atom * atmr, ic_data * prev, bool is_variable)
{
//cout << "add BEGIN - ";
/////////////////////////////////////
/////////////////////////////////////
	ic_data * newic = new ic_data();
	
	newic->atmr = atmr;
	newic->previous = prev;
	
	ic_data * prev2 = prev->previous;
	ic_data * prev3 = prev2->previous;
	
	v3d<fGL> v1(prev->atmr->GetCRD(crdset), atmr->GetCRD(crdset));
	newic->len = v1.len();
	
	v3d<fGL> v2(prev->atmr->GetCRD(crdset), prev2->atmr->GetCRD(crdset));
	newic->ang = v1.ang(v2);
	
	v3d<fGL> v3(prev2->atmr->GetCRD(crdset), prev3->atmr->GetCRD(crdset));
	newic->torc = v1.tor(v2, v3); newic->torv = 0.0;
	
	newic->is_variable = is_variable;
	if (is_variable) variable_index_vector.push_back(ic_vector.size());
	
	ic_vector.push_back(newic);
/////////////////////////////////////
/////////////////////////////////////
//cout << "add END" << endl;
}

void intcrd::UpdateCartesian(void)
{
	for (i32u n1 = 0;n1 < ic_vector.size();n1++)
	{
		ic_data * curr = ic_vector[n1];
		ic_data * prev = curr->previous;
		ic_data * prev2 = prev->previous;
		ic_data * prev3 = prev2->previous;
		
		fGL tor = curr->torc + prev->torv;
		v3d<fGL> tmpvA(prev2->atmr->GetCRD(crdset), prev3->atmr->GetCRD(crdset));
		
		v3d<fGL> v1(prev->atmr->GetCRD(crdset), prev2->atmr->GetCRD(crdset));
		
		v3d<fGL> tmpvB = v1 * (v1.spr(tmpvA) / (v1.len() * v1.len()));
		v3d<fGL> v2 = tmpvA - tmpvB;
		
		v3d<fGL> v3 = tmpvA.vpr(v1);
		
		v1 = v1 * (curr->len * cos(curr->ang)) / v1.len();
		v2 = v2 * (curr->len * sin(curr->ang) * cos(tor)) / v2.len();
		v3 = v3 * (curr->len * sin(curr->ang) * sin(tor)) / v3.len();
		
		v3d<fGL> tmpvC = ((v1 + v2) + v3);
		
		const fGL * cdata = prev->atmr->GetCRD(crdset);
		fGL x = cdata[0] + tmpvC[0];
		fGL y = cdata[1] + tmpvC[1];
		fGL z = cdata[2] + tmpvC[2];
		
		curr->atmr->SetCRD(crdset, x, y, z);
	}
}

i32s intcrd::GetVariableCount(void)
{
	return variable_index_vector.size();
}

i32s intcrd::FindVariable(atom * ref1, atom * ref2)
{
	i32s index2 = FindTorsion(ref1, ref2);
	if (index2 < 0) return NOT_DEFINED;
	
	// now try to find the torsion from variable vector...
	// now try to find the torsion from variable vector...
	// now try to find the torsion from variable vector...
	
	for (i32u index1 = 0;index1 < variable_index_vector.size();index1++)
	{
		if (variable_index_vector[index1] == (i32u) index2) return index1;
	}
	
	return NOT_DEFINED;
}

void intcrd::SetVariable(i32s index1, fGL value)
{
	i32u index2 = variable_index_vector[index1];
	ic_vector[index2]->torv = value;
}

fGL intcrd::GetVariable(i32s index1)
{
	i32u index2 = variable_index_vector[index1];
	return ic_vector[index2]->torv;
}

i32s intcrd::GetTorsionCount(void)
{
	return ic_vector.size();
}

i32s intcrd::FindTorsion(atom * ref1, atom * ref2)
{
	i32u n1 = 0;
	while (n1 < ic_vector.size())
	{
		if (ic_vector[n1]->atmr == ref1 && ic_vector[n1]->previous->atmr == ref2) return n1;
		if (ic_vector[n1]->atmr == ref2 && ic_vector[n1]->previous->atmr == ref1) return n1;
		
		n1++;
	}
	
	return NOT_DEFINED;
}

void intcrd::SetTorsion(i32s index, fGL value)
{
	ic_vector[index]->torv = value;
}

fGL intcrd::GetTorsion(i32s index)
{
	return ic_vector[index]->torv;
}

/*################################################################################################*/

// eof
