// ENGINE.H : the "engine" base class for computation engine classes.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef ENGINE_H
#define ENGINE_H

#include "libconfig.h"

class setup;
class engine;

class engine_mbp;	// modified boundary potential
class engine_pbc;	// periodic boundary conditions

class number_density_evaluator;
class radial_density_function_evaluator;

/*################################################################################################*/

class atom;	// atom.h
class bond;	// bond.h

class model;	// model.h

#include "typedef.h"

#include <stdlib.h>

#include <vector>
using namespace std;

/*################################################################################################*/

/// The setup class is used to define the submodel boundaries in the model.
/** There can be only a single setup object for each model object. 
The setup object will create the engine objects. */

class setup
{
	private:
	
	model * mdl;
	
	protected:
	
	engine * current_eng;
	i32s current_eng_index;
	
	bool has_setup_tables;
	
	// the global tables.
	// ^^^^^^^^^^^^^^^^^^

	atom ** atmtab; i32s natm;
	
	// the local tables.
	// ^^^^^^^^^^^^^^^^^
	
	atom ** qm_atmtab; i32s qm_natm;
	bond ** qm_bndtab; i32s qm_nbnd;
	
	atom ** mm_atmtab; i32s mm_natm;
	bond ** mm_bndtab; i32s mm_nbnd;
	
	bond ** boundary_bndtab; i32s boundary_nbnd;
	
	atom ** sf_atmtab; i32s sf_natm;
	
	friend class model;				// needs to access current_eng_index.
	friend class transition_state_search;		// needs to access current_eng.
	
	friend class project;		// FOR GHEMICAL!!!
	friend class setup_druid;	// FOR GHEMICAL!!!	// needs to access current_eng_index.	// for BBB
	friend class setup_dialog;	// FOR GHEMICAL!!!	// needs to access current_eng_index.	// for HEAD
	
	public:
	
	setup(model *);
	virtual ~setup(void);
	
	model * GetModel(void) { return mdl; }
	
	bool HasSetupTables(void) { return has_setup_tables; }
	
	// access to global tables.
	// ^^^^^^^^^^^^^^^^^^^^^^^^
	
	atom ** GetAtoms(void) { return atmtab; }
	i32s GetAtomCount(void) { return natm; }
	
	// access to local tables.
	// ^^^^^^^^^^^^^^^^^^^^^^^
	
	atom ** GetQMAtoms(void) { return qm_atmtab; }
	i32s GetQMAtomCount(void) { return qm_natm; }
	bond ** GetQMBonds(void) { return qm_bndtab; }
	i32s GetQMBondCount(void) { return qm_nbnd; }

	atom ** GetMMAtoms(void) { return mm_atmtab; }
	i32s GetMMAtomCount(void) { return mm_natm; }
	bond ** GetMMBonds(void) { return mm_bndtab; }
	i32s GetMMBondCount(void) { return mm_nbnd; }
	
	bond ** GetBoundaryBonds(void) { return boundary_bndtab; }
	i32s GetBoundaryBondCount(void) { return boundary_nbnd; }
	
	atom ** GetSFAtoms(void) { return sf_atmtab; }
	i32s GetSFAtomCount(void) { return sf_natm; }
	
	engine * GetCurrentEngine(void) { return current_eng; }
	i32s GetCurrEngIndex(void) { return current_eng_index; }
	
	virtual void UpdateAtomFlags(void) = 0;
	
	void DiscardSetupInfo(void);
	void UpdateSetupInfo(void);
	
	// functions for obtaining information about available eng objects.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	virtual i32u GetEngineCount(void) = 0;
	virtual i32u GetEngineIDNumber(i32u) = 0;		// this is not much used yet. for saving eng info in files?!?!?!
	virtual const char * GetEngineName(i32u) = 0;
	virtual const char * GetClassName(void) = 0;
	
	void SetCurrentEngIndex(i32s index) { current_eng_index = index; }	// only file operations etc should use this...
	
	void CreateCurrentEngine(void);				///< Create the current_eng object for plotting...
	void DiscardCurrentEngine(void);
	
	virtual engine * CreateEngineByIndex(i32u) = 0;		///< Create an engine object.
	engine * CreateEngineByIDNumber(i32u);
};

/*################################################################################################*/

