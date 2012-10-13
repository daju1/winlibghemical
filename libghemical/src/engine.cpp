// ENGINE.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "engine.h"

#include "model.h"


/*################################################################################################*/

setup::setup(model * p1)
{
	mdl = p1;
	
	// initialize the tables...
	
	has_setup_tables = false;
	
	atmtab = NULL; natm = NOT_DEFINED;
	
	qm_atmtab = NULL; qm_natm = NOT_DEFINED;
	qm_bndtab = NULL; qm_nbnd = NOT_DEFINED;
	
	mm_atmtab = NULL; mm_natm = NOT_DEFINED;
	mm_bndtab = NULL; mm_nbnd = NOT_DEFINED;
	
	boundary_bndtab = NULL; boundary_nbnd = NOT_DEFINED;
	
	sf_atmtab = NULL; sf_natm = NOT_DEFINED;
	
	current_eng = NULL;
	current_eng_index = 0;
}

setup::~setup(void)
{
	DiscardCurrentEngine();
	DiscardSetupInfo();
}

void setup::DiscardSetupInfo(void)
{
	if (atmtab != NULL) { delete[] atmtab; atmtab = NULL;}
	
	if (qm_atmtab != NULL) { delete[] qm_atmtab; qm_atmtab = NULL; }
	if (qm_bndtab != NULL) { delete[] qm_bndtab; qm_bndtab = NULL; }
	
	if (mm_atmtab != NULL) { delete[] mm_atmtab; mm_atmtab = NULL; }
	if (mm_bndtab != NULL) { delete[] mm_bndtab; mm_bndtab = NULL; }
	
	if (boundary_bndtab != NULL) { delete[] boundary_bndtab; boundary_bndtab = NULL; }
		
	if (sf_atmtab != NULL) { delete[] sf_atmtab; sf_atmtab = NULL; }
	
	has_setup_tables = false;
}

