// BOUNDARY_OPT.H : geometry optimization classes.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef BOUNDARY_OPT_H
#define BOUNDARY_OPT_H

#include "libconfig.h"

class boundary_opt_param;

class boundary_opt;

/*################################################################################################*/

#include "engine.h"
#include "eng1_qm.h"

#include "conjgrad.h"

#include "geomopt.h"

/*################################################################################################*/

class boundary_opt_param
{
	protected:
	
	bool confirm;
	
	friend class model;
	friend class boundary_opt_dialog;
	friend class win_project; //temporary
	
	public:
	
	bool show_dialog;
	
	bool enable_nsteps;
	i32s treshold_nsteps;
	
	bool enable_grad;
	f64  treshold_grad;
	
	bool enable_delta_e;
	f64  treshold_delta_e;
	
	public:
	
	boundary_opt_param(setup * su)
	{
		confirm = false;
		show_dialog = true;
		
		treshold_nsteps = 2500;
		treshold_grad = 1.0e-3;
		treshold_delta_e = 1.0e-7;
		
		setup1_qm * suqm = dynamic_cast<setup1_qm *>(su);
		if (suqm != NULL) treshold_nsteps = 500;		// override...

		// my correct
		treshold_nsteps = 100;
	}
	
	~boundary_opt_param(void) { }
};

/*################################################################################################*/

/// A geometry optimization class.

class boundary_opt : public conjugate_gradient
{
	protected:
	
	engine * eng;
	model * mdl;

	f64 * d1;
	
	public:
	
	boundary_opt(geomopt_param::box_optimization_type box_opt, model* , engine *, i32s, f64, f64);
	~boundary_opt(void);
	
	f64 GetValue(void);		// virtual
	//f64 GetGradient(void);		// virtual
};

/*################################################################################################*/

#endif	// GEOMOPT_H

// eof
