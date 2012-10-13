// MODEL.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "model.h"

#include "eng1_qm.h"
#include "eng1_mm.h"
#include "eng1_sf.h"
#include "eng2_qm_mm.h"
#include "eng2_mm_sf.h"

#include "resonance.h"

#include "tab_mm_default.h"

#include "boundary_opt.h"
#include "geomopt.h"
#include "moldyn.h"
#include "search.h"

#include <stdio.h>

#include <iomanip>
#include <sstream>
#include <strstream>//fixme
using namespace std;

bool ReadGPR(model &, istream &, bool, bool);			// see fileio.cpp!!!

/*################################################################################################*/
const char model::libversion[16] = LIBVERSION;
const char model::libdata_path[256] = LIBDATA_PATH;



sequencebuilder model::amino_builder(chn_info::amino_acid, AMINO_BUILDER_FILE);
sequencebuilder model::nucleic_builder(chn_info::nucleic_acid, NUCLEIC_BUILDER_FILE);

// sf_symbols and sf_types are in the same order.

const char model::sf_symbols[20 + 1] = "ARNDCQEGHILKMFPSTWYV";

const i32s model::sf_types[SF_NUM_TYPES] =
{
	0x0000,				// A
	0x0100, 0x0101, 0x0102,		// R
	0x0200, 0x0201,			// N
	0x0300, 0x0301,			// D
	0x0400, 0x0401,			// C
	0x0500, 0x0501,			// Q
	0x0600, 0x0601,			// E
	0x0700,				// G
	0x0800, 0x0801,			// H
	0x0900, 0x0901,			// I
	0x0A00, 0x0A01,			// L
	0x0B00, 0x0B01, 0x0B02,		// K
	0x0C00, 0x0C01,			// M
	0x0D00, 0x0D01,			// F
	0x0E00,				// P
	0x0F00,				// S
	0x1000,				// T
	0x1100, 0x1101, 0x1102,		// W
	0x1200, 0x1201,			// Y
	0x1300				// V
};

const bool model::sf_is_polar[SF_NUM_TYPES] =
{
	true,				// A
	true, false, true,		// R
	true, true,			// N
	true, true,			// D
	true, false,			// C
	true, true,			// Q
	true, true,			// E
	true,				// G
	true, true,			// H
	true, false,			// I
	true, false,			// L
	true, false, true,		// K
	true, false,			// M
	true, false,			// F
	true,				// P
	true,				// S
	true,				// T
	true, true, false,		// W
	true, true,			// Y
	false				// V
};

bool ReadTargetListFile(char * filename, vector<i32s>& target_list)
{
	target_list.clear();
	if (filename)
	{
		FILE * stream = fopen(filename, "rt");
		if (stream)
		{
			const int n = 256;
			char szBuff[n];
			while (!feof(stream))
			{
				char * ch = fgets(szBuff,n,stream);
				if( ch != NULL && strlen(szBuff) > 1)
				{
					i32s v = atoi(szBuff);
					target_list.push_back(v);
				}
			}

			fclose(stream);
			return true;
		}
	}
	return false;
}


model::model(void)
{
printf("model::model(void)\n\n");

	m_bMaxMinCoordCalculed = false;

	current_setup = new setup1_mm(this);
	rs = NULL;
	
	crd_table_size_glob = 1;
	cs_vector.push_back(new crd_set());
	SetCRDSetVisible(0, true);
	
	is_index_clean = false;
	is_groups_clean = false;
	is_groups_sorted = false;
	
	qm_total_charge = 0;
	qm_current_orbital = 0;
	
	use_boundary_potential = false;
	boundary_potential_radius1 = 1.0;
	boundary_potential_radius2 = 1.0;
	
	use_periodic_boundary_conditions = false;
	periodic_box_HALFdim[0] = 1.0;
	periodic_box_HALFdim[1] = 1.0;
	periodic_box_HALFdim[2] = 1.0;
	
	nmol = NOT_DEFINED;
	ref_civ = NULL;
}

model::~model(void)
{
	if (current_setup != NULL)
	{
		delete current_setup;
		current_setup = NULL;
	}
	else
	{
		cout << "FIXME : current_setup was NULL at model dtor." << endl;
		exit(EXIT_FAILURE);
	}
	
	if (rs != NULL) delete rs;
	
	for (i32u n1 = 0;n1 < cs_vector.size();n1++)
	{
		delete cs_vector[n1];
	}
}

void model::ThreadLock(void)
{
}

void model::ThreadUnlock(void)
{
}

bool model::SetProgress(double, double *)
{
	return false;
}

void model::Message(const char * msg)
{
	cerr << msg << endl;
}

void model::WarningMessage(const char * msg)
{
	cerr << msg << endl;
}

void model::ErrorMessage(const char * msg)
{
	cerr << msg << endl;
}

setup * model::GetCurrentSetup(void)
{
	return current_setup;
}

resonance_structures * model::GetRS(void)
{
	return rs;
}

void model::CreateRS(void)
{
	if (rs == NULL) rs = new resonance_structures(this);
}

void model::DestroyRS(void)
{
	if (rs != NULL)
	{
		delete rs;
		rs = NULL;
	}
}

void model::OpenLibDataFile(ifstream & file, bool is_binary_file, const char * fn)
{
	ostringstream oss;
	oss << model::libdata_path << DIR_SEPARATOR << model::libversion << DIR_SEPARATOR << fn << ends;
	
	//cout << "DEBUG ; preparing to open file " << oss.str() << endl;
	
	if (!is_binary_file) file.open(oss.str().c_str(), ios::in);
	else 
	{
		cerr << " file binary " << oss.str() << endl;
		file.open(oss.str().c_str(), ios::in | ios::binary);
	}
	if (file.good())
	{
		return;
	}
	
	file.close();
	
	cerr << "Error : could not open file " << oss.str() << endl;
	exit(EXIT_FAILURE);
}

void model::DiscardCurrEng(void)
{
//	cout << "discard!!!" << endl;
	GetCurrentSetup()->DiscardCurrentEngine();
}

void model::SetupPlotting(void)
{
	engine * eng = GetCurrentSetup()->GetCurrentEngine();
	if (eng != NULL) eng->SetupPlotting();
}

/*##############################################*/
/*##############################################*/

i32u model::GetCRDSetCount(void)
{
	return cs_vector.size();
}

bool model::GetCRDSetVisible(i32u index)
{
	if (index < cs_vector.size())
	{
		return cs_vector[index]->visible;
	}
	else
	{
		cout << "BUG: invalid call to model::GetCRDSetVisible()" << endl;
		exit(EXIT_FAILURE);
	}
}

void model::SetCRDSetVisible(i32u index, bool visible)
{
	if (index < cs_vector.size())
	{
		cs_vector[index]->visible = visible;
	}
	else
	{
		cout << "BUG: invalid call to model::SetCRDSetVisible()" << endl;
		exit(EXIT_FAILURE);
	}
}

void model::PushCRDSets(i32u p1)
{
	i32u old_size = cs_vector.size();
	
	for (i32u n1 = 0;n1 < p1;n1++) cs_vector.push_back(new crd_set());
	
	i32u new_size = cs_vector.size();
	
	// determine whether we need to reallocate the crd_tables, and reallocate if needed.
	// determine whether we need to reallocate the crd_tables, and reallocate if needed.
	// determine whether we need to reallocate the crd_tables, and reallocate if needed.
	
	if (new_size > crd_table_size_glob)
	{
//		i32u old_cap = crd_table_size_glob;	this is actually not needed...
		i32u new_cap;
		
		new_cap = new_size;	// just allocate the minimum amount (simple).
		
		crd_table_size_glob = new_cap;
		fGL * buff = new fGL[old_size * 3];
		
		for (iter_al it1 = GetAtomsBegin();it1 != GetAtomsEnd();it1++)
		{
			for (i32u n1 = 0;n1 < old_size;n1++)
			{
				buff[n1 * 3 + 0] = (* it1).crd_table[n1 * 3 + 0];
				buff[n1 * 3 + 1] = (* it1).crd_table[n1 * 3 + 1];
				buff[n1 * 3 + 2] = (* it1).crd_table[n1 * 3 + 2];
			}
			
			delete[] (* it1).crd_table;
			(* it1).crd_table = new fGL[new_cap * 3];
			(* it1).crd_table_size_loc = new_cap;

			{
			for (i32u n1 = 0;n1 < old_size;n1++)
			{
				(* it1).crd_table[n1 * 3 + 0] = buff[n1 * 3 + 0];
				(* it1).crd_table[n1 * 3 + 1] = buff[n1 * 3 + 1];
				(* it1).crd_table[n1 * 3 + 2] = buff[n1 * 3 + 2];
			}
			}
		}
		
		delete[] buff;
	}
	
	// initialize the new memory blocks.
	// initialize the new memory blocks.
	// initialize the new memory blocks.
	
	for (iter_al it1 = GetAtomsBegin();it1 != GetAtomsEnd();it1++)
	{
		for (i32u n1 = old_size;n1 < new_size;n1++)
		{
			(* it1).crd_table[n1 * 3 + 0] = 0.0;
			(* it1).crd_table[n1 * 3 + 1] = 0.0;
			(* it1).crd_table[n1 * 3 + 2] = 0.0;
		}
	}
}

void model::PopCRDSets(i32u p1)
{
	// do not deallocate the memory; in the future deallocation could be added... 2003-06-12 TH
	// do not deallocate the memory; in the future deallocation could be added... 2003-06-12 TH
	// do not deallocate the memory; in the future deallocation could be added... 2003-06-12 TH
	
	for (i32u n1 = 0;n1 < p1;n1++)
	{
		delete cs_vector.back();
		cs_vector.pop_back();
	}
}

void model::CopyCRDSet(i32u p1, i32u p2)
{
	if (p1 >= crd_table_size_glob || p2 >= crd_table_size_glob)
	{
		cout << "BUG: cs overflow at model::CopyCRDSet()." << endl;
		exit(EXIT_FAILURE);
	}
	
	for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		(* it1).crd_table[p2 * 3 + 0] = (* it1).crd_table[p1 * 3 + 0];
		(* it1).crd_table[p2 * 3 + 1] = (* it1).crd_table[p1 * 3 + 1];
		(* it1).crd_table[p2 * 3 + 2] = (* it1).crd_table[p1 * 3 + 2];
	}
}

void model::SwapCRDSets(i32u, i32u)
{
cout << "Oops!!! This function is not yet ready." << endl;
}

void model::CenterCRDSet(i32u p1, bool all_atoms)
{
	if (p1 >= crd_table_size_glob)
	{
		cout << "BUG: cs overflow at model::CenterCRDSet()." << endl;
		exit(EXIT_FAILURE);
	}
	
	fGL sum[3] = { 0.0, 0.0, 0.0 };
	for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		if (!all_atoms && (* it1).flags & ATOMFLAG_IS_HIDDEN) continue;
		
		fGL * crd_table = (* it1).crd_table;
		for (i32s n1 = 0;n1 < 3;n1++) sum[n1] += crd_table[p1 * 3 + n1];
	}
	{
	
	for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		if (!all_atoms && (* it1).flags & ATOMFLAG_IS_HIDDEN) continue;
		
		fGL * crd_table = (* it1).crd_table;
		for (i32s n1 = 0;n1 < 3;n1++) crd_table[p1 * 3 + n1] -= sum[n1] / (fGL) GetAtomCount();
	}
	}
}

void model::ReserveCRDSets(i32u)
{
cout << "Oops!!! This function is not yet ready." << endl;
}

/*##############################################*/
/*##############################################*/

void model::Add_Atom(atom & p1)
{
	SystemWasModified();
//////////////////////////////////////////////////////////////////////////obsolete...
//	p1.index = (i32s) atom_list.size();	// try to keep the atom::index records up-to-date...
//////////////////////////////////////////////////////////////////////////obsolete...
	
	i32s added_index = (i32s) atom_list.size();
	atom_list.push_back(p1);
	
	atom_list.back().index = added_index;	// try to keep the atom::index records up-to-date...
	atom_list.back().mdl = this;
}

void model::RemoveAtom(iter_al it1)
{
	SystemWasModified();
	
	// this strange while-loop is needed because removing bonds
	// will invalidate all those atom::cr_list-iterators...
	
	while (!(* it1).cr_list.empty())
	{
		crec * ref = & (* it1).cr_list.back();
		iter_bl it2 = find(bond_list.begin(), bond_list.end(), (* ref->bndr));
		
		if (it2 != bond_list.end()) RemoveBond(it2);
		else
		{
			cout << "BUG: find failed at model::RemoveAtom(); n = " << (* it1).cr_list.size() << endl;
			exit(EXIT_FAILURE);
		}
	}
	
	(* it1).mdl = NULL;
	
	i32s removed_index = (* it1).index;
	atom_list.erase(it1);
	
	// try to keep the atom::index records up-to-date...
	
	for (it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		if ((* it1).index < removed_index) continue;
		else (* it1).index--;
	}
}

void model::AddBond(bond & p1)
{
	if (p1.atmr[0] == p1.atmr[1])
	{
		cout << "BUG1: tried to add an invalid bond at model::AddBond()!" << endl;
		exit(EXIT_FAILURE);
	}
	
	if (p1.atmr[0]->mdl != p1.atmr[1]->mdl || p1.atmr[0]->mdl == NULL)
	{
		//cout << "BUG2: tried to add an invalid bond at model::AddBond()!" << endl;
		//exit(EXIT_FAILURE);
		
		cout << "AddBond mdl check ; " << p1.atmr[0]->mdl << " " << p1.atmr[1]->mdl << endl;
	}
	
	SystemWasModified();
	
	bond_list.push_back(p1);
	
	crec info1 = crec(p1.atmr[1], & bond_list.back());
	p1.atmr[0]->cr_list.push_back(info1);
/*	if (p1.atmr[0]->cr_list.size() > 8)	// this is for debugging purposes only...
	{
		cout << "BUG: model::AddBond() : too many bonds detected!" << endl;
		exit(EXIT_FAILURE);
	}	*/
	
	crec info2 = crec(p1.atmr[0], & bond_list.back());
	p1.atmr[1]->cr_list.push_back(info2);
/*	if (p1.atmr[1]->cr_list.size() > 8)	// this is for debugging purposes only...
	{
		cout << "BUG: model::AddBond() : too many bonds detected!" << endl;
		exit(EXIT_FAILURE);
	}	*/
}

void model::RemoveBond(iter_bl it1)
{
	SystemWasModified();
	
	crec tmpinfo = crec(NULL, & (* it1));
	iter_cl it2;
	
	it2 = find((* it1).atmr[0]->cr_list.begin(), (* it1).atmr[0]->cr_list.end(), tmpinfo);
	if (it2 != (* it1).atmr[0]->cr_list.end()) (* it1).atmr[0]->cr_list.erase(it2);
	else
	{
		cout << "BUG: find #1 failed in RemoveBond()" << endl;
		exit(EXIT_FAILURE);
	}
	
	it2 = find((* it1).atmr[1]->cr_list.begin(), (* it1).atmr[1]->cr_list.end(), tmpinfo);
	if (it2 != (* it1).atmr[1]->cr_list.end()) (* it1).atmr[1]->cr_list.erase(it2);
	else
	{
		cout << "BUG: find #2 failed in RemoveBond()" << endl;
		exit(EXIT_FAILURE);
	}
	
	bond_list.erase(it1);
}

// this should be called ALWAYS if ANY modification is done to the system.
// automatically called by Add/Remove/Atom/Bond. GUI should change if change in element etc...

void model::SystemWasModified(void)
{
	// if atoms/bonds added/removed, all engine-objects have to be discarded.
	// also if element is changed, the MOPAC/MPQC QM engine-objects (at least) must be discarded.
	
	DiscardCurrEng();
	
	// in the setup object, all atom/bond tables must be discarded for the same reasons as above...
	
	current_setup->DiscardSetupInfo();
	
	// the resonance_structures object must be invalidated, if there is one...
	
	if (rs != NULL)
	{
		delete rs;
		rs = NULL;
	}
	
	// any change that might lead to change in detected sequences must invalidate the SF engine objects...
	
	if (dynamic_cast<setup1_sf *>(current_setup) != NULL)
//	if (current_setup)
	{
		delete current_setup;
		current_setup = new setup1_mm(this);
	}
	
	// any addition/removal of atoms/bonds will invalidate information about molecules.
	
	InvalidateGroups();
}

iter_al model::FindAtomByIndex(i32s index)
{
	iter_al end_it = GetAtomsEnd();
	if (index < 0) return end_it;
	
	i32s counter = 0;
	iter_al iter = GetAtomsBegin();
	while (counter != index)
	{
		counter++; iter++;
		if (iter == end_it) return end_it;
	}
	
	return iter;
}

void model::ClearModel(void)
{
	while (!bond_list.empty())
	{
		iter_bl it1 = bond_list.begin();
		RemoveBond(it1);
	}
	
	while (!atom_list.empty())
	{
		iter_al it1 = atom_list.begin();
		RemoveAtom(it1);
	}
}

/*##############################################*/
/*##############################################*/

void model::GetRange(i32s ind, i32s value, iter_al * result)
{
	iter_al range[2] = { atom_list.begin(), atom_list.end() };
	GetRange(ind, range, value, result);				// call GetRange() using full atom_list!!!
}

void model::GetRange(i32s ind, iter_al * range, i32s value, iter_al * result)
{
	if (!is_groups_sorted)
	{
		cout << "fatal error : model::GetRange() was called while model::IsGroupsSorted() is false!" << endl;
		exit(EXIT_FAILURE);
	}
	
	result[0] = range[0]; while (result[0] != range[1] && (* result[0]).id[ind] != value) result[0]++;
	result[1] = result[0]; while (result[1] != range[1] && (* result[1]).id[ind] == value) result[1]++;
}

void model::GetRange(i32s molecule, iter_bl * result)
{
	if (!is_groups_sorted)
	{
		cout << "fatal error : model::GetRange() was called while model::IsGroupsSorted() is false!" << endl;
		exit(EXIT_FAILURE);
	}
	
	// assume that the molecule numbers of both atoms in a bond object are identical!!!
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	result[0] = bond_list.begin(); while (result[0] != bond_list.end() && (* result[0]).atmr[0]->id[0] != molecule) result[0]++;
	result[1] = result[0]; while (result[1] != bond_list.end() && (* result[1]).atmr[0]->id[0] == molecule) result[1]++;
}

i32s model::FindPath(atom * ref1, atom * ref2, i32s max, i32s flag, i32s dist)
{
printf ("model::FindPath (%d, %d) max = %d flag = %d dist = %d\n", ref1->index, ref2->index, max, flag, dist);
	if (ref1 == ref2) return dist;
	if (dist == max) return NOT_FOUND;
	
	i32s tmp1 = NOT_FOUND; iter_cl it1;
	for (it1 = ref1->cr_list.begin();it1 != ref1->cr_list.end();it1++)
	{
		if ((* it1).bndr->flags[flag]) continue;
		
		(* it1).bndr->flags[flag] = true;
		i32s tmp2 = FindPath((* it1).atmr, ref2, max, flag, dist + 1);
		(* it1).bndr->flags[flag] = false;
		
		if (tmp2 < tmp1) tmp1 = tmp2;
	}
	
	return tmp1;
}

vector<bond *> * model::FindPathV(atom * ref1, atom * ref2, i32s max, i32s flag, i32s dist)
{
	if (ref1 == ref2) return new vector<bond *>;
	if (dist == max) return NULL;
	
	vector<bond *> * tmp1 = NULL; iter_cl it1;
	for (it1 = ref1->cr_list.begin();it1 != ref1->cr_list.end();it1++)
	{
		if ((* it1).bndr->flags[flag]) continue;
		
		(* it1).bndr->flags[flag] = true;
		vector<bond *> * tmp2 = FindPathV((* it1).atmr, ref2, max, flag, dist + 1);
		(* it1).bndr->flags[flag] = false;
		
		if (tmp2 != NULL)
		{
			tmp2->push_back((* it1).bndr);
			if (tmp1 == NULL) tmp1 = tmp2;
			else
			{
				if (tmp2->size() < tmp1->size())
				{
					delete tmp1;
					tmp1 = tmp2;
				}
			}
		}
	}
	
	return tmp1;
}

bool model::FindRing(atom * ref1, atom * ref2, signed char * str, i32s size, i32s flag, i32s dist)
{
	static vector<signed char> ring_vector;
		
	if (!dist && str != NULL) ring_vector.resize(0);
	else if (dist && ref1 == ref2)
	{
		if (dist != size) return false;
		
		if (str != NULL)
		{
			for (i32u n1 = 0;n1 < strlen((const char *) str);n1++)
			{
				if (!(n1 % 2) && str[n1] == '?') continue;
				if ((n1 % 2) && str[n1] == NOT_DEFINED) continue;
				if (str[n1] != ring_vector[n1]) return false;
			}
		}
		
		return true;
	}
	
	if (dist == size) return false; iter_cl it1;
	for (it1 = ref1->cr_list.begin();it1 != ref1->cr_list.end();it1++)
	{
		if ((* it1).bndr->flags[flag]) continue;
		
		if (str != NULL)
		{
			ring_vector.push_back((* it1).bndr->bt.GetSymbol2());
			ring_vector.push_back((signed char) (* it1).atmr->el.GetAtomicNumber());
		}
		
		(* it1).bndr->flags[flag] = true;
		bool result = FindRing((* it1).atmr, ref2, str, size, flag, dist + 1);
		(* it1).bndr->flags[flag] = false;
		
		if (str != NULL)
		{
			ring_vector.pop_back();
			ring_vector.pop_back();
		}
		
		if (result) return true;
	}
	
	return false;
}

void model::InvalidateGroups(void)
{
	is_index_clean = false;
	is_groups_clean = false;
	is_groups_sorted = false;
	
	nmol = NOT_DEFINED;
	if (ref_civ != NULL)
	{
		delete ref_civ;
		ref_civ = NULL;
	}
}

void model::UpdateIndex(void)
{
	i32s counter = 0;
	iter_al it1 = atom_list.begin();
	while (it1 != atom_list.end())
	{
		(* it1++).index = counter++;
	}
	
	is_index_clean = true;
}

