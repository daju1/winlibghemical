// CONJGRAD.H : a general-purpose conjugate-gradient optimizer class.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef CONJGRAD_H
#define CONJGRAD_H

#include "libconfig.h"

class conjugate_gradient;

/*################################################################################################*/

#include "libdefine.h"
#include "typedef.h"

#include <vector>
using namespace std;

/*################################################################################################*/

/// Is used to hold some data about the variables.

struct cgvar
{
	f64 * ref1;	// ref to the variable
	f64 * ref2;	// ref to the derivative
	
	f64 data1;
	f64 data2;
};

/*################################################################################################*/

/// Is a generic Conjugate Gradient optimizer class.
/** This is supposed to be a common base class, and you should implement your own derived classes 
which then define the function to be optimized by implementing the GetValue() function. */

class conjugate_gradient
{
	public:
	
/// Available line-search modes.
/** The first-derivative methods look for zero, and the 2nd-derivative methods look for a local minimum. */
	enum ls_mode
	{
		Const = 0,		///< Always takes the default step; for debugging purposes only!
		
		Simple = 1,		///< A simple but VERY INEFFICIENT method.
		
		Newton2An = 2,		///< Second-derivative Newton's method, half numerical.
		
		Newton2Num = 3,		///< Second-derivative Newton's method, full numerical.
		
		Newton1Num = 4		///< First-derivative Newton's method, full numerical.
	};
	
	protected:
	
	vector<cgvar> cgvar_vector;
	f64 newip; f64 oldip; f64 beta;
	
	public:		// why public?!?!?!
	
	i32s step; i32s reset;
	f64 defstp; f64 maxstp;
	f64 ngdelta;
	
	f64 optstp; f64 optval;
	
	public:
	
	conjugate_gradient(i32s, f64, f64);
	virtual ~conjugate_gradient(void);
	
	void SetNGDelta(f64 p1) { ngdelta = p1; }
	void AddVar(f64 *, f64 *);
	
/// Will perform a single Conjugate Gradient step, using the line search mode passed as a parameter.
/** Haataja J, Käpyaho J, Rahola J : "##Numeeriset menetelmät", CSC-Tieteellinen laskenta OY, 1993
*/
	void TakeCGStep(ls_mode);
	void InitLineSearch(f64);
	
/// Must compute the value of the function to be optimized, and return that value.
	virtual f64 GetValue(void) = 0;
	
/// Must compute the 1st derivative value of the function to be optimized.
/** Must compute the first derivatives of the funcion to be optimized, and in 
addition to that do the same that GetValue() does (that is, to compute the 
value of the function to be optimized and return that value). A simple numerical 
derivatives are computed by default, but you are encouraged to implement analytical 
derivatives in your own derived classes; that will improve performance remarkably! */
	virtual f64 GetGradient(void);
};

/*################################################################################################*/

#endif	// CONJGRAD_H

// eof