void setup::UpdateSetupInfo(void)
{
	// discard the previous tables, if needed...
	
	DiscardSetupInfo();
	
	// clear all atom flags, and then bring them up-to-date.
	
	for (iter_al it1 = GetModel()->GetAtomsBegin();it1 != GetModel()->GetAtomsEnd();it1++)
	{
		(* it1).flags &= (~ ATOMFLAG_IS_QM_ATOM);
		(* it1).flags &= (~ ATOMFLAG_IS_MM_ATOM);
		(* it1).flags &= (~ ATOMFLAG_IS_SF_ATOM);
		(* it1).flags &= (~ ATOMFLAG_IS_HIDDEN);
	}
	
	UpdateAtomFlags();
	
	// start initializing the tables...
	
	natm = 0; i32s nbnd = 0;
	
	qm_natm = 0; qm_nbnd = 0;
	mm_natm = 0; mm_nbnd = 0;
	boundary_nbnd = 0;
	
	sf_natm = 0;
	
	// count the atoms and bonds that are taken into computations.
	// count the atoms and bonds that are taken into computations.
	// count the atoms and bonds that are taken into computations.
	
	iter_al ita = mdl->GetAtomsBegin();
	while (ita != mdl->GetAtomsEnd())
	{
		bool test1 = ((* ita).flags & (ATOMFLAG_IS_QM_ATOM | ATOMFLAG_IS_MM_ATOM | ATOMFLAG_IS_SF_ATOM));
		bool test2 = ((* ita).flags & ATOMFLAG_IS_HIDDEN);
		bool skip = (!test1 || test2);
		
		if (!skip)
		{
			natm++;
			
			if ((* ita).flags & ATOMFLAG_IS_QM_ATOM)
			{
				qm_natm++;
			}
			
			if ((* ita).flags & ATOMFLAG_IS_MM_ATOM)
			{
				mm_natm++;
			}
			
			if ((* ita).flags & ATOMFLAG_IS_SF_ATOM)
			{
				sf_natm++;
			}
		}
		
		ita++;
	}
	
	iter_bl itb = mdl->GetBondsBegin();
	while (itb != mdl->GetBondsEnd())
	{
		bool testA1 = ((* itb).atmr[0]->flags & (ATOMFLAG_IS_QM_ATOM | ATOMFLAG_IS_MM_ATOM));
		bool testA2 = ((* itb).atmr[0]->flags & ATOMFLAG_IS_HIDDEN);
		bool skipA = (!testA1 || testA2);
		
		bool testB1 = ((* itb).atmr[1]->flags & (ATOMFLAG_IS_QM_ATOM | ATOMFLAG_IS_MM_ATOM));
		bool testB2 = ((* itb).atmr[1]->flags & ATOMFLAG_IS_HIDDEN);
		bool skipB = (!testB1 || testB2);
		
		if (!skipA && !skipB)
		{
			nbnd++;
			
			if (((* itb).atmr[0]->flags & ATOMFLAG_IS_QM_ATOM) && ((* itb).atmr[1]->flags & ATOMFLAG_IS_QM_ATOM))
			{
				qm_nbnd++;
			}
			
			if (((* itb).atmr[0]->flags & ATOMFLAG_IS_MM_ATOM) && ((* itb).atmr[1]->flags & ATOMFLAG_IS_MM_ATOM))
			{
				mm_nbnd++;
			}
			
			bool tqmmm1 = (((* itb).atmr[0]->flags & ATOMFLAG_IS_QM_ATOM) && ((* itb).atmr[1]->flags & ATOMFLAG_IS_MM_ATOM));
			bool tqmmm2 = (((* itb).atmr[0]->flags & ATOMFLAG_IS_MM_ATOM) && ((* itb).atmr[1]->flags & ATOMFLAG_IS_QM_ATOM));
			if (tqmmm1 || tqmmm2)
			{
				boundary_nbnd++;
			}
		}
		
		itb++;
	}
	
	// bonds (in contrast to atoms) should only be counted in a single category; check!!!
	
	if (qm_nbnd + mm_nbnd + boundary_nbnd != nbnd)
	{
		cout << "ERROR : bond counting mismatch : " << qm_nbnd << " + " << mm_nbnd << " + " << boundary_nbnd << " != " << nbnd << endl;
		exit(EXIT_FAILURE);
	}
	
	// allocate the tables.
	
	atmtab = new ref_atom[natm];
	
	qm_atmtab = new ref_atom[qm_natm];
	qm_bndtab = new ref_bond[qm_nbnd];
	
	mm_atmtab = new ref_atom[mm_natm];
	mm_bndtab = new ref_bond[mm_nbnd];

	boundary_bndtab = new ref_bond[boundary_nbnd];
	
	sf_atmtab = new ref_atom[sf_natm];
	
	// fill the tables; BE SURE TO USE THE SAME RULES AS IN THE COUNTING STAGE!!!
	// fill the tables; BE SURE TO USE THE SAME RULES AS IN THE COUNTING STAGE!!!
	// fill the tables; BE SURE TO USE THE SAME RULES AS IN THE COUNTING STAGE!!!
	
	i32u ac = 0;
	i32u qm_ac = 0;
	i32u mm_ac = 0;
	i32u sf_ac = 0;
	
	ita = mdl->GetAtomsBegin();
	while (ita != mdl->GetAtomsEnd())
	{
		bool test1 = ((* ita).flags & (ATOMFLAG_IS_QM_ATOM | ATOMFLAG_IS_MM_ATOM | ATOMFLAG_IS_SF_ATOM));
		bool test2 = ((* ita).flags & ATOMFLAG_IS_HIDDEN);
		bool skip = (!test1 || test2);
		
		if (!skip)
		{
			(* ita).varind = ac;		// SET THE VARIABLE INDEX!!!
			atmtab[ac++] = & (* ita);
			
			if ((* ita).flags & ATOMFLAG_IS_QM_ATOM)
			{
				qm_atmtab[qm_ac++] = & (* ita);
			}
			
			if ((* ita).flags & ATOMFLAG_IS_MM_ATOM)
			{
				mm_atmtab[mm_ac++] = & (* ita);
			}
			
			if ((* ita).flags & ATOMFLAG_IS_SF_ATOM)
			{
				sf_atmtab[sf_ac++] = & (* ita);
			}
		}
		else (* ita).varind = NOT_DEFINED;
		
		ita++;
	}
	
	i32u qm_bc = 0;
	i32u mm_bc = 0;
	i32u boundary_bc = 0;
	
	itb = mdl->GetBondsBegin();
	while (itb != mdl->GetBondsEnd())
	{
		bool testA1 = ((* itb).atmr[0]->flags & (ATOMFLAG_IS_QM_ATOM | ATOMFLAG_IS_MM_ATOM));
		bool testA2 = ((* itb).atmr[0]->flags & ATOMFLAG_IS_HIDDEN);
		bool skipA = (!testA1 || testA2);
		
		bool testB1 = ((* itb).atmr[1]->flags & (ATOMFLAG_IS_QM_ATOM | ATOMFLAG_IS_MM_ATOM));
		bool testB2 = ((* itb).atmr[1]->flags & ATOMFLAG_IS_HIDDEN);
		bool skipB = (!testB1 || testB2);
		
		if (!skipA && !skipB)
		{
			if (((* itb).atmr[0]->flags & ATOMFLAG_IS_QM_ATOM) && ((* itb).atmr[1]->flags & ATOMFLAG_IS_QM_ATOM))
			{
				qm_bndtab[qm_bc++] = & (* itb);
			}
			
			if (((* itb).atmr[0]->flags & ATOMFLAG_IS_MM_ATOM) && ((* itb).atmr[1]->flags & ATOMFLAG_IS_MM_ATOM))
			{
				mm_bndtab[mm_bc++] = & (* itb);
			}
			
			bool tqmmm1 = (((* itb).atmr[0]->flags & ATOMFLAG_IS_QM_ATOM) && ((* itb).atmr[1]->flags & ATOMFLAG_IS_MM_ATOM));
			bool tqmmm2 = (((* itb).atmr[0]->flags & ATOMFLAG_IS_MM_ATOM) && ((* itb).atmr[1]->flags & ATOMFLAG_IS_QM_ATOM));
			if (tqmmm1 || tqmmm2)
			{
				boundary_bndtab[boundary_bc++] = & (* itb);
			}
		}
		
		itb++;
	}
	
	// finally check that the counts match!!!
	// finally check that the counts match!!!
	// finally check that the counts match!!!
	
	if ((i32s) ac != natm) { cout << "ERROR : atom mismatch! " << ac << " != " << natm << endl; exit(EXIT_FAILURE); }
	
	if ((i32s) qm_ac != qm_natm) { cout << "ERROR : qm-atom mismatch! " << qm_ac << " != " << qm_natm << endl; exit(EXIT_FAILURE); }
	if ((i32s) qm_bc != qm_nbnd) { cout << "ERROR : qm-bond mismatch! " << qm_bc << " != " << qm_nbnd << endl; exit(EXIT_FAILURE); }
	
	if ((i32s) mm_ac != mm_natm) { cout << "ERROR : mm-atom mismatch! " << mm_ac << " != " << mm_natm << endl; exit(EXIT_FAILURE); }
	if ((i32s) mm_bc != mm_nbnd) { cout << "ERROR : mm-bond mismatch! " << mm_bc << " != " << mm_nbnd << endl; exit(EXIT_FAILURE); }
	
	if ((i32s) boundary_bc != boundary_nbnd) { cout << "ERROR : boundary-bond mismatch! " << boundary_bc << " != " << boundary_nbnd << endl; exit(EXIT_FAILURE); }
	
	if ((i32s) sf_ac != sf_natm) { cout << "ERROR : sf-atom mismatch! " << sf_ac << " != " << sf_natm << endl; exit(EXIT_FAILURE); }
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//cout << "atoms : " << qm_ac << " " << mm_ac << " " << sf_ac << "     bonds : " << qm_bc << " " << mm_bc << endl;	//CGI!
//////////////////////////////////////////////////
//////////////////////////////////////////////////
	
	has_setup_tables = true;
}

