// ENG1_MM.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "eng1_mm.h"

#include "eng1_mm_tripos52.h"
#include "eng1_mm_default.h"
#include "eng1_mm_prmfit.h"

#include <algorithm>
#include <strstream>
using namespace std;

/*################################################################################################*/

const i32u setup1_mm::eng_id_tab[] =
{
	ENG1_MM_TRIPOS52,
	
	ENG1_MM_DEFAULT,
	ENG1_MM_PERIODIC,
	ENG1_MM_PERIODIC_WBP,
	
	ENG1_MM_EXPERIMENTAL,
	
	(i32u) NOT_DEFINED
};

const char * setup1_mm::eng_name_tab[] =
{
	"tripos5.2 implementation ported from ghemical-1.00",
	
	"the default engine (under construction) ; default_mbp",
	"the periodic engine (minimum image model) ; default_mbp",
	"the periodic engine (wall boundary potential) ; default_wbp",
	
	"experimental ; eng1_prmfit",
	
	NULL
};

setup1_mm::setup1_mm(model * p1) : setup(p1)
{
//	exceptions = true;
	exceptions = false;	// keep false as long as there are hard-coded bug traps that call exit()...
	
	// REQUEST_UNITED_ATOM??? here or below???
	// REQUEST_UNITED_ATOM??? here or below???
	// REQUEST_UNITED_ATOM??? here or below???
}

setup1_mm::~setup1_mm(void)
{
}

void setup1_mm::UpdateAtomFlags(void)
{
	for (iter_al it1 = GetModel()->GetAtomsBegin();it1 != GetModel()->GetAtomsEnd();it1++)
	{
		(* it1).flags |= ATOMFLAG_IS_MM_ATOM;
	}
}

i32u setup1_mm::static_GetEngineCount(void)
{
	i32u count = 0;		// counted by names...
	while (eng_name_tab[count] != NULL) count++;
	
	return count;
}

i32u setup1_mm::static_GetEngineIDNumber(i32u eng_index)
{
	return eng_id_tab[eng_index];
}

const char * setup1_mm::static_GetEngineName(i32u eng_index)
{
	return eng_name_tab[eng_index];
}

const char * setup1_mm::static_GetClassName(void)
{
	static char cn[] = "allmm";
	return cn;
}

i32u setup1_mm::GetEngineCount(void)
{
	return static_GetEngineCount();
}

i32u setup1_mm::GetEngineIDNumber(i32u eng_index)
{
	return static_GetEngineIDNumber(eng_index);
}

const char * setup1_mm::GetEngineName(i32u eng_index)
{
	return static_GetEngineName(eng_index);
}

const char * setup1_mm::GetClassName(void)
{
	return static_GetClassName();
}

engine * setup1_mm::CreateEngineByIndex(i32u eng_index)
{
	if (eng_index >= GetEngineCount())
	{
		cout << "setup1_mm::CreateEngineByIndex() failed!" << endl;
		return NULL;
	}
	
	GetModel()->UpdateIndex();
	UpdateSetupInfo();
	
prmfit_tables * tab = NULL; eng1_mm_prmfit * eng = NULL;

char forcefield_path[256];
ostrstream fps(forcefield_path, sizeof(forcefield_path));
fps << model::libdata_path << "/param_mm/prmfit" << ends;
	
	switch (eng_id_tab[eng_index])
	{
		case ENG1_MM_DEFAULT:
		GetModel()->use_periodic_boundary_conditions = false;
		
if (GetModel()->use_boundary_potential) GetModel()->Message("use_boundary_potential = TRUE");
		return new eng1_mm_default_mbp(this, 1);
		
		case ENG1_MM_TRIPOS52:
		GetModel()->use_periodic_boundary_conditions = false;
		return new eng1_mm_tripos52_mbp(this, 1);
		
		case ENG1_MM_PERIODIC:
		GetModel()->use_periodic_boundary_conditions = true;
		
		if (!GetModel()->IsGroupsClean()) GetModel()->UpdateGroups();
		return new eng1_mm_default_mim(this, 1);
		
		case ENG1_MM_PERIODIC_WBP:
		GetModel()->use_periodic_boundary_conditions = true;
		
		if (!GetModel()->IsGroupsClean()) GetModel()->UpdateGroups();
		return new eng1_mm_default_wbp(this, 1);
		
		case ENG1_MM_EXPERIMENTAL:
		GetModel()->use_periodic_boundary_conditions = false;
		
	tab = new prmfit_tables(forcefield_path);	// experimental!!! for debugging only!!!
	eng = new eng1_mm_prmfit(this, 1, * tab);	// experimental!!! for debugging only!!!
	delete tab; return eng;				// experimental!!! for debugging only!!!
		
		default:
		cout << "fatal error at setup1_mm::CreateEngineByIndex()" << endl;
		return NULL;
	}
}

