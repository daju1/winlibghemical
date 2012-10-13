// BOUNDARY_OPT.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "boundary_opt.h"

/*################################################################################################*/

boundary_opt::boundary_opt(model* m, engine * p1, i32s p2, f64 p3, f64 p4) : conjugate_gradient(p2, p3, p4)
{
	eng = p1;

	mdl = m;

	SetNGDelta(1.0e-4);
	
	d1 = new f64[3];	

	for (i32s n2 = 0;n2 < 3;n2++)
	{
		AddVar(& mdl->periodic_box_HALFdim[n2], & d1[n2]);
	}
}

boundary_opt::~boundary_opt(void)
{
	delete[] d1;
}

f64 boundary_opt::GetValue(void)
{
	eng->Compute(0);	// request energy
	return eng->energy;
}

/*
f64 boundary_opt::GetGradient(void)
{
	eng->Compute(1);	// request energy and gradient
	return eng->energy;
}
*/
/*################################################################################################*/

// eof