void model::UpdateGroups(void)
{
	InvalidateGroups();
	UpdateIndex();		// looks foolish, but this is a quick operation...
	
	nmol = 0;
	
	iter_al it1;
	for (it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		(* it1).id[0] = (* it1).id[1] = (* it1).id[2] = (* it1).id[3] = NOT_DEFINED;
	}
	
	while (true)
	{
		for (it1 = atom_list.begin();it1 != atom_list.end() && (* it1).id[0] != NOT_DEFINED;it1++);
		if (it1 != atom_list.end()) GatherAtoms(& (* it1), nmol++); else break;
	}
	
	UpdateIndex();		// this is so that we can keep is_index_clean true!
	
	is_groups_clean = true;
}

void model::SortGroups(bool verbose)
{
	if (!is_groups_clean)
	{
		cout << "fatal error : model::SortGroups() was called while model::IsGroupsClean() is false!" << endl;
		exit(EXIT_FAILURE);
	}
	
	// sorting the atom list to get contiguous molecules/chains/residues is an efficient technique, but may be
	// confusing in the cases where atom indexing must not change (like TSS); therefore always inform the user!!!
	
	const char message[] = "Calling model::SortGroups() so the atom indexing may change!\n";
	if (verbose) Message(message); else PrintToLog(message);
	
	atom_list.sort();	// this should be the ONLY place where atom_list is sorted!!!
	UpdateIndex();		// this is so that we can keep is_index_clean true!
	
	bond_list.sort();	// this should be the ONLY place where bond_list is sorted!!!
	
	// sorting the atom_list may cause that model::atom_list and setup::atmtab are ordered differently; synchronize!!!
	// sorting the atom_list may cause that model::atom_list and setup::atmtab are ordered differently; synchronize!!!
	// sorting the atom_list may cause that model::atom_list and setup::atmtab are ordered differently; synchronize!!!
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	//GetCurrentSetup()->UpdateSetupInfo();		// not needed? called in every setup::CreateEngineByIndex() implementation...
	
	is_groups_sorted = true;
}

void model::UpdateChains(void)
{
	if (!is_groups_clean) UpdateGroups();
	if (!is_groups_sorted) SortGroups(false);	// at sequencebuilder::Identify() we will need model::GetRange().
	
	if (ref_civ != NULL) delete ref_civ;
	ref_civ = new vector<chn_info>;
	
	model::amino_builder.Identify(this);
	model::nucleic_builder.Identify(this);
	
	SortGroups(true);	// ok, then sort the atom_list using identified chains/residues...
}

void model::GatherAtoms(atom * ref, i32s id)
{
	if (ref->id[0] != NOT_DEFINED) return;
	ref->id[0] = id; iter_cl it1 = ref->cr_list.begin();
	while (it1 != ref->cr_list.end()) GatherAtoms((* it1++).atmr, id);
}

/*##############################################*/
/*##############################################*/

atom * model::cp_FindAtom(iter_al * res_rng, i32s id)		// see default_tables::e_UT_FindAtom()...
{
	iter_al it1 = res_rng[0];
	while (it1 != res_rng[1] && ((* it1).builder_res_id & 0xFF) != id) it1++;
	if (it1 == res_rng[1]) return NULL; else return & (* it1);
}

void model::CheckProtonation(vector<atom *> & av, vector<float> & vv)
{
	if (ref_civ == NULL) UpdateChains();
	
	vector<chn_info> & ci_vector = (* ref_civ);
	for (i32u n1 = 0;n1 < ci_vector.size();n1++)
	{
		iter_al chnR[2]; GetRange(1, n1, chnR);
		
		// amino...
		// amino...
		// amino...
		
		if (ci_vector[n1].GetType() == chn_info::amino_acid)
		{
			const char * sequence = ci_vector[n1].GetSequence();
			const char * pstate = ci_vector[n1].GetProtonationStates();
			
			char txtbuff[256];
			if (pstate != NULL)
			{
				ostrstream str(txtbuff, sizeof(txtbuff));
				str << "CheckProtonation() : pstate array found for chain " << n1 << "." << endl << ends;
				PrintToLog(txtbuff);
			}
			else
			{
				ostrstream str(txtbuff, sizeof(txtbuff));
				str << "CheckProtonation() : no pstate array found for chain " << n1 << "; USING DEFAULTS!" << endl << ends;
				PrintToLog(txtbuff);
			}
			
			for (i32s n2 = 0;n2 < ci_vector[n1].GetLength();n2++)
			{
				iter_al resR[2]; GetRange(2, chnR, n2, resR);
				
				atom * atmr_00 = cp_FindAtom(resR, 0x00);	// N
				atom * atmr_01 = cp_FindAtom(resR, 0x01);	// alpha-C
				atom * atmr_02 = cp_FindAtom(resR, 0x02);	// carbonyl-C
				atom * atmr_10 = cp_FindAtom(resR, 0x10);	// carbonyl-O
				
				if (!atmr_00 || !atmr_01 || !atmr_02 || !atmr_10) continue;
				
				int sidechain_charge = 0;
				if (pstate != NULL)
				{
					sidechain_charge = (pstate[n2] & PSTATE_CHARGE_mask);
					if (pstate[n2] & PSTATE_SIGN_NEGATIVE) sidechain_charge = -sidechain_charge;
					else if (!(pstate[n2] & PSTATE_SIGN_POSITIVE))
					{
						cout << "ERROR : no sign information found from pstate!!!" << endl;
						exit(EXIT_FAILURE);
					}
				}
				
			//	cout << "sidechain_charge = " << sidechain_charge << endl;	// debug...
				
				if (n2 == 0)					// N-terminal residue???
				{
					bool charged = true;
					if (pstate != NULL) charged = ((pstate[n2] & PSTATE_CHARGED_TERMINAL) ? true : false);
					
					ostrstream str(txtbuff, sizeof(txtbuff));
					str << "CheckProtonation() : setting N-terminal " << (charged ? "charged." : "neutral.") << endl << ends;
					PrintToLog(txtbuff);
					
					if (charged)
					{
						av.push_back(atmr_00); vv.push_back(4.0);
						
						sidechain_charge -= 1;
					}
					else
					{
						// atmr_00 : default valence 3.0 is OK.
					}
				}
				
				if (n2 == ci_vector[n1].GetLength() - 1)	// C-terminal residue???
				{
				//	atom * atmr_11 = cp_FindAtom(resR, 0x11);
					atom * atmr_11 = NULL;	// search OXT directly since seq-builder misses it...
					for (iter_cl it1 = atmr_02->cr_list.begin();it1 != atmr_02->cr_list.end();it1++)
					{
						if ((* it1).atmr->el.GetAtomicNumber() != 8) continue;
						if (((* it1).atmr->builder_res_id & 0xFF) == 0x10) continue;
						
						atmr_11 = (* it1).atmr;
					}
					
					bool charged = true;
					if (pstate != NULL) charged = ((pstate[n2] & PSTATE_CHARGED_TERMINAL) ? true : false);
					
					ostrstream str(txtbuff, sizeof(txtbuff));
					str << "CheckProtonation() : setting C-terminal " << (charged ? "charged." : "neutral.") << endl << ends;
					PrintToLog(txtbuff);
					
					bond tmpb1(atmr_02, atmr_10, bondtype('S'));
					iter_bl tmpi1 = find(bond_list.begin(), bond_list.end(), tmpb1);
					
					bond tmpb2(atmr_02, atmr_11, bondtype('S'));
					iter_bl tmpi2 = find(bond_list.begin(), bond_list.end(), tmpb2);
					
					if (charged)
					{
						av.push_back(atmr_10); vv.push_back(1.0);
						av.push_back(atmr_11); vv.push_back(1.0);
						
						if (tmpi1 != bond_list.end()) (* tmpi1).bt = bondtype('C');
						if (tmpi2 != bond_list.end()) (* tmpi2).bt = bondtype('C');
						
						sidechain_charge += 1;
					}
					else
					{
						// atmr_10 : default valence 2.0 is OK.
						// atmr_11 : default valence 2.0 is OK.
						
						if (tmpi1 != bond_list.end()) (* tmpi1).bt = bondtype('D');
						if (tmpi2 != bond_list.end()) (* tmpi2).bt = bondtype('S');
					}
				}
				
				// start handling the side chains!!!
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
				
			/*	if (sequence[n2] == 'A')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
				}	*/
				
			/*	if (sequence[n2] == 'R')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
					atom * atmr_25 = cp_FindAtom(resR, 0x25);
					atom * atmr_26 = cp_FindAtom(resR, 0x26);
				}	*/
				
			/*	if (sequence[n2] == 'N')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
				}	*/
				
				if (sequence[n2] == 'D')
				{
				/*	atom * atmr_20 = cp_FindAtom(resR, 0x20);	*/
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
					
					bool charged = true;
					if (pstate != NULL)
					{
						switch (sidechain_charge)
						{
							case 0:		charged = false; break;
							case -1:	charged = true; break;
							default:
							cout << "charge " << sidechain_charge << " out of valid range for residue D (" << n1 << "/" << n2 << ")." << endl;
							exit(EXIT_FAILURE);
						}
					}
					
					ostrstream str(txtbuff, sizeof(txtbuff));
					str << "CheckProtonation() : setting residue " << n2 << " D " << (charged ? "charged." : "neutral.") << endl << ends;
					PrintToLog(txtbuff);
					
					bond tmpb1(atmr_21, atmr_22, bondtype('S'));
					iter_bl tmpi1 = find(bond_list.begin(), bond_list.end(), tmpb1);
					
					bond tmpb2(atmr_21, atmr_23, bondtype('S'));
					iter_bl tmpi2 = find(bond_list.begin(), bond_list.end(), tmpb2);
					
					if (charged)
					{
						av.push_back(atmr_22); vv.push_back(1.0);
						av.push_back(atmr_23); vv.push_back(1.0);
						
						if (tmpi1 != bond_list.end()) (* tmpi1).bt = bondtype('C');
						if (tmpi2 != bond_list.end()) (* tmpi2).bt = bondtype('C');
					}
					else
					{
						// atmr_22 : default valence 2.0 is OK.
						// atmr_23 : default valence 2.0 is OK.
						
						if (tmpi1 != bond_list.end()) (* tmpi1).bt = bondtype('D');
						if (tmpi2 != bond_list.end()) (* tmpi2).bt = bondtype('S');
					}
				}
				
				if (sequence[n2] == 'C')
				{
				/*	atom * atmr_20 = cp_FindAtom(resR, 0x20);	*/
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					
					bool charged = false;
					if (pstate != NULL)
					{
						switch (sidechain_charge)
						{
							case 0:		charged = false; break;
							case -1:	charged = true; break;
							default:
							cout << "charge " << sidechain_charge << " out of valid range for residue C (" << n1 << "/" << n2 << ")." << endl;
							exit(EXIT_FAILURE);
						}
					}
					
					ostrstream str(txtbuff, sizeof(txtbuff));
					str << "CheckProtonation() : setting residue " << n2 << " C " << (charged ? "charged." : "neutral.") << endl << ends;
					PrintToLog(txtbuff);
					
					if (charged)
					{
						av.push_back(atmr_21); vv.push_back(1.0);
					}
					else
					{
						// atmr_21 : default valence 2.0 is OK.
					}
				}
				
			/*	if (sequence[n2] == 'Q')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
				}	*/
				
				if (sequence[n2] == 'E')
				{
				/*	atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);	*/
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
					
					bool charged = true;
					if (pstate != NULL)
					{
						switch (sidechain_charge)
						{
							case 0:		charged = false; break;
							case -1:	charged = true; break;
							default:
							cout << "charge " << sidechain_charge << " out of valid range for residue E (" << n1 << "/" << n2 << ")." << endl;
							exit(EXIT_FAILURE);
						}
					}
					
					ostrstream str(txtbuff, sizeof(txtbuff));
					str << "CheckProtonation() : setting residue " << n2 << " E " << (charged ? "charged." : "neutral.") << endl << ends;
					PrintToLog(txtbuff);
					
					bond tmpb1(atmr_22, atmr_23, bondtype('S'));
					iter_bl tmpi1 = find(bond_list.begin(), bond_list.end(), tmpb1);
					
					bond tmpb2(atmr_22, atmr_24, bondtype('S'));
					iter_bl tmpi2 = find(bond_list.begin(), bond_list.end(), tmpb2);
					
					if (charged)
					{
						av.push_back(atmr_23); vv.push_back(1.0);
						av.push_back(atmr_24); vv.push_back(1.0);
						
						if (tmpi1 != bond_list.end()) (* tmpi1).bt = bondtype('C');
						if (tmpi2 != bond_list.end()) (* tmpi2).bt = bondtype('C');
					}
					else
					{
						// atmr_23 : default valence 2.0 is OK.
						// atmr_24 : default valence 2.0 is OK.
						
						if (tmpi1 != bond_list.end()) (* tmpi1).bt = bondtype('D');
						if (tmpi2 != bond_list.end()) (* tmpi2).bt = bondtype('S');
					}
				}
				
			/*	if (sequence[n2] == 'G')
				{
				}	*/
				
				if (sequence[n2] == 'H')
				{
				/*	atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);	*/
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
				/*	atom * atmr_23 = cp_FindAtom(resR, 0x23);	*/
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
				/*	atom * atmr_25 = cp_FindAtom(resR, 0x25);	*/
					
					bool charged = false;
					if (pstate != NULL)
					{
						switch (sidechain_charge)
						{
							case 0:		charged = false; break;
							case +1:	charged = true; break;
							default:
							cout << "charge " << sidechain_charge << " out of valid range for residue H (" << n1 << "/" << n2 << ")." << endl;
							exit(EXIT_FAILURE);
						}
					}
					
					// if neutral, should it be epsilon (HIE) or delta (HID)???
					
					ostrstream str(txtbuff, sizeof(txtbuff));
					str << "CheckProtonation() : setting residue " << n2 << " H " << (charged ? "charged." : "neutral(HIE).") << endl << ends;
					PrintToLog(txtbuff);
					
					if (charged)
					{
						av.push_back(atmr_22); vv.push_back(4.0);
						av.push_back(atmr_24); vv.push_back(4.0);
					}
					else
					{
						av.push_back(atmr_24); vv.push_back(4.0);
					}
				}
				
			/*	if (sequence[n2] == 'I')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
				}	*/
				
			/*	if (sequence[n2] == 'L')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
				}	*/
				
				if (sequence[n2] == 'K')
				{
				/*	atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);	*/
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
					
					bool charged = true;
					if (pstate != NULL)
					{
						switch (sidechain_charge)
						{
							case 0:		charged = false; break;
							case +1:	charged = true; break;
							default:
							cout << "charge " << sidechain_charge << " out of valid range for residue K (" << n1 << "/" << n2 << ")." << endl;
							exit(EXIT_FAILURE);
						}
					}
					
					ostrstream str(txtbuff, sizeof(txtbuff));
					str << "CheckProtonation() : setting residue " << n2 << " K " << (charged ? "charged." : "neutral.") << endl << ends;
					PrintToLog(txtbuff);
					
					if (charged)
					{
						av.push_back(atmr_24); vv.push_back(4.0);
					}
					else
					{
						// atmr_24 : default valence 3.0 is OK.
					}
				}
				
			/*	if (sequence[n2] == 'M')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
				}	*/
				
			/*	if (sequence[n2] == 'F')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
					atom * atmr_25 = cp_FindAtom(resR, 0x25);
					atom * atmr_26 = cp_FindAtom(resR, 0x26);
				}	*/
				
			/*	if (sequence[n2] == 'P')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
				}	*/
				
			/*	if (sequence[n2] == 'S')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
				}	*/
				
			/*	if (sequence[n2] == 'T')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
				}	*/
				
				if (sequence[n2] == 'W')
				{
				/*	atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);	*/
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
				/*	atom * atmr_24 = cp_FindAtom(resR, 0x24);
					atom * atmr_25 = cp_FindAtom(resR, 0x25);	// a bug in seq-builder; need to swap 25<->29???
					atom * atmr_26 = cp_FindAtom(resR, 0x26);	// a bug in seq-builder; need to swap 26<->28???
					atom * atmr_27 = cp_FindAtom(resR, 0x27);
					atom * atmr_28 = cp_FindAtom(resR, 0x28);	// a bug in seq-builder; need to swap 26<->28???
					atom * atmr_29 = cp_FindAtom(resR, 0x29);	// a bug in seq-builder; need to swap 25<->29???	*/
					
					av.push_back(atmr_23); vv.push_back(4.0);
				}
				
			/*	if (sequence[n2] == 'Y')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
					atom * atmr_25 = cp_FindAtom(resR, 0x25);
					atom * atmr_26 = cp_FindAtom(resR, 0x26);
					atom * atmr_27 = cp_FindAtom(resR, 0x27);
				}	*/
				
			/*	if (sequence[n2] == 'V')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
				}	*/
			}
		}
		
		// nucleic...
		// nucleic...
		// nucleic...
		
		if (ci_vector[n1].GetType() == chn_info::nucleic_acid)
		{
		//	const char * sequence = ci_vector[n1].GetSequence();
			
			for (i32s n2 = 0;n2 < ci_vector[n1].GetLength();n2++)
			{
				iter_al resR[2]; GetRange(2, chnR, n2, resR);
				
			/*	atom * atmr_01 = cp_FindAtom(resR, 0x01);
				atom * atmr_02 = cp_FindAtom(resR, 0x02);
				atom * atmr_03 = cp_FindAtom(resR, 0x03);
				atom * atmr_04 = cp_FindAtom(resR, 0x04);
				atom * atmr_05 = cp_FindAtom(resR, 0x05);
				atom * atmr_06 = cp_FindAtom(resR, 0x06);
				atom * atmr_07 = cp_FindAtom(resR, 0x07);
				atom * atmr_08 = cp_FindAtom(resR, 0x08);
				
				if (!atmr_01 || !atmr_02 || !atmr_03 || !atmr_04) continue;
				if (!atmr_05 || !atmr_06 || !atmr_07 || !atmr_08) continue;	*/
				
				atom * atmr_10 = cp_FindAtom(resR, 0x10);
				atom * atmr_11 = cp_FindAtom(resR, 0x11);
				atom * atmr_12 = cp_FindAtom(resR, 0x12);	// 5'-terminal...
				if (atmr_10 != NULL) { av.push_back(atmr_10); vv.push_back(1.0); }
				if (atmr_11 != NULL) { av.push_back(atmr_11); vv.push_back(1.0); }
				if (atmr_12 != NULL) { av.push_back(atmr_12); vv.push_back(1.0); }
				
			/*	if (n2 == 0)					// 5'-terminal residue???
				{
				}
				
				if (n2 == ci_vector[n1].GetLength() - 1)	// 3'-terminal residue???
				{
				}	*/
				
				// start handling the side chains!!!
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
				
			/*	if (sequence[n2] == 'A')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
					atom * atmr_25 = cp_FindAtom(resR, 0x25);
					atom * atmr_26 = cp_FindAtom(resR, 0x26);
					atom * atmr_27 = cp_FindAtom(resR, 0x27);
					atom * atmr_28 = cp_FindAtom(resR, 0x28);
					atom * atmr_40 = cp_FindAtom(resR, 0x40);
				}	*/
				
			/*	if (sequence[n2] == 'C')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
					atom * atmr_25 = cp_FindAtom(resR, 0x25);
					atom * atmr_40 = cp_FindAtom(resR, 0x40);
					atom * atmr_41 = cp_FindAtom(resR, 0x41);
				}	*/
				
			/*	if (sequence[n2] == 'G')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
					atom * atmr_25 = cp_FindAtom(resR, 0x25);
					atom * atmr_26 = cp_FindAtom(resR, 0x26);
					atom * atmr_27 = cp_FindAtom(resR, 0x27);
					atom * atmr_28 = cp_FindAtom(resR, 0x28);
					atom * atmr_40 = cp_FindAtom(resR, 0x40);
					atom * atmr_41 = cp_FindAtom(resR, 0x41);
				}	*/
				
			/*	if (sequence[n2] == 'T')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
					atom * atmr_25 = cp_FindAtom(resR, 0x25);
					atom * atmr_40 = cp_FindAtom(resR, 0x40);
					atom * atmr_41 = cp_FindAtom(resR, 0x41);
					atom * atmr_42 = cp_FindAtom(resR, 0x42);
				}	*/
				
			/*	if (sequence[n2] == 'a')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
					atom * atmr_25 = cp_FindAtom(resR, 0x25);
					atom * atmr_26 = cp_FindAtom(resR, 0x26);
					atom * atmr_27 = cp_FindAtom(resR, 0x27);
					atom * atmr_28 = cp_FindAtom(resR, 0x28);
					atom * atmr_40 = cp_FindAtom(resR, 0x40);
				}	*/
				
			/*	if (sequence[n2] == 'c')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
					atom * atmr_25 = cp_FindAtom(resR, 0x25);
					atom * atmr_40 = cp_FindAtom(resR, 0x40);
					atom * atmr_41 = cp_FindAtom(resR, 0x41);
				}	*/
				
			/*	if (sequence[n2] == 'g')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
					atom * atmr_25 = cp_FindAtom(resR, 0x25);
					atom * atmr_26 = cp_FindAtom(resR, 0x26);
					atom * atmr_27 = cp_FindAtom(resR, 0x27);
					atom * atmr_28 = cp_FindAtom(resR, 0x28);
					atom * atmr_40 = cp_FindAtom(resR, 0x40);
					atom * atmr_41 = cp_FindAtom(resR, 0x41);
				}	*/
				
			/*	if (sequence[n2] == 'u')
				{
					atom * atmr_20 = cp_FindAtom(resR, 0x20);
					atom * atmr_21 = cp_FindAtom(resR, 0x21);
					atom * atmr_22 = cp_FindAtom(resR, 0x22);
					atom * atmr_23 = cp_FindAtom(resR, 0x23);
					atom * atmr_24 = cp_FindAtom(resR, 0x24);
					atom * atmr_25 = cp_FindAtom(resR, 0x25);
					atom * atmr_40 = cp_FindAtom(resR, 0x40);
					atom * atmr_41 = cp_FindAtom(resR, 0x41);
				}	*/
			}
		}
	}
}