/*################################################################################################*/

const f64 eng1_mm::fudgeLJ = 0.75;
const f64 eng1_mm::fudgeQQ = 0.75;

eng1_mm::eng1_mm(setup * p1, i32u p2) : engine(p1, p2)
{

// make qm-atoms "extra-atoms" separately at the end ; those can then be included in the calculations in a qm/mm model!
// still all the computations and intermediate results will be calculated in a similar way for qm/mm atoms.
//
//	mm-atom
//	mm-atom
//	mm-atom
//	mm-atom
//		 << qm_atom_limit
//	qm-atom
//	qm-atom
//
//	AND THE SAME FOR BONDS???

	// create the local-to-global lookup table...
	// create the local-to-global lookup table...
	// create the local-to-global lookup table...
	
	l2g_mm = new i32u[GetSetup()->GetMMAtomCount()];
	
	atom ** atmtab = GetSetup()->GetMMAtoms();
	atom ** glob_atmtab = GetSetup()->GetAtoms();
	for (i32s n1 = 0;n1 < GetSetup()->GetMMAtomCount();n1++)
	{
		i32s index = 0;
		while (index < GetSetup()->GetAtomCount())
		{
			if (atmtab[n1] == glob_atmtab[index]) break;
			else index++;
		}
		
		if (index >= GetSetup()->GetAtomCount())
		{
			cout << "BUG: eng1_mm ctor failed to create the l2g lookup table." << endl;
			exit(EXIT_FAILURE);
		}
		
		l2g_mm[n1] = index;
	}
	
	// create the connectivity records...
	// create the connectivity records...
	// create the connectivity records...
	
	// the records itself are made only for MM-atoms, but since atom::cr_list
	// is used to do the search, the lists may contain also non-MM-atoms.
	
	range_cr1 = new i32s[GetSetup()->GetMMAtomCount() + 1];		// this is for 1-2 and 1-3 interactions...
	range_cr2 = new i32s[GetSetup()->GetMMAtomCount() + 1];		// this is only for 1-4 interactions...
	
	//cout << "creating connectivity records..." << endl;		//CGI!
	
	for (i32s index = 0;index < GetSetup()->GetMMAtomCount();index++)
	{
		range_cr1[index] = cr1.size();
		range_cr2[index] = cr2.size();
		
		SearchCR1a(atmtab[index]);
	}
	
	range_cr1[GetSetup()->GetMMAtomCount()] = cr1.size();		// this is just the end mark...
	range_cr2[GetSetup()->GetMMAtomCount()] = cr2.size();		// this is just the end mark...
}

eng1_mm::~eng1_mm(void)
{
	delete[] l2g_mm;
	
	delete[] range_cr1;
	delete[] range_cr2;
}

void eng1_mm::Compute(i32u p1)
{
	E_solute = 0.0;		// re-think this for eng2_xx_yy classes!!!
	E_solvent = 0.0;	// re-think this for eng2_xx_yy classes!!!
	E_solusolv = 0.0;	// re-think this for eng2_xx_yy classes!!!
	
	if (p1)
	{
		for (i32s n1 = 0;n1 < GetSetup()->GetMMAtomCount();n1++)
		{
			d1[l2g_mm[n1] * 3 + 0] = 0.0;
			d1[l2g_mm[n1] * 3 + 1] = 0.0;
			d1[l2g_mm[n1] * 3 + 2] = 0.0;
		}
	}
	
	ComputeBT1(p1);
	
	// communicate via MPI if parallel?!?!?!
	// contents of bt1_data tables should be syncronized...
	
	ComputeBT2(p1);
	
	// communicate via MPI if parallel?!?!?!
	// contents of bt2_data tables should be syncronized...
	
	ComputeBT3(p1);
	ComputeBT4(p1);
	
	ComputeNBT1(p1);
	ComputeNBT2(p1);
	
	energy = energy_bt1 + energy_bt2 + energy_bt3 + energy_bt4;
	energy += energy_nbt1a + energy_nbt1b + energy_nbt1c + energy_nbt1d;
	
//cout << "energy-b = " << energy_bt1 << " " << energy_bt2 << " " << energy_bt3 << " " << energy_bt4 << " " << endl;
//cout << "energy-nb = " << energy_nbt1a << " " << energy_nbt1b << " " << energy_nbt1c << " " << energy_nbt1d << " " << endl;
}