void setup::CreateCurrentEngine(void)
{
	DiscardCurrentEngine();
	current_eng = CreateEngineByIndex(current_eng_index);
}

void setup::DiscardCurrentEngine(void)
{
	if (current_eng != NULL)
	{
		delete current_eng;
		current_eng = NULL;
	}
}

engine * setup::CreateEngineByIDNumber(i32u id)
{
	i32u index = 0;
	while (index < GetEngineCount())
	{
		if (GetEngineIDNumber(index) == id)
		{
			break;
		}
		else 
		{
			index++;
		}
	}
	
	if (index < GetEngineCount())
	{
		return CreateEngineByIndex(index);
	}
	else
	{
		cout << "setup::CreateEngineByIDNumber() failed!" << endl;
		return NULL;
	}
}

/*################################################################################################*/

engine::engine(setup * p1, i32u p2)
{
//cout << "engine p1 = " << p1 << " p2 = " << p2 << endl;
	stp = p1;
	if (!stp->HasSetupTables()) { cout << "fatal error : no setup tables at engine::engine()." << endl; exit(EXIT_FAILURE); }
	
	// here it is very important that we take the atom count from setup::GetAtomCount(), not model::GetAtomCount()!!!
	// it is possible that these are not at all the same!!! some atoms might be hidden from the calculations.
	
	natm = stp->GetAtomCount();
	
	crd = new f64[natm * 3];
	
	if (p2 > 0) d1 = new f64[natm * 3];
	else d1 = NULL;
	
	if (p2 > 1) d2 = new f64[natm * natm * 9];
	else d2 = NULL;
}

engine::~engine(void)
{
	delete[] crd;
	
	if (d1 != NULL) delete[] d1;
	if (d2 != NULL) delete[] d2;
}

void engine::Check(i32u p1)
{
	const f64 delta = 0.000001;	// the finite difference step...
	
	Compute(1);
	f64 tmp1 = energy;
	
	f64 tmp2; f64 old;
	for (i32s n1 = 0;n1 < natm;n1++)
	{
		for (i32u n2 = 0;n2 < 3;n2++)
		{
			old = crd[n1 * 3 + n2];
			crd[n1 * 3 + n2] = old + delta;
			Compute(0); tmp2 = (energy - tmp1) / delta;
			crd[n1 * 3 + n2] = old;
			
			cout << n1 << ((char) ('x' + n2)) << " ";
			cout << "a = " << d1[n1 * 3 + n2] << " ";
			cout << "n = " << tmp2 << endl;
			
			//if ((n1 % 5) == 4) cin >> old;
		}
	}
	cin >> old;
}

f64 engine::GetGradientVectorLength(void)
{
	f64 sum = 0.0;
	
	for (i32s n1 = 0;n1 < natm;n1++)
	{
		for (i32u n2 = 0;n2 < 3;n2++)
		{
			f64 tmp1 = d1[n1 * 3 + n2];
			sum += tmp1 * tmp1;
		}
	}
	
	return sqrt(sum);
}

void engine::DoSHAKE(void)
{
}

/*################################################################################################*/

fGL value_VDWSurf(engine * eng, fGL * crd, fGL * grad)
{
	return eng->GetVDWSurf(crd, grad);
}

fGL value_ESP(engine * eng, fGL * crd, fGL * grad)
{
	return eng->GetESP(crd, grad);
}

fGL value_ElDens(engine * eng, fGL * crd, fGL * grad)
{
	return eng->GetElDens(crd, grad);
}

fGL value_Orbital(engine * eng, fGL * crd, fGL * grad)
{
	return eng->GetOrbital(crd, grad);
}

fGL value_OrbDens(engine * eng, fGL * crd, fGL * grad)
{
	return eng->GetOrbDens(crd, grad);
}

// this #include macro is located here, because the eng1_sf class derived from the
// engine class and therefore it only can be introduced after the engine class itself...

