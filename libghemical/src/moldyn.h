// MOLDYN.H : molecular dynamics classes.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef MOLDYN_H
#define MOLDYN_H

#include "libconfig.h"

class moldyn_param;

class moldyn;
class moldyn_langevin;

class temperature_meter;
class pressure_meter;

/*################################################################################################*/

#include "engine.h"
#include "eng1_sf.h"

/*################################################################################################*/

class moldyn_param
{
	protected:
	
	bool confirm;
	
	friend class model;
	friend class moldyn_dialog;
	friend class win_project; //temporary
	
	public:
	
	bool show_dialog;
	
	i32s nsteps_h;		// heat
	i32s nsteps_e;		// equilibration
	i32s nsteps_s;		// simulation
	f64 temperature;
	f64 timestep;
	bool constant_e;
	bool langevin;


	double g[3];

	
	char filename[1024];

#if SNARJAD_TARGET_WORKING
	// начальная скорость снаряда
	f64 start_snarjad_vel[3];
#endif
#if SNARJAD_TARGET_WORKING || PROBNIY_ATOM_WORKING
	// список номеров атомов - целей
	vector<i32s> target_list;
#endif
	public:
	
	moldyn_param(setup * su);	
	~moldyn_param(void) { }
};

/*################################################################################################*/

/**	A molecular dynamics class.
	
	This is a "velocity-Verlet"-type integrator...
	Allen MP, Tildesley DJ : "##Computer Simulation of Liquids", Clarendon Press, 1987
	
	So far very long simulations have not been tested, so possile translation/rotation 
	problems are not solved... current solution is to start simulations from 0 K -> 
	no translation/rotation in the initial state -> conservation of linear and angular 
	momentum -> no need to worry at all... but for how long it will work???
*/

class moldyn
{
	friend class moldyn_atomlist_dialog;
	protected:
	
	engine * eng;
	
	f64 * vel;
	f64 * acc;
	
	f64 * mass;
	
	char * locked;
	int num_locked;
	
	char * gravi;
	int num_gravi;
	double m_g[3];
	
	f64 tstep1;	// timestep
	f64 tstep2;	// timestep ^ 2
	
	f64 ekin;
	f64 epot;
	
	i32s step_counter;
	
	public:
	
	f64 temperature;
	
	f64 temperature_coupling;
	
	public:
	
	moldyn(engine *, f64);
	virtual ~moldyn(void);
	
	f64 GetEKin(void) { return ekin; }
	f64 GetEPot(void) { return epot; }
	
	virtual void TakeMDStep(bool);
	
	f64 KineticEnergy(void);
	
	f64 ConvTempEKin(f64);
	f64 ConvEKinTemp(f64);
	
	void SetEKin(f64);

	void SetGraviG(double *p);

#if SNARJAD_TARGET_WORKING
	// my functions:
	i32s n_snarjad;
	void ShootSnarjad(i32s nAtom, f64* v, f64* c = 0);
	bool GetSnarjadVelCrd(f64* v, f64* c, f64* f);
	//
	//char * target;
#endif
#if SNARJAD_TARGET_WORKING || PROBNIY_ATOM_WORKING
	void LockAtom(i32s nAtom);
	int num_target;
	vector<i32s> m_target_list;
	vector<vector<double> > target_distanse_matrix;
	f64 target_crd[3]; // координата центра отверстия в мембране
	void SetTarget();
	void SetTarget(vector<i32s>& target_list);
	void ComputeTargetCrd();
#endif
#if PROBNIY_ATOM_WORKING
	i32s n_prob_atom;
	void SetProbAtom(i32s nAtom, f64* c);
	bool GetProbAtomForce(f64* f);
#endif

};

/*################################################################################################*/

/**	A molecular dynamics class for Langevin dynamics.
	
	This is not any of the well-known LD integrators, just LD stuff added
	on top of a "velocity Verlet" MD integrator.
	
	TODO : convert this is into a "BBK"-type integrator???
*/

class moldyn_langevin : public moldyn
{
	protected:
	
	eng1_sf * engsf;
	
	public:
	
	f64 * langevin_r_forces;
	
	f64 langevin_frict_fc;
	f64 langevin_random_fc;
	
	f64 temperature_coupling;
	f64 langevin_coupling;
	
	f64 * langevin_weight;
	f64 * langevin_frict;
	
	public:
	
	moldyn_langevin(engine *, f64);
	virtual ~moldyn_langevin(void);
	
	void TakeMDStep(bool);		// virtual...
};

/*################################################################################################*/

/**	Estimates pressure in a cubic volume element.
*/

class temperature_meter
{
};

/**	Estimates pressure in a cubic volume element.
*/

class pressure_meter
{
};

/*################################################################################################*/

#endif	// MOLDYN_H

// eof
