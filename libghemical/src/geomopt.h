// GEOMOPT.H : geometry optimization classes.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef GEOMOPT_H
#define GEOMOPT_H

#include "libconfig.h"

class geomopt_param;

class geomopt;

/*################################################################################################*/

#include "engine.h"
#include "eng1_qm.h"

#include "conjgrad.h"

/*################################################################################################*/

class geomopt_param
{
	protected:
	
	bool confirm;
	
	friend class model;
	friend class geomopt_dialog;
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
	
	geomopt_param(setup * su)
	{
		confirm = false;
		show_dialog = true;

		enable_nsteps	= true;
		enable_grad		= true;
		enable_delta_e	= true;
		
		treshold_nsteps = 2500;
		treshold_grad = 1.0e-3;
		treshold_delta_e = 1.0e-7;
		
		setup1_qm * suqm = dynamic_cast<setup1_qm *>(su);
		if (suqm != NULL) treshold_nsteps = 500;		// override...

		// my correct
		treshold_nsteps = 500;
		//treshold_nsteps = 10;
	}
	
	~geomopt_param(void) { }
};

/*################################################################################################*/

/// A geometry optimization class.

class geomopt : public conjugate_gradient
{
	protected:
	
	engine * eng;
	
	public:
	
	geomopt(engine *, i32s, f64, f64);
	~geomopt(void);
	
	f64 GetValue(void);		// virtual
	f64 GetGradient(void);		// virtual
};

/*################################################################################################*/

// New (Extended) geometry optimization class.
// расширенный класс оптимизации геометрии позволяющий 
// при оптимизации фиксировать например, z координату 
// определённых атомов - путём невключения этих координат 
// в процедуру оптимизации. Этот класс может быть использован, 
// например, для решения задачи поиска потенциального барьера 
// проникновения пробного атома через мембрану. У пробного атома 
// и у нескольких атомов мембраны фиксируется z координата, 
// производится оптимизация геометрии с расчётом потенциальной 
// энергии системы, затем в цикле изменяется z координата 
// пробного атома, и производится перерасчёт оптимизации 
// геометрии и потенциальной энергии. В итоге строится 
// зависимомть потенциальной энергии системы 
// от z координаты пробного атома

class geomopt_ex : public conjugate_gradient
{
	protected:
	
	engine * eng;
	
	public:
	
	geomopt_ex(vector<i32s> & /*missed_atoms_list*/, i32s /*missed_dim*/, engine *, i32s, f64, f64);
	~geomopt_ex(void);
	
	f64 GetValue(void);		// virtual
	f64 GetGradient(void);		// virtual
};

/*################################################################################################*/
#endif	// GEOMOPT_H

// eof