void model::AddHydrogens(void)
{
	vector<atom *> av;	// non-standard valences based on ref_civ->p_state information...
	vector<float> vv;	// non-standard valences based on ref_civ->p_state information...
	
	if (ref_civ != NULL)
	{
		PrintToLog("Sequence information found; calling CheckProtonation().\n");
		
		CheckProtonation(av, vv);
		
		if (av.size() != vv.size())
		{
			cout << "ERROR : CheckProtonation() returned invalid arrays." << endl;
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		PrintToLog("Using default rules in AddHydrogens().\n");
	}
	
	for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		bool skip = true;
		
		fGL valence = NOT_DEFINED;
		int an = (* it1).el.GetAtomicNumber();
		switch (an)
		{
			case 6:		valence = 4; skip = false; break;
			case 7:		valence = 3; skip = false; break;
			case 8:		valence = 2; skip = false; break;
			case 16:	valence = 2; skip = false; break;
		}
		
		if (skip) continue;
		
		// CHECK THE VALENCE ARRAYS!!!
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^
		
		for (i32u n1 = 0;n1 < av.size();n1++)
		{
			if (av[n1] != & (* it1)) continue;
			else
			{
				valence = vv[n1];
				break;
			}
		}
		
		i32s geometry = 0;	// codes: 0=tetrahedral, 1=planar, 2=linear
		vector<atom *> other_atoms;
		
		for (iter_cl it2 = (* it1).cr_list.begin();it2 != (* it1).cr_list.end();it2++)
		{
			other_atoms.push_back((* it2).atmr);
			
			switch ((* it2).bndr->bt.GetValue())
			{
				case 1:		valence -= 1.0; break;
				case 2:		valence -= 2.0; if (!geometry) geometry++; break;
				case 3:		valence -= 3.0; geometry = 2; break;
				case 0:		valence -= 1.5; if (!geometry) geometry++; break;
			}
		}
		
		// add the atoms/bonds...
		// ^^^^^^^^^^^^^^^^^^^^^^
		
		fGL _len = 0.11;
		
		fGL _ang;
		if (geometry == 2) _ang = 180.0 * M_PI / 180.0;
		else if (geometry == 1) _ang = 120.0 * M_PI / 180.0;
		else _ang = 109.5 * M_PI / 180.0;
		
		const fGL * crdx = (* it1).GetCRD(0);
		while (valence > 0.0)
		{
			fGL rx = ((fGL) rand() / (fGL) RAND_MAX) - 0.5;
			fGL ry = ((fGL) rand() / (fGL) RAND_MAX) - 0.5;
			fGL rz = ((fGL) rand() / (fGL) RAND_MAX) - 0.5;
			v3d<fGL> v1(rx, ry, rz); v1 = v1 / v1.len();		// the default direction is random!!!
			
			switch (other_atoms.size())
			{
				case 1:
				{	// local variables...
					const fGL * crd0 = other_atoms[0]->GetCRD(0);
					v3d<fGL> v2(crdx, crd0); v2 = v2 / v2.len();
					v3d<fGL> v3 = v1.vpr(v2); v3 = v3 / v3.len();
					v1 = (v2 * cos(_ang)) + (v3 * sin(_ang));
				}	// local variables...
				break;
				
				case 2:
				{	// local variables...
					const fGL * crd0 = other_atoms[0]->GetCRD(0);
					const fGL * crd1 = other_atoms[1]->GetCRD(0);
					v3d<fGL> v2(crdx, crd0); v2 = v2 / v2.len();
					v3d<fGL> v3(crdx, crd1); v3 = v3 / v3.len();
					if (!geometry)
					{
						v3d<fGL> v4 = v2 + v3; v4 = v4 / -v4.len();
						v3d<fGL> v5 = v2.vpr(v3); v5 = v5 / v5.len();
						v1 = (v4 * 0.5) + (v5 * 0.5); v1 = v1 / v1.len();
					}
					else
					{
						v1 = v2 + v3; v1 = v1 / -v1.len();
					}
				}	// local variables...
				break;
				
				case 3:
				{	// local variables...
					const fGL * crd0 = other_atoms[0]->GetCRD(0);
					const fGL * crd1 = other_atoms[1]->GetCRD(0);
					const fGL * crd2 = other_atoms[2]->GetCRD(0);
					v3d<fGL> v2(crdx, crd0); v2 = v2 / v2.len();
					v3d<fGL> v3(crdx, crd1); v3 = v3 / v3.len();
					v3d<fGL> v4(crdx, crd2); v4 = v4 / v4.len();
					v1 = v2 + v3 + v4; v1 = v1 / -v1.len();
				}	// local variables...
				break;
			}
			
			fGL crdh[3];
			crdh[0] = crdx[0] + _len * v1.data[0];
			crdh[1] = crdx[1] + _len * v1.data[1];
			crdh[2] = crdx[2] + _len * v1.data[2];
			
			atom newatom(element(1), crdh, cs_vector.size());
			Add_Atom(newatom); other_atoms.push_back(& atom_list.back());
			bond newbond(& (* it1), & atom_list.back(), bondtype('S'));
			AddBond(newbond);
			
			valence -= 1.0;
		}
	}
}

void model::RemoveHydrogens(void)
{
	iter_bl itb1 = bond_list.begin();
	while (itb1 != bond_list.end())
	{
		bool flag = false;
		if ((* itb1).atmr[0]->el.GetAtomicNumber() == 1) flag = true;
		if ((* itb1).atmr[1]->el.GetAtomicNumber() == 1) flag = true;
		
		if (flag)
		{
			RemoveBond(itb1);		// now this iterator is invalidated?!?!?!
			itb1 = bond_list.begin();
		}
		else itb1++;
	}
	
	iter_al ita1 = atom_list.begin();
	while (ita1 != atom_list.end())
	{
		bool flag = false;
		if ((* ita1).el.GetAtomicNumber() == 1) flag = true;
		
		if (flag)
		{
			RemoveAtom(ita1);		// now this iterator is invalidated?!?!?!
			ita1 = atom_list.begin();
		}
		else ita1++;
	}
}

void model::SolvateBox(fGL dimx, fGL dimy, fGL dimz, fGL density, int element_number, model * solvent, const char * export_gromacs)
{
//	use_periodic_boundary_conditions = true;	// DO NOT SET THIS!!! this is broken apparently...
	periodic_box_HALFdim[0] = dimx;
	periodic_box_HALFdim[1] = dimy;
	periodic_box_HALFdim[2] = dimz;
	SystemWasModified();
	
	if (density <= 0.0) return;	// zero density -> infinite distance.
//	const fGL distance = S_Initialize(density, & solvent);		// H2O
//	const fGL distance = S_Initialize(density, & solvent, element_number);	

	fGL distance;

	if (element_number > 0)
	{
		distance = S_Initialize(density, & solvent, element_number);
	}
	else
	{
		distance = S_Initialize(density, & solvent);		// H2O
	}
	
	bool system_was_empty = (!atom_list.size() && !bond_list.size());
	
	// make the box...
	// ^^^^^^^^^^^^^^^
	
//	dimx *= 0.5; dimy *= 0.5; dimz *= 0.5;	// convert to half-dimensions!!!
	dimx *= 0.75; dimy *= 0.75; dimz *= 0.75;	// convert to half-dimensions!!!
	
	i32s limx = (i32s) floor(dimx / distance) + 2;		// +1 is the minimum.
	i32s limy = (i32s) floor(dimy / distance) + 2;		// +1 is the minimum.
	i32s limz = (i32s) floor(dimz / distance) + 2;		// +1 is the minimum.
	
	i32s solvent_molecules_added = 0;
	for (i32s lx = -limx + 1;lx < limx;lx++)
	{
		for (i32s ly = -limy + 1;ly < limy;ly++)
		{
			for (i32s lz = -limz + 1;lz < limz;lz++)
			{
				fGL crdS[3];
				crdS[0] = distance * lx;
				crdS[1] = distance * ly;
				crdS[2] = distance * lz;
				
				if (lz % 2)	// twist the cubic lattice to an octahedral one ; OPTIONAL!
				{
					crdS[0] += 0.5 * distance;
					crdS[1] += 0.5 * distance;
				}
				
				if (crdS[0] < -dimx || crdS[0] > +dimx) continue;	// skip if the molecule is too far...
				if (crdS[1] < -dimy || crdS[1] > +dimy) continue;	// skip if the molecule is too far...
				if (crdS[2] < -dimz || crdS[2] > +dimz) continue;	// skip if the molecule is too far...
				
				bool skip = false;	// skip if there is overlap with solute atoms ; FOR SMALL SOLVENTS ONLY?!?
				for (iter_al it1 = GetAtomsBegin();it1 != GetAtomsEnd();it1++)
				{
					if ((* it1).flags & ATOMFLAG_IS_SOLVENT_ATOM) continue;
					
					const fGL * crdX = (* it1).GetCRD(0);
					
					fGL dx = crdS[0] - crdX[0];
					fGL dy = crdS[1] - crdX[1];
					fGL dz = crdS[2] - crdX[2];
					
					fGL d2s = sqrt(dx * dx + dy * dy + dz * dz);
					if (d2s < 0.175) skip = true;
					
					if (skip) break;
				}	if (skip) continue;
				
				solvent_molecules_added++;
				
				f64 rot[3];
				rot[0] = 2.0 * M_PI * (f64) rand() / (f64) RAND_MAX;
				rot[1] = 2.0 * M_PI * (f64) rand() / (f64) RAND_MAX;
				rot[2] = 2.0 * M_PI * (f64) rand() / (f64) RAND_MAX;
				
				vector<atom *> av1;
				vector<atom *> av2;
				{
				
				for (iter_al it1 = solvent->GetAtomsBegin();it1 != solvent->GetAtomsEnd();it1++)
				{
					const fGL * orig = (* it1).GetCRD(0);
					
					fGL d2b[3];		// rotate x (y,z)...
					d2b[0] = orig[0];
					d2b[1] = orig[1] * cos(rot[0]) - orig[2] * sin(rot[0]);
					d2b[2] = orig[1] * sin(rot[0]) + orig[2] * cos(rot[0]);
					
					fGL d2c[3];		// rotate y (z,x)...
					d2c[0] = d2b[2] * sin(rot[1]) + d2b[0] * cos(rot[1]);
					d2c[1] = d2b[1];
					d2c[2] = d2b[2] * cos(rot[1]) - d2b[0] * sin(rot[1]);
					
					fGL d2d[3];		// rotate z (x,y)...
					d2d[0] = d2c[0] * cos(rot[2]) - d2c[1] * sin(rot[2]);
					d2d[1] = d2c[0] * sin(rot[2]) + d2c[1] * cos(rot[2]);
					d2d[2] = d2c[2];
					
					d2d[0] += crdS[0];
					d2d[1] += crdS[1];
					d2d[2] += crdS[2];
					
					atom newA((* it1).el, d2d, GetCRDSetCount());
					newA.flags |= ATOMFLAG_IS_SOLVENT_ATOM;
				//	newA.flags |= ATOMFLAG_MEASURE_ND_RDF;		// what about this???
					
					Add_Atom(newA);
					av1.push_back(& (* it1));
					av2.push_back(& GetLastAtom());
				}
				}
				{

				
				for (iter_bl it1 = solvent->GetBondsBegin();it1 != solvent->GetBondsEnd();it1++)
				{
					i32u ind1 = 0;
					while (ind1 < av1.size())
					{
						if ((* it1).atmr[0] == av1[ind1]) break;
						else ind1++;
					}
					
					i32u ind2 = 0;
					while (ind2 < av1.size())
					{
						if ((* it1).atmr[1] == av1[ind2]) break;
						else ind2++;
					}
					
					if (ind1 == av1.size() || ind2 == av1.size())
					{
						cout << "index search failed!" << endl;
						exit(EXIT_FAILURE);
					}
					
					bond newB(av2[ind1], av2[ind2], (* it1).bt);
					AddBond(newB);
				}
				}
			}
		}
	}
	
	cout << "added " << solvent_molecules_added << " solvent molecules." << endl;
	
	if (export_gromacs != NULL)
	{
		if (!system_was_empty)
		{
			Message("Sorry, the export option is available for pure solvents only!");
			delete solvent; return;
		}
		
		if (dynamic_cast<setup1_mm *>(GetCurrentSetup()) == NULL)
		{
			Message("Sorry, the export option is available for MM setups only!");
			delete solvent; return;
		}
		
		delete solvent->current_setup;
		solvent->current_setup = new setup1_mm(solvent);
		solvent->current_setup->current_eng_index = GetCurrentSetup()->GetCurrEngIndex();
		
		solvent->current_setup->CreateCurrentEngine();
		engine * eng1 = solvent->current_setup->GetCurrentEngine();
		eng1_mm_default_bt * eng2 = dynamic_cast<eng1_mm_default_bt *>(eng1);
		if (eng1 == NULL || eng2== NULL)
		{
			Message("Export failed!");
			delete solvent; return;
		}
		
		CopyCRD(solvent, eng2, 0);
		eng2->Compute(0);
		
		atom ** atmtab = solvent->current_setup->GetMMAtoms();
		
		// write the top file
		// ^^^^^^^^^^^^^^^^^^
		
		char top_fn[256]; ostrstream tfns(top_fn, sizeof(top_fn));
		tfns << export_gromacs << ".top" << ends;
		
		ofstream top_f;
		top_f.open(top_fn, ios::out);
		
		top_f << "; this is a top file exported by libghemical " << LIBVERSION << ", not an original gromacs file!" << endl;
		top_f << endl;
		
		top_f << "[ defaults ]" << endl;
		top_f << ";	nbfunc	comb-rule	gen-pairs	fudgeLJ	fudgeQQ" << endl;
		top_f << "	1	2		yes		" << eng1_mm::fudgeLJ << "\t" << eng1_mm::fudgeQQ << endl;
		top_f << endl;
		
		top_f << "[ atomtypes ]" << endl;
		top_f << ";	name	mass	charge	ptype	sigma	epsilon" << endl;
		
		for (i32s ai = 0;ai < solvent->current_setup->GetMMAtomCount();ai++)
		{
			const default_at * at; f64 r1 = 0.150; f64 e1 = 0.175;
			at = default_tables::GetInstance()->GetAtomType(atmtab[ai]->atmtp);
			if (at != NULL) { r1 = at->vdw_R; e1 = at->vdw_E; }
			
			top_f << "\t";
			top_f << atmtab[ai]->el.GetSymbol() << (ai + 1) << "\t";
			top_f << atmtab[ai]->el.GetAtomicMass() << "\t";
			top_f << atmtab[ai]->charge << "\t";
			top_f << "A" << "\t";
			top_f << (2.0 * r1) << "\t";
			top_f << e1 << endl;
		}
		
		top_f << endl;
		
		top_f << "[ moleculetype ]" << endl;
		top_f << ";	name	nrexcl" << endl;
		top_f << "	SOL		3" << endl;
		top_f << endl;
		
		top_f << "[ atoms ]" << endl;
		top_f << ";	nr	type	resnr	residu	atom	cgnr	charge" << endl;
		
		for (i32s ind = 0;ind < solvent->current_setup->GetMMAtomCount();ind++)
		{
			const default_at * at; f64 r1 = 0.150; f64 e1 = 0.175;
			at = default_tables::GetInstance()->GetAtomType(atmtab[ind]->atmtp);
			if (at != NULL) { r1 = at->vdw_R; e1 = at->vdw_E; }
			
			top_f << "\t";
			top_f << (ind + 1) << "\t";
			top_f << atmtab[ind]->el.GetSymbol() << (ind + 1) << "\t";	// what this really should be?
			top_f << "1\t";
			top_f << "SOL\t";
			top_f << atmtab[ind]->el.GetSymbol() << (ind + 1) << "\t";	// what this really should be?
			top_f << "1\t";
			top_f << atmtab[ind]->charge << endl;
		}
		
		top_f << endl;
		
		if (eng2->bt1_vector.size() > 0)
		{
			top_f << "[ bonds ]" << endl;
			top_f << ";	ai	aj	funct	c0	c1" << endl;
			
			for (i32u ind = 0;ind < eng2->bt1_vector.size();ind++)
			{
				top_f << "\t";
				top_f << (eng2->bt1_vector[ind].atmi[0] + 1) << "\t";
				top_f << (eng2->bt1_vector[ind].atmi[1] + 1) << "\t";
				top_f << "1\t";
				top_f << eng2->bt1_vector[ind].opt << "\t";
				top_f << eng2->bt1_vector[ind].fc << endl;
			}
			
			top_f << endl;
		}
		
		if (eng2->bt2_vector.size() > 0)
		{
			top_f << "[ angles ]" << endl;
			top_f << ";	ai	aj	ak	funct	c0	c1" << endl;
			
			for (i32u ind = 0;ind < eng2->bt2_vector.size();ind++)
			{
				top_f << "\t";
				top_f << (eng2->bt2_vector[ind].atmi[0] + 1) << "\t";
				top_f << (eng2->bt2_vector[ind].atmi[1] + 1) << "\t";
				top_f << (eng2->bt2_vector[ind].atmi[2] + 1) << "\t";
				top_f << "1\t";
				top_f << (180.0 * eng2->bt2_vector[ind].opt / M_PI) << "\t";	// convert rad->deg!!!
				top_f << eng2->bt2_vector[ind].fc << endl;
			}
			
			top_f << endl;
		}
		
		if (eng2->bt3_vector.size() > 0)
		{
			top_f << "[ dihedrals ]" << endl;
			top_f << ";	ai	aj	ak	al	funct	phi	cp	mult" << endl;
			
			for (i32u ind = 0;ind < eng2->bt3_vector.size();ind++)
			{
				// can set only a single term? then select the most important term!
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
				
				int mult = 1; f64 maxv = eng2->bt3_vector[ind].fc1;
				if (fabs(eng2->bt3_vector[ind].fc2) > fabs(maxv)) { mult = 2; maxv = eng2->bt3_vector[ind].fc2; }
				if (fabs(eng2->bt3_vector[ind].fc3) > fabs(maxv)) { mult = 3; maxv = eng2->bt3_vector[ind].fc3; }
				
				top_f << "\t";
				top_f << (eng2->bt3_vector[ind].atmi[0] + 1) << "\t";
				top_f << (eng2->bt3_vector[ind].atmi[1] + 1) << "\t";
				top_f << (eng2->bt3_vector[ind].atmi[2] + 1) << "\t";
				top_f << (eng2->bt3_vector[ind].atmi[3] + 1) << "\t";
				top_f << "1\t";
				top_f << (maxv < 0.0 ? 180.0 : 0.0) << "\t";
				top_f << fabs(maxv) << "\t";
				top_f << mult << endl;
			}
			
			top_f << endl;
		}
		
		if (eng2->bt4_vector.size() > 0)
		{
			top_f << "[ dihedrals ]" << endl;
			top_f << ";	ai	aj	ak	al	funct	q0	cq" << endl;
			
			for (i32u ind = 0;ind < eng2->bt4_vector.size();ind++)
			{
				top_f << "\t";
				top_f << (eng2->bt4_vector[ind].atmi[1] + 1) << "\t";	// plane
				top_f << (eng2->bt4_vector[ind].atmi[0] + 1) << "\t";	// plane
				top_f << (eng2->bt4_vector[ind].atmi[2] + 1) << "\t";	// plane
				top_f << (eng2->bt4_vector[ind].atmi[3] + 1) << "\t";	// ref
				top_f << "2\t";
				top_f << (180.0 * eng2->bt4_vector[ind].opt / M_PI) << "\t";	// convert rad->deg!!!
				top_f << eng2->bt4_vector[ind].fc << endl;
			}
			
			top_f << endl;
		}
		
		top_f << "[ system ]" << endl;
		top_f << "exported_from_libghemical" << endl;
		top_f << endl;
		
		top_f << "[ molecules ]" << endl;
		top_f << "SOL\t" << (atom_list.size() / solvent->current_setup->GetMMAtomCount()) << endl;
		
		top_f.close();
		
		// write the gro file
		// ^^^^^^^^^^^^^^^^^^
		
		char gro_fn[256]; ostrstream gfns(gro_fn, sizeof(gro_fn));
		gfns << export_gromacs << ".gro" << ends;
		
		ofstream gro_f;
		gro_f.open(gro_fn, ios::out);
		
		gro_f << "; this is a gro file exported by libghemical " << LIBVERSION << ", not an original gromacs file!" << endl;
		gro_f << atom_list.size() << endl;
		
		i32u counter = 0; i32u rescnt = 0;
		for (iter_al it1 = GetAtomsBegin();it1 != GetAtomsEnd();it1++)
		{
			i32u atmcnt = (counter % solvent->current_setup->GetMMAtomCount());
			if (!atmcnt) rescnt++;
			
			char atomname[32];
			ostrstream ans(atomname, sizeof(atomname));
			ans << (* it1).el.GetSymbol() << (atmcnt + 1) << ends;
			
			gro_f << setw(5) << rescnt;
			gro_f << "SOL  ";
			
			int nb = 5 - strlen(atomname); if (nb < 0) nb = 0;
			for (int blank = 0;blank < nb;blank++) gro_f << " ";
			gro_f << atomname;
			
			gro_f << setw(5) << (counter + 1);
			
			const fGL * crd = (* it1).GetCRD(0);
			gro_f << setw(8) << setprecision(3) << crd[0];
			gro_f << setw(8) << setprecision(3) << crd[1];
			gro_f << setw(8) << setprecision(3) << crd[2];
			gro_f << "   0.000";
			gro_f << "   0.000";
			gro_f << "   0.000";
			gro_f << endl;
			
			counter++;
		}
		
		gro_f << (dimx * 2.0) << " " << (dimy * 2.0) << " " << (dimz * 2.0) << endl;
		
		gro_f.close();
	}
	
	delete solvent;
	
//	for (iter_al itX = GetAtomsBegin();itX != GetAtomsEnd();itX++)
//	{ cout << (* itX).index << " " << ((* itX).flags & ATOMFLAG_IS_SOLVENT_ATOM) << endl; }
}

