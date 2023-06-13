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
class moldyn_tst;
class moldyn_langevin;

class temperature_meter;
class pressure_meter;

#if GRAVI_OSCILLATOR_WORKING 
class engine_pbc2;
#endif

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
	
	char filename[1024];


	public:
	
	moldyn_param(setup * su);	
	~moldyn_param(void) { }
};

struct molecule
{
	i32s ncarbons;
	std::list<i32s> natoms;

	molecule(i32s _ncarbons, i32s natom)
	{
		ncarbons = _ncarbons;
		natoms.push_back(natom);
	}
};

enum molgrouptype
{
	GAS,
	MEMBRANE_OR_GLOBULE
};

struct molgroup
{
	enum molgrouptype molgrouptype;
	std::list<i32s> natoms;
	f64 sum_p[3];
	int num_locked;

	molgroup(enum molgrouptype _molgrouptype);

	molgroup(enum molgrouptype _molgrouptype, std::list<i32s> _natoms);

	void ForceMoleculesMomentumToZero(moldyn * mld);
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
	
	f64 * vel;			// [1.0e+3 m/s]
	f64 * acc;			// [1.0e+12 m/s^2]

	f64 * cumsum_vel;
	f64 * cumsum_acc;
	f64 * cumsum_f;

	f64 * mass;			// [kg/mol]
	
	char * locked;
	int num_locked;
	
	f64 tstep1;		// timestep [fs]
	f64 tstep2;		// timestep ^ 2
	
	f64 ekin;
	f64 epot;
	
	i32s step_counter;
	
	friend void model::WriteTrajectoryFrame(long_ofstream &, moldyn_tst *);
	friend void model::WriteTrajectoryHeader(long_ofstream & ofile, int, moldyn_tst *, int);
	friend void molgroup::ForceMoleculesMomentumToZero(moldyn * );
	
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

};

/*################################################################################################*/

class moldyn_tst_param
{
	protected:
	
	bool confirm;
	
	friend class model;
	friend class moldyn_tst_dialog;
	friend class win_project; //temporary
	
	public:
	
	bool show_dialog;
	
	i32s nsteps_h;		// heat
	i32s nsteps_e;		// equilibration
	i32s nsteps_s;		// simulation
	f64 temperature;
	f64 maxwell_init_temperature;
	f64 timestep;
	bool constant_e;
	bool langevin;
	bool recalc_box, box_optimization;
	bool maxwell_distribution_init;


	double g[3];

	
	char filename_traj[1024];
	char filename_input_frame[1024];
	char filename_output_frame[1024];
	bool load_last_frame;
	bool extended_trajectory;

#if SNARJAD_TARGET_WORKING
	// начальная скорость снаряда
	f64 start_snarjad_vel[3];
#endif
#if SNARJAD_TARGET_WORKING || PROBNIY_ATOM_WORKING
	// список номеров атомов - целей
	vector<i32s> target_list;
#endif

#if GRAVI_OSCILLATOR_WORKING 	
	f64 len_n_len_g;
#if GRAVI_OSCILLATOR_WORKING_LINEAR
	f64 k_lin_gravi;
#endif
#endif
#if SOUND_GRAVI_OSCILLATOR
	f64 g0, m_T, m_g_gravi_dim;
	int gravi_dim;
#endif

	public:
	
	moldyn_tst_param(setup * su);	
	~moldyn_tst_param(void) { }
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

class moldyn_tst  : public moldyn
{
	friend class moldyn_atomlist_dialog;
	friend class model;
	protected:
	
	engine * eng;
	
	f64 * vel;
	f64 * acc;
	
	f64 * mass;
	
	char * locked;
	int num_locked;
	
	char * worked;
	int num_worked;
	
	char * gravi;
	int num_gravi;
	f64 m_g[3];
	
	f64 tstep1;	// timestep
	f64 tstep2;	// timestep ^ 2
	
	f64 ekin;
	f64 epot;
	
	i32s step_counter;
	
	public:
	
	f64 temperature;
	
	f64 temperature_coupling;

	f64 psi, relax_rate_2;

	void MaxwellDistribution(f64 Temp);

	
	public:
	
	moldyn_tst(engine *, f64);
	virtual ~moldyn_tst(void);
	
	f64 GetEKin(void) { return ekin; }
	f64 GetEPot(void) { return epot; }
	
	virtual void TakeMDStep(bool);
	
	void SumModelImpuls(f64 * sum_p);
	
	f64 KineticEnergy(void);
	
	f64 ConvTempEKin(f64);
	f64 ConvEKinTemp(f64);
	
	void SetEKin(f64, bool with_constant_impuls, bool and_with_zero_impuls = false);

	void SetGraviG(f64 *p);
	f64 GetGravi(i32s);
	
	void LockAtoms(vector<i32s>& nAtoms);
	void LockAtoms(char * fn);
	void ForceMoleculesMomentumToZero();
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
#if GRAVI_OSCILLATOR_WORKING 
// режим работы с наложением броуновского осциллятора 
// в виде гравитационной осцилляции
	i32s start_gravi_step_counter;
	bool to_start_gravi;
	double v_pract, v_theor, v0; // m/s
	double g0;//амплитуда гравитационногоосциллятора [m/s^2]
	double omega;//круговая частота гравитационного осциллятора [s^-1]
	double len_g;// длина гравитационного осциллятора, нм
	double len_n;// длина нанотрубки вычисляется как удвоенное значение полудлины периодич. ячейки для оси z, нм
	double len_n_len_g;// отношение длины нанотрубки к длине осциллятора, меньше или равно 1.0, рекоменд. 0.1
	double gamma; //показатель адиабаты газа в осцилляторе, , для одноатомного газа равен 5.0/3.0
	double N_mol; // число молекул газа в центральной части осциллятора
	double M; // молярная масса газа в осцилляторе,кг/моль
	//функция инициализации гравитационного осциллятора, вызывается перед началом работы
	void InitGraviOscillator(moldyn_param& , engine_pbc2 * );
	// на каждом шаге вычисляется значение амплитуды g в центре осцилляторе
	void TakeGraviStep();
	double GetTheorV();
	double GetPractV();
#if GRAVI_OSCILLATOR_WORKING_LINEAR
	double k_lin_gravi;
#endif

#endif
#if SOUND_GRAVI_OSCILLATOR
	void InitGraviOscillator(moldyn_param&);
	void TakeGraviStep();
	f64 g0, m_T, omega, m_g_gravi_dim;
	int gravi_dim;
	i32s start_gravi_step_counter;
	bool to_start_gravi;
#endif
#if WRITE_LOCKED_FORCES
	void WriteLockedForcesHeader();

	char locked_forces_fn[1024];
#endif
#if WRITE_WORKED_FORCES
	void WriteWorkedForcesHeader();

	char worked_forces_fn[1024];
#endif
	void SaveLastFrame(char * fn);
	void SaveLastFrame(ofstream& ofile);
	void SaveTrajectoryFrame(long_ofstream& ofile, i32s trajectory_version);
	void SaveLastFrameTxt(char * fn);
	void ReadLastFrame(char * fn);
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
