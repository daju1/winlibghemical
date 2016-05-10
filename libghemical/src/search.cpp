// SEARCH.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "search.h"

#include "geomopt.h"
#include "utility.h"

#include <strstream>
using namespace std;

/*################################################################################################*/

random_search::random_search(model * p1, i32s p2, i32s p3, i32s p4, i32s p5, i32s p6)
{
	mdl = p1;
	molnum = p2;
	in_crdset = p3;
	out_crdset = p4;
	
	cycles = p5;
	optsteps = p6;
	
	if (!mdl->IsGroupsClean()) mdl->UpdateGroups();		// intcrd requires this!!!
	if (!mdl->IsGroupsSorted()) mdl->SortGroups(true);	// intcrd requires this!!!
	
	ic = new intcrd((* mdl), molnum, in_crdset);
	
	eng = mdl->GetCurrentSetup()->GetCurrentEngine();
	go = NULL;
	
	counter1 = 0;
	counter2 = NOT_DEFINED;
	
	if (!ic->GetVariableCount())
	{
		mdl->ErrorMessage("ERROR: no rotatable bonds!!!");
		counter1 = cycles;	// skip the search...
	}
	
	CopyCRD(mdl, eng, in_crdset); CopyCRD(eng, mdl, out_crdset);
	eng->Compute(0); min_energy = eng->energy;
	
	last_step = NOT_DEFINED;
	last_E = NOT_DEFINED;
}

