// ENG1_SF.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "eng1_sf.h"	// config.h is here -> we get ENABLE-macros here...

#include <fstream>
#include <strstream>
#include <algorithm>
using namespace std;

/*################################################################################################*/

// the surface area code apparently contains some bugs, since it sometimes
// crashes. another possibility is that the surface area math contains some
// bad cases (like arcs/segments with zero length/area ???) which should be
// avoided somehow. either way, the numerical and analytical gradients of
// surface area seem to match.

// LOWLIMIT is a cheat to prevent zero-divisions in surface-area calculations.
// if a zero-division seems to happen, the values are just changed to LOWLIMIT
// in as early stage as possible, thus making minimum effects on results...

#define LOWLIMIT 0.0000001	// 0.0000001 seems to work quite well...

/*################################################################################################*/

void eng1_sf_param_SetDefaultValues(eng1_sf_param & prm)
{
	prm.vdwrad = 1.325;
	prm.lenjon = 1.225;
	prm.wescc = 1.000;
	prm.wescd = 0.500;
	prm.dipole1 = 2.0;
	prm.dipole2 = 4.0;
	prm.epsilon1 = 2.00;
	prm.epsilon2 = 1.00;
	prm.epsilon3 = 2.00;
	prm.epsilon4 = 1.40;
	prm.epsilon5 = 1.96;
	prm.epsilon9 = 0.0015;
	prm.exp_solv_1n = 0.15;
	prm.exp_solv_1p = 1.00;
	prm.exp_solv_2 = 1.00;
	prm.imp_solv_1n = +4.00;
	prm.imp_solv_1p = +2.00;
	prm.imp_solv_2 = -12.00;
	prm.solvrad = 0.15;
	prm.wang = 0.85;
	prm.wtor1 = 0.75;
	prm.wtor2 = 0.50;
	prm.wrep = 0.035;
	
	prm.charge_wes[0] = +0.0744;
	prm.charge_wes[1] = +0.0600;
	prm.charge_wes[2] = -0.0488;
	prm.charge_wes[3] = -0.0681;
	
	prm.charge_sasa1[0] = -9.605;
	prm.charge_sasa1[1] = -1.505;
	prm.charge_sasa1[2] = +0.523;
	prm.charge_sasa1[3] = +2.593;
	
	prm.charge_sasa2[0] = +2.069;
	prm.charge_sasa2[1] = -0.629;
	prm.charge_sasa2[2] = -0.224;
	prm.charge_sasa2[3] = -0.272;
	
	prm.charge_pKa[0] = 7.830;
	prm.charge_pKa[1] = 4.651;
	prm.charge_pKa[2] = 11.5;
	prm.charge_pKa[3] = 5.069;
	prm.charge_pKa[4] = 9.5;
	prm.charge_pKa[5] = 6.319;
	prm.charge_pKa[6] = 6.088;
	prm.charge_pKa[7] = 10.64;
	prm.charge_pKa[8] = 10.81;
	
	prm.charge_acid[0] = false;
	prm.charge_acid[1] = true;
	prm.charge_acid[2] = false;
	prm.charge_acid[3] = true;
	prm.charge_acid[4] = true;
	prm.charge_acid[5] = true;
	prm.charge_acid[6] = false;
	prm.charge_acid[7] = false;
	prm.charge_acid[8] = true;
	
	// the pH setting is a default value that can be modified!!!
	// the pH setting is a default value that can be modified!!!
	// the pH setting is a default value that can be modified!!!
	
	prm.pH = 7.0;
}

void eng1_sf_param_MakeCopy(eng1_sf_param & sprm, eng1_sf_param & tprm)
{
	tprm.vdwrad = sprm.vdwrad;
	tprm.lenjon = sprm.lenjon;
	tprm.wescc = sprm.wescc;
	tprm.wescd = sprm.wescd;
	tprm.dipole1 = sprm.dipole1;
	tprm.dipole2 = sprm.dipole2;
	tprm.epsilon1 = sprm.epsilon1;
	tprm.epsilon2 = sprm.epsilon2;
	tprm.epsilon3 = sprm.epsilon3;
	tprm.epsilon4 = sprm.epsilon4;
	tprm.epsilon5 = sprm.epsilon5;
	tprm.epsilon9 = sprm.epsilon9;
	tprm.exp_solv_1n = sprm.exp_solv_1n;
	tprm.exp_solv_1p = sprm.exp_solv_1p;
	tprm.exp_solv_2 = sprm.exp_solv_2;
	tprm.imp_solv_1n = sprm.imp_solv_1n;
	tprm.imp_solv_1p = sprm.imp_solv_1p;
	tprm.imp_solv_2 = sprm.imp_solv_2;
	tprm.solvrad = sprm.solvrad;
	tprm.wang = sprm.wang;
	tprm.wtor1 = sprm.wtor1;
	tprm.wtor2 = sprm.wtor2;
	tprm.wrep = sprm.wrep;
	
	for (int n1 = 0;n1 < 4;n1++)
	{
		tprm.charge_wes[n1] = sprm.charge_wes[n1];
		tprm.charge_sasa1[n1] = sprm.charge_sasa1[n1];
		tprm.charge_sasa2[n1] = sprm.charge_sasa2[n1];
	}
	
	for (int n1 = 0;n1 < 9;n1++)
	{
		tprm.charge_pKa[n1] = sprm.charge_pKa[n1];
		tprm.charge_acid[n1] = sprm.charge_acid[n1];
	}
	
	tprm.pH = sprm.pH;
}

/*################################################################################################*/

sf_chn::sf_chn(void)
{
}

sf_chn::sf_chn(const sf_chn & p1)
{
	res_vector = p1.res_vector;
}

sf_chn::~sf_chn(void)
{
}

/*################################################################################################*/

sf_res::sf_res(void)
{
	symbol = '?';
	for (i32s n1 = 0;n1 < 5;n1++)
	{
		peptide[n1] = NULL;
	}
	
	natm = 0;
	atmr[0] = atmr[1] = atmr[2] = NULL;
	loc_varind[0] = loc_varind[1] = loc_varind[2] = NOT_DEFINED;
	
	state = SF_STATE_LOOP;
}

sf_res::sf_res(char p1, atom * a1, atom * a2, atom * a3, atom * a4, atom * a5, i32s p2, atom * r1, atom * r2, atom * r3, i32s i1, i32s i2, i32s i3)
{
	symbol = p1;
	peptide[0] = a1;
	peptide[1] = a2;
	peptide[2] = a3;
	peptide[3] = a4;
	peptide[4] = a5;
	
	natm = p2;
	atmr[0] = r1;
	atmr[1] = r2;
	atmr[2] = r3;
	loc_varind[0] = i1;
	loc_varind[1] = i2;
	loc_varind[2] = i3;
	
	state = SF_STATE_LOOP;
}

sf_res::sf_res(const sf_res & p1)
{
	symbol = p1.symbol;
	for (i32s n1 = 0;n1 < 5;n1++)
	{
		peptide[n1] = p1.peptide[n1];
	}
	
	natm = p1.natm;
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		atmr[n1] = p1.atmr[n1];
		loc_varind[n1] = p1.loc_varind[n1];
	}
	
	state = p1.state;
}

sf_res::~sf_res(void)
{
}

/*################################################################################################*/

setup1_sf::setup1_sf(model * p1, bool convert_crd) : setup(p1)
{
	united_atoms = false;
	
	eng1_sf_param_SetDefaultValues(prm);
	
	exp_solv = false;
	
	// since the SF is the highest-level model, it must contain all parts of the system.
	// take copies of all detected chains in the system, whether it was requested or not.
	
	// initialize...
	
	GetModel()->UpdateChains();
	DefineSecondaryStructure(GetModel());
	vector<chn_info> & ci_vector = (* GetModel()->ref_civ);
	
	// the main loop; make contiguous chains like this:
	
	// rA0 -- rB0 -- rC0 -- rD0 -- and_so_on
	//         |             |
	//        rB1           rD1
	//         |
	//        rB2
	
	fGL * crd_sf[3];
	crd_sf[0] = new fGL[GetModel()->cs_vector.size() * 3];
	crd_sf[1] = new fGL[GetModel()->cs_vector.size() * 3];
	crd_sf[2] = new fGL[GetModel()->cs_vector.size() * 3];
	
	i32s loc_varind_counter = 0;
	for (i32u n1 = 0;n1 < ci_vector.size();n1++)
	{
		sf_chn newchn;
		chn_vector.push_back(newchn);
		
		if (ci_vector[n1].GetType() != chn_info::amino_acid) continue;		// keep the same number of chains...
		
		iter_al range1[2];
		GetModel()->GetRange(1, n1, range1);
		
		iter_al range2[2];
		for (i32s n2 = 0;n2 < ci_vector[n1].length;n2++)
		{
			GetModel()->GetRange(2, range1, n2, range2);
			if (range2[0] == range2[1]) { cout << "empty residue!!!" << endl; exit(EXIT_FAILURE); }
			
			i32s tmp1[2]; tmp1[0] = (* range2[0]).builder_res_id >> 8;
			for (tmp1[1] = 0;tmp1[1] < (i32s) model::amino_builder.residue_vector.size();tmp1[1]++)
			{
				if (model::amino_builder.residue_vector[tmp1[1]].id == tmp1[0]) break;
			} if (tmp1[1] == (i32s) model::amino_builder.residue_vector.size()) continue;
			
			char symbol_sf = model::amino_builder.residue_vector[tmp1[1]].symbol;
			
			i32s ind = 0; while (ind < 20 && model::sf_symbols[ind] != symbol_sf) ind++;
			if (ind == 20) { cout << "BUG: unknown residue symbol." << endl; exit(EXIT_FAILURE); }
			
			i32s natm_sf = NOT_DEFINED;
			i32s atmi_sf[3] = { NOT_DEFINED, NOT_DEFINED, NOT_DEFINED };
			
			switch (symbol_sf)
			{
				case 'A': natm_sf = 1; break;
				
				case 'R': natm_sf = 3; break;
				
				case 'N': natm_sf = 2; break;
				
				case 'D': natm_sf = 2; break;
				
				case 'C': natm_sf = 2; break;
				
				case 'Q': natm_sf = 2; break;
				
				case 'E': natm_sf = 2; break;
				
				case 'G': natm_sf = 1; break;
				
				case 'H': natm_sf = 2; break;
				
				case 'I': natm_sf = 2; break;
				
				case 'L': natm_sf = 2; break;
				
				case 'K': natm_sf = 3; break;
				
				case 'M': natm_sf = 2; break;
				
				case 'F': natm_sf = 2; break;
				
				case 'P': natm_sf = 1; break;
				
				case 'S': natm_sf = 1; break;
				
				case 'T': natm_sf = 1; break;
				
				case 'W': natm_sf = 3; break;
				
				case 'Y': natm_sf = 2; break;
				
				case 'V': natm_sf = 1; break;
				
				default:
				cout << "BUG: unknown residue " << symbol_sf << " at aa2sf_Convert_sub1()." << endl;
				exit(EXIT_FAILURE);
			}
			
			vector<i32s> idv;
			idv.push_back(0x01);
			
			atmi_sf[0] = idv.front();
			for (i32u n3 = 0;n3 < GetModel()->cs_vector.size();n3++)
			{
				GetReducedCRD(range2, idv, & crd_sf[0][n3 * 3], n3);
			}
			
			if (natm_sf > 1)
			{
				idv.resize(0);
				switch (symbol_sf)
				{
					case 'R':
					idv.push_back(0x22); idv.push_back(0x21);
					break;
					
					case 'N':
					idv.push_back(0x21);
					break;
					
					case 'D':
					idv.push_back(0x21);
					break;
					
					case 'C':
					idv.push_back(0x21);
					break;
					
					case 'Q':
					idv.push_back(0x22); idv.push_back(0x21);
					break;
					
					case 'E':
					idv.push_back(0x22); idv.push_back(0x21);
					break;
					
					case 'H':
					idv.push_back(0x21); idv.push_back(0x22);
					idv.push_back(0x23); idv.push_back(0x24); idv.push_back(0x25);
					break;
					
					case 'I':
					idv.push_back(0x22);
					break;
					
					case 'L':
					idv.push_back(0x21);
					break;
					
					case 'K':
					idv.push_back(0x21);
					break;
					
					case 'M':
					idv.push_back(0x22); idv.push_back(0x21); idv.push_back(0x23);
					break;
					
					case 'F':
					idv.push_back(0x21); idv.push_back(0x24);
					break;
					
					case 'W':
					idv.push_back(0x21); idv.push_back(0x23); idv.push_back(0x24);
					break;
					
					case 'Y':
					idv.push_back(0x21); idv.push_back(0x24);
					break;
				}
				
				atmi_sf[1] = idv.front();
				for (i32u n3 = 0;n3 < GetModel()->cs_vector.size();n3++)
				{
					GetReducedCRD(range2, idv, & crd_sf[1][n3 * 3], n3);
				}
			}
			
			if (natm_sf > 2)
			{
				idv.resize(0);
				switch (symbol_sf)
				{
					case 'R':
					idv.push_back(0x24);
					break;
					
					case 'K':
					idv.push_back(0x23);
					break;
					
					case 'W':
					idv.push_back(0x28); idv.push_back(0x25);
					break;
				}
				
				atmi_sf[2] = idv.front();
				for (i32u n3 = 0;n3 < GetModel()->cs_vector.size();n3++)
				{
					GetReducedCRD(range2, idv, & crd_sf[2][n3 * 3], n3);
				}
			}
			
			// hide all atoms of the residue.
			
			for (iter_al it1 = range2[0];it1 != range2[1];it1++)
			{
				(* it1).flags |= ATOMFLAG_IS_HIDDEN;
			}
			
			// make the residue and un-hide the simplified atoms.
			
			atom * peptide_sf[5] = { NULL, NULL, NULL, NULL, NULL };
			
			atom * atmr_sf[3] = { NULL, NULL, NULL };
			i32s iloc_sf[3] = { NOT_DEFINED, NOT_DEFINED, NOT_DEFINED };
			
			bool last_residue = (n2 == ci_vector[n1].length - 1);
			for (iter_al it1 = range2[0];it1 != range2[1];it1++)
			{
				if (((* it1).builder_res_id & 0xFF) == 0x00) peptide_sf[0] = & (* it1);		// N
				if (((* it1).builder_res_id & 0xFF) == 0x01) peptide_sf[1] = & (* it1);		// C
				if (((* it1).builder_res_id & 0xFF) == 0x02) peptide_sf[2] = & (* it1);		// C
				if (((* it1).builder_res_id & 0xFF) == 0x10) peptide_sf[3] = & (* it1);			// O
				if (last_residue && ((* it1).builder_res_id & 0xFF) == 0x11) peptide_sf[4] = & (* it1);		// O
				
				if (atmi_sf[0] != NOT_DEFINED && ((* it1).builder_res_id & 0xFF) == atmi_sf[0])		// 1st
				{
					atmr_sf[0] = & (* it1);
					iloc_sf[0] = loc_varind_counter++;
				}
				
				if (atmi_sf[1] != NOT_DEFINED && ((* it1).builder_res_id & 0xFF) == atmi_sf[1])		// 2nd
				{
					atmr_sf[1] = & (* it1);
					iloc_sf[1] = loc_varind_counter++;
				}
				
				if (atmi_sf[2] != NOT_DEFINED && ((* it1).builder_res_id & 0xFF) == atmi_sf[2])		// 3rd
				{
					atmr_sf[2] = & (* it1);
					iloc_sf[2] = loc_varind_counter++;
				}
			}
			
			sf_res newres = sf_res(symbol_sf, peptide_sf[0], peptide_sf[1], peptide_sf[2], peptide_sf[3], peptide_sf[4], natm_sf, atmr_sf[0], atmr_sf[1], atmr_sf[2], iloc_sf[0], iloc_sf[1], iloc_sf[2]);
			chn_vector.back().res_vector.push_back(newres);
			
			fGL vdwr1 = -1.0; fGL vdwr2 = -1.0; fGL vdwr3 = -1.0;
			fGL mass1 = -1.0; fGL mass2 = -1.0; fGL mass3 = -1.0;
			switch (symbol_sf)
			{
				case 'A':						// 71.1
				vdwr1 = 0.193391;
				mass1 = 71.1;
				break;
				
				case 'R':						// 156.7
				vdwr1 = 0.222079;
				mass1 = 53.9;			// 1.05
				
				vdwr2 = 0.195669;
				mass2 = 51.4;			// 1.00
				
				vdwr3 = 0.189176;
				mass3 = 51.4;			// 1.00
				break;
				
				case 'N':						// 114.1
				vdwr1 = 0.224929;
				mass1 = 58.4;			// 1.05
				
				vdwr2 = 0.187591;
				mass2 = 55.7;			// 1.00
				break;
				
				case 'D':						// 114.5
				vdwr1 = 0.225038;
				mass1 = 58.6;			// 1.05
				
				vdwr2 = 0.187136;
				mass2 = 55.9;			// 1.00
				break;
				
				case 'C':						// 103.2
				vdwr1 = 0.229061;
				mass1 = 51.6;			// 1.00
				
				vdwr2 = 0.179623;
				mass2 = 51.6;			// 1.00
				break;
				
				case 'Q':						// 128.2
				vdwr1 = 0.226117;
				mass1 = 64.1;			// 1.00
				
				vdwr2 = 0.204211;
				mass2 = 64.1;			// 1.00
				break;
				
				case 'E':						// 128.7
				vdwr1 = 0.222040;
				mass1 = 64.3;			// 1.00
				
				vdwr2 = 0.191868;
				mass2 = 64.3;			// 1.00
				break;
				
				case 'G':						// 57.1
				vdwr1 = 0.173764;
				mass1 = 57.1;
				break;
				
				case 'H':						// 137.7
				vdwr1 = 0.229728;
				mass1 = 68.9;			// 1.00
				
				vdwr2 = 0.213733;
				mass2 = 68.9;			// 1.00
				break;
				
				case 'I':						// 113.1
				vdwr1 = 0.224550;
				mass1 = 56.6;			// 1.00
				
				vdwr2 = 0.180242;
				mass2 = 56.6;			// 1.00
				break;
				
				case 'L':						// 113.1
				vdwr1 = 0.220761;
				mass1 = 56.6;			// 1.00
				
				vdwr2 = 0.182951;
				mass2 = 56.6;			// 1.00
				break;
				
				case 'K':						// 129.2
				vdwr1 = 0.222174;
				mass1 = 49.7;			// 1.25
				
				vdwr2 = 0.184924;
				mass2 = 39.8;			// 1.00
				
				vdwr3 = 0.199069;
				mass3 = 39.8;			// 1.00
				break;
				
				case 'M':						// 131.2
				vdwr1 = 0.230562;
				mass1 = 65.6;			// 1.00
				
				vdwr2 = 0.221798;
				mass2 = 65.6;			// 1.00
				break;
				
				case 'F':						// 147.2
				vdwr1 = 0.224414;
				mass1 = 68.5;			// 1.00
				
				vdwr2 = 0.198277;
				mass2 = 78.7;			// 1.15
				break;
				
				case 'P':						// 97.1
				vdwr1 = 0.210783;
				mass1 = 97.1;
				break;
				
				case 'S':						// 87.1
				vdwr1 = 0.204505;
				mass1 = 87.1;
				break;
				
				case 'T':						// 101.1
				vdwr1 = 0.214132;
				mass1 = 101.1;
				break;
				
				case 'W':						// 188.2
				vdwr1 = 0.232409;
				mass1 = 62.7;			// 1.00
				
				vdwr2 = 0.233762;
				mass2 = 62.7;			// 1.00
				
				vdwr3 = 0.225098;
				mass3 = 62.7;			// 1.00
				break;
				
				case 'Y':						// 163.2
				vdwr1 = 0.224633;
				mass1 = 72.5;			// 1.00
				
				vdwr2 = 0.198270;
				mass2 = 90.6;			// 1.25
				break;
				
				case 'V':						// 99.1
				vdwr1 = 0.213718;
				mass1 = 99.1;
				break;
				
				default:
				cout << "BUG: unknown residue " << symbol_sf << " at setup1_sf ctor." << endl;
				exit(EXIT_FAILURE);
			}
			
			if (atmr_sf[0] != NULL)
			{
				for (i32u n3 = 0;n3 < GetModel()->cs_vector.size();n3++)
				{
					fGL x = crd_sf[0][n3 * 3 + 0];
					fGL y = crd_sf[0][n3 * 3 + 1];
					fGL z = crd_sf[0][n3 * 3 + 2];
					
					if (convert_crd) atmr_sf[0]->SetCRD(n3, x, y, z);
				}
				
				if (vdwr1 < 0.0 || mass1 < 0.0) { cout << "bad values for atom #1" << endl; exit(EXIT_FAILURE); }
				
				atmr_sf[0]->vdwr = vdwr1;
				atmr_sf[0]->mass = mass1;
			}
			
			if (atmr_sf[1] != NULL)
			{
				for (i32u n3 = 0;n3 < GetModel()->cs_vector.size();n3++)
				{
					fGL x = crd_sf[1][n3 * 3 + 0];
					fGL y = crd_sf[1][n3 * 3 + 1];
					fGL z = crd_sf[1][n3 * 3 + 2];
					
					if (convert_crd) atmr_sf[1]->SetCRD(n3, x, y, z);
				}
				
				if (vdwr2 < 0.0 || mass2 < 0.0) { cout << "bad values for atom #2" << endl; exit(EXIT_FAILURE); }
				
				atmr_sf[1]->vdwr = vdwr2;
				atmr_sf[1]->mass = mass2;
			}
			
			if (atmr_sf[2] != NULL)
			{
				for (i32u n3 = 0;n3 < GetModel()->cs_vector.size();n3++)
				{
					fGL x = crd_sf[2][n3 * 3 + 0];
					fGL y = crd_sf[2][n3 * 3 + 1];
					fGL z = crd_sf[2][n3 * 3 + 2];
					
					if (convert_crd) atmr_sf[2]->SetCRD(n3, x, y, z);
				}
				
				if (vdwr3 < 0.0 || mass3 < 0.0) { cout << "bad values for atom #3" << endl; exit(EXIT_FAILURE); }
				
				atmr_sf[2]->vdwr = vdwr3;
				atmr_sf[2]->mass = mass3;
			}
		}
	}
	
//	cout << "loc_varind_counter = " << loc_varind_counter << endl;		// this MUST match...
//	int stop; cin >> stop;							// to GetSFAtomCount()!!!
	
	delete[] crd_sf[0];
	delete[] crd_sf[1];
	delete[] crd_sf[2];
	
	UpdateAtomFlags();	// THE NEW WAY!!!!!
	
	// disulphide bridges...
	
	for (i32u n1 = 0;n1 < ci_vector.size();n1++)
	{
		iter_al range1a[2];
		GetModel()->GetRange(1, n1, range1a);
		
		// intra-chain ones...
		
		vector<i32s> cys_data1;
		vector<atom *> cys_ref1;
		
		for (i32s n2 = 0;n2 < ci_vector[n1].length;n2++)
		{
			if (ci_vector[n1].sequence[n2] == 'C')
			{
				bool flag = true;
				
				iter_al range1b[2];
				GetModel()->GetRange(2, range1a, n2, range1b);
				if (range1b[0] == range1b[1]) flag = false;
				
				iter_al it1 = range1b[0];
				while (it1 != range1b[1] && ((* it1).builder_res_id & 0xFF) != 0x21) it1++;
				if (it1 == range1b[1]) flag = false;
				
				if (flag)
				{
					cys_data1.push_back(n2);
					cys_ref1.push_back(& (* it1));
				}
			}
		}
		
		for (i32s n2 = 0;n2 < ((i32s) cys_ref1.size()) - 1;n2++)
		{
			for (i32s n3 = n2 + 1;n3 < (i32s) cys_ref1.size();n3++)
			{
				bond tb = bond(cys_ref1[n2], cys_ref1[n3], bondtype('S'));
				iter_bl it1 = find(GetModel()->bond_list.begin(), GetModel()->bond_list.end(), tb);
				if (it1 != GetModel()->bond_list.end())
				{
					sf_dsb newdsb;
					newdsb.chn[0] = n1; newdsb.res[0] = cys_data1[n2];
					newdsb.chn[1] = n1; newdsb.res[1] = cys_data1[n3];
					dsb_vector.push_back(newdsb);
				}
			}
		}
		
		// interchain ones...
		
		for (i32u n2 = n1 + 1;n2 < ci_vector.size();n2++)
		{
			iter_al range2a[2];
			GetModel()->GetRange(1, n2, range2a);
			
			vector<i32s> cys_data2;
			vector<atom *> cys_ref2;
			
			for (i32s n3 = 0;n3 < ci_vector[n2].length;n3++)
			{
				if (ci_vector[n2].sequence[n3] == 'C')
				{
					bool flag = true;
					
					iter_al range2b[2];
					GetModel()->GetRange(2, range2a, n3, range2b);
					if (range2b[0] == range2b[1]) flag = false;
					
					iter_al it1 = range2b[0];
					while (it1 != range2b[1] && ((* it1).builder_res_id & 0xFF) != 0x21) it1++;
					if (it1 == range2b[1]) flag = false;
					
					if (flag)
					{
						cys_data2.push_back(n3);
						cys_ref2.push_back(& (* it1));
					}
				}
			}
			
			for (i32u n3 = 0;n3 < cys_ref1.size();n3++)
			{
				for (i32u n4 = 0;n4 < cys_ref2.size();n4++)
				{
					bond tb = bond(cys_ref1[n3], cys_ref2[n4], bondtype('S'));
					iter_bl it1 = find(GetModel()->bond_list.begin(), GetModel()->bond_list.end(), tb);
					if (it1 != GetModel()->bond_list.end())
					{
						sf_dsb newdsb;
						newdsb.chn[0] = n1; newdsb.res[0] = cys_data1[n3];
						newdsb.chn[1] = n2; newdsb.res[1] = cys_data2[n4];
						dsb_vector.push_back(newdsb);
					}
				}
			}
		}
	}
	
	// secondary structure...
	
	// the default state is always loop. strands have the same places as in K&S.
	// helices are shifted: the smallest helical peptide is "4...." -> "LHHHL"!!!!!
	
	for (i32u n1 = 0;n1 < chn_vector.size();n1++)
	{
		if (ci_vector[n1].GetLength() != (i32s) chn_vector[n1].res_vector.size())
		{
			cout << "BUG: the sizes of chn_info and chn_vector differ!!!" << endl;
			exit(EXIT_FAILURE);
		}
		
		const char * state = ci_vector[n1].GetSecStrStates();
		for (i32s n2 = 0;n2 < (i32s) chn_vector[n1].res_vector.size();n2++)
		{
			chn_vector[n1].res_vector[n2].state = SF_STATE_LOOP;	// set the default!!!
			
			switch (state[n2])
			{
				case '4':
				chn_vector[n1].res_vector[n2].state = SF_STATE_HELIX4;
				break;
				
				case 'S':
				chn_vector[n1].res_vector[n2].state = SF_STATE_STRAND;
				break;
			}
		}
	}
}

setup1_sf::~setup1_sf(void)
{
	// un-hide all atoms and reset the atomic radii and masses to their default values...
	
	for (iter_al it1 = GetModel()->GetAtomsBegin();it1 != GetModel()->GetAtomsEnd();it1++)
	{
		(* it1).flags &= (~ATOMFLAG_IS_HIDDEN);
		
		(* it1).vdwr = (* it1).el.GetVDWRadius();
		(* it1).mass = (* it1).el.GetAtomicMass();
	}
}

void setup1_sf::UpdateAtomFlags(void)
{
	// first hide all atoms, then later un-hide what is needed...
	// first hide all atoms, then later un-hide what is needed...
	// first hide all atoms, then later un-hide what is needed...
	
	for (iter_al it1 = GetModel()->GetAtomsBegin();it1 != GetModel()->GetAtomsEnd();it1++)
	{
		(* it1).flags |= ATOMFLAG_IS_HIDDEN;
	}
	
	// handle chains...
	
	for (i32u n1 = 0;n1 < chn_vector.size();n1++)
	{
		for (i32u n2 = 0;n2 < chn_vector[n1].res_vector.size();n2++)
		{
			for (i32s n3 = 0;n3 < chn_vector[n1].res_vector[n2].natm;n3++)
			{
				chn_vector[n1].res_vector[n2].atmr[n3]->flags |= ATOMFLAG_IS_SF_ATOM;
				chn_vector[n1].res_vector[n2].atmr[n3]->flags &= (~ATOMFLAG_IS_HIDDEN);
			}
		}
	}
	
	// handle solvent molecules...
	
	for (iter_al it1 = GetModel()->GetAtomsBegin();it1 != GetModel()->GetAtomsEnd();it1++)
	{
		if ((* it1).el.GetAtomicNumber() != 8) continue;		// assume H2O!!!
		if (!((* it1).flags & ATOMFLAG_IS_SOLVENT_ATOM)) continue;	// assume H2O!!!
		
		(* it1).flags |= ATOMFLAG_IS_SF_ATOM;
		(* it1).flags &= (~ATOMFLAG_IS_HIDDEN);
		
		(* it1).vdwr = 0.155;
		(* it1).mass = 18.016;
		
// TODO : now assumes that oxygen coords = water coords.

	}
}

void setup1_sf::GetReducedCRD(iter_al * iter, vector<i32s> & idv, fGL * crd, i32u cset)
{
	vector<atom *> refv;
	for (i32u n1 = 0;n1 < idv.size();n1++)
	{
		iter_al it2 = iter[0];
		while (it2 != iter[1] && ((* it2).builder_res_id & 0xFF) != idv[n1]) it2++;
		if (it2 != iter[1]) refv.push_back(& (* it2));
	}
	
	if (!refv.size()) { cout << "BUG: no atoms found!" << endl; exit(EXIT_FAILURE); }
	
	for (i32u n1 = 0;n1 < 3;n1++)
	{
		crd[n1] = 0.0;
		for (i32u n2 = 0;n2 < refv.size();n2++)
		{
			const fGL * cdata = refv[n2]->GetCRD(cset);
			crd[n1] += cdata[n1];
		}
		
		crd[n1] /= (f64) refv.size();
	}
}

