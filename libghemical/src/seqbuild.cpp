// SEQBUILD.CPP

// Copyright (C) 1998 Tommi Hassinen, Geoff Hutchison.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "seqbuild.h"

#include "libdefine.h"

#include <string.h>

#include <iomanip>
#include <iostream>
#include <strstream>
using namespace std;

/*################################################################################################*/

chn_info::chn_info(void)
{
	type = chn_info::not_defined;
	
	id_mol = NOT_DEFINED;
	id_chn = NOT_DEFINED;
	
	length = NOT_DEFINED;
	
	sequence = NULL;
	
	ss_state = NULL;
	p_state = NULL;
	
	description = NULL;
}

chn_info::chn_info(chn_type p1, i32s p2)
{
	type = p1;
	
	id_mol = NOT_DEFINED;
	id_chn = NOT_DEFINED;
	
	length = p2;
	
	sequence = new char[length];
	
	ss_state = NULL;
	p_state = NULL;
	
	description = NULL;
}

chn_info::chn_info(const chn_info & p1)
{
	type = p1.type;
	
	id_mol = p1.id_mol;
	id_chn = p1.id_chn;
	
	length = p1.length;
	
	if (p1.sequence != NULL)
	{
		sequence = new char[length];
		for (i32s n1 = 0;n1 < length;n1++) sequence[n1] = p1.sequence[n1];
	}
	else sequence = NULL;
	
	if (p1.ss_state != NULL)
	{
		ss_state = new char[length];
		for (i32s n1 = 0;n1 < length;n1++) ss_state[n1] = p1.ss_state[n1];
	}
	else ss_state = NULL;

	if (p1.p_state != NULL)
	{
		p_state = new char[length];
		for (i32s n1 = 0;n1 < length;n1++) p_state[n1] = p1.p_state[n1];
	}
	else p_state = NULL;
	
	if (p1.description != NULL)
	{
		description = new char[strlen(p1.description) + 1];
		strcpy(description, p1.description);
	}
	else description = NULL;
}

chn_info::~chn_info(void)
{
	if (sequence != NULL) delete[] sequence;
	
	if (ss_state != NULL) delete[] ss_state;
	if (p_state != NULL) delete[] p_state;
	
	if (description != NULL) delete[] description;
}

/*################################################################################################*/

sequencebuilder::sequencebuilder(chn_info::chn_type ct, char * filename)
{

	type = ct;
	
	ifstream file;
	file.unsetf(ios::dec | ios::oct | ios::hex);
	
	model::OpenLibDataFile(file, false, filename);
	

	// read information about the main chain...
	
	while (file.peek() != 'M') file.getline(buffer, sizeof(buffer));
	file.getline(buffer, sizeof(buffer));
	
	
	while (file.peek() != 'E')
	{
		sb_data_atm newatm; file >> newatm;
		while (file.peek() != '(') file.get();
		newatm.tr = new typerule(& file, & cout);
		file.getline(buffer, sizeof(buffer));
		main_vector.push_back(newatm);
	}
	
	// read descriptions about the chain initiation...
	
	while (file.peek() != 'H') file.getline(buffer, sizeof(buffer));
	file.getline(buffer, sizeof(buffer));
	
	while (file.peek() != 'E')
	{
		typerule newrule = typerule(& file, & cout);
		file.getline(buffer, sizeof(buffer));
		head_vector.push_back(newrule);
	}
	
	// read descriptions about the chain termination...
	
	while (file.peek() != 'T') file.getline(buffer, sizeof(buffer));
	file.getline(buffer, sizeof(buffer));

	
	while (file.peek() != 'E')
	{
		typerule newrule = typerule(& file, & cout);
		file.getline(buffer, sizeof(buffer));
		tail_vector.push_back(newrule);
	}
	
	// read the default modifications for residues...
	
	while (file.peek() != 'B') file.getline(buffer, sizeof(buffer));
	file.getline(buffer, sizeof(buffer)); mod[0] = new sb_data_res; mod[0]->ReadModification(file);
	
	while (file.peek() != 'H') file.getline(buffer, sizeof(buffer));
	file.getline(buffer, sizeof(buffer)); mod[1] = new sb_data_res; mod[1]->ReadModification(file);
	
	while (file.peek() != 'T') file.getline(buffer, sizeof(buffer));
	file.getline(buffer, sizeof(buffer)); mod[2] = new sb_data_res; mod[2]->ReadModification(file);

	
	// read descriptions for the residues...
	
	while (file.peek() != 'E')
	{
		if (file.peek() == 'R')
		{
			sb_data_res newres; file >> newres;
			residue_vector.push_back(newres);
		}
		else file.getline(buffer, sizeof(buffer));
	}
	
	// ready!!!
	
	file.close();
}