void model::SolvateSphere(fGL rad1, fGL rad2, fGL density, model * solvent)
{
	use_boundary_potential = true;
	boundary_potential_radius1 = rad1;
	boundary_potential_radius2 = rad2;
	SystemWasModified();
	
	if (density <= 0.0) return;	// zero density -> infinite distance.
	const fGL distance = S_Initialize(density, & solvent);
	
	// make the sphere of size rad2...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	i32s lim = (i32s) floor(rad2 / distance) + 2;		// +1 is the minimum.
	
	i32s solvent_molecules_added = 0;
	for (i32s lx = -lim + 1;lx < lim;lx++)
	{
		for (i32s ly = -lim + 1;ly < lim;ly++)
		{
			for (i32s lz = -lim + 1;lz < lim;lz++)
			{
				fGL crdS[3];
				crdS[0] = distance * lx;
				crdS[1] = distance * ly;
				crdS[2] = distance * lz;
				
				if (lz % 2)	// twist the cubic lattice to an octahedral one ; OPTIONAL!
				{
					crdS[0] += 0.5 * distance;
					crdS[1] += 0.5 * distance;
				}
				
				fGL dist = sqrt(crdS[0] * crdS[0] + crdS[1] * crdS[1] + crdS[2] * crdS[2]);
				if (dist > rad2) continue;	// skip if the molecule is too far...
				
				bool skip = false;	// skip if there is overlap with solute atoms ; FOR SMALL SOLVENTS ONLY?!?
				for (iter_al it1 = GetAtomsBegin();it1 != GetAtomsEnd();it1++)
				{
					if ((* it1).flags & ATOMFLAG_IS_SOLVENT_ATOM) continue;
					
					const fGL * crdX = (* it1).GetCRD(0);
					
					fGL dx = crdS[0] - crdX[0];
					fGL dy = crdS[1] - crdX[1];
					fGL dz = crdS[2] - crdX[2];
					
					fGL d2s = sqrt(dx * dx + dy * dy + dz * dz);
					if (d2s < 0.175) skip = true;
					
					if (skip) break;
				}	if (skip) continue;
				
				solvent_molecules_added++;
				
				f64 rot[3];
				rot[0] = 2.0 * M_PI * (f64) rand() / (f64) RAND_MAX;
				rot[1] = 2.0 * M_PI * (f64) rand() / (f64) RAND_MAX;
				rot[2] = 2.0 * M_PI * (f64) rand() / (f64) RAND_MAX;
				
				vector<atom *> av1;
				vector<atom *> av2;
				{
				
				for (iter_al it1 = solvent->GetAtomsBegin();it1 != solvent->GetAtomsEnd();it1++)
				{
					const fGL * orig = (* it1).GetCRD(0);
					
					fGL d2b[3];		// rotate x (y,z)...
					d2b[0] = orig[0];
					d2b[1] = orig[1] * cos(rot[0]) - orig[2] * sin(rot[0]);
					d2b[2] = orig[1] * sin(rot[0]) + orig[2] * cos(rot[0]);
					
					fGL d2c[3];		// rotate y (z,x)...
					d2c[0] = d2b[2] * sin(rot[1]) + d2b[0] * cos(rot[1]);
					d2c[1] = d2b[1];
					d2c[2] = d2b[2] * cos(rot[1]) - d2b[0] * sin(rot[1]);
					
					fGL d2d[3];		// rotate z (x,y)...
					d2d[0] = d2c[0] * cos(rot[2]) - d2c[1] * sin(rot[2]);
					d2d[1] = d2c[0] * sin(rot[2]) + d2c[1] * cos(rot[2]);
					d2d[2] = d2c[2];
					
					d2d[0] += crdS[0];
					d2d[1] += crdS[1];
					d2d[2] += crdS[2];
					
					atom newA((* it1).el, d2d, GetCRDSetCount());
					newA.flags |= ATOMFLAG_IS_SOLVENT_ATOM;
				//	newA.flags |= ATOMFLAG_MEASURE_ND_RDF;		// what about this???
					
					Add_Atom(newA);
					av1.push_back(& (* it1));
					av2.push_back(& GetLastAtom());
				}
				}
				{
				
				for (iter_bl it1 = solvent->GetBondsBegin();it1 != solvent->GetBondsEnd();it1++)
				{
					i32u ind1 = 0;
					while (ind1 < av1.size())
					{
						if ((* it1).atmr[0] == av1[ind1]) break;
						else ind1++;
					}
					
					i32u ind2 = 0;
					while (ind2 < av1.size())
					{
						if ((* it1).atmr[1] == av1[ind2]) break;
						else ind2++;
					}
					
					if (ind1 == av1.size() || ind2 == av1.size())
					{
						cout << "index search failed!" << endl;
						exit(EXIT_FAILURE);
					}
					
					bond newB(av2[ind1], av2[ind2], (* it1).bt);
					AddBond(newB);
				}
				}
			}
		}
	}
	
	cout << "added " << solvent_molecules_added << " solvent molecules." << endl;
	
	delete solvent;
	
//	for (iter_al itX = GetAtomsBegin();itX != GetAtomsEnd();itX++)
//	{ cout << (* itX).index << " " << ((* itX).flags & ATOMFLAG_IS_SOLVENT_ATOM) << endl; }
}

fGL model::S_Initialize(fGL density, model ** ref2solv, int element_number)
{
	// here we set H2O to default solvent if there is no other molecule set,
	// and calculate the distances between the solvent molecules using density.
	
	if (!(* ref2solv))
	{
		// build a water molecule!!!
		// ^^^^^^^^^^^^^^^^^^^^^^^^^
		
		(* ref2solv) = new model();
		
		fGL crd[3] = { 0.0, 0.0, 0.0 };
		atom newA(element(element_number), crd, (* ref2solv)->GetCRDSetCount());
		(* ref2solv)->Add_Atom(newA);
	}
	
	f64 molarmass = 0.0;
	for (iter_al it1 = (* ref2solv)->GetAtomsBegin();it1 != (* ref2solv)->GetAtomsEnd();it1++)
	{
		molarmass += (* it1).el.GetAtomicMass();
	}
	
	if (molarmass < 0.1)
	{
		Message("Could not calculate molar mass!\nFailed to read the solvent file.");
		return NOT_DEFINED;
	}
	
	// density = mass / volume   ->   mass = density * volume
	// calculate the mass (in grams) for 1 dm^3 of solvent.
	
	f64 m1 = 1000.0 * density * 1.0;
	
	// quantity = mass / molar_mass ; convert the mass into quantity
	// and further into number of particles using Avogadro's constant.
	
	f64 n1 = m1 / molarmass;
	f64 n2 = n1 * 6.022e+23;
	
	// now assume that all these particles are placed into a cubic lattice
	// of volume 1 dm^3 with even spacings ; calculate the distance (in nm)
	// between the particles in this lattice.
	
	f64 distance = pow(1.0e+24 / n2, 1.0 / 3.0);
	return distance;
}


fGL model::S_Initialize(fGL density, model ** ref2solv)
{
	// here we set H2O to default solvent if there is no other molecule set,
	// and calculate the distances between the solvent molecules using density.
	
	if (!(* ref2solv))
	{
		// build a water molecule!!!
		// ^^^^^^^^^^^^^^^^^^^^^^^^^
		
		(* ref2solv) = new model();
		const fGL angle = 109.5 * M_PI / 180.0;
		
		fGL crdO[3] = { 0.0, 0.0, 0.0 };
		atom newO(element(8), crdO, (* ref2solv)->GetCRDSetCount());
		(* ref2solv)->Add_Atom(newO); atom * ref_O = & (* ref2solv)->GetLastAtom();
		
		fGL crdH1[3] = { 0.095, 0.0, 0.0 };
		atom newH1(element(1), crdH1, (* ref2solv)->GetCRDSetCount());
		(* ref2solv)->Add_Atom(newH1); atom * ref_H1 = & (* ref2solv)->GetLastAtom();
		
		fGL crdH2[3] = { cos(angle)*0.095, sin(angle)*0.095, 0.0 };
		atom newH2(element(1), crdH2, (* ref2solv)->GetCRDSetCount());
		(* ref2solv)->Add_Atom(newH2); atom * ref_H2 = & (* ref2solv)->GetLastAtom();
		
		bond newb1(ref_O, ref_H1, bondtype('S')); (* ref2solv)->AddBond(newb1);
		bond newb2(ref_O, ref_H2, bondtype('S')); (* ref2solv)->AddBond(newb2);
	}
	
	f64 molarmass = 0.0;
	for (iter_al it1 = (* ref2solv)->GetAtomsBegin();it1 != (* ref2solv)->GetAtomsEnd();it1++)
	{
		molarmass += (* it1).el.GetAtomicMass();
	}
	
	if (molarmass < 0.1)
	{
		Message("Could not calculate molar mass!\nFailed to read the solvent file.");
		return NOT_DEFINED;
	}
	
	// density = mass / volume   ->   mass = density * volume
	// calculate the mass (in grams) for 1 dm^3 of solvent.
	
	f64 m1 = 1000.0 * density * 1.0;
	
	// quantity = mass / molar_mass ; convert the mass into quantity
	// and further into number of particles using Avogadro's constant.
	
	f64 n1 = m1 / molarmass;
	f64 n2 = n1 * 6.022e+23;
	
	// now assume that all these particles are placed into a cubic lattice
	// of volume 1 dm^3 with even spacings ; calculate the distance (in nm)
	// between the particles in this lattice.
	
	f64 distance = pow(1.0e+24 / n2, 1.0 / 3.0);
	return distance;
}

void model::DoEnergy(void)
{
#if USE_ORIGINAL_ENGINE_SELECTION
	//#####################################################################
	// original code 
	//#####################################################################
	engine * eng = GetCurrentSetup()->GetCurrentEngine();
	if (eng == NULL) GetCurrentSetup()->CreateCurrentEngine();
	eng = GetCurrentSetup()->GetCurrentEngine();
#else
	//test code
	engine * eng = GetCurrentSetup()->CreateEngineByIDNumber(ENG1_MM_PERIODIC);
#endif
	//#####################################################################
	if (eng == NULL) return;
	
	char mbuff1[256];
	ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << "Calculating Energy ";
	str1 << "(setup = " << GetCurrentSetup()->GetClassName();
	str1 << ", engine = " << GetCurrentSetup()->GetEngineName(GetCurrentSetup()->GetCurrEngIndex());
	str1 << ")." << endl << ends;
	PrintToLog(mbuff1);
	
	CopyCRD(this, eng, 0);
	eng->Compute(0);
	
	if (dynamic_cast<eng1_sf *>(eng) != NULL) CopyCRD(eng, this, 0);	// for ribbons...
	
	char buffer[128];
	ostrstream str(buffer, sizeof(buffer)); str.setf(ios::fixed); str.precision(8);
	
	str << "Energy = " << eng->energy << " kJ/mol" << endl << ends;
	PrintToLog(buffer);
	
// we will not delete current_eng here, so that we can draw plots using it...
// we will not delete current_eng here, so that we can draw plots using it...
// we will not delete current_eng here, so that we can draw plots using it...
	
	SetupPlotting();
}

void model::DoGeomOpt(geomopt_param & param, bool updt)
{
// make this thread-safe since this can be called from project::pcs_job_RandomSearch() at the app side...
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// this means: 1) make sure that PrintToLog() is safe, 2) only call UpdateAllGraphicsViews(false) because
// OGL-context is owned by the GUI-thread ; calling with "false" will pass the update to the GUI-thread.
	
	if (param.show_dialog)
	{
		ThreadLock();
		GeomOptGetParam(param);
		ThreadUnlock();
		
		if (!param.confirm) return;
	}
	
	ThreadLock();
	//#####################################################################
#if USE_ORIGINAL_ENGINE_SELECTION
	//#####################################################################
	// original code 
	//#####################################################################
	engine * eng = GetCurrentSetup()->GetCurrentEngine();
	if (eng == NULL) GetCurrentSetup()->CreateCurrentEngine();
	eng = GetCurrentSetup()->GetCurrentEngine();
	//#####################################################################
#else
	//test code
	engine * eng = GetCurrentSetup()->CreateEngineByIDNumber(ENG1_MM_PERIODIC);
#endif
	//#####################################################################
	if (eng == NULL)
	{
		ThreadUnlock();
		return;
	}
	
	char mbuff1[256];
	ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << "Starting Geometry Optimization ";
	str1 << "(setup = " << GetCurrentSetup()->GetClassName();
	str1 << ", engine = " << GetCurrentSetup()->GetEngineName(GetCurrentSetup()->GetCurrEngIndex());
	str1 << ")." << endl << ends;
	PrintToLog(mbuff1);
	
	CopyCRD(this, eng, 0);
	if (!m_bMaxMinCoordCalculed)
		CalcMaxMinCoordinates(this, eng, 0);
	
	geomopt * opt = new geomopt(eng, 100, 0.025, 10.0);		// optimal settings?!?!?
#if USE_BOUNDARY_OPT_ON_GEOMOPT
	boundary_opt * b_opt = NULL;
	//printf("param.box_opt = %d\n", param.box_opt);
	if(param.box_opt != geomopt_param::box_optimization_type::no)
	{
		b_opt = new boundary_opt(param.box_opt, this, eng, 100, 0.025, 10.0);
	}


#endif
	char buffer[1024];
	f64  last_energy = 0.0;		// this is for output and delta_e test...
	
	PrintToLog("Cycle    Energy            Gradient       Step        Delta E\n");
	
	ThreadUnlock();
	
	i32s n1 = 0;		// n1 counts the number of steps...
	bool cancel = false;
	while (!cancel)
	{
		opt->TakeCGStep(conjugate_gradient::Newton2An);

#if USE_BOUNDARY_OPT_ON_GEOMOPT
		if (b_opt)
			b_opt->TakeCGStep(conjugate_gradient::Newton2An);
#endif	
// problem: the gradient information is in fact not precise in this stage. the current gradient
// is the one that was last calculated in the search, and it is not necessarily the best one.
// to update the gradient, we need to Compute(1) here. JUST SLOWS GEOMOPT DOWN -> DISABLE!
///////////////////////////////////////////////////////////////////////////////////////////////
//eng->Compute(1);	// this is not vital, but will update the gradient vector length...
///////////////////////////////////////////////////////////////////////////////////////////////
		
		ThreadLock();
		
		if (!(n1 % 5))
		{
			double progress = 0.0;
			if (param.enable_nsteps) progress = (double) n1 / (double) (param.treshold_nsteps);
			
			double graphdata = opt->optval;		// this is an array of size 1...
			
			cancel = SetProgress(progress, & graphdata);
			
			if (n1 != 0)
			{
				sprintf(buffer, "%4d %12.5f kJ/mol  %10.4e %10.4e %10.4e \n", n1,
				opt->optval, eng->GetGradientVectorLength(), opt->optstp, last_energy - opt->optval);
			}
			else
			{
				sprintf(buffer, "%4d %12.5f kJ/mol  %10.4e %10.4e ********** \n", n1,
				opt->optval, eng->GetGradientVectorLength(), opt->optstp);
			}
			
			PrintToLog(buffer);
		}
		
		bool terminate = false;
		
		if (param.enable_nsteps)	// the nsteps test...
		{
			if (n1 >= param.treshold_nsteps)
			{
				terminate = true;
				PrintToLog("the nsteps termination test was passed.\n");
			}
		}
		
		if (param.enable_grad)		// the grad test...
		{
			if (eng->GetGradientVectorLength() < param.treshold_grad)
			{
				terminate = true;
				PrintToLog("the grad termination test was passed.\n");
			}
		}
		
		if (param.enable_delta_e)	// the delta_e test...
		{
			bool flag = false; //const f64 treshold_step = 1.0e-12;		// can we keep this as a constant???
			//if (n1 != 0 && (last_energy - opt->optval) != 0.0 && fabs(last_energy - opt->optval) < param.treshold_delta_e) flag = true;
			if (n1 > param.treshold_delta_e_min_nsteps && fabs(last_energy - opt->optval) < param.treshold_delta_e) flag = true;
			//if ((opt->optstp != 0.0) && (opt->optstp < treshold_step)) flag = true;
			
			if (flag)
			{
				terminate = true;
				PrintToLog("the delta_e termination test was passed.\n");
			}
		}
		
		last_energy = opt->optval;
		
		if (!(n1 % 10) || terminate)
		{
			CopyCRD(eng, this, 0);
			CenterCRDSet(0, false);
			
			UpdateAllGraphicsViews(updt);
		}
		
		ThreadUnlock();
		
		if (terminate) break;		// exit the loop here!!!
		
		n1++;	// update the number of steps...
	}
	
	delete opt;
#if USE_BOUNDARY_OPT_ON_GEOMOPT
	if (b_opt)
		delete b_opt;
#endif
// we will not delete current_eng here, so that we can draw plots using it...
// we will not delete current_eng here, so that we can draw plots using it...
// we will not delete current_eng here, so that we can draw plots using it...
	
	// above, CopyCRD was done eng->mdl and then CenterCRDSet() was done for mdl.
	// this might cause that old coordinates remain in eng object, possibly affecting plots.
	// here we sync the coordinates and other plotting data in the eng object.
	
	ThreadLock();
	CopyCRD(this, eng, 0);
	SetupPlotting();
	ThreadUnlock();
}

void model::GeomOptGetParam(geomopt_param & param)
{
	param.show_dialog = false;
	param.confirm = true;
}

#define LANGEVIN_RANDOM_FC	4850.0		// see moldyn.cpp ; for langevin_frict_fc 16.0e-6 ; 20050124
#define LANGEVIN_COUPLING	0.0005		// see moldyn.cpp ; this is 1/100 temperature coupling...


#include "../../src/glade/moldyn_atomlist_dialog.h"

