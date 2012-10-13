// SPLINE.H : a "B-spline" class for ribbon models.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef SPLINE_H
#define SPLINE_H

#include "appconfig.h"

class spline;

/*################################################################################################*/

#include "../libghemical/src/typedef.h"

/*################################################################################################*/

/// This is a "##B-spline" class for those ribbon models...

class spline
{
	private:
	
	i32s order;
	i32s ncp; i32s nk;
	
	fGL * knot;
	fGL * weight;
	fGL ** cpcrd;		// no data, just the pointers...
	
	public:
	
	spline(i32s p1, i32s p2);
	~spline(void);
	
	void Compute(fGL, fGL *);
	
	void SetPoint(i32s p1, fGL * p2) { cpcrd[p1] = p2; }
	void SetKnot(i32s p1, fGL p2) { knot[p1] = p2; }
	
	i32s GetOrder(void) { return order; }
};

/*################################################################################################*/

#endif	// SPLINE_H

// eof
