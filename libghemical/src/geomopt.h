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
	i32s treshold_delta_e_min_nsteps;
	bool recalc_box, box_optimization;

	enum box_optimization_type
	{
		no = 0,
		xyz,
		xy,
		z
	};

	bool Write(char * filename);

	box_optimization_type box_opt;

	bool don_t_move_fixed_atoms;
	
	public:
	
	geomopt_param(setup * su);

	
	~geomopt_param(void) { }
};

/*################################################################################################*/

/// A geometry optimization class.

class geomopt : public conjugate_gradient
{
	protected:
	
	engine * eng;
	
	public:
	
	geomopt(engine *, i32s, f64, f64, bool don_t_move_fixed_atoms = true);
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
	// а в этом варианте у пробного атома фиксируются все три координаты
	geomopt_ex(i32s /*n_prob_atom*/, vector<i32s> & /*missed_atoms_list*/, i32s /*missed_dim*/, engine *, i32s, f64, f64);
	geomopt_ex(vector<i32s> & /*missed_atoms_list*/, i32s /*missed_dim*/, engine *, i32s, f64, f64);
	~geomopt_ex(void);
	
	f64 GetValue(void);		// virtual
	f64 GetGradient(void);		// virtual
};

/*################################################################################################*/
#endif	// GEOMOPT_H

// eof