void model::DoMolDyn(moldyn_param & param, bool updt)
{
#if DIFFUSE_WORKING
	FILE * out;
	out = fopen("diffuse.txt", "wt");
	fclose (out);
#endif
// make this thread-safe since this can be called from project::pcs_job_RandomSearch() at the app side...
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// this means: 1) make sure that PrintToLog() is safe, 2) only call UpdateAllGraphicsViews(false) because
// OGL-context is owned by the GUI-thread ; calling with "false" will pass the update to the GUI-thread.
printf("model::DoMolDyn(moldyn_param & param, bool updt)\n");	
	if (param.show_dialog)
	{
		ThreadLock();
		MolDynGetParam(param);
		ThreadUnlock();
		
		if (!param.confirm)
		{
			return;
		}
	}
	
	ThreadLock();
#if USE_ORIGINAL_ENGINE_SELECTION
	//#####################################################################
	// original code 
	//#####################################################################
	engine * eng = GetCurrentSetup()->GetCurrentEngine();
	if (eng == NULL) GetCurrentSetup()->CreateCurrentEngine();
	eng = GetCurrentSetup()->GetCurrentEngine();
	//#####################################################################
#else
	//test code
	engine * eng = GetCurrentSetup()->CreateEngineByIDNumber(ENG1_MM_PERIODIC);
#endif
	
	if (eng == NULL)
	{
		ThreadUnlock();
		return;
	}
	
	// eng_mbp != NULL if we will use a system with boundary potential...
	engine_mbp * eng_mbp = dynamic_cast<engine_mbp *>(eng);
	eng_mbp = NULL;
	// eng_pbc != NULL if we will use a system with periodic boundary conditions...
#if USE_ENGINE_PBC2
	engine_pbc2 * eng_pbc = dynamic_cast<engine_pbc2 *>(eng);
#else
	engine_pbc * eng_pbc = dynamic_cast<engine_pbc *>(eng);
#endif /*USE_ENGINE_PBC2*/
	
// THIS IS OPTIONAL!!! FOR BOUNDARY POTENTIAL STUFF ONLY!!!
//if (eng_mbp != NULL) eng_mbp->nd_eval = new number_density_evaluator(eng_mbp, false, 20);
// THIS IS OPTIONAL!!! FOR RADIAL DENSITY FUNCTION STUFF ONLY!!!
//if (eng_mbp != NULL) eng_mbp->rdf_eval = new radial_density_function_evaluator(eng_mbp, 80, 0.15, 0.95);		// 0.95 - 0.15 = 0.80
//if (eng_mbp != NULL) eng_mbp->rdf_eval = new radial_density_function_evaluator(eng_mbp, 50, 0.15, 0.65, 0.00, 0.75);	// 0.65 - 0.15 = 0.50

	char mbuff1[1024];
	ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << "Starting Molecular Dynamics ";
	str1 << "(setup = " << GetCurrentSetup()->GetClassName();
	str1 << ", engine = " << GetCurrentSetup()->GetEngineName(GetCurrentSetup()->GetCurrEngIndex()) << str1 << ")." << endl;
	str1 << "MD steps " << param.nsteps_h << " " << param.nsteps_e << " " << param.nsteps_s << " ";
	str1 << "CE= " << param.constant_e << " LA= " << param.constant_e << " ";
	str1 << "T= " << param.temperature << " ts= " << param.timestep << " ";
	str1 << ends;
	PrintToLog(mbuff1);
	
	CopyCRD(this, eng, 0);
	if (!m_bMaxMinCoordCalculed)
		CalcMaxMinCoordinates(this, eng, 0);

	//this->Correct_periodic_box_HALFdim(eng);

	eng1_sf * engsf = dynamic_cast<eng1_sf *>(eng);
	
	moldyn * dyn = NULL;
	if (!param.langevin) dyn = new moldyn(eng, param.timestep);
	else dyn = new moldyn_langevin(eng, param.timestep);

	//#######################################################################
	//#######################################################################
	if (false)
		new moldyn_atomlist_dialog(dyn);		// the object will call delete itself...
	//#######################################################################
	//#######################################################################

	moldyn_langevin * dyn_l = dynamic_cast<moldyn_langevin *>(dyn);
	
	dyn->temperature_coupling = 0.075;				// set the initial MD settings...
	if (dyn_l != NULL) dyn_l->langevin_coupling = 0.000;		// set the initial MD settings...
	/////////////////////////////////////////////////////
	i32s last_dot;
	char outfilename[1024];
	strcpy(outfilename, param.filename);

#if SNARJAD_TARGET_WORKING || PROBNIY_ATOM_WORKING
	char ofilename[1024];
	strcpy(ofilename, param.filename);
	last_dot = NOT_DEFINED;
	for (i32u fn = 0;fn < strlen(ofilename);fn++)
	{
		if (ofilename[fn] == '.') last_dot = (i32s) fn;
	}	
	if (last_dot < 0) last_dot = strlen(ofilename);
	ofilename[last_dot + 0] = '\0'; 


#if SNARJAD_TARGET_WORKING
	sprintf(outfilename, "%s_vel_%0.3f.traj", ofilename, param.start_snarjad_vel[2]);
#endif

#if PROBNIY_ATOM_WORKING
	sprintf(outfilename, "%s_at_mv_dir_%d.traj", ofilename, this->prob_atom_move_direction);
#endif

#endif

	printf("outfilename = %s\n", outfilename);
	/////////////////////////////////////////////////////
	ofstream ofile;
	//ofile.open(param.filename, ios::out | ios::binary);
	ofile.open(outfilename, ios_base::out | ios_base::trunc | ios_base::binary);
	
	char logfilename[1024];
	strcpy(logfilename, param.filename);
	last_dot = NOT_DEFINED;
	for (i32u fn = 0;fn < strlen(logfilename);fn++)
	{
		if (logfilename[fn] == '.') last_dot = (i32s) fn;
	}	if (last_dot < 0) last_dot = strlen(logfilename);
	logfilename[last_dot + 0] = '.'; logfilename[last_dot + 1] = 'l';
	logfilename[last_dot + 2] = 'o'; logfilename[last_dot + 3] = 'g';
	logfilename[last_dot + 4] = 0;
	
	ofstream logfile;
	logfile.open(logfilename, ios::out);
	
// MOVE THIS INTO PROJECT::WRITETRAJECTORY!!!!!!!!!!!!!!
// MOVE THIS INTO PROJECT::WRITETRAJECTORY!!!!!!!!!!!!!!
// MOVE THIS INTO PROJECT::WRITETRAJECTORY!!!!!!!!!!!!!!

	const int number_of_atoms = GetAtomCount();
	const char file_id[10] = "traj_v10";
	
	const int frame_save_frq = 100;
	const int total_frames = param.nsteps_s / frame_save_frq;
	
	ofile.write((char *) file_id, 8);					// file id, 8 chars.
	ofile.write((char *) & number_of_atoms, sizeof(number_of_atoms));	// number of atoms, int.
	ofile.write((char *) & total_frames, sizeof(total_frames));		// total number of frames, int.
	
	ThreadUnlock();
	
	ostrstream str2b(mbuff1, sizeof(mbuff1));
	str2b << "step\t" << "T\t";
	str2b << "Epot\t" << "Efull\t";
	str2b << "E_solute\t";		// a primitive implementation for energy components ; FIXME!!!
	str2b << "E_solvent\t";	// a primitive implementation for energy components ; FIXME!!!
	str2b << "E_solusolv\t";	// a primitive implementation for energy components ; FIXME!!!
	str2b << "NThroughZ\t";
	str2b << "NThroughZmin\t";
	str2b << "NThroughZmax\t";
	str2b << endl << ends;
	logfile << mbuff1;

#if SNARJAD_TARGET_WORKING
	i32s n_snarjad = eng->GetAtomCount() - 1;
	printf("pre LockAtom()\n");
	dyn->LockAtom(n_snarjad);
	// устанавливаем цель
	dyn->SetTarget(param.target_list);
	dyn->n_snarjad = n_snarjad;

	char logfilename2[1024];
	strcpy(logfilename2, param.filename);
	last_dot = NOT_DEFINED;
	for (i32u fn = 0;fn < strlen(logfilename2);fn++)
	{
		if (logfilename2[fn] == '.') last_dot = (i32s) fn;
	}	
	if (last_dot < 0) last_dot = strlen(logfilename2);
	logfilename2[last_dot + 0] = '\0'; 


	sprintf(logfilename, "%s_vel_%0.3f.log", logfilename2, param.start_snarjad_vel[2]);
	
	printf("logfilename = %s\n", logfilename);

	ofstream logfile2;
	logfile2.open(logfilename, ios::out);

	{	
		ostrstream str2b(mbuff1, sizeof(mbuff1));
		str2b << "step," << "T,";
		str2b << "r," << "rz," << "Epot,";
		str2b << "sn_vel[0],";
		str2b << "sn_vel[1],";
		str2b << "sn_vel[2],";
		str2b << "sn_crd[0],";
		str2b << "sn_crd[1],";
		str2b << "sn_crd[2],";
		str2b << "sn_f[0],";
		str2b << "sn_f[1],";
		str2b << "sn_f[2],";
		str2b << "target_crd[0],";
		str2b << "target_crd[1],";
		str2b << "target_crd[2]";

#if USE_BOUNDARY_OPT_ON_MOLDYN
		str2b << ",boundary[0]";
		str2b << ",boundary[1]";
		str2b << ",boundary[2]";
#endif

		str2b << endl << ends;
		logfile2 << mbuff1;
	}
#endif /*SNARJAD_TARGET_WORKING*/

#if PROBNIY_ATOM_WORKING
	i32s n_prob_atom = eng->GetAtomCount() - 1;
	printf("pre LockAtom()\n");
	dyn->LockAtom(n_prob_atom);
	// устанавливаем цель
	dyn->SetTarget(param.target_list);
	dyn->n_prob_atom = n_prob_atom;

	char logfilename2[1024];
	strcpy(logfilename2, param.filename);
	last_dot = NOT_DEFINED;
	for (i32u fn = 0;fn < strlen(logfilename2);fn++)
	{
		if (logfilename2[fn] == '.') last_dot = (i32s) fn;
	}	
	if (last_dot < 0) last_dot = strlen(logfilename2);
	logfilename2[last_dot + 0] = '\0'; 


	sprintf(logfilename, "%s_pr_at_mv_dir_%d.log", logfilename2, prob_atom_move_direction ? +1 : -1);
	
	printf("logfilename = %s\n", logfilename);

	ofstream logfile2;
	logfile2.open(logfilename, ios::out);

	{	
		ostrstream str2b(mbuff1, sizeof(mbuff1));
		str2b << "step," << "T,";
		str2b << "rz," << "Epot,";

		str2b << "prob_atom_crd[0],";
		str2b << "prob_atom_crd[1],";
		str2b << "prob_atom_crd[2],";

		str2b << "prob_atom_f[0],";
		str2b << "prob_atom_f[1],";
		str2b << "prob_atom_f[2],";

		str2b << "target_crd[0],";
		str2b << "target_crd[1],";
		str2b << "target_crd[2]";
/*
#if USE_BOUNDARY_OPT_ON_MOLDYN
		str2b << ",boundary[0]";
		str2b << ",boundary[1]";
		str2b << ",boundary[2]";
#endif
*/
		str2b << endl << ends;
		logfile2 << mbuff1;
	}
#endif /*PROBNIY_ATOM_WORKING*/

#if USE_BOUNDARY_OPT_ON_MOLDYN
	boundary_opt * b_opt = new boundary_opt(
		geomopt_param::box_optimization_type::xyz,
		this, eng, 100, 0.025, 10.0);
#endif

	for (i32s n1 = 0;n1 < param.nsteps_h + param.nsteps_e + param.nsteps_s;n1++)
	{
		Sleep(10);
		if (!(n1 % 10))
		{
			//#############################################
			eng1_mm * engmm = dynamic_cast<eng1_mm *>(eng);
			if (engmm)	engmm->UpdateTerms();
			//#############################################
		}
		if (!(n1 % 10) && eng_pbc != NULL) eng_pbc->CheckLocations();
		if (!(n1 % 10) && eng_pbc != NULL) eng_pbc->update = true;
		
		if (n1 < param.nsteps_h && !(n1 % 100))
		{
			dyn->temperature = param.temperature * ((f64) n1 / (f64) param.nsteps_h);
			//cout << "setting T = " << dyn->temperature << endl;
			
			if (dyn_l != NULL)
			{
				dyn_l->langevin_random_fc = ((f64) n1 / (f64) param.nsteps_h) * LANGEVIN_RANDOM_FC;
			}
		}
		else if (n1 == param.nsteps_h)
		{
			f64 final_tc = 0.010;
			if (engsf != NULL) final_tc = 0.050;		// ???
			
			dyn->temperature = param.temperature;		// set the final MD settings...
			dyn->temperature_coupling = final_tc;		// set the final MD settings...
			
			if (dyn_l != NULL)
			{
				dyn_l->langevin_random_fc = LANGEVIN_RANDOM_FC;
				dyn_l->langevin_coupling = LANGEVIN_COUPLING;
			}
		}
		
		bool enable_tc = false;
		if (n1 < param.nsteps_h + param.nsteps_e) enable_tc = true;
		if (!param.constant_e || param.langevin) enable_tc = true;

#if SNARJAD_TARGET_WORKING
		if (n1 == param.nsteps_h + param.nsteps_e)
		{
			// запуск снаряда по цели
			printf("zapusk_snarjada\n");
			f64 start_snarjad_crd[3];
			// извлекаем начальные координаты снаряда
			dyn->GetSnarjadVelCrd(NULL, start_snarjad_crd, NULL);
			// устанавливаем цель
			dyn->SetTarget(param.target_list);
			// расчитываем координаты цели
			dyn->ComputeTargetCrd();
			// корректируем x и y координаты снаряда, чтобы они совпадали с центром цели
			start_snarjad_crd[0] = dyn->target_crd[0];
			start_snarjad_crd[1] = dyn->target_crd[1];

			if (param.start_snarjad_vel[2] > 0)
			{
				start_snarjad_crd[2] = dyn->target_crd[2]-0.5;
			}
			else
			{
				start_snarjad_crd[2] = dyn->target_crd[2]+0.5;
			}

			// запуск снаряда по цели
			dyn->ShootSnarjad(n_snarjad, param.start_snarjad_vel, start_snarjad_crd);
			//MessageBox(0,"","",0);
		}
#endif
#if PROBNIY_ATOM_WORKING
		f64 prob_atom_crd[3];
		if (n1 == param.nsteps_h + param.nsteps_e)
		{
			// устанавливаем цель
			dyn->SetTarget(param.target_list);
		}
		if (
			n1 >= param.nsteps_h + param.nsteps_e &&
			!(n1 % 10))
		{
			int iii = n1 - param.nsteps_h + param.nsteps_e;
			f64 step = 0.001;
			// расчитываем координаты цели
			dyn->ComputeTargetCrd();
			// корректируем x и y координаты prob_atom, чтобы они совпадали с центром цели
			prob_atom_crd[0] = dyn->target_crd[0];
			prob_atom_crd[1] = dyn->target_crd[1];

			if (this->prob_atom_move_direction)
			{
				prob_atom_crd[2] = dyn->target_crd[2]-0.5 + iii*step;
			}
			else
			{
				prob_atom_crd[2] = dyn->target_crd[2]+0.5 - iii*step;
			}
			dyn->SetProbAtom(n_prob_atom, prob_atom_crd);
		}

#endif
		dyn->TakeMDStep(enable_tc);
#if USE_BOUNDARY_OPT_ON_MOLDYN
		if (!(n1 % 10)) b_opt->TakeCGStep(conjugate_gradient::Newton2An);
#endif

#if PROBNIY_ATOM_WORKING
		if (
			n1 >= param.nsteps_h + param.nsteps_e &&
			!(n1 % 10))
		{
			f64 prob_atom_f[3];
			dyn->GetProbAtomForce(prob_atom_f);
			double rz = prob_atom_crd[2] - dyn->target_crd[2];

			ostrstream str2b(mbuff1, sizeof(mbuff1));
			str2b << n1 << "," << dyn->ConvEKinTemp(dyn->GetEKin()) << ",";
			str2b << rz << "," << dyn->GetEPot() << ",";
			str2b << prob_atom_crd[0] << "," << prob_atom_crd[1] << "," << prob_atom_crd[2] << ",";
			str2b << -prob_atom_f[0] << "," << -prob_atom_f[1] << "," << -prob_atom_f[2] << ",";
			str2b << dyn->target_crd[0] << "," << dyn->target_crd[1] << "," << dyn->target_crd[2];
/*#if USE_BOUNDARY_OPT_ON_MOLDYN
			str2b 
				<< "," << this->periodic_box_HALFdim[0] 
				<< "," << this->periodic_box_HALFdim[1]
				<< "," << this->periodic_box_HALFdim[2];
#endif*/
			str2b << endl << ends;

			logfile2 << mbuff1;
			PrintToLog(mbuff1);

		}
#endif

#if SNARJAD_TARGET_WORKING
		if (
			n1 > param.nsteps_h + param.nsteps_e &&
			!(n1 % 10))
		{
			f64 sn_crd[3], sn_vel[3], sn_f[3];
			// извлекаем координаты снаряда
			dyn->GetSnarjadVelCrd(sn_vel, sn_crd, sn_f);
			// расчитываем координаты цели
			dyn->ComputeTargetCrd();

			double r = 0.0;

			for(i32s n2 = 0; n2 < 3; n2++)
			{
				double tmp0 = dyn->target_crd[n2] - sn_crd[n2];
				r += tmp0 * tmp0;
			}
			r = sqrt(r);

			double rz = sn_crd[2] - dyn->target_crd[2];
			// если предыдущий шаг инициализирован
			if(b_snarjad_through_membrana_started)
			{
				// если это не проход через граничные периодические условия
				if (fabs(m_pre_snarjad_crd_z-sn_crd[2]) < this->periodic_box_HALFdim[2])
				{
					// если сняряд прошёл цель с отрицательной стороны
					if (m_pre_snarjad_crd_z < m_pre_target_crd_z
						&&
						sn_crd[2] >= dyn->target_crd[2])
					{
						n_snarjad_through_membrana++;
					}
					// если сняряд прошёл цель с положительной стороны
					if (m_pre_snarjad_crd_z >= m_pre_target_crd_z
						&&
						sn_crd[2] < dyn->target_crd[2])
					{
						n_snarjad_through_membrana--;
					}
				}
			}


			m_pre_snarjad_crd_z					= sn_crd[2];
			m_pre_target_crd_z					= dyn->target_crd[2];
			//m_pre_snarjad_membrana_dist_z		= rz;
			b_snarjad_through_membrana_started	= true;

			ostrstream str2b(mbuff1, sizeof(mbuff1));
			str2b << n1 << "," << dyn->ConvEKinTemp(dyn->GetEKin()) << ",";
			str2b << r << "," << rz << "," << dyn->GetEPot() << ",";
			str2b << sn_vel[0] << "," << sn_vel[1] << "," << sn_vel[2] << ",";
			str2b << sn_crd[0] << "," << sn_crd[1] << "," << sn_crd[2] << ",";
			str2b << -sn_f[0] << "," << -sn_f[1] << "," << -sn_f[2] << ",";
			str2b << dyn->target_crd[0] << "," << dyn->target_crd[1] << "," << dyn->target_crd[2];
#if USE_BOUNDARY_OPT_ON_MOLDYN
			str2b 
				<< "," << this->periodic_box_HALFdim[0] 
				<< "," << this->periodic_box_HALFdim[1]
				<< "," << this->periodic_box_HALFdim[2];
#endif
			str2b << endl << ends;

			logfile2 << mbuff1;
			PrintToLog(mbuff1);
		}

#endif
		if (!(n1 % 10))
		{
			ThreadLock();
			
			double progress = (double) (n1 + 1) / (double) (param.nsteps_h + param.nsteps_e + param.nsteps_s);
			bool cancel = SetProgress(progress, NULL);
			
			if (cancel)
			{
				ThreadUnlock();
				break;
			}
			
			ostrstream str2a(mbuff1, sizeof(mbuff1));
			str2a << "step " << n1 << "  T = " << dyn->ConvEKinTemp(dyn->GetEKin()) << " K  ";
			str2a << "Epot = " << dyn->GetEPot() << " kJ/mol  Etot = " << (dyn->GetEKin() + dyn->GetEPot()) << " kJ/mol ";
			str2b << "NThroughZ = "  <<  eng_pbc->GetNThroughZ() << "";
			str2b << "NThroughZmin = "  <<  eng_pbc->GetNThroughZmin() << "";
			str2b << "NThroughZmax = "  <<  eng_pbc->GetNThroughZmax() << "";
			str2a << endl << ends;
			PrintToLog(mbuff1);
#if 0
			ostrstream str2b(mbuff1, sizeof(mbuff1));
			str2b << "step " << n1 << " T = " << dyn->ConvEKinTemp(dyn->GetEKin()) << " ";
			str2b << "Epot = " << dyn->GetEPot() << " Etot = " << (dyn->GetEKin() + dyn->GetEPot()) << " ;; ";
			str2b << "E_solute = " << eng->E_solute << " ";		// a primitive implementation for energy components ; FIXME!!!
			str2b << "E_solvent = " << eng->E_solvent << " ";	// a primitive implementation for energy components ; FIXME!!!
			str2b << "E_solusolv = " << eng->E_solusolv << " ";	// a primitive implementation for energy components ; FIXME!!!
			str2b << endl << ends;
			logfile << mbuff1;
#else			
			
			ostrstream str2b(mbuff1, sizeof(mbuff1));
			str2b << /*`"step " <<*/ n1 << "\t";
			str2b << dyn->ConvEKinTemp(dyn->GetEKin()) << "\t";
			str2b << /*"Epot = " <<*/ dyn->GetEPot() << "\t";
			str2b << (dyn->GetEKin() + dyn->GetEPot()) << "\t";
			str2b << /*"E_solute = " <<*/ eng->E_solute << "\t";		// a primitive implementation for energy components ; FIXME!!!
			str2b << /*"E_solvent = " <<*/ eng->E_solvent << "\t";	// a primitive implementation for energy components ; FIXME!!!
			str2b << /*"E_solusolv = " <<*/ eng->E_solusolv << "\t";	// a primitive implementation for energy components ; FIXME!!!
			str2b << eng_pbc->GetNThroughZ() << "\t";
			str2b << eng_pbc->GetNThroughZmin() << "\t";
			str2b << eng_pbc->GetNThroughZmax() << "\t";
			str2b << endl << ends;
			logfile << mbuff1;
#endif
			ThreadUnlock();
		}
		
		if (!(n1 % 1000))
		{
			if (eng_mbp != NULL && eng_mbp->nd_eval != NULL)
			{
				ThreadLock();
				
				ostrstream str3(mbuff1, sizeof(mbuff1));
				eng_mbp->nd_eval->PrintResults(str3); str3 << ends;
				PrintToLog(mbuff1); logfile << mbuff1;
				
				ThreadUnlock();
			}
			
			if (eng_mbp != NULL && eng_mbp->rdf_eval != NULL)
			{
				ThreadLock();
				
				ostrstream str3(mbuff1, sizeof(mbuff1));
				eng_mbp->rdf_eval->PrintResults(str3); str3 << ends;
				PrintToLog(mbuff1); logfile << mbuff1;
				
				ThreadUnlock();
			}
		}
		
		if (!(n1 < param.nsteps_h + param.nsteps_e) && !(n1 % frame_save_frq))
		{
			CopyCRD(eng, this, 0);
			
			const float ekin = dyn->GetEKin();
			const float epot = dyn->GetEPot();
			
			ofile.write((char *) & ekin, sizeof(ekin));	// kinetic energy, float.
			ofile.write((char *) & epot, sizeof(epot));	// potential energy, float.
			
			for (iter_al itx = GetAtomsBegin();itx != GetAtomsEnd();itx++)
			{
				const fGL * cdata = (* itx).GetCRD(0);
				for (i32s t4 = 0;t4 < 3;t4++)		// all coordinates, float.
				{
					float t1a = cdata[t4];
					ofile.write((char *) & t1a, sizeof(t1a));
				}
			}
		}
		
		//if (!(n1 % 100))
		if (!(n1 % 10))
		{
			ThreadLock();
			
			CopyCRD(eng, this, 0);
			UpdateAllGraphicsViews(updt);
			
			ThreadUnlock();
		}
	}
	
	ofile.close();
	logfile.close();
#if SNARJAD_TARGET_WORKING || PROBNIY_ATOM_WORKING
	logfile2.close();
#endif
	
	delete dyn;
#if USE_BOUNDARY_OPT_ON_MOLDYN
	delete b_opt;
#endif
// we will not delete current_eng here, so that we can draw plots using it...
// we will not delete current_eng here, so that we can draw plots using it...
// we will not delete current_eng here, so that we can draw plots using it...
	
	// above, CopyCRD was done eng->mdl and then CenterCRDSet() was done for mdl.
	// this might cause that old coordinates remain in eng object, possibly affecting plots.
	// here we sync the coordinates and other plotting data in the eng object.
	
	ThreadLock();
	CopyCRD(this, eng, 0);
	SetupPlotting();
	ThreadUnlock();
}

void model::MolDynGetParam(moldyn_param & param)
{
	param.show_dialog = false;
	param.confirm = true;
}

void model::DoRandomSearch(i32s cycles, i32s optsteps, bool updt)
{
// make this thread-safe since this can be called from project::pcs_job_RandomSearch() at the app side...
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// this means: 1) make sure that PrintToLog() is safe, 2) only call UpdateAllGraphicsViews(false) because
// OGL-context is owned by the GUI-thread ; calling with "false" will pass the update to the GUI-thread.
	
	ThreadLock();
	
// eng is not really needed here, but just check that it's available...
// see also project::DoEnergyPlot1D and project::DoEnergyPlot2D
	engine * eng = GetCurrentSetup()->GetCurrentEngine();
	if (eng == NULL) GetCurrentSetup()->CreateCurrentEngine();
	eng = GetCurrentSetup()->GetCurrentEngine();
	if (eng == NULL)
	{
		ThreadUnlock();
		return;
	}
	
	if (cs_vector.size() < 2)
	{
		PushCRDSets(1);
		SetCRDSetVisible(1, false);
	}
	
	random_search rs(this, 0, 0, 1, cycles, optsteps);
	
	ThreadUnlock();
	
	bool cancel = false;
	while (!cancel)
	{
		int zzz = rs.TakeStep();
		//cout << zzz << endl;
		
		ThreadLock();
		
		if (rs.last_step != NOT_DEFINED)
		{
			char mbuff1[256]; strstream str1(mbuff1, sizeof(mbuff1));
			str1 << "step " << rs.last_step << "/" << cycles << "   energy = " << rs.last_E << " kJ/mol" << endl << ends;
			PrintToLog(mbuff1);
			
			double progress = (double) rs.last_step / (double) cycles;
			cancel = SetProgress(progress, NULL);
		}
		
		static int updatefrq = 0;
		if (!(updatefrq % 10))
		{
			UpdateAllGraphicsViews(updt);
		}
		
		ThreadUnlock();
		
		if (zzz < 0) break;
	}
	
	ThreadLock();
	
	CopyCRDSet(1, 0);	// get the best structure!!!
	PopCRDSets(1);		// remove the extra crd-sets.
	
	DiscardCurrEng();	// the coordinates are changed -> disable plotting.
	//SetupPlotting();	// is this an alternative??? need to check this...
	
	UpdateAllGraphicsViews(updt);
	
	char mbuff1[256]; strstream str1(mbuff1, sizeof(mbuff1));
	str1 << "lowest energy found = " << rs.GetMinEnergy() << " kJ/mol" << endl << ends;
	PrintToLog(mbuff1);
	
	ostringstream oss2;
	oss2 << "RANDOM SEARCH IS READY";
	if (cancel) oss2 << " (cancelled)";
	oss2 << "." << endl << ends;
	PrintToLog(oss2.str().c_str());
	
	ThreadUnlock();
}

