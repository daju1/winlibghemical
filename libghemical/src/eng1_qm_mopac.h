// ENG1_QM_MOPAC.H : the MOPAC engine.

// Copyright (C) 2001 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef ENG1_QM_MOPAC_H
#define ENG1_QM_MOPAC_H

#include "libconfig.h"
#ifdef ENABLE_MOPAC7

class eng1_qm_mopac;

/*################################################################################################*/

#include "eng1_qm.h"

/*################################################################################################*/

class eng1_qm_mopac : public eng1_qm
{
	private:
	
	static eng1_qm_mopac * mopac_lock;
	
	protected:
	
	public:
	
	eng1_qm_mopac(setup *, i32u, i32u);
	~eng1_qm_mopac(void);
	
	static eng1_qm_mopac * GetLock(void) { return mopac_lock; }

	virtual i32s GetOrbitalCount(void);	// virtual
	virtual f64 GetOrbitalEnergy(i32s);	// virtual
	
	virtual i32s GetElectronCount(void);	// virtual

	virtual void Compute(i32u);	// virtual

	virtual void SetupPlotting(void);	// virtual
	
	virtual fGL GetESP(fGL *, fGL *);	// virtual
	
	virtual fGL GetElDens(fGL *, fGL *);	// virtual
	
	virtual fGL GetOrbital(fGL *, fGL *);	// virtual
	virtual fGL GetOrbDens(fGL *, fGL *);	// virtual
};

/*################################################################################################*/

#endif	// ENABLE_MOPAC7
#endif	// ENG1_QM_MOPAC_H

// eof