void setup1_sf::StorePStatesToModel(eng1_sf * eng)
{
	if (!GetModel()->ref_civ) return;
	
	vector<chn_info> & ci_vector = (* GetModel()->ref_civ);
	if (chn_vector.size() != ci_vector.size())
	{
		cout << "ERROR : chain counts mismatch!" << endl;
		exit(EXIT_FAILURE);
	}
	
	for (i32u n1 = 0;n1 < chn_vector.size();n1++)
	{
		if (!chn_vector[n1].res_vector.size()) continue;	// empty chain -> nucleic acid.
		if (chn_vector[n1].res_vector.size() != (i32u) ci_vector[n1].length)
		{
			cout << "ERROR : chain lengths mismatch!" << endl;
			exit(EXIT_FAILURE);
		}
		
		// make the tables, if needed...
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		
		if (ci_vector[n1].p_state == NULL)
		{
			ci_vector[n1].p_state = new char[ci_vector[n1].length];
		}
		
		for (i32u n2 = 0;n2 < chn_vector[n1].res_vector.size();n2++)
		{
			bool flag_tc = false;	// check for terminal charges...
			if (n2 == 0 || ((i32s) n2) == ((i32s) chn_vector[n1].res_vector.size()) - 1)
			{
				fGL tmpc = chn_vector[n1].res_vector[n2].atmr[0]->charge;
				if (tmpc < -0.5 || tmpc > +0.5) flag_tc = true;
			}
			
			int charge = 0;
			for (i32s n3 = 0;n3 < chn_vector[n1].res_vector[n2].natm;n3++)
			{
				fGL tmpc = chn_vector[n1].res_vector[n2].atmr[n3]->charge;
				if (tmpc < -0.5) charge -= 1; if (tmpc > +0.5) charge += 1;
			}
			
			// store the information...
			// ^^^^^^^^^^^^^^^^^^^^^^^^
			
			char flags = abs(charge);
			if (charge < 0) flags |= PSTATE_SIGN_NEGATIVE;
			else flags |= PSTATE_SIGN_POSITIVE;
			
			if (flag_tc) flags |= PSTATE_CHARGED_TERMINAL;
			
			ci_vector[n1].p_state[n2] = flags;
		}
	}
}

i32u setup1_sf::static_GetEngineCount(void)
{
	return 1;
}

i32u setup1_sf::static_GetEngineIDNumber(i32u eng_index)
{
	if (eng_index < static_GetEngineCount()) return 0x01;		// 0x01 is the default so far (we have only a single class).
	else return (i32u) NOT_DEFINED;		// error code...
}

const char * setup1_sf::static_GetEngineName(i32u eng_index)
{
	static char name[] = "simplified forcefield";
	
	if (eng_index < static_GetEngineCount()) return name;
	else return NULL;	// error code...
}

const char * setup1_sf::static_GetClassName(void)
{
	static char cn[] = "allsf";
	return cn;
}

i32u setup1_sf::GetEngineCount(void)
{
	return static_GetEngineCount();
}

i32u setup1_sf::GetEngineIDNumber(i32u eng_index)
{
	return static_GetEngineIDNumber(eng_index);
}

const char * setup1_sf::GetEngineName(i32u eng_index)
{
	return static_GetEngineName(eng_index);
}

const char * setup1_sf::GetClassName(void)
{
	return static_GetClassName();
}

engine * setup1_sf::CreateEngineByIndex(i32u eng_index)
{
	if (eng_index >= GetEngineCount())
	{
		cout << "setup1_sf::CreateEngineByIndex() failed!" << endl;
		return NULL;
	}
	
	GetModel()->use_periodic_boundary_conditions = false;
if (GetModel()->use_boundary_potential) GetModel()->Message("use_boundary_potential = TRUE");
	
	GetModel()->UpdateIndex();
	UpdateSetupInfo();
	
//	return new eng1_sf(this, 1, true, false);	// explicit...
//	return new eng1_sf(this, 1, false, true);	// implicit...
	return new eng1_sf(this, 1, exp_solv, !exp_solv);
}

/*################################################################################################*/

// here we are dependent on secondary structure constraints -> if secondary structure is modified,
// the engine class must be discarded and a new one must be created to take effects into account...

	// or take a closer look to CopyCRD() ?!?!?!? well this is not that urgent problem.

// IMPORTANT!!! constraints are no longer a part of energy calculations (since they practically affect them,
// due to rounding errors)!!! you have to add constraints to energy to get the total energy!!!