sequencebuilder::~sequencebuilder(void)
{
	delete mod[0];
	delete mod[1];
	delete mod[2];
}

// here we use only the first coordinate set, no matter how many there are...

void sequencebuilder::Build(model * mdl, char * sequence, f64 * tor)
{
	fGL b1crd[3] = { 1.0, 1.0, 0.0 };	// these should be const...
	fGL b2crd[3] = { 1.0, 0.0, 0.0 };
	fGL b3crd[3] = { 0.0, 0.0, 0.0 };
	
	atom base[3] =
	{
		atom(element(), b1crd, mdl->cs_vector.size()),		// these should be const...
		atom(element(), b2crd, mdl->cs_vector.size()),
		atom(element(), b3crd, mdl->cs_vector.size())
	};
	
	id_vector.resize(0); ref_vector.resize(0);
	id_vector.push_back(main_vector[0].prev[0]); ref_vector.push_back(& base[0]);
	id_vector.push_back(main_vector[0].prev[1]); ref_vector.push_back(& base[1]);
	id_vector.push_back(main_vector[0].prev[2]); ref_vector.push_back(& base[2]);
	
	for (i32u n1 = 0;n1 < strlen(sequence);n1++)
	{
		for (i32u n2 = 0;n2 < main_vector.size();n2++)
		{
			atom * prev[3];
			f64 ic[3]; fGL crd[3];
			for (i32s n3 = 0;n3 < 3;n3++)
			{
				i32s n4 = 0; while (id_vector[n4] != main_vector[n2].prev[n3]) n4++;
				prev[n3] = ref_vector[n4]; ic[n3] = main_vector[n2].ic1[n3];
			}
			
			if (main_vector[n2].ic2 > -1) ic[2] += tor[main_vector[n2].ic2];
			Convert(prev, ic, crd);
			
			id_vector.push_back(main_vector[n2].id[0]);
			atom newatom(main_vector[n2].el, crd, mdl->cs_vector.size());
			mdl->Add_Atom(newatom);
			
			ref_vector.push_back(& mdl->atom_list.back());
			
			if (n1 || n2)
			{
				bond newbond(prev[0], & mdl->atom_list.back(), main_vector[n2].bt[0]);
				mdl->AddBond(newbond);
			}
		}
		
		if (!n1) Build(mdl, mod[1], tor);
		else if (n1 == (strlen(sequence) - 1)) Build(mdl, mod[2], tor);
		else Build(mdl, mod[0], tor);
		
		i32u res = 0;
		while (residue_vector[res].symbol != sequence[n1])
		{
			if (res != residue_vector.size()) res++;
			else break;
		}
		
		if (res != residue_vector.size()) Build(mdl, & residue_vector[res], tor);
		else cout << "unknown residue " << sequence[n1] << endl;
		
		vector<i32s> tmpv1; atmr_vector tmpv2;
		for (/*i32u*/ n2 = 0;n2 < id_vector.size();n2++)
		{
			if (id_vector[n2] & 0xFF00) continue;
			tmpv1.push_back(id_vector[n2]); tmpv2.push_back(ref_vector[n2]);
		}
		
		id_vector.resize(0); ref_vector.resize(0);
		for (/*i32u*/ n2 = 0;n2 < tmpv1.size();n2++)
		{
			id_vector.push_back(tmpv1[n2] + 0xFF00);
			ref_vector.push_back(tmpv2[n2]);
		}
	}
	
	// do centering. can't affect current_eng object since
	// above many atoms/bonds added -> current_eng deleted!!!
	
	mdl->CenterCRDSet(0, true);
}

// here we will identify the sequences (if there is any), write down the sequences
// into model::ref_civ and update the /mol/chn/res-numbering...

// only model::UpdateChains() should call this???
// only model::UpdateChains() should call this???
// only model::UpdateChains() should call this???