#include "eng1_sf.h"
void CalcMaxMinCoordinates(model * p1, engine * p2, i32u p3)
{
	atom ** glob_atmtab = p2->GetSetup()->GetAtoms();
	for (i32s n1 = 0;n1 < p2->GetSetup()->GetAtomCount();n1++)
	{
		const fGL * cdata = glob_atmtab[n1]->GetCRD(p3);
		if ( n1 == 0)
		{
			p1->maxCRD[0] = cdata[0];
			p1->minCRD[0] = cdata[0];
			p1->maxCRD[1] = cdata[1];
			p1->minCRD[1] = cdata[1];
			p1->maxCRD[2] = cdata[2];
			p1->minCRD[2] = cdata[2];
		}
		else
		{
			if (p1->maxCRD[0] < cdata[0]) p1->maxCRD[0] = cdata[0];
			if (p1->minCRD[0] > cdata[0]) p1->minCRD[0] = cdata[0];
			if (p1->maxCRD[1] < cdata[1]) p1->maxCRD[1] = cdata[1];
			if (p1->minCRD[1] > cdata[1]) p1->minCRD[1] = cdata[1];
			if (p1->maxCRD[2] < cdata[2]) p1->maxCRD[2] = cdata[2];
			if (p1->minCRD[2] > cdata[2]) p1->minCRD[2] = cdata[2];
		}		
	}	
	if (p1->m_bMaxMinCoordCalculed)
		return;

	//p1->periodic_box_HALFdim[0] = (p1->maxCRD[0] - p1->minCRD[0]+0.14) / 2.;
	//p1->periodic_box_HALFdim[1] = (p1->maxCRD[1] - p1->minCRD[1]+0.12) / 2.;

	p1->periodic_box_HALFdim[0] = (p1->maxCRD[0] - p1->minCRD[0]) / 2. + 0.08;
	p1->periodic_box_HALFdim[1] = (p1->maxCRD[1] - p1->minCRD[1]) / 2. + 0.07;
	p1->periodic_box_HALFdim[2] = (p1->maxCRD[2] - p1->minCRD[2]) / 2. + 0.08;

	p1->m_bMaxMinCoordCalculed = true;

printf("periodic_box_HALFdim[0] = %f\n", p1->periodic_box_HALFdim[0]);
printf("periodic_box_HALFdim[1] = %f\n", p1->periodic_box_HALFdim[1]);
printf("periodic_box_HALFdim[2] = %f\n", p1->periodic_box_HALFdim[2]);
}
void CopyCRD(model * p1, engine * p2, i32u p3)
{
	if (p3 >= p1->cs_vector.size())
	{
		cout << "BUG: cs overflow at CopyCRD() mdl->eng." << endl;
		exit(EXIT_FAILURE);
	}
	
	atom ** glob_atmtab = p2->GetSetup()->GetAtoms();
	for (i32s n1 = 0;n1 < p2->GetSetup()->GetAtomCount();n1++)
	{
		const fGL * cdata = glob_atmtab[n1]->GetCRD(p3);
		/*if ( n1 == 0)
		{
			p1->maxCRD[0] = cdata[0];
			p1->minCRD[0] = cdata[0];
			p1->maxCRD[1] = cdata[1];
			p1->minCRD[1] = cdata[1];
			p1->maxCRD[2] = cdata[2];
			p1->minCRD[2] = cdata[2];
		}
		else
		{
			if (p1->maxCRD[0] < cdata[0]) p1->maxCRD[0] = cdata[0];
			if (p1->minCRD[0] > cdata[0]) p1->minCRD[0] = cdata[0];
			if (p1->maxCRD[1] < cdata[1]) p1->maxCRD[1] = cdata[1];
			if (p1->minCRD[1] > cdata[1]) p1->minCRD[1] = cdata[1];
			if (p1->maxCRD[2] < cdata[2]) p1->maxCRD[2] = cdata[2];
			if (p1->minCRD[2] > cdata[2]) p1->minCRD[2] = cdata[2];
		}*/
		
		p2->crd[n1 * 3 + 0] = cdata[0];
		p2->crd[n1 * 3 + 1] = cdata[1];
		p2->crd[n1 * 3 + 2] = cdata[2];
	}
	
	// the rest is SF-related...
	// the rest is SF-related...
	// the rest is SF-related...
	
	eng1_sf * esf = dynamic_cast<eng1_sf *>(p2);
	setup1_sf * ssf = dynamic_cast<setup1_sf *>(p2->GetSetup());
	if (esf != NULL && ssf != NULL)
	{
		i32s bt3_counter = 0;
		
		for (i32u cc = 0;cc < ssf->chn_vector.size();cc++)
		{
			for (i32s rc = 1;rc < ((i32s) ssf->chn_vector[cc].res_vector.size()) - 2;rc++)
			{
				const fGL * prev = ssf->chn_vector[cc].res_vector[rc - 1].atmr[0]->GetCRD(p3);
				const fGL * curr = ssf->chn_vector[cc].res_vector[rc + 0].atmr[0]->GetCRD(p3);
				const fGL * next = ssf->chn_vector[cc].res_vector[rc + 1].atmr[0]->GetCRD(p3);
				
				atom * ref_to_C = ssf->chn_vector[cc].res_vector[rc + 0].peptide[2];
				atom * ref_to_O = ssf->chn_vector[cc].res_vector[rc + 0].peptide[3];
				
				v3d<fGL> v1(curr, prev); v3d<fGL> v2(curr, next);
				v3d<fGL> v3(ref_to_O->GetCRD(p3), ref_to_C->GetCRD(p3));
				fGL pbdd = v1.tor(v2, v3);
				
				if (bt3_counter >= (i32s) esf->bt3_vector.size())
				{
					cout << "oops! something went wrong when trying to update bt3::pbdd" << endl;
					exit(EXIT_FAILURE);
				}
				
				esf->bt3_vector[bt3_counter++].pbdd = pbdd;
			}
		}
	}
	
	// ready!!!
	// ready!!!
	// ready!!!
}

