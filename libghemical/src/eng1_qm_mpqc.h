// ENG1_QM1_MPQC.H : the MPQC engine.

// Copyright (C) 2000 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef ENG1_QM1_MPQC_H
#define ENG1_QM1_MPQC_H

#include "libconfig.h"		// ENABLE_MPQC
#ifdef ENABLE_MPQC

class eng1_qm_mpqc;

/*################################################################################################*/

#include "eng1_qm.h"

#include <fstream>
using namespace std;

#include "util/keyval/keyval.h"			// KeyVal

#include "util/group/message.h"			// MessageGrp
#include "util/group/thread.h"			// ThreadGrp

#include "chemistry/molecule/energy.h"		// MolecularEnergy
#include "chemistry/qc/wfn/obwfn.h"		// OneBodyWavefunction
#include "chemistry/qc/wfn/wfn.h"		// Wavefunction

#include "math/scmat/abstract.h"		// SCMatrix

#if(SC_MAJOR_VERSION > 1 && SC_MINOR_VERSION > 0)

using namespace sc;	// the namespace "sc" was added in version 2.1.0, apparently.

#endif

/*################################################################################################*/

/**	The MPQC "wrapper" engine class...

	Strategy: 
	Initialize using a standard MPQC input file -> we will have also an input file 
	generator. For each different situation (closed shell, open shell, whatever) we 
	will have a different class. This is only for closed shell cases -> split this 
	to a base class/derived classes in future!!!!! For example, open shell cases 
	might have separate matrices for alpha and beta orbitals etc... 
*/

class eng1_qm_mpqc : public eng1_qm
{
	protected:
	
	static i32u ifile_name_counter;
	
#if(SC_MAJOR_VERSION == 1)
	
	RefMessageGrp grp;
	RefThreadGrp thread;
	
	RefParsedKeyVal parsedkv;
	RefKeyVal keyval;
	
	RefMolecularEnergy mole;
	
	RefOneBodyWavefunction obwfn;		// for plotting...
	RefWavefunction wfn;			// for plotting...
	
#else	// assume SC_MAJOR_VERSION == 2

	Ref<MessageGrp> grp;
	Ref<ThreadGrp> thread;
	
	Ref<ParsedKeyVal> parsedkv;
	Ref<KeyVal> keyval;
	
	Ref<MolecularEnergy> mole;
	
	Ref<OneBodyWavefunction> obwfn;		// for plotting...
	Ref<Wavefunction> wfn;			// for plotting...
	
#endif

	public:
	
	eng1_qm_mpqc(setup *, i32u, i32u);
	~eng1_qm_mpqc(void);
	
	void GetInputFileName(char *, i32u);
	void WriteInputFile(ofstream &, i32s);
	
	virtual i32s GetOrbitalCount(void);	// virtual
	virtual f64 GetOrbitalEnergy(i32s);	// virtual
	
	virtual i32s GetElectronCount(void);	// virtual
	
/**	It seems that we get the energy in atomic units... A set of unit conversion 
	functions should be made that makes conversions si-units <-> atomic units <-> 
	whatever units. These engine classes should have some "states" which determine 
	units that are printed out ?!?!?!?!?!?
*/
	virtual void Compute(i32u);		// virtual
	
	virtual void SetupPlotting(void);	// virtual
	
	virtual fGL GetESP(fGL *, fGL *);	// virtual
	virtual fGL GetElDens(fGL *, fGL *);	// virtual
	
	virtual fGL GetOrbital(fGL *, fGL *);	// virtual
	virtual fGL GetOrbDens(fGL *, fGL *);	// virtual
};

/*################################################################################################*/

#endif	// ENABLE_MPQC
#endif	// ENG1_QM1_MPQC_H

// eof