void sequencebuilder::Identify(model * mdl)
{
	if (!mdl->IsGroupsClean())
	{
		cout << "BUG: is_groups_clean was false at sequencebuilder::Identify()." << endl;
		exit(EXIT_FAILURE);	//mdl->UpdateGroups();
	}
	
	if (mdl->ref_civ == NULL)
	{
		cout << "BUG: mdl->ref_civ == NULL at sequencebuilder::Identify()." << endl;
		exit(EXIT_FAILURE);
	}
	
cout << "sequencebuilder::Identify() starts..." << endl;
cout << "nmol = " << mdl->nmol << endl;
	
	// here we will find all possible chains and identify them.
	
	for (i32s n1 = 0;n1 < mdl->nmol;n1++)
	{
		iter_al range[2];
		mdl->GetRange(0, n1, range);
		
		vector<atom *> head_atoms;
		vector<atom *> tail_atoms;
		
		for (iter_al it1 = range[0];it1 != range[1];it1++)
		{
			i32s tmp1 = (* it1).el.GetAtomicNumber();
			
			if (tmp1 == main_vector.front().el.GetAtomicNumber())		// look for heads...
			{
				for (i32u n2 = 0;n2 < head_vector.size();n2++)
				{
					bool flag = head_vector[n2].Check(mdl, & (* it1), 0);
					if (!flag) continue; head_atoms.push_back(& (* it1)); break;
				}
			}
			
			if (tmp1 == main_vector.back().el.GetAtomicNumber())		// look for tails...
			{
				for (i32u n2 = 0;n2 < tail_vector.size();n2++)
				{
					bool flag = tail_vector[n2].Check(mdl, & (* it1), 0);
					if (!flag) continue; tail_atoms.push_back(& (* it1)); break;
				}
			}
		}
		
		cout << "found " << head_atoms.size() << " possible heads and ";
		cout << tail_atoms.size() << " possible tails." << endl;
		
		// now we have all possible head/tail atoms. next we have to check
		// all possible paths between them to find all possible main chains...
		
		path_vector.resize(0);
		for (i32u n2 = 0;n2 < head_atoms.size();n2++)
		{
			for (i32u n3 = 0;n3 < tail_atoms.size();n3++)
			{
				FindPath(mdl, head_atoms[n2], tail_atoms[n3]);
			}
		}
		
		if (path_vector.size()) cout << path_vector.size() << " chains:" << endl;
		
		for (/*i32s*/ n2 = 0;n2 < (i32s) path_vector.size();n2++)
		{
			for (i32s n3 = 0;n3 < ((i32s) path_vector[n2].size()) - 1;n3++)		// tag the main-chain bonds...
			{
				iter_cl it1 = path_vector[n2][n3]->cr_list.begin();
				while ((* it1).atmr != path_vector[n2][n3 + 1]) it1++;
				(* it1).bndr->flags[0] = true;
			}
			
			vector<char> sequence;
			bool head_blocked = !head_vector[0].Check(mdl, path_vector[n2].front(), 1);	// what exactly is this???
			bool tail_blocked = !tail_vector[0].Check(mdl, path_vector[n2].back(), 1);	// what exactly is this???
			
			i32u acount = 0;	// atom counter
			i32u rcount = 0;	// residue counter
			
			while (acount < path_vector[n2].size())
			{
				i32u rsize = main_vector.size();
				
				// rsize is the residue size (how many atoms it will chop away from the
				// path_vector). we use it to determine whether we have reached the last
				// residue, and to increment acount.
				
				bool is_first = !head_blocked && !rcount;
				bool is_last = !tail_blocked && (acount + rsize >= path_vector[n2].size());
				
				// now we start identifying the residues.
				
				// we make a simple template for each of them, compare those to
				// the residue descriptions we have, and pick the closest match.
				// there may not be any missing atoms, and the closest match is
				// the one with largest number of correct atoms identified.
				
				// some problems with the blocked head/tail residues???
				
				i32s tmp1[2] = { NOT_DEFINED, NOT_DEFINED };
				for (i32s n4 = 0;n4 < (i32s) residue_vector.size();n4++)	// n3???
				{
					vector<sb_tdata> tdata;
					BuildTemplate(tdata, n4, is_first, is_last);
					
					for (i32s n5 = 0;n5 < (i32s) main_vector.size();n5++)
					{
						tdata[n5].ref = path_vector[n2][acount + n5];
					}
					
					bool result = CheckTemplate(tdata, 0);
					if (result && (tmp1[0] == NOT_DEFINED || ((i32s) tdata.size()) > tmp1[1]))
					{
						tmp1[0] = n4;
						tmp1[1] = (i32s) tdata.size();
					}
				}
				
				if (tmp1[0] == NOT_DEFINED)
				{
					cout << "WARNING : residue " << rcount << " was of unknown type!!!" << endl;
					
					sequence.push_back('?');				// handle the sequence...
					
					for (i32u n3 = 0;n3 < main_vector.size();n3++)		// handle the numbers...
					{
						path_vector[n2][acount + n3]->builder_res_id = NOT_DEFINED;	// WHAT HERE???
						
						path_vector[n2][acount + n3]->id[1] = mdl->ref_civ->size();
						path_vector[n2][acount + n3]->id[2] = rcount;
					}
				}
				else
				{
					sequence.push_back(residue_vector[tmp1[0]].symbol);
					
					// rebuild the best matching template and use that to
					// set up the atom ID numbers...
					
					vector<sb_tdata> tdata;
					BuildTemplate(tdata, tmp1[0], is_first, is_last);
					
					for (i32s n4 = 0;n4 < (i32s) main_vector.size();n4++)
					{
						tdata[n4].ref = path_vector[n2][acount + n4];
					}
					
					CheckTemplate(tdata, 0);
					for (/*i32s*/ n4 = 0;n4 < (i32s) tdata.size();n4++)
					{
						tdata[n4].ref->builder_res_id = (residue_vector[tmp1[0]].id << 8) + tdata[n4].id[0];
						
						tdata[n4].ref->id[1] = mdl->ref_civ->size();
						tdata[n4].ref->id[2] = rcount;
					}
					
					// finally do a hard-coded fix for unambiguous TRP residues...
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
					
					if (type == chn_info::amino_acid && residue_vector[tmp1[0]].symbol == 'W')
					{
						atom * ref23 = NULL;
						atom * ref25 = NULL;
						atom * ref26 = NULL;
						atom * ref28 = NULL;
						atom * ref29 = NULL;
						
						for (i32s n4 = 0;n4 < (i32s) tdata.size();n4++)
						{
							if ((tdata[n4].ref->builder_res_id & 0xFF) == 0x23) ref23 = tdata[n4].ref;
							if ((tdata[n4].ref->builder_res_id & 0xFF) == 0x25) ref25 = tdata[n4].ref;
							if ((tdata[n4].ref->builder_res_id & 0xFF) == 0x26) ref26 = tdata[n4].ref;
							if ((tdata[n4].ref->builder_res_id & 0xFF) == 0x28) ref28 = tdata[n4].ref;
							if ((tdata[n4].ref->builder_res_id & 0xFF) == 0x29) ref29 = tdata[n4].ref;
						}
						
						if (!ref23) { cout << "TRP-fix : ref23 not found!" << endl; exit(EXIT_FAILURE); }
						if (!ref25) { cout << "TRP-fix : ref25 not found!" << endl; exit(EXIT_FAILURE); }
						if (!ref26) { cout << "TRP-fix : ref26 not found!" << endl; exit(EXIT_FAILURE); }
						if (!ref28) { cout << "TRP-fix : ref28 not found!" << endl; exit(EXIT_FAILURE); }
						if (!ref29) { cout << "TRP-fix : ref29 not found!" << endl; exit(EXIT_FAILURE); }
						
						bond b1 = bond(ref23, ref25, bondtype('S'));
						iter_bl itb1 = find(mdl->GetBondsBegin(), mdl->GetBondsEnd(), b1);
						bool b1exists = (itb1 != mdl->GetBondsEnd());
						
						bond b2 = bond(ref23, ref29, bondtype('S'));
						iter_bl itb2 = find(mdl->GetBondsBegin(), mdl->GetBondsEnd(), b2);
						bool b2exists = (itb2 != mdl->GetBondsEnd());
						
						if (b1exists == b2exists) { cout << "TRP-fix error #1" << endl; exit(EXIT_FAILURE); }
						if (b1exists && !b2exists)
						{
							cout << "TRP-fix!!!" << endl;
							
							// ok, this seems to be "wrong" way...
							// fix by exhanging 25<->29 and 26<->28.
							
							int tmpid;
							
							tmpid = ref25->builder_res_id;
							ref25->builder_res_id = ref29->builder_res_id;
							ref29->builder_res_id = tmpid;
							
							tmpid = ref26->builder_res_id;
							ref26->builder_res_id = ref28->builder_res_id;
							ref28->builder_res_id = tmpid;
						}
					}
				}
				
				rcount++;
				acount += rsize;
			}
			
			// next we will write down the sequence...
			
			chn_info newinfo(type, sequence.size());
			
			newinfo.id_mol = n1;
			newinfo.id_chn = mdl->ref_civ->size();
			
			for (/*i32u*/ n3 = 0;n3 < sequence.size();n3++) newinfo.sequence[n3] = sequence[n3];
			
			mdl->ref_civ->push_back(newinfo);
			
			// and make some output (using FASTA format again)...
			
			cout << "> chain " << newinfo.id_chn;
			cout << ", length " << newinfo.length << ":" << endl;
			
			for (/*i32u*/ n3 = 0;n3 < sequence.size();n3++)
			{
				cout << sequence[n3];
				
				bool is_break = !((n3 + 1) % 50);
				bool is_end = ((n3 + 1) == sequence.size());
				
				if (is_break || is_end) cout << endl;
			}
			
			// finally un-tag the main-chain bonds...
			
			for (/*i32s*/ n3 = 0;n3 < ((i32s) path_vector[n2].size()) - 1;n3++)
			{
				iter_cl it1 = path_vector[n2][n3]->cr_list.begin();
				while ((* it1).atmr != path_vector[n2][n3 + 1]) it1++;
				(* it1).bndr->flags[0] = false;
			}
		}
		
		// now check the HYDROGENS that are ignored by the seqbuilder generally.
		// for hydrogens, set the same id's that the heavy atom has if they are valid ones.
		{
		for (iter_al it1 = range[0];it1 != range[1];it1++)
		{
			if ((* it1).el.GetAtomicNumber() != 1) continue;
			
			if ((* it1).cr_list.size() != 1)
			{
				cout << "WARNING : seqbuild : H atom with abnormal connectivity found." << endl;
				continue;
			}
			
			atom * heavy = (* it1).cr_list.front().atmr;
			if (heavy->id[1] < 0) continue;		// id < 0 means NOT_DEFINED...
			if (heavy->id[2] < 0) continue;		// id < 0 means NOT_DEFINED...
			
			// ok, set the heavy atom id's to hydrogen as well...
			
			(* it1).id[1] = heavy->id[1];
			(* it1).id[2] = heavy->id[2];
		}
		}
	}
	
	// traditionally, we sorted the atom list here to make the newly assigned chains in nice order.
	// nowadays, this is done at model::UpdateChains() since there might be many different seqbuilder
	// objects (like for amino/nucleic acid variants) and we like to use them all...
}