/// A virtual base class for computations.
/** The engine classes are used to implement the computational details of various models 
(like different quantum-mechanical models and molecular mechanics models).

The engine class will store it's own atom coordinates.

When we want to compute something, we create an instance of some suitable engine-class using 
our setup-class. The engine-class will copy that information it needs, and calculates those results 
it is supposed to calculate. If we calculate some useful results that change our original system, 
we can copy those results back to our model-class.

The setup class will create two kinds of atom and bond tables using contents of the model object; 
one (global) contains all atoms/bonds, and the others (local ones) contain only those atoms/bonds 
that belong to a submodel (say, QM or MM submodel).

Since there is, for example, only one table for coordinates and derivatives at the engine base class, 
the different derived engine classes must create a suitable lookup table that maps the local tables 
back to the global one. */

class engine
{
	private:
	
	setup * stp;
	
	protected:

	i32s natm;
	
	f64 * crd;

	f64 energy;	// GetEnergy() ???
	
// add atom::ecomp_grp_ind ; define/register the new groups in the model object (and save them in files).
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ecomp_grp		vector<const char *>	<- in model!!!
// ecomp_ngrp		int			<- in engine!!!
// ecomp_data		double *
// ecomp_map		int *			<- make the size (n*n).
	f64 E_solute;
	f64 E_solvent;
	f64 E_solusolv;
	
	f64 * d1;	// GetD1() ???
	f64 * d2;	// GetD2() ???
	
	friend class model;
	friend class project;
	
	friend class boundary_opt;			// NOT NECESSARY
	friend class geomopt;				// NOT NECESSARY
	friend class geomopt_ex;				// NOT NECESSARY
	friend class moldyn;				// NOT NECESSARY
	friend class moldyn_langevin;		// NOT NECESSARY
	friend class moldyn_atomlist_dialog;
	
	friend class random_search;			// NOT NECESSARY
	friend class systematic_search;			// NOT NECESSARY
	friend class monte_carlo_search;		// NOT NECESSARY
	friend class transition_state_search;		// needs to access crd.
	friend class stationary_state_search;		// needs to access d1.
	
	public:
	
	friend void CopyCRD(model *, engine *, i32u);
	friend void CopyCRD(engine *, model *, i32u);
	
	public:
	
	engine(setup *, i32u);
	virtual ~engine(void);
	
	setup * GetSetup(void) { return stp; }
	i32s GetAtomCount(void) { return natm; }
	
	void Check(i32u);			///< This is for debugging; will compare the computed gradient to a numerical one.
	f64 GetGradientVectorLength(void);	///< This calculates the gradient vector length (using the d1 array).
	
	virtual void Compute(i32u) = 0;		///< Will compute the energy, and the gradient if needed.
	
	virtual void DoSHAKE(void);		///< This is an optional method that contains some SHAKE-like corrections to atomic coordinates ; is called before calculating forces in MD.
	
	// these are added to make it easier to set torsional constraints (like for plotting etc).
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	virtual i32s FindTorsion(atom *, atom *, atom *, atom *) = 0;
	virtual bool SetTorsionConstraint(i32s, f64, f64, bool) = 0;
	
	// these are mainly for drawing energy level diagrams...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	virtual i32s GetOrbitalCount(void) = 0;
	virtual f64 GetOrbitalEnergy(i32s) = 0;
	
	virtual i32s GetElectronCount(void) = 0;
	
	// these are for plotting purposes...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	virtual void SetupPlotting(void) = 0;
	
	virtual fGL GetVDWSurf(fGL *, fGL *) = 0;
	
	virtual fGL GetESP(fGL *, fGL *) = 0;
	
	virtual fGL GetElDens(fGL *, fGL *) = 0;
	
	virtual fGL GetOrbital(fGL *, fGL *) = 0;
	virtual fGL GetOrbDens(fGL *, fGL *) = 0;
	
	// these are to take data out for external apps etc...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	f64 GetEnergy(void) { return energy; }		///< Compute() must be called before this one!

	virtual void WriteAnglesHeader(FILE * stream, int delim){}
	virtual void WriteAngles(FILE * stream, int delim){}
};

/*################################################################################################*/

fGL value_VDWSurf(engine *, fGL *, fGL *);
fGL value_ESP(engine *, fGL *, fGL *);
fGL value_ElDens(engine *, fGL *, fGL *);
fGL value_Orbital(engine *, fGL *, fGL *);
fGL value_OrbDens(engine *, fGL *, fGL *);

void CalcMaxMinCoordinates(model * p1, engine * p2, i32u);
void CopyCRD(model *, engine *, i32u);
void CopyCRD(engine *, model *, i32u);

/*################################################################################################*/

/// A base engine class for systems that utiliza a (modified) boundary potential.

class engine_mbp : virtual public engine
{
	protected:
	
	bool use_simple_bp; f64 bp_center[3];
	f64 bp_radius_solute; f64 bp_fc_solute;
	f64 bp_radius_solvent; f64 bp_fc_solvent;
	
