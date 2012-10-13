// ENG2_QM_MM.H : the mixed QM/MM "engine" classes.

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef ENG2_QM_MM_H
#define ENG2_QM_MM_H

#include "libconfig.h"

class setup2_qm_mm;

#include "eng1_qm.h"
#include "eng1_mm_default.h"

#ifdef ENABLE_MOPAC7
class eng2_qm_mm_mopac;
#include "eng1_qm_mopac.h"
#endif	// ENABLE_MOPAC7

#ifdef ENABLE_MPQC
class eng2_qm_mm_mpqc;
#include "eng1_qm_mpqc.h"
#endif	// ENABLE_MPQC

/*################################################################################################*/

class setup2_qm_mm : public setup1_qm, public setup1_mm
{
	protected:

	static const i32u eng_id_tab[];
	static const char * eng_name_tab[];
	
	public:
	
	setup2_qm_mm(model *);
	~setup2_qm_mm(void);

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

#ifdef ENABLE_MOPAC7

class eng2_qm_mm_mopac : public eng1_qm_mopac, public eng1_mm_default_mbp
{
	protected:
	
	public:
	
	eng2_qm_mm_mopac(setup *, i32u, i32u);
	~eng2_qm_mm_mopac(void);
	
	i32s GetOrbitalCount(void);	// virtual
	f64 GetOrbitalEnergy(i32s);	// virtual
	
	i32s GetElectronCount(void);	// virtual
	
	void Compute(i32u);	// virtual
	
	i32s FindTorsion(atom *, atom *, atom *, atom *) { cout << "BUG: FindTorsion() is not yet implemented at eng2_qm_mm!" << endl; exit(EXIT_FAILURE); }
	bool SetTorsionConstraint(i32s, f64, f64, bool) { cout << "BUG: SetTorsionConstraint() is not yet implemented at eng2_qm_mm!" << endl; exit(EXIT_FAILURE); }
	
	void SetupPlotting(void);	// virtual

	fGL GetVDWSurf(fGL *, fGL *);	// virtual
	
	fGL GetESP(fGL *, fGL *);	// virtual
	
	fGL GetElDens(fGL *, fGL *);	// virtual
	
	fGL GetOrbital(fGL *, fGL *);	// virtual
	fGL GetOrbDens(fGL *, fGL *);	// virtual
};

#endif	// ENABLE_MOPAC7

/*################################################################################################*/

#ifdef ENABLE_MPQC

class eng2_qm_mm_mpqc : public eng1_qm_mpqc, public eng1_mm_default_mbp
{
	protected:
	
	public:
	
	eng2_qm_mm_mpqc(setup *, i32u, i32u);
	~eng2_qm_mm_mpqc(void);
	
	i32s GetOrbitalCount(void);	// virtual
	f64 GetOrbitalEnergy(i32s);	// virtual
	
	i32s GetElectronCount(void);	// virtual
	
	void Compute(i32u);	// virtual
	
	i32s FindTorsion(atom *, atom *, atom *, atom *) { cout << "BUG: FindTorsion() is not yet implemented at eng2_qm_mm!" << endl; exit(EXIT_FAILURE); }
	bool SetTorsionConstraint(i32s, f64, f64, bool) { cout << "BUG: SetTorsionConstraint() is not yet implemented at eng2_qm_mm!" << endl; exit(EXIT_FAILURE); }
	
	void SetupPlotting(void);	// virtual

	fGL GetVDWSurf(fGL *, fGL *);	// virtual
	
	fGL GetESP(fGL *, fGL *);	// virtual
	
	fGL GetElDens(fGL *, fGL *);	// virtual
	
	fGL GetOrbital(fGL *, fGL *);	// virtual
	fGL GetOrbDens(fGL *, fGL *);	// virtual
};

#endif	// ENABLE_MPQC

/*################################################################################################*/

#endif	// ENG2_QM_MM_H

// eof
