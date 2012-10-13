// GEOMOPT.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "geomopt.h"

/*################################################################################################*/

geomopt::geomopt(engine * p1, i32s p2, f64 p3, f64 p4) : conjugate_gradient(p2, p3, p4)
{
	eng = p1;
	
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
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