eng1_sf::eng1_sf(setup * p1, i32u p2, bool esf, bool isf) : engine(p1, p2), engine_mbp(p1, p2)
{
	use_explicit_solvent = esf;
	use_implicit_solvent = isf;
if (use_explicit_solvent) GetSetup()->GetModel()->Message("using EXPLICIT solvent");
if (use_implicit_solvent) GetSetup()->GetModel()->Message("using IMPLICIT solvent");
if (use_explicit_solvent && use_implicit_solvent)
{
	cout << "sorry, this is not yet supported!" << endl;
	exit(EXIT_FAILURE);
}
	
	// also check engine::bp_center!!!
	// also check engine::bp_center!!!
	// also check engine::bp_center!!!
	bp_fc_solute = 5000.0;		// 50 kJ/(mol*Å^2) = 5000 kJ/(mol*(nm)^2)
	bp_fc_solvent = 12500.0;	// 125 kJ/(mol*Å^2) = 12500 kJ/(mol*(nm)^2)
	
	if (use_simple_bp)
	{
		cout << "use_simple_bp ; ";
		cout << bp_radius_solute << " " << bp_fc_solute << " ; ";
		cout << bp_radius_solvent << " " << bp_fc_solvent << endl;
	}
	
	// create the local-to-global lookup table...
	// create the local-to-global lookup table...
	// create the local-to-global lookup table...
	
	l2g_sf = new i32u[GetSetup()->GetSFAtomCount()];
	
	atom ** atmtab = GetSetup()->GetSFAtoms();
	atom ** glob_atmtab = GetSetup()->GetAtoms();
	for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount();n1++)
	{
		i32s index = 0;
		while (index < GetSetup()->GetAtomCount())
		{
			if (atmtab[n1] == glob_atmtab[index]) break;
			else index++;
		}
		
		if (index >= GetSetup()->GetAtomCount())
		{
			cout << "BUG: eng1_sf ctor failed to create the l2g lookup table." << endl;
			exit(EXIT_FAILURE);
		}
		
		l2g_sf[n1] = index;
	}
	
	// various initialization tasks...
	// various initialization tasks...
	// various initialization tasks...
	
	setup1_sf * mysu = dynamic_cast<setup1_sf *>(GetSetup());
	if (!mysu) { cout << "BUG: cast to setup1_sf failed." << endl; exit(EXIT_FAILURE); }
	myprm = & mysu->prm;
	
	index_chn = new i32s[GetSetup()->GetSFAtomCount()];
	index_res = new i32s[GetSetup()->GetSFAtomCount()];
	
	num_solvent = 0;
	
	for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount();n1++)
	{
		bool found = false;
		
		for (i32u cc = 0;cc < mysu->chn_vector.size();cc++)
		{
			for (i32u rc = 0;rc < mysu->chn_vector[cc].res_vector.size();rc++)
			{
				for (i32s ac = 0;ac < mysu->chn_vector[cc].res_vector[rc].natm;ac++)
				{
					if (mysu->chn_vector[cc].res_vector[rc].atmr[ac] != atmtab[n1]) continue;
					
					index_chn[n1] = cc;
					index_res[n1] = rc;
					
					bool has_valid_varind = true;
					if (atmtab[n1]->varind < 0) has_valid_varind = false;
					if (atmtab[n1]->varind >= GetSetup()->GetAtomCount()) has_valid_varind = false;
					if (!has_valid_varind)
					{
						cout << "ERROR : invalid varind " << atmtab[n1]->varind << " was found!!!" << endl;
						exit(EXIT_FAILURE);
					}
					
					if (n1 != mysu->chn_vector[cc].res_vector[rc].loc_varind[ac])
					{
						cout << "ERROR : an incorrect loc_varind was detected!!!" << endl;
						exit(EXIT_FAILURE);
					}
					
					if (num_solvent != 0)
					{
						cout << "ERROR : ordering error : solvent atoms last!" << endl;
						exit(EXIT_FAILURE);
					}
					
					found = true;
					break;
				}
				
				if (found) break;	// not necessary; an optimization for speed.
			}
			
			if (found) break;	// not necessary; an optimization for speed.
		}
		
		if (!found)	// if no match was found, this must be a solvent molecule...
		{
			if (!(atmtab[n1]->flags & ATOMFLAG_IS_SOLVENT_ATOM))
			{
				cout << "ERROR : an unknown (non-solvent) atom was found!!!" << endl;
				exit(EXIT_FAILURE);
			}
			
			bool has_valid_varind = true;
			if (atmtab[n1]->varind < 0) has_valid_varind = false;
			if (atmtab[n1]->varind >= GetSetup()->GetAtomCount()) has_valid_varind = false;
			if (!has_valid_varind)
			{
				cout << "ERROR : invalid varind " << atmtab[n1]->varind << " was found!!!" << endl;
				exit(EXIT_FAILURE);
			}
			
			index_chn[n1] = NOT_DEFINED;
			index_res[n1] = NOT_DEFINED;
			num_solvent++;
		}
	}
	
	if (use_explicit_solvent && num_solvent == 0)
	{
		cout << "ERROR : explicit solvent requested but no solvent atoms detected!" << endl;
		exit(EXIT_FAILURE);
	}
	
	constraints = 0.0;
	for (i32u n1 = 0;n1 < mysu->chn_vector.size();n1++)
	{
		for (i32u n2 = 0;n2 < mysu->chn_vector[n1].res_vector.size();n2++)
		{
			bool is_helix4 = false;
			if ((n2 - 1) >= 0 && mysu->chn_vector[n1].res_vector[n2 - 1].state == SF_STATE_HELIX4) is_helix4 = true;
			if ((n2 - 2) >= 0 && mysu->chn_vector[n1].res_vector[n2 - 2].state == SF_STATE_HELIX4) is_helix4 = true;
			if ((n2 - 3) >= 0 && mysu->chn_vector[n1].res_vector[n2 - 3].state == SF_STATE_HELIX4) is_helix4 = true;
		
			if (is_helix4)
			{
				switch (mysu->chn_vector[n1].res_vector[n2].symbol)
				{
					case 'A':	constraints += +1.446e+01; break;
					case 'R':	constraints += -6.833e+00; break;
					case 'N':	constraints += +9.452e+00; break;
					case 'D':	constraints += +5.893e+00; break;
					case 'C':	constraints += +1.779e+00; break;
					case 'Q':	constraints += -1.667e+01; break;
					case 'E':	constraints += +7.172e+00; break;
					case 'G':	constraints += +1.646e+01; break;
					case 'H':	constraints += +7.052e+00; break;
					case 'I':	constraints += -1.236e+01; break;
					case 'L':	constraints += +1.775e-01; break;
					case 'K':	constraints += -8.890e-01; break;
					case 'M':	constraints += +4.644e+00; break;
					case 'F':	constraints += +5.412e+00; break;
					case 'P':	constraints += -3.191e+00; break;
					case 'S':	constraints += +1.971e+01; break;
					case 'T':	constraints += +4.572e+00; break;
					case 'W':	constraints += -6.210e+00; break;
					case 'Y':	constraints += -1.238e+01; break;
					case 'V':	constraints += -1.009e+01; break;
					
					default:
					cout << "BUG: unknown residue (helix) : " << mysu->chn_vector[n1].res_vector[n2].symbol << endl;
					exit(EXIT_FAILURE);
				}
			}
			
			if (mysu->chn_vector[n1].res_vector[n2].state == SF_STATE_STRAND)
			{
				switch (mysu->chn_vector[n1].res_vector[n2].symbol)
				{
					case 'A':	constraints += -8.465e+00; break;
					case 'R':	constraints += -7.428e+00; break;
					case 'N':	constraints += -1.505e+01; break;
					case 'D':	constraints += -9.706e+00; break;
					case 'C':	constraints += -9.969e+00; break;
					case 'Q':	constraints += -8.787e+00; break;
					case 'E':	constraints += -9.782e+00; break;
					case 'G':	constraints += -8.186e+00; break;
					case 'H':	constraints += -1.019e+01; break;
					case 'I':	constraints += -1.172e+01; break;
					case 'L':	constraints += -1.168e+01; break;
					case 'K':	constraints += -6.592e+00; break;
					case 'M':	constraints += -1.007e+01; break;
					case 'F':	constraints += -1.414e+01; break;
					case 'P':	constraints += -9.055e+00; break;
					case 'S':	constraints += -7.650e+00; break;
					case 'T':	constraints += -1.405e+01; break;
					case 'W':	constraints += -9.975e+00; break;
					case 'Y':	constraints += -1.431e+01; break;
					case 'V':	constraints += -1.285e+01; break;
					
					default:
					cout << "BUG: unknown residue (strand) : " << mysu->chn_vector[n1].res_vector[n2].symbol << endl;
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	
	tmp_vartab = NULL;
	tmp_parames = NULL;
	tmp_paramsa1 = NULL;
	tmp_paramsa2 = NULL;
	tmp_newpKa = NULL;
	
	mass = new f64[GetSetup()->GetSFAtomCount()];
	vdwr = new f64[GetSetup()->GetSFAtomCount()];
	charge1 = new f64[GetSetup()->GetSFAtomCount()];
	charge2 = new f64[GetSetup()->GetSFAtomCount()];
	
	for (i32u n1 = 0;n1 < LAYERS;n1++)
	{
		vdwr1[n1] = new f64[GetSetup()->GetSFAtomCount() - num_solvent];
		vdwr2[n1] = new f64[GetSetup()->GetSFAtomCount() - num_solvent];
		sasaE[n1] = new f64[GetSetup()->GetSFAtomCount() - num_solvent];
		
		solv_exp[n1] = new fGL[GetSetup()->GetSFAtomCount() - num_solvent];
	}
	
	for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount();n1++)
	{
		mass[n1] = atmtab[n1]->mass;
		vdwr[n1] = atmtab[n1]->vdwr * myprm->vdwrad;
		
		charge1[n1] = 0.0;		// SetupCharges() will handle this...
		charge2[n1] = 0.0;		// SetupCharges() will handle this...
	}
	
	for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount() - num_solvent;n1++)
	{
		for (i32u n2 = 0;n2 < LAYERS;n2++)
		{
			vdwr1[n2][n1] = vdwr[n1] + ((f64) (n2 + 1)) * myprm->solvrad;
			vdwr2[n2][n1] = vdwr1[n2][n1] * vdwr1[n2][n1];
			
			sasaE[n2][n1] = 0.0;	// SetupCharges() will handle this...
		}
	}
cout << "init ok; natm = " << GetSetup()->GetSFAtomCount() << endl;

/*##############################################*/
/*##############################################*/

	// main-chain t1-terms: direction is always from the N- to the C-terminal.
	//                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	for (i32u n1 = 0;n1 < mysu->chn_vector.size();n1++)
	{
		for (i32s n2 = 0;n2 < ((i32s) mysu->chn_vector[n1].res_vector.size()) - 1;n2++)
		{
			sf_bt1 newbt1;
			newbt1.atmi[0] = mysu->chn_vector[n1].res_vector[n2 + 0].loc_varind[0];
			newbt1.atmi[1] = mysu->chn_vector[n1].res_vector[n2 + 1].loc_varind[0];
			
			bool proline = (mysu->chn_vector[n1].res_vector[n2 + 1].symbol == 'P');
			newbt1.opt = (proline ? 0.352 : 0.38126);	// assume 1/3 X-Pro cis...
			
			newbt1.fc = 51.7e+03;
			
			bt1_vector.push_back(newbt1);
		}
	}
cout << "main bt1 ok; n = " << bt1_vector.size() << endl;

/*##############################################*/
/*##############################################*/

	// main-chain t2-terms: add like t1-terms -> directions for the t1-terms
	// will always be FALSE for the first one and TRUE for the second one.
	//                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	for (i32s n1 = 0;n1 < ((i32s) bt1_vector.size()) - 1;n1++)
	{
		i32s test1 = bt1_vector[n1].GetIndex(0, false);
		i32s test2 = bt1_vector[n1 + 1].GetIndex(0, true);
		
		if (test1 == test2)
		{
			sf_bt2 newbt2;
			newbt2.index1[0] = n1; newbt2.dir1[0] = false;
			newbt2.index1[1] = n1 + 1; newbt2.dir1[1] = true;
			
			newbt2.atmi[0] = bt1_vector[newbt2.index1[0]].GetIndex(1, newbt2.dir1[0]);
			newbt2.atmi[1] = bt1_vector[newbt2.index1[0]].GetIndex(0, newbt2.dir1[0]);
			newbt2.atmi[2] = bt1_vector[newbt2.index1[1]].GetIndex(1, newbt2.dir1[1]);
			
			i32s indc = index_chn[newbt2.atmi[1]];
			i32s indr = index_res[newbt2.atmi[1]];
			
			newbt2.ttype = TTYPE_LOOP;
			
			if ((indr - 1) >= 0 && mysu->chn_vector[indc].res_vector[indr - 1].state == SF_STATE_HELIX4) newbt2.ttype = TTYPE_HELIX;
			if ((indr - 2) >= 0 && mysu->chn_vector[indc].res_vector[indr - 2].state == SF_STATE_HELIX4) newbt2.ttype = TTYPE_HELIX;
			if ((indr - 3) >= 0 && mysu->chn_vector[indc].res_vector[indr - 3].state == SF_STATE_HELIX4) newbt2.ttype = TTYPE_HELIX;
			
			if (mysu->chn_vector[indc].res_vector[indr].state == SF_STATE_STRAND) newbt2.ttype = TTYPE_STRAND;
			
			switch (newbt2.ttype)
			{
				case TTYPE_HELIX:
				newbt2.opt = -0.0712161;			// 94.1 deg
				newbt2.fc[0] = 181.7 * myprm->wang;
				newbt2.fc[1] = mysu->prm.wrep;
				break;
				
				case TTYPE_STRAND:
				newbt2.opt = -0.543596;				// 122.9 deg
				newbt2.fc[0] = 40.12 * myprm->wang;
				newbt2.fc[1] = mysu->prm.wrep;
				break;
				
				default:	// this is for TTYPE_LOOP...
				newbt2.opt = -0.319972;				// 108.7 deg
				newbt2.fc[0] = 35.75 * myprm->wang;
				newbt2.fc[1] = mysu->prm.wrep;
				break;
			}
			
			bt2_vector.push_back(newbt2);
		}
	}
cout << "main bt2 ok; n = " << bt2_vector.size() << endl;
	
/*##############################################*/
/*##############################################*/

	// main-chain t3-terms: again add like t1-terms -> both t2-terms will
	// always have directions TRUE+TRUE -> implementation can take care of them.
	//                        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	for (i32s n1 = 0;n1 < ((i32s) bt2_vector.size()) - 1;n1++)
	{
		i32s test1i = bt2_vector[n1].index1[1];
		bool test1d = bt2_vector[n1].dir1[1];
		
		i32s test2i = bt2_vector[n1 + 1].index1[0];
		bool test2d = bt2_vector[n1 + 1].dir1[0];
		
		if (test1i == test2i && test1d != test2d)
		{
			sf_bt3 newbt3;
			newbt3.index2[0] = n1; newbt3.index2[1] = n1 + 1;
			
			newbt3.index1[0] = bt2_vector[newbt3.index2[0]].index1[0];
			newbt3.dir1[0] = bt2_vector[newbt3.index2[0]].dir1[0];
			
			newbt3.index1[1] = bt2_vector[newbt3.index2[0]].index1[1];
			newbt3.dir1[1] = bt2_vector[newbt3.index2[0]].dir1[1];
			
			newbt3.index1[2] = bt2_vector[newbt3.index2[1]].index1[0];
			newbt3.dir1[2] = bt2_vector[newbt3.index2[1]].dir1[0];
			
			newbt3.index1[3] = bt2_vector[newbt3.index2[1]].index1[1];
			newbt3.dir1[3] = bt2_vector[newbt3.index2[1]].dir1[1];
			
			newbt3.atmi[0] = bt1_vector[newbt3.index1[0]].GetIndex(1, newbt3.dir1[0]);
			newbt3.atmi[1] = bt1_vector[newbt3.index1[0]].GetIndex(0, newbt3.dir1[0]);
			newbt3.atmi[2] = bt1_vector[newbt3.index1[3]].GetIndex(0, newbt3.dir1[3]);
			newbt3.atmi[3] = bt1_vector[newbt3.index1[3]].GetIndex(1, newbt3.dir1[3]);
			
			ifstream file;
			
			i32s indc1 = index_chn[newbt3.atmi[1]];
			i32s indr1 = index_res[newbt3.atmi[1]];
			
			i32s indc2 = index_chn[newbt3.atmi[2]];
			i32s indr2 = index_res[newbt3.atmi[2]];
			
bool h4flag_0 = false; if ((indr1 - 0) >= 0 && mysu->chn_vector[indc1].res_vector[indr1 - 0].state == SF_STATE_HELIX4) h4flag_0 = true;
bool h4flag_1 = false; if ((indr1 - 1) >= 0 && mysu->chn_vector[indc1].res_vector[indr1 - 1].state == SF_STATE_HELIX4) h4flag_1 = true;
bool h4flag_2 = false; if ((indr1 - 2) >= 0 && mysu->chn_vector[indc1].res_vector[indr1 - 2].state == SF_STATE_HELIX4) h4flag_2 = true;
bool h4flag_3 = false; if ((indr1 - 3) >= 0 && mysu->chn_vector[indc1].res_vector[indr1 - 3].state == SF_STATE_HELIX4) h4flag_3 = true;
			
			bool ts1 = (mysu->chn_vector[indc1].res_vector[indr1].state == SF_STATE_STRAND);
			bool ts2 = (mysu->chn_vector[indc2].res_vector[indr2].state == SF_STATE_STRAND);
			
			// tor_type... tor_type... tor_type... tor_type... tor_type...
			// tor_type... tor_type... tor_type... tor_type... tor_type...
			// tor_type... tor_type... tor_type... tor_type... tor_type...
			
			newbt3.tor_ttype = TTYPE_LOOP;
			if (h4flag_1 || h4flag_2) newbt3.tor_ttype = TTYPE_HELIX;
			if (ts1 && ts2) newbt3.tor_ttype = TTYPE_STRAND;
			
			switch (newbt3.tor_ttype)
			{
				case TTYPE_HELIX:
				newbt3.tors[0] = +0.807732;
				newbt3.tors[1] = 31.28;		// OBSOLETE!!! * GetModel()->prm.wtor1;
				break;
				
				case TTYPE_STRAND:
				newbt3.tors[0] = -2.80308;
				newbt3.tors[1] = 7.209;		// OBSOLETE!!! * GetModel()->prm.wtor1;
				break;
				
				default:	// this is for TTYPE_LOOP...
				model::OpenLibDataFile(file, false, "param_sf/default/looptor.txt");
				
				while (true)
				{
					if (file.peek() == 'e')
					{
						cout << "ERROR : end of file was reached at looptor.txt" << endl;
						exit(EXIT_FAILURE);
					}
					
					char buffer[256]; char tp1; char tp2; file >> tp1 >> tp2;
					bool test1 = (tp1 != mysu->chn_vector[indc1].res_vector[indr1].symbol);
					bool test2 = (tp2 != mysu->chn_vector[indc2].res_vector[indr2].symbol);
					if (test1 || test2) file.getline(buffer, sizeof(buffer));
					else
					{
						f64 value;
						file >> value; newbt3.torc[0] = value * myprm->wtor1;
						file >> value; newbt3.torc[1] = value * myprm->wtor1;
						file >> value; newbt3.torc[2] = value * myprm->wtor1;
						file >> value; newbt3.tors[0] = value * myprm->wtor1;
						file >> value; newbt3.tors[1] = value * myprm->wtor1;
						file >> value; newbt3.tors[2] = value * myprm->wtor1;
						break;		// exit the loop...
					}
				}
				
				file.close();		// looptor.txt
				break;
			}
			
			// dip_type... dip_type... dip_type... dip_type... dip_type...
			// dip_type... dip_type... dip_type... dip_type... dip_type...
			// dip_type... dip_type... dip_type... dip_type... dip_type...
			
			newbt3.dip_ttype = TTYPE_LOOP;
			if (h4flag_1 || h4flag_2) newbt3.dip_ttype = TTYPE_HELIX;
			if (ts1 || ts2) newbt3.dip_ttype = TTYPE_STRAND;
			
			switch (newbt3.dip_ttype)
			{
				case TTYPE_HELIX:	break;
				case TTYPE_STRAND:	break;
				
				default:	// this is for TTYPE_LOOP...
				model::OpenLibDataFile(file, false, "param_sf/default/loopdip.txt");
				
				while (true)
				{
					if (file.peek() == 'e')
					{
						cout << "ERROR : end of file was reached at loopdip.txt" << endl;
						exit(EXIT_FAILURE);
					}
					
					char buffer[256]; char tp1; char tp2; file >> tp1 >> tp2;
					bool test1 = (tp1 != mysu->chn_vector[indc1].res_vector[indr1].symbol);
					bool test2 = (tp2 != mysu->chn_vector[indc2].res_vector[indr2].symbol);
					if (test1 || test2) file.getline(buffer, sizeof(buffer));
					else
					{
						f64 value;
						file >> value; newbt3.dipc[0] = value;
						file >> value; newbt3.dipc[1] = value;
						file >> value; newbt3.dipc[2] = value;
						file >> value; newbt3.dips[0] = value;
						file >> value; newbt3.dips[1] = value;
						file >> value; newbt3.dips[2] = value;
						file >> value; newbt3.dipk[0] = value;
						file >> value; newbt3.dipk[1] = value;
						break;		// exit the loop...
					}
				}
				
				file.close();		// loopdip.txt
				break;
			}
			
			// set the skip flag if this is a X-pro case !!!!!!!!!!!!!
			// set the skip flag if this is a X-pro case !!!!!!!!!!!!!
			// set the skip flag if this is a X-pro case !!!!!!!!!!!!!
			
			newbt3.skip = (mysu->chn_vector[indc2].res_vector[indr2].symbol == 'P');
			
			bt3_vector.push_back(newbt3);
		}
	}
cout << "main bt3 ok; n = " << bt3_vector.size() << endl;

/*##############################################*/
/*##############################################*/

	// side-chain t1-terms: directions are always MAIN -> SIDE1 -> SIDE2.
	//                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	i32u bt1_mark_side = bt1_vector.size();
	for (i32u n1 = 0;n1 < mysu->chn_vector.size();n1++)
	{
		for (i32u n2 = 0;n2 < mysu->chn_vector[n1].res_vector.size();n2++)
		{
			for (i32s n3 = 0;n3 < mysu->chn_vector[n1].res_vector[n2].natm - 1;n3++)
			{
				sf_bt1 newbt1;
				newbt1.atmi[0] = mysu->chn_vector[n1].res_vector[n2].loc_varind[n3];
				newbt1.atmi[1] = mysu->chn_vector[n1].res_vector[n2].loc_varind[n3 + 1];
				
				if (!n3)
				{
					switch (mysu->chn_vector[n1].res_vector[n2].symbol)
					{
						case 'R':
						newbt1.opt = 0.29852; newbt1.fc = 6.24e+03;
						break;
						
						case 'N':
						newbt1.opt = 0.25417; newbt1.fc = 38.5e+03;
						break;
						
						case 'D':
						newbt1.opt = 0.25474; newbt1.fc = 38.3e+03;
						break;
						
						case 'C':
						newbt1.opt = 0.27946; newbt1.fc = 32.6e+03;
						break;
						
						case 'Q':
						newbt1.opt = 0.29621; newbt1.fc = 5.28e+03;
						break;
						
						case 'E':
						newbt1.opt = 0.29818; newbt1.fc = 5.48e+03;
						break;
						
						case 'H':
						newbt1.opt = 0.35533; newbt1.fc = 21.4e+03;
						break;
						
						case 'I':
						newbt1.opt = 0.25265; newbt1.fc = 49.4e+03;
						break;
						
						case 'L':
						newbt1.opt = 0.26045; newbt1.fc = 32.9e+03;
						break;
						
						case 'K':
						newbt1.opt = 0.25549; newbt1.fc = 48.1e+03;
						break;
						
						case 'M':
						newbt1.opt = 0.35232; newbt1.fc = 1.24e+03;
						break;
						
						case 'F':
						newbt1.opt = 0.37875; newbt1.fc = 27.7e+03;
						break;
						
						case 'W':
						newbt1.opt = 0.34120; newbt1.fc = 19.9e+03;
						break;
						
						case 'Y':
						newbt1.opt = 0.37809; newbt1.fc = 19.6e+03;
						break;
						
						default:
						cout << "problems: unknown residue (S-T1-A) : " << mysu->chn_vector[n1].res_vector[n2].symbol << endl;
						exit(EXIT_FAILURE);
					}
				}
				else
				{
					switch (mysu->chn_vector[n1].res_vector[n2].symbol)
					{
						case 'R':
						newbt1.opt = 0.28736; newbt1.fc = 9.51e+03;
						break;
						
						case 'K':
						newbt1.opt = 0.25333; newbt1.fc = 35.4e+03;
						break;
						
						case 'W':
						newbt1.opt = 0.21051; newbt1.fc = 60.0e+03;	// modified fc!!!
						break;
						
						default:
						cout << "problems: unknown residue (S-T1-B) : " << mysu->chn_vector[n1].res_vector[n2].symbol << endl;
						exit(EXIT_FAILURE);
					}
				}
				
				bt1_vector.push_back(newbt1);
			}
		}
	}
cout << "side bt1 ok; n = " << bt1_vector.size() - bt1_mark_side << endl;

/*##############################################*/
/*##############################################*/

	// side-chain t2-terms: directions are always MAIN -> SIDE1 -> SIDE2.
	//                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	i32u bt2_mark_side = bt2_vector.size();
	for (i32s n1 = (i32s) bt1_mark_side;n1 < ((i32s) bt1_vector.size()) - 1;n1++)
	{
		i32s test1 = bt1_vector[n1].GetIndex(0, false);
		i32s test2 = bt1_vector[n1 + 1].GetIndex(0, true);
		
		i32s indc = index_chn[test1];
		i32s indr = index_res[test1];
		
		bool sidechain = (mysu->chn_vector[indc].res_vector[indr].atmr[0] != atmtab[test1]);
		if (test1 == test2 && sidechain)
		{
			sf_bt2 newbt2;
			newbt2.index1[0] = n1; newbt2.dir1[0] = false;
			newbt2.index1[1] = n1 + 1; newbt2.dir1[1] = true;
			
			newbt2.atmi[0] = bt1_vector[newbt2.index1[0]].GetIndex(1, newbt2.dir1[0]);
			newbt2.atmi[1] = bt1_vector[newbt2.index1[0]].GetIndex(0, newbt2.dir1[0]);
			newbt2.atmi[2] = bt1_vector[newbt2.index1[1]].GetIndex(1, newbt2.dir1[1]);
			
			newbt2.ttype = TTYPE_SIDE;
			
			switch (mysu->chn_vector[indc].res_vector[indr].symbol)
			{
				case 'R':
				newbt2.opt = -0.62392;
				newbt2.fc[0] = 14.7;
				newbt2.fc[1] = 0.0;
				break;
				
				case 'K':
				newbt2.opt = -0.85737;
				newbt2.fc[0] = 9.65;
				newbt2.fc[1] = 0.0;
				break;
				
				case 'W':
				newbt2.opt = -0.48517;
				newbt2.fc[0] = 68.5;
				newbt2.fc[1] = 0.0;
				break;
				
				default:
				cout << "problems: unknown residue (S-T2) : " << mysu->chn_vector[indc].res_vector[indr].symbol << endl;
				exit(EXIT_FAILURE);
			}
			
			bt2_vector.push_back(newbt2);
		}
	}
cout << "side bt2 ok; n = " << bt2_vector.size() - bt2_mark_side << endl;
	
/*##############################################*/
/*##############################################*/

	// t4-terms for all non-terminal residues with side-chains: just add
	// t1- and t2-subterms like defined before; all directions are TRUE.
	//                                          ^^^^^^^^^^^^^^^^^^^^^^^^
	
	for (i32u n1 = 0;n1 < mysu->chn_vector.size();n1++)
	{
		for (i32s n2 = 1;n2 < ((i32s) mysu->chn_vector[n1].res_vector.size()) - 1;n2++)
		{
			if (mysu->chn_vector[n1].res_vector[n2].natm > 1)
			{
				i32s index[2];
				index[0] = mysu->chn_vector[n1].res_vector[n2].loc_varind[0];
				index[1] = mysu->chn_vector[n1].res_vector[n2].loc_varind[1];
				
				sf_bt4 newbt4;
				newbt4.index1 = 0;	// find the first side-chain t1-term:
				while (newbt4.index1 < (i32s) bt1_vector.size())
				{
					bool test1 = (bt1_vector[newbt4.index1].atmi[0] == index[0]);
					bool test2 = (bt1_vector[newbt4.index1].atmi[1] == index[1]);
					if (test1 && test2) break; else newbt4.index1++;
				}
				
				if (newbt4.index1 >= (i32s) bt1_vector.size())
				{
					cout << "ERROR : bt4/index1 search failed!!!" << endl;
					exit(EXIT_FAILURE);
				}
				
				newbt4.index2 = 0;	// find the first t2-term (main-chain):
				while (newbt4.index2 < (i32s) bt2_vector.size())
				{
					bool test1 = (bt2_vector[newbt4.index2].atmi[1] == index[0]);
					if (test1) break; else newbt4.index2++;
				}
				
				if (newbt4.index2 >= (i32s) bt2_vector.size())
				{
					cout << "ERROR : bt4/index2 search failed!!!" << endl;
					exit(EXIT_FAILURE);
				}
				
				char symbol = mysu->chn_vector[n1].res_vector[n2].symbol;
				i32s state = mysu->chn_vector[n1].res_vector[n2].state;
				
// changed quickly 20050608 ; checkme!!!
	if ((n2 - 1) >= 0 && mysu->chn_vector[n1].res_vector[n2 - 1].state == SF_STATE_HELIX4) state = SF_STATE_HELIX4;
	if ((n2 - 2) >= 0 && mysu->chn_vector[n1].res_vector[n2 - 2].state == SF_STATE_HELIX4) state = SF_STATE_HELIX4;
	if ((n2 - 3) >= 0 && mysu->chn_vector[n1].res_vector[n2 - 3].state == SF_STATE_HELIX4) state = SF_STATE_HELIX4;
				
				switch (symbol)
				{
					case 'R':
					newbt4.opt = 0.82733; newbt4.fc = 78.42;
					
					switch (state)
					{
				case SF_STATE_HELIX4:
				newbt4.fscos[0] = +1.66715; newbt4.fscos[1] = +2.52406; newbt4.fscos[2] = -0.81090;
				newbt4.fssin[0] = -0.44044; newbt4.fssin[1] = -0.56621; newbt4.fssin[2] = +0.16812;
				break;
				
				case SF_STATE_STRAND:
				newbt4.fscos[0] = +2.81092; newbt4.fscos[1] = -2.18907; newbt4.fscos[2] = -1.31969;
				newbt4.fssin[0] = +0.80986; newbt4.fssin[1] = +0.21052; newbt4.fssin[2] = -0.07480;
				break;
				
				default:
				newbt4.fscos[0] = +1.84105; newbt4.fscos[1] = -0.34928; newbt4.fscos[2] = -1.37635;
				newbt4.fssin[0] = -1.07776; newbt4.fssin[1] = +0.50658; newbt4.fssin[2] = +0.43117;
					}
					break;
					
					case 'N':
					newbt4.opt = 0.79233; newbt4.fc = 135.9;
					
					switch (state)
					{
				case SF_STATE_HELIX4:
				newbt4.fscos[0] = +2.06084; newbt4.fscos[1] = +2.21810; newbt4.fscos[2] = -2.68616;
				newbt4.fssin[0] = -1.72997; newbt4.fssin[1] = +0.13154; newbt4.fssin[2] = +1.59189;
				break;
				
				case SF_STATE_STRAND:
				newbt4.fscos[0] = +3.12145; newbt4.fscos[1] = -3.27636; newbt4.fscos[2] = -3.00767;
				newbt4.fssin[0] = +0.96246; newbt4.fssin[1] = +0.62799; newbt4.fssin[2] = +0.05736;
				break;
				
				default:
				newbt4.fscos[0] = +1.74298; newbt4.fscos[1] = -0.79241; newbt4.fscos[2] = -2.61712;
				newbt4.fssin[0] = -1.03069; newbt4.fssin[1] = +0.19134; newbt4.fssin[2] = +1.25520;
					}
					break;
					
					case 'D':
					newbt4.opt = 0.79856; newbt4.fc = 136.2;
					
					switch (state)
					{
				case SF_STATE_HELIX4:
				newbt4.fscos[0] = +1.85269; newbt4.fscos[1] = +2.31000; newbt4.fscos[2] = -2.26981;
				newbt4.fssin[0] = -2.09388; newbt4.fssin[1] = -0.13391; newbt4.fssin[2] = +1.83416;
				break;
				
				case SF_STATE_STRAND:
				newbt4.fscos[0] = +3.39290; newbt4.fscos[1] = -3.94822; newbt4.fscos[2] = -2.65580;
				newbt4.fssin[0] = +1.70965; newbt4.fssin[1] = -0.36173; newbt4.fssin[2] = -0.43294;
				break;
				
				default:
				newbt4.fscos[0] = +1.59474; newbt4.fscos[1] = -0.74304; newbt4.fscos[2] = -2.43083;
				newbt4.fssin[0] = -0.82258; newbt4.fssin[1] = -0.02264; newbt4.fssin[2] = +1.50055;
					}
					break;
					
					case 'C':
					newbt4.opt = 0.77013; newbt4.fc = 134.1;
					
					switch (state)
					{
				case SF_STATE_HELIX4:
				newbt4.fscos[0] = +1.29742; newbt4.fscos[1] = +2.82739; newbt4.fscos[2] = -2.06328;
				newbt4.fssin[0] = -1.07293; newbt4.fssin[1] = +0.12694; newbt4.fssin[2] = +1.99976;
				break;
				
				case SF_STATE_STRAND:
				newbt4.fscos[0] = +2.17623; newbt4.fscos[1] = -2.82029; newbt4.fscos[2] = -3.48759;
				newbt4.fssin[0] = +0.45237; newbt4.fssin[1] = +0.60356; newbt4.fssin[2] = -0.52859;
				break;
				
				default:
				newbt4.fscos[0] = +1.39788; newbt4.fscos[1] = -0.52709; newbt4.fscos[2] = -2.47451;
				newbt4.fssin[0] = -0.95942; newbt4.fssin[1] = +0.51342; newbt4.fssin[2] = +0.98760;
					}
					break;
					
					case 'Q':
					newbt4.opt = 0.81477; newbt4.fc = 80.30;
					
					switch (state)
					{
				case SF_STATE_HELIX4:
				newbt4.fscos[0] = +1.71509; newbt4.fscos[1] = +1.93731; newbt4.fscos[2] = -0.76456;
				newbt4.fssin[0] = -1.01086; newbt4.fssin[1] = -1.05898; newbt4.fssin[2] = +0.48023;
				break;
				
				case SF_STATE_STRAND:
				newbt4.fscos[0] = +2.80955; newbt4.fscos[1] = -2.04764; newbt4.fscos[2] = -1.83362;
				newbt4.fssin[0] = +0.63332; newbt4.fssin[1] = +0.24795; newbt4.fssin[2] = -0.05034;
				break;
				
				default:
				newbt4.fscos[0] = +1.85194; newbt4.fscos[1] = -0.29906; newbt4.fscos[2] = -1.32592;
				newbt4.fssin[0] = -1.41994; newbt4.fssin[1] = +0.29379; newbt4.fssin[2] = +0.35692;
					}
					break;
					
					case 'E':
					newbt4.opt = 0.81811; newbt4.fc = 97.26;
					
					switch (state)
					{
				case SF_STATE_HELIX4:
				newbt4.fscos[0] = +1.44402; newbt4.fscos[1] = +1.94219; newbt4.fscos[2] = -0.80589;
				newbt4.fssin[0] = -1.01587; newbt4.fssin[1] = -1.09656; newbt4.fssin[2] = -0.07502;
				break;
				
				case SF_STATE_STRAND:
				newbt4.fscos[0] = +2.83636; newbt4.fscos[1] = -2.33477; newbt4.fscos[2] = -1.91387;
				newbt4.fssin[0] = +0.41781; newbt4.fssin[1] = -0.03389; newbt4.fssin[2] = +0.16658;
				break;
				
				default:
				newbt4.fscos[0] = +1.61321; newbt4.fscos[1] = -0.33262; newbt4.fscos[2] = -1.16010;
				newbt4.fssin[0] = -1.13465; newbt4.fssin[1] = -0.04564; newbt4.fssin[2] = +0.28664;
					}
					break;
					
					case 'H':
					newbt4.opt = 0.68790; newbt4.fc = 104.7;
					
					switch (state)
					{
				case SF_STATE_HELIX4:
				newbt4.fscos[0] = +2.53650; newbt4.fscos[1] = +2.73410; newbt4.fscos[2] = -3.45313;
				newbt4.fssin[0] = +0.20418; newbt4.fssin[1] = +0.76434; newbt4.fssin[2] = +2.08214;
				break;
				
				case SF_STATE_STRAND:
				newbt4.fscos[0] = +2.26449; newbt4.fscos[1] = -2.53277; newbt4.fscos[2] = -4.92778;
				newbt4.fssin[0] = +0.01335; newbt4.fssin[1] = +0.70585; newbt4.fssin[2] = -0.44344;
				break;
				
				default:
				newbt4.fscos[0] = +1.83546; newbt4.fscos[1] = -0.26818; newbt4.fscos[2] = -3.69531;
				newbt4.fssin[0] = -1.45726; newbt4.fssin[1] = +0.46062; newbt4.fssin[2] = +0.96875;
					}
					break;
					
					case 'I':
					newbt4.opt = 0.84384; newbt4.fc = 108.5;
					
					switch (state)
					{
				case SF_STATE_HELIX4:
				newbt4.fscos[0] = +0.36523; newbt4.fscos[1] = +2.54766; newbt4.fscos[2] = -1.29027;
				newbt4.fssin[0] = -2.97542; newbt4.fssin[1] = -0.24663; newbt4.fssin[2] = +2.89687;
				break;
				
				case SF_STATE_STRAND:
				newbt4.fscos[0] = +2.00042; newbt4.fscos[1] = -2.35794; newbt4.fscos[2] = -3.63292;
				newbt4.fssin[0] = -0.46656; newbt4.fssin[1] = +2.35817; newbt4.fssin[2] = -0.45473;
				break;
				
				default:
				newbt4.fscos[0] = +0.67483; newbt4.fscos[1] = -0.72266; newbt4.fscos[2] = -2.26144;
				newbt4.fssin[0] = -1.90307; newbt4.fssin[1] = +0.81662; newbt4.fssin[2] = +0.64670;
					}
					break;
					
					case 'L':
					newbt4.opt = 0.82350; newbt4.fc = 175.9;
					
					switch (state)
					{
				case SF_STATE_HELIX4:
				newbt4.fscos[0] = +3.84136; newbt4.fscos[1] = +3.32244; newbt4.fscos[2] = -1.22385;
				newbt4.fssin[0] = -1.23669; newbt4.fssin[1] = -0.52039; newbt4.fssin[2] = +1.13885;
				break;
				
				case SF_STATE_STRAND:
				newbt4.fscos[0] = +4.76904; newbt4.fscos[1] = -2.43156; newbt4.fscos[2] = -3.14965;
				newbt4.fssin[0] = +0.65700; newbt4.fssin[1] = +0.38307; newbt4.fssin[2] = -0.30443;
				break;
				
				default:
				newbt4.fscos[0] = +3.52823; newbt4.fscos[1] = +0.38402; newbt4.fscos[2] = -1.72298;
				newbt4.fssin[0] = -2.37699; newbt4.fssin[1] = -0.03600; newbt4.fssin[2] = +0.40297;
					}
					break;
					
					case 'K':
					newbt4.opt = 0.79880; newbt4.fc = 144.6;
					
					switch (state)
					{
				case SF_STATE_HELIX4:
				newbt4.fscos[0] = +1.91050; newbt4.fscos[1] = +2.79925; newbt4.fscos[2] = -1.45299;
				newbt4.fssin[0] = -0.47100; newbt4.fssin[1] = -0.31985; newbt4.fssin[2] = +1.06281;
				break;
				
				case SF_STATE_STRAND:
				newbt4.fscos[0] = +2.98681; newbt4.fscos[1] = -1.90303; newbt4.fscos[2] = -2.31907;
				newbt4.fssin[0] = +0.31173; newbt4.fssin[1] = +0.01677; newbt4.fssin[2] = -0.22673;
				break;
				
				default:
				newbt4.fscos[0] = +1.96145; newbt4.fscos[1] = +0.12819; newbt4.fscos[2] = -1.91454;
				newbt4.fssin[0] = -1.33542; newbt4.fssin[1] = +0.29320; newbt4.fssin[2] = +0.55337;
					}
					break;
					
					case 'M':
					newbt4.opt = 0.83147; newbt4.fc = 37.65;
					
					switch (state)
					{
				case SF_STATE_HELIX4:
				newbt4.fscos[0] = +1.88761; newbt4.fscos[1] = +0.43592; newbt4.fscos[2] = +0.43654;
				newbt4.fssin[0] = -0.59080; newbt4.fssin[1] = -0.24729; newbt4.fssin[2] = -0.97472;
				break;
				
				case SF_STATE_STRAND:
				newbt4.fscos[0] = +2.95738; newbt4.fscos[1] = -1.50891; newbt4.fscos[2] = -0.86063;
				newbt4.fssin[0] = +0.77162; newbt4.fssin[1] = +0.23999; newbt4.fssin[2] = +0.00057;
				break;
				
				default:
				newbt4.fscos[0] = +2.17883; newbt4.fscos[1] = -0.87283; newbt4.fscos[2] = -0.23810;
				newbt4.fssin[0] = -0.78868; newbt4.fssin[1] = +0.60167; newbt4.fssin[2] = +0.11778;
					}
					break;
					
					case 'F':
					newbt4.opt = 0.67581; newbt4.fc = 114.0;
					
					switch (state)
					{
				case SF_STATE_HELIX4:
				newbt4.fscos[0] = +3.73550; newbt4.fscos[1] = +3.12113; newbt4.fscos[2] = -3.65848;
				newbt4.fssin[0] = +0.46830; newbt4.fssin[1] = +1.08106; newbt4.fssin[2] = +1.64272;
				break;
				
				case SF_STATE_STRAND:
				newbt4.fscos[0] = +2.62637; newbt4.fscos[1] = -3.22908; newbt4.fscos[2] = -5.53782;
				newbt4.fssin[0] = -0.34849; newbt4.fssin[1] = +0.99799; newbt4.fssin[2] = -0.06089;
				break;
				
				default:
				newbt4.fscos[0] = +2.14212; newbt4.fscos[1] = -0.32047; newbt4.fscos[2] = -3.57612;
				newbt4.fssin[0] = -1.37620; newbt4.fssin[1] = +0.72569; newbt4.fssin[2] = +0.51568;
					}
					break;
					
					case 'W':
					newbt4.opt = 0.70518; newbt4.fc = 111.5;
					
					switch (state)
					{
				case SF_STATE_HELIX4:
				newbt4.fscos[0] = +2.41235; newbt4.fscos[1] = +2.56594; newbt4.fscos[2] = -3.14962;
				newbt4.fssin[0] = +0.41312; newbt4.fssin[1] = +0.98169; newbt4.fssin[2] = +1.69113;
				break;
				
				case SF_STATE_STRAND:
				newbt4.fscos[0] = +2.23143; newbt4.fscos[1] = -3.10454; newbt4.fscos[2] = -4.07617;
				newbt4.fssin[0] = +0.32407; newbt4.fssin[1] = +0.78458; newbt4.fssin[2] = -0.90925;
				break;
				
				default:
				newbt4.fscos[0] = +1.89729; newbt4.fscos[1] = -0.94277; newbt4.fscos[2] = -3.46953;
				newbt4.fssin[0] = -1.08009; newbt4.fssin[1] = +1.02104; newbt4.fssin[2] = +1.06076;
					}
					break;
					
					case 'Y':
					newbt4.opt = 0.67080; newbt4.fc = 106.7;
					
					switch (state)
					{
				case SF_STATE_HELIX4:
				newbt4.fscos[0] = +3.25507; newbt4.fscos[1] = +3.38981; newbt4.fscos[2] = -3.76346;
				newbt4.fssin[0] = +0.41418; newbt4.fssin[1] = +0.76818; newbt4.fssin[2] = +1.97564;
				break;
				
				case SF_STATE_STRAND:
				newbt4.fscos[0] = +2.36446; newbt4.fscos[1] = -2.66255; newbt4.fscos[2] = -5.36649;
				newbt4.fssin[0] = -0.19421; newbt4.fssin[1] = +0.99025; newbt4.fssin[2] = -0.25182;
				break;
				
				default:
				newbt4.fscos[0] = +2.07289; newbt4.fscos[1] = -0.22634; newbt4.fscos[2] = -3.81184;
				newbt4.fssin[0] = -1.35353; newbt4.fssin[1] = +0.74664; newbt4.fssin[2] = +0.72466;
					}
					break;
					
					default:
					cout << "problems: unknown residue (S-T4) : " << symbol << endl;
					exit(EXIT_FAILURE);
				}
				
				newbt4.fc *= myprm->wang;
				
				newbt4.fscos[0] *= myprm->wtor2;
				newbt4.fscos[1] *= myprm->wtor2;
				newbt4.fscos[2] *= myprm->wtor2; 
				
				newbt4.fssin[0] *= myprm->wtor2;
				newbt4.fssin[1] *= myprm->wtor2;
				newbt4.fssin[2] *= myprm->wtor2; 
				
				bt4_vector.push_back(newbt4);
			}
		}
	}
cout << "bt4 ok; n = " << bt4_vector.size() << endl;

/*##############################################*/
/*##############################################*/

	// dsb-terms, both bt1 and bt2...
	
	for (i32u n1 = 0;n1 < mysu->dsb_vector.size();n1++)
	{
		i32s index1[2];
		index1[0] = mysu->chn_vector[mysu->dsb_vector[n1].chn[0]].res_vector[mysu->dsb_vector[n1].res[0]].loc_varind[0];
		index1[1] = mysu->chn_vector[mysu->dsb_vector[n1].chn[0]].res_vector[mysu->dsb_vector[n1].res[0]].loc_varind[1];
		
		i32s ind1 = 0;
		while (ind1 < (i32s) bt1_vector.size())
		{
			bool test1 = (bt1_vector[ind1].atmi[0] == index1[0]);
			bool test2 = (bt1_vector[ind1].atmi[1] == index1[1]);
			if (test1 && test2) break;
			
			ind1++;
		}
		
		if (ind1 >= (i32s) bt1_vector.size())
		{
			cout << "DSB error #1" << endl;
			exit(EXIT_FAILURE);
		}
		
		i32s index2[2];
		index2[0] = mysu->chn_vector[mysu->dsb_vector[n1].chn[1]].res_vector[mysu->dsb_vector[n1].res[1]].loc_varind[0];
		index2[1] = mysu->chn_vector[mysu->dsb_vector[n1].chn[1]].res_vector[mysu->dsb_vector[n1].res[1]].loc_varind[1];
		
		i32s ind2 = 0;
		while (ind2 < (i32s) bt1_vector.size())
		{
			bool test1 = (bt1_vector[ind2].atmi[0] == index2[0]);
			bool test2 = (bt1_vector[ind2].atmi[1] == index2[1]);
			if (test1 && test2) break;
			
			ind2++;
		}
		
		if (ind2 >= (i32s) bt1_vector.size())
		{
			cout << "DSB error #2" << endl;
			exit(EXIT_FAILURE);
		}
		
		sf_bt1 newbt1;
		newbt1.atmi[0] = index1[1];
		newbt1.atmi[1] = index2[1];
		
		newbt1.opt = 0.203251; newbt1.fc = 60.0e+03;	// modified fc!!!
		
		i32s ind3 = bt1_vector.size();
		bt1_vector.push_back(newbt1);
		
		sf_bt2 newbt2;
		newbt2.ttype = TTYPE_BRIDGE;
		newbt2.opt = -0.236514;
		newbt2.fc[0] = 26.0;
		newbt2.fc[1] = 0.0;
		
		// first angle...
		
		newbt2.index1[0] = ind1; newbt2.dir1[0] = false;
		newbt2.index1[1] = ind3; newbt2.dir1[1] = true;
		
		newbt2.atmi[0] = bt1_vector[newbt2.index1[0]].GetIndex(1, newbt2.dir1[0]);
		newbt2.atmi[1] = bt1_vector[newbt2.index1[0]].GetIndex(0, newbt2.dir1[0]);
		newbt2.atmi[2] = bt1_vector[newbt2.index1[1]].GetIndex(1, newbt2.dir1[1]);
		
		bt2_vector.push_back(newbt2);
		
		// second angle...
		
		newbt2.index1[0] = ind2; newbt2.dir1[0] = false;
		newbt2.index1[1] = ind3; newbt2.dir1[1] = false;
		
		newbt2.atmi[0] = bt1_vector[newbt2.index1[0]].GetIndex(1, newbt2.dir1[0]);
		newbt2.atmi[1] = bt1_vector[newbt2.index1[0]].GetIndex(0, newbt2.dir1[0]);
		newbt2.atmi[2] = bt1_vector[newbt2.index1[1]].GetIndex(1, newbt2.dir1[1]);
		
		bt2_vector.push_back(newbt2);
	}
cout << "dsb ok." << endl;

/*##############################################*/
/*##############################################*/

	// terminal angles...
	
	for (i32u n1 = 0;n1 < mysu->chn_vector.size();n1++)
	{
		if (mysu->chn_vector[n1].res_vector[0].natm > 1)
		{
			i32s index[3];
			index[0] = mysu->chn_vector[n1].res_vector[0].loc_varind[1];
			index[1] = mysu->chn_vector[n1].res_vector[0].loc_varind[0];
			index[2] = mysu->chn_vector[n1].res_vector[1].loc_varind[0];
			
			i32s ind1 = 0;
			while (ind1 < (i32s) bt1_vector.size())
			{
				bool test1 = (bt1_vector[ind1].atmi[0] == index[1]);
				bool test2 = (bt1_vector[ind1].atmi[1] == index[0]);
				if (test1 && test2) break;
				
				ind1++;
			}
			
			if (ind1 >= (i32s) bt1_vector.size())
			{
				cout << "NT error #1" << endl;
				exit(EXIT_FAILURE);
			}
			
			i32s ind2 = 0;
			while (ind2 < (i32s) bt1_vector.size())
			{
				bool test1 = (bt1_vector[ind2].atmi[0] == index[1]);
				bool test2 = (bt1_vector[ind2].atmi[1] == index[2]);
				if (test1 && test2) break;
				
				ind2++;
			}
			
			if (ind2 >= (i32s) bt1_vector.size())
			{
				cout << "NT error #2" << endl;
				exit(EXIT_FAILURE);
			}
			
			sf_bt2 newbt2;
			newbt2.index1[0] = ind1; newbt2.dir1[0] = true;
			newbt2.index1[1] = ind2; newbt2.dir1[1] = true;
			
			newbt2.atmi[0] = bt1_vector[newbt2.index1[0]].GetIndex(1, newbt2.dir1[0]);
			newbt2.atmi[1] = bt1_vector[newbt2.index1[0]].GetIndex(0, newbt2.dir1[0]);
			newbt2.atmi[2] = bt1_vector[newbt2.index1[1]].GetIndex(1, newbt2.dir1[1]);
			
			newbt2.ttype = TTYPE_TERM;
			newbt2.opt = -0.328360;
			newbt2.fc[0] = 12.0;
			newbt2.fc[1] = myprm->wrep;
			
			bt2_vector.push_back(newbt2);
		}
		
		i32s sz = mysu->chn_vector[n1].res_vector.size();
		if (mysu->chn_vector[n1].res_vector[sz - 1].natm > 1)
		{
			i32s index[3];
			index[0] = mysu->chn_vector[n1].res_vector[sz - 1].loc_varind[1];
			index[1] = mysu->chn_vector[n1].res_vector[sz - 1].loc_varind[0];
			index[2] = mysu->chn_vector[n1].res_vector[sz - 2].loc_varind[0];
			
			i32s ind1 = 0;
			while (ind1 < (i32s) bt1_vector.size())
			{
				bool test1 = (bt1_vector[ind1].atmi[0] == index[1]);
				bool test2 = (bt1_vector[ind1].atmi[1] == index[0]);
				if (test1 && test2) break;
				
				ind1++;
			}
			
			if (ind1 >= (i32s) bt1_vector.size())
			{
				cout << "CT error #1" << endl;
				exit(EXIT_FAILURE);
			}
			
			i32s ind2 = 0;
			while (ind2 < (i32s) bt1_vector.size())
			{
				bool test1 = (bt1_vector[ind2].atmi[0] == index[2]);
				bool test2 = (bt1_vector[ind2].atmi[1] == index[1]);
				if (test1 && test2) break;
				
				ind2++;
			}
			
			if (ind2 >= (i32s) bt1_vector.size())
			{
				cout << "CT error #2" << endl;
				exit(EXIT_FAILURE);
			}
			
			sf_bt2 newbt2;
			newbt2.index1[0] = ind1; newbt2.dir1[0] = true;
			newbt2.index1[1] = ind2; newbt2.dir1[1] = false;
			
			newbt2.atmi[0] = bt1_vector[newbt2.index1[0]].GetIndex(1, newbt2.dir1[0]);
			newbt2.atmi[1] = bt1_vector[newbt2.index1[0]].GetIndex(0, newbt2.dir1[0]);
			newbt2.atmi[2] = bt1_vector[newbt2.index1[1]].GetIndex(1, newbt2.dir1[1]);
			
			newbt2.ttype = TTYPE_TERM;
			newbt2.opt = -0.339739;
			newbt2.fc[0] = 20.5;
			newbt2.fc[1] = myprm->wrep;
			
			bt2_vector.push_back(newbt2);
		}
	}
cout << "term ok." << endl;

/*##############################################*/
/*##############################################*/

	// nb1-terms...
	
	vector <sf_nonbonded_lookup> lookup;
	
	ifstream file;
	model::OpenLibDataFile(file, false, "param_sf/default/nonbonded.txt");
	
	while (file.peek() != 'e')
	{
		char s1; i32u a1;
		char s2; i32u a2;
		f64 opte;
		
		file >> s1 >> a1 >> s2 >> a2 >> opte;
		
		char buffer[256];
		file.getline(buffer, sizeof(buffer));
		
		sf_nonbonded_lookup newlu;
		newlu.s1 = s1; newlu.a1 = a1;
		newlu.s2 = s2; newlu.a2 = a2;
		newlu.opte = opte;
		
//	cout << "adding a new lookup record : " << s1 << " " << a1 << " " << s2 << " " << a2 << " " << opte << endl;
		
		lookup.push_back(newlu);
	}
	
	file.close();		// nonbonded.txt
	
	for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount() - 1;n1++)
	{
		for (i32s n2 = n1 + 1;n2 < GetSetup()->GetSFAtomCount();n2++)
		{
			sf_nbt1 newnbt1;
			newnbt1.atmi[0] = n1;		// this is a local index...
			newnbt1.atmi[1] = n2;		// this is a local index...
			
			if (!InitNBT1(& newnbt1, lookup)) continue;
			else nbt1_vector.push_back(newnbt1);
		}
	}
cout << "nbt1 ok; n = " << nbt1_vector.size() << endl;

/*##############################################*/
/*##############################################*/
	
	// the solvent term... nbt1 are needed for all VA's separated by more than 1 bonds !!!
	// the solvent term... nbt1 are needed for all VA's separated by more than 1 bonds !!!
	// the solvent term... nbt1 are needed for all VA's separated by more than 1 bonds !!!
	
	for (i32u n1 = 0;n1 < LAYERS;n1++)
	{
		nbt3_nl[n1] = new sf_nbt3_nl[GetSetup()->GetSFAtomCount() - num_solvent];
		
		for (i32s n2 = 0;n2 < GetSetup()->GetSFAtomCount() - num_solvent;n2++)
		{
			bool skip = false;
			
		// do not skip any of the layers since solv_exp[] is used and should be up-to-date. see also SetupCharges()
		// do not skip any of the layers since solv_exp[] is used and should be up-to-date. see also SetupCharges()
		// do not skip any of the layers since solv_exp[] is used and should be up-to-date. see also SetupCharges()
			
			if (skip) /*nbt3_nl[n1][n2].index = NULL*/ exit(EXIT_FAILURE);
			else nbt3_nl[n1][n2].index = new i32s[size_nl[n1]];
		}
	}
	
	dist1 = new i32s[GetSetup()->GetSFAtomCount() - num_solvent];
	dist2 = new f64[(GetSetup()->GetSFAtomCount() - num_solvent) * ((GetSetup()->GetSFAtomCount() - num_solvent) - 1) / 2];
	
	i32s n1 = 0; i32s n2 = 0;
	while (n2 < GetSetup()->GetSFAtomCount() - num_solvent)
	{
		dist1[n2++] = n1;
		n1 += (GetSetup()->GetSFAtomCount() - num_solvent) - n2;
	}
	
/*##############################################*/
/*##############################################*/
	
	// allocate the arrays for intermediate results...
	// allocate the arrays for intermediate results...
	// allocate the arrays for intermediate results...
	
	bt1data = new sf_bt1_data[bt1_vector.size()];
	bt2data = new sf_bt2_data[bt2_vector.size()];
	
	// finally update the charges...
	
	CopyCRD(GetSetup()->GetModel(), this, 0);	// WHICH COORDINATES?!?!? define an "active" crd-set in the model class?!?!?!
	SetupCharges();
}

