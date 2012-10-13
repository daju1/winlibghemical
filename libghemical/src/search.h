// SEARCH.H : conformational search classes.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef SEARCH_H
#define SEARCH_H

#include "libconfig.h"

#define UPDATEFRQ 25
class random_search;		// DO WE NEED TO RANDOMIZE THE RANDOM NUMBER GENERATOR SOMEWAY???
class systematic_search;	// DO WE NEED TO RANDOMIZE THE RANDOM NUMBER GENERATOR SOMEWAY???
class monte_carlo_search;	// DO WE NEED TO RANDOMIZE THE RANDOM NUMBER GENERATOR SOMEWAY???

class transition_state_search;

/*################################################################################################*/

#include "engine.h"
#include "eng1_qm.h"
#include "conjgrad.h"
#include "intcrd.h"

/*################################################################################################*/

/// A random conformational search (intcrd based).

class random_search
{
	protected:
	
	model * mdl; i32s molnum;
	i32s in_crdset; i32s out_crdset;
	
	i32s cycles;
	i32s optsteps;
	
	intcrd * ic;
	
	engine * eng;
	geomopt * go;
	
	i32s counter1;
	i32s counter2;
	
	f64 min_energy;
	
	public:
	
	random_search(model *, i32s, i32s, i32s, i32s, i32s);
	~random_search(void);
	
	f64 GetMinEnergy(void) { return min_energy; }
	
	// the search is split in small steps to make it "look nice" in graphic environment.
	// you should just keep callig TakeStep() until it returns a negative value...
	
	i32s TakeStep(void);
	
	public:
	
	int last_step;
	float last_E;
};

/*################################################################################################*/

/// A systematic conformational search (intcrd based).

class systematic_search
{
	protected:

	model * mdl; i32s molnum;
	i32s in_crdset; i32s out_crdset;
	
	i32s divisions;
	i32s optsteps;
	
	intcrd * ic;
	
	engine * eng;
	geomopt * go;
	
	i32s nvar;
	i32s * counter1;
	i32s counter2;
	
	f64 min_energy;
	
	public:
	
	systematic_search(model *, i32s, i32s, i32s, i32s, i32s);
	~systematic_search(void);
	
	f64 GetMinEnergy(void) { return min_energy; }
	
	// the search is split in small steps to make it "look nice" in graphic environment.
	// you should just keep callig TakeStep() until it returns a negative value...
	
	i32s TakeStep(void);
};

/*################################################################################################*/

/**	A monte carlo search (intcrd based). both for conformational search and for collecting 
	a set of probable states for a molecule.
*/

class monte_carlo_search
{
	protected:

	model * mdl; i32s molnum;
	i32s in_crdset; i32s out_crdset;
	
	i32s n_init_steps;
	i32s n_simul_steps;
	i32s optsteps;
	
	intcrd * ic;
	
	engine * eng;
	geomopt * go;
	
	i32s counter1;
	i32s counter2;
	
	i32s nvar;
	f64 * curr_ic1;
	f64 * curr_ic2;
	f64 curr_energy;
	
	f64 min_energy;
	
	public:
	
	monte_carlo_search(model *, i32s, i32s, i32s, i32s, i32s, i32s);
	~monte_carlo_search(void);
	
	f64 GetMinEnergy(void) { return min_energy; }
	
	// the search is split in small steps to make it "look nice" in graphic environment.
	// you should just keep callig TakeStep() until it returns a negative value...
	
	i32s TakeStep(void);
};

/*################################################################################################*/
/*################################################################################################*/
/*################################################################################################*/

/**	A transition state search for QM methods.
	This is NOT a "gradient extremal" method, but works roughly the same way.
	Simple energy constraints are added to drive the reactants/products towards each other.
*/

class transition_state_search
{
	protected:
	
	model * mdl;
	setup1_qm * suqm;
	
	bool init_failed;
	
	f64 deltae;
	f64 * target[2];
	
	f64 energy1[2];		// energy with constraints
	f64 energy2[2];		// energy without constraints
	
	f64 fc[2];
	
	f64 t[2];		// target
	f64 last_de[2];
	
	f64 p[2];		// progress ; actually a "reaction coordinate" for sorting the structures.
	bool ready[2];
	
	public:
	
	vector<i32u> patoms;		// this is for calculating the final reaction coordinates.
	vector<bond *> rbonds;		// this is for graphics only...
	vector<bond *> pbonds;		// this is for graphics only...
	
	public:
	
	transition_state_search(model *, f64, f64);
	~transition_state_search(void);
	
	bool InitFailed(void) { return init_failed; }
	
	void Run(i32s *);
	void UpdateTargets(bool *);
	
	f64 GetE(i32s p1) { return energy2[p1]; }
	f64 GetP(i32s p1) { return p[p1]; }
	
	bool GetR(i32s p1) { return ready[p1]; }
	
	protected:
	
	void SetTarget(i32s, i32s);
};

/*################################################################################################*/

/**	This class finds stationary states by minimizing forces in a structure.
	Used as a final stage in the transition state search.
*/

class stationary_state_search : public conjugate_gradient
{
	protected:
	
	engine * eng;
	
	f64 * d1;
	
	public:
	
	stationary_state_search(engine *, i32s, f64, f64);
	~stationary_state_search(void);
	
	f64 GetValue(void);		// virtual
	
	// no GetGradient() function defined here; just use the default numerical one...
	// no GetGradient() function defined here; just use the default numerical one...
	// no GetGradient() function defined here; just use the default numerical one...
};

/*################################################################################################*/

#endif	// SEARCH_H

// eof