void sequencebuilder::Build(model * mdl, sb_data_res * res, f64 * tor)
{
	for (i32u n1 = 0;n1 < res->atm_vector.size();n1++)
	{
		atom * prev[3];
		f64 ic[3]; fGL crd[3];
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			i32s n3 = 0; while (id_vector[n3] != res->atm_vector[n1].prev[n2]) n3++;
			prev[n2] = ref_vector[n3]; ic[n2] = res->atm_vector[n1].ic1[n2];
		}
		
		if (res->atm_vector[n1].ic2 > -1) ic[2] += tor[res->atm_vector[n1].ic2];
		Convert(prev, ic, crd);
		
		id_vector.push_back(res->atm_vector[n1].id[0]);
		atom newatom(res->atm_vector[n1].el, crd, mdl->cs_vector.size());
		mdl->Add_Atom(newatom);
		
		ref_vector.push_back(& mdl->atom_list.back());
		
		bond newbond(prev[0], & mdl->atom_list.back(), res->atm_vector[n1].bt[0]);
		mdl->AddBond(newbond);
	}
	
	for (/*i32u*/ n1 = 0;n1 < res->bnd_vector.size();n1++)
	{
		atom * tmp1[2]; i32s tmp2;
		tmp2 = 0; while (id_vector[tmp2] != res->bnd_vector[n1].atm[0]) tmp2++; tmp1[0] = ref_vector[tmp2];
		tmp2 = 0; while (id_vector[tmp2] != res->bnd_vector[n1].atm[1]) tmp2++; tmp1[1] = ref_vector[tmp2];
		bond newbond(tmp1[0], tmp1[1], res->bnd_vector[n1].bt);
		mdl->AddBond(newbond);
	}
}