eng1_sf::~eng1_sf(void)
{
	delete[] l2g_sf;
	delete[] index_chn;
	delete[] index_res;
	
	delete[] mass;
	delete[] vdwr;
	delete[] charge1;
	delete[] charge2;
	
	for (i32u n1 = 0;n1 < LAYERS;n1++)
	{
		delete[] vdwr1[n1];
		delete[] vdwr2[n1];
		delete[] sasaE[n1];
		
		delete[] solv_exp[n1];
		
		for (i32s n2 = 0;n2 < GetSetup()->GetSFAtomCount() - num_solvent;n2++)
		{
			if (!nbt3_nl[n1][n2].index) continue;
			else delete[] nbt3_nl[n1][n2].index;
		}
		
		delete[] nbt3_nl[n1];
	}
	
	delete[] dist1;
	delete[] dist2;
	
	delete[] bt1data;
	delete[] bt2data;
	
	if (tmp_vartab != NULL)
	{
		delete[] tmp_vartab; tmp_vartab = NULL;
		delete[] tmp_parames; tmp_parames = NULL;
		delete[] tmp_paramsa1; tmp_paramsa1 = NULL;
		delete[] tmp_paramsa2; tmp_paramsa2 = NULL;
		delete[] tmp_newpKa; tmp_newpKa = NULL;
	}
}

// HOW TO PROPERLY DETERMINE THE STATES???? DEPENDENT ON pH AND/OR GEOMETRY?!?!?!?
// HOW TO PROPERLY DETERMINE THE STATES???? DEPENDENT ON pH AND/OR GEOMETRY?!?!?!?
// HOW TO PROPERLY DETERMINE THE STATES???? DEPENDENT ON pH AND/OR GEOMETRY?!?!?!?

void eng1_sf::SetupCharges(void)
{
	// first, we calculate energy just to get the sasa values...
	
	Compute(0);
	
	// initialize the charges...
	
	vector<i32s> variables;
	atom ** atmtab = GetSetup()->GetSFAtoms();
	
	for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount();n1++)
	{
		charge1[n1] = 0.0;
		
		i32s cgi; i32s cvi;
		GetChgGrpVar(n1, cgi, cvi);
		if (cvi < 0) continue;
		
		variables.push_back(n1);
	}
	
	const i32s smoothing = 10;
	
	if (tmp_vartab != NULL)
	{
		delete[] tmp_vartab;
		delete[] tmp_parames;
		delete[] tmp_paramsa1;
		delete[] tmp_paramsa2;
		delete[] tmp_newpKa;
	}
	
	tmp_vartab = new i32s[variables.size() + 1];
	tmp_parames = new f64[variables.size()];
	tmp_paramsa1 = new f64[variables.size()];
	tmp_paramsa2 = new f64[variables.size()];
	tmp_newpKa = new f64[variables.size()];
	
	for (i32u n1 = 0;n1 < variables.size();n1++)	// copy the contents of variables...
	{
		tmp_vartab[n1] = variables[n1];
	}	tmp_vartab[variables.size()] = -1;	// terminate by -1!!!
	
	f64 * tmp_charge = new f64[variables.size()];
	f64 * tmp_smooth = new f64[variables.size() * smoothing];
	
// for acid-type groups: HA <--> H+ + A-   // product neg. charged; products more charged than reactants.
// for base-type groups: BH+ <--> H+ + B   // reactant pos. charged; equal charges on reactants and products.
	
	for (i32u n1 = 0;n1 < variables.size();n1++)
	{
		i32s cgi; i32s cvi;
		GetChgGrpVar(variables[n1], cgi, cvi);
		bool is_acid = myprm->charge_acid[cvi];
		bool is_base = !myprm->charge_acid[cvi];
		f64 pKa = myprm->charge_pKa[cvi];
		
		// pKa = pH + log([HX]/[X-])
		// log([X-]/[HX]) = pH - pKa = log(x/(1-x))
		// x = 10^(pH - pKa) / (1 + 10^(pH - pKa))
		
		f64 tmp1 = pow(10.0, myprm->pH - pKa);
		f64 tmp2 = tmp1 / (1.0 + tmp1);
		
		if (is_acid) charge1[variables[n1]] = -tmp2;
		if (is_base) charge1[variables[n1]] = 1.0 - tmp2;
		
		for (i32s n2 = 0;n2 < smoothing;n2++)
		{
			tmp_smooth[n1 * smoothing + n2] = charge1[variables[n1]];
		}
	}
	
	// the loop...
	
//ofstream logf("/tmp/pKa.log", ios::out);
	for (i32s loop = 0;loop < 250;loop++)
	{
		for (i32u n1 = 0;n1 < variables.size();n1++)
		{
			i32s cgi; i32s cvi;
			GetChgGrpVar(variables[n1], cgi, cvi);
			bool is_acid = myprm->charge_acid[cvi];
			bool is_base = !myprm->charge_acid[cvi];
			f64 pKa = myprm->charge_pKa[cvi];
			
			f64 wes = myprm->charge_wes[cgi];
			f64 sasa1 = myprm->charge_sasa1[cgi];
			f64 sasa2 = myprm->charge_sasa2[cgi];
			
			f64 k1 = pow(10.0, -pKa);
			f64 dg = -8.31451 * 300.0 * log(k1);
			
			f64 chargeV = -1.0;
			if (is_base) chargeV = +1.0;
			
			f64 parames = 0.0;
			f64 * crdV = & crd[l2g_sf[variables[n1]] * 3];
			for (i32s n2 = 0;n2 < GetSetup()->GetSFAtomCount();n2++)
			{
				if (variables[n1] == n2) continue;
				if (charge1[n2] == 0.0) continue;
				
				f64 * crdA = & crd[l2g_sf[n2] * 3];
				
				f64 r2 = 0.0;
				for (i32s n9 = 0;n9 < 3;n9++)
				{
					f64 tmp1 = crdV[n9] - crdA[n9];
					r2 += tmp1 * tmp1;
				}
				
				if (r2 < 0.01) continue;	// always skip if r < 1 Å !!!
				f64 r1 = sqrt(r2);
				
			//	if (r1 > 1.5) continue;		// also skip large distances; charge neutralization???
				
	// e = 2 + 76 * (( r / A ) ^ n) / (1 + ( r / A ) ^ n) = 2 + 76 * f / g
	
	f64 e1 = solv_exp[0][variables[n1]];
	f64 e2 = solv_exp[0][n2];
	
	f64 eps_n = myprm->epsilon1 + r2 * myprm->epsilon2;							// assume constant!!!
	f64 eps_A = myprm->epsilon3 - log(1.0 + (e1 + e2) * myprm->epsilon4 + e1 * e2 * myprm->epsilon5);	// assume constant!!!
	
	f64 t7a = r1 / eps_A;
	f64 t7b = pow(t7a, eps_n);
	f64 t7c = 1.0 + t7b;
	f64 t7d = 2.0 + 76.0 * (t7b / t7c);
				
				// f = Q/(e*r)
				// df/dr = -Q * ((1/e*r^2) + (de/dr)/(e^2*r))
				
				f64 qq = 138.9354518 * chargeV * charge1[n2];
				parames += qq / (t7d * r1);
			}
			
			tmp_parames[n1] = parames;
			
			f64 paramsa1 = solv_exp[0][variables[n1]] + solv_exp[1][variables[n1]] + solv_exp[2][variables[n1]];		// sum : 3*0.5 = 1.5
			f64 paramsa2 = 12.0 * solv_exp[0][variables[n1]] * solv_exp[1][variables[n1]] * solv_exp[2][variables[n1]];	// product : 0.5^3 = 0.125
			
			tmp_paramsa1[n1] = paramsa1;
			tmp_paramsa2[n1] = paramsa2;
			
			// here "cc" is a coupling constant used to stabilize the fit...
			// here "cc" is a coupling constant used to stabilize the fit...
			// here "cc" is a coupling constant used to stabilize the fit...
			
			f64 cc = loop / 100.0;		// this is for 250 steps!!!
			const f64 limit = 1.00; if (cc > limit) cc = limit;
			
			dg += cc * wes * parames * 1000.0;
			dg += cc * sasa1 * paramsa1 * 1000.0;
			dg += cc * sasa2 * paramsa2 * 1000.0;
			
			f64 k2 = exp(-dg / (8.31451 * 300.0));
			pKa = -log10(k2);
			
			f64 tmp1 = pow(10.0, myprm->pH - pKa);
			f64 tmp2 = tmp1 / (1.0 + tmp1);
			
			if (is_acid) tmp_charge[n1] = -tmp2;
			if (is_base) tmp_charge[n1] = 1.0 - tmp2;
			
			tmp_newpKa[n1] = pKa;		// do these oscillate as well???
		}
		
		f64 delta = 0.0;
		
		for (i32u n1 = 0;n1 < variables.size();n1++)
		{
			f64 ch1 = charge1[variables[n1]];
			f64 ch2 = tmp_charge[n1];
			
			for (i32s n2 = 0;n2 < smoothing - 1;n2++)
			{
				f64 tmp1 = tmp_smooth[n1 * smoothing + n2 + 1];
				tmp_smooth[n1 * smoothing + n2 + 0] = tmp1;
			}
			
			tmp_smooth[n1 * smoothing + (smoothing - 1)] = ch2;
			
			ch2 = 0.0;	// now do the smoothing by averaging the array...
			for (i32s n2 = 0;n2 < smoothing;n2++) ch2 += tmp_smooth[n1 * smoothing + n2];
			ch2 /= (f64) smoothing;
			
			f64 dch = ch2 - ch1;
			delta += dch * dch;
			
			charge1[variables[n1]] = ch2;
//logf << ch2 << " ";
		}
//logf << endl;
		
		cout << "cycle = " << loop << " delta = " << delta << endl;
//		if ((loop % 50) == 49) { int qqq; cin >> qqq; }
	}
//logf.close();

	delete[] tmp_charge;
	delete[] tmp_smooth;
	
	// do a simple charge neutralization...
	
	f64 net_charge = 0.0; i32s nneg = 0; i32s npos = 0;
	for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount();n1++)
	{
		charge2[n1] = charge1[n1];
		
		net_charge += charge1[n1];
		if (charge1[n1] < -0.05) nneg++;
		if (charge1[n1] > +0.05) npos++;
	}
	
	if (net_charge < -0.05)
	{
		cout << "NET NEGATIVE: " << net_charge << endl;
		f64 counterion_effect = fabs(net_charge) / (f64) nneg;
		for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount();n1++)
		{
			if (charge1[n1] < -0.05) charge2[n1] += counterion_effect;
		}
	}
	else if (net_charge > +0.05)
	{
		cout << "NET POSITIVE: " << net_charge << endl;
		f64 counterion_effect = net_charge / (f64) npos;
		for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount();n1++)
		{
			if (charge1[n1] > +0.05) charge2[n1] -= counterion_effect;
		}
	}
	
	// report the results briefly...
	
	i32u counter = 0;
	for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount();n1++)
	{
		atmtab[n1]->charge = 0.0;		// update the atom objects as well!!!
		
		i32s cgi; i32s cvi;
		GetChgGrpVar(n1, cgi, cvi);
		if (cvi < 0) continue;
		
		atmtab[n1]->charge = charge1[n1];	// update the atom objects as well!!! see StorePStatesToModel().
		
		cout << "atmi = " << n1 << " (" << index_chn[n1] << "/" << index_res[n1] << ") ";
		cout << "pKa = " << tmp_newpKa[counter] << " (shift = " << (tmp_newpKa[counter] - myprm->charge_pKa[cvi]) << ") ";
		cout << "charge1 = " << charge1[n1] << " charge2 = " << charge2[n1] << endl;
		
		counter++;
	}
	
	// save the results in mdl::ref_civ if available; mdl::CheckProtonation() uses it!!!
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	setup1_sf * susf = dynamic_cast<setup1_sf *>(GetSetup());
	if (susf != NULL) susf->StorePStatesToModel(this);
	
	delete[] tmp_vartab; tmp_vartab = NULL;		// normally, this is not needed anymore...
	delete[] tmp_parames; tmp_parames = NULL;	// normally, this is not needed anymore...
	delete[] tmp_paramsa1; tmp_paramsa1 = NULL;	// normally, this is not needed anymore...
	delete[] tmp_paramsa2; tmp_paramsa2 = NULL;	// normally, this is not needed anymore...
	delete[] tmp_newpKa; tmp_newpKa = NULL;		// normally, this is not needed anymore...
	
	// since the charges have changed, update sasa and neighbor lists as well!!!
	// since the charges have changed, update sasa and neighbor lists as well!!!
	// since the charges have changed, update sasa and neighbor lists as well!!!
	
	setup1_sf * mysu = dynamic_cast<setup1_sf *>(GetSetup());
	if (!mysu) { cout << "BUG: cast to setup1_sf failed." << endl; exit(EXIT_FAILURE); }
	
	for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount() - num_solvent;n1++)
	{
		i32s indc = index_chn[n1];
		i32s indr = index_res[n1];
		f64 tmpc = charge1[n1];
		
		char symbol = mysu->chn_vector[indc].res_vector[indr].symbol;
		i32s ind = 0; while (ind < 20 && model::sf_symbols[ind] != symbol) ind++;
		if (ind == 20) { cout << "BUG: unknown residue symbol." << endl; exit(EXIT_FAILURE); }
		
		i32s inda = 0;
		while (mysu->chn_vector[indc].res_vector[indr].atmr[inda] != atmtab[n1])
		{
			inda++;
			if (inda >= mysu->chn_vector[indc].res_vector[indr].natm)
			{
				cout << "search of inda failed!!! i = " << n1 << endl;
				exit(EXIT_FAILURE);
			}
		}
		
		i32s tmpt = (ind << 8) | inda;
		i32s tmpi = 0; while (tmpi < SF_NUM_TYPES && model::sf_types[tmpi] != tmpt) tmpi++;
		if (tmpi >= SF_NUM_TYPES) { cout << "BUG: tmpi overflow."; exit(EXIT_FAILURE); }
		
		f64 tmps = (model::sf_is_polar[tmpi] ? myprm->imp_solv_1p : myprm->imp_solv_1n);
		tmps += myprm->imp_solv_2 * tmpc * tmpc;
		
		for (i32u n2 = 0;n2 < LAYERS;n2++)
		{
			const f64 tmpsasa2[2] = { 0.80, 0.64 };		// NOT_PROPERLY_OPTIMIZED!!!
			
			f64 svalue = tmps;
			if (n2 != 0)
			{
				f64 tmp1 = vdwr2[0][n1] / vdwr2[n2][n1];
				svalue *= tmp1 * tmpsasa2[n2 - 1];
				
				// neg: -4 -2 -1	long range???
				// pos: +2 0 0		short range???
				if (svalue > 0.0) svalue = 0.0;		// NOT_PROPERLY_OPTIMIZED!!!
			}
			
			sasaE[n2][n1] = svalue * vdwr2[n2][n1];
			
			// now update the neighbor lists...
			
			if (nbt3_nl[n2][n1].index != NULL) delete[] nbt3_nl[n2][n1].index;
			
			bool skip = false;
			
		// do not skip any of the layers since solv_exp[] is used and should be up-to-date!!!
		// do not skip any of the layers since solv_exp[] is used and should be up-to-date!!!
		// do not skip any of the layers since solv_exp[] is used and should be up-to-date!!!
			
			if (skip) /*nbt3_nl[n2][n1].index = NULL*/ exit(EXIT_FAILURE);
			else nbt3_nl[n2][n1].index = new i32s[size_nl[n2]];
		}
	}
}

void eng1_sf::GetChgGrpVar(i32s atmi, i32s & cgi, i32s & cvi)
{
	i32s indc = index_chn[atmi];
	i32s indr = index_res[atmi];
	
	if (indc < 0)	// detect and handle solvent atom cases...
	{
		cgi = NOT_DEFINED;
		cvi = NOT_DEFINED;
		return;
	}
	
	atom ** atmtab = GetSetup()->GetSFAtoms();
	setup1_sf * mysu = dynamic_cast<setup1_sf *>(GetSetup());
	if (!mysu) { cout << "BUG: cast to setup1_sf failed." << endl; exit(EXIT_FAILURE); }
	myprm = & mysu->prm;
	
	i32s inda = 0;
	while (mysu->chn_vector[indc].res_vector[indr].atmr[inda] != atmtab[atmi])
	{
		inda++;
		if (inda >= mysu->chn_vector[indc].res_vector[indr].natm)
		{
			cout << "search of inda failed!!! i = " << atmi << endl;
			exit(EXIT_FAILURE);
		}
	}
	
	// now determine the variable index!!!
	// now determine the variable index!!!
	// now determine the variable index!!!
	
	cvi = NOT_DEFINED;
	
	if (inda == 0 && indr == 0)
	{
		cvi = 0;		// n-terminal
	}
	
	if (inda == 0 && indr == ((i32s) mysu->chn_vector[indc].res_vector.size()) - 1)
	{
		cvi = 1;		// c-terminal
	}
	
	char symbol = mysu->chn_vector[indc].res_vector[indr].symbol;
	if (symbol == 'R' && inda == 2) cvi = 2;
	if (symbol == 'D' && inda == 1) cvi = 3;
	if (symbol == 'C' && inda == 1) cvi = 4;		// cysteine!!!
	if (symbol == 'E' && inda == 1) cvi = 5;
	if (symbol == 'H' && inda == 1) cvi = 6;
	if (symbol == 'K' && inda == 2) cvi = 7;
	if (symbol == 'Y' && inda == 1) cvi = 8;
	
	// check the cases where cysteine is disulphide-bridged!!!
	// check the cases where cysteine is disulphide-bridged!!!
	// check the cases where cysteine is disulphide-bridged!!!
	
	if (cvi == 4)		// this is a test for cysteine, see above...
	{
		bool flag = false;
		
		for (i32u n1 = 0;n1 < mysu->dsb_vector.size();n1++)
		{
			if (mysu->dsb_vector[n1].chn[0] == indc && mysu->dsb_vector[n1].res[0] == indr) flag = true;
			if (mysu->dsb_vector[n1].chn[1] == indc && mysu->dsb_vector[n1].res[1] == indr) flag = true;
			
			if (flag) break;
		}
		
		if (flag) cvi = NOT_DEFINED;
	}

	// now determine the group index!!!
	// now determine the group index!!!
	// now determine the group index!!!
	
	switch (cvi)
	{
		case 1:		// ct
		case 3:		// D1
		case 5:		// E1
		cgi = 0;		// strong acids
		break;
		
		case 4:		// C1
		case 8:		// Y1
		cgi = 1;		// weak acids
		break;
		
		case 0:		// nt
		case 2:		// R2
		case 7:		// K2
		cgi = 2;		// strong bases
		break;
		
		case 6:
		cgi = 3;		// weak bases
		break;
		
		default:
		cgi = NOT_DEFINED;
	}
}

bool eng1_sf::InitNBT1(sf_nbt1 * ref, vector<sf_nonbonded_lookup> & lookup)
{
	atom ** atmtab = GetSetup()->GetSFAtoms();
	
	setup1_sf * mysu = dynamic_cast<setup1_sf *>(GetSetup());
	if (!mysu) { cout << "BUG: cast to setup1_sf failed." << endl; exit(EXIT_FAILURE); }
	myprm = & mysu->prm;
	
// these are used to save memory when measuring vdw-radii and surface areas... 2.0 * (0.3 + 0.2) = 1.0
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//const fGL * c1 = atmtab[ref->atmi[0]]->GetCRD(0); const fGL * c2 = atmtab[ref->atmi[1]]->GetCRD(0);
//v3d<fGL> v1 = v3d<fGL>(c1, c2); if (v1.len() > 1.5) return false;
	
	i32s indc1 = index_chn[ref->atmi[0]];
	i32s indr1 = index_res[ref->atmi[0]];
	
	i32s indc2 = index_chn[ref->atmi[1]];
	i32s indr2 = index_res[ref->atmi[1]];
	
	if (indc1 < 0 || indc2 < 0)		// solvent-solute or solvent-solvent
	{
		f64 radius[2];
		radius[0] = vdwr[ref->atmi[0]];
		radius[1] = vdwr[ref->atmi[1]];
		
		f64 optr = radius[0] + radius[1];
		f64 opte;
		
		// nonbonded.txt min/median/max = 0.18/0.83/3.56
		// that is multiplied by myprm->lenjon = 1.225 -> average = 0.83 * 1.225 = 1.02
		
		// for H2O, if opte > 3.00 -> more or less frozen state?!?!?!
		// difficult to estimate for other cases, since also depends on mass/size...
		
		if (indc1 < 0 && indc2 < 0) opte = 2.00;	// opte > 3.00 -> frozen???
		else	// solvent-solute cases...
		{
			i32s indc; i32s indr; i32s n1;
			if (indc2 < 0) { indc = indc1; indr = indr1; n1 = ref->atmi[0]; }
			else { indc = indc2; indr = indr2; n1 = ref->atmi[1]; }
			
			f64 tmpc = charge1[n1];
			
			char symbol = mysu->chn_vector[indc].res_vector[indr].symbol;
			i32s ind = 0; while (ind < 20 && model::sf_symbols[ind] != symbol) ind++;
			if (ind == 20) { cout << "BUG: unknown residue symbol." << endl; exit(EXIT_FAILURE); }
			
			i32s inda = 0;
			while (mysu->chn_vector[indc].res_vector[indr].atmr[inda] != atmtab[n1])
			{
				inda++;
				if (inda >= mysu->chn_vector[indc].res_vector[indr].natm)
				{
					cout << "search of inda failed!!! i = " << n1 << endl;
					exit(EXIT_FAILURE);
				}
			}
			
			i32s tmpt = (ind << 8) | inda;
			i32s tmpi = 0; while (tmpi < SF_NUM_TYPES && model::sf_types[tmpi] != tmpt) tmpi++;
			if (tmpi >= SF_NUM_TYPES) { cout << "BUG: tmpi overflow."; exit(EXIT_FAILURE); }
			
			f64 tmps = (model::sf_is_polar[tmpi] ? myprm->exp_solv_1p : myprm->exp_solv_1n);
			tmps += myprm->exp_solv_2 * tmpc * tmpc;
			
		// ??? see also SetupCharges()!!!
			
			opte = tmps;
		}
		
		InitLenJon(ref, optr, opte);
		return true;
	}
	
	i32s inda1 = 0;
	while (mysu->chn_vector[indc1].res_vector[indr1].atmr[inda1] != atmtab[ref->atmi[0]])
	{
		inda1++;
		if (inda1 >= mysu->chn_vector[indc1].res_vector[indr1].natm)
		{
			cout << "initNB : search of inda1 failed!!! i = " << ref->atmi[0] << endl;
			exit(EXIT_FAILURE);
		}
	}
	
	i32s inda2 = 0;
	while (mysu->chn_vector[indc2].res_vector[indr2].atmr[inda2] != atmtab[ref->atmi[1]])
	{
		inda2++;
		if (inda2 >= mysu->chn_vector[indc2].res_vector[indr2].natm)
		{
			cout << "initNB : search of inda2 failed!!! i = " << ref->atmi[1] << endl;
			exit(EXIT_FAILURE);
		}
	}
	
	// nbt1 are needed for all VA's separated by more than 1 bonds (solvent term).
	// nbt1 are needed for all VA's separated by more than 1 bonds (solvent term).
	// nbt1 are needed for all VA's separated by more than 1 bonds (solvent term).
	
	if (inda1 && inda2)		// bridge???
	{
		for (i32u n1 = 0;n1 < mysu->dsb_vector.size();n1++)
		{
			bool test1a = (indc1 == mysu->dsb_vector[n1].chn[0]);
			bool test1b = (indr1 == mysu->dsb_vector[n1].res[0]);
			bool test1c = (indc2 == mysu->dsb_vector[n1].chn[1]);
			bool test1d = (indr2 == mysu->dsb_vector[n1].res[1]);
			if (test1a && test1b && test1c && test1d) return false;
			
			bool test2a = (indc1 == mysu->dsb_vector[n1].chn[1]);
			bool test2b = (indr1 == mysu->dsb_vector[n1].res[1]);
			bool test2c = (indc2 == mysu->dsb_vector[n1].chn[0]);
			bool test2d = (indr2 == mysu->dsb_vector[n1].res[0]);
			if (test2a && test2b && test2c && test2d) return false;
		}
	}
	
	bool same_chn = (indc1 == indc2);
	
	bool rdist_is_0 = same_chn && ((indr2 - indr1) == 0);
	bool rdist_is_1 = same_chn && ((indr2 - indr1) == 1);
	
	if (rdist_is_0 && ((inda2 - inda1) < 2)) return false;
	if (rdist_is_1 && !inda1 && !inda2) return false;
	
	bool rdist_is_2 = same_chn && ((indr2 - indr1) == 2);
	bool rdist_is_3 = same_chn && ((indr2 - indr1) == 3);
	bool rdist_is_4 = same_chn && ((indr2 - indr1) == 4);
	
	bool both_in_main_chn = (!inda1 && !inda2);
	
	bool check3 = (indr1 + 1 < (i32s) mysu->chn_vector[indc1].res_vector.size());
	bool check4 = (indr1 + 2 < (i32s) mysu->chn_vector[indc1].res_vector.size());
	bool check5 = (indr1 + 3 < (i32s) mysu->chn_vector[indc1].res_vector.size());
	
	bool helix3 = check3 && (mysu->chn_vector[indc1].res_vector[indr1 + 1].state == SF_STATE_HELIX4);
	bool helix4 = check4 && helix3 && (mysu->chn_vector[indc1].res_vector[indr1 + 2].state == SF_STATE_HELIX4);
	bool helix5 = check5 && helix4 && (mysu->chn_vector[indc1].res_vector[indr1 + 3].state == SF_STATE_HELIX4);
	
	f64 radius[2];
	radius[0] = vdwr[ref->atmi[0]];
	radius[1] = vdwr[ref->atmi[1]];
	
	f64 optr = radius[0] + radius[1];
	f64 opte = 0.0;
	
	char symbol[2] =
	{
		mysu->chn_vector[indc1].res_vector[indr1].symbol,
		mysu->chn_vector[indc2].res_vector[indr2].symbol
	};
	
	i32u index = 0;
	while (true)
	{
		if (index >= lookup.size()) { cout << "nonbonded lookup failed!!!" << endl; exit(EXIT_FAILURE); }
		
		bool test1 = (lookup[index].s1 == symbol[0] && lookup[index].a1 == (i32u) inda1 && lookup[index].s2 == symbol[1] && lookup[index].a2 == (i32u) inda2);
		bool test2 = (lookup[index].s1 == symbol[1] && lookup[index].a1 == (i32u) inda2 && lookup[index].s2 == symbol[0] && lookup[index].a2 == (i32u) inda1);
		if (test1 || test2)
		{
			opte = lookup[index].opte * myprm->lenjon;
			break;		// exit the loop...
		}
		else index++;
	}
	
	// use modified values in helices!!! these will produce correct helix dimensions.
	// use modified values in helices!!! these will produce correct helix dimensions.
	// use modified values in helices!!! these will produce correct helix dimensions.
	
	if (both_in_main_chn)
	{
		bool test3 = (rdist_is_2 && helix3);
		bool test4 = (rdist_is_3 && helix4);
		bool test5 = (rdist_is_4 && helix5);
		
		if (test3 || test4 || test5)
		{
			optr = 0.650;
			opte = 0.95 * myprm->lenjon;
		}
	}
	
	InitLenJon(ref, optr, opte);
	return true;
}

void eng1_sf::InitLenJon(sf_nbt1 * ref, f64 opt, f64 fc)
{
	if (opt < 0.100)
	{
		cout << "eng1_sf::InitLenJon() : too small opt : " << opt << endl;
		exit(EXIT_FAILURE);
	}
	
	if (fc < 0.100)
	{
		cout << "eng1_sf::InitLenJon() : too small fc : " << fc << endl;
		exit(EXIT_FAILURE);
	}
	
//	ref->data[0] = opt * pow(2.0 * fc, 1.0 / 9.0);		// 6-9
//	ref->data[1] = opt * pow(3.0 * fc, 1.0 / 6.0);
	ref->data[0] = opt * pow(1.0 * fc, 1.0 / 12.0);		// 6-12
	ref->data[1] = opt * pow(2.0 * fc, 1.0 / 6.0);
//	ref->data[0] = opt * pow(3.0 * fc, 1.0 / 12.0);		// 9-12
//	ref->data[1] = opt * pow(4.0 * fc, 1.0 / 9.0);
}

