// CONJGRAD.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "conjgrad.h"

#include <math.h>

/*################################################################################################*/

conjugate_gradient::conjugate_gradient(i32s p1, f64 p2, f64 p3)
{
	step = 0; reset = p1;
	defstp = p2; maxstp = p3;
	ngdelta = 0.0;
}

conjugate_gradient::~conjugate_gradient(void)
{
}

void conjugate_gradient::AddVar(f64 * p1, f64 * p2)
{
	cgvar newvar; newvar.ref1 = p1; newvar.ref2 = p2;
	cgvar_vector.push_back(newvar);
}

void conjugate_gradient::TakeCGStep(ls_mode mode)
{
	optstp = 0.0;
	optval = GetGradient();
	
	// compute inner product of the latest gradient to newip.
	
	newip = 0.0;
	for (i32u n1 = 0;n1 < cgvar_vector.size();n1++)
	{
		f64 tmp1 = (* cgvar_vector[n1].ref2);
		newip += tmp1 * tmp1;
	}
	
	// either reset beta or update it, using the Fletcher-Reeves method.
	
	if (!(step++ % reset) || oldip == 0.0)
	{
		beta = 0.0;
		for (i32u n1 = 0;n1 < cgvar_vector.size();n1++)
		{
			cgvar_vector[n1].data1 = 0.0;
		}
	}
	else
	{
		beta = newip / oldip;
	}
	
	oldip = newip;
	
	// store the current search direction and the current value for a while...
	
	f64 sum = 0.0;
	{
	for (i32u n1 = 0;n1 < cgvar_vector.size();n1++)
	{
		cgvar_vector[n1].data2 = (* cgvar_vector[n1].ref1);
		f64 tmp1 = cgvar_vector[n1].data1 * beta - (* cgvar_vector[n1].ref2);
		cgvar_vector[n1].data1 = tmp1; sum += tmp1 * tmp1;
	}
	}
	
	f64 scale = sqrt(sum);
	if (scale < +1.0e-70)		// for float -35 and for double -70 ?!?!?!
	{
		cout << "WARNING: too small \"scale\" at conjugate_gradient::TakeCGStep()" << endl;
		scale = +1.0e-70;	// try to avoid "division by zero" conditions...
	}
	
	f64 stpln = defstp / scale;
	f64 maxscl = maxstp / scale;
	
	// do the line search...
	
	if (mode == Const)
	{
		InitLineSearch(stpln);
		f64 tmp1 = GetValue();
		
		optstp = stpln;
		optval = tmp1;
	}
	
	if (mode == Simple)
	{
		i32s succ_cycle_count = 0;
		
		while (true)
		{
			InitLineSearch(stpln);
			f64 tmp1 = GetValue();
			
			// multipliers 2.15 and 0.10 were chosen so that the loop will
			// always generate different values: (1/0.1)/2.15 = 4.651162791...
			
			if (tmp1 < optval)
			{
				optstp = stpln;		// store...
				optval = tmp1;		// store...
				
				stpln *= 2.15;
				if (stpln > maxscl)
				{
					cout << "WARNING : conjugate_gradient::TakeCGStep() damped steplength " << stpln << " to " << maxscl << endl;
					stpln = maxscl;
				}
				
				succ_cycle_count++;
				if (succ_cycle_count >= 10) break;
			}
			else
			{
				stpln *= 0.10;
				
				// require that we have at least one successfull cycle.
				// theoretically, it should happen BUT we have a chance of an eternal loop here...
				
				if (succ_cycle_count >= 1) break;
			}
		}
	}
	
	if (mode == Newton2An)
	{
		i32s newton = 0;
		
		while (true)
		{
			InitLineSearch(stpln);
			f64 tmp1 = GetGradient();
			
			if (tmp1 < optval)
			{
				optstp = stpln;
				optval = tmp1;
			}
			
			if (newton++ > 3) break;
			f64 delta = stpln * 0.001;
			
			f64 tmp2 = 0.0; f64 tmp3 = 0.0;
			for (i32u n1 = 0;n1 < cgvar_vector.size();n1++)
			{
				tmp2 += cgvar_vector[n1].data1 * (* cgvar_vector[n1].ref2) / scale;
			}
			
			InitLineSearch(stpln + delta);
			GetGradient();
			{
			
			for (i32u n1 = 0;n1 < cgvar_vector.size();n1++)
			{
				tmp3 += cgvar_vector[n1].data1 * (* cgvar_vector[n1].ref2) / scale;
			}
			}
			
			f64 denom = tmp3 - tmp2;
			if (denom != 0.0)
			{
				stpln = fabs(stpln - delta * tmp2 / denom);
				if (stpln > maxscl)
				{
					cout << "WARNING : conjugate_gradient::TakeCGStep() damped steplength " << stpln << " to " << maxscl << endl;
					stpln = maxscl;
				}
			}
			else break;
		}
		
		if (optstp == 0.0)	// if we still don't have any valid stplen, just try a very small step...
		{
			stpln = 0.001 * defstp / scale;
			
			InitLineSearch(stpln);
			f64 tmp1 = GetValue();
			
			if (tmp1 < optval)
			{
				optstp = stpln;
				optval = tmp1;
			}
		}
	}
	
	if (mode == Newton2Num)
	{
		i32s newton = 0;
		
		while (true)
		{
			InitLineSearch(stpln);
			f64 tmp1 = GetValue();
			
			if (tmp1 < optval)
			{
				optstp = stpln;
				optval = tmp1;
			}
			
			if (newton++ > 3) break;
			f64 delta = stpln * 0.001;
			
			InitLineSearch(stpln + delta);
			f64 tmp2 = GetValue();
			
			InitLineSearch(stpln + delta * 2.0);
			f64 tmp3 = GetValue();
			
			f64 denom = tmp3 - 2.0 * tmp2 + tmp1;
			if (denom != 0.0)
			{
				stpln = fabs(stpln - delta * (tmp2 - tmp1) / denom);
				if (stpln > maxscl)
				{
					cout << "WARNING : conjugate_gradient::TakeCGStep() damped steplength " << stpln << " to " << maxscl << endl;
					stpln = maxscl;
				}
			}
			else break;
		}
		
		if (optstp == 0.0)	// if we still don't have any valid stplen, just try a very small step...
		{
			stpln = 0.001 * defstp / scale;
			
			InitLineSearch(stpln);
			f64 tmp1 = GetValue();
			
			if (tmp1 < optval)
			{
				optstp = stpln;
				optval = tmp1;
			}
		}
	}
		
	if (mode == Newton1Num)
	{
		i32s newton = 0;
		
		while (true)
		{
			InitLineSearch(stpln);
			f64 tmp1 = GetValue();
			
			if (fabs(tmp1) < fabs(optval))
			{
				optstp = stpln;
				optval = tmp1;
			}
			
			if (newton++ > 3) break;
			f64 delta = stpln * 0.001;
			
			InitLineSearch(stpln + delta);
			f64 tmp2 = GetValue();
			
			f64 denom = tmp2 - tmp1;
			if (denom != 0.0)
			{
				stpln = fabs(stpln - delta * tmp1 / denom);
				if (stpln > maxscl)
				{
					cout << "WARNING : conjugate_gradient::TakeCGStep() damped steplength " << stpln << " to " << maxscl << endl;
					stpln = maxscl;
				}
			}
			else break;
		}
		
		if (optstp == 0.0)	// if we still don't have any valid stplen, just try a very small step...
		{
			stpln = 0.001 * defstp / scale;
			
			InitLineSearch(stpln);
			f64 tmp1 = GetValue();
			
			if (tmp1 < optval)
			{
				optstp = stpln;
				optval = tmp1;
			}
		}
	}
	
	// finally take the optimal step and scale back.
	
	InitLineSearch(optstp);
	optstp *= scale;
}

void conjugate_gradient::InitLineSearch(f64 p1)
{
	for (i32u n1 = 0;n1 < cgvar_vector.size();n1++)
	{
		f64 tmp1 = cgvar_vector[n1].data2 + cgvar_vector[n1].data1 * p1;
		(* cgvar_vector[n1].ref1) = tmp1;
	}
}

f64 conjugate_gradient::GetGradient(void)
{
	f64 value = GetValue();
	for (i32u n1 = 0;n1 < cgvar_vector.size();n1++)
	{
		f64 old = (* cgvar_vector[n1].ref1);
		(* cgvar_vector[n1].ref1) = old + ngdelta;
		(* cgvar_vector[n1].ref2) = (GetValue() - value) / ngdelta;
		(* cgvar_vector[n1].ref1) = old;
	}
	
	return value;
}

/*################################################################################################*/

// eof