void sequencebuilder::Convert(atom * prev[], f64 * ic, fGL * crd)
{
	v3d<fGL> v1 = v3d<fGL>(prev[0]->GetCRD(0), prev[1]->GetCRD(0));
	
	v3d<fGL> tmp1 = v3d<fGL>(prev[1]->GetCRD(0), prev[2]->GetCRD(0));
	v3d<fGL> tmp2 = v1 * (tmp1.spr(v1) / v1.spr(v1));
	v3d<fGL> v2 = tmp1 - tmp2;
	
	v3d<fGL> v3 = v2.vpr(v1);
	
	v1 = v1 * (ic[0] * cos(ic[1]) / v1.len());
	v2 = v2 * (ic[0] * sin(ic[1]) * cos(ic[2]) / v2.len());
	v3 = v3 * (ic[0] * sin(ic[1]) * sin(ic[2]) / v3.len());
	
	v3d<fGL> tmp3 = v3d<fGL>(prev[0]->GetCRD(0)); tmp3 = tmp3 + (v1 + (v2 + v3));
	for (i32s n1 = 0;n1 < 3;n1++) crd[n1] = tmp3.data[n1];
}

// idea is that this path-searching function should be used only inside a molecule.
// this way we are sure to find always at least one path. we collect references to
// atoms for all paths and collect this way all possible paths.