void eng1_mm::SearchCR1a(atom * atm1)
{
	for (iter_cl itc = atm1->cr_list.begin();itc != atm1->cr_list.end();itc++)
	{
		atom * atm2 = (* itc).atmr;
		cr1.push_back(atm2);
		
		SearchCR1b(atm2, (* itc).bndr);
	}
}

void eng1_mm::SearchCR1b(atom * atm2, bond * reserved1)
{
	for (iter_cl itc = atm2->cr_list.begin();itc != atm2->cr_list.end();itc++)
	{
		if ((* itc).bndr == reserved1) continue;
		
		atom * atm3 = (* itc).atmr;
		cr1.push_back(atm3);
		
		SearchCR2(atm3, reserved1, (* itc).bndr);
	}
}

void eng1_mm::SearchCR2(atom * atm3, bond * reserved1, bond * reserved2)
{
	for (iter_cl itc = atm3->cr_list.begin();itc != atm3->cr_list.end();itc++)
	{
		if ((* itc).bndr == reserved1) continue;
		if ((* itc).bndr == reserved2) continue;
		
		atom * atm4 = (* itc).atmr;
		cr2.push_back(atm4);
	}
}

// f = sum[(r/a)^-3] = sum[a^3 * r^-3]		// now seems to be r^-12
// df/dr = -3 * sum[a^3 * r^-4]

fGL eng1_mm::GetVDWSurf(fGL * pp, fGL * dd)
{
	fGL vdwsv = 0.0;
	if (dd != NULL) dd[0] = dd[1] = dd[2] = 0.0;
	
	atom ** atmtab = GetSetup()->GetMMAtoms();
	for (i32s n1 = 0;n1 < GetSetup()->GetMMAtomCount();n1++)
	{
		fGL tmp1[3]; fGL r2 = 0.0;
		const f64 * cdata = & crd[l2g_mm[n1] * 3];
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			tmp1[n2] = pp[n2] - cdata[n2];
			r2 += tmp1[n2] * tmp1[n2];
		}
		
		if (r2 == 0.0) return +1.0e+35;		// numeric_limits<fGL>::max()?!?!?!
		fGL r1 = sqrt(r2);
		
		fGL tmp2 = r1 / (atmtab[n1]->vdwr + 0.0);	// solvent radius??? 0.15
		fGL qqq = tmp2 * tmp2 * tmp2 * tmp2;
		
		fGL tmp3 = 1.0 / (qqq * qqq * qqq);
		vdwsv += tmp3;
		
		if (dd != NULL)		// sign ??? constant ???
		{
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				fGL tmp4 = tmp1[n2] / r1;
				fGL tmp5 = tmp4 * tmp3 / tmp2;
				dd[n2] += tmp5;
			}
		}
	}
	
	return vdwsv;
}

// f = sum[Q/r] = sum[Q * r^-1]
// df/dr = -1 * sum[Q * r^-2]

fGL eng1_mm::GetESP(fGL * pp, fGL * dd)
{
	fGL espv = 0.0;
	if (dd != NULL) dd[0] = dd[1] = dd[2] = 0.0;
	
	atom ** atmtab = GetSetup()->GetMMAtoms();
	for (i32s n1 = 0;n1 < GetSetup()->GetMMAtomCount();n1++)
	{
		fGL tmp1[3]; fGL r2 = 0.0;
		const f64 * cdata = & crd[l2g_mm[n1] * 3];
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			tmp1[n2] = pp[n2] - cdata[n2];
			r2 += tmp1[n2] * tmp1[n2];
		}
		
		if (r2 == 0.0) return +1.0e+35;		// numeric_limits<fGL>::max()?!?!?!
		fGL r1 = sqrt(r2);
		
		// do we have a correct constant here??? I think so, if we define
		// electrostatic potential as potential energy of a unit positive charge.
		
		fGL tmp2 = 4.1868 * 33.20716 * atmtab[n1]->charge / r1;
		espv += tmp2;
		
		if (dd != NULL)		// sign ??? constant ???
		{
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				fGL tmp3 = tmp1[n2] / r1;
				fGL tmp4 = tmp3 * tmp2 / r1;
				dd[n2] += tmp4;
			}
		}
	}
	
	return espv;
}
/*################################################################################################*/

// eof
