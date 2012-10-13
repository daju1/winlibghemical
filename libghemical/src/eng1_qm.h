// ENG1_QM.H : quantum mechanics "engine" base classes.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef ENG1_QM_H
#define ENG1_QM_H

#include "libconfig.h"

class setup1_qm;

class eng1_qm;

/*################################################################################################*/

#include "atom.h"
#include "bond.h"

#include "model.h"
#include "engine.h"

/*################################################################################################*/

// THE ID NUMBERS SHOULD NOT CHANGE!!! the numbering logic is the following:
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// use bits 0xFF00 to select the eng class, and bits 0x00FF as ID numbers.

// 0xFF00 determines the eng class...

#define ENG1_QM_MOPAC		0x0100		// eng1_qm_mopac
#define ENG1_QM_MPQC		0x0200		// eng1_qm_mpqc

// 0x00FF determines the hamiltonian/basis set...

#define MOPAC_MNDO		0x01
#define MOPAC_MINDO3		0x02
#define MOPAC_AM1		0x03
#define MOPAC_PM3		0x04

#define MPQC_STO3G		0x01
#define MPQC_STO6G		0x02
#define MPQC_3_21G		0x03
#define MPQC_3_21GS		0x04
#define MPQC_4_31G		0x05
#define MPQC_4_31GS		0x06
#define MPQC_4_31GSS		0x07
#define MPQC_6_31G		0x08
#define MPQC_6_31GS		0x09
#define MPQC_6_31GSS		0x0A
#define MPQC_6_311G		0x0B
#define MPQC_6_311GS		0x0C
#define MPQC_6_311GSS		0x0D

/// A setup class for MM submodels; should always pass the atoms/bonds of MM submodel to eng.

class setup1_qm : virtual public setup
{
	protected:
	
	static const i32u eng_id_tab[];
	static const char * eng_name_tab[];
	
	public:
	
	setup1_qm(model *);
	~setup1_qm(void);

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
	
	static bool CheckSettings(setup *);
};

/*################################################################################################*/

/**	A base class for molecular QM calculations.

	The qm1-models can have different basis functions and other such different details, 
	so it is not as straightforward to draw planes/surfaces for those than for molecular 
	mechanics models. Therefore qm1-engine classes must provide us the values we need 
	for planes/surfaces.
*/

class eng1_qm : virtual public engine
{
	protected:

	i32u * l2g_qm;		// the local-to-global lookup table.
	
	f64 * tss_ref_str;	// this is for transition state search only; DO NOT DELETE HERE!!!
	f64 tss_force_const;	// this is for transition state search only...
	f64 tss_delta_ene;	// this is for transition state search only...
	
	friend class transition_state_search;
	
	public:
	
	eng1_qm(setup *, i32u);
	virtual ~eng1_qm(void);
	
	virtual i32s FindTorsion(atom *, atom *, atom *, atom *) { cout << "BUG: FindTorsion() is not yet implemented at eng1_qm!" << endl; exit(EXIT_FAILURE); }
	virtual bool SetTorsionConstraint(i32s, f64, f64, bool) { cout << "BUG: SetTorsionConstraint() is not yet implemented at eng1_qm!" << endl; exit(EXIT_FAILURE); }
	
	virtual fGL GetVDWSurf(fGL *, fGL *);	// virtual
};

/*################################################################################################*/

#endif	// ENG1_QM_H

// eof