// different flags should be used in type checking and in this path-searching...

void sequencebuilder::FindPath(model * mdl, atom * ref1, atom * ref2, i32u index)
{
	if (index >= main_vector.size()) index = 0;
	
	if (ref1->el.GetAtomicNumber() != main_vector[index].el.GetAtomicNumber()) return;
	if (!main_vector[index].tr->Check(mdl, ref1, 0)) return;
	
	temporary_vector.push_back(ref1);
	
//	for (i32u debug = 0;debug < temporary_vector.size();debug++)
//	{
//		cout << temporary_vector[debug]->el.GetSymbol();
//	}	cout << endl;
	
	if (ref1 == ref2) path_vector.push_back(temporary_vector);
	else
	{
		for (iter_cl it1 = ref1->cr_list.begin();it1 != ref1->cr_list.end();it1++)
		{
			if ((* it1).bndr->flags[2]) continue;
			
			(* it1).bndr->flags[2] = true;
			FindPath(mdl, (* it1).atmr, ref2, (index + 1));
			(* it1).bndr->flags[2] = false;
		}
	}
	
	temporary_vector.pop_back();
}

void sequencebuilder::BuildTemplate(vector<sb_tdata> & tdata, i32s res, bool is_first, bool is_last)
{
	BuildTemplate(tdata, main_vector);
	
	if (is_first) BuildTemplate(tdata, mod[1]->atm_vector);
	else if (is_last) BuildTemplate(tdata, mod[2]->atm_vector);
	else BuildTemplate(tdata, mod[0]->atm_vector);
	
	BuildTemplate(tdata, residue_vector[res].atm_vector);
}

void sequencebuilder::BuildTemplate(vector<sb_tdata> & tdata, vector<sb_data_atm> & adata)
{
	for (i32u n1 = 0;n1 < adata.size();n1++)
	{
		sb_tdata newdata;
		newdata.id[0] = adata[n1].id[0];
		
		if (adata[n1].prev[0] & 0xFF00) newdata.id[1] = NOT_DEFINED;
		else newdata.id[1] = adata[n1].prev[0];
		
		newdata.el = adata[n1].el;
		newdata.bt = adata[n1].bt[1];
		newdata.ref = NULL;
		
		tdata.push_back(newdata);
	}
}

