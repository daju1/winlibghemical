// ENG1_MM.H : molecular mechanics "engine" base classes.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef ENG1_MM_H
#define ENG1_MM_H

#include "libconfig.h"

class setup1_mm;

struct mm_bt1_data;		// saved distance results.
struct mm_bt2_data;		// saved angle results.

class eng1_mm;

/*################################################################################################*/

#include "atom.h"
#include "bond.h"

#include "model.h"
#include "engine.h"

#include <vector>
using namespace std;

/*################################################################################################*/

// THE ID NUMBERS SHOULD NOT CHANGE!!! the numbering logic is the following:
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// for periodic ones, set the bit 0x0100. use bits 0x00FF as ID numbers.

#define ENG1_MM_DEFAULT		0x0001		// eng1_mm_default
#define ENG1_MM_TRIPOS52	0x0050		// eng1_mm_tripos52
#define ENG1_MM_PERIODIC	0x0101		// eng1_mm_default_mim

#define ENG1_MM_EXPERIMENTAL	0x00F1		// eng1_mm_prmfit ; ALWAYS THE LAST ONE (disable in standard setup???)

/// A setup class for MM submodels; should always pass the atoms/bonds of MM submodel to eng.

class setup1_mm : virtual public setup
{
	protected:
	
	static const i32u eng_id_tab[];
	static const char * eng_name_tab[];
	
	bool exceptions;
	
	friend class default_tables;
	friend class setup_druid;	// FIXME:  for ghemical		// for BBB
	friend class setup_dialog;	// FIXME:  for ghemical		// for HEAD
	
	public:
	
	setup1_mm(model *);
	~setup1_mm(void);
	
	bool EnableExceptions(void) { return exceptions; }
	
	void UpdateAtomFlags(void);		// virtual
	
	static i32u static_GetEngineCount(void);
	static i32u static_GetEngineIDNumber(i32u);
	static const char * static_GetEngineName(i32u);
	static const char * static_GetClassName(void);
	
	i32u GetEngineCount(void);		// virtual
	i32u GetEngineIDNumber(i32u);		// virtual
	const char * GetEngineName(i32u);	// virtual
	const char * GetClassName(void);	// virtual
	
	engine * CreateEngineByIndex(i32u);	// virtual
};

/*################################################################################################*/

struct mm_bt1_data		// saved distance results.
{
	f64 len;
	f64 dlen[2][3];
};

struct mm_bt2_data		// saved angle results.
{
	f64 csa;
	f64 dcsa[3][3];
};

/*################################################################################################*/

/// A base engine class for molecular mechanics.

#define NEAR_LINEAR_LIMIT (165.0*M_PI/180.0)

class eng1_mm : virtual public engine
{
	protected:
	
	i32u * l2g_mm;		// the local-to-global lookup table.
	
	vector<atom *> cr1; i32s * range_cr1;	// connectivity records...
	vector<atom *> cr2; i32s * range_cr2;	// connectivity records...
	
	public:
	
	static const f64 fudgeLJ;	// how the 1-4 nonbonded interactions are modified...
	static const f64 fudgeQQ;	// how the 1-4 nonbonded interactions are modified...
	
	f64 energy_bt1;
	f64 energy_bt2;
	f64 energy_bt3;
	f64 energy_bt4;
	
	f64 energy_nbt1a;	// dispersion
	f64 energy_nbt1b;	// electrostatic
	f64 energy_nbt1c;
	f64 energy_nbt1d;
	
	public:


		f64 GetBondStretchEnergy()		{return energy_bt1;}
		f64 GetAngleStretchEnergy()		{return energy_bt2;}
		f64 GetTorsionEnergy()			{return energy_bt3;}
		f64 GetOutOfPlaneEnergy()		{return energy_bt4;}
	
		f64 GetDispersionEnergy()		{return energy_nbt1a;}	// dispersion
		f64 GetElectrostaticEnergy()	{return energy_nbt1b;}	// electrostatic
		f64 GetNonBondedEnergyC()		{return energy_nbt1c;}
		f64 GetNonBondedEnergyD()		{return energy_nbt1d;}

		f64 GetBondedTermsEnergy()		{return (energy_bt1 + energy_bt2 + energy_bt3 + energy_bt4);}
		f64 GetNonBondedTermsEnergy()	{return (energy_nbt1a + energy_nbt1b + energy_nbt1c + energy_nbt1d);}


	
	eng1_mm(setup *, i32u);
	virtual ~eng1_mm(void);
	
	void Compute(i32u);	// virtual

	virtual void UpdateTerms(void){}

	
	virtual i32s GetOrbitalCount(void) { return 0; }	// virtual
	virtual f64 GetOrbitalEnergy(i32s) { return 0.0; }	// virtual
	
	virtual i32s GetElectronCount(void) { return 0; }	// virtual
	
	virtual void SetupPlotting(void) { }		// virtual
	
	virtual fGL GetVDWSurf(fGL *, fGL *);		// virtual
	
	virtual fGL GetESP(fGL *, fGL *);		// virtual
	
	virtual fGL GetElDens(fGL *, fGL *) { return 0.0; }	// virtual
	
	virtual fGL GetOrbital(fGL *, fGL *) { return 0.0; }	// virtual
	virtual fGL GetOrbDens(fGL *, fGL *) { return 0.0; }	// virtual
	
	private:
	
	void SearchCR1a(atom *);
	void SearchCR1b(atom *, bond *);
	void SearchCR2(atom *, bond *, bond *);
	
	protected:
	
	virtual void ComputeBT1(i32u) = 0;	// bond streching
	virtual void ComputeBT2(i32u) = 0;	// angle bending
	virtual void ComputeBT3(i32u) = 0;	// torsion
	virtual void ComputeBT4(i32u) = 0;	// out of plane
	
	virtual void ComputeNBT1(i32u) = 0;	// nonbonded
};

/*################################################################################################*/

#endif	// ENG1_MM_H

// eof