void CopyCRD(engine * p1, model * p2, i32u p3)
{
	if (p3 >= p2->cs_vector.size())
	{
		cout << "BUG: cs overflow at CopyCRD() eng->mdl." << endl;
		exit(EXIT_FAILURE);
	}
	
	atom ** glob_atmtab = p1->GetSetup()->GetAtoms();
	for (i32s n1 = 0;n1 < p1->GetSetup()->GetAtomCount();n1++)
	{
		fGL x = p1->crd[n1 * 3 + 0];
		fGL y = p1->crd[n1 * 3 + 1];
		fGL z = p1->crd[n1 * 3 + 2];
		
		glob_atmtab[n1]->SetCRD(p3, x, y, z);
	}
	
	// the rest is SF-related...
	// the rest is SF-related...
	// the rest is SF-related...
	
	eng1_sf * esf = dynamic_cast<eng1_sf *>(p1);
	setup1_sf * ssf = dynamic_cast<setup1_sf *>(p1->GetSetup());
	if (esf != NULL && ssf != NULL)
	{
		i32s bt3_counter = 0;
		
		for (i32u cc = 0;cc < ssf->chn_vector.size();cc++)
		{
			for (i32s rc = 1;rc < ((i32s) ssf->chn_vector[cc].res_vector.size()) - 2;rc++)
			{
				const fGL * prev = ssf->chn_vector[cc].res_vector[rc - 1].atmr[0]->GetCRD(p3);
				const fGL * curr = ssf->chn_vector[cc].res_vector[rc + 0].atmr[0]->GetCRD(p3);
				const fGL * next = ssf->chn_vector[cc].res_vector[rc + 1].atmr[0]->GetCRD(p3);

				v3d<fGL> v1(curr, prev); v3d<fGL> v2(curr, next);
				v3d<fGL> v3 = v1.vpr(v2); v3 = v3 / v3.len();		// this is vector c in the JCC 2001 paper.
				v3d<fGL> v4 = v2.vpr(v3); v4 = v4 / v4.len();		// this is vector n in the JCC 2001 paper.
				
				if (bt3_counter >= (i32s) esf->bt3_vector.size())
				{
					cout << "oops! something went wrong when trying to update the N/C/O atoms!" << endl;
					exit(EXIT_FAILURE);
				}
				
				fGL pbdd = esf->bt3_vector[bt3_counter++].pbdd;
				v3d<fGL> v5 = (v4 * cos(pbdd)) - (v3 * sin(pbdd));
				
				v2 = v2 / v2.len(); const fGL scale = 0.3785;
				v3d<fGL> vC = (v2 * (+0.384 * scale)) + (v5 * (-0.116 * scale));
				v3d<fGL> vO = (v2 * (+0.442 * scale)) + (v5 * (-0.449 * scale));
				v3d<fGL> vN = (v2 * (+0.638 * scale)) + (v5 * (+0.109 * scale));
				
				atom * ref_to_C = ssf->chn_vector[cc].res_vector[rc + 0].peptide[2];
				atom * ref_to_O = ssf->chn_vector[cc].res_vector[rc + 0].peptide[3];
				atom * ref_to_N = ssf->chn_vector[cc].res_vector[rc + 1].peptide[0];
				
				fGL x; fGL y; fGL z;
				
				x = curr[0] + vC[0]; y = curr[1] + vC[1]; z = curr[2] + vC[2]; ref_to_C->SetCRD(p3, x, y, z);
				x = curr[0] + vO[0]; y = curr[1] + vO[1]; z = curr[2] + vO[2]; ref_to_O->SetCRD(p3, x, y, z);
				x = curr[0] + vN[0]; y = curr[1] + vN[1]; z = curr[2] + vN[2]; ref_to_N->SetCRD(p3, x, y, z);
			}
		}
	}
	
	// ready!!!
	// ready!!!
	// ready!!!
}

/*################################################################################################*/

engine_mbp::engine_mbp(setup * p1, i32u p2) : engine(p1, p2)
{
	use_simple_bp = GetSetup()->GetModel()->use_boundary_potential;
	bp_radius_solute = GetSetup()->GetModel()->boundary_potential_radius1;
	bp_radius_solvent = GetSetup()->GetModel()->boundary_potential_radius2;
	
	bp_center[0] = 0.0;
	bp_center[1] = 0.0;
	bp_center[2] = 0.0;
	
	nd_eval = NULL;
	rdf_eval = NULL;
}

engine_mbp::~engine_mbp(void)
{
	if (nd_eval != NULL) delete nd_eval;
	if (rdf_eval != NULL) delete rdf_eval;
}

/*################################################################################################*/

engine_pbc::engine_pbc(setup * p1, i32u p2) : engine(p1, p2)
{
printf("engine_pbc::engine_pbc\np1 = %x p2 = %d", p1, p2);
	// assume that GetModel()->GatherGroups() is done...
	
	nmol_mm = 0;
	
// count the molecules present in the MM part.
// since the "mol" level is the highest criteria in sorting, atoms in a molecule should
// be adjacent -> a continuous range of pointers.

// here we calculate the molecule locations precisely, but a simple trigger atom could be used as well...

	i32s previous = -123;	// what is the safest setting here??? NOT_DEFINED might be used there???
	atom ** atmtab = GetSetup()->GetMMAtoms();
	for (i32s index = 0;index < GetSetup()->GetMMAtomCount();index++)
	{
		if (atmtab[index]->id[0] != previous)
		{
			nmol_mm++;
			previous = atmtab[index]->id[0];
		}
	}
	
	mrange = new i32s[nmol_mm + 1];
	
	mrange[0] = 0; 
	/*i32s*/ index = 1;
	for (i32s n1 = 0;n1 < nmol_mm;n1++)
	{
		while (index < GetSetup()->GetMMAtomCount() && atmtab[index]->id[0] == n1) index++;
		mrange[n1 + 1] = index;
	}
}