bool sequencebuilder::CheckTemplate(vector<sb_tdata> & tdata, i32s flag)
{
	vector<i32s> tmpv1;
	
	// look for a suitable starting point to start matching in the template.
	// if there is no such point we assume the template fits. however, the template
	// could match since it is too small (for exaple, the template for glycine will
	// match for all residues). therefore we have to test all templates and choose
	// the biggest matching one!!!
	
	i32u index = 0;
	while (index < tdata.size())
	{
		if (tdata[index].ref != NULL)
		{
			tmpv1.resize(0);
			for (i32u n1 = 0;n1 < tdata.size();n1++)
			{
				bool test1 = (tdata[n1].ref == NULL);
				bool test2 = (tdata[n1].id[1] == tdata[index].id[0]);
				if (test1 && test2) tmpv1.push_back(n1);
			}
			
			if (tmpv1.size()) break;
		}
		
		index++;
	}
	
	// if there is no suitable starting point, we have tested all atoms and
	// none of them is missing -> template fits -> TRUE!!!
	
	if (index == tdata.size()) return true;
	
	// find all possible bonds that we can use to continue testing...
	
	vector<crec> tmpv2; iter_cl it1;
	for (it1 = tdata[index].ref->cr_list.begin();it1 != tdata[index].ref->cr_list.end();it1++)
	{
		bool test1 = (* it1).bndr->flags[flag];
		bool test2 = ((* it1).atmr->el.GetAtomicNumber() != 1);
		if (!test1 && test2) tmpv2.push_back((* it1));
	}
	
	// if there is less bond that we need, the template can't match -> FALSE!!!
	
	if (tmpv2.size() < tmpv1.size()) return false;
	
	vector<i32s> tmpv3; tmpv3.resize(tmpv2.size());
	for (i32u n1 = 0;n1 < tmpv3.size();n1++) tmpv3[n1] = n1;
	
	// then we will check all bonds against the template in all possible permutations.
	// if some combination is a match then leave the ID-numbers untouched, clean the
	// bond flags and leave -> TRUE!!!
	
	while (true)
	{
		bool test = true;
		for (i32u n1 = 0;n1 < tmpv1.size();n1++)
		{
			i32s el = tdata[tmpv1[n1]].el.GetAtomicNumber();
			if (el != NOT_DEFINED && el != tmpv2[tmpv3[n1]].atmr->el.GetAtomicNumber()) test = false;
			
			i32s bt = tdata[tmpv1[n1]].bt.GetValue();
			if (bt != NOT_DEFINED && bt != tmpv2[tmpv3[n1]].bndr->bt.GetValue()) test = false;
		}
		
		if (test)
		{
			for (i32u n1 = 0;n1 < tmpv1.size();n1++)
			{
				tmpv2[tmpv3[n1]].bndr->flags[flag] = true;
				tdata[tmpv1[n1]].ref = tmpv2[tmpv3[n1]].atmr;
			}
			
			bool result = CheckTemplate(tdata, flag);
			
			for (/*i32u*/ n1 = 0;n1 < tmpv1.size();n1++)
			{
				tmpv2[tmpv3[n1]].bndr->flags[flag] = false;
				if (!result) tdata[tmpv1[n1]].ref = NULL;
			}
			
			if (result) return true;
		}
		
		if (!next_permutation(tmpv3.begin(), tmpv3.end())) break;
//		if (!next_permutation(tmpv3.begin(), tmpv3.end(), less<i32s>())) break;	// for WIN32 ???
	}
	
	// none of those combinations matched -> the template doesn't fit -> FALSE!!!
	
	return false;
}

/*################################################################################################*/

sb_data_atm::sb_data_atm(void)
{
	tr = NULL;
}

sb_data_atm::sb_data_atm(const sb_data_atm & p1)
{
	el = p1.el; ic2 = p1.ic2;
	bt[0] = p1.bt[0]; bt[1] = p1.bt[1];
	id[0] = p1.id[0]; id[1] = p1.id[1];
	
	if (p1.tr != NULL) tr = new typerule(* p1.tr);
	else tr = NULL;
	
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		prev[n1] = p1.prev[n1];
		ic1[n1] = p1.ic1[n1];
	}
}

sb_data_atm::~sb_data_atm(void)
{
	if (tr != NULL) delete tr;
}

