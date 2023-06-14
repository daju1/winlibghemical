// GEOMOPT.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "geomopt.h"
geomopt_param::geomopt_param(setup * su)
{
	confirm = false;
	show_dialog = true;

	enable_nsteps	= true;
	enable_grad		= false;
	enable_delta_e	= false;

	recalc_box = false;
	box_optimization = false;

	
	treshold_nsteps = 500;
	treshold_grad = 1.0e-3;
	treshold_delta_e = 1.0e-7;
	treshold_delta_e_min_nsteps = 100;
	
	setup1_qm * suqm = dynamic_cast<setup1_qm *>(su);
	if (suqm != NULL) treshold_nsteps = 500;		// override...

	box_opt = geomopt_param::box_optimization_type::no;
	don_t_move_fixed_atoms = true;
}
bool geomopt_param::Write(char * filename)
{
	FILE * stream;
	stream = fopen(filename, "wt");
	if (!stream) return false;

	fprintf(stream, "enable_nsteps = %d\n", enable_nsteps);
	fprintf(stream, "treshold_nsteps = %d\n", treshold_nsteps);
	fprintf(stream, "\n");

	fprintf(stream, "enable_grad = %d\n", enable_grad);
	fprintf(stream, "treshold_grad = %e\n", treshold_grad);
	fprintf(stream, "\n");

	fprintf(stream, "enable_delta_e = %d\n", enable_delta_e);
	fprintf(stream, "treshold_delta_e = %e\n", treshold_delta_e);
	fprintf(stream, "treshold_delta_e_min_nsteps = %d\n", treshold_delta_e_min_nsteps);
	fprintf(stream, "\n");

	switch(this->box_opt)
	{
	case geomopt_param::box_optimization_type::no:
		fprintf(stream, "box_optimization_type::no\n");
		break;
	case geomopt_param::box_optimization_type::xyz:
		fprintf(stream, "box_optimization_type::xyz\n");
		break;
	case geomopt_param::box_optimization_type::xy:
		fprintf(stream, "box_optimization_type::xy\n");
		break;
	case geomopt_param::box_optimization_type::z:
		fprintf(stream, "box_optimization_type::z\n");
		break;
	}

	fclose(stream);
	return true;
}

/*################################################################################################*/

geomopt::geomopt(engine * p1, i32s p2, f64 p3, f64 p4, bool don_t_move_fixed_atoms) : conjugate_gradient(p2, p3, p4)
{
	eng = p1;
		
	atom ** glob_atmtab = eng->GetSetup()->GetAtoms();

	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		if (don_t_move_fixed_atoms)
		{
			if (glob_atmtab[n1]->flags & ATOMFLAG_IS_LOCKED)
				continue;
		}

		for (i32s n2 = 0;n2 < 3;n2++)
		{
			AddVar(& eng->crd[n1 * 3 + n2], & eng->d1[n1 * 3 + n2]);
		}
	}
}

geomopt::~geomopt(void)
{
}

f64 geomopt::GetValue(void)
{
	eng->Compute(0);	// request energy
	return eng->energy;
}

f64 geomopt::GetGradient(void)
{
	eng->Compute(1);	// request energy and gradient
	return eng->energy;
}

/*################################################################################################*/
/*################################################################################################*/

geomopt_ex::geomopt_ex(i32s n_prob_atom, vector<i32s> & missed_atoms_list, i32s missed_dim, engine * p1, i32s p2, f64 p3, f64 p4) : conjugate_gradient(p2, p3, p4)
{
	eng = p1;

	vector<bool> b_atoms;
	b_atoms.resize(eng->GetAtomCount());

	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		b_atoms[n1] = true;
	}

	for (i32s i = 0; i < missed_atoms_list.size(); i++)
	{
		if(missed_atoms_list[i] >= 0 && missed_atoms_list[i] < eng->GetAtomCount())
		{
printf("missed_atoms_list[%d] = %d\n", i, missed_atoms_list[i]);
			b_atoms[missed_atoms_list[i]] = false;
		}
	}

	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		if (n1 == n_prob_atom)
		{
			// у пробного атома фиксируются все три координаты
			continue;
		}
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			if ( b_atoms[n1] || (!b_atoms[n1] && n2 != missed_dim) )
			{
				AddVar(& eng->crd[n1 * 3 + n2], & eng->d1[n1 * 3 + n2]);
printf("AddVar(& eng->crd[n1 * 3 + n2], & eng->d1[n1 * 3 + n2]); n1 = %d n2 = %d\n", n1, n2);
			}
		}
	}
}

geomopt_ex::geomopt_ex(vector<i32s> & missed_atoms_list, i32s missed_dim, engine * p1, i32s p2, f64 p3, f64 p4) : conjugate_gradient(p2, p3, p4)
{
	eng = p1;

	vector<bool> b_atoms;
	b_atoms.resize(eng->GetAtomCount());

	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		b_atoms[n1] = true;
	}

	for (i32s i = 0; i < missed_atoms_list.size(); i++)
	{
		if(missed_atoms_list[i] >= 0 && missed_atoms_list[i] < eng->GetAtomCount())
		{
printf("missed_atoms_list[%d] = %d\n", i, missed_atoms_list[i]);
			b_atoms[missed_atoms_list[i]] = false;
		}
	}

	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			if ( b_atoms[n1] || (!b_atoms[n1] && n2 != missed_dim) )
			{
				AddVar(& eng->crd[n1 * 3 + n2], & eng->d1[n1 * 3 + n2]);
printf("AddVar(& eng->crd[n1 * 3 + n2], & eng->d1[n1 * 3 + n2]); n1 = %d n2 = %d\n", n1, n2);
			}
		}
	}
}

geomopt_ex::~geomopt_ex(void)
{
}

f64 geomopt_ex::GetValue(void)
{
	eng->Compute(0);	// request energy
	return eng->energy;
}

f64 geomopt_ex::GetGradient(void)
{
	eng->Compute(1);	// request energy and gradient
	return eng->energy;
}

/*################################################################################################*/


// eof