engine_pbc::~engine_pbc(void)
{
	delete[] mrange;
}

void engine_pbc::CheckLocations(void)
{
	atom ** atmtab = GetSetup()->GetMMAtoms();
	for (i32s n1 = 0;n1 < nmol_mm;n1++)
	{
		f64 sum[3] = { 0.0, 0.0, 0.0 };
		f64 ac = (f64) (mrange[n1 + 1] - mrange[n1]);
		for (i32s n2 = mrange[n1];n2 < mrange[n1 + 1];n2++)
		{
			i32u index = atmtab[n2]->varind;
			for (i32s n3 = 0;n3 < 3;n3++)
			{
				sum[n3] += crd[index * 3 + n3];
			}
		}

		for (/*i32s*/ n2 = 0;n2 < 3;n2++)
		{
			f64 test = sum[n2] / ac;
			
			if (test < -GetSetup()->GetModel()->periodic_box_HALFdim[n2])
			{
				for (i32s n3 = mrange[n1];n3 < mrange[n1 + 1];n3++)
				{
					i32u index = atmtab[n3]->varind;
					crd[index * 3 + n2] += 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n2];
				}
			}
			else if (test > +GetSetup()->GetModel()->periodic_box_HALFdim[n2])
			{
				for (i32s n3 = mrange[n1];n3 < mrange[n1 + 1];n3++)
				{
					i32u index = atmtab[n3]->varind;
					crd[index * 3 + n2] -= 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n2];
				}
			}
		}
	}
}

// TODO :
// GetVDWSurf() for engine_pbc???
// GetESP() for engine_pbc???

/*################################################################################################*/

engine_pbc2::engine_pbc2(setup * p1, i32u p2) : engine_pbc(p1, p2), engine(p1, p2)
{
	n_solvent_through_z = 0;
#if KLAPAN_DIFFUSE_WORKING
	n_solvent_through_klapan = 0;

	num_klap = 0;
#endif
	//Вычисляем число молекул растворителя
	n_solvent_molecules = 0;
	for (i32s n1 = 0;n1 < nmol_mm;n1++)// цикл по молекулам
	{
		// число атомов в молекуле
		f64 ac = (f64) (mrange[n1 + 1] - mrange[n1]);
		if (ac <= 3) // если это молекула растворителя - определяем по числу атомов
			n_solvent_molecules++;
	}
	printf("engine_pbc2::engine_pbc\np1 = %x p2 = %d", p1, p2);
	printf("n_solvent_molecules = %d", n_solvent_molecules);

}

engine_pbc2::~engine_pbc2(void)
{
}
#if GRAVI_OSCILLATOR_WORKING 	
void engine_pbc2::SetGraviAtomFlagOnSolvent()
{
	M_solvent = 0.0;
	atom ** atmtab = GetSetup()->GetMMAtoms();
	for (i32s n1 = 0;n1 < nmol_mm;n1++)// цикл по молекулам
	{
		// число атомов в молекуле
		f64 ac = (f64) (mrange[n1 + 1] - mrange[n1]);
		if (ac <= 3) // если это молекула растворителя - определяем по числу атомов
		{
			for (i32s n2 = mrange[n1];n2 < mrange[n1 + 1];n2++)
			{
				atmtab[n2]->flags |= ATOMFLAG_IS_GRAVI;
				M_solvent += atmtab[n2]->mass;
			}
		}
	}
	M_solvent /= n_solvent_molecules;
	M_solvent *= 1.6605402e-27 * 6.0221367e+23;
	// умножаем на массу единицы атомного веса углеродной шкалы в кг и на число Авогадро
}
#endif
#if KLAPAN_DIFFUSE_WORKING
void engine_pbc2::ReadClapanList(char * fn)
{
	ReadTargetListFile(fn, klapan_list);
	num_klap = klapan_list.size();
	vz_klap.resize(num_klap);
	if(num_klap > 0) vdz_klap.resize(num_klap - 1);
}
#endif

