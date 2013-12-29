// SPLINE.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "spline.h"

/*################################################################################################*/

spline::spline(i32s p1, i32s p2)
{
	order = p1; ncp = p2; nk = order + ncp;
	knot = new fGL[nk]; weight = new fGL[ncp + 1];
	cpcrd = new ref_fGL[ncp];
}

spline::~spline(void)
{
	delete[] weight;
	delete[] cpcrd;
	delete[] knot;
}

void spline::Compute(fGL p1, fGL * p2)
{
	p2[0] = p2[1] = p2[2] = 0.0;
	for (i32s n1 = 0;n1 < ncp + 1;n1++)
	{
		if (p1 < knot[n1]) weight[n1] = 0.0;
		else if (p1 < knot[n1 + 1]) weight[n1] = 1.0;
		else weight[n1] = 0.0;
	}
	
	for (i32s n1 = 1;n1 < order;n1++)
	{
		for (i32s n2 = 0;n2 < ncp;n2++)
		{
			fGL tmp1 = (p1 - knot[n2]) / (knot[n2 + n1] - knot[n2]) * weight[n2];
			fGL tmp2 = (knot[n2 + n1 + 1] - p1) / (knot[n2 + n1 + 1] - knot[n2 + 1]) * weight[n2 + 1];
			weight[n2] = tmp1 + tmp2;
		}
	}
	
	for (i32s n1 = 0;n1 < ncp;n1++)
	{
		p2[0] += cpcrd[n1][0] * weight[n1];
		p2[1] += cpcrd[n1][1] * weight[n1];
		p2[2] += cpcrd[n1][2] * weight[n1];
	}
}

/*################################################################################################*/

// eof
