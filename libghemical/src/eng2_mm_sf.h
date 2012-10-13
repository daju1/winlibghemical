// ENG2_MM_SF.H : the mixed MM/SF "engine" classes.

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef ENG2_MM_SF_H
#define ENG2_MM_SF_H

#include "libconfig.h"

class setup2_mm_sf;

class eng2_mm_sf;

#include "eng1_mm_default.h"

#include "eng1_sf.h"

/*################################################################################################*/

class setup2_mm_sf : public setup1_mm, public setup1_sf
{
	protected:
	
	static const i32u eng_id_tab[];
	static const char * eng_name_tab[];
	
	public:
	
	setup2_mm_sf(model *, bool = true);
	~setup2_mm_sf(void);

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

class eng2_mm_sf : public eng1_mm_default_mbp, public eng1_sf
{
	protected:
	
	public:
	
	eng2_mm_sf(setup *, i32u);
	~eng2_mm_sf(void);
	
	i32s GetOrbitalCount(void);	// virtual	// NOT NEEDED???
	f64 GetOrbitalEnergy(i32s);	// virtual	// NOT NEEDED???
	
	i32s GetElectronCount(void);	// virtual	// NOT NEEDED???
	
	void Compute(i32u);	// virtual
	
	i32s FindTorsion(atom *, atom *, atom *, atom *) { cout << "BUG: FindTorsion() is not yet implemented at eng2_mm_sf!" << endl; exit(EXIT_FAILURE); }
	bool SetTorsionConstraint(i32s, f64, f64, bool) { cout << "BUG: SetTorsionConstraint() is not yet implemented at eng2_mm_sf!" << endl; exit(EXIT_FAILURE); }
	
	void SetupPlotting(void);	// virtual

	fGL GetVDWSurf(fGL *, fGL *);	// virtual
	
	fGL GetESP(fGL *, fGL *);	// virtual
	
	fGL GetElDens(fGL *, fGL *);	// virtual
	
	fGL GetOrbital(fGL *, fGL *);	// virtual
	fGL GetOrbDens(fGL *, fGL *);	// virtual
};

/*################################################################################################*/

#endif	// ENG2_MM_SF_H

// eof