void engine_pbc2::CheckLocations(void)
{
	static DWORD nchecking = 0;
	atom ** atmtab = GetSetup()->GetMMAtoms();
#if KLAPAN_DIFFUSE_WORKING
	// вычисляем z-координату клапана
	if (num_klap > 0)
	{
		//заполняем вектор координат атомов клапана
		for(int i = 0; i < num_klap; i++)
			vz_klap[i] = crd[this->klapan_list[i] * 3 + 2];
		// сортировка z координаты атомов клапана
		std::sort(vz_klap.begin(), vz_klap.end());
		// размах объекта клапана по оси z
		double Dz = vz_klap[num_klap - 1] - vz_klap[0];

		if (Dz == 0.0)
		{
			z_klapan = vz_klap[0];
		}
		else
		{
			// вектор расстояний между атомами клапана
			for(i = 0; i < num_klap - 1; i++)
				vdz_klap[i] = vz_klap[i+1] - vz_klap[i];
			// дополнение к размеру ячейки от размаха клапана
			double box_Dz = 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[2] - Dz;
			// максимальное расстояние между атомами объекта
			double max_dz = - 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[2];
			for(i = 0; i < num_klap - 1; i++)
				if (max_dz < vdz_klap[i]) max_dz = vdz_klap[i];

			if (box_Dz > max_dz)
			{
				z_klapan = 0.0;
				for(int i = 0; i < num_klap; i++)
					z_klapan += vz_klap[i];
				z_klapan /= num_klap;
			}
			else
			{
				int i_razdel;
				for(i = 0; i < num_klap - 1; i++)
				{
					if (max_dz == vdz_klap[i]) 
					{
						i_razdel = i; 
						break;
					}
				}

				if (z_klapan > 0)
				{
					z_klapan = 0.0;
					for(int i = 0; i <= i_razdel; i++)
						z_klapan += vz_klap[i] + 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[2];
					for(int i = i_razdel+1; i < num_klap; i++)
						z_klapan += vz_klap[i];
				}
				else
				{
					z_klapan = 0.0;
					for(int i = 0; i <= i_razdel; i++)
						z_klapan += vz_klap[i];
					for(int i = i_razdel+1; i < num_klap; i++)
						z_klapan += vz_klap[i] - 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[2];
				}				
				z_klapan /= num_klap;
			}
		}
	}
	else
		z_klapan = 0.0;
	// показатель корректировалась ли координата клапана
	int d_klap_through_z = 0; // равен нулю если координата клапана не корректировлась
	if (z_klapan < -GetSetup()->GetModel()->periodic_box_HALFdim[2])
	{
		z_klapan  += 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[2];
		d_klap_through_z = -1;
	}
	else if (z_klapan > +GetSetup()->GetModel()->periodic_box_HALFdim[2])
	{
		z_klapan  -= 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[2];
		d_klap_through_z = +1;
	}

	n_solvent_above_klapan = 0;
	dn_solvent_through_z   = 0;

	z_solvent = 0.0;
#endif

	for (i32s n1 = 0;n1 < nmol_mm;n1++)// цикл по молекулам
	{
		f64 sum[3] = { 0.0, 0.0, 0.0 };
		// число атомов в молекуле
		f64 ac = (f64) (mrange[n1 + 1] - mrange[n1]);
		if (ac <= 3) // если это молекула растворителя - определяем по числу атомов
		{
			for (i32s n2 = mrange[n1];n2 < mrange[n1 + 1];n2++)
			{
				i32u index = atmtab[n2]->varind;
				for (i32s n3 = 0;n3 < 3;n3++)
				{
					sum[n3] += crd[index * 3 + n3];
				}
			}
			for ( n2 = 0;n2 < 3;n2++)
			{
				f64 test = sum[n2] / ac;
				
				if (test < -GetSetup()->GetModel()->periodic_box_HALFdim[n2])
				{
					// считаем проникновение молекул растворителя сквозь z
					if (n2 == 2)
					{
						dn_solvent_through_z--;
#if KLAPAN_DIFFUSE_WORKING
						z_solvent += 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n2];
#endif
					}
					for (i32s n3 = mrange[n1];n3 < mrange[n1 + 1];n3++)
					{
						i32u index = atmtab[n3]->varind;
						crd[index * 3 + n2] += 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n2];
					}
					
				}
				else if (test > +GetSetup()->GetModel()->periodic_box_HALFdim[n2])
				{
					// считаем проникновение молекул растворителя сквозь z
					if (n2 == 2)
					{
						dn_solvent_through_z++;
#if KLAPAN_DIFFUSE_WORKING
						z_solvent -= 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n2];
#endif
					}
					for (i32s n3 = mrange[n1];n3 < mrange[n1 + 1];n3++)
					{
						i32u index = atmtab[n3]->varind;
						crd[index * 3 + n2] -= 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n2];
					}
				}

				if (n2 == 2)
				{					
#if KLAPAN_DIFFUSE_WORKING

					if (
						(test > z_klapan && test <= +GetSetup()->GetModel()->periodic_box_HALFdim[n2])
						||
						(test < -GetSetup()->GetModel()->periodic_box_HALFdim[n2])
						) 
						n_solvent_above_klapan++;

					z_solvent += test;
#endif
				}
			}
		}
		else
		{
			for (i32s n2 = mrange[n1];n2 < mrange[n1 + 1];n2++)
			{
				i32u index = atmtab[n2]->varind;
				for (i32s n3 = 0;n3 < 3;n3++)
				{
					f64 test = crd[index * 3 + n3];
					
					if (test < -GetSetup()->GetModel()->periodic_box_HALFdim[n3])
					{
						crd[index * 3 + n3] += 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n3];
					}
					else if (test > +GetSetup()->GetModel()->periodic_box_HALFdim[n3])
					{
						crd[index * 3 + n3] -= 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n3];
					}
				}
			}	
		}
	}

#if KLAPAN_DIFFUSE_WORKING
	z_solvent /= n_solvent_molecules;

	if (nchecking)
	{ 
		dn_solvent_through_klapan = -d_klap_through_z * n_solvent_molecules 
			+ n_solvent_above_klapan - pre_solvent_above_klapan + dn_solvent_through_z;
	}
	else
		dn_solvent_through_klapan = 0;

	pre_solvent_above_klapan = n_solvent_above_klapan;


	n_solvent_through_klapan += dn_solvent_through_klapan;
#endif
	n_solvent_through_z    += dn_solvent_through_z;
	

#if KLAPAN_DIFFUSE_WORKING
	FILE * out;
	out = fopen("diffuse_klap.txt", "at");
	fprintf(out, "%d,%d,%f,%d\n", nchecking, n_solvent_through_z, z_klapan, n_solvent_through_klapan);
	fclose (out);
#endif

	nchecking++;

#if DIFFUSE_WORKING
	FILE * out;
	out = fopen("diffuse.txt", "at");
	fprintf(out, "%d\n",n_through_z);
	fclose (out);
#endif
}

// TODO :
// GetVDWSurf() for engine_pbc???
// GetESP() for engine_pbc???

/*################################################################################################*/