istream & operator>>(istream & p1, sb_data_atm & p2)
{
	char buffer[256];
	while (p1.get() != 'M');
	p1 >> p2.id[0]; while (p1.get() != ':');
	p1 >> buffer; p2.el = element(buffer);
	p1 >> p2.prev[0] >> p2.prev[1] >> p2.prev[2];
	p1 >> p2.ic1[0] >> p2.ic1[1] >> p2.ic2 >> p2.ic1[2];
	
	p2.ic1[1] = M_PI * p2.ic1[1] / 180.0;
	p2.ic1[2] = M_PI * p2.ic1[2] / 180.0;
	
	p1 >> buffer;
	p2.bt[0] = bondtype(buffer[0]);
	
	p1 >> p2.id[1] >> buffer;
	p2.bt[1] = bondtype(buffer[0]);
	
	return p1;
}

ostream & operator<<(ostream & p1, sb_data_atm & p2)
{
	p1 << hex << "0x" << setw(4) << setfill('0') << p2.id << dec;
	p1 << " " << p2.el.GetSymbol() << " " << p2.bt[0].GetSymbol1();
	if (p2.tr != NULL) p1 << " " << (* p2.tr); p1 << " ";
	
	p1 << p2.ic1[0] << " " << p2.ic1[1] << " " << p2.ic2 << " " << p2.ic1[2] << " ";
	p1 << hex << "0x" << setw(4) << setfill('0') << p2.prev[0] << dec << " ";
	p1 << hex << "0x" << setw(4) << setfill('0') << p2.prev[1] << dec << " ";
	p1 << hex << "0x" << setw(4) << setfill('0') << p2.prev[2] << dec;
	
	return p1;
}

/*################################################################################################*/

sb_data_bnd::sb_data_bnd(void)
{
}

sb_data_bnd::~sb_data_bnd(void)
{
}

istream & operator>>(istream & p1, sb_data_bnd & p2)
{
	char buffer[256];
	while (p1.get() != ':');
	p1 >> p2.atm[0] >> p2.atm[1];
	p1 >> buffer; p2.bt = bondtype(buffer[0]);
	return p1;
}

/*################################################################################################*/

sb_data_res::sb_data_res(void)
{
	description = NULL;
}

sb_data_res::sb_data_res(const sb_data_res & p1)
{
	id = p1.id;
	symbol = p1.symbol;
	
	if (p1.description != NULL)
	{
		description = new char[strlen(p1.description) + 1];
		strcpy(description, p1.description);
	}
	else description = NULL;
	
	atm_vector = p1.atm_vector;
	bnd_vector = p1.bnd_vector;
}

sb_data_res::~sb_data_res(void)
{
	if (description != NULL) delete[] description;
}

void sb_data_res::ReadModification(istream & p1)
{
	char buffer[256];
	while (p1.peek() != 'E')
	{
		if (p1.peek() == 'A')
		{
			sb_data_atm newatm; /*(istream)*/p1 >> (sb_data_atm)newatm;
			p1.getline(buffer, sizeof(buffer));
			atm_vector.push_back(newatm);
			continue;
		}		
		if (p1.peek() == 'B')
		{
			sb_data_bnd newbnd; p1 >> newbnd;
			p1.getline(buffer, sizeof(buffer));
			bnd_vector.push_back(newbnd);
			continue;
		}		
		p1.getline(buffer, sizeof(buffer));

	}	
	p1.getline(buffer, sizeof(buffer));
}

istream & operator>>(istream & p1, sb_data_res & p2)
{
	char buffer[256];
	while (p1.get() != 'S'); p1 >> p2.id;
	while (p1.get() != ':'); p1 >> p2.symbol;
	
	while (p1.get() != '"');
	p1.getline(buffer, sizeof(buffer), '"');
	
	p2.description = new char[strlen(buffer) + 1];
	strcpy(p2.description, buffer);
	
	p1.getline(buffer, sizeof(buffer));
	
	while (p1.peek() != 'E')
	{
		if (p1.peek() == 'A')
		{
			sb_data_atm newatm; p1 >> newatm;
			p1.getline(buffer, sizeof(buffer));
			p2.atm_vector.push_back(newatm);
			continue;
		}
		
		if (p1.peek() == 'B')
		{
			sb_data_bnd newbnd; p1 >> newbnd;
			p1.getline(buffer, sizeof(buffer));
			p2.bnd_vector.push_back(newbnd);
			continue;
		}
		
		p1.getline(buffer, sizeof(buffer));
	}
	
	p1.getline(buffer, sizeof(buffer));
	return p1;
}

/*################################################################################################*/

// eof