void model::DoSystematicSearch(i32s divisions, i32s optsteps, bool)
{
// eng is not really needed here, but just check that it's available...
// see also project::DoEnergyPlot1D and project::DoEnergyPlot2D
engine * eng = GetCurrentSetup()->GetCurrentEngine();
if (eng == NULL) GetCurrentSetup()->CreateCurrentEngine();
eng = GetCurrentSetup()->GetCurrentEngine();
if (eng == NULL) return;

	if (cs_vector.size() < 2)
	{
		PushCRDSets(1);
		SetCRDSetVisible(1, false);
	}
	
	systematic_search ss(this, 0, 0, 1, divisions, optsteps);
	
	while (true)
	{
		int zzz = ss.TakeStep();
	//	cout << zzz << endl;
		
		UpdateAllGraphicsViews(true);
		if (zzz < 0) break;
	}
	
	CopyCRDSet(1, 0);	// get the best structure!!!
	PopCRDSets(1);		// remove the extra crd-sets.
	
	DiscardCurrEng();	// the coordinates are changed -> disable plotting.
	//SetupPlotting();	// is this an alternative??? need to check this...
	
	UpdateAllGraphicsViews(true);
	
	char mbuff1[256]; strstream str1(mbuff1, sizeof(mbuff1));
	str1 << "lowest energy found = " << ss.GetMinEnergy() << " kJ/mol" << endl << ends;
	PrintToLog(mbuff1);
	
	PrintToLog("SYSTEMATIC SEARCH IS READY.\n");
}

void model::DoMonteCarloSearch(i32s n_init_steps, i32s n_simul_steps, i32s optsteps, bool)
{
// eng is not really needed here, but just check that it's available...
// see also project::DoEnergyPlot1D and project::DoEnergyPlot2D
engine * eng = GetCurrentSetup()->GetCurrentEngine();
if (eng == NULL) GetCurrentSetup()->CreateCurrentEngine();
eng = GetCurrentSetup()->GetCurrentEngine();
if (eng == NULL) return;

	
	if (cs_vector.size() < 2)
	{
		PushCRDSets(1);
		SetCRDSetVisible(1, false);
	}
	
	monte_carlo_search mcs(this, 0, 0, 1, n_init_steps, n_simul_steps, optsteps);
	
	while (true)
	{
		int zzz = mcs.TakeStep();
	//	cout << zzz << endl;
		
		UpdateAllGraphicsViews(true);
		if (zzz < 0) break;
	}
	
	CopyCRDSet(1, 0);	// get the best structure!!!
	PopCRDSets(1);		// remove the extra crd-sets.
	
	DiscardCurrEng();	// the coordinates are changed -> disable plotting.
	//SetupPlotting();	// is this an alternative??? need to check this...
	
	UpdateAllGraphicsViews(true);
	
	char mbuff1[256]; strstream str1(mbuff1, sizeof(mbuff1));
	str1 << "lowest energy found = " << mcs.GetMinEnergy() << " kJ/mol" << endl << ends;
	PrintToLog(mbuff1);
	
	PrintToLog("MONTE CARLO SEARCH IS READY.\n");
}

/*##############################################*/
/*##############################################*/

// what comes to PDB chain id's here, we follow the "pdb-select principle" by converting
// the empty id ' ' with '_' to improve readability...

readpdb_mdata * model::readpdb_ReadMData(const char * filename)
{
	cout << "reading PDB metadata from file " << filename << endl;
	
	readpdb_mdata * mdata = new readpdb_mdata;
	
	char buffer[1024];
	
	ifstream file(filename, ios::in);
	if (file.fail())
	{
		cout << "file \"" << filename << "\" not found." << endl;
		
		file.close();
		return mdata;
	}
	
	while (!file.eof())
	{
		char record_id[8];
		for (i32s n1 = 0;n1 < 6;n1++)
		{
			char tchar = (char) file.get();
			if (tchar != ' ') record_id[n1] = tchar;
			else record_id[n1] = 0;
		}
		
		record_id[6] = 0;	// terminate the record string...
		
		if (!strcmp("SEQRES", record_id))
		{
			i32s blocknum; file >> blocknum;
			file.get(); char chn_id = (char) file.get();
			i32s chn_length; file >> chn_length;
			
			if (chn_id == ' ') chn_id = '_';	// fix the empty chain id...
			
			i32s chn_num;
			if (blocknum == 1)	// this is a new record...
			{
				cout << "found a new chain " << mdata->chn_vector.size();
				cout << " '" << chn_id << "' with " << chn_length << " residues." << endl;
				
				readpdb_mdata_chain * new_data = new readpdb_mdata_chain;
				new_data->chn_id = chn_id; new_data->seqres = new char[chn_length + 1];
				
				for (i32s n1 = 0;n1 < chn_length;n1++) new_data->seqres[n1] = '?';
				new_data->seqres[chn_length] = 0;	// terminate the string!!!
				
				chn_num = mdata->chn_vector.size();
				mdata->chn_vector.push_back(new_data);
			}
			else		// this is an old record, find it...
			{
				chn_num = 0;
				while (chn_num < ((i32s) mdata->chn_vector.size()))
				{
					if (mdata->chn_vector[chn_num]->chn_id == chn_id) break;
					else chn_num++;
				}
				
				if (chn_num == ((i32s) mdata->chn_vector.size()))
				{
					cout << "BUG : readpdb_ReadMData : unknown chain found!!!" << endl;
					exit(EXIT_FAILURE);
				}
			}
			
			chn_length = strlen(mdata->chn_vector[chn_num]->seqres);
			i32s counter = (blocknum - 1) * 13;
			
			for (i32s n1 = 0;n1 < 13;n1++)
			{
				char residue[16];
				file >> residue;
				
				char symbol = '?';
				if (!strcmp("ALA", residue)) symbol = 'A';
				if (!strcmp("ARG", residue)) symbol = 'R';
				if (!strcmp("ASN", residue)) symbol = 'N';
				if (!strcmp("ASP", residue)) symbol = 'D';
				if (!strcmp("CYS", residue)) symbol = 'C';
				if (!strcmp("GLN", residue)) symbol = 'Q';
				if (!strcmp("GLU", residue)) symbol = 'E';
				if (!strcmp("GLY", residue)) symbol = 'G';
				if (!strcmp("HIS", residue)) symbol = 'H';
				if (!strcmp("ILE", residue)) symbol = 'I';
				if (!strcmp("LEU", residue)) symbol = 'L';
				if (!strcmp("LYS", residue)) symbol = 'K';
				if (!strcmp("MET", residue)) symbol = 'M';
				if (!strcmp("PHE", residue)) symbol = 'F';
				if (!strcmp("PRO", residue)) symbol = 'P';
				if (!strcmp("SER", residue)) symbol = 'S';
				if (!strcmp("THR", residue)) symbol = 'T';
				if (!strcmp("TRP", residue)) symbol = 'W';
				if (!strcmp("TYR", residue)) symbol = 'Y';
				if (!strcmp("VAL", residue)) symbol = 'V';
				
				mdata->chn_vector[chn_num]->seqres[counter++] = symbol;
				if (counter == chn_length) break;
			}
		}
		
		file.getline(buffer, sizeof(buffer));		// move to the next line...
	}
	
	file.close();
	
	// ready...
	
	if (mdata->chn_vector.empty()) cout << "WARNING : no chains found!!!" << endl;
	cout << "done." << endl;
	
	return mdata;
}

// chn_num must be either NOT_DEFINED (which is -1 meaning all chains) or the chain index in mdata...
// chn_num must be either NOT_DEFINED (which is -1 meaning all chains) or the chain index in mdata...
// chn_num must be either NOT_DEFINED (which is -1 meaning all chains) or the chain index in mdata...

//#define READPDB_ENABLE_MULTIPLE_CRDSETS	// enable this to read all data from NMR entries...

