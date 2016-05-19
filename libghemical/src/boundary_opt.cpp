// BOUNDARY_OPT.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "boundary_opt.h"

/*################################################################################################*/

boundary_opt::boundary_opt(geomopt_param::box_optimization_type box_opt, 
						   model* m, engine * p1, i32s p2, f64 p3, f64 p4) : conjugate_gradient(p2, p3, p4)
{
	eng = p1;

	mdl = m;

	SetNGDelta(1.0e-4);

	d1 = NULL;

	switch(box_opt)
	{
	case geomopt_param::box_optimization_type::xyz:
		{
			MessageBox(0, "xyz","b_opt",0);
			d1 = new f64[3];
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				AddVar(& mdl->periodic_box_HALFdim[n2], & d1[n2]);
			}
		}
		break;
	case geomopt_param::box_optimization_type::xy:
		{
			MessageBox(0, "xy","b_opt",0);
			d1 = new f64[2];
			for (i32s n2 = 0;n2 < 2;n2++)
			{
				AddVar(& mdl->periodic_box_HALFdim[n2], & d1[n2]);
			}
		}
		break;
	case geomopt_param::box_optimization_type::z:
		{
			MessageBox(0, "z","b_opt",0);
			d1 = new f64[1];	
			i32s n2 = 2;
			{
				AddVar(& mdl->periodic_box_HALFdim[n2], & d1[0]);
			}
		}
		break;
	}
}

boundary_opt::~boundary_opt(void)
{
	if (d1) delete[] d1;
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