void eng1_sf::ComputeBT1(i32u p1)
{
	energy_bt1 = 0.0;
	
	// data1 -> length of the bond vector, in nanometers [nm].
	// data2[0] -> grad[0-2]: for atom 1 when direction = 0, for atom 0 when direction = 1
	// data2[1] -> grad[0-2]: for atom 0 when direction = 0, for atom 1 when direction = 1
	
	// mm1_eng_exp1::ComputeBT1() works in a similar way...
	
	for (i32u n1 = 0;n1 < bt1_vector.size();n1++)
	{
		i32s * atmi = bt1_vector[n1].atmi;
		
		f64 t1a[3]; f64 t1b = 0.0;
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			f64 t9a = crd[l2g_sf[atmi[0]] * 3 + n2];
			f64 t9b = crd[l2g_sf[atmi[1]] * 3 + n2];
			
			t1a[n2] = t9a - t9b;
			t1b += t1a[n2] * t1a[n2];
		}
		
		f64 t1c = sqrt(t1b);
		bt1data[n1].data1 = t1c;
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			f64 t9a = t1a[n2] / t1c;
			bt1data[n1].data2[0][n2] = +t9a;
			bt1data[n1].data2[1][n2] = -t9a;
		}
		
		/*##############################################*/
		/*##############################################*/
		// this is for the surface area term...
		
		bool first = (atmi[0] > atmi[1]);
		dist2[dist1[atmi[first]] + (atmi[!first] - atmi[first]) - 1] = t1c;
		
		// 1st layer...
		// 1st layer...
		// 1st layer...
		
		// none of the SASA terms should be skipped at 1st layer -> no test...
		
		if (t1c < (vdwr1[0][atmi[0]] + vdwr1[0][atmi[1]]))
		{
			nbt3_nl[0][atmi[0]].index[nbt3_nl[0][atmi[0]].index_count++] = atmi[1];
			if (nbt3_nl[0][atmi[0]].index_count >= size_nl[0]) { cout << "BUG: NL overflow 1a!!!" << endl; exit(EXIT_FAILURE); }
			
			nbt3_nl[0][atmi[1]].index[nbt3_nl[0][atmi[1]].index_count++] = atmi[0];
			if (nbt3_nl[0][atmi[1]].index_count >= size_nl[0]) { cout << "BUG: NL overflow 1a!!!" << endl; exit(EXIT_FAILURE); }
		}
		
		// 2nd layer...
		// 2nd layer...
		// 2nd layer...
		
		bool test2a = (nbt3_nl[1][atmi[0]].index != NULL);
		if (test2a && t1c < (vdwr1[1][atmi[0]] + vdwr1[0][atmi[1]]) && t1c > (vdwr1[1][atmi[0]] - vdwr1[0][atmi[1]]))
		{
			nbt3_nl[1][atmi[0]].index[nbt3_nl[1][atmi[0]].index_count++] = atmi[1];
			if (nbt3_nl[1][atmi[0]].index_count >= size_nl[1]) { cout << "BUG: NL overflow 2a!!!" << endl; exit(EXIT_FAILURE); }
		}
		
		bool test2b = (nbt3_nl[1][atmi[1]].index != NULL);
		if (test2b && t1c < (vdwr1[0][atmi[0]] + vdwr1[1][atmi[1]]) && t1c > (vdwr1[1][atmi[1]] - vdwr1[0][atmi[0]]))
		{
			nbt3_nl[1][atmi[1]].index[nbt3_nl[1][atmi[1]].index_count++] = atmi[0];
			if (nbt3_nl[1][atmi[1]].index_count >= size_nl[1]) { cout << "BUG: NL overflow 2a!!!" << endl; exit(EXIT_FAILURE); }
		}
		
		// 3rd layer...
		// 3rd layer...
		// 3rd layer...
		
		bool test3a = (nbt3_nl[2][atmi[0]].index != NULL);
		if (test3a && t1c < (vdwr1[2][atmi[0]] + vdwr1[0][atmi[1]]) && t1c > (vdwr1[2][atmi[0]] - vdwr1[0][atmi[1]]))
		{
			nbt3_nl[2][atmi[0]].index[nbt3_nl[2][atmi[0]].index_count++] = atmi[1];
			if (nbt3_nl[2][atmi[0]].index_count >= size_nl[2]) { cout << "BUG: NL overflow 3a!!!" << endl; exit(EXIT_FAILURE); }
		}
		
		bool test3b = (nbt3_nl[2][atmi[1]].index != NULL);
		if (test3b && t1c < (vdwr1[0][atmi[0]] + vdwr1[2][atmi[1]]) && t1c > (vdwr1[2][atmi[1]] - vdwr1[0][atmi[0]]))
		{
			nbt3_nl[2][atmi[1]].index[nbt3_nl[2][atmi[1]].index_count++] = atmi[0];
			if (nbt3_nl[2][atmi[1]].index_count >= size_nl[2]) { cout << "BUG: NL overflow 3a!!!" << endl; exit(EXIT_FAILURE); }
		}
		
		// this is for the surface area term...
		/*##############################################*/
		/*##############################################*/
		
		// f = a(x-b)^2
		// df/dx = 2a(x-b)
		
		f64 t2a = t1c - bt1_vector[n1].opt;
		energy_bt1 += bt1_vector[n1].fc * t2a * t2a;
		
		if (p1 > 0)
		{
			f64 t2b = 2.0 * bt1_vector[n1].fc * t2a;
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t2c = bt1data[n1].data2[0][n2] * t2b;
				
				d1[l2g_sf[atmi[0]] * 3 + n2] += t2c;
				d1[l2g_sf[atmi[1]] * 3 + n2] -= t2c;
			}
		}
	}
}

void eng1_sf::ComputeBT2(i32u p1)
{
	energy_bt2 = 0.0;
	
	// data1 -> cosine of the bond angle, in the usual range [-1.0, +1.0]
	// data2[0] -> grad[0-2]: for atom 2 when direction = 0, for atom 0 when direction = 1
	// data2[1] -> grad[0-2]: for atom 1 when direction = 0, for atom 1 when direction = 1
	// data2[2] -> grad[0-2]: for atom 0 when direction = 0, for atom 2 when direction = 1
	
	// mm1_eng_exp1::ComputeBT2() works in a similar way...
	
	for (i32u n1 = 0;n1 < bt2_vector.size();n1++)
	{
		i32s * atmi = bt2_vector[n1].atmi;
		
		i32s * index1 = bt2_vector[n1].index1;
		bool * dir1 = bt2_vector[n1].dir1;
		
		f64 * t1a = bt1data[index1[0]].data2[dir1[0]];
		f64 * t1b = bt1data[index1[1]].data2[dir1[1]];
		
		f64 t1c = t1a[0] * t1b[0] + t1a[1] * t1b[1] + t1a[2] * t1b[2];
		
		if (t1c < -1.0) t1c = -1.0;		// domain check...
		if (t1c > +1.0) t1c = +1.0;		// domain check...
		
// if the main-chain angles approach 180.0 deg we will have some serious numerical problems in later
// stages. fc[1] must be large enough to prevent such problems. here is how we can monitor this...

bool problem1 = (t1c < -0.999);				// is it too close to 180 deg ???
bool problem2 = (bt2_vector[n1].fc[1] > 0.0);		// is it really a main-chain angle???
if (problem1 && problem2) { cout << "BUG: BT2 ang -> 180.0 deg." << endl; exit(EXIT_FAILURE); }

		bt2data[n1].data1 = t1c;
		
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			f64 t9a = (t1b[n2] - t1c * t1a[n2]) / bt1data[index1[0]].data1;
			f64 t9b = (t1a[n2] - t1c * t1b[n2]) / bt1data[index1[1]].data1;
			
			bt2data[n1].data2[0][n2] = t9a;
			bt2data[n1].data2[1][n2] = -(t9a + t9b);
			bt2data[n1].data2[2][n2] = t9b;
		}
		
		// f = a(x-b)^2
		// df/dx = 2a(x-b)
		
		f64 t3b = t1c - bt2_vector[n1].opt;
		energy_bt2 += bt2_vector[n1].fc[0] * t3b * t3b;
		
		// f = a/(x+1)^2
		// df/dx = -2a/(x+1)^3
		
		f64 t3c = t1c + 1.0;
		
		f64 t3d = t3c * t3c;
		energy_bt2 += bt2_vector[n1].fc[1] / t3d;
		
		if (p1 > 0)
		{
			f64 t2a = 2.0 * bt2_vector[n1].fc[0] * t3b;
			
			f64 t3f = t3d * t3c;
			f64 t2b = 2.0 * bt2_vector[n1].fc[1] / t3f;
			
			f64 t2c = t2a - t2b;
			
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				d1[l2g_sf[atmi[0]] * 3 + n2] += bt2data[n1].data2[0][n2] * t2c;
				d1[l2g_sf[atmi[1]] * 3 + n2] += bt2data[n1].data2[1][n2] * t2c;
				d1[l2g_sf[atmi[2]] * 3 + n2] += bt2data[n1].data2[2][n2] * t2c;
			}
		}
	}
}

void eng1_sf::ComputeBT3(i32u p1)
{
	energy_bt3a = 0.0;
	energy_bt3b = 0.0;
	
	for (i32u n1 = 0;n1 < bt3_vector.size();n1++)
	{
		i32s * atmi = bt3_vector[n1].atmi;
		
		i32s * index2 = bt3_vector[n1].index2;
		i32s * index1 = bt3_vector[n1].index1;
		bool * dir1 = bt3_vector[n1].dir1;
		
		// ang is never > 180.0 deg -> sin(ang) is never < 0.0 -> no need to check signs!!!
		// ang is never > 180.0 deg -> sin(ang) is never < 0.0 -> no need to check signs!!!
		// ang is never > 180.0 deg -> sin(ang) is never < 0.0 -> no need to check signs!!!
		
		f64 t1a[2] = { bt2data[index2[0]].data1, bt2data[index2[1]].data1 };
		f64 t1b[2] = { 1.0 - t1a[0] * t1a[0], 1.0 - t1a[1] * t1a[1] };
		
		f64 t1c[2][3];
		t1c[0][0] = bt1data[index1[0]].data2[dir1[0]][0] - t1a[0] * bt1data[index1[1]].data2[dir1[1]][0];
		t1c[0][1] = bt1data[index1[0]].data2[dir1[0]][1] - t1a[0] * bt1data[index1[1]].data2[dir1[1]][1];
		t1c[0][2] = bt1data[index1[0]].data2[dir1[0]][2] - t1a[0] * bt1data[index1[1]].data2[dir1[1]][2];
		t1c[1][0] = bt1data[index1[3]].data2[dir1[3]][0] - t1a[1] * bt1data[index1[2]].data2[dir1[2]][0];
		t1c[1][1] = bt1data[index1[3]].data2[dir1[3]][1] - t1a[1] * bt1data[index1[2]].data2[dir1[2]][1];
		t1c[1][2] = bt1data[index1[3]].data2[dir1[3]][2] - t1a[1] * bt1data[index1[2]].data2[dir1[2]][2];
		
		f64 t9a = t1c[0][0] * t1c[1][0] + t1c[0][1] * t1c[1][1] + t1c[0][2] * t1c[1][2];
		f64 t1d = t9a / sqrt(t1b[0] * t1b[1]);
		
		if (t1d < -1.0) t1d = -1.0;		// domain check...
		if (t1d > +1.0) t1d = +1.0;		// domain check...
		
		f64 t1e = acos(t1d);
		
		// now we still have to determine sign of the result...
		
		v3d<f64> tmpv1 = v3d<f64>(t1c[0]);
		v3d<f64> tmpv2 = v3d<f64>(bt1data[index1[2]].data2[dir1[2]]);
		v3d<f64> tmpv3 = v3d<f64>(bt1data[index1[3]].data2[dir1[3]]);
		v3d<f64> tmpv4 = tmpv2.vpr(tmpv3);
		
		if (tmpv1.spr(tmpv4) < 0.0) t1e = -t1e;
		
		f64 t1f; f64 t1g;		// f and df/dx !!!
		f64 cs[3]; f64 sn[3];
		
		if (bt3_vector[n1].tor_ttype != TTYPE_LOOP)	// helix + strand
		{
			// Dx = x-b			| for the following formulas...
			
			// f = a(2PI-Dx)^2		| if Dx > +PI
			// df/fx = -2a(2PI-Dx)
			
			// f = a(2PI+Dx)^2		| if Dx < -PI
			// df/fx = +2a(2PI+Dx)
			
			// f = a(Dx)^2			| otherwise
			// df/fx = 2a(Dx)
			
			f64 t1h = t1e - bt3_vector[n1].tors[0];
			if (t1h > +M_PI)
			{
				t1h = 2.0 * M_PI - t1h;
				
				t1f = bt3_vector[n1].tors[1] * t1h * t1h;
				t1g = -2.0 * bt3_vector[n1].tors[1] * t1h;
			}
			else if (t1h < -M_PI)
			{
				t1h = 2.0 * M_PI + t1h;
				
				t1f = bt3_vector[n1].tors[1] * t1h * t1h;
				t1g = +2.0 * bt3_vector[n1].tors[1] * t1h;
			}
			else
			{
				t1f = bt3_vector[n1].tors[1] * t1h * t1h;
				t1g = 2.0 * bt3_vector[n1].tors[1] * t1h;
			}
			
			energy_bt3a += t1f;
		}
		else		// loop!!!
		{
			// f = a*cos(x) + b*cos(2x) + c*cos(3x) + d*sin(x) + e*sin(2x) + f*sin(3x)
			// df/dx = d*cos(x) + 2e*cos(2x) + 3f*cos(3x) - a*sin(x) - 2b*sin(2x) - 3c*sin(3x)
			
			f64 t1m = t1e + t1e; f64 t1n = t1m + t1e;
			cs[0] = cos(t1e); cs[1] = cos(t1m); cs[2] = cos(t1n);
			sn[0] = sin(t1e); sn[1] = sin(t1m); sn[2] = sin(t1n);
			
			f64 * fsc = bt3_vector[n1].torc; f64 * fss = bt3_vector[n1].tors;
			
			t1f = fsc[0] * cs[0] + fsc[1] * cs[1] + fsc[2] * cs[2];
			t1f += fss[0] * sn[0] + fss[1] * sn[1] + fss[2] * sn[2];
			
			t1g = fss[0] * cs[0] + 2.0 * fss[1] * cs[1] + 3.0 * fss[2] * cs[2];
			t1g -= fsc[0] * sn[0] + 2.0 * fsc[1] * sn[1] + 3.0 * fsc[2] * sn[2];
			
			energy_bt3b += t1f;
		}
		
		f64 t4c[3]; f64 t5c[3]; f64 t6a[3]; f64 t7a[3];		// these are needed later...
		
		if (p1 > 0)
		{
			f64 t2a = bt1data[index1[0]].data1 * t1b[0];
			f64 t2b = bt1data[index1[0]].data1 * t1a[0] / bt1data[index1[1]].data1;
			
			f64 t3a = bt1data[index1[3]].data1 * t1b[1];
			f64 t3b = bt1data[index1[3]].data1 * t1a[1] / bt1data[index1[2]].data1;
			
			const i32s cp[3][3] = { { 0, 1, 2 }, { 1, 2, 0 }, { 2, 0, 1 } };
			
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t4a = bt1data[index1[0]].data2[dir1[0]][cp[n2][1]] * bt1data[index1[1]].data2[dir1[1]][cp[n2][2]];
				f64 t4b = bt1data[index1[0]].data2[dir1[0]][cp[n2][2]] * bt1data[index1[1]].data2[dir1[1]][cp[n2][1]];
				t4c[n2] = (t4a - t4b) / t2a;
				
				f64 t5a = bt1data[index1[2]].data2[dir1[2]][cp[n2][2]] * bt1data[index1[3]].data2[dir1[3]][cp[n2][1]];
				f64 t5b = bt1data[index1[2]].data2[dir1[2]][cp[n2][1]] * bt1data[index1[3]].data2[dir1[3]][cp[n2][2]];
				t5c[n2] = (t5a - t5b) / t3a;
				
				d1[l2g_sf[atmi[0]] * 3 + n2] += t1g * t4c[n2];
				d1[l2g_sf[atmi[3]] * 3 + n2] += t1g * t5c[n2];
				
				t6a[n2] = (t2b - 1.0) * t4c[n2] - t3b * t5c[n2];
				t7a[n2] = (t3b - 1.0) * t5c[n2] - t2b * t4c[n2];
				
				d1[l2g_sf[atmi[1]] * 3 + n2] += t1g * t6a[n2];
				d1[l2g_sf[atmi[2]] * 3 + n2] += t1g * t7a[n2];
			}
		}
		
		// calculate dipole directions (with derivatives if needed)...
		// calculate dipole directions (with derivatives if needed)...
		// calculate dipole directions (with derivatives if needed)...
		
		// f = a*cos(x) + b*cos(2x) + c*cos(3x) + d*sin(x) + e*sin(2x) + f*sin(3x) + g*x + h
		// df/dx = d*cos(x) + 2e*cos(2x) + 3f*cos(3x) - a*sin(x) - 2b*sin(2x) - 3c*sin(3x) + g
		
		f64 loop1; f64 loop2; f64 dpbdd[4][3];
		f64 * fsc = bt3_vector[n1].dipc; f64 * fss = bt3_vector[n1].dips;
		
		switch (bt3_vector[n1].dip_ttype)
		{
			case TTYPE_HELIX:
			bt3_vector[n1].pbdd = -0.935131;
			break;
			
			case TTYPE_STRAND:
			bt3_vector[n1].pbdd = +1.994676;
			break;
			
			default:	// this is for TTYPE_LOOP...
			loop1 = bt3_vector[n1].dipk[0] * t1e + bt3_vector[n1].dipk[1];
			loop1 += fsc[0] * cs[0] + fsc[1] * cs[1] + fsc[2] * cs[2];
			loop1 += fss[0] * sn[0] + fss[1] * sn[1] + fss[2] * sn[2];
			bt3_vector[n1].pbdd = loop1;
			
			if (p1 > 0)
			{
				loop2 = bt3_vector[n1].dipk[0];
				loop2 += fss[0] * cs[0] + 2.0 * fss[1] * cs[1] + 3.0 * fss[2] * cs[2];
				loop2 -= fsc[0] * sn[0] + 2.0 * fsc[1] * sn[1] + 3.0 * fsc[2] * sn[2];
				
				for (i32s n2 = 0;n2 < 3;n2++)
				{
					dpbdd[0][n2] = loop2 * t4c[n2];
					dpbdd[3][n2] = loop2 * t5c[n2];
					
					dpbdd[1][n2] = loop2 * t6a[n2];
					dpbdd[2][n2] = loop2 * t7a[n2];
				}
			}
		}
		
		// calculate some dipole results already here...
		// calculate some dipole results already here...
		// calculate some dipole results already here...
		
		if (bt3_vector[n1].skip) continue;	// skip all X-pro cases...
		
		f64 t2a[3]; f64 t2b[3][3];		// 1st unit bond vector + derivatives...
		f64 t2c[3]; f64 t2d[3][3];		// 2nd unit bond vector + derivatives...
		
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			t2a[n2] = bt1data[index1[0]].data2[dir1[0]][n2];
			t2c[n2] = bt1data[index1[1]].data2[dir1[1]][n2];
		}
		
		if (p1 > 0)
		{
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				for (i32s n3 = 0;n3 < 3;n3++)
				{
					if (n2 != n3)
					{
						t2b[n2][n3] = -t2a[n2] * t2a[n3] / bt1data[index1[0]].data1;
						t2d[n2][n3] = -t2c[n2] * t2c[n3] / bt1data[index1[1]].data1;
					}
					else
					{
						t2b[n2][n2] = (1.0 - t2a[n2] * t2a[n2]) / bt1data[index1[0]].data1;
						t2d[n2][n2] = (1.0 - t2c[n2] * t2c[n2]) / bt1data[index1[1]].data1;
					}
				}
			}
		}
		
		f64 t2e = bt2data[index2[0]].data1;
		f64 t2f = t2e * t2e; f64 t2g = 1.0 - t2f;
		f64 t2h = sqrt(t2g);
		
		// 1st basis vector... 1st basis vector... 1st basis vector... 1st basis vector...
		// 1st basis vector... 1st basis vector... 1st basis vector... 1st basis vector...
		// 1st basis vector... 1st basis vector... 1st basis vector... 1st basis vector...
		
		bt3_vector[n1].bv[0][0] = (t2a[0] - t2e * t2c[0]) / t2h;
		bt3_vector[n1].bv[0][1] = (t2a[1] - t2e * t2c[1]) / t2h;
		bt3_vector[n1].bv[0][2] = (t2a[2] - t2e * t2c[2]) / t2h;
		
		if (p1 > 0)
		{
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t3a = -t2e * bt2data[index2[0]].data2[0][n2] / t2h;		// D sin(alpha)
				f64 t3b = -t2e * bt2data[index2[0]].data2[2][n2] / t2h;		// D sin(alpha)
				
				for (i32s n3 = 0;n3 < 3;n3++)
				{
					f64 t4a = t2h * t2b[n2][n3] - t2a[n3] * t3a;
					f64 t4b = t2h * bt2data[index2[0]].data2[0][n2] - t2e * t3a;
					f64 t4c = (t4a - t2c[n3] * t4b) / t2g;
					
					f64 t5a = t3b * (t2a[n3] - t2e * t2c[n3]);
					f64 t5b = t2h * (t2e * t2d[n2][n3] + t2c[n3] * bt2data[index2[0]].data2[2][n2]);
					f64 t5c = -(t5a + t5b) / t2g;
					
					bt3_vector[n1].dbv[0][0][n2][n3] = t4c;
					bt3_vector[n1].dbv[0][1][n2][n3] = -(t4c + t5c);
					bt3_vector[n1].dbv[0][2][n2][n3] = t5c;
				}
			}
		}
		
		// 2nd basis vector... 2nd basis vector... 2nd basis vector... 2nd basis vector...
		// 2nd basis vector... 2nd basis vector... 2nd basis vector... 2nd basis vector...
		// 2nd basis vector... 2nd basis vector... 2nd basis vector... 2nd basis vector...
		
		f64 t2i[3];
		t2i[0] = bt1data[index1[0]].data2[dir1[0]][1] * bt1data[index1[1]].data2[dir1[1]][2] -
			bt1data[index1[0]].data2[dir1[0]][2] * bt1data[index1[1]].data2[dir1[1]][1];
		t2i[1] = bt1data[index1[0]].data2[dir1[0]][2] * bt1data[index1[1]].data2[dir1[1]][0] -
			bt1data[index1[0]].data2[dir1[0]][0] * bt1data[index1[1]].data2[dir1[1]][2];
		t2i[2] = bt1data[index1[0]].data2[dir1[0]][0] * bt1data[index1[1]].data2[dir1[1]][1] -
			bt1data[index1[0]].data2[dir1[0]][1] * bt1data[index1[1]].data2[dir1[1]][0];
		
		bt3_vector[n1].bv[1][0] = t2i[0] / t2h;
		bt3_vector[n1].bv[1][1] = t2i[1] / t2h;
		bt3_vector[n1].bv[1][2] = t2i[2] / t2h;
		
		if (p1 > 0)
		{
			for (i32s n2 = 0;n2 < 3;n2++)
			{
	f64 t3a[2];	// ???epsilon i
	t3a[0] = bt2data[index2[0]].data2[0][n2] * bt2data[index2[0]].data1 / t2g;
	t3a[1] = bt2data[index2[0]].data2[2][n2] * bt2data[index2[0]].data1 / t2g;
	
	f64 t3b[2];	// ???sigma ii / r2X
	t3b[0] = (1.0 - bt1data[index1[0]].data2[dir1[0]][n2] * bt1data[index1[0]].data2[dir1[0]][n2]) / bt1data[index1[0]].data1;
	t3b[1] = (1.0 - bt1data[index1[1]].data2[dir1[1]][n2] * bt1data[index1[1]].data2[dir1[1]][n2]) / bt1data[index1[1]].data1;
	
	i32s n9[2];
	n9[0] = (n2 + 1) % 3;	// index j
	n9[1] = (n2 + 2) % 3;	// index k
	
	f64 t3c[2];	// ???sigma ij / r2X
	t3c[0] = -bt1data[index1[0]].data2[dir1[0]][n2] * bt1data[index1[0]].data2[dir1[0]][n9[0]] / bt1data[index1[0]].data1;
	t3c[1] = -bt1data[index1[1]].data2[dir1[1]][n2] * bt1data[index1[1]].data2[dir1[1]][n9[0]] / bt1data[index1[1]].data1;
	
	f64 t3d[2];	// ???sigma ik / r2X
	t3d[0] = -bt1data[index1[0]].data2[dir1[0]][n2] * bt1data[index1[0]].data2[dir1[0]][n9[1]] / bt1data[index1[0]].data1;
	t3d[1] = -bt1data[index1[1]].data2[dir1[1]][n2] * bt1data[index1[1]].data2[dir1[1]][n9[1]] / bt1data[index1[1]].data1;
	
	bt3_vector[n1].dbv[1][0][n2][n2] = (t3c[0] * bt1data[index1[1]].data2[dir1[1]][n9[1]] -
		t3d[0] * bt1data[index1[1]].data2[dir1[1]][n9[0]] + t2i[n2] * t3a[0]) / t2h;
	bt3_vector[n1].dbv[1][0][n2][n9[0]] = (t3d[0] * bt1data[index1[1]].data2[dir1[1]][n2] -
		t3b[0] * bt1data[index1[1]].data2[dir1[1]][n9[1]] + t2i[n9[0]] * t3a[0]) / t2h;
	bt3_vector[n1].dbv[1][0][n2][n9[1]] = (t3b[0] * bt1data[index1[1]].data2[dir1[1]][n9[0]] -
		t3c[0] * bt1data[index1[1]].data2[dir1[1]][n2] + t2i[n9[1]] * t3a[0]) / t2h;
	
	bt3_vector[n1].dbv[1][2][n2][n2] = (t3d[1] * bt1data[index1[0]].data2[dir1[0]][n9[0]] -
		t3c[1] * bt1data[index1[0]].data2[dir1[0]][n9[1]] + t2i[n2] * t3a[1]) / t2h;
	bt3_vector[n1].dbv[1][2][n2][n9[0]] = (t3b[1] * bt1data[index1[0]].data2[dir1[0]][n9[1]] -
		t3d[1] * bt1data[index1[0]].data2[dir1[0]][n2] + t2i[n9[0]] * t3a[1]) / t2h;
	bt3_vector[n1].dbv[1][2][n2][n9[1]] = (t3c[1] * bt1data[index1[0]].data2[dir1[0]][n2] -
		t3b[1] * bt1data[index1[0]].data2[dir1[0]][n9[0]] + t2i[n9[1]] * t3a[1]) / t2h;
			}
			
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				bt3_vector[n1].dbv[1][1][n2][0] = -(bt3_vector[n1].dbv[1][0][n2][0] + bt3_vector[n1].dbv[1][2][n2][0]);
				bt3_vector[n1].dbv[1][1][n2][1] = -(bt3_vector[n1].dbv[1][0][n2][1] + bt3_vector[n1].dbv[1][2][n2][1]);
				bt3_vector[n1].dbv[1][1][n2][2] = -(bt3_vector[n1].dbv[1][0][n2][2] + bt3_vector[n1].dbv[1][2][n2][2]);
			}
		}
		
		// dipole vector... dipole vector... dipole vector... dipole vector...
		// dipole vector... dipole vector... dipole vector... dipole vector...
		// dipole vector... dipole vector... dipole vector... dipole vector...
		
		f64 csd = cos(bt3_vector[n1].pbdd);
		f64 snd = sin(bt3_vector[n1].pbdd);
		
		bt3_vector[n1].dv[0] = csd * bt3_vector[n1].bv[0][0] - snd * bt3_vector[n1].bv[1][0];
		bt3_vector[n1].dv[1] = csd * bt3_vector[n1].bv[0][1] - snd * bt3_vector[n1].bv[1][1];
		bt3_vector[n1].dv[2] = csd * bt3_vector[n1].bv[0][2] - snd * bt3_vector[n1].bv[1][2];
		
////////// debug!!!
//energy_bt3 += bt3_vector[n1].dv[0] + bt3_vector[n1].dv[1] + bt3_vector[n1].dv[2];
////////// debug!!!

		if (p1 > 0)
		{
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				for (i32s n3 = 0;n3 < 3;n3++)
				{
	bt3_vector[n1].ddv[0][n2][n3] = csd * bt3_vector[n1].dbv[0][0][n2][n3] - snd * bt3_vector[n1].dbv[1][0][n2][n3];
	bt3_vector[n1].ddv[1][n2][n3] = csd * bt3_vector[n1].dbv[0][1][n2][n3] - snd * bt3_vector[n1].dbv[1][1][n2][n3];
	bt3_vector[n1].ddv[2][n2][n3] = csd * bt3_vector[n1].dbv[0][2][n2][n3] - snd * bt3_vector[n1].dbv[1][2][n2][n3];
	bt3_vector[n1].ddv[3][n2][n3] = 0.0;
				}
			}
			
			if (bt3_vector[n1].dip_ttype == TTYPE_LOOP)
			{
				for (i32s n2 = 0;n2 < 3;n2++)
				{
					for (i32s n3 = 0;n3 < 3;n3++)
					{
	bt3_vector[n1].ddv[0][n2][n3] -= snd * dpbdd[0][n2] * bt3_vector[n1].bv[0][n3] + csd * dpbdd[0][n2] * bt3_vector[n1].bv[1][n3];
	bt3_vector[n1].ddv[1][n2][n3] -= snd * dpbdd[1][n2] * bt3_vector[n1].bv[0][n3] + csd * dpbdd[1][n2] * bt3_vector[n1].bv[1][n3];
	bt3_vector[n1].ddv[2][n2][n3] -= snd * dpbdd[2][n2] * bt3_vector[n1].bv[0][n3] + csd * dpbdd[2][n2] * bt3_vector[n1].bv[1][n3];
	bt3_vector[n1].ddv[3][n2][n3] -= snd * dpbdd[3][n2] * bt3_vector[n1].bv[0][n3] + csd * dpbdd[3][n2] * bt3_vector[n1].bv[1][n3];
					}
				}
			}
			
////////// debug!!!
//for (i32s n2 = 0;n2 < 3;n2++) {
//for (i32s n3 = 0;n3 < 3;n3++) {
//d1[l2g_sf[atmi[0]] * 3 + n2] += bt3_vector[n1].ddv[0][n2][n3];
//d1[l2g_sf[atmi[1]] * 3 + n2] += bt3_vector[n1].ddv[1][n2][n3];
//d1[l2g_sf[atmi[2]] * 3 + n2] += bt3_vector[n1].ddv[2][n2][n3];
//d1[l2g_sf[atmi[3]] * 3 + n2] += bt3_vector[n1].ddv[3][n2][n3]; } }
////////// debug!!!

		}
	}
}

// it seems that we compute here the same cross product than in BT3... combine ??????????
// it seems that we compute here the same cross product than in BT3... combine ??????????
// it seems that we compute here the same cross product than in BT3... combine ??????????