	number_density_evaluator * nd_eval;
	radial_density_function_evaluator * rdf_eval;

	friend class model;
	
	friend class number_density_evaluator;
	friend class radial_density_function_evaluator;
	
	public:
	
	engine_mbp(setup *, i32u);
	virtual ~engine_mbp(void);
};

/*################################################################################################*/

/// A base engine class for systems with periodic boundary conditions.

class engine_pbc : virtual public engine
{
	protected:
	
	i32s nmol_mm;
	i32s * mrange;
	
	bool update;
	
	// TODO : how to use RDF-evaluator also here???

	friend class model;
	
	public:
	
	engine_pbc(setup *, i32u);
	virtual ~engine_pbc(void);
	
/**	This will check that molecules have not escaped from the periodic box. 
	If we doing geometry optimization or molecular dynamics for periodic models, 
	we should remember to call this at suitable intervals...
*/
	void CheckLocations(void);
};

class engine_pbc2 : /*virtual*/ public engine_pbc
{
#if KLAPAN_DIFFUSE_WORKING

	int num_klap;
	double z_klapan;
	double z_solvent;

	long n_solvent_above_klapan;
	long pre_solvent_above_klapan;
	long dn_solvent_through_klapan;
	
	vector<i32s> klapan_list;
	vector<f64> vz_klap;
	vector<f64> vdz_klap;
	long n_solvent_through_klapan;
#endif
	long dn_solvent_through_z;
	long n_solvent_through_z;
public:
#if KLAPAN_DIFFUSE_WORKING
	int GetSolventNumberThroughKlapan(){return n_solvent_through_klapan;}
	void ReadClapanList(char * fn);
	double GetKlapanZ(){return z_klapan;}
	double GetSolventZ(){return z_solvent;}
#endif

	double M_solvent;
	int n_solvent_molecules;


protected:
	//bool update;
	
	friend class model;

public:
	int GetSolventNumberThroughZ(){return n_solvent_through_z;}

	
	engine_pbc2(setup *, i32u);
	virtual ~engine_pbc2(void);
	
/**	This will check that molecules have not escaped from the periodic box. 
	If we doing geometry optimization or molecular dynamics for periodic models, 
	we should remember to call this at suitable intervals...
*/
#if GRAVI_OSCILLATOR_WORKING 
	void SetGraviAtomFlagOnSolvent();
#endif
	void CheckLocations(void);
};

/*################################################################################################*/

/// Calculates "number density" of solvent molecules -> engine::bp_center.

class number_density_evaluator
{
	protected:
	
	engine_mbp * eng;
	bool linear;
	
	i32s classes;
	f64 * upper_limits;
	f64 * class_volumes;
	
	i32u cycles;
	i32u * counts;
	
	public:
	
	number_density_evaluator(engine_mbp *, bool, i32s);
	~number_density_evaluator(void);
	
	private:
	
	void UpdateClassLimits(void);
	void ResetCounters(void);
	
	public:
	
	void PrintResults(ostream &);
	
	inline void AddCycle(void)
	{
		cycles++;
	}
	
	inline void AddValue(f64 p1)
	{
		i32s index = 0;
		while (index < classes)
		{
			if (p1 >= upper_limits[index]) index++;
			else break;
		}
		
		counts[index]++;
	}
};

/*################################################################################################*/

/// Calculates radial density function of solvent molecules (in practive O-O distances of H2O) -> engine::bp_center.

class radial_density_function_evaluator
{
	protected:
	
	engine_mbp * eng;
	
	i32s classes;
	f64 graph_begin;
	f64 graph_end;
	f64 count_begin;
	f64 count_end;
	
	f64 * upper_limits;
	f64 * class_volumes;
	
	i32u cycles;
	i32u * counts;
	
	friend class eng1_mm_tripos52_nbt_mbp;
	friend class eng1_mm_default_nbt_mbp;
	friend class eng1_sf;
	
	public:
	
	radial_density_function_evaluator(engine_mbp *, i32s, f64, f64, f64 = -1.0, f64 = -1.0);
	~radial_density_function_evaluator(void);
	
	private:
	
	void ResetCounters(void);
	
	public:
	
	void PrintResults(ostream &);
	
	inline void AddCycle(void)
	{
		cycles++;
	}
	
	inline void AddValue(f64 p1)
	{
		if (p1 < graph_begin) return;
		if (p1 >= graph_end) return;
		
		i32s index = 0;
		while (index < classes)
		{
			if (p1 >= upper_limits[index]) index++;
			else break;
		}
		
		counts[index]++;
	}
};

/*################################################################################################*/

#endif	// ENGINE_H

// eof