void model::readpdb_ReadData(const char * filename, readpdb_mdata * mdata, i32s chn_num)
{
	cout << "reading PDB data from file " << filename << endl;
	
	ifstream file(filename, ios::in);
	if (file.fail())
	{
		cout << "file \"" << filename << "\" not found." << endl;
		
		file.close();
		return;
	}
	
	// read the whole file to temporary arrays...
	
	vector<readpdb_data_atom> atom_data;
	vector<readpdb_data_ssbond> ssbond_data;
	
	i32s model_counter = 0;
	i32s atom_counter = NOT_DEFINED;
	
	char buffer[1024];
	
	while (!file.eof())
	{
		char record_id[8];
		for (i32s n1 = 0;n1 < 6;n1++)
		{
			char tchar = (char) file.get();
			if (tchar != ' ') record_id[n1] = tchar;
			else record_id[n1] = 0;
		}
		
		record_id[6] = 0;	// terminate the record string...
		
		if (!strcmp("ATOM", record_id))
		{
			i32s serial_number; file >> serial_number;
			
			for (i32s n1 = 0;n1 < 18;n1++) buffer[n1] = (char) file.get();
			buffer[5] = ' '; buffer[15] = ' ';
			
			char chn_id; i32s res_num; char res_name[5]; char atm_name[5]; fGL crd[3];
			
			istrstream str(buffer, sizeof(buffer));
			str >> atm_name >> res_name; str.get(); chn_id = (char) str.get(); str >> res_num;
			file >> crd[0]; crd[0] /= 10.0; file >> crd[1]; crd[1] /= 10.0; file >> crd[2]; crd[2] /= 10.0;
			
			if (chn_id == ' ') chn_id = '_';	// fix the empty chain id...
			
			bool test1 = (chn_num == NOT_DEFINED);
			bool test2 = test1 ? false : (chn_id == mdata->chn_vector[chn_num]->chn_id);
			
			if (test1 || test2)
			{
				if (model_counter == 0)
				{
					readpdb_data_atom new_data;
					new_data.ref = NULL;
					
					strcpy(new_data.atm_name, atm_name);
					strcpy(new_data.res_name, res_name);
					
					new_data.chn_id = chn_id;
					new_data.res_num = res_num;
					
					new_data.crd[model_counter][0] = crd[0];
					new_data.crd[model_counter][1] = crd[1];
					new_data.crd[model_counter][2] = crd[2];
					
					atom_data.push_back(new_data);
				}
				else
				{
					bool test1 = !strcmp(atom_data[atom_counter].res_name, res_name);
					bool test2 = !strcmp(atom_data[atom_counter].atm_name, atm_name);
					
					if (!test1) { cout << "BUG: test1 failed!!!" << endl; exit(EXIT_FAILURE); }
					if (!test2) { cout << "BUG: test2 failed!!!" << endl; exit(EXIT_FAILURE); }
					
					atom_data[atom_counter].crd[model_counter][0] = crd[0];
					atom_data[atom_counter].crd[model_counter][1] = crd[1];
					atom_data[atom_counter].crd[model_counter][2] = crd[2];
					
					atom_counter++;
				}
			}
		}
		
		if (!strcmp("SSBOND", record_id))
		{
			i32s serial_number; file >> serial_number;
			readpdb_data_ssbond new_data; new_data.ref = NULL;
			
			for (i32s n1 = 0;n1 < 2;n1++)
			{
				file >> buffer; file.get();
				char chn_id = (char) file.get();
				
				if (chn_id == ' ') chn_id = '_';	// fix the empty chain id...
				
				new_data.chn_id = chn_id;
				
				for (i32s n2 = 0;n2 < 6;n2++) buffer[n2] = (char) file.get();
				istrstream str(buffer, sizeof(buffer));
				str >> new_data.res_num;
				
				ssbond_data.push_back(new_data);
			}
		}
		
		// how to deal with multiple coordinate sets (often present in NMR entries)???
		// how to deal with multiple coordinate sets (often present in NMR entries)???
		// how to deal with multiple coordinate sets (often present in NMR entries)???
		
#ifdef READPDB_ENABLE_MULTIPLE_CRDSETS

		// in this caseat the moment, we will read multiple records up to READPDB_MAX_CRDSETS.
		// if READPDB_MAX_CRDSETS is exceeded, the program stops; must recompile then...
		
		if (!strcmp("MODEL", record_id))
		{
			i32s model_number; file >> model_number;
			
			if (model_number > 1)
			{
				model_counter++;
				
				if (model_counter == READPDB_MAX_CRDSETS)	// this should never happen...
				{
					cout << "BUG: READPDB_MAX_CRDSETS exceeded!!!" << endl;
					exit(EXIT_FAILURE);
				}
				
				atom_counter = 0;
			}
		}
		
#else	// READPDB_ENABLE_MULTIPLE_CRDSETS

		// in this case, we just look for "ENDMDL" record, which says this coordinate set is
		// now ready. so, we read only the first coordinate set. the "MODEL"/"ENDMDL" records
		// are only after "SSBOND", so it should not have big effects...
		
		if (!strcmp("ENDMDL", record_id))
		{
			cout << "ENDMDL record found, skipping the rest of this file..." << endl;
			break;
		}
		
#endif	// READPDB_ENABLE_MULTIPLE_CRDSETS

		file.getline(buffer, sizeof(buffer));		// move to the next line...
	}
	
	file.close();
	
	if (atom_data.empty())
	{
		cout << "no atoms found !!!" << endl;
		return;
	}
		
	// we have now read all the crd-sets; check if we need more space for them...
	
	i32s csets = cs_vector.size(); i32s new_csets = ((model_counter + 1) - csets);
	cout << "there were " << csets << " old crd-sets, creating " << new_csets << " new..." << endl;
	
	PushCRDSets(new_csets);
	
	for (i32u n1 = 0;n1 < cs_vector.size();n1++) SetCRDSetVisible(n1, true);
	
	// count the chains and relate them to mdata, create new records to mdata if necessary...
	
	vector<i32s> chn_index;
	char tmp_chn_id; i32u tmp_index;
	
	tmp_chn_id = atom_data.front().chn_id;
	
	tmp_index = 0;
	while (tmp_index < mdata->chn_vector.size())
	{
		if (tmp_chn_id == mdata->chn_vector[tmp_index]->chn_id) break;
		else tmp_index++;
	}
	
	if (tmp_index == mdata->chn_vector.size())
	{
		readpdb_mdata_chain * unknown = new readpdb_mdata_chain;
		unknown->chn_id = tmp_chn_id; unknown->seqres = NULL;
		mdata->chn_vector.push_back(unknown);
	}
	
	chn_index.push_back(tmp_index);

	{
	
	for (i32u n1 = 1;n1 < atom_data.size();n1++)
	{
		tmp_chn_id = atom_data[n1].chn_id;
		if (tmp_chn_id == mdata->chn_vector[chn_index.back()]->chn_id) continue;
		else
		{
			tmp_index = 0;
			while (tmp_index < mdata->chn_vector.size())
			{
				if (tmp_chn_id == mdata->chn_vector[tmp_index]->chn_id) break;
				else tmp_index++;
			}
			
			if (tmp_index == mdata->chn_vector.size())
			{
				readpdb_mdata_chain * unknown = new readpdb_mdata_chain;
				unknown->chn_id = tmp_chn_id; unknown->seqres = NULL;
				mdata->chn_vector.push_back(unknown);
			}
			
			chn_index.push_back(tmp_index);
		}
	}
	}
	
	// now create the chains, checking validity of the residues...
	{
	for (i32u n1 = 0;n1 < chn_index.size();n1++)
	{
		char current_chn_id = mdata->chn_vector[chn_index[n1]]->chn_id;
		
		i32s current_chn_length;
		if (mdata->chn_vector[chn_index[n1]]->seqres != NULL)
		{
			current_chn_length = strlen(mdata->chn_vector[chn_index[n1]]->seqres);
		}
		else
		{
			current_chn_length = NOT_DEFINED;
		}
		
		i32s previous_residue = 0;
		
		i32s range1[2];
		
		range1[0] = 0;
		while (atom_data[range1[0]].chn_id != current_chn_id) range1[0]++;
		
		range1[1] = range1[0];
		while (range1[1] < (i32s) atom_data.size() && atom_data[range1[1]].chn_id == current_chn_id) range1[1]++;
		
		vector<i32s> res_data;
		
		i32s range2[2];
		range2[0] = range1[0];
		while (range2[0] < range1[1])
		{
			i32s residue = atom_data[range2[0]].res_num;
			const char * res_name = atom_data[range2[0]].res_name;
			
			range2[1] = range2[0];
			while (range2[1] < (i32s) atom_data.size() && atom_data[range2[1]].res_num == residue) range2[1]++;
			
			bool standard = false;
			if (!strcmp("ALA", res_name)) standard = true;
			if (!strcmp("ARG", res_name)) standard = true;
			if (!strcmp("ASN", res_name)) standard = true;
			if (!strcmp("ASP", res_name)) standard = true;
			if (!strcmp("CYS", res_name)) standard = true;
			if (!strcmp("GLN", res_name)) standard = true;
			if (!strcmp("GLU", res_name)) standard = true;
			if (!strcmp("GLY", res_name)) standard = true;
			if (!strcmp("HIS", res_name)) standard = true;
			if (!strcmp("ILE", res_name)) standard = true;
			if (!strcmp("LEU", res_name)) standard = true;
			if (!strcmp("LYS", res_name)) standard = true;
			if (!strcmp("MET", res_name)) standard = true;
			if (!strcmp("PHE", res_name)) standard = true;
			if (!strcmp("PRO", res_name)) standard = true;
			if (!strcmp("SER", res_name)) standard = true;
			if (!strcmp("THR", res_name)) standard = true;
			if (!strcmp("TRP", res_name)) standard = true;
			if (!strcmp("TYR", res_name)) standard = true;
			if (!strcmp("VAL", res_name)) standard = true;
			
			bool skip = false;
			if (standard)
			{
				if (readpdb_ReadData_sub1(atom_data, range2, "N", true) == NOT_DEFINED) skip = true;
				if (readpdb_ReadData_sub1(atom_data, range2, "CA", true) == NOT_DEFINED) skip = true;
				if (readpdb_ReadData_sub1(atom_data, range2, "C", true) == NOT_DEFINED) skip = true;
				if (readpdb_ReadData_sub1(atom_data, range2, "O", true) == NOT_DEFINED) skip = true;
			}
			else
			{
				cout << "could not recognize this residue: " << res_name << endl;
				skip = true;
			}
			
			if (skip)
			{
				cout << "skipping broken residue " << residue << " " << res_name << endl;
				atom_data.erase(atom_data.begin() + range2[0], atom_data.begin() + range2[1]);
				range1[1] += (range2[0] - range2[1]);
			}
			else
			{
				// ok, there seems to be no big problems with this residue.
				// first we make some bookkeeping with the original sequence...
				
				if ((previous_residue + 1) != residue)
				{
					for (i32s n2 = (previous_residue + 1);n2 < residue;n2++)
					{
						// the variables use "pascal/fortran-style" counting, beginning 1,
						// but we will store the values in "c-style" way, beginning 0!!!
						
						mdata->chn_vector[chn_index[n1]]->missing_residues.push_back(n2 - 1);
					}
				}
				
				previous_residue = residue;
				bool alpha_carbon_is_found = false;
				
				// ...and then we just create the atoms.
				
				res_data.push_back(range2[0]);
				for (i32s n2 = range2[0];n2 < range2[1];n2++)
				{
					strcpy(buffer, atom_data[n2].atm_name); buffer[1] = 0;		// here we suppose
					element el = element(buffer);					// single-char elements!!!
					
					if (el.GetAtomicNumber() == 1) continue;
					atom newatom(el, NULL, cs_vector.size());
					
					for (i32u n3 = 0;n3 < cs_vector.size();n3++)
					{
						fGL x = atom_data[n2].crd[n3][0];
						fGL y = atom_data[n2].crd[n3][1];
						fGL z = atom_data[n2].crd[n3][2];
						newatom.SetCRD(n3, x, y, z);
					}
					
					Add_Atom(newatom); atom_data[n2].ref = (& atom_list.back());
					
					// if this atom was an alpha-carbon, then add it to the list... but alternative
					// locations might cause trouble here; make sure that only one c-alpha is added!!!
					
					if (!strcmp(atom_data[n2].atm_name, "CA") && !alpha_carbon_is_found)
					{
						mdata->chn_vector[chn_index[n1]]->alpha_carbons.push_back(atom_data[n2].ref);
						alpha_carbon_is_found = true;
					}
				}
				
				readpdb_ReadData_sub2(atom_data, range2, "N", "CA", 'S');
				readpdb_ReadData_sub2(atom_data, range2, "CA", "C", 'S');
				readpdb_ReadData_sub2(atom_data, range2, "C", "O", 'D');
				
				if (!strcmp("ALA", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
				}
				
				if (!strcmp("ARG", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "CD", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CD", "NE", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "NE", "CZ", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CZ", "NH1", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CZ", "NH2", 'C');
				}
				
				if (!strcmp("ASN", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "OD1", 'D');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "ND2", 'S');
				}
				
				if (!strcmp("ASP", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "OD1", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "OD2", 'C');
				}
				
				if (!strcmp("CYS", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "SG", 'S');
				}
				
				if (!strcmp("GLN", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "CD", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CD", "OE1", 'D');
					readpdb_ReadData_sub2(atom_data, range2, "CD", "NE2", 'S');
				}
				
				if (!strcmp("GLU", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "CD", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CD", "OE1", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CD", "OE2", 'C');
				}
				
				// GLY -> we don't have to do anything in this case...
				// GLY -> we don't have to do anything in this case...
				// GLY -> we don't have to do anything in this case...
				
				if (!strcmp("HIS", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "ND1", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "CD2", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "ND1", "CE1", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CD2", "NE2", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CE1", "NE2", 'C');
				}
				
				if (!strcmp("ILE", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG1", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG2", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG1", "CD1", 'S');
				}
				
				if (!strcmp("LEU", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "CD1", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "CD2", 'S');
				}
				
				if (!strcmp("LYS", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "CD", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CD", "CE", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CE", "NZ", 'S');
				}
				
				if (!strcmp("MET", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "SD", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "SD", "CE", 'S');
				}
				
				if (!strcmp("PHE", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "CD1", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "CD2", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CD1", "CE1", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CD2", "CE2", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CE1", "CZ", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CE2", "CZ", 'C');
				}
				
				if (!strcmp("PRO", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "CD", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CD", "N", 'S');
				}
				
				if (!strcmp("SER", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "OG", 'S');
				}
				
				if (!strcmp("THR", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "OG1", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG2", 'S');
				}
				
				if (!strcmp("TRP", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "CD1", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "CD2", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CD1", "NE1", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CD2", "CE2", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CD2", "CE3", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "NE1", "CE2", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CE2", "CZ2", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CE3", "CZ3", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CZ2", "CH2", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CZ3", "CH2", 'C');
				}
				
				if (!strcmp("TYR", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "CD1", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CG", "CD2", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CD1", "CE1", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CD2", "CE2", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CE1", "CZ", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CE2", "CZ", 'C');
					readpdb_ReadData_sub2(atom_data, range2, "CZ", "OH", 'S');
				}
				
				if (!strcmp("VAL", res_name))
				{
					readpdb_ReadData_sub2(atom_data, range2, "CA", "CB", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG1", 'S');
					readpdb_ReadData_sub2(atom_data, range2, "CB", "CG2", 'S');
				}
				
				range2[0] = range2[1];
			}
		}
		
		// the end of sequence bookkeeping : check if there were missing n-terminal residues.
		
		if (previous_residue < current_chn_length)
		{
			for (i32s n2 = previous_residue;n2 < current_chn_length;n2++)
			{
				mdata->chn_vector[chn_index[n1]]->missing_residues.push_back(n2);
			}
		}
		
		if (!mdata->chn_vector[chn_index[n1]]->missing_residues.empty())
		{
			char out1 = mdata->chn_vector[chn_index[n1]]->chn_id;
			i32u out2 = mdata->chn_vector[chn_index[n1]]->missing_residues.size();
			
			cout << "at chain '" << out1 << "' there were " << out2 << " missing residues:" << endl;
			
			for (i32u n2 = 0;n2 < mdata->chn_vector[chn_index[n1]]->missing_residues.size();n2++)
			{
				cout << mdata->chn_vector[chn_index[n1]]->missing_residues[n2] << " ";
			}
			
			cout << endl;
		}
		
		// add connecting bonds between residues...
		
		res_data.push_back(range1[1]);
		for (i32s n2 = 0;n2 < ((i32s) res_data.size()) - 2;n2++)
		{
			i32s r1[2] = { res_data[n2],  res_data[n2 + 1] };
			i32s r2[2] = { res_data[n2 + 1],  res_data[n2 + 2] };
			
			i32s ind1 = readpdb_ReadData_sub1(atom_data, r1, "C", false);
			i32s ind2 = readpdb_ReadData_sub1(atom_data, r2, "N", false);
			
			if (ind1 == NOT_DEFINED || ind2 == NOT_DEFINED)
			{
				cout << "BUG: could not connect ..." << endl;
				exit(EXIT_FAILURE);
			}
			else
			{
				bondtype bt = bondtype('S');
				bond newbond(atom_data[ind1].ref, atom_data[ind2].ref, bt);
				AddBond(newbond);
			}
		}
		
		// check the c-terminal residue...
		
		if (res_data.size() < 2) continue;
		i32s last[2] = { res_data[res_data.size() - 2], res_data[res_data.size() - 1] };
		
		i32s ind1 = readpdb_ReadData_sub1(atom_data, last, "C", false);
		i32s ind2 = readpdb_ReadData_sub1(atom_data, last, "O", false);
		
		if (ind1 == NOT_DEFINED || ind2 == NOT_DEFINED)
		{
			cout << "BUG: could not find c-term group..." << endl;
			exit(EXIT_FAILURE);
		}
		else
		{
			bondtype bt = bondtype('C');
			bond tmpbond(atom_data[ind1].ref, atom_data[ind2].ref, bt);
			iter_bl it1 = find(bond_list.begin(), bond_list.end(), tmpbond);
			if (it1 != bond_list.end()) (* it1).bt = bt;
			
			ind2 = readpdb_ReadData_sub1(atom_data, last, "OXT", false);
			if (ind2 != NOT_DEFINED)
			{
				bond newbond(atom_data[ind1].ref, atom_data[ind2].ref, bt);
				AddBond(newbond);
			}
			else
			{
				cout << "missing terminal oxygen..." << endl;
				
				i32s ind[3];
				ind[0] = readpdb_ReadData_sub1(atom_data, last, "CA", false);
				ind[1] = readpdb_ReadData_sub1(atom_data, last, "C", false);
				ind[2] = readpdb_ReadData_sub1(atom_data, last, "O", false);
				
				const fGL * ref1; const fGL * ref2;
				
				ref1 = atom_data[ind[1]].ref->GetCRD(0);
				ref2 = atom_data[ind[0]].ref->GetCRD(0);
				v3d<fGL> v1 = v3d<fGL>(ref1, ref2);
				
				ref1 = atom_data[ind[1]].ref->GetCRD(0);
				ref2 = atom_data[ind[2]].ref->GetCRD(0);
				v3d<fGL> v2 = v3d<fGL>(ref1, ref2);
				
				fGL tmp2 = v1.len(); fGL tmp3 = v1.spr(v2) / (tmp2 * tmp2);
				v1 = v1 * tmp3; v2 = v2 - v1;
				
				fGL tmp4[3];
				const fGL * cdata = atom_data[ind[1]].ref->GetCRD(0);
				for (i32s n1 = 0;n1 < 3;n1++)
				{
					tmp4[n1] = cdata[n1];
					tmp4[n1] += v1.data[n1] - v2.data[n1];
				}
				
				element el = element(8);
				atom newatom(el, tmp4, cs_vector.size());
				Add_Atom(newatom);
				
				bond newbond(atom_data[ind1].ref, (& atom_list.back()), bt);
				AddBond(newbond);
			}
		}
	}
	}
	
	// add disulphide bonds...
	{
	
	for (i32u n1 = 0;n1 < ssbond_data.size();n1++)
	{
		i32u counter = 0;
		while (counter < atom_data.size())
		{
			bool test1 = (ssbond_data[n1].chn_id == atom_data[counter].chn_id);
			bool test2 = (ssbond_data[n1].res_num == atom_data[counter].res_num);
			bool test3 = !strcmp(atom_data[counter].atm_name, "SG");
			if (test1 && test2 && test3) break; else counter++;
		}
		
		if (counter == atom_data.size()) continue;
		ssbond_data[n1].ref = atom_data[counter].ref;
	}
	}
	{
	for (i32u n1 = 0;n1 < ssbond_data.size();n1 += 2)
	{
		if (ssbond_data[n1 + 0].ref == NULL || ssbond_data[n1 + 1].ref == NULL)
		{
			cout << "could not create bridge ";
			cout << ssbond_data[n1 + 0].chn_id << " " << ssbond_data[n1 + 0].res_num << " -> ";
			cout << ssbond_data[n1 + 1].chn_id << " " << ssbond_data[n1 + 1].res_num << endl;
		}
		else
		{
			bondtype bt = bondtype('S');
			bond newbond(ssbond_data[n1 + 0].ref, ssbond_data[n1 + 1].ref, bt);
			AddBond(newbond);
		}
	}
	}
	
	// ready...
	
	cout << "done." << endl;
	
	CenterCRDSet(0, true);
}

i32s model::readpdb_ReadData_sub1(vector<readpdb_data_atom> & adata, i32s * range, const char * atom_name, bool flag)
{
	for (i32s n1 = range[0];n1 < range[1];n1++)
	{
		if (!strcmp(adata[n1].atm_name, atom_name)) return n1;
	}
	
	cout << "atom " << atom_name << " is missing..." << endl;
	return NOT_DEFINED;
}

void model::readpdb_ReadData_sub2(vector<readpdb_data_atom> & adata, i32s * range, const char * at1, const char * at2, char btype)
{
	i32s ind1 = readpdb_ReadData_sub1(adata, range, at1, false);
	i32s ind2 = readpdb_ReadData_sub1(adata, range, at2, false);
	
	if (ind1 == NOT_DEFINED || ind2 == NOT_DEFINED) return;
	
	bondtype bt = bondtype(btype);
	bond newbond(adata[ind1].ref, adata[ind2].ref, bt);
	AddBond(newbond);
}

// the egrp stuff is here...
// ^^^^^^^^^^^^^^^^^^^^^^^^^

void model::ecomp_AddGroupU(const char * gn)
{
	const int new_grp_i = (int) ecomp_grp_name_usr.size();
	const char * copy_of_grp_name = new char[strlen(gn) + 1];
	ecomp_grp_name_usr.push_back(copy_of_grp_name);
	
	// set the currently selected atoms in this new group!!!
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	int atom_counter = 0;
	for (iter_al itA = GetAtomsBegin();itA != GetAtomsEnd();itA++)
	{
		if ((* itA).ecomp_grp_i >= new_grp_i)
		{
			cout << "invalid atom::ecomp_grp_i found in model::ecomp_AddGroupU() ; " << (* itA).ecomp_grp_i << endl;
			exit(EXIT_FAILURE);
		}
		
		if ((* itA).flags & ATOMFLAG_SELECTED)
		{
			(* itA).ecomp_grp_i = new_grp_i;
			atom_counter++;
		}
	}
	
	char mbuff1[1024];
	ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << "Added " << atom_counter << " atoms in a new ecomp_grp " << gn << endl << ends;
	PrintToLog(mbuff1);
}

/*bool model::ecomp_DeleteGroupU(int)
{
}

void model::ecomp_UnRegisterAll(void)
{
}

void model::ecomp_RegisterNoAuto(void)
{
}

void model::ecomp_RegisterWithAutoSolv(void)
{
}

void model::ecomp_RegisterWithAutoSolvChn1(void)
{
}

void model::ecomp_RegisterWithAutoSolvChn2(void)
{
}	*/

void model::ecomp_Register(void)
{
	// onko taulukot NULL ? ellei niin VIRHE
	// laske montako ryhmдд/vдliryhmдд tarttee
	// kopioi/luo ryhmien nimet
	// kopioi/luo muut taulukot
}
#if DIFFUSE_WORKING
void model::work_diffuse(char *infile_name)
{
	this->ClearModel();
	
	cout << "trying to open a file " << infile_name << " ; ";
	ifstream ifile;
	ifile.open(infile_name, ios::in);
	
	if (ifile.fail())
	{
		cout << "failed!!!" << endl;
		
		ifile.close();
		return;
	}

	cout << "ok!!!" << endl;

	ReadGPR(*this, ifile, false, false);
	ifile.close();

	{
		const bool updt = true;
		setup * su = this->GetCurrentSetup();

printf("periodic_box_HALFdim[0] = %f\n", this->periodic_box_HALFdim[0]);
printf("periodic_box_HALFdim[1] = %f\n", this->periodic_box_HALFdim[1]);
printf("periodic_box_HALFdim[2] = %f\n", this->periodic_box_HALFdim[2]);

		this->periodic_box_HALFdim[2] = 10.0;

		geomopt_param go = geomopt_param(su);
		this->DoGeomOpt(go, updt);

printf("periodic_box_HALFdim[0] = %f\n", this->periodic_box_HALFdim[0]);
printf("periodic_box_HALFdim[1] = %f\n", this->periodic_box_HALFdim[1]);
printf("periodic_box_HALFdim[2] = %f\n", this->periodic_box_HALFdim[2]);

		fGL xdim = this->periodic_box_HALFdim[0];
		fGL ydim = this->periodic_box_HALFdim[1];
		fGL zdim = this->periodic_box_HALFdim[2];

		//fGL density = 1.00;;
		fGL density = 0.5;;

		int element_number = 36;
		model * solvent = NULL;
		char * export_fn = NULL;
		SolvateBox(xdim, ydim, zdim, density, element_number, solvent, export_fn);


		moldyn_param md = moldyn_param(su);

		this->DoMolDyn(md,updt);
	}
}


#endif/*DIFFUSE_WORKING*/
#if SNARJAD_TARGET_WORKING
void model::working(double vel, char *infile_name, char * trgtlst_name)
{
	this->ClearModel();
	
	cout << "trying to open a file " << infile_name << " ; ";
	ifstream ifile;
	ifile.open(infile_name, ios::in);
	
	if (ifile.fail())
	{
		cout << "failed!!!" << endl;
		
		ifile.close();
		return;
	}

	cout << "ok!!!" << endl;

	ReadGPR(*this, ifile, false, false);
	ifile.close();

	{
		const bool updt = true;
		setup * su = this->GetCurrentSetup();

printf("periodic_box_HALFdim[0] = %f\n", this->periodic_box_HALFdim[0]);
printf("periodic_box_HALFdim[1] = %f\n", this->periodic_box_HALFdim[1]);
printf("periodic_box_HALFdim[2] = %f\n", this->periodic_box_HALFdim[2]);

		geomopt_param go = geomopt_param(su);
		this->DoGeomOpt(go, updt);

printf("periodic_box_HALFdim[0] = %f\n", this->periodic_box_HALFdim[0]);
printf("periodic_box_HALFdim[1] = %f\n", this->periodic_box_HALFdim[1]);
printf("periodic_box_HALFdim[2] = %f\n", this->periodic_box_HALFdim[2]);

		moldyn_param md = moldyn_param(su);
		ReadTargetListFile(trgtlst_name, md.target_list);
		for(int i = 0; i < md.target_list.size(); i++)
		{
			cout << "md.target_list[" << i << "] = " << md.target_list[i] << endl;
		}
		md.start_snarjad_vel[0] = 0.0;
		md.start_snarjad_vel[1] = 0.0;
		md.start_snarjad_vel[2] = vel;

		//strcpy(md.filename, infile_name);
		this->DoMolDyn(md,updt);
	}
}
void model::work(double start, double step, double fin, char *infile_name, char * trgtlst_name)
{
	char filename[] = "n_snarjad_through_membrana.txt";
	FILE * stream;
	stream  = fopen(filename,"wt");
	fclose(stream);
	for (double vel = start; vel <= fin; vel+= step)
	{
		i32s 
			n_snarjad_through_membrana_plus,
			n_snarjad_through_membrana_minus;
		//--------------------------------------------
		b_snarjad_through_membrana_started = false;
		n_snarjad_through_membrana = 0;
		this->working(vel, infile_name, trgtlst_name);
		n_snarjad_through_membrana_plus = n_snarjad_through_membrana;
		//--------------------------------------------
		b_snarjad_through_membrana_started = false;
		n_snarjad_through_membrana = 0;
		this->working(-vel, infile_name, trgtlst_name);
		n_snarjad_through_membrana_minus = n_snarjad_through_membrana;
		//--------------------------------------------
	stream  = fopen(filename,"at");
	fprintf(stream, "%f,%d,%d\n", vel, n_snarjad_through_membrana_minus, n_snarjad_through_membrana_plus);
	fclose(stream);
	}
}
#endif /*SNARJAD_TARGET_WORKING*/

#if PROBNIY_ATOM_WORKING

void model::work_prob_atom(char *infile_name, char * trgtlst_name)//my experiment
{
	this->prob_atom_move_direction = true;
	this->working_prob_atom(infile_name, trgtlst_name);
	this->prob_atom_move_direction = false;
	this->working_prob_atom(infile_name, trgtlst_name);

}
void model::working_prob_atom(char *infile_name, char * trgtlst_name)
{
	this->ClearModel();
	
	cout << "trying to open a file " << infile_name << " ; ";
	ifstream ifile;
	ifile.open(infile_name, ios::in);
	
	if (ifile.fail())
	{
		cout << "failed!!!" << endl;
		
		ifile.close();
		return;
	}

	cout << "ok!!!" << endl;

	ReadGPR(*this, ifile, false, false);
	ifile.close();

	{
		const bool updt = true;
		setup * su = this->GetCurrentSetup();

printf("periodic_box_HALFdim[0] = %f\n", this->periodic_box_HALFdim[0]);
printf("periodic_box_HALFdim[1] = %f\n", this->periodic_box_HALFdim[1]);
printf("periodic_box_HALFdim[2] = %f\n", this->periodic_box_HALFdim[2]);

		geomopt_param go = geomopt_param(su);
		this->DoGeomOpt(go, updt);

printf("periodic_box_HALFdim[0] = %f\n", this->periodic_box_HALFdim[0]);
printf("periodic_box_HALFdim[1] = %f\n", this->periodic_box_HALFdim[1]);
printf("periodic_box_HALFdim[2] = %f\n", this->periodic_box_HALFdim[2]);

		moldyn_param md = moldyn_param(su);
		ReadTargetListFile(trgtlst_name, md.target_list);
		for(int i = 0; i < md.target_list.size(); i++)
		{
			cout << "md.target_list[" << i << "] = " << md.target_list[i] << endl;
		}
/*
		md.start_snarjad_vel[0] = 0.0;
		md.start_snarjad_vel[1] = 0.0;
		md.start_snarjad_vel[2] = vel;
*/
		//strcpy(md.filename, infile_name);
		this->DoMolDyn(md,updt);
	}
}
#endif /*PROBNIY_ATOM_WORKING*/ 

bool model::Read_GPR(char *infile_name)
{
	//открываем файл с молекулами	
	cout << "trying to open a file " << infile_name << " ; ";
	ifstream ifile;
	ifile.open(infile_name, ios::in);	
	if (ifile.fail())
	{
		cout << "failed!!!" << endl;
		
		ifile.close();
		return false;
	}
	cout << "ok!!!" << endl;
	//загружаем молекулы в модель
	bool read = ReadGPR(*this, ifile, false, false);
	// и закрываем файл
	ifile.close();
	return read;
}

#if PROBNIY_ATOM_GEOMOPT
void model::work_prob_atom_GeomOpt(geomopt_param & param, char *infile_name, char * trgtlst_name, char * box_name, char * fixed_name, int total_frames)//my experiment
{
	param.Write("geomopt_param.txt");

	this->prob_atom_move_direction = true;
	this->working_prob_atom_GeomOpt(param, infile_name, trgtlst_name, box_name, fixed_name, total_frames);
	this->prob_atom_move_direction = false;
	this->working_prob_atom_GeomOpt(param, infile_name, trgtlst_name, box_name, fixed_name, total_frames);

}
void model::working_prob_atom_GeomOpt(geomopt_param & param, char *infile_name, char * trgtlst_name, char * box_name, char * fixed_name, int total_frames)//my experiment
{
	//очищаем модель
	this->ClearModel();
	//открываем файл с молекулой мембраны и пробным атомом	
	if (!this->Read_GPR(infile_name))
	{
		return;
	}

	//
	// производим оптимизацию геометрии молекулы по общепринятому алгоритму
	// здесь же происходит оптимизация граничных условий
	const bool updt = true;

	printf("periodic_box_HALFdim[0] = %f\n", this->periodic_box_HALFdim[0]);
	printf("periodic_box_HALFdim[1] = %f\n", this->periodic_box_HALFdim[1]);
	printf("periodic_box_HALFdim[2] = %f\n", this->periodic_box_HALFdim[2]);

#if 0
	// отменяем оптимизацию геометрии, потому что имеем в виду, 
	// что мы имеем на входе уже молекулу с оптимизированной геометрией
	// так же потому что оптимизация здесь нужна была для того лишь, чтобы
	// оптимизировать размеры периодической ячейки
	// Лучше вместо этого загружать готовые размеры периодической ячейки из файла,
	// которые получены ранее при предварительной оптимизации геометрии
	this->DoGeomOpt(param, updt);
#else
	//загрузка размеров периодической ячейки
	this->LoadBox(box_name);
#endif

	printf("periodic_box_HALFdim[0] = %f\n", this->periodic_box_HALFdim[0]);
	printf("periodic_box_HALFdim[1] = %f\n", this->periodic_box_HALFdim[1]);
	printf("periodic_box_HALFdim[2] = %f\n", this->periodic_box_HALFdim[2]);

	// теперь читаем из файла номера целевых атомов
	vector<i32s> target_list;
	ReadTargetListFile(trgtlst_name, target_list);
	size_t num_target = target_list.size();
	for(int i = 0; i < num_target; i++)
	{
		cout << "target_list[" << i << "] = " << target_list[i] << endl;
	}

	/////////////////////////////////////////////////////
	/////////////////////////////////////////////////////
	/////////////////////////////////////////////////////
	// открываем файл для записи траэктории
	char outfilename[1024];
	sprintf(outfilename, "prob_at_mv_dir_%d_geomopt.traj", prob_atom_move_direction ? +1 : -1);
	printf("outfilename = %s\n", outfilename);
	/////////////////////////////////////////////////////
	ofstream ofile;
	ofile.open(outfilename, ios_base::out | ios_base::trunc | ios_base::binary);
	//записываем заголовок файла траэктории
	const int number_of_atoms = GetAtomCount();
	const char file_id[10] = "traj_v10";
	
	

	ofile.write((char *) file_id, 8);					// file id, 8 chars.
	ofile.write((char *) & number_of_atoms, sizeof(number_of_atoms));	// number of atoms, int.
	ofile.write((char *) & total_frames, sizeof(total_frames));		// total number of frames, int.
	/////////////////////////////////////////////////////
	/////////////////////////////////////////////////////
	/////////////////////////////////////////////////////
	//#####################################################################
	// готовим объект вычислительной машины
	engine * eng = GetCurrentSetup()->CreateEngineByIDNumber(ENG1_MM_PERIODIC);
	//#####################################################################
	if (eng == NULL)
	{
		return;
	}

	char mbuff1[256];
	ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << "Starting Geometry Optimization ";
	str1 << "(setup = " << GetCurrentSetup()->GetClassName();
	str1 << ", engine = " << GetCurrentSetup()->GetEngineName(GetCurrentSetup()->GetCurrEngIndex());
	str1 << ")." << endl << ends;
	PrintToLog(mbuff1);

	// копируем координаты атомов из модели в вычислительную машину	
	CopyCRD(this, eng, 0);


	f64 energy00 = 0.0;

	// пробный атом - получаем его номер
	i32s n_prob_atom = eng->GetAtomCount() - 1;

	// добавляем номер пробного атома к списку номеров атомов с фиксированной координатой
	vector<i32s> missed_atoms_list;
	ReadTargetListFile(fixed_name, missed_atoms_list);
	missed_atoms_list.push_back(n_prob_atom);

	size_t num_missed = missed_atoms_list.size();
	for(int i = 0; i < num_missed; i++)
	{
		cout << "missed_atoms_list[" << i << "] = " << missed_atoms_list[i] << endl;
	}

#if PROBNIY_ATOM_GEOMOPT_TRADITIONAL
	geomopt * opt = new geomopt(eng, 100, 0.025, 10.0);		// optimal settings?!?!?
#else
	// расширенный класс оптимизации геометрии позволяющий 
	// при оптимизации фиксировать например, z координату 
	// определённых атомов - путём невключения этих координат 
	// в процедуру оптимизации. Этот класс может быть использован, 
	// например, для решения задачи поиска потенциального барьера 
	// проникновения пробного атома через мембрану. У пробного атома 
	// и у нескольких атомов мембраны фиксируется z координата, 
	// производится оптимизация геометрии с расчётом потенциальной 
	// энергии системы, затем в цикле изменяется z координата 
	// пробного атома, и производится перерасчёт оптимизации 
	// геометрии и потенциальной энергии. В итоге строится 
	// зависимомть потенциальной энергии системы 
	// от z координаты пробного атома

	// создаём объект оптимизатора геометрии с использованием списка 
	// фиксируемых атомов по координате 2 (z)
	geomopt_ex * opt = new geomopt_ex(missed_atoms_list, 2, eng, 100, 0.025, 10.0);		// optimal settings?!?!?
#endif /*PROBNIY_ATOM_GEOMOPT_TRADITIONAL*/
	// создаём объект оптимизатора граничных условий натяжения мембраны
#if USE_BOUNDARY_OPT_ON_PROBNIY_ATOM_GEOMOPT
	boundary_opt * b_opt = new boundary_opt(this, eng, 100, 0.025, 10.0);
#endif
	// открываем лог файл
	char datfilename[1024];
	sprintf(datfilename, "prob_at_mv_dir_%d_geomopt.dat", prob_atom_move_direction ? +1 : -1);
	printf("datfilename = %s\n", datfilename);

	FILE * dat = fopen(datfilename, "wt");



	// открываем лог файл
	char logfilename[1024];
	sprintf(logfilename, "prob_at_mv_dir_%d_geomopt.log", prob_atom_move_direction ? +1 : -1);
	printf("logfilename = %s\n", logfilename);
	//////////////////////////////////////////
	ofstream logfile2;
	logfile2.open(logfilename, ios::out);
	// пишем заголовок лог файла
	// №№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№
	ostrstream str2b(mbuff1, sizeof(mbuff1));
	str2b << "step,";
	str2b << "rz," << "Epot," << "Epot00,";

	//str2b << "prob_atom_crd[0],";
	//str2b << "prob_atom_crd[1],";
	str2b << "prob_atom_crd[2],";

	//str2b << "prob_atom_f[0],";
	//str2b << "prob_atom_f[1],";
	str2b << "prob_atom_f[2],";

	//str2b << "target_crd[0],";
	//str2b << "target_crd[1],";
	//str2b << "target_crd[2]";
	str2b << "n_steps_of_optimization";

/*
#if USE_BOUNDARY_OPT_ON_PROBNIY_ATOM_GEOMOPT
	str2b << ",boundary[0]";
	str2b << ",boundary[1]";
	str2b << ",boundary[2]";
#endif
*/

	eng1_mm * engmm = dynamic_cast<eng1_mm *>(eng);
	if (engmm)
	{
		str2b << ",BondedTermsEnergy";
		str2b << ",NonBondedTermsEnergy";

		str2b << ",BondStretchEnergy";
		str2b << ",AngleStretchEnergy"; 
		str2b << ",TorsionEnergy";
		str2b << ",OutOfPlaneEnergy"; 

		str2b << ",DispersionEnergy";
		str2b << ",ElectrostaticEnergy"; 
		//str2b << ",NonBondedEnergyC"; 
		//str2b << ",NonBondedEnergyD";
	}



	str2b << endl << ends;
	logfile2 << mbuff1;
	// №№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№
	f64 target_crd[3]; // координата центра отверстия в мембране
	f64 prob_atom_crd[3];
	for (int iframe = 0; iframe < total_frames; iframe++)
	{			
		// расчитываем координаты цели
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			target_crd[n2] = 0.0;
			for(size_t i1 = 0; i1 < num_target; i1++)
			{
				// здесь не учтены пока периодические условия
				// если атомы цели разрежутся периодической границей, 
				// возникнет ошибка
				target_crd[n2] += eng->crd[target_list[i1] * 3 + n2];
			}
		}
		if(num_target)
		{
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				target_crd[n2] /= num_target;
			}
		}

		// корректируем x и y координаты prob_atom, чтобы они совпадали с центром цели
		prob_atom_crd[0] = target_crd[0];
		prob_atom_crd[1] = target_crd[1];
#if 0
		f64 step = 1.0 / double(total_frames);
		if (this->prob_atom_move_direction)
		{
			prob_atom_crd[2] = target_crd[2]-0.5 + iframe*step;
		}
		else
		{
			prob_atom_crd[2] = target_crd[2]+0.5 - iframe*step;
		}
#else
		f64 step = 2.0 * this->periodic_box_HALFdim[2] / double(total_frames);
		if (this->prob_atom_move_direction)
		{
			prob_atom_crd[2] = -this->periodic_box_HALFdim[2] + iframe*step;
		}
		else
		{
			prob_atom_crd[2] = +this->periodic_box_HALFdim[2] - iframe*step;
		}
#endif
		//dyn->SetProbAtom(n_prob_atom, prob_atom_crd);
		//устанавливаем координаты пробного атома в модели
		atom ** glob_atmtab = eng->GetSetup()->GetAtoms();
		if (n_prob_atom < eng->GetSetup()->GetAtomCount())
		{
			fGL x = prob_atom_crd[0];
			fGL y = prob_atom_crd[1];
			fGL z = prob_atom_crd[2];
			
			glob_atmtab[n_prob_atom]->SetCRD(0, x, y, z);
		}
		// производим оптимизацию геометрии при фиксированной z координате пробного атома и атомов цели
		// this->working_prob_atom_GeomOpt(go);			
		//{					
		//const bool updt = true;

		// копируем координаты из модели в машину - здесь координата пробного атома уже изменена

		CopyCRD(this, eng, 0);

		//#############################################
		eng1_mm * engmm = dynamic_cast<eng1_mm *>(eng);
		if (engmm)	engmm->UpdateTerms();
		//#############################################


		char buffer[1024];
		f64  last_energy = 0.0;		// this is for output and delta_e test...
		
		PrintToLog("Cycle    Energy            Gradient       Step        Delta E\n");
		
		//ThreadUnlock();
		
		i32s n1 = 0;		// n1 counts the number of steps...
		bool cancel = false;
		while (!cancel)
		{
			Sleep(10);
			opt->TakeCGStep(conjugate_gradient::Newton2An);

	#if USE_BOUNDARY_OPT_ON_PROBNIY_ATOM_GEOMOPT
			b_opt->TakeCGStep(conjugate_gradient::Newton2An);
	#endif	
			//#############################################
			eng1_mm * engmm = dynamic_cast<eng1_mm *>(eng);
			if (engmm)	engmm->UpdateTerms();
			//#############################################
			// problem: the gradient information is in fact not precise in this stage. the current gradient
			// is the one that was last calculated in the search, and it is not necessarily the best one.
			// to update the gradient, we need to Compute(1) here. JUST SLOWS GEOMOPT DOWN -> DISABLE!
			///////////////////////////////////////////////////////////////////////////////////////////////
			eng->Compute(1);	// this is not vital, but will update the gradient vector length...
			///////////////////////////////////////////////////////////////////////////////////////////////
			//printf("eng->GetEnergy() = %f\n", eng->GetEnergy());
			//ThreadLock();

			fprintf(dat, "%d,%d,%e\n", iframe, n1, eng->GetEnergy());
			
			if (!(n1 % 5))
			{
				double progress = 0.0;
				if (param.enable_nsteps) progress = (double) n1 / (double) (param.treshold_nsteps);
				
				double graphdata = opt->optval;		// this is an array of size 1...
				
				cancel = SetProgress(progress, & graphdata);
				
				if (n1 != 0)
				{
					sprintf(buffer, "%4d %12.5f kJ/mol  %10.4e %10.4e %10.4e \n", n1,
					opt->optval, eng->GetGradientVectorLength(), opt->optstp, last_energy - opt->optval);
				}
				else
				{
					sprintf(buffer, "%4d %12.5f kJ/mol  %10.4e %10.4e ********** \n", n1,
					opt->optval, eng->GetGradientVectorLength(), opt->optstp);
				}
				
				PrintToLog(buffer);
			}
			
			bool terminate = false;
			
			if (param.enable_nsteps)	// the nsteps test...
			{
				if (n1 >= param.treshold_nsteps)
				{
					terminate = true;
					PrintToLog("the nsteps termination test was passed.\n");
				}
			}
			
			if (param.enable_grad)		// the grad test...
			{
				if (eng->GetGradientVectorLength() < param.treshold_grad)
				{
					terminate = true;
					PrintToLog("the grad termination test was passed.\n");
				}
			}
			
			if (param.enable_delta_e)	// the delta_e test...
			{
				bool flag = false; const f64 treshold_step = 1.0e-12;		// can we keep this as a constant???
				//if (n1 != 0 && (last_energy - opt->optval) != 0.0 && fabs(last_energy - opt->optval) < param.treshold_delta_e) flag = true;
				if (n1 > param.treshold_delta_e_min_nsteps && fabs(last_energy - opt->optval) < param.treshold_delta_e) flag = true;
				//if ((opt->optstp != 0.0) && (opt->optstp < treshold_step)) flag = true;
				
				if (flag)
				{
					terminate = true;
					PrintToLog("the delta_e termination test was passed.\n");
				}
			}

			
			
			last_energy = opt->optval;
			
			if (!(n1 % 10) || terminate)
			{
				CopyCRD(eng, this, 0);
				//CenterCRDSet(0, false);
				
				UpdateAllGraphicsViews(updt);
			}
			
			//ThreadUnlock();
			
			if (terminate) break;		// exit the loop here!!!
			
			n1++;	// update the number of steps...
		}
			
		// we will not delete current_eng here, so that we can draw plots using it...
		// we will not delete current_eng here, so that we can draw plots using it...
		// we will not delete current_eng here, so that we can draw plots using it...
			
		// above, CopyCRD was done eng->mdl and then CenterCRDSet() was done for mdl.
		// this might cause that old coordinates remain in eng object, possibly affecting plots.
		// here we sync the coordinates and other plotting data in the eng object.
		
		ThreadLock();
		//CopyCRD(this, eng, 0);
		SetupPlotting();
		ThreadUnlock();
		//}

		//"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""//
		//"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""//
		//запись строки в лог файл
		f64 prob_atom_f[3];
		//получаем силу, действующую на пробный атом
		for (i32s n2 = 0; n2 < 3; n2++)
		{
			prob_atom_f[n2] = eng->d1[n_prob_atom * 3 + n2];
		}
		double rz = prob_atom_crd[2] - target_crd[2];

		if (iframe == 0)
			energy00 = opt->optval;


		ostrstream str2b(mbuff1, sizeof(mbuff1));
		str2b << iframe << ",";
		str2b << rz << "," << opt->optval << "," << opt->optval-energy00 << ",";
		str2b << prob_atom_crd[2] << ",";
		if(prob_atom_move_direction)
			str2b << prob_atom_f[2] << ",";
		else
			str2b << -prob_atom_f[2] << ",";

		//str2b << target_crd[2];
		str2b << n1;

/*#if USE_BOUNDARY_OPT_ON_PROBNIY_ATOM_GEOMOPT
		str2b 
			<< "," << this->periodic_box_HALFdim[0] 
			<< "," << this->periodic_box_HALFdim[1]
			<< "," << this->periodic_box_HALFdim[2];
#endif*/

		eng1_mm * eng_mm = dynamic_cast<eng1_mm *>(eng);
		if (eng_mm)
		{
			str2b << "," << eng_mm->GetBondedTermsEnergy(); 
			str2b << "," << eng_mm->GetNonBondedTermsEnergy();

			str2b << "," << eng_mm->GetBondStretchEnergy(); 
			str2b << "," << eng_mm->GetAngleStretchEnergy();
			str2b << "," << eng_mm->GetTorsionEnergy(); 
			str2b << "," << eng_mm->GetOutOfPlaneEnergy(); 

			str2b << "," << eng_mm->GetDispersionEnergy(); 
			str2b << "," << eng_mm->GetElectrostaticEnergy(); 
			//str2b << "," << eng_mm->GetNonBondedEnergyC(); 
			//str2b << "," << eng_mm->GetNonBondedEnergyD();
		}
		str2b << endl << ends;

		logfile2 << mbuff1;
		PrintToLog(mbuff1);
		//"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""//
		//"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""//

//eng->Check(1);
//получаем силу, действующую на пробный атом
/*for (i32s n2 = 0; n2 < 3; n2++)
{
	printf("eng->d1[n_prob_atom * 3 + n2] = %f\n", eng->d1[n_prob_atom * 3 + n2]);
}
f64 old;
cin >> old;*/
		//"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""//
		//"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""//
		// запись фрейма в файл траектории
		{
			CopyCRD(eng, this, 0);
			
			const float ekin = 0.0;
			const float epot = opt->optval;
			
			ofile.write((char *) & ekin, sizeof(ekin));	// kinetic energy, float.
			ofile.write((char *) & epot, sizeof(epot));	// potential energy, float.
			
			for (iter_al itx = GetAtomsBegin();itx != GetAtomsEnd();itx++)
			{
				const fGL * cdata = (* itx).GetCRD(0);
				for (i32s t4 = 0;t4 < 3;t4++)		// all coordinates, float.
				{
					float t1a = cdata[t4];
					ofile.write((char *) & t1a, sizeof(t1a));
				}
			}
		}
		//"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""//
		//"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""//
	}
	ofile.close();
	logfile2.close();
	fclose(dat);

	delete opt;
#if USE_BOUNDARY_OPT_ON_PROBNIY_ATOM_GEOMOPT
	delete b_opt;
#endif
}
#endif /*PROBNIY_ATOM_GEOMOPT*/

void model::LoadSelected(char * filename)
{
	vector<i32s> selected_atoms_list;
	ReadTargetListFile(filename, selected_atoms_list);
	i32s index = 0;
	for (iter_al itx = GetAtomsBegin();itx != GetAtomsEnd();itx++)
	{
		bool selected = false;
		for (vector<i32s>::iterator it = selected_atoms_list.begin();
			it != selected_atoms_list.end(); it++)
		{
			if (index == (*it))
			{
				selected = true;
				break;
			}
		}
		if (selected) 
			(* itx).flags |= ATOMFLAG_SELECTED;
		else
			(* itx).flags &= (~ATOMFLAG_SELECTED);

		index++;			
	}
}
void model::SaveSelected(char * filename)
{
	FILE * stream;
	stream = fopen(filename, "wt");
	if(stream)
	{        
		int index = 0;
		for (iter_al itx = GetAtomsBegin();itx != GetAtomsEnd();itx++)
		{
			bool selected = ((* itx).flags & ATOMFLAG_SELECTED);
			if (selected) fprintf(stream, "%d\n", index);
			index++;			
		}
		fclose(stream);
	}
}

void model::SaveBox(char * boxfilename)
{
	char mbuff1[256];
	// открываем box файл
	ofstream boxfile;
	boxfile.open(boxfilename, ios::out);
	// пишем box файл
	ostrstream str3b(mbuff1, sizeof(mbuff1));
	str3b << this->periodic_box_HALFdim[0] << " ";
	str3b << this->periodic_box_HALFdim[1] << " ";
	str3b << this->periodic_box_HALFdim[2] << endl << ends;
	boxfile << mbuff1;
	boxfile.close();
}

bool model::LoadBox(char * boxfilename)
{
	const int n = 1024;
	char szBuff[n];
	bool result = false;
	FILE * stream;
	stream = fopen(boxfilename, "rt");
	if(stream)
	{
		if (!feof(stream))
		{
			char * ch;
			ch = fgets(szBuff,n,stream);
			if( ch != NULL && strlen(szBuff) > 1)
			{
				printf("szBuff = %s\n", szBuff);

				f64	box0, box1, box2;
				if (3 == sscanf(szBuff, "%lf %lf %lf", 
					&box0, 
					&box1, 
					&box2)) 
				{
					printf("box = [%f %f %f]\n", box0, box1, box2);

					this->periodic_box_HALFdim[0] = box0;
					this->periodic_box_HALFdim[1] = box1;
					this->periodic_box_HALFdim[2] = box2;
					result = true;
				}
			}
		}
		fclose(stream);
	}
	return result;
}


/*
void model::Correct_periodic_box_HALFdim(engine * eng)
{
	const f64 delta = 0.01;	// the finite difference step...
	const f64 epsilon = 0.000;	// the finite difference step...
	f64 dE;
	if (eng)
	{	
		eng->Compute(0);
		f64 tmp1 = eng->energy;
		dE = eng->energy - tmp1;

printf("this->periodic_box_HALFdim[0] = %f\n", this->periodic_box_HALFdim[0]);
printf("this->periodic_box_HALFdim[1] = %f\n", this->periodic_box_HALFdim[1]);
printf("this->periodic_box_HALFdim[2] = %f\n", this->periodic_box_HALFdim[2]);
printf("eng->energy = %f\n", eng->energy);
printf("tmp1 = %f\n", tmp1);
printf("eng->energy - tmp1 = %0.20f\n", eng->energy - tmp1);

		while(true)
		{
			int tmp = 0;
			// x
			this->periodic_box_HALFdim[0] -= delta;
			eng->Compute(0);
			dE = eng->energy - tmp1;

printf("this->periodic_box_HALFdim[0] = %f\n", this->periodic_box_HALFdim[0]);
printf("this->periodic_box_HALFdim[1] = %f\n", this->periodic_box_HALFdim[1]);
printf("this->periodic_box_HALFdim[2] = %f\n", this->periodic_box_HALFdim[2]);
printf("eng->energy = %f\n", eng->energy);
printf("tmp1 = %f\n", tmp1);
printf("dE = %0.20f\n", dE);

			if (dE < -epsilon)
			{
				tmp1 = eng->energy;
printf("x eng->energy - tmp1 = %f %f\n", dE, log10(fabs(dE)));
			}
			else
			{
				this->periodic_box_HALFdim[0] += delta;
				tmp++;
			}
			// y
			this->periodic_box_HALFdim[1] -= delta;
			eng->Compute(0);
			dE = eng->energy - tmp1;

printf("this->periodic_box_HALFdim[0] = %f\n", this->periodic_box_HALFdim[0]);
printf("this->periodic_box_HALFdim[1] = %f\n", this->periodic_box_HALFdim[1]);
printf("this->periodic_box_HALFdim[2] = %f\n", this->periodic_box_HALFdim[2]);
printf("eng->energy = %f\n", eng->energy);
printf("tmp1 = %f\n", tmp1);
printf("dE = %0.20f\n", dE);

			if (dE < -epsilon)
			{
				tmp1 = eng->energy;
printf("y eng->energy - tmp1 = %f\n", dE, log10(fabs(dE)));
			}
			else
			{
				this->periodic_box_HALFdim[1] += delta;
				tmp++;
			}

			if (tmp == 2)
				break;
		}
	}

}
*/
/*################################################################################################*/

crd_set::crd_set(void)
{
	description = NULL;
	
	accum_weight = 1.0;		// accum_value is not updated here?!?!?!?!?!
	visible = false;
}

crd_set::crd_set(const crd_set & p1)
{
	description = NULL;
	SetDescription(p1.description);
	
	accum_weight = p1.accum_weight;
	accum_value = p1.accum_value;
	visible = p1.visible;
}

crd_set::~crd_set(void)
{
	if (description != NULL) delete[] description;
}

void crd_set::SetDescription(const char * p1)
{
	if (description != NULL) delete[] description;
	
	if (p1 != NULL)
	{
		description = new char[strlen(p1) + 1];
		strcpy(description, p1);
	}
	else description = NULL;
}

/*################################################################################################*/

// eof

//DEL void model::CalcMaxMinCoordinates()
//DEL {
//DEL 	/*
//DEL 	atom ** glob_atmtab = p2->GetSetup()->GetAtoms();
//DEL 	for (i32s n1 = 0;n1 < p2->GetSetup()->GetAtomCount();n1++)
//DEL 	{
//DEL 		const fGL * cdata = glob_atmtab[n1]->GetCRD(p3);
//DEL 		if ( n1 == 0)
//DEL 		{
//DEL 			p1->maxCRD[0] = cdata[0];
//DEL 			p1->minCRD[0] = cdata[0];
//DEL 			p1->maxCRD[1] = cdata[1];
//DEL 			p1->minCRD[1] = cdata[1];
//DEL 			p1->maxCRD[2] = cdata[2];
//DEL 			p1->minCRD[2] = cdata[2];
//DEL 		}
//DEL 		else
//DEL 		{
//DEL 			if (p1->maxCRD[0] < cdata[0]) p1->maxCRD[0] = cdata[0];
//DEL 			if (p1->minCRD[0] > cdata[0]) p1->minCRD[0] = cdata[0];
//DEL 			if (p1->maxCRD[1] < cdata[1]) p1->maxCRD[1] = cdata[1];
//DEL 			if (p1->minCRD[1] > cdata[1]) p1->minCRD[1] = cdata[1];
//DEL 			if (p1->maxCRD[2] < cdata[2]) p1->maxCRD[2] = cdata[2];
//DEL 			if (p1->minCRD[2] > cdata[2]) p1->minCRD[2] = cdata[2];
//DEL 		}
//DEL 		
//DEL 		p2->crd[n1 * 3 + 0] = cdata[0];
//DEL 		p2->crd[n1 * 3 + 1] = cdata[1];
//DEL 		p2->crd[n1 * 3 + 2] = cdata[2];
//DEL 	}
//DEL 	*/
//DEL }