void eng1_sf::ComputeBT4(i32u p1)
{
	energy_bt4a = 0.0;
	energy_bt4b = 0.0;
	
	for (i32u n1 = 0;n1 < bt4_vector.size();n1++)
	{
		i32s * atma = bt2_vector[bt4_vector[n1].index2].atmi;
		i32s atmb = bt1_vector[bt4_vector[n1].index1].atmi[1];
		
		i32s * index1a = bt2_vector[bt4_vector[n1].index2].index1;
		bool * dir1a = bt2_vector[bt4_vector[n1].index2].dir1;
		
		i32s index1b = bt4_vector[n1].index1;
		i32s index2 = bt4_vector[n1].index2;
		
		f64 t1a[3];
		f64 t1b = 1.0 - bt2data[index2].data1 * bt2data[index2].data1; f64 t1c = sqrt(t1b);
		t1a[0] = bt1data[index1a[0]].data2[dir1a[0]][1] * bt1data[index1a[1]].data2[dir1a[1]][2] -
			bt1data[index1a[0]].data2[dir1a[0]][2] * bt1data[index1a[1]].data2[dir1a[1]][1];
		t1a[1] = bt1data[index1a[0]].data2[dir1a[0]][2] * bt1data[index1a[1]].data2[dir1a[1]][0] -
			bt1data[index1a[0]].data2[dir1a[0]][0] * bt1data[index1a[1]].data2[dir1a[1]][2];
		t1a[2] = bt1data[index1a[0]].data2[dir1a[0]][0] * bt1data[index1a[1]].data2[dir1a[1]][1] -
			bt1data[index1a[0]].data2[dir1a[0]][1] * bt1data[index1a[1]].data2[dir1a[1]][0];
		
		f64 t2a[3];
		f64 t2b = 2.0 * (bt2data[index2].data1 + 1.0); f64 t2c = sqrt(t2b);
		t2a[0] = bt1data[index1a[0]].data2[dir1a[0]][0] + bt1data[index1a[1]].data2[dir1a[1]][0];
		t2a[1] = bt1data[index1a[0]].data2[dir1a[0]][1] + bt1data[index1a[1]].data2[dir1a[1]][1];
		t2a[2] = bt1data[index1a[0]].data2[dir1a[0]][2] + bt1data[index1a[1]].data2[dir1a[1]][2];
		
		static const f64 sb = sin(M_PI * BETA / 180.0);		// sin(beta) = cos(gamma)
		static const f64 cb = cos(M_PI * BETA / 180.0);		// cos(beta) = sin(gamma)
		
		f64 t3a[3];		// vector e
		t3a[0] = cb * t1a[0] / t1c - sb * t2a[0] / t2c;
		t3a[1] = cb * t1a[1] / t1c - sb * t2a[1] / t2c;
		t3a[2] = cb * t1a[2] / t1c - sb * t2a[2] / t2c;
		
		f64 t3b[3];		// cos(kappa)
		t3b[0] = t3a[0] * bt1data[index1b].data2[1][0];
		t3b[1] = t3a[1] * bt1data[index1b].data2[1][1];
		t3b[2] = t3a[2] * bt1data[index1b].data2[1][2];
		f64 t3c = t3b[0] + t3b[1] + t3b[2];
		
		if (t3c < -1.0) t3c = -1.0;	// domain check...
		if (t3c > +1.0) t3c = +1.0;	// domain check...
		
// here like in BT2 if the kappa-angles approach 180.0 deg we will have some
// serious numerical problems in later stages. here is how we can monitor this...

bool problem1 = (t3c < -0.999);		// is it too close to 180 deg ???
if (problem1) { cout << "BUG: BT4 ang -> 180.0 deg." << endl; exit(EXIT_FAILURE); }

		// f = a(x-b)^2
		// df/dx = 2a(x-b)
		
		f64 t3g = t3c - bt4_vector[n1].opt;
		f64 t9a = bt4_vector[n1].fc * t3g * t3g;
		f64 t9b = 2.0 * bt4_vector[n1].fc * t3g;
		
		energy_bt4a += t9a;
		
		// f = a/(x-1)^2
		// df/dx = -2a/(x-1)^3
		
		// rep-fc is here (45->0) smaller than in BT2 (120->180).
		// rep-fc is here (45->0) smaller than in BT2 (120->180).
		// rep-fc is here (45->0) smaller than in BT2 (120->180).
		const f64 rep_weight = 0.030;
		
		f64 t9x = t3c - 1.0;
		
		f64 t9y = t9x * t9x;
		energy_bt4a += rep_weight * myprm->wrep / t9y;
		
		f64 t9z = t9y * t9x;
		t9b -= 2.0 * rep_weight * myprm->wrep / t9z;
		
		f64 t3d = 1.0 - t3c * t3c;	// sin(kappa)^2
		f64 t3e = sqrt(t3d);		// sin(kappa)
		
// here we will calculate cos(lambda) with the derivatives. however, we also have terms which
// depend on sin(lambda), and there we will run into problems at 0 deg and 180 deg.

// this problem is mainly due to bad design, but the sine-terms are still quite important...

// easiest way out is to cheat a bit and use the LOWLIMIT-idea also here... this is certainly
// NOT an ideal fix, since it will affect also accuracy, but still better than dividing by zero.

		f64 t4a = 0.0;			// cos(lambda)
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			t4a += sb / t3e * bt1data[index1b].data2[1][n2] * t1a[n2] / t1c;
			t4a += cb / t3e * bt1data[index1b].data2[1][n2] * t2a[n2] / t2c;
		}
		
		if (t4a < -1.0) t4a = -1.0;	// domain check...
		if (t4a > +1.0) t4a = +1.0;	// domain check...
		
		f64 t4b[3];		// vector h
		t4b[0] = (bt1data[index1b].data2[1][0] - t3a[0] * t3c) / t3e;
		t4b[1] = (bt1data[index1b].data2[1][1] - t3a[1] * t3c) / t3e;
		t4b[2] = (bt1data[index1b].data2[1][2] - t3a[2] * t3c) / t3e;
		
		f64 t4c[3];		// vector k
		t4c[0] = (t2a[1] * t1a[2] - t2a[2] * t1a[1]) / (t2c * t1c);
		t4c[1] = (t2a[2] * t1a[0] - t2a[0] * t1a[2]) / (t2c * t1c);
		t4c[2] = (t2a[0] * t1a[1] - t2a[1] * t1a[0]) / (t2c * t1c);
		
		f64 t4d = t4b[0] * t4c[0] + t4b[1] * t4c[1] + t4b[2] * t4c[2];
		
		f64 t4e = sqrt(1.0 - t4a * t4a);	// sin(lambda)
		
		if (t4e > +1.0) t4e = +1.0;			// domain check...
		if (t4e < LOWLIMIT) t4e = LOWLIMIT;		// apply the LOWLIMIT cheat...
		if (t4d < 0.0) t4e = -t4e;			// sign check...
		
		f64 t9c = t4a * t4a;		// x*x
		f64 t9d = 4.0 * t9c - 1.0;	// 4*x*x-1
		
		// f1 = a*x + b*(2*x*x-1) + c*x*(4*x*x-3)
		// df1/dx = a + 4*b*x + 3*c*(4*x*x-1)
		
		f64 t9e = bt4_vector[n1].fscos[0] * t4a;
		t9e += bt4_vector[n1].fscos[1] * (2.0 * t9c - 1.0);
		t9e += bt4_vector[n1].fscos[2] * t4a * (4.0 * t9c - 3.0);
		
		f64 t9f = bt4_vector[n1].fscos[0];
		t9f += 4.0 * bt4_vector[n1].fscos[1] * t4a;
		t9f += 3.0 * bt4_vector[n1].fscos[2] * t9d;
		
		// f2 = d*Sy + e*2*Sy*x + f*Sy*(4*x*x-1)
		// df2/dx = -d*x/Sy + 2e*(-x*x/Sy+Sy) + f*(-x/Sy*(4*x*x-1)+8*x*Sy)
		
		t9e += bt4_vector[n1].fssin[0] * t4e;
		t9e += 2.0 * bt4_vector[n1].fssin[1] * t4e * t4a;
		t9e += bt4_vector[n1].fssin[2] * t4e * t9d;
		
		t9f -= bt4_vector[n1].fssin[0] * t4a / t4e;
		t9f += 2.0 * bt4_vector[n1].fssin[1] * (t4e - t9c / t4e);
		t9f += bt4_vector[n1].fssin[2] * (8.0 * t4a * t4e - t4a * t9d / t4e);
		
		energy_bt4b += t9e;
		
		if (p1 > 0)
		{
			for (i32s n2 = 0;n2 < 3;n2++)
			{
	f64 t6a[2];	// epsilon i
	t6a[0] = bt2data[index2].data2[0][n2] * bt2data[index2].data1 / t1b;
	t6a[1] = bt2data[index2].data2[2][n2] * bt2data[index2].data1 / t1b;
	
	f64 t6b[2];	// sigma ii / r2X
	t6b[0] = (1.0 - bt1data[index1a[0]].data2[dir1a[0]][n2] * bt1data[index1a[0]].data2[dir1a[0]][n2]) / bt1data[index1a[0]].data1;
	t6b[1] = (1.0 - bt1data[index1a[1]].data2[dir1a[1]][n2] * bt1data[index1a[1]].data2[dir1a[1]][n2]) / bt1data[index1a[1]].data1;
	
	i32s n3[2];
	n3[0] = (n2 + 1) % 3;		// index j
	n3[1] = (n2 + 2) % 3;		// index k
	
	f64 t6c[2];	// sigma ij / r2X
	t6c[0] = -bt1data[index1a[0]].data2[dir1a[0]][n2] * bt1data[index1a[0]].data2[dir1a[0]][n3[0]] / bt1data[index1a[0]].data1;
	t6c[1] = -bt1data[index1a[1]].data2[dir1a[1]][n2] * bt1data[index1a[1]].data2[dir1a[1]][n3[0]] / bt1data[index1a[1]].data1;
	
	f64 t6d[2];	// sigma ik / r2X
	t6d[0] = -bt1data[index1a[0]].data2[dir1a[0]][n2] * bt1data[index1a[0]].data2[dir1a[0]][n3[1]] / bt1data[index1a[0]].data1;
	t6d[1] = -bt1data[index1a[1]].data2[dir1a[1]][n2] * bt1data[index1a[1]].data2[dir1a[1]][n3[1]] / bt1data[index1a[1]].data1;
	
	f64 t5a[2][3];	// components of dc/di
	t5a[0][n2] = (t6c[0] * bt1data[index1a[1]].data2[dir1a[1]][n3[1]] -
		t6d[0] * bt1data[index1a[1]].data2[dir1a[1]][n3[0]] + t1a[n2] * t6a[0]) / t1c;
	t5a[0][n3[0]] = (t6d[0] * bt1data[index1a[1]].data2[dir1a[1]][n2] -
		t6b[0] * bt1data[index1a[1]].data2[dir1a[1]][n3[1]] + t1a[n3[0]] * t6a[0]) / t1c;
	t5a[0][n3[1]] = (t6b[0] * bt1data[index1a[1]].data2[dir1a[1]][n3[0]] -
		t6c[0] * bt1data[index1a[1]].data2[dir1a[1]][n2] + t1a[n3[1]] * t6a[0]) / t1c;
	t5a[1][n2] = (t6d[1] * bt1data[index1a[0]].data2[dir1a[0]][n3[0]] -
		t6c[1] * bt1data[index1a[0]].data2[dir1a[0]][n3[1]] + t1a[n2] * t6a[1]) / t1c;
	t5a[1][n3[0]] = (t6b[1] * bt1data[index1a[0]].data2[dir1a[0]][n3[1]] -
		t6d[1] * bt1data[index1a[0]].data2[dir1a[0]][n2] + t1a[n3[0]] * t6a[1]) / t1c;
	t5a[1][n3[1]] = (t6c[1] * bt1data[index1a[0]].data2[dir1a[0]][n2] -
		t6b[1] * bt1data[index1a[0]].data2[dir1a[0]][n3[0]] + t1a[n3[1]] * t6a[1]) / t1c;
	
	f64 t5b[2][3];	// components of dd/di
	f64 t6e = (bt1data[index1a[0]].data2[dir1a[0]][n2] + bt1data[index1a[1]].data2[dir1a[1]][n2]) / t2b;
	t5b[0][n2] = (t6b[0] - bt2data[index2].data2[0][n2] * t6e) / t2c;
	t5b[1][n2] = (t6b[1] - bt2data[index2].data2[2][n2] * t6e) / t2c;
	t6e = (bt1data[index1a[0]].data2[dir1a[0]][n3[0]] + bt1data[index1a[1]].data2[dir1a[1]][n3[0]]) / t2b;
	t5b[0][n3[0]] = (t6c[0] - bt2data[index2].data2[0][n2] * t6e) / t2c;
	t5b[1][n3[0]] = (t6c[1] - bt2data[index2].data2[2][n2] * t6e) / t2c;
	t6e = (bt1data[index1a[0]].data2[dir1a[0]][n3[1]] + bt1data[index1a[1]].data2[dir1a[1]][n3[1]]) / t2b;
	t5b[0][n3[1]] = (t6d[0] - bt2data[index2].data2[0][n2] * t6e) / t2c;
	t5b[1][n3[1]] = (t6d[1] - bt2data[index2].data2[2][n2] * t6e) / t2c;
				
				f64 t5c[4] = { 0.0, 0.0, 0.0, 0.0 };		// dcos(kappa)/di
				for (i32s n3 = 0;n3 < 3;n3++)
				{
					t5c[0] += bt1data[index1b].data2[1][n3] * (cb * t5a[0][n3] - sb * t5b[0][n3]);
					t5c[2] += bt1data[index1b].data2[1][n3] * (cb * t5a[1][n3] - sb * t5b[1][n3]);
					
					if (n2 != n3) t6e = -bt1data[index1b].data2[1][n2] * bt1data[index1b].data2[1][n3];
					else t6e = 1.0 - bt1data[index1b].data2[1][n2] * bt1data[index1b].data2[1][n2];
					t5c[3] += t3a[n3] * t6e / bt1data[index1b].data1;
				}
				
				t5c[1] = -(t5c[0] + t5c[2] + t5c[3]);
				
				d1[l2g_sf[atma[0]] * 3 + n2] += t9b * t5c[0];
				d1[l2g_sf[atma[1]] * 3 + n2] += t9b * t5c[1];
				d1[l2g_sf[atma[2]] * 3 + n2] += t9b * t5c[2];
				d1[l2g_sf[atmb] * 3 + n2] += t9b * t5c[3];
				
				f64 t7e[3];
				t7e[0] = t3c * t5c[0] / t3d;
				t7e[1] = t3c * t5c[2] / t3d;
				t7e[2] = t3c * t5c[3] / t3d;
				
				f64 t5d[4] = { 0.0, 0.0, 0.0, 0.0 };		// dcos(lambda)/di
				for (i32s n3 = 0;n3 < 3;n3++)
				{
					f64 t7a = t1a[n3] / t1c;	// f64 t7b = t7a * t7a;
					f64 t7c = t2a[n3] / t2c;	// f64 t7d = t7c * t7c;
					
					t5d[0] += sb * bt1data[index1b].data2[1][n3] * (t7e[0] * t7a + t5a[0][n3]) / t3e;
					t5d[0] += cb * bt1data[index1b].data2[1][n3] * (t7e[0] * t7c + t5b[0][n3]) / t3e;
					
					t5d[2] += sb * bt1data[index1b].data2[1][n3] * (t7e[1] * t7a + t5a[1][n3]) / t3e;
					t5d[2] += cb * bt1data[index1b].data2[1][n3] * (t7e[1] * t7c + t5b[1][n3]) / t3e;
					
					if (n2 != n3) t6e = -bt1data[index1b].data2[1][n2] * bt1data[index1b].data2[1][n3];
					else t6e = 1.0 - bt1data[index1b].data2[1][n2] * bt1data[index1b].data2[1][n2];
					
					f64 t7f = t7e[2] * bt1data[index1b].data2[1][n3] + t6e / bt1data[index1b].data1;
					t5d[3] += (sb * t7a + cb * t7c) * t7f / t3e;
				}
				
				t5d[1] = -(t5d[0] + t5d[2] + t5d[3]);
				
				d1[l2g_sf[atma[0]] * 3 + n2] += t9f * t5d[0];
				d1[l2g_sf[atma[1]] * 3 + n2] += t9f * t5d[1];
				d1[l2g_sf[atma[2]] * 3 + n2] += t9f * t5d[2];
				d1[l2g_sf[atmb] * 3 + n2] += t9f * t5d[3];
			}
		}
	}
}

void eng1_sf::ComputeNBT1(i32u p1)
{
	energy_nbt1a = 0.0;
	energy_nbt1b = 0.0;
	energy_nbt1c = 0.0;	// not needed anymore???
	
	atom ** atmtab = GetSetup()->GetSFAtoms();
	
	// an additional pass for the boundary potential (optional).
	// an additional pass for the boundary potential (optional).
	// an additional pass for the boundary potential (optional).
	
	if (use_simple_bp)
	{
		if (nd_eval != NULL) nd_eval->AddCycle();
		for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount();n1++)
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
				f64 t9b = crd[l2g_sf[n1] * 3 + n2];
				
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
			
			if (p1 > 0)
			{
				f64 t2c = 2.0 * fc * t2a;
				
				for (i32s n2 = 0;n2 < 3;n2++)
				{
					f64 t2d = (t1a[n2] / t1c) * t2c;
					
					d1[l2g_sf[n1] * 3 + n2] -= t2d;
				}
			}
		}
	}
	
	i32s limit = GetSetup()->GetSFAtomCount() - num_solvent;
	for (i32u n1 = 0;n1 < nbt1_vector.size();n1++)
	{
		i32s * atmi = nbt1_vector[n1].atmi;
		
		f64 t1a[3]; f64 t1b = 0.0;
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			f64 t2a = crd[l2g_sf[atmi[0]] * 3 + n2];
			f64 t2b = crd[l2g_sf[atmi[1]] * 3 + n2];
			
			t1a[n2] = t2a - t2b;
			t1b += t1a[n2] * t1a[n2];
		}
		
		f64 t1c = sqrt(t1b);
		
		// f = ((x+s)/a)^-9 - ((x+s)/b)^-6
		// df/dx = -9/a((x+s)/a)^-10 + 6/b((x+s)/b)^-7
		
		const f64 buff = 0.005;
		f64 tmp1 = t1c + buff;
		
		f64 t3a = tmp1 / nbt1_vector[n1].data[0];
		f64 t3b = tmp1 / nbt1_vector[n1].data[1];
		
		f64 t4a = t3a * t3a; f64 t4b = t4a * t3a; f64 t4c = t4a * t4b;		// ^2 ^3 ^5
		f64 t5a = t3b * t3b; f64 t5b = t5a * t3b; //f64 t5c = t5a * t5b;	// ^2 ^3 ^5
		
	//	energy_nbt1a += 1.0 / (t4b * t4b * t4b) - 1.0 / (t5b * t5b);			// 6-9 << also see InitLenJon()!!!
		energy_nbt1a += 1.0 / (t4c * t4c * t4a) - 1.0 / (t5b * t5b);			// 6-12 << also see InitLenJon()!!!
	//	energy_nbt1a += 1.0 / (t4c * t4c * t4a) - 1.0 / (t5b * t5b * t5b);		// 9-12 << also see InitLenJon()!!!
		
		f64 t6c = 0.0;
		if (atmi[0] < limit && atmi[1] < limit)
		{
			// e = 2 + 76 * (( r / A ) ^ n) / (1 + ( r / A ) ^ n) = 2 + 76 * f / g
			// de/dr = 76 * (( g * Df - f * Dg ) / g ^ 2 )
			
	f64 e1 = solv_exp[0][atmi[0]];
	f64 e2 = solv_exp[0][atmi[1]];
	
	f64 eps_n = myprm->epsilon1 + t1b * myprm->epsilon2;							// assume constant!!!
	f64 eps_A = myprm->epsilon3 - log(1.0 + (e1 + e2) * myprm->epsilon4 + e1 * e2 * myprm->epsilon5);	// assume constant!!!
if (eps_A < 0.2) { cout << "BUGGER!!! " << eps_A << endl; exit(EXIT_FAILURE); }

	f64 t7a = t1c / eps_A;
	f64 t7b = pow(t7a, eps_n);
	f64 t7c = 1.0 + t7b;
	f64 t7d = 2.0 + 76.0 * (t7b / t7c);
	
	f64 t7x = eps_n * pow(t7a, eps_n - 1.0) / eps_A;
	f64 t7y = 76.0 * ((t7c * t7x - t7b * t7x) / (t7c * t7c));
		
			f64 qq = 138.9354518 * charge2[atmi[0]] * charge2[atmi[1]] * myprm->wescc;
		
			// f = Q/(e*r)
			// df/dr = -Q * ((1/e*r^2) + (de/dr)/(e^2*r))
		
			energy_nbt1b += qq / (t7d * t1c);
			
			t6c = -qq * (1.0 / (t7d * t1b) + t7y / (t7d * t7d * t1c));
		}
		
		if (p1 > 0)
		{
	//		f64 t6a = 9.0 / (nbt1_vector[n1].data[0] * t4c * t4c);			// 6-9
	//		f64 t6b = 6.0 / (nbt1_vector[n1].data[1] * t5a * t5a * t5b);
			f64 t6a = 12.0 / (nbt1_vector[n1].data[0] * t4c * t4c * t4b);		// 6-12
			f64 t6b = 6.0 / (nbt1_vector[n1].data[1] * t5a * t5a * t5b);
	//		f64 t6a = 12.0 / (nbt1_vector[n1].data[0] * t4c * t4c * t4b);		// 9-12
	//		f64 t6b = 9.0 / (nbt1_vector[n1].data[1] * t5c * t5c);
			
			f64 t2a = (t6b - t6a) + t6c;
			
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t1e = (t1a[n2] / t1c) * t2a;
				
				d1[l2g_sf[atmi[0]] * 3 + n2] += t1e;
				d1[l2g_sf[atmi[1]] * 3 + n2] -= t1e;
			}
		}
	}
}