number_density_evaluator::number_density_evaluator(engine_mbp * p1, bool p2, i32s p3)
{
	eng = p1;
	linear = p2;
	classes = p3;
	
	if (!eng->use_simple_bp)
	{
		cout << "number_density_evaluator : eng->use_simple_bp was false!" << endl;
		exit(EXIT_FAILURE);
	}
	
	upper_limits = new f64[classes];
	class_volumes = new f64[classes];
	UpdateClassLimits();
	
	counts = new i32u[classes + 1];
	ResetCounters();
}

number_density_evaluator::~number_density_evaluator(void)
{
	delete[] upper_limits;
	delete[] class_volumes;
	
	delete[] counts;
}

void number_density_evaluator::UpdateClassLimits(void)
{
	if (linear)
	{
		f64 prev_radius = 0.0;
		for (i32s n1 = 0;n1 < classes;n1++)
		{
			f64 next_radius = eng->bp_radius_solvent * ((f64) (n1 + 1)) / (f64) classes;
			upper_limits[n1] = next_radius;
			
			f64 volume1 = 4.0 * M_PI * prev_radius * prev_radius * prev_radius / 3.0;
			f64 volume2 = 4.0 * M_PI * next_radius * next_radius * next_radius / 3.0;
			
			class_volumes[n1] = volume2 - volume1;
			
			prev_radius = next_radius;	// this is the last operation...
		}
	}
	else
	{
		f64 volume1 = 4.0 * M_PI * eng->bp_radius_solvent * eng->bp_radius_solvent * eng->bp_radius_solvent / 3.0;
		f64 volume2 = volume1 / (f64) classes;
		
		f64 prev_radius = 0.0;
		for (i32s n1 = 0;n1 < classes;n1++)
		{
			f64 tmp1 = volume2 + 4.0 * M_PI * prev_radius * prev_radius * prev_radius / 3.0;
			f64 tmp2 = tmp1 / (4.0 * M_PI / 3.0);
			
			f64 next_radius = pow(tmp2, 1.0 / 3.0);
			
			upper_limits[n1] = next_radius;
			class_volumes[n1] = volume2;
			
			prev_radius = next_radius;	// this is the last operation...
		}
	}
}

void number_density_evaluator::ResetCounters(void)
{
	cycles = 0;
	for (i32s n1 = 0;n1 < classes + 1;n1++)
	{
		counts[n1] = 0;
	}
}

void number_density_evaluator::PrintResults(ostream & str)
{
	str << "ND : ";
	for (i32s n1 = 0;n1 < classes;n1++)
	{
		f64 tmp1 = ((f64) counts[n1]) / ((f64) cycles);
		f64 tmp2 = tmp1 / class_volumes[n1];
		
		str << tmp2 << " ";
	}
	
	f64 tmp1 = ((f64) counts[classes]) / ((f64) cycles);
	str << "(outside bp_radius = " << tmp1 << ")." << endl;
	
	ResetCounters();
}

/*################################################################################################*/

radial_density_function_evaluator::radial_density_function_evaluator(engine_mbp * p1, i32s p2, f64 gb, f64 ge, f64 cb, f64 ce)
{
	eng = p1;
	classes = p2;
	
	graph_begin = gb;
	graph_end = ge;
	
	count_begin = gb;
	count_end = ge;
	
	if (count_begin < 0.0)
	{
		if (!eng->use_simple_bp)
		{
			cout << "radial_density_function_evaluator : eng->use_simple_bp was false!" << endl;
			exit(EXIT_FAILURE);
		}
		
		if (!eng->nd_eval)
		{
			cout << "radial_density_function_evaluator : eng->nd_eval was NULL!" << endl;
			exit(EXIT_FAILURE);
		}
		
		f64 graph_width = graph_end - graph_begin;
		f64 count_width = count_end - count_begin;
		if (count_width < graph_width)
		{
			cout << "radial_density_function_evaluator : too narrow counting window!" << endl;
			exit(EXIT_FAILURE);
		}
	}
	
	upper_limits = new f64[classes];
	class_volumes = new f64[classes];
	
	f64 prev_radius = graph_begin;
	for (i32s n1 = 0;n1 < classes;n1++)
	{
		f64 next_radius = graph_begin + (graph_end - graph_begin) * ((f64) (n1 + 1)) / (f64) classes;
		upper_limits[n1] = next_radius;
		
		f64 volume1 = 4.0 * M_PI * prev_radius * prev_radius * prev_radius / 3.0;
		f64 volume2 = 4.0 * M_PI * next_radius * next_radius * next_radius / 3.0;
		
		class_volumes[n1] = volume2 - volume1;
		
		prev_radius = next_radius;	// this is the last operation...
	}
	
	counts = new i32u[classes];
	ResetCounters();
}

radial_density_function_evaluator::~radial_density_function_evaluator(void)
{
	delete[] upper_limits;
	delete[] class_volumes;
	
	delete[] counts;
}

void radial_density_function_evaluator::ResetCounters(void)
{
	cycles = 0;
	for (i32s n1 = 0;n1 < classes;n1++)
	{
		counts[n1] = 0;
	}
}

void radial_density_function_evaluator::PrintResults(ostream & str)
{
	str << "RDF : ";
	for (i32s n1 = 0;n1 < classes;n1++)
	{
		f64 tmp1 = ((f64) counts[n1]) / ((f64) cycles);
		f64 tmp2 = tmp1 / class_volumes[n1];
		
		str << tmp2 << " ";
	}	str << endl;
	
	ResetCounters();
}

/*################################################################################################*/

// eof