random_search::~random_search(void)
{
	if (go != NULL) delete go;
	delete ic;
	
	// do not delete eng since it was obtained from setup->GetCurrentEngine()!
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

i32s random_search::TakeStep(void)
{
	last_step = NOT_DEFINED;
	last_E = NOT_DEFINED;
	
	if (counter1 < cycles)
	{
		if (counter2 == NOT_DEFINED)	// start a new cycle...
		{
			counter1++; counter2 = 0;
			
			fGL rotprob = 1.0 / sqrt((fGL) ic->GetVariableCount());
			for (i32s n1 = 0;n1 < ic->GetVariableCount();n1++)
			{
				fGL random;
				
				random = (fGL) rand() / (fGL) RAND_MAX;
				if (random > rotprob) continue;
				
				random = (fGL) rand() / (fGL) RAND_MAX;
				ic->SetVariable(n1, 2.0 * M_PI * random);
			}
			
			ic->UpdateCartesian();
			mdl->CenterCRDSet(in_crdset, true);
			CopyCRD(mdl, eng, in_crdset);
			
			if (go != NULL) delete go;
			go = new geomopt(eng, 50, 0.005, 10.0);
		}
		
		for (i32s n1 = 0;n1 < UPDATEFRQ;n1++)	// optimize...
		{
			counter2++;
			go->TakeCGStep(conjugate_gradient::Newton2An);
		}
		
		CopyCRD(eng, mdl, in_crdset);
		
		i32s retval = counter2;
		if (counter2 >= optsteps)
		{
			eng->Compute(0);
			if (eng->energy < min_energy)
			{
				CopyCRD(eng, mdl, out_crdset);
				min_energy = eng->energy;
			}
			
// do not make any output here because that might not be thread-safe...
//char mbuff1[256]; strstream str1(mbuff1, sizeof(mbuff1));
//str1 << "step " << (counter1 + 1) << "/" << cycles << "   energy = " << eng->energy << " kJ/mol" << endl << ends;
//mdl->PrintToLog(mbuff1);
			last_step = (counter1 + 1);
			last_E = eng->energy;
			
			counter2 = NOT_DEFINED;
		}
		
		return retval;
	}
	else return -1;
}

/*################################################################################################*/

systematic_search::systematic_search(model * p1, i32s p2, i32s p3, i32s p4, i32s p5, i32s p6)
{
	mdl = p1;
	molnum = p2;
	in_crdset = p3;
	out_crdset = p4;
	
	divisions = p5;
	optsteps = p6;
	
	if (!mdl->IsGroupsClean()) mdl->UpdateGroups();		// intcrd requires this!!!
	if (!mdl->IsGroupsSorted()) mdl->SortGroups(true);	// intcrd requires this!!!
	
	ic = new intcrd((* mdl), molnum, in_crdset);
	
	eng = mdl->GetCurrentSetup()->GetCurrentEngine();
	go = NULL;
	
	nvar = ic->GetVariableCount();
	if (nvar != 0)
	{
		counter1 = new i32s[nvar];
		for (i32s n1 = 0;n1 < nvar;n1++)
		{
			counter1[n1] = 0;
		}
	}
	else
	{
		mdl->ErrorMessage("ERROR: no rotatable bonds!!!");
		counter1 = NULL;	// skip the search...
	}
	
	counter2 = NOT_DEFINED;
	
	CopyCRD(mdl, eng, in_crdset); CopyCRD(eng, mdl, out_crdset);
	eng->Compute(0); min_energy = eng->energy;
}

systematic_search::~systematic_search(void)
{
	if (counter1 != NULL) delete[] counter1;	// this should never happen...
	
	if (go != NULL) delete go;
	delete ic;
	
	// do not delete eng since it was obtained from setup->GetCurrentEngine()!
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

i32s systematic_search::TakeStep(void)
{
	if (counter1 != NULL)
	{
		if (counter2 == NOT_DEFINED)	// start a new cycle...
		{
			counter2 = 0;
			
			i32s n1; bool overflow = false;
			for (n1 = 0;n1 < nvar;n1++)
			{
				overflow = false;
				
				counter1[n1]++;
				if (counter1[n1] >= divisions)
				{
					counter1[n1] = 0;
					overflow = true;
				}
				
				fGL value = (fGL) counter1[n1] / (fGL) divisions;
				ic->SetVariable(n1, 2.0 * M_PI * value);
				
				if (!overflow) break;
			}
			
			if (overflow && n1 == nvar)	// if overflow happened in the last counter, the search is ready!!!
			{
				delete[] counter1;
				counter1 = NULL;
			}
			
			ic->UpdateCartesian();
			mdl->CenterCRDSet(in_crdset, true);
			CopyCRD(mdl, eng, in_crdset);
			
			if (go != NULL) delete go;
			go = new geomopt(eng, 50, 0.005, 10.0);
		}
		
		for (i32s n1 = 0;n1 < UPDATEFRQ && counter2 < optsteps;n1++)	// optimize...
		{
			counter2++;
			go->TakeCGStep(conjugate_gradient::Newton2An);
		}
		
		CopyCRD(eng, mdl, in_crdset);
		
		i32s retval = counter2;
		if (counter2 >= optsteps)
		{
			eng->Compute(0);
			if (eng->energy < min_energy)
			{
				CopyCRD(eng, mdl, out_crdset);
				min_energy = eng->energy;
			}
			
if (counter1 != NULL)
{
	char mbuff1[256]; strstream str1(mbuff1, sizeof(mbuff1));
	str1 << "step "; for (int v = 0;v < nvar;v++) { str1 << (char) ('A' + counter1[(nvar - 1) - v]); }
	str1 << "   energy = " << eng->energy << " kJ/mol" << endl << ends;
	mdl->PrintToLog(mbuff1);
}
			
			counter2 = NOT_DEFINED;
		}
		
		return retval;
	}
	else return -1;
}

/*################################################################################################*/

monte_carlo_search::monte_carlo_search(model * p1, i32s p2, i32s p3, i32s p4, i32s p5, i32s p6, i32s p7)
{
	mdl = p1;
	molnum = p2;
	in_crdset = p3;
	out_crdset = p4;
	
	n_init_steps = p5;
	n_simul_steps = p6;
	optsteps = p7;
	
// OPTIMIZE WITH NO NONBONDED TERMS TO GET AN IDEAL SYMMETRIC STARTING GEOMETRY?!?!?
// OPTIMIZE WITH NO NONBONDED TERMS TO GET AN IDEAL SYMMETRIC STARTING GEOMETRY?!?!?
// OPTIMIZE WITH NO NONBONDED TERMS TO GET AN IDEAL SYMMETRIC STARTING GEOMETRY?!?!?
	
	if (!mdl->IsGroupsClean()) mdl->UpdateGroups();		// intcrd requires this!!!
	if (!mdl->IsGroupsSorted()) mdl->SortGroups(true);	// intcrd requires this!!!
	
	ic = new intcrd((* mdl), molnum, in_crdset);
	
	eng = mdl->GetCurrentSetup()->GetCurrentEngine();
	go = NULL;
	
	counter1 = -n_init_steps;
	counter2 = NOT_DEFINED;
	
	if (!ic->GetVariableCount())
	{
		mdl->ErrorMessage("ERROR: no rotatable bonds!!!");
		counter1 = n_simul_steps;	// skip the search...
	}
	
	nvar = ic->GetVariableCount();
	curr_ic1 = new f64[nvar]; curr_ic2 = new f64[nvar];
	for (i32s n1 = 0;n1 < nvar;n1++)
	{
		curr_ic1[n1] = ic->GetVariable(n1);
	}
	
	CopyCRD(mdl, eng, in_crdset); CopyCRD(eng, mdl, out_crdset);
	eng->Compute(0); curr_energy = min_energy = eng->energy;
}

monte_carlo_search::~monte_carlo_search(void)
{
	delete[] curr_ic1;
	delete[] curr_ic2;
	
	if (go != NULL) delete go;
	delete ic;
	
	// do not delete eng since it was obtained from setup->GetCurrentEngine()!
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

i32s monte_carlo_search::TakeStep(void)
{
	if (counter1 < n_simul_steps)
	{
		if (counter2 == NOT_DEFINED)	// start a new cycle...
		{
			counter1++; counter2 = 0;
			
			const fGL rotprob = 1.0 / sqrt((fGL) nvar);
			for (i32s n1 = 0;n1 < nvar;n1++)
			{
				fGL value = curr_ic1[n1];
				curr_ic2[n1] = value;
				
				fGL random;
				
				random = (fGL) rand() / (fGL) RAND_MAX;
				if (random > rotprob) continue;
				
				random = (fGL) rand() / (fGL) RAND_MAX;
				curr_ic2[n1] = 2.0 * M_PI * random;
			}
			
			for (i32s n1 = 0;n1 < nvar;n1++)
			{
				ic->SetVariable(n1, curr_ic2[n1]);
			}
			
			ic->UpdateCartesian();
			mdl->CenterCRDSet(in_crdset, true);
			CopyCRD(mdl, eng, in_crdset);
			
			if (go != NULL) delete go;
			go = new geomopt(eng, 50, 0.005, 10.0);
		}
		
		for (i32s n1 = 0;n1 < UPDATEFRQ && counter2 < optsteps;n1++)	// optimize...
		{
			counter2++;
			go->TakeCGStep(conjugate_gradient::Newton2An);
		}
		
		CopyCRD(eng, mdl, in_crdset);
		
		i32s retval = counter2;
		if (counter2 >= optsteps)
		{
			bool accept = false;
			
			eng->Compute(0);
			if (eng->energy < curr_energy) accept = true;
			
			f64 tmp1 = eng->energy - curr_energy;
			f64 tmp2 = 1000.0 * tmp1 / (8.31451 * 300.0);
			
			if (!accept && ((fGL) rand() / (fGL) RAND_MAX < exp(-tmp2))) accept = true;
	cout << counter1 << "   " << eng->energy << "   " << curr_energy << "   TESTVALUE = " << exp(-tmp2) << endl;
			
			if (accept)
			{
				for (i32s n1 = 0;n1 < nvar;n1++)
				{
					curr_ic1[n1] = curr_ic2[n1];
				}
				
				curr_energy = eng->energy;
				
	char mbuff1[256]; strstream str1(mbuff1, sizeof(mbuff1));
	str1 << "step " << (counter1 + 1) << "/" << n_simul_steps << "   energy = " << eng->energy << " kJ/mol" << endl << ends;
	mdl->PrintToLog(mbuff1);
				
// set the weight???
// set the weight???
// set the weight???
			}
			else
			{
				counter1--;
				
// increase the weighting factor???
// increase the weighting factor???
// increase the weighting factor???
			}
			
			counter2 = NOT_DEFINED;
			
			if (eng->energy < min_energy)
			{
				CopyCRD(eng, mdl, out_crdset);
				min_energy = eng->energy;
			}
		}
		
		return retval;
	}
	else return -1;
}

/*################################################################################################*/
/*################################################################################################*/
/*################################################################################################*/

// Szabo A, Ostlund N : "Modern Quantum Chemistry : introduction to advanced electronic
// structure theory" McGraw-Hill, 1989

transition_state_search::transition_state_search(model * p1, f64 p2, f64 p3)
{
	mdl = p1; deltae = p2; fc[0] = fc[1] = p3;
	
	init_failed = true;
	target[0] = target[1] = NULL;
	
	setup * su = mdl->GetCurrentSetup();
	suqm = dynamic_cast<setup1_qm *>(su);
	
	bool bad_setup = false;
	if (suqm == NULL) bad_setup = true;
	else
	{
		if (suqm->GetCurrEngIndex() < 0) bad_setup = true;	// valid values (MOPAC) are currently from 0 to 3!!!
		if (suqm->GetCurrEngIndex() > 3) bad_setup = true;	// valid values (MOPAC) are currently from 0 to 3!!!
	}
	
	if (bad_setup)
	{
		mdl->ErrorMessage("Must use an all-QM/MOPAC setup!\nPlease see the User's Manual.");
		return;
	}
	
	if (mdl->GetAtomCount() % 2 != 0)
	{
		mdl->ErrorMessage("Atom count must be even!\nPlease see the User's Manual.");
		return;
	}
	
	i32u half = mdl->GetAtomCount() / 2;
	atom ** atab1 = new ref_atom[half];
	atom ** atab2 = new ref_atom[half];
	
	i32u counter1 = 0; i32u counter2 = 0;
	for (iter_al it1 = mdl->GetAtomsBegin();it1 != mdl->GetAtomsEnd();it1++)
	{
		if (counter1 < half)
		{
			atab1[counter1] = & (* it1);
			counter1++;
		}
		else
		{
			atab2[counter2] = & (* it1);
			counter2++;
		}
	}
	
	bool identical = true;
	for (i32u n1 = 0;n1 < half;n1++)
	{
		if (atab1[n1]->el.GetAtomicNumber() != atab2[n1]->el.GetAtomicNumber()) identical = false;
	}
	
	if (!identical)
	{
		delete[] atab1;
		delete[] atab2;
		
		mdl->ErrorMessage("No proper pair of reactants/products found!\nPlease see the User's Manual.");
		return;
	}
	
	init_failed = false;	// ok, it seems that we can go on...
	
	// copy the coordinates...
	
	i32s new_csets1 = 2 - mdl->GetCRDSetCount();
	if (new_csets1 > 0) mdl->PushCRDSets(new_csets1);
	
	for (i32u n1 = 0;n1 < half;n1++)
	{
		const fGL * crd = atab2[n1]->GetCRD(0);
		atab1[n1]->SetCRD(1, crd[0], crd[1], crd[2]);
	}
	
	// handle the bonds that are not identical in reactants/products; collect patoms.
	// TODO : add an option that all atoms are written into patoms??? but keep the current simple system!
	
	vector<bond *> bvect_r; vector<bond *> bvect_p;
	for (iter_bl it1 = mdl->GetBondsBegin();it1 != mdl->GetBondsEnd();it1++)
	{
		bool bonded_to_r = false;
		for (i32u n1 = 0;n1 < half;n1++)
		{
			if ((* it1).atmr[0] == atab1[n1]) bonded_to_r = true;
			if ((* it1).atmr[1] == atab1[n1]) bonded_to_r = true;
		}
	
		bool bonded_to_p = false;
		for (i32u n1 = 0;n1 < half;n1++)
		{
			if ((* it1).atmr[0] == atab2[n1]) bonded_to_p = true;
			if ((* it1).atmr[1] == atab2[n1]) bonded_to_p = true;
		}
		
		if (bonded_to_r && bonded_to_p) continue;	// no such cases should exist, ignored...
		else
		{
			i32s ind1[2] = { NOT_DEFINED, NOT_DEFINED };
			for (i32u n1 = 0;n1 < half;n1++)
			{
				if ((* it1).atmr[0] == atab1[n1]) ind1[0] = n1;
				if ((* it1).atmr[1] == atab1[n1]) ind1[1] = n1;
				
				if ((* it1).atmr[0] == atab2[n1]) ind1[0] = n1 + half;
				if ((* it1).atmr[1] == atab2[n1]) ind1[1] = n1 + half;
			}
			
			if (ind1[0] == NOT_DEFINED || ind1[1] == NOT_DEFINED) { cout << "tss ind1 search failed." << endl; exit(EXIT_FAILURE); }
			
			bool has_equivalent = false;
			for (iter_bl it2 = mdl->GetBondsBegin();it2 != mdl->GetBondsEnd();it2++)
			{
				if (it1 == it2) continue;
				
				i32s ind2[2] = { NOT_DEFINED, NOT_DEFINED };
				for (i32u n1 = 0;n1 < half;n1++)
				{
					if ((* it2).atmr[0] == atab1[n1]) ind2[0] = n1;
					if ((* it2).atmr[1] == atab1[n1]) ind2[1] = n1;
					
					if ((* it2).atmr[0] == atab2[n1]) ind2[0] = n1 + half;
					if ((* it2).atmr[1] == atab2[n1]) ind2[1] = n1 + half;
				}
				
				if (ind2[0] == NOT_DEFINED || ind2[1] == NOT_DEFINED) { cout << "tss ind2 search failed." << endl; exit(EXIT_FAILURE); }
				
				if (ind1[0] == ind2[0] + (i32s) half && ind1[1] == ind2[1] + (i32s) half) has_equivalent = true;
				if (ind1[0] + (i32s) half == ind2[0] && ind1[1] + (i32s) half == ind2[1]) has_equivalent = true;
				if (ind1[0] == ind2[1] + (i32s) half && ind1[1] == ind2[0] + (i32s) half) has_equivalent = true;
				if (ind1[0] + (i32s) half == ind2[1] && ind1[1] + (i32s) half == ind2[0]) has_equivalent = true;
				
				if (has_equivalent && (* it1).bt.GetValue() != (* it2).bt.GetValue())	// for graphics only...
				{
					(* it1).bt = bondtype('C');
				}
				
				if (has_equivalent) break;	// just a speed optimization...
			}
			
			if (!has_equivalent)
			{
				if (bonded_to_r && !bonded_to_p) bvect_r.push_back(& (* it1));
				else if (!bonded_to_r && bonded_to_p) bvect_p.push_back(& (* it1));
				else
				{
					cout << "fatal bug in tss found!" << endl;
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	
	for (i32u n1 = 0;n1 < bvect_r.size();n1++)
	{
		for (i32u n2 = 0;n2 < 2;n2++)
		{
			atom * atmr = bvect_r[n1]->atmr[n2];
			
			i32s atmi = NOT_DEFINED;
			for (i32u n3 = 0;n3 < half;n3++)
			{
				if (atab1[n3] == atmr)		// search reactant side!
				{
					atmi = n3;
					break;
				}
			}
			
			if (atmi == NOT_DEFINED) { cout << "search r1 failed!" << endl; exit(EXIT_FAILURE); }
			
			bool unique = true;
			for (i32u n3 = 0;n3 < patoms.size();n3++)
			{
				if (patoms[n3] == (i32u) atmi) unique = false;
			}
			
			if (unique) patoms.push_back((i32u) atmi);
		}
		
		bond tmpb = bond(bvect_r[n1]->atmr[0], bvect_r[n1]->atmr[1], bondtype());
		iter_bl itb = find(mdl->GetBondsBegin(), mdl->GetBondsEnd(), tmpb);
		
		if (itb == mdl->GetBondsEnd()) { cout << "search r2 failed!" << endl; exit(EXIT_FAILURE); }
		
		rbonds.push_back(& (* itb));	// we are on the reactant side -> the bond is ok!
	}
	
	for (i32u n1 = 0;n1 < bvect_p.size();n1++)
	{
		i32s stored_atmi[2] = { NOT_DEFINED, NOT_DEFINED };
		
		for (i32u n2 = 0;n2 < 2;n2++)
		{
			atom * atmr = bvect_p[n1]->atmr[n2];
			
			i32s atmi = NOT_DEFINED;
			for (i32u n3 = 0;n3 < half;n3++)
			{
				if (atab2[n3] == atmr)		// search product side!
				{
					atmi = n3;
					break;
				}
			}
			
			stored_atmi[n2] = atmi;
			if (atmi == NOT_DEFINED) { cout << "search p1 failed!" << endl; exit(EXIT_FAILURE); }
			
			bool unique = true;
			for (i32u n3 = 0;n3 < patoms.size();n3++)
			{
				if (patoms[n3] == (i32u) atmi) unique = false;
			}
			
			if (unique) patoms.push_back((i32u) atmi);
		}
		
		bond tmpb = bond(bvect_p[n1]->atmr[0], bvect_p[n1]->atmr[1], bondtype());
		iter_bl itb = find(mdl->GetBondsBegin(), mdl->GetBondsEnd(), tmpb);
		
		if (itb == mdl->GetBondsEnd()) { cout << "search p2 failed!" << endl; exit(EXIT_FAILURE); }
		
		// at the product side, we need to duplicate the bond in reactant side!!!
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		
		tmpb = bond(atab1[stored_atmi[0]], atab1[stored_atmi[1]], (* itb).bt);
		mdl->AddBond(tmpb); pbonds.push_back(& mdl->GetLastBond());
		
		mdl->RemoveBond(itb);		// this is in fact not needed ; would be deleted anyway below!!!
	}
	
	// remove the duplicate atoms...
	
	for (i32u n1 = 0;n1 < half;n1++)
	{
		iter_al it1 = mdl->GetAtomsBegin();
		while (it1 != mdl->GetAtomsEnd())
		{
			if (& (* it1) == atab2[n1]) break;
			else it1++;
		}
		
		if (it1 == mdl->GetAtomsEnd()) { cout << "tss : atom search failed!" << endl; exit(EXIT_FAILURE); }
		
		mdl->RemoveAtom(it1);
	}
	
	delete[] atab1;
	delete[] atab2;
	
	// ...and then superimpose the initial reactant/product structures.
	
	superimpose si(mdl, 0, 1);
	for (i32s n1 = 0;n1 < 100;n1++)
	{
		si.TakeCGStep(conjugate_gradient::Newton2An);
		cout << "step = " << n1 << " value = " << si.optval << endl;
	}
	
	si.Transform();
	
	// ok, the geometry setup is ready.
	
	target[0] = new f64[mdl->GetAtomCount() * 3];
	SetTarget(0, 1);	// use products as target for reactants...
	
	target[1] = new f64[mdl->GetAtomCount() * 3];
	SetTarget(1, 0);	// use reactants as target for products...
	
	// if no patoms were found (that is, no change in bonds; for example amine inversion) set the atom that shifts most!
	
	if (!patoms.size())
	{
		i32s index = NOT_DEFINED; fGL maxd = 0.0; i32s counter = 0;
		for (iter_al it1 = mdl->GetAtomsBegin();it1 != mdl->GetAtomsEnd();it1++)
		{
			const fGL * crd1 = (* it1).GetCRD(0);
			const fGL * crd2 = (* it1).GetCRD(1);
			
			fGL dist = 0.0; fGL tmp1;
			tmp1 = crd2[0] - crd1[0]; dist += tmp1 * tmp1;
			tmp1 = crd2[1] - crd1[1]; dist += tmp1 * tmp1;
			tmp1 = crd2[2] - crd1[2]; dist += tmp1 * tmp1;
			dist = sqrt(dist);
			
			if (dist > maxd)
			{
				maxd = dist;
				index = counter;
			}
			
			counter++;
		}
		
		patoms.push_back(index);
		
		char txtbuff[256]; ostrstream txts(txtbuff, sizeof(txtbuff));
		txts << "no patoms found; using " << index << " as a default." << endl << ends;
		mdl->PrintToLog(txtbuff); cout << txtbuff;
	}
	
	// refine the initial structures + other initializations.
	
	suqm->DiscardCurrentEngine();	// make sure to release mopac_lock!!!
	
	for (i32s n1 = 0;n1 < 2;n1++)
	{
		engine * eng1 = suqm->CreateEngineByIndex(suqm->GetCurrEngIndex());
		eng1_qm * eng2 = dynamic_cast<eng1_qm *>(eng1);
		
		if (eng2 == NULL)
		{
			cout << "tss : could not create eng object!" << endl;
			exit(EXIT_FAILURE);
		}
		
		CopyCRD(mdl, eng2, n1);
		eng2->tss_ref_str = target[n1];
		eng2->tss_force_const = fc[n1];
		
		geomopt * opt = new geomopt(eng2, 20, 0.0125, 10.0);		// optimal settings?!?!?
		
		eng2->Compute(0);
		f64 prev = eng2->energy;
		
		i32s failed_steps = 0;
		for (i32s n2 = 0;n2 < 100;n2++)
		{
			opt->TakeCGStep(conjugate_gradient::Newton2An);
			cout << n2 << " " << opt->optval << " " << opt->optstp << endl;
			
			f64 diff = prev - opt->optval;
			if (diff > 0.001) failed_steps = 0;
			else failed_steps++;
			
			prev = opt->optval;
			
			if (failed_steps >= 20) break;
		}
		
		delete opt;
		
		// store the calculated energy and the optimized structure...
		
		eng2->Compute(0);
		energy1[n1] = eng2->energy;
		energy2[n1] = eng2->energy - eng2->tss_force_const * eng2->tss_delta_ene;
		CopyCRD(eng2, mdl, n1);
		
		delete eng1;
	}
	
	SetTarget(0, 1);	// use products as target for reactants...
	SetTarget(1, 0);	// use reactants as target for products...
	
	t[0] = energy1[0] + deltae;
	t[1] = energy1[1] + deltae;
	
	p[0] = -10000.0;	// set initial reaction coordinates...
	p[1] = +10000.0;	// ...that are used only to sort the results.
	
	ready[0] = ready[1] = false;
	last_de[0] = last_de[1] = 0.0;
}

transition_state_search::~transition_state_search(void)
{
	if (target[0] != NULL) delete[] target[0];
	if (target[1] != NULL) delete[] target[1];
}

void transition_state_search::Run(i32s * prev_not_stored)
{
	if (init_failed) { cout << "tss init failed!" << endl; exit(EXIT_FAILURE); }
	
	for (i32s n1 = 0;n1 < 2;n1++)
	{
		if (prev_not_stored[n1] == 1) continue;
		
		// perform constrained optimization...
		
		engine * eng1 = suqm->CreateEngineByIndex(suqm->GetCurrEngIndex());
		eng1_qm * eng2 = dynamic_cast<eng1_qm *>(eng1);
		
		if (eng2 == NULL)
		{
			cout << "tss : could not create eng object!" << endl;
			exit(EXIT_FAILURE);
		}
		
		CopyCRD(mdl, eng2, n1);
		eng2->tss_ref_str = target[n1];
		eng2->tss_force_const = fc[n1];
		
		geomopt * opt = new geomopt(eng2, 10, 0.0125, 10.0);		// optimal settings?!?!?
		
		i32s n2 = 0;
		while (true)
		{
			opt->TakeCGStep(conjugate_gradient::Newton2An);
			cout << n2 << " " << opt->optval << " " << opt->optstp << endl;
			
			if (n2 != 0 && !(n2 % 20))
			{
				if (eng2->tss_delta_ene < 1.0e-15)	// flipped??? test using zero???
				{
////////////////////////////////////////////////////////////////
//cout << "FLIPPED!!! n1= " << n1 << endl; int yy;cin>>yy;
////////////////////////////////////////////////////////////////
					iter_al it1; i32u counter = 0;
					for (it1 = mdl->GetAtomsBegin();it1 != mdl->GetAtomsEnd();it1++)
					{
						eng2->crd[counter] = target[!n1][counter++];
						eng2->crd[counter] = target[!n1][counter++];
						eng2->crd[counter] = target[!n1][counter++];
					}
					
					ready[n1] = true;
					break;
				}
				
				const f64 limit = deltae * 0.10;
				if (opt->optval > (t[n1] - limit) && opt->optval < (t[n1] + limit)) break;
				
				f64 diff = t[n1] - opt->optval;
				eng2->tss_force_const += diff / eng2->tss_delta_ene * 0.50;	// optimal???
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//cout << "n2 = " << n2 << " E = " << opt->optval << " t = " << t[n1] << " fc = " << eng2->tss_force_const << " de = " << eng2->tss_delta_ene << endl; int qq; cin >> qq;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			}
			
			n2++;
		}
		
		delete opt;
		
		if (n1 == 0) p[n1] += 1.0;
		if (n1 == 1) p[n1] -= 1.0;
		
		// copy the optimized structure and update energy...
		
		eng2->Compute(0);
		energy1[n1] = eng2->energy;
		energy2[n1] = eng2->energy - eng2->tss_force_const * eng2->tss_delta_ene;
		fc[n1] = eng2->tss_force_const; last_de[n1] = eng2->tss_delta_ene;
		CopyCRD(eng2, mdl, n1);
		
		delete eng1;
	}
}

void transition_state_search::UpdateTargets(bool * update)
{
	if (init_failed) { cout << "tss init failed!" << endl; exit(EXIT_FAILURE); }
	
	if (!update[0] && !update[1]) return;
	
	bool check_fc[2] = { false, false };
	for (i32s n1 = 0;n1 < 2;n1++)
	{
		if (!update[n1]) continue;
		
		t[n1] = energy1[n1] + deltae;
		
		SetTarget(!n1, n1);
		check_fc[!n1] = true;
	}
	
	for (i32s n1 = 0;n1 < 2;n1++)
	{
		if (last_de[n1] < 1.0e-15) continue;	// flipped??? test using zero???
		if (!check_fc[n1]) continue;
		
		engine * eng1 = suqm->CreateEngineByIndex(suqm->GetCurrEngIndex());
		eng1_qm * eng2 = dynamic_cast<eng1_qm *>(eng1);
		
		if (eng2 == NULL)
		{
			cout << "tss : could not create eng object!" << endl;
			exit(EXIT_FAILURE);
		}
		
		CopyCRD(mdl, eng2, n1);
		eng2->tss_ref_str = target[n1];
		eng2->tss_force_const = fc[n1];
		
		eng2->Compute(0);
		
		fc[n1] *= last_de[n1] / eng2->tss_delta_ene;
		last_de[n1] = eng2->tss_delta_ene;
		
		delete eng1;
	}
}

void transition_state_search::SetTarget(i32s index, i32s cset)
{
	if (init_failed) { cout << "tss init failed!" << endl; exit(EXIT_FAILURE); }
	
	iter_al it1; i32u counter = 0;
	for (it1 = mdl->GetAtomsBegin();it1 != mdl->GetAtomsEnd();it1++)
	{
		const fGL * crd = (* it1).GetCRD(cset);
		
		target[index][counter++] = crd[0];
		target[index][counter++] = crd[1];
		target[index][counter++] = crd[2];
	}
}

/*################################################################################################*/

// Szabo A, Ostlund N : "Modern Quantum Chemistry : introduction to advanced electronic
// structure theory" McGraw-Hill, 1989

stationary_state_search::stationary_state_search(engine * p1, i32s p2, f64 p3, f64 p4) : conjugate_gradient(p2, p3, p4)
{
	eng = p1;
	SetNGDelta(1.0e-4);
	
	d1 = new f64[eng->GetAtomCount() * 3];
	
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			AddVar(& eng->crd[n1 * 3 + n2], & d1[n1 * 3 + n2]);
		}
	}
}

stationary_state_search::~stationary_state_search(void)
{
	delete[] d1;
}

f64 stationary_state_search::GetValue(void)
{
	eng->Compute(1);	// request energy and gradient!!!
	
	f64 sigma = 0.0;
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			// the scaling factor 0.01 here is just for conveniency;
			// conjgrad/linesearch is easier to handle this way.
			
			f64 tmp1 = eng->d1[n1 * 3 + n2] * 0.01;
			sigma += tmp1 * tmp1;
		}
	}
	
	return sigma;
}

/*################################################################################################*/

// eof