void eng1_sf::ComputeNBT2(i32u p1)
{
	energy_nbt2a = 0.0;
	energy_nbt2b = 0.0;	// not needed anymore???
	energy_nbt2c = 0.0;
	
	// dipole moment for the peptide unit:
	// CRC handbook of Chem & Phys, 1st Student Edition, 1988, CRC Press Inc. (page E-53)
	// "acetyl methylamine" 3.73 debyes -> 3.73 * 3.338e-30 / 1.6021773e-19 * 1.0e+09 = 0.07771137
	
/*##############################################*/
/*##############################################*/

	for (i32u n1 = 0;n1 < bt3_vector.size();n1++)
	{
		if (bt3_vector[n1].skip) continue;	// skip all X-pro cases...
		i32s * atmi = bt3_vector[n1].atmi;
		
		for (i32s n2 = 0;n2 < GetSetup()->GetSFAtomCount() - num_solvent;n2++)
		{
			if ((charge2[n2] == 0.0)) continue;
			if (atmi[1] == (i32s) n2 || atmi[2] == (i32s) n2) continue;
			
	f64 t1a[3];
	t1a[0] = crd[l2g_sf[n2] * 3 + 0] - 0.5 * (crd[l2g_sf[atmi[1]] * 3 + 0] + crd[l2g_sf[atmi[2]] * 3 + 0]);
	t1a[1] = crd[l2g_sf[n2] * 3 + 1] - 0.5 * (crd[l2g_sf[atmi[1]] * 3 + 1] + crd[l2g_sf[atmi[2]] * 3 + 1]);
	t1a[2] = crd[l2g_sf[n2] * 3 + 2] - 0.5 * (crd[l2g_sf[atmi[1]] * 3 + 2] + crd[l2g_sf[atmi[2]] * 3 + 2]);
			
			f64 t1b = t1a[0] * t1a[0] + t1a[1] * t1a[1] + t1a[2] * t1a[2];
			f64 t1c = sqrt(t1b); f64 t1d[2][3]; f64 t1e[2][3][3];
			
			// t1d = dr/dx, and therefore t1d[0] is a unit vector...
			// t1d = dr/dx, and therefore t1d[0] is a unit vector...
			// t1d = dr/dx, and therefore t1d[0] is a unit vector...
			
			for (i32s n3 = 0;n3 < 3;n3++)
			{
				t1d[0][n3] = t1a[n3] / t1c;
				t1d[1][n3] = -0.5 * t1d[0][n3];
			}
			
			f64 t1f[3]; f64 t1g[5][3];			// theta
			t1f[0] = t1d[0][0] * bt3_vector[n1].dv[0];
			t1f[1] = t1d[0][1] * bt3_vector[n1].dv[1];
			t1f[2] = t1d[0][2] * bt3_vector[n1].dv[2];
			f64 t1h = t1f[0] + t1f[1] + t1f[2];
			
////////// debug!!!
//energy_nbt2 += t1h;
////////// debug!!!

			if (p1 > 0)
			{
				for (i32s n3 = 0;n3 < 3;n3++)
				{
					for (i32s n4 = 0;n4 < 3;n4++)
					{
						if (n3 != n4) t1e[0][n3][n4] = -t1d[0][n3] * t1d[0][n4] / t1c;
						else t1e[0][n3][n4] = (1.0 - t1d[0][n3] * t1d[0][n3]) / t1c;
						t1e[1][n3][n4] = -0.5 * t1e[0][n3][n4];
					}
				}
				
				for (i32s n3 = 0;n3 < 3;n3++)
				{
					t1g[0][n3] = t1g[1][n3] = t1g[2][n3] = t1g[3][n3] = t1g[4][n3] = 0.0;
					
					for (i32s n4 = 0;n4 < 3;n4++)
					{
	t1g[0][n3] += t1d[0][n4] * bt3_vector[n1].ddv[0][n3][n4];
	t1g[1][n3] += t1d[0][n4] * bt3_vector[n1].ddv[1][n3][n4] + t1e[1][n3][n4] * bt3_vector[n1].dv[n4];
	t1g[2][n3] += t1d[0][n4] * bt3_vector[n1].ddv[2][n3][n4] + t1e[1][n3][n4] * bt3_vector[n1].dv[n4];
	t1g[3][n3] += t1d[0][n4] * bt3_vector[n1].ddv[3][n3][n4];
	t1g[4][n3] += t1e[0][n3][n4] * bt3_vector[n1].dv[n4];
					}
					
////////// debug!!!
//d1[l2g_sf[atmi[0]] * 3 + n3] += t1g[0][n3];
//d1[l2g_sf[atmi[1]] * 3 + n3] += t1g[1][n3];
//d1[l2g_sf[atmi[2]] * 3 + n3] += t1g[2][n3];
//d1[l2g_sf[atmi[3]] * 3 + n3] += t1g[3][n3];
//d1[l2g_sf[n2] * 3 + n3] += t1g[4][n3];
////////// debug!!!

				}
			}
			
	f64 t9x = t1b * t1b;	// this is r^4 !!!!!!!
	f64 t9y = t9x * t9x;	// this is r^8 !!!!!!!
	
	// e = 2 + 76 * (( r / A ) ^ n) / (1 + ( r / A ) ^ n) + Z/r^9 = 2 + 76 * f / g + Z/r^9
	// de/dr = 76 * (( g * Df - f * Dg ) / g ^ 2 ) - 9Z/r^10
	
	f64 eps_n = myprm->epsilon1 + t1b * myprm->epsilon2;	// assume constant!!!
	f64 eps_A = 0.75;	// most chg-dip interactions at surface -> use a small value!
	
	f64 t7a = t1c / eps_A;
	f64 t7b = pow(t7a, eps_n);
	f64 t7c = 1.0 + t7b;
	f64 t7d = 2.0 + 76.0 * (t7b / t7c) + myprm->epsilon9 / (t9y * t1c);
	
	f64 t7x = eps_n * pow(t7a, eps_n - 1.0) / eps_A;
	f64 t7y = 76.0 * ((t7c * t7x - t7b * t7x) / (t7c * t7c)) - 9.0 * myprm->epsilon9 / (t9y * t1b);
			
			f64 qd = 138.9354518 * charge2[n2] * 0.077711 * myprm->dipole1 * myprm->wescd;
			
			// f = Q/(e*r^2)
			// df/dr = -Q * ((2/e*r^3) + (de/dr)/(e^2*r^2))
			
			energy_nbt2a += qd * t1h / (t7d * t1b);
			
			if (p1 > 0)
			{
				f64 t3a = -qd * t1h * (2.0 / (t7d * t1b * t1c) + t7y / (t7d * t7d * t1b));
				f64 t3b = qd / (t7d * t1b);
				
				for (i32s n3 = 0;n3 < 3;n3++)
				{
					d1[l2g_sf[atmi[1]] * 3 + n3] += t3a * t1d[1][n3];
					d1[l2g_sf[atmi[2]] * 3 + n3] += t3a * t1d[1][n3];
					d1[l2g_sf[n2] * 3 + n3] += t3a * t1d[0][n3];
					
					d1[l2g_sf[atmi[0]] * 3 + n3] += t3b * t1g[0][n3];
					d1[l2g_sf[atmi[1]] * 3 + n3] += t3b * t1g[1][n3];
					d1[l2g_sf[atmi[2]] * 3 + n3] += t3b * t1g[2][n3];
					d1[l2g_sf[atmi[3]] * 3 + n3] += t3b * t1g[3][n3];
					d1[l2g_sf[n2] * 3 + n3] += t3b * t1g[4][n3];
				}
			}
		}
	}
	
/*##############################################*/
/*##############################################*/

	for (i32s n1 = 0;n1 < ((i32s) bt3_vector.size()) - 1;n1++)
	{
		if (bt3_vector[n1].skip) continue;		// skip all X-pro cases...
		i32s * atmi1 = bt3_vector[n1].atmi;
		
		for (i32s n2 = n1 + 1;n2 < (i32s) bt3_vector.size();n2++)
		{
			if (bt3_vector[n2].skip) continue;	// skip all X-pro cases...
			i32s * atmi2 = bt3_vector[n2].atmi;
			
	f64 t1a[3];
	t1a[0] = 0.5 * (crd[l2g_sf[atmi2[1]] * 3 + 0] + crd[l2g_sf[atmi2[2]] * 3 + 0] - crd[l2g_sf[atmi1[1]] * 3 + 0] - crd[l2g_sf[atmi1[2]] * 3 + 0]);
	t1a[1] = 0.5 * (crd[l2g_sf[atmi2[1]] * 3 + 1] + crd[l2g_sf[atmi2[2]] * 3 + 1] - crd[l2g_sf[atmi1[1]] * 3 + 1] - crd[l2g_sf[atmi1[2]] * 3 + 1]);
	t1a[2] = 0.5 * (crd[l2g_sf[atmi2[1]] * 3 + 2] + crd[l2g_sf[atmi2[2]] * 3 + 2] - crd[l2g_sf[atmi1[1]] * 3 + 2] - crd[l2g_sf[atmi1[2]] * 3 + 2]);
			
			f64 t1b = t1a[0] * t1a[0] + t1a[1] * t1a[1] + t1a[2] * t1a[2];
			f64 t1c = sqrt(t1b); f64 t1d[3]; f64 t1e[3][3];
			
			// t1d = dr/dx, and therefore is a vector of constant length 1/2...
			// t1d = dr/dx, and therefore is a vector of constant length 1/2...
			// t1d = dr/dx, and therefore is a vector of constant length 1/2...
			
			t1d[0] = 0.5 * t1a[0] / t1c;
			t1d[1] = 0.5 * t1a[1] / t1c;
			t1d[2] = 0.5 * t1a[2] / t1c;
			
			f64 t1f[3]; f64 t1g[5][3];			// theta1 * 1/2
			t1f[0] = t1d[0] * bt3_vector[n1].dv[0];
			t1f[1] = t1d[1] * bt3_vector[n1].dv[1];
			t1f[2] = t1d[2] * bt3_vector[n1].dv[2];
			f64 t1h = t1f[0] + t1f[1] + t1f[2];
			
			f64 t1i[3]; f64 t1j[5][3];			// theta2 * 1/2
			t1i[0] = t1d[0] * bt3_vector[n2].dv[0];
			t1i[1] = t1d[1] * bt3_vector[n2].dv[1];
			t1i[2] = t1d[2] * bt3_vector[n2].dv[2];
			f64 t1k = t1i[0] + t1i[1] + t1i[2];
			
			f64 t1l[3]; f64 t1m[8][3];
			t1l[0] = bt3_vector[n1].dv[0] * bt3_vector[n2].dv[0];
			t1l[1] = bt3_vector[n1].dv[1] * bt3_vector[n2].dv[1];
			t1l[2] = bt3_vector[n1].dv[2] * bt3_vector[n2].dv[2];
			f64 t1n = t1l[0] + t1l[1] + t1l[2];
			
////////// debug!!!
//energy_nbt2 += t1h;
//energy_nbt2 += t1k;
//energy_nbt2 += t1n;
////////// debug!!!

			if (p1 > 0)
			{
				for (i32s n3 = 0;n3 < 3;n3++)
				{
					for (i32s n4 = 0;n4 < 3;n4++)
					{
						if (n3 != n4) t1e[n3][n4] = -t1d[n3] * t1d[n4] / t1c;
						else t1e[n3][n4] = (0.25 - t1d[n3] * t1d[n3]) / t1c;
					}
				}
				
				for (i32s n3 = 0;n3 < 3;n3++)
				{
					t1g[0][n3] = t1g[1][n3] = t1g[2][n3] = t1g[3][n3] = t1g[4][n3] = 0.0;
					t1j[0][n3] = t1j[1][n3] = t1j[2][n3] = t1j[3][n3] = t1j[4][n3] = 0.0;
					
					t1m[0][n3] = t1m[1][n3] = t1m[2][n3] = t1m[3][n3] = 0.0;
					t1m[4][n3] = t1m[5][n3] = t1m[6][n3] = t1m[7][n3] = 0.0;
					
					for (i32s n4 = 0;n4 < 3;n4++)
					{
	t1g[0][n3] += t1d[n4] * bt3_vector[n1].ddv[0][n3][n4];
	t1g[1][n3] += t1d[n4] * bt3_vector[n1].ddv[1][n3][n4] - t1e[n3][n4] * bt3_vector[n1].dv[n4];
	t1g[2][n3] += t1d[n4] * bt3_vector[n1].ddv[2][n3][n4] - t1e[n3][n4] * bt3_vector[n1].dv[n4];
	t1g[3][n3] += t1d[n4] * bt3_vector[n1].ddv[3][n3][n4];
	t1g[4][n3] += t1e[n3][n4] * bt3_vector[n1].dv[n4];
	
	t1j[0][n3] += t1d[n4] * bt3_vector[n2].ddv[0][n3][n4];
	t1j[1][n3] += t1d[n4] * bt3_vector[n2].ddv[1][n3][n4] + t1e[n3][n4] * bt3_vector[n2].dv[n4];
	t1j[2][n3] += t1d[n4] * bt3_vector[n2].ddv[2][n3][n4] + t1e[n3][n4] * bt3_vector[n2].dv[n4];
	t1j[3][n3] += t1d[n4] * bt3_vector[n2].ddv[3][n3][n4];
	t1j[4][n3] -= t1e[n3][n4] * bt3_vector[n2].dv[n4];
	
	t1m[0][n3] += bt3_vector[n1].ddv[0][n3][n4] * bt3_vector[n2].dv[n4];
	t1m[1][n3] += bt3_vector[n1].ddv[1][n3][n4] * bt3_vector[n2].dv[n4];
	t1m[2][n3] += bt3_vector[n1].ddv[2][n3][n4] * bt3_vector[n2].dv[n4];
	t1m[3][n3] += bt3_vector[n1].ddv[3][n3][n4] * bt3_vector[n2].dv[n4];
	t1m[4][n3] += bt3_vector[n1].dv[n4] * bt3_vector[n2].ddv[0][n3][n4];
	t1m[5][n3] += bt3_vector[n1].dv[n4] * bt3_vector[n2].ddv[1][n3][n4];
	t1m[6][n3] += bt3_vector[n1].dv[n4] * bt3_vector[n2].ddv[2][n3][n4];
	t1m[7][n3] += bt3_vector[n1].dv[n4] * bt3_vector[n2].ddv[3][n3][n4];
					}
					
////////// debug!!!
//d1[l2g_sf[atmi1[0]] * 3 + n3] += t1g[0][n3];
//d1[l2g_sf[atmi1[1]] * 3 + n3] += t1g[1][n3];
//d1[l2g_sf[atmi1[2]] * 3 + n3] += t1g[2][n3];
//d1[l2g_sf[atmi1[3]] * 3 + n3] += t1g[3][n3];
//d1[l2g_sf[atmi2[1]] * 3 + n3] += t1g[4][n3];
//d1[l2g_sf[atmi2[2]] * 3 + n3] += t1g[4][n3];
//
//d1[l2g_sf[atmi2[0]] * 3 + n3] += t1j[0][n3];
//d1[l2g_sf[atmi2[1]] * 3 + n3] += t1j[1][n3];
//d1[l2g_sf[atmi2[2]] * 3 + n3] += t1j[2][n3];
//d1[l2g_sf[atmi2[3]] * 3 + n3] += t1j[3][n3];
//d1[l2g_sf[atmi1[1]] * 3 + n3] += t1j[4][n3];
//d1[l2g_sf[atmi1[2]] * 3 + n3] += t1j[4][n3];
//
//d1[l2g_sf[atmi1[0]] * 3 + n3] += t1m[0][n3];
//d1[l2g_sf[atmi1[1]] * 3 + n3] += t1m[1][n3];
//d1[l2g_sf[atmi1[2]] * 3 + n3] += t1m[2][n3];
//d1[l2g_sf[atmi1[3]] * 3 + n3] += t1m[3][n3];
//d1[l2g_sf[atmi2[0]] * 3 + n3] += t1m[4][n3];
//d1[l2g_sf[atmi2[1]] * 3 + n3] += t1m[5][n3];
//d1[l2g_sf[atmi2[2]] * 3 + n3] += t1m[6][n3];
//d1[l2g_sf[atmi2[3]] * 3 + n3] += t1m[7][n3];
////////// debug!!!

				}
			}
			
	f64 t9x = t1b * t1b;	// this is r^4 !!!!!!!
	f64 t9y = t9x * t9x;	// this is r^8 !!!!!!!
	
	// e = 2 + 76 * (( r / A ) ^ n) / (1 + ( r / A ) ^ n) + Z/r^9 = 2 + 76 * f / g + Z/r^9
	// de/dr = 76 * (( g * Df - f * Dg ) / g ^ 2 ) - 9Z/r^10
	
	f64 eps_n = myprm->epsilon1 + t1b * myprm->epsilon2;	// assume constant!!!
	f64 eps_A = 1.75;	// most dip-dip interactions at interior -> use a large value!
	
	f64 t7a = t1c / eps_A;
	f64 t7b = pow(t7a, eps_n);
	f64 t7c = 1.0 + t7b;
	f64 t7d = 2.0 + 76.0 * (t7b / t7c) + myprm->epsilon9 / (t9y * t1c);
	
	f64 t7x = eps_n * pow(t7a, eps_n - 1.0) / eps_A;
	f64 t7y = 76.0 * ((t7c * t7x - t7b * t7x) / (t7c * t7c)) - 9.0 * myprm->epsilon9 / (t9y * t1b);
			
			// f = Q/(e*r^3)
			// df/dr = -Q * ((3/e*r^4) + (de/dr)/(e^2*r^3))
			
			f64 dd = 138.9354518 * 0.077711 * 0.077711 * myprm->dipole1 * myprm->dipole1;
			
			// enhance alpha-helices...
			// enhance alpha-helices...
			// enhance alpha-helices...
			
			if ((n2 - n1) == 3)
			{
				bool at1 = (bt3_vector[n1 + 0].dip_ttype == TTYPE_HELIX);
				bool at2 = (bt3_vector[n1 + 1].dip_ttype == TTYPE_HELIX);
				bool at3 = (bt3_vector[n1 + 2].dip_ttype == TTYPE_HELIX);
				bool at4 = (bt3_vector[n1 + 3].dip_ttype == TTYPE_HELIX);
				
				bool same_chn = (bt3_vector[n1].atmi[3] == bt3_vector[n2].atmi[0]);
				
				if (at1 && at2 && at3 && at4 && same_chn) dd *= myprm->dipole2;
			}
			
			// enhance beta-sheets...
			// enhance beta-sheets...
			// enhance beta-sheets...
			
			bool bt1 = (bt3_vector[n1].dip_ttype == TTYPE_STRAND);
			bool bt2 = (bt3_vector[n2].dip_ttype == TTYPE_STRAND);
			if (bt1 && bt2) dd *= myprm->dipole2;
			
			f64 t1x = dd / (t7d * t1b * t1c);	// radial part...
			f64 t1y = t1n - 12.0 * t1h * t1k;	// angular part...
			energy_nbt2c += t1x * t1y;
			
			if (p1 > 0)
			{
				f64 t3a = -dd * t1y * (3.0 / (t7d * t9x) + t7y / (t7d * t7d * t1b * t1c));
				f64 t3b = -12.0 * t1k * t1x;
				f64 t3c = -12.0 * t1h * t1x;
				
				for (i32s n3 = 0;n3 < 3;n3++)
				{
					d1[l2g_sf[atmi1[1]] * 3 + n3] -= t3a * t1d[n3];
					d1[l2g_sf[atmi1[2]] * 3 + n3] -= t3a * t1d[n3];
					d1[l2g_sf[atmi2[1]] * 3 + n3] += t3a * t1d[n3];
					d1[l2g_sf[atmi2[2]] * 3 + n3] += t3a * t1d[n3];
					
					d1[l2g_sf[atmi1[0]] * 3 + n3] += t3b * t1g[0][n3];
					d1[l2g_sf[atmi1[1]] * 3 + n3] += t3b * t1g[1][n3];
					d1[l2g_sf[atmi1[2]] * 3 + n3] += t3b * t1g[2][n3];
					d1[l2g_sf[atmi1[3]] * 3 + n3] += t3b * t1g[3][n3];
					d1[l2g_sf[atmi2[1]] * 3 + n3] += t3b * t1g[4][n3];
					d1[l2g_sf[atmi2[2]] * 3 + n3] += t3b * t1g[4][n3];
					
					d1[l2g_sf[atmi2[0]] * 3 + n3] += t3c * t1j[0][n3];
					d1[l2g_sf[atmi2[1]] * 3 + n3] += t3c * t1j[1][n3];
					d1[l2g_sf[atmi2[2]] * 3 + n3] += t3c * t1j[2][n3];
					d1[l2g_sf[atmi2[3]] * 3 + n3] += t3c * t1j[3][n3];
					d1[l2g_sf[atmi1[1]] * 3 + n3] += t3c * t1j[4][n3];
					d1[l2g_sf[atmi1[2]] * 3 + n3] += t3c * t1j[4][n3];
					
					d1[l2g_sf[atmi1[0]] * 3 + n3] += t1x * t1m[0][n3];
					d1[l2g_sf[atmi1[1]] * 3 + n3] += t1x * t1m[1][n3];
					d1[l2g_sf[atmi1[2]] * 3 + n3] += t1x * t1m[2][n3];
					d1[l2g_sf[atmi1[3]] * 3 + n3] += t1x * t1m[3][n3];
					d1[l2g_sf[atmi2[0]] * 3 + n3] += t1x * t1m[4][n3];
					d1[l2g_sf[atmi2[1]] * 3 + n3] += t1x * t1m[5][n3];
					d1[l2g_sf[atmi2[2]] * 3 + n3] += t1x * t1m[6][n3];
					d1[l2g_sf[atmi2[3]] * 3 + n3] += t1x * t1m[7][n3];
				}
			}
		}
	}
}

// Richmond TJ : "Solvent Accessible Surface Area and Extended Volume in Proteins"
// J. Mol. Biol 178, 63-89 (1984)

// Fraczkiewicz R, Braun W : "Exact and Efficient Analytical Calculation of the Accessible
// Surface Areas and Their Gradients for Macromolecules" J. Comp. Chem 19, 319-333, (1998)

// Weiser J, Weiser AA, Shenkin PS, Still WC : "Neigbor-List Reduction: Optimization for
// Computation of Molecular van der Waals and Solvent-Accessible Surface Areas"
// J. Comp. Chem. 19, 797-808, (1998)

// Do Carmo MP : "Differential Geometry of Curves and Surfaces" Prentice Hall Inc., 1976

void eng1_sf::ComputeNBT3(i32u p1)
{
	i32s limit = GetSetup()->GetSFAtomCount() - num_solvent;
	for (i32u n1 = 0;n1 < nbt1_vector.size();n1++)
	{
		i32s * atmi = nbt1_vector[n1].atmi;
		
		if (atmi[0] < limit && atmi[1] < limit)
		{
			f64 t1a[3]; f64 t1b = 0.0;
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t2a = crd[l2g_sf[atmi[0]] * 3 + n2];
				f64 t2b = crd[l2g_sf[atmi[1]] * 3 + n2];
				
				t1a[n2] = t2a - t2b;
				t1b += t1a[n2] * t1a[n2];
			}
			
			f64 t1c = sqrt(t1b);
			
			bool first = (atmi[0] > atmi[1]);
			dist2[dist1[atmi[first]] + (atmi[!first] - atmi[first]) - 1] = t1c;
			
			// 1st layer...
			// 1st layer...
			// 1st layer...
			
			// none of the SASA terms should be skipped at 1st layer -> no test...
			
			if (t1c < (vdwr1[0][atmi[0]] + vdwr1[0][atmi[1]]))
			{
				nbt3_nl[0][atmi[0]].index[nbt3_nl[0][atmi[0]].index_count++] = atmi[1];
				if (nbt3_nl[0][atmi[0]].index_count >= size_nl[0]) { cout << "BUG: NL overflow 1b!!!" << endl; exit(EXIT_FAILURE); }
				
				nbt3_nl[0][atmi[1]].index[nbt3_nl[0][atmi[1]].index_count++] = atmi[0];
				if (nbt3_nl[0][atmi[1]].index_count >= size_nl[0]) { cout << "BUG: NL overflow 1b!!!" << endl; exit(EXIT_FAILURE); }
			}
			
			// 2nd layer...
			// 2nd layer...
			// 2nd layer...
			
			bool test2a = (nbt3_nl[1][atmi[0]].index != NULL);
			if (test2a && t1c < (vdwr1[1][atmi[0]] + vdwr1[0][atmi[1]]) && t1c > (vdwr1[1][atmi[0]] - vdwr1[0][atmi[1]]))
			{
				nbt3_nl[1][atmi[0]].index[nbt3_nl[1][atmi[0]].index_count++] = atmi[1];
				if (nbt3_nl[1][atmi[0]].index_count >= size_nl[1]) { cout << "BUG: NL overflow 2b!!!" << endl; exit(EXIT_FAILURE); }
			}
			
			bool test2b = (nbt3_nl[1][atmi[1]].index != NULL);
			if (test2b && t1c < (vdwr1[0][atmi[0]] + vdwr1[1][atmi[1]]) && t1c > (vdwr1[1][atmi[1]] - vdwr1[0][atmi[0]]))
			{
				nbt3_nl[1][atmi[1]].index[nbt3_nl[1][atmi[1]].index_count++] = atmi[0];
				if (nbt3_nl[1][atmi[1]].index_count >= size_nl[1]) { cout << "BUG: NL overflow 2b!!!" << endl; exit(EXIT_FAILURE); }
			}
			
			// 3rd layer...
			// 3rd layer...
			// 3rd layer...
			
			bool test3a = (nbt3_nl[2][atmi[0]].index != NULL);
			if (test3a && t1c < (vdwr1[2][atmi[0]] + vdwr1[0][atmi[1]]) && t1c > (vdwr1[2][atmi[0]] - vdwr1[0][atmi[1]]))
			{
				nbt3_nl[2][atmi[0]].index[nbt3_nl[2][atmi[0]].index_count++] = atmi[1];
				if (nbt3_nl[2][atmi[0]].index_count >= size_nl[2]) { cout << "BUG: NL overflow 3b!!!" << endl; exit(EXIT_FAILURE); }
			}
			
			bool test3b = (nbt3_nl[2][atmi[1]].index != NULL);
			if (test3b && t1c < (vdwr1[0][atmi[0]] + vdwr1[2][atmi[1]]) && t1c > (vdwr1[2][atmi[1]] - vdwr1[0][atmi[0]]))
			{
				nbt3_nl[2][atmi[1]].index[nbt3_nl[2][atmi[1]].index_count++] = atmi[0];
				if (nbt3_nl[2][atmi[1]].index_count >= size_nl[2]) { cout << "BUG: NL overflow 3b!!!" << endl; exit(EXIT_FAILURE); }
			}
		}
		
	}
	
	energy_nbt3a = 0.0;
	energy_nbt3b = 0.0;
	
	for (i32u layer = 0;layer < LAYERS;layer++)
	{
		sf_nbt3_nl * nlist = nbt3_nl[layer];
	//	if (!nlist) continue;	// skip if no neighbor list; should exist for all layers now!!!
		
		for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount() - num_solvent;n1++)
		{
			sf_nbt3_nd ndt[size_nl[LAYERS - 1]];
			for (i32s n2 = 0;n2 < nlist[n1].index_count;n2++)
			{
				ndt[n2].index = nlist[n1].index[n2];
				i32s atmi[2] = { n1, ndt[n2].index }; bool first = (atmi[0] > atmi[1]);
				ndt[n2].distance = dist2[dist1[atmi[first]] + (atmi[!first] - atmi[first]) - 1];
			}
			
			sort(ndt, ndt + nlist[n1].index_count);
			i32s n_count = 0; i32s nt[SIZE_NT];
			
			// neighbor-list reduction... THIS WON'T WORK IF SOME BT1/NBT1-TERMS ARE LEFT OUT!!!
			// neighbor-list reduction... THIS WON'T WORK IF SOME BT1/NBT1-TERMS ARE LEFT OUT!!!
			// neighbor-list reduction... THIS WON'T WORK IF SOME BT1/NBT1-TERMS ARE LEFT OUT!!!
			
			// test this against a slow-but-simple implementation?!?!?!
			
			for (i32s n2 = 0;n2 < nlist[n1].index_count;n2++)
			{
				i32s ind1 = ndt[n2].index;
				f64 dij = ndt[n2].distance;
				
				bool flag = true;
				
				for (i32s n3 = n2 + 1;n3 < nlist[n1].index_count;n3++)
				{
					i32s ind2 = ndt[n3].index;
					
					i32s atmi[2] = { ind1, ind2 }; bool first = (atmi[0] > atmi[1]);
					f64 djk = dist2[dist1[atmi[first]] + (atmi[!first] - atmi[first]) - 1];
					
					if (djk > dij) continue;
					
					f64 dij2 = dij * dij; f64 djk2 = djk * djk;
					f64 dik = ndt[n3].distance; f64 dik2 = dik * dik;
					
					// here dij and dik both represent distances which should never be
					// very close to zero (if LJ-terms work as they should) -> no checking
					
					f64 ca = (vdwr2[layer][n1] + dij2 - vdwr2[0][ind1]) / (2.0 * vdwr1[layer][n1] * dij);
					f64 cb = (vdwr2[layer][n1] + dik2 - vdwr2[0][ind2]) / (2.0 * vdwr1[layer][n1] * dik);
					f64 cg = (dij2 + dik2 - djk2) / (2.0 * dij * dik);
					
					f64 sa2 = 1.0 - ca * ca;
					f64 sg2 = 1.0 - cg * cg;
					
					f64 dc = sa2 * sg2;
					if (dc < 0.0) dc = 0.0;		// domain check...
					
					if (cb < ca * cg - sqrt(dc))
					{
						flag = false;
						break;
					}
				}
				
				if (flag)
				{
					nt[n_count++] = ind1;
					if (n_count >= SIZE_NT) { cout << "BUG: NT overflow!!!" << endl; exit(EXIT_FAILURE); }
				}
			}
			
			i32s coi_count = 0; sf_nbt3_coi coit[SIZE_COI];
			
			// next we will create the coi-table...
			// next we will create the coi-table...
			// next we will create the coi-table...
			
			for (i32s n2 = 0;n2 < n_count;n2++)
			{
				coit[coi_count].index = nt[n2]; coit[coi_count].flag = false;
				
				f64 t1a[3]; f64 t1b = 0.0;
				for (i32s n3 = 0;n3 < 3;n3++)
				{
					f64 t9a = crd[l2g_sf[n1] * 3 + n3];
					f64 t9b = crd[l2g_sf[coit[coi_count].index] * 3 + n3];
					
					t1a[n3] = t9b - t9a;
					t1b += t1a[n3] * t1a[n3];
				}
				
				f64 t1c = sqrt(t1b);
				coit[coi_count].dist = t1c;
				
				// also t1c is a distance which should never be close to zero -> no checking
				
				f64 t2a[3];
				for (i32s n3 = 0;n3 < 3;n3++)
				{
					t2a[n3] = t1a[n3] / t1c;
					coit[coi_count].dv[n3] = t2a[n3];
				}
				
				coit[coi_count].g = (t1b + vdwr2[layer][n1] - vdwr2[0][coit[coi_count].index]) / (2.0 * t1c);
				coit[coi_count].ct = coit[coi_count].g / vdwr1[layer][n1];
				
				if (p1 > 0 && use_implicit_solvent)
				{
					for (i32s n3 = 0;n3 < 3;n3++)
					{
						for (i32s n4 = 0;n4 < 3;n4++)
						{
							f64 t9a = t2a[n3] * t2a[n4]; f64 t9b;
							if (n3 != n4) t9b = -t9a; else t9b = 1.0 - t9a;
							coit[coi_count].ddv[n3][n4] = t9b / t1c;
						}
					}
					
					f64 t3a = (t1c - coit[coi_count].g) / t1c;
					coit[coi_count].dg[0] = t3a * coit[coi_count].dv[0];
					coit[coi_count].dg[1] = t3a * coit[coi_count].dv[1];
					coit[coi_count].dg[2] = t3a * coit[coi_count].dv[2];
					
					coit[coi_count].dct[0] = coit[coi_count].dg[0] / vdwr1[layer][n1];
					coit[coi_count].dct[1] = coit[coi_count].dg[1] / vdwr1[layer][n1];
					coit[coi_count].dct[2] = coit[coi_count].dg[2] / vdwr1[layer][n1];
				}
				
				coit[coi_count++].ipd_count = 0;
				if (coi_count >= SIZE_COI) { cout << "BUG: COI overflow!!!" << endl; exit(EXIT_FAILURE); }
			}
			
			i32s ips_total_count = 0;
			i32s ips_count = 0; sf_nbt3_ips ipst[SIZE_IPS];
			
			// next we will create the ips-table...
			// next we will create the ips-table...
			// next we will create the ips-table...
			
			for (i32s n2 = 0;n2 < coi_count - 1;n2++)
			{
				for (i32s n3 = n2 + 1;n3 < coi_count;n3++)
				{
					f64 t1a[3];
					t1a[0] = coit[n2].dv[0] * coit[n3].dv[0];
					t1a[1] = coit[n2].dv[1] * coit[n3].dv[1];
					t1a[2] = coit[n2].dv[2] * coit[n3].dv[2];
					
					f64 t1b = t1a[0] + t1a[1] + t1a[2];	// cos phi
					
					if (t1b < -1.0) t1b = -1.0;	// domain check...
					if (t1b > +1.0) t1b = +1.0;	// domain check...
					
					f64 t1c = 1.0 - t1b * t1b;		// sin^2 phi
					if (t1c < LOWLIMIT) t1c = LOWLIMIT;
					
					f64 t2a = (coit[n2].g - coit[n3].g * t1b) / t1c;	// tau_kj
					f64 t2b = (coit[n3].g - coit[n2].g * t1b) / t1c;	// tau_jk
					
					f64 t2c = vdwr2[layer][n1] - coit[n2].g * t2a - coit[n3].g * t2b;	// gamma^2
					if (t2c < LOWLIMIT) continue;		// these will not intercept...
					
					ips_total_count++;
					coit[n2].flag = true;
					coit[n3].flag = true;
					
					f64 t3a[3];	// eta
					t3a[0] = coit[n2].dv[0] * t2a + coit[n3].dv[0] * t2b;
					t3a[1] = coit[n2].dv[1] * t2a + coit[n3].dv[1] * t2b;
					t3a[2] = coit[n2].dv[2] * t2a + coit[n3].dv[2] * t2b;
					
					f64 t1d = sqrt(t1c);	// sin phi
					
					f64 t3b[3];	// omega
					t3b[0] = (coit[n2].dv[1] * coit[n3].dv[2] - coit[n2].dv[2] * coit[n3].dv[1]) / t1d;
					t3b[1] = (coit[n2].dv[2] * coit[n3].dv[0] - coit[n2].dv[0] * coit[n3].dv[2]) / t1d;
					t3b[2] = (coit[n2].dv[0] * coit[n3].dv[1] - coit[n2].dv[1] * coit[n3].dv[0]) / t1d;
					
					f64 t2d = sqrt(t2c);	// gamma
					
					for (i32s n4 = 0;n4 < 3;n4++)
					{
						f64 t9a = t3b[n4] * t2d;
						ipst[ips_count].ipv[0][n4] = t3a[n4] - t9a;
						ipst[ips_count].ipv[1][n4] = t3a[n4] + t9a;
					}
					
					// skip those intersection points that fall inside any other sphere...
					// SKIP ALSO IF EQUAL DISTANCE??? i.e. compare using "<" or "<=" ???
					
					bool skip_both = false;
					bool skip[2] = { false, false };
					for (i32s n4 = 0;n4 < n_count;n4++)
					{
						i32s n5 = nt[n4];
						if (n5 == coit[n2].index || n5 == coit[n3].index) continue;
						
	f64 t9a[3];
	t9a[0] = (crd[l2g_sf[n1] * 3 + 0] + ipst[ips_count].ipv[0][0]) - crd[l2g_sf[n5] * 3 + 0];
	t9a[1] = (crd[l2g_sf[n1] * 3 + 1] + ipst[ips_count].ipv[0][1]) - crd[l2g_sf[n5] * 3 + 1];
	t9a[2] = (crd[l2g_sf[n1] * 3 + 2] + ipst[ips_count].ipv[0][2]) - crd[l2g_sf[n5] * 3 + 2];
	f64 t9b = t9a[0] * t9a[0] + t9a[1] * t9a[1] + t9a[2] * t9a[2];
	if (t9b < vdwr2[0][n5]) skip[0] = true;
	
	f64 t9c[3];
	t9c[0] = (crd[l2g_sf[n1] * 3 + 0] + ipst[ips_count].ipv[1][0]) - crd[l2g_sf[n5] * 3 + 0];
	t9c[1] = (crd[l2g_sf[n1] * 3 + 1] + ipst[ips_count].ipv[1][1]) - crd[l2g_sf[n5] * 3 + 1];
	t9c[2] = (crd[l2g_sf[n1] * 3 + 2] + ipst[ips_count].ipv[1][2]) - crd[l2g_sf[n5] * 3 + 2];
	f64 t9d = t9c[0] * t9c[0] + t9c[1] * t9c[1] + t9c[2] * t9c[2];
	if (t9d < vdwr2[0][n5]) skip[1] = true;
						
						skip_both = (skip[0] && skip[1]);
						if (skip_both) break;
					}
					
					if (skip_both) continue;	// overwrite this one...
					
					ipst[ips_count].coi[0] = n2;
					ipst[ips_count].coi[1] = n3;
					
					if (!skip[0])
					{
						coit[n2].AddIPD(ipst[ips_count].ipv[0], ips_count);
						coit[n3].AddIPD(ipst[ips_count].ipv[0], ips_count | ORDER_FLAG);
					}
					
					if (!skip[1])
					{
						coit[n2].AddIPD(ipst[ips_count].ipv[1], ips_count | INDEX_FLAG | ORDER_FLAG);
						coit[n3].AddIPD(ipst[ips_count].ipv[1], ips_count | INDEX_FLAG);
					}
					
					if (p1 > 0 && use_implicit_solvent)
					{
						f64 t1f[3];	// d(cos phi) / dXk
						t1f[0] = (coit[n3].dv[0] - t1b * coit[n2].dv[0]) / coit[n2].dist;
						t1f[1] = (coit[n3].dv[1] - t1b * coit[n2].dv[1]) / coit[n2].dist;
						t1f[2] = (coit[n3].dv[2] - t1b * coit[n2].dv[2]) / coit[n2].dist;
						
						f64 t1g[3];	// d(cos phi) / dXj
						t1g[0] = (coit[n2].dv[0] - t1b * coit[n3].dv[0]) / coit[n3].dist;
						t1g[1] = (coit[n2].dv[1] - t1b * coit[n3].dv[1]) / coit[n3].dist;
						t1g[2] = (coit[n2].dv[2] - t1b * coit[n3].dv[2]) / coit[n3].dist;
						
						f64 t2e[3];	// d(tau_kj) / dXk
						t2e[0] = (t1f[0] * (2.0 * t2a * t1b - coit[n3].g) + coit[n2].dg[0]) / t1c;
						t2e[1] = (t1f[1] * (2.0 * t2a * t1b - coit[n3].g) + coit[n2].dg[1]) / t1c;
						t2e[2] = (t1f[2] * (2.0 * t2a * t1b - coit[n3].g) + coit[n2].dg[2]) / t1c;
						
						f64 t2f[3];	// d(tau_kj) / dXj
						t2f[0] = (t1g[0] * (2.0 * t2a * t1b - coit[n3].g) - t1b * coit[n3].dg[0]) / t1c;
						t2f[1] = (t1g[1] * (2.0 * t2a * t1b - coit[n3].g) - t1b * coit[n3].dg[1]) / t1c;
						t2f[2] = (t1g[2] * (2.0 * t2a * t1b - coit[n3].g) - t1b * coit[n3].dg[2]) / t1c;
						
						f64 t2g[3];	// d(tau_jk) / dXk
						t2g[0] = (t1f[0] * (2.0 * t2b * t1b - coit[n2].g) - t1b * coit[n2].dg[0]) / t1c;
						t2g[1] = (t1f[1] * (2.0 * t2b * t1b - coit[n2].g) - t1b * coit[n2].dg[1]) / t1c;
						t2g[2] = (t1f[2] * (2.0 * t2b * t1b - coit[n2].g) - t1b * coit[n2].dg[2]) / t1c;
						
						f64 t2h[3];	// d(tau_jk) / dXj
						t2h[0] = (t1g[0] * (2.0 * t2b * t1b - coit[n2].g) + coit[n3].dg[0]) / t1c;
						t2h[1] = (t1g[1] * (2.0 * t2b * t1b - coit[n2].g) + coit[n3].dg[1]) / t1c;
						t2h[2] = (t1g[2] * (2.0 * t2b * t1b - coit[n2].g) + coit[n3].dg[2]) / t1c;
						
						f64 t3c[3][3];	// d(eta) / dXk
						f64 t3d[3][3];	// d(eta) / dXj
						
						for (i32s n4 = 0;n4 < 3;n4++)
						{
							for (i32s n5 = 0;n5 < 3;n5++)
							{
					f64 t9a = coit[n2].dv[n5]; f64 t9b = coit[n3].dv[n5];
					t3c[n4][n5] = t9a * t2e[n4] + t9b * t2g[n4] + t2a * coit[n2].ddv[n4][n5];
					t3d[n4][n5] = t9a * t2f[n4] + t9b * t2h[n4] + t2b * coit[n3].ddv[n4][n5];
							}
						}
						
						f64 t3e[3][3];	// d(omega) / dXk
						f64 t3f[3][3];	// d(omega) / dXj
						
						for (i32s n4 = 0;n4 < 3;n4++)
						{
							for (i32s n5 = 0;n5 < 3;n5++)
							{
								t3e[n4][n5] = t1b * t3b[n5] * t1f[n4] / t1c;
								t3f[n4][n5] = t1b * t3b[n5] * t1g[n4] / t1c;
							}
							
	t3e[n4][0] += (coit[n2].ddv[n4][1] * coit[n3].dv[2] - coit[n2].ddv[n4][2] * coit[n3].dv[1]) / t1d;
	t3e[n4][1] += (coit[n2].ddv[n4][2] * coit[n3].dv[0] - coit[n2].ddv[n4][0] * coit[n3].dv[2]) / t1d;
	t3e[n4][2] += (coit[n2].ddv[n4][0] * coit[n3].dv[1] - coit[n2].ddv[n4][1] * coit[n3].dv[0]) / t1d;
	
	t3f[n4][0] += (coit[n2].dv[1] * coit[n3].ddv[n4][2] - coit[n2].dv[2] * coit[n3].ddv[n4][1]) / t1d;
	t3f[n4][1] += (coit[n2].dv[2] * coit[n3].ddv[n4][0] - coit[n2].dv[0] * coit[n3].ddv[n4][2]) / t1d;
	t3f[n4][2] += (coit[n2].dv[0] * coit[n3].ddv[n4][1] - coit[n2].dv[1] * coit[n3].ddv[n4][0]) / t1d;
						}
						
	f64 t2i[3];	// d(gamma) / dXk
	t2i[0] = -(coit[n2].g * t2e[0] + t2a * coit[n2].dg[0] + coit[n3].g * t2g[0]) / (2.0 * t2d);
	t2i[1] = -(coit[n2].g * t2e[1] + t2a * coit[n2].dg[1] + coit[n3].g * t2g[1]) / (2.0 * t2d);
	t2i[2] = -(coit[n2].g * t2e[2] + t2a * coit[n2].dg[2] + coit[n3].g * t2g[2]) / (2.0 * t2d);
						
	f64 t2j[3];	// d(gamma) / dXj
	t2j[0] = -(coit[n2].g * t2f[0] + coit[n3].g * t2h[0] + t2b * coit[n3].dg[0]) / (2.0 * t2d);
	t2j[1] = -(coit[n2].g * t2f[1] + coit[n3].g * t2h[1] + t2b * coit[n3].dg[1]) / (2.0 * t2d);
	t2j[2] = -(coit[n2].g * t2f[2] + coit[n3].g * t2h[2] + t2b * coit[n3].dg[2]) / (2.0 * t2d);
						
						// the final result is derivatives for points dipv[2][2][3][3].
						// indexes are as follows: [point][atom][variable][xyz].
						
						for (i32s n4 = 0;n4 < 3;n4++)
						{
							for (i32s n5 = 0;n5 < 3;n5++)
							{
								ipst[ips_count].dipv[0][0][n4][n5] = t3c[n4][n5];
								ipst[ips_count].dipv[0][1][n4][n5] = t3d[n4][n5];
								ipst[ips_count].dipv[1][0][n4][n5] = t3c[n4][n5];
								ipst[ips_count].dipv[1][1][n4][n5] = t3d[n4][n5];
							}
							
							for (i32s n5 = 0;n5 < 3;n5++)
							{
								f64 t9a = t3b[n5] * t2i[n4] + t2d * t3e[n4][n5];
								f64 t9b = t3b[n5] * t2j[n4] + t2d * t3f[n4][n5];
								
								ipst[ips_count].dipv[0][0][n4][n5] -= t9a;
								ipst[ips_count].dipv[0][1][n4][n5] -= t9b;
								ipst[ips_count].dipv[1][0][n4][n5] += t9a;
								ipst[ips_count].dipv[1][1][n4][n5] += t9b;
							}
						}          
					}
					
					ips_count++;
					if (ips_count >= SIZE_IPS) { cout << "BUG: IPS overflow!!!" << endl; exit(EXIT_FAILURE); }
				}
			}
			
			i32s arc_count = 0; sf_nbt3_arc arct[SIZE_ARC];
			
			// next we will create the arc-table...
			// next we will create the arc-table...
			// next we will create the arc-table...
			
			for (i32s n2 = 0;n2 < coi_count;n2++)
			{
				f64 t1z = vdwr2[layer][n1] - coit[n2].g * coit[n2].g;
				if (t1z < 0.0) t1z = 0.0;		// domain check...
				
				f64 t1a = sqrt(t1z);
				if (t1a < LOWLIMIT) t1a = LOWLIMIT;
				
				sort(coit[n2].ipdt, coit[n2].ipdt + coit[n2].ipd_count);
				
				for (i32s n3 = 0;n3 < coit[n2].ipd_count;n3++)
				{
					if (coit[n2].ipdt[n3].ipdata & ORDER_FLAG) continue;
					i32s n4 = n3 + 1; if (n4 == coit[n2].ipd_count) n4 = 0;
					if (!(coit[n2].ipdt[n4].ipdata & ORDER_FLAG)) continue;
					
					arct[arc_count].coi = n2; arct[arc_count].flag = false;
					
					arct[arc_count].ipdata[0] = (coit[n2].ipdt[n3].ipdata & ~ORDER_FLAG);
					arct[arc_count].ipdata[1] = (coit[n2].ipdt[n4].ipdata & ~ORDER_FLAG);
					
					i32s i1a = (arct[arc_count].ipdata[0] & FLAG_MASK);
					bool i1b = (arct[arc_count].ipdata[0] & INDEX_FLAG ? 1 : 0);
					
					i32s i2a = (arct[arc_count].ipdata[1] & FLAG_MASK);
					bool i2b = (arct[arc_count].ipdata[1] & INDEX_FLAG ? 1 : 0);
					
					arct[arc_count].index[0][0] = coit[ipst[i1a].coi[i1b]].index;
					arct[arc_count].index[0][1] = coit[ipst[i1a].coi[!i1b]].index;
					
					arct[arc_count].index[1][0] = coit[ipst[i2a].coi[!i2b]].index;
					arct[arc_count].index[1][1] = coit[ipst[i2a].coi[i2b]].index;
					
					// let's compute the tangent vectors...
					
					f64 * ref1 = ipst[i1a].ipv[i1b];
					arct[arc_count].tv[0][0] = (ref1[1] * coit[n2].dv[2] - ref1[2] * coit[n2].dv[1]) / t1a;
					arct[arc_count].tv[0][1] = (ref1[2] * coit[n2].dv[0] - ref1[0] * coit[n2].dv[2]) / t1a;
					arct[arc_count].tv[0][2] = (ref1[0] * coit[n2].dv[1] - ref1[1] * coit[n2].dv[0]) / t1a;
					
					f64 * ref2 = ipst[i2a].ipv[i2b];
					arct[arc_count].tv[1][0] = (ref2[1] * coit[n2].dv[2] - ref2[2] * coit[n2].dv[1]) / t1a;
					arct[arc_count].tv[1][1] = (ref2[2] * coit[n2].dv[0] - ref2[0] * coit[n2].dv[2]) / t1a;
					arct[arc_count].tv[1][2] = (ref2[0] * coit[n2].dv[1] - ref2[1] * coit[n2].dv[0]) / t1a;
					
					if (p1 > 0 && use_implicit_solvent)
					{
						for (i32s n4 = 0;n4 < 3;n4++)
						{
							f64 t9a = coit[n2].g * coit[n2].dg[n4] / t1a;
							for (i32s n5 = 0;n5 < 3;n5++)
							{
								arct[arc_count].dtv[0][0][n4][n5] = t9a * arct[arc_count].tv[0][n5];
								arct[arc_count].dtv[1][0][n4][n5] = t9a * arct[arc_count].tv[1][n5];
							}
							
	f64 * ref1a = ipst[i1a].dipv[i1b][i1b][n4];	// d(P1) / dXk
	arct[arc_count].dtv[0][0][n4][0] += ref1a[1] * coit[n2].dv[2] - ref1a[2] * coit[n2].dv[1];
	arct[arc_count].dtv[0][0][n4][1] += ref1a[2] * coit[n2].dv[0] - ref1a[0] * coit[n2].dv[2];
	arct[arc_count].dtv[0][0][n4][2] += ref1a[0] * coit[n2].dv[1] - ref1a[1] * coit[n2].dv[0];
	
	f64 * ref1b = ipst[i2a].dipv[i2b][!i2b][n4];	// d(P2) / dXk
	arct[arc_count].dtv[1][0][n4][0] += ref1b[1] * coit[n2].dv[2] - ref1b[2] * coit[n2].dv[1];
	arct[arc_count].dtv[1][0][n4][1] += ref1b[2] * coit[n2].dv[0] - ref1b[0] * coit[n2].dv[2];
	arct[arc_count].dtv[1][0][n4][2] += ref1b[0] * coit[n2].dv[1] - ref1b[1] * coit[n2].dv[0];
	
	f64 * ref2a = ipst[i1a].ipv[i1b];
	arct[arc_count].dtv[0][0][n4][0] += ref2a[1] * coit[n2].ddv[n4][2] - ref2a[2] * coit[n2].ddv[n4][1];
	arct[arc_count].dtv[0][0][n4][1] += ref2a[2] * coit[n2].ddv[n4][0] - ref2a[0] * coit[n2].ddv[n4][2];
	arct[arc_count].dtv[0][0][n4][2] += ref2a[0] * coit[n2].ddv[n4][1] - ref2a[1] * coit[n2].ddv[n4][0];
	
	f64 * ref2b = ipst[i2a].ipv[i2b];
	arct[arc_count].dtv[1][0][n4][0] += ref2b[1] * coit[n2].ddv[n4][2] - ref2b[2] * coit[n2].ddv[n4][1];
	arct[arc_count].dtv[1][0][n4][1] += ref2b[2] * coit[n2].ddv[n4][0] - ref2b[0] * coit[n2].ddv[n4][2];
	arct[arc_count].dtv[1][0][n4][2] += ref2b[0] * coit[n2].ddv[n4][1] - ref2b[1] * coit[n2].ddv[n4][0];
							
							for (i32s n5 = 0;n5 < 3;n5++)
							{
								arct[arc_count].dtv[0][0][n4][n5] /= t1a;
								arct[arc_count].dtv[1][0][n4][n5] /= t1a;
							}
							
	f64 * ref3a = ipst[i1a].dipv[i1b][!i1b][n4];	// d(P1) / dXj
	arct[arc_count].dtv[0][1][n4][0] = (ref3a[1] * coit[n2].dv[2] - ref3a[2] * coit[n2].dv[1]) / t1a;
	arct[arc_count].dtv[0][1][n4][1] = (ref3a[2] * coit[n2].dv[0] - ref3a[0] * coit[n2].dv[2]) / t1a;
	arct[arc_count].dtv[0][1][n4][2] = (ref3a[0] * coit[n2].dv[1] - ref3a[1] * coit[n2].dv[0]) / t1a;
	
	f64 * ref3b = ipst[i2a].dipv[i2b][i2b][n4];	// d(P2) / dXj
	arct[arc_count].dtv[1][1][n4][0] = (ref3b[1] * coit[n2].dv[2] - ref3b[2] * coit[n2].dv[1]) / t1a;
	arct[arc_count].dtv[1][1][n4][1] = (ref3b[2] * coit[n2].dv[0] - ref3b[0] * coit[n2].dv[2]) / t1a;
	arct[arc_count].dtv[1][1][n4][2] = (ref3b[0] * coit[n2].dv[1] - ref3b[1] * coit[n2].dv[0]) / t1a;
						}
					}
					
					arc_count++;
					if (arc_count >= SIZE_ARC) { cout << "BUG: ARC overflow!!!" << endl; exit(EXIT_FAILURE); }
				}
			}
			
			// all cases will pass through this point!!!
			// all cases will pass through this point!!!
			// all cases will pass through this point!!!
			
			f64 area;
			if (!arc_count)
			{
				if (ips_total_count)
				{
					solv_exp[layer][n1] = 0.0;	// fully buried...
					continue;			// fully buried...
				}
				else area = 4.0 * M_PI;
			}
			else
			{
				area = 0.0;
				i32s arc_counter = 0;
				
				do
				{
					i32s prev; i32s curr = 0;
					while (arct[curr].flag)
					{
						curr++;
						if (curr == arc_count)
						{
							cout << "area_panic: can't find the first arc!!!" << endl;
							goto area_panic;
						}
					}
					
					i32s first = curr;
					
					f64 sum1 = 0.0;
					f64 sum2 = 0.0;
					
					while (true)
					{
						i32s coi = arct[curr].coi;
						
	f64 t1a[3];
	t1a[0] = arct[curr].tv[1][1] * arct[curr].tv[0][2] - arct[curr].tv[1][2] * arct[curr].tv[0][1];
	t1a[1] = arct[curr].tv[1][2] * arct[curr].tv[0][0] - arct[curr].tv[1][0] * arct[curr].tv[0][2];
	t1a[2] = arct[curr].tv[1][0] * arct[curr].tv[0][1] - arct[curr].tv[1][1] * arct[curr].tv[0][0];
						
						f64 t1b[3];
						t1b[0] = coit[coi].dv[0] * t1a[0];
						t1b[1] = coit[coi].dv[1] * t1a[1];
						t1b[2] = coit[coi].dv[2] * t1a[2];
						
						f64 t1c = (t1b[0] + t1b[1] + t1b[2] < 0.0 ? -1.0 : +1.0);
						
						f64 t2a[3];
						t2a[0] = arct[curr].tv[0][0] * arct[curr].tv[1][0];
						t2a[1] = arct[curr].tv[0][1] * arct[curr].tv[1][1];
						t2a[2] = arct[curr].tv[0][2] * arct[curr].tv[1][2];
						
						f64 t2b = t2a[0] + t2a[1] + t2a[2];
						
						if (t2b < -1.0) t2b = -1.0;	// domain check...
						if (t2b > +1.0) t2b = +1.0;	// domain check...
						
						f64 t2c = (1.0 - t1c) * M_PI + t1c * acos(t2b);
						sum1 += t2c * coit[coi].ct;
						
						if (p1 > 0 && use_implicit_solvent)
						{
							f64 t2x = fabs(sin(t2c));
							if (t2x < LOWLIMIT) t2x = LOWLIMIT;
							
							f64 t2y = -sasaE[layer][n1] * coit[coi].ct * t1c / t2x;
							
							// 1st are same points and 2nd are different ones...
							// 1st are same points and 2nd are different ones...
							// 1st are same points and 2nd are different ones...
							
							for (i32s n2 = 0;n2 < 3;n2++)
							{
					f64 t3a[3];
					t3a[0] = arct[curr].dtv[0][0][n2][0] * arct[curr].tv[1][0];
					t3a[1] = arct[curr].dtv[0][0][n2][1] * arct[curr].tv[1][1];
					t3a[2] = arct[curr].dtv[0][0][n2][2] * arct[curr].tv[1][2];
					f64 t3b = t3a[0] + t3a[1] + t3a[2];
					
					f64 t3c[3];
					t3c[0] = arct[curr].tv[0][0] * arct[curr].dtv[1][0][n2][0];
					t3c[1] = arct[curr].tv[0][1] * arct[curr].dtv[1][0][n2][1];
					t3c[2] = arct[curr].tv[0][2] * arct[curr].dtv[1][0][n2][2];
					f64 t3d = t3c[0] + t3c[1] + t3c[2];
					
					f64 t4a[3];
					t4a[0] = arct[curr].dtv[0][1][n2][0] * arct[curr].tv[1][0];
					t4a[1] = arct[curr].dtv[0][1][n2][1] * arct[curr].tv[1][1];
					t4a[2] = arct[curr].dtv[0][1][n2][2] * arct[curr].tv[1][2];
					f64 t4b = t4a[0] + t4a[1] + t4a[2];
					
					f64 t4c[3];
					t4c[0] = arct[curr].tv[0][0] * arct[curr].dtv[1][1][n2][0];
					t4c[1] = arct[curr].tv[0][1] * arct[curr].dtv[1][1][n2][1];
					t4c[2] = arct[curr].tv[0][2] * arct[curr].dtv[1][1][n2][2];
					f64 t4d = t4c[0] + t4c[1] + t4c[2];
					
					f64 t3e = t2y * (t3b + t3d) + sasaE[layer][n1] * t2c * coit[coi].dct[n2];
					f64 t5a = t2y * t4b; f64 t5b = t2y * t4d;
					
					d1[l2g_sf[arct[curr].index[0][0]] * 3 + n2] += t3e;
					d1[l2g_sf[arct[curr].index[0][1]] * 3 + n2] += t5a;
					d1[l2g_sf[arct[curr].index[1][1]] * 3 + n2] += t5b;
					d1[l2g_sf[n1] * 3 + n2] -= t3e + t5a + t5b;
							}
						}
						
						prev = curr; curr = 0;
						i32u ipd = arct[prev].ipdata[1];
						while (true)
						{
							if (arct[curr].ipdata[0] != ipd) curr++;
							else break;
							
							if (curr == arc_count)
							{
								cout << "area_panic: incomplete set of arcs!!!" << endl;
								goto area_panic;
							}
						}
						
						arc_counter++;
						arct[curr].flag = true;
						
						f64 t2d[3];
						t2d[0] = arct[prev].tv[1][0] * arct[curr].tv[0][0];
						t2d[1] = arct[prev].tv[1][1] * arct[curr].tv[0][1];
						t2d[2] = arct[prev].tv[1][2] * arct[curr].tv[0][2];
						
						f64 t2e = t2d[0] + t2d[1] + t2d[2];
						
						if (t2e < -1.0) t2e = -1.0;	// domain check...
						if (t2e > +1.0) t2e = +1.0;	// domain check...
						
						f64 t2f = -acos(t2e); sum2 += t2f;
						
						if (p1 > 0 && use_implicit_solvent)
						{
							f64 t2x = fabs(sin(t2f));
							if (t2x < LOWLIMIT) t2x = LOWLIMIT;
							
							f64 t2y = sasaE[layer][n1] / t2x;
							
							// prev_k = curr_j and prev_j = curr_k !!!
							// prev_k = curr_j and prev_j = curr_k !!!
							// prev_k = curr_j and prev_j = curr_k !!!
							
							for (i32s n2 = 0;n2 < 3;n2++)
							{
					f64 t3a[3];
					t3a[0] = arct[prev].dtv[1][0][n2][0] * arct[curr].tv[0][0];
					t3a[1] = arct[prev].dtv[1][0][n2][1] * arct[curr].tv[0][1];
					t3a[2] = arct[prev].dtv[1][0][n2][2] * arct[curr].tv[0][2];
					f64 t3b = t3a[0] + t3a[1] + t3a[2];
					
					f64 t3c[3];
					t3c[0] = arct[prev].tv[1][0] * arct[curr].dtv[0][1][n2][0];
					t3c[1] = arct[prev].tv[1][1] * arct[curr].dtv[0][1][n2][1];
					t3c[2] = arct[prev].tv[1][2] * arct[curr].dtv[0][1][n2][2];
					f64 t3d = t3c[0] + t3c[1] + t3c[2];
					
					f64 t4a[3];
					t4a[0] = arct[prev].dtv[1][1][n2][0] * arct[curr].tv[0][0];
					t4a[1] = arct[prev].dtv[1][1][n2][1] * arct[curr].tv[0][1];
					t4a[2] = arct[prev].dtv[1][1][n2][2] * arct[curr].tv[0][2];
					f64 t4b = t4a[0] + t4a[1] + t4a[2];
					
					f64 t4c[3];
					t4c[0] = arct[prev].tv[1][0] * arct[curr].dtv[0][0][n2][0];
					t4c[1] = arct[prev].tv[1][1] * arct[curr].dtv[0][0][n2][1];
					t4c[2] = arct[prev].tv[1][2] * arct[curr].dtv[0][0][n2][2];
					f64 t4d = t4c[0] + t4c[1] + t4c[2];
					
					f64 t3e = t2y * (t3b + t3d);
					f64 t4e = t2y * (t4b + t4d);
					
					d1[l2g_sf[arct[prev].index[1][0]] * 3 + n2] += t3e;
					d1[l2g_sf[arct[prev].index[1][1]] * 3 + n2] += t4e;
					d1[l2g_sf[n1] * 3 + n2] -= t3e + t4e;
							}    
						}
						
						if (curr == first) break;
					}
					
					area += 2.0 * M_PI + sum1 + sum2;
				} while (arc_counter < arc_count);
				
				// when we have some problems somewhere above (for example, if we have
				// an incomplete set of arcs or no arcs at all; these things are possible
				// in rare special cases; for example we might have to reject some arcs
				// if they contained some singular intermediate values) we will truncate
				// the sum and jump right here.
				
				// in this case we will calculate incorrect value for the area, but the
				// good news is that the value and the gradient will still be consistent.
				
				// since these cases are very rare, this probably won't make big problems
				// in any applications...
				
				area_panic:	// we will jump here in all problematic cases...
				
				while (area > 4.0 * M_PI) area -= 4.0 * M_PI;
			}
			
			// finally here we will handle the single separate patches...
			// finally here we will handle the single separate patches...
			// finally here we will handle the single separate patches...
			
			for (i32s n2 = 0;n2 < coi_count;n2++)
			{
				if (coit[n2].flag) continue;
				
				f64 t1a = 2.0 * M_PI / vdwr1[layer][n1];
				area -= t1a * (vdwr1[layer][n1] - coit[n2].g);
				
				if (p1 > 0 && use_implicit_solvent)
				{
					for (i32s n3 = 0;n3 < 3;n3++)
					{
						f64 t1b = sasaE[layer][n1] * t1a * coit[n2].dg[n3];
						
						d1[l2g_sf[coit[n2].index] * 3 + n3] += t1b;
						d1[l2g_sf[n1] * 3 + n3] -= t1b;
					}
				}
			}
			
			f64 max_area;	// this is 4pi precisely, but the values below are practical max values.
			switch (layer)
			{
				case 0:	max_area = 8.7; break;
				case 1:	max_area = 10.0; break;
				case 2: max_area = 10.8; break;
				default:
				cout << "unknown layer!" << endl;
				exit(EXIT_FAILURE);
			}
			
			solv_exp[layer][n1] = area / max_area;
			if (solv_exp[layer][n1] < 0.0) solv_exp[layer][n1] = 0.0;	// domain check!!!
			if (solv_exp[layer][n1] > 1.0) solv_exp[layer][n1] = 1.0;	// domain check!!!
			
			if (!use_implicit_solvent) continue;
			
			f64 value = sasaE[layer][n1] * area;
			if (sasaE[layer][n1] > 0.0) energy_nbt3a += value;	// positive values...
			else energy_nbt3b += value;				// negative values...
		}
//cout << "layer = " << layer << " energy_nbt3 = " << energy_nbt3 << endl;
	}
}

void eng1_sf::Compute(i32u p1)
{
	if (p1 > 0)
	{
		for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount();n1++)
		{
			d1[l2g_sf[n1] * 3 + 0] = 0.0;
			d1[l2g_sf[n1] * 3 + 1] = 0.0;
			d1[l2g_sf[n1] * 3 + 2] = 0.0;
		}
	}
	
	// this is for the surface area term...
	// this is for the surface area term...
	// this is for the surface area term...
	
	for (i32u n1 = 0;n1 < LAYERS;n1++)
	{
		for (i32s n2 = 0;n2 < GetSetup()->GetSFAtomCount() - num_solvent;n2++)
		{
			nbt3_nl[n1][n2].index_count = 0;
		}
	}
	
	ComputeBT1(p1);		// we need this also for the surface area term...
	ComputeBT2(p1);
	ComputeBT3(p1);
	ComputeBT4(p1);
//energy_bt1 = 0.0;
//energy_bt2 = 0.0;
//energy_bt3a = 0.0; energy_bt3b = 0.0;
//energy_bt4a = 0.0; energy_bt4b = 0.0;
	
	ComputeNBT3(p1);	// we need this also for determining epsilon in electrostatics...
	ComputeNBT2(p1);
	ComputeNBT1(p1);
//energy_nbt3a = 0.0; energy_nbt3b = 0.0;
//energy_nbt2a = 0.0; energy_nbt2b = 0.0; energy_nbt2c = 0.0;
//energy_nbt1a = 0.0; energy_nbt1b = 0.0; energy_nbt1c = 0.0;
	
	energy = energy_bt1 + energy_bt2;
	energy += energy_bt3a + energy_bt3b;
	energy += energy_bt4a + energy_bt4b;
	energy += energy_nbt1a + energy_nbt1b + energy_nbt1c;
	energy += energy_nbt2a + energy_nbt2b + energy_nbt2c;
	energy += energy_nbt3a + energy_nbt3b;
	
/////////////////////////////////////////////
/////////////////////////////////////////////
// this will print also the components...
// this will print also the components...
// this will print also the components...
/*char buffer[1024];
ostrstream str(buffer, sizeof(buffer));
str.setf(ios::fixed); str.precision(8);

str << "B: ";
str << energy_bt1 << " ";
str << energy_bt2 << " ";
str << energy_bt3a << " " << energy_bt3b << " ";
str << energy_bt4a << " " << energy_bt4b << " ";
str << "NB: ";
str << energy_nbt1a << " " << energy_nbt1b << " " << energy_nbt1c << " ";
str << energy_nbt2a << " " << energy_nbt2b << " " << energy_nbt2c << " ";
str << energy_nbt3a << " " << energy_nbt3b << " ";
str << endl;

str << "Energy = " << energy << " kJ/mol + " << constraints << " kJ/mol = ";
str << (energy + constraints) << " kJ/mol" << ends;

cout << buffer << endl;*/
/////////////////////////////////////////////
/////////////////////////////////////////////
	
	// for consistency, it seems we have to include constraints in energy...
	// for consistency, it seems we have to include constraints in energy...
	// for consistency, it seems we have to include constraints in energy...
	
	energy += constraints;
}

// f = sum[(r/a)^-3] = sum[a^3 * r^-3]		// now seems to be r^-12
// df/dr = -3 * sum[a^3 * r^-4]

fGL eng1_sf::GetVDWSurf(fGL * pp, fGL * dd)
{
	fGL vdwsv = 0.0;
	if (dd != NULL) dd[0] = dd[1] = dd[2] = 0.0;
	
	atom ** atmtab = GetSetup()->GetSFAtoms();
	for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount();n1++)
	{
		fGL tmp1[3]; fGL r2 = 0.0;
		const f64 * cdata = & crd[l2g_sf[n1] * 3];
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			tmp1[n2] = pp[n2] - cdata[n2];
			r2 += tmp1[n2] * tmp1[n2];
		}
		
		if (r2 == 0.0) return +1.0e+35;		// numeric_limits<fGL>::max()?!?!?!
		fGL r1 = sqrt(r2);
		
		fGL tmp2 = r1 / (atmtab[n1]->vdwr + 0.15);	// solvent radius??? 0.15
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

fGL eng1_sf::GetESP(fGL * pp, fGL * dd)
{
	fGL espv = 0.0;
	if (dd != NULL) dd[0] = dd[1] = dd[2] = 0.0;
	
	atom ** atmtab = GetSetup()->GetSFAtoms();
	for (i32s n1 = 0;n1 < GetSetup()->GetSFAtomCount() - num_solvent;n1++)
	{
		fGL tmp1[3]; fGL r2 = 0.0;
		const f64 * cdata = & crd[l2g_sf[n1] * 3];
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			tmp1[n2] = pp[n2] - cdata[n2];
			r2 += tmp1[n2] * tmp1[n2];
		}
		
		if (r2 == 0.0) return +1.0e+35;		// numeric_limits<fGL>::max()?!?!?!
		fGL r1 = sqrt(r2);
		
	// e = 2 + 76 * (( r / A ) ^ n) / (1 + ( r / A ) ^ n) + Z/r^9 = 2 + 76 * f / g
	// de/dr = 76 * (( g * Df - f * Dg ) / g ^ 2 )
	
	f64 eps_n = myprm->epsilon1 + r2 * myprm->epsilon2;	// assume constant!!!
	f64 eps_A = 1.25;					// assume constant!!!
	
	f64 t7a = r1 / eps_A;
	f64 t7b = pow(t7a, eps_n);
	f64 t7c = 1.0 + t7b;
	f64 t7d = 2.0 + 76.0 * (t7b / t7c);
	
	f64 t7x = eps_n * pow(t7a, eps_n - 1.0) / eps_A;
	f64 t7y = 76.0 * ((t7c * t7x - t7b * t7x) / (t7c * t7c));
		
		// do we have a correct constant here??? I think so, if we define
		// electrostatic potential as potential energy of a unit positive charge.
		
	//	fGL tmp2 = 4.1868 * 33.20716 * atmtab[n1]->charge / r1;
		fGL tmp2 = 4.1868 * 33.20716 * atmtab[n1]->charge / (t7d * r1);					// screened!
		espv += tmp2;
		
		if (dd != NULL)		// sign ??? constant ???
		{
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				fGL tmp3 = tmp1[n2] / r1;
			//	fGL tmp4 = tmp3 * tmp2 / r1;
				fGL tmp4 = tmp3 * (-tmp2 * (1.0 / (t7d * r2) + t7y / (t7d * t7d * r1)));	// screened!
				dd[n2] += tmp4;
			}
		}
	}
	
	return espv;
}

/*################################################################################################*/

// eof
