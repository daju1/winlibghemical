// ENG2_QM_MM.CPP

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "eng2_qm_mm.h"

/*################################################################################################*/

// THE ID NUMBERS SHOULD NOT CHANGE!!! the numbering logic is the following:
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// for the QM stuff, use bits 0x00FFFF as explained in eng1_qm.h; for MM just set 0x010000 so far...

const i32u setup2_qm_mm::eng_id_tab[] =
{
// nothing really usable is ready here ; this is more like a demonstration of the setup class
// design that allows nested engine classes, if needed ; but this is not a top priority right now.

/*	0x010000 | (ENG1_QM_MOPAC | MOPAC_MNDO),
	0x010000 | (ENG1_QM_MOPAC | MOPAC_MINDO3),
	0x010000 | (ENG1_QM_MOPAC | MOPAC_AM1),
	0x010000 | (ENG1_QM_MOPAC | MOPAC_PM3),
	
#ifdef ENABLE_MPQC
	
	0x010000 | (ENG1_QM_MPQC | MPQC_STO3G),
	0x010000 | (ENG1_QM_MPQC | MPQC_STO6G),
	0x010000 | (ENG1_QM_MPQC | MPQC_3_21G),
	0x010000 | (ENG1_QM_MPQC | MPQC_3_21GS),
	0x010000 | (ENG1_QM_MPQC | MPQC_4_31G),
	0x010000 | (ENG1_QM_MPQC | MPQC_4_31GS),
	0x010000 | (ENG1_QM_MPQC | MPQC_4_31GSS),
	0x010000 | (ENG1_QM_MPQC | MPQC_6_31G),
	0x010000 | (ENG1_QM_MPQC | MPQC_6_31GS),
	0x010000 | (ENG1_QM_MPQC | MPQC_6_31GSS),
	
#endif	// ENABLE_MPQC	*/
	
	(i32u) NOT_DEFINED
};

const char * setup2_qm_mm::eng_name_tab[] =
{
// nothing really usable is ready here ; this is more like a demonstration of the setup class
// design that allows nested engine classes, if needed ; but this is not a top priority right now.

/*	"eng2_qm_mm_mopac : MOPAC7 / MNDO",
	"eng2_qm_mm_mopac : MOPAC7 / MINDO/3",
	"eng2_qm_mm_mopac : MOPAC7 / AM1",
	"eng2_qm_mm_mopac : MOPAC7 / PM3",
	
#ifdef ENABLE_MPQC
	
	"eng2_qm_mm_mpqc : MPQC / STO-3G",
	"eng2_qm_mm_mpqc : MPQC / STO-6G",
	"eng2_qm_mm_mpqc : MPQC / 3-21G",
	"eng2_qm_mm_mpqc : MPQC / 3-21G*",
	"eng2_qm_mm_mpqc : MPQC / 4-31G",
	"eng2_qm_mm_mpqc : MPQC / 4-31G*",
	"eng2_qm_mm_mpqc : MPQC / 4-31G**",
	"eng2_qm_mm_mpqc : MPQC / 6-31G",
	"eng2_qm_mm_mpqc : MPQC / 6-31G*",
	"eng2_qm_mm_mpqc : MPQC / 6-31G**",
	
#endif	// ENABLE_MPQC	*/
	
	NULL
};

setup2_qm_mm::setup2_qm_mm(model * p1) : setup(p1), setup1_qm(p1), setup1_mm(p1)
{
}

setup2_qm_mm::~setup2_qm_mm(void)
{
}

void setup2_qm_mm::UpdateAtomFlags(void)
{
cout << "not yet ready!" << endl;
exit(EXIT_FAILURE);
}

i32u setup2_qm_mm::static_GetEngineCount(void)
{
	i32u count = 0;		// counted by names...
	while (eng_name_tab[count] != NULL) count++;
	
	return count;
}

i32u setup2_qm_mm::static_GetEngineIDNumber(i32u eng_index)
{
	return eng_id_tab[eng_index];
}

const char * setup2_qm_mm::static_GetEngineName(i32u eng_index)
{
	return eng_name_tab[eng_index];
}

const char * setup2_qm_mm::static_GetClassName(void)
{
	static char cn[] = "qm_mm";
	return cn;
}

i32u setup2_qm_mm::GetEngineCount(void)
{
	return static_GetEngineCount();
}

i32u setup2_qm_mm::GetEngineIDNumber(i32u eng_index)
{
	return static_GetEngineIDNumber(eng_index);
}

const char * setup2_qm_mm::GetEngineName(i32u eng_index)
{
	return static_GetEngineName(eng_index);
}

const char * setup2_qm_mm::GetClassName(void)
{
	return static_GetClassName();
}

engine * setup2_qm_mm::CreateEngineByIndex(i32u eng_index)
{
	if (eng_index >= GetEngineCount())
	{
		cout << "setup2_qm_mm::CreateEngineByIndex() failed!" << endl;
		return NULL;
	}
	
	GetModel()->UpdateIndex();
	UpdateSetupInfo();
	
	bool test = setup1_qm::CheckSettings(this);
	if (!test) return NULL;		// something was wrong with the QM settings...
	
	i32s engtype1 = (eng_id_tab[eng_index] & 0xff00);
	i32s engtype2 = (eng_id_tab[eng_index] & 0x00ff);
	
	switch (engtype1)
	{
		
#ifdef ENABLE_MOPAC7
		
		case ENG1_QM_MOPAC:
		if (eng1_qm_mopac::GetLock() != NULL)
		{
			GetModel()->ErrorMessage("MOPAC lock failed!!!\nCan't run multiple MOPAC calculations.");
			return NULL;
		}
		else return new eng2_qm_mm_mopac(this, 1, engtype2);
		
#endif	// ENABLE_MOPAC7
		
#ifdef ENABLE_MPQC
		
		case ENG1_QM_MPQC:
		return new eng2_qm_mm_mpqc(this, 1, engtype2);
		
#endif	// ENABLE_MPQC
		
		default:
		cout << "fatal error at setup2_qm_mm::CreateEngineByIndex()" << endl;
		return NULL;
	}
}

/*################################################################################################*/

#ifdef ENABLE_MOPAC7

eng2_qm_mm_mopac::eng2_qm_mm_mopac(setup * p1, i32u p2, i32u mode) : engine(p1, p2), eng1_mm(p1, p2),
	engine_mbp(p1, p2), eng1_qm_mopac(p1, p2, mode), eng1_mm_default_mbp(p1, p2)
{
}

eng2_qm_mm_mopac::~eng2_qm_mm_mopac(void)
{
}

i32s eng2_qm_mm_mopac::GetOrbitalCount(void)
{
	return eng1_qm_mopac::GetOrbitalCount();
}

f64 eng2_qm_mm_mopac::GetOrbitalEnergy(i32s p1)
{
	return eng1_qm_mopac::GetOrbitalEnergy(p1);
}

i32s eng2_qm_mm_mopac::GetElectronCount(void)
{
	return eng1_qm_mopac::GetElectronCount();
}

void eng2_qm_mm_mopac::Compute(i32u p1)
{
	f64 total_energy = 0.0;
	
	eng1_qm_mopac::Compute(p1);
cout << "Eqm = " << energy << "   ";
	total_energy += energy;
	
	eng1_mm_default_mbp::Compute(p1);
cout << "Emm = " << energy << "   ";
	total_energy += energy;
	
	// glue code???
	
	energy = total_energy;
cout << "Etot = " << energy << endl;
}

void eng2_qm_mm_mopac::SetupPlotting(void)
{
	return eng1_qm_mopac::SetupPlotting();
}

fGL eng2_qm_mm_mopac::GetVDWSurf(fGL * pp, fGL * dd)
{
	fGL value = 0.0;
	value += eng1_qm_mopac::GetVDWSurf(pp, dd);
	value += eng1_mm_default_mbp::GetVDWSurf(pp, dd);
	
	if (dd != NULL)		// numerical gradient...
	{
		fGL old;
		const fGL delta = 0.0001;
		
		old = pp[0]; pp[0] += delta;
		dd[0] = (GetVDWSurf(pp, NULL) - value) / delta;
		pp[0] = old;
		
		old = pp[1]; pp[1] += delta;
		dd[1] = (GetVDWSurf(pp, NULL) - value) / delta;
		pp[1] = old;
		
		old = pp[2]; pp[2] += delta;
		dd[2] = (GetVDWSurf(pp, NULL) - value) / delta;
		pp[2] = old;
	}
	
	return value;
}

fGL eng2_qm_mm_mopac::GetESP(fGL * pp, fGL * dd)
{
	fGL value = 0.0;
	value += eng1_qm_mopac::GetESP(pp, dd);
	value += eng1_mm_default_mbp::GetESP(pp, dd);
	
	if (dd != NULL)		// numerical gradient...
	{
		fGL old;
		const fGL delta = 0.0001;
		
		old = pp[0]; pp[0] += delta;
		dd[0] = (GetESP(pp, NULL) - value) / delta;
		pp[0] = old;
		
		old = pp[1]; pp[1] += delta;
		dd[1] = (GetESP(pp, NULL) - value) / delta;
		pp[1] = old;
		
		old = pp[2]; pp[2] += delta;
		dd[2] = (GetESP(pp, NULL) - value) / delta;
		pp[2] = old;
	}
	
	return value;
}

fGL eng2_qm_mm_mopac::GetElDens(fGL * pp, fGL * dd)
{
	return eng1_qm_mopac::GetElDens(pp, dd);
}

fGL eng2_qm_mm_mopac::GetOrbital(fGL * pp, fGL * dd)
{
	return eng1_qm_mopac::GetOrbital(pp, dd);
}

fGL eng2_qm_mm_mopac::GetOrbDens(fGL * pp, fGL * dd)
{
	return eng1_qm_mopac::GetOrbDens(pp, dd);
}

#endif	// ENABLE_MOPAC7

/*################################################################################################*/

#ifdef ENABLE_MPQC

eng2_qm_mm_mpqc::eng2_qm_mm_mpqc(setup * p1, i32u p2, i32u mode) : engine(p1, p2), eng1_mm(p1, p2),
	engine_mbp(p1, p2), eng1_qm_mpqc(p1, p2, mode), eng1_mm_default_mbp(p1, p2)
{
}

eng2_qm_mm_mpqc::~eng2_qm_mm_mpqc(void)
{
}

i32s eng2_qm_mm_mpqc::GetOrbitalCount(void)
{
	return eng1_qm_mpqc::GetOrbitalCount();
}

f64 eng2_qm_mm_mpqc::GetOrbitalEnergy(i32s p1)
{
	return eng1_qm_mpqc::GetOrbitalEnergy(p1);
}

i32s eng2_qm_mm_mpqc::GetElectronCount(void)
{
	return eng1_qm_mpqc::GetElectronCount();
}

void eng2_qm_mm_mpqc::Compute(i32u p1)
{
	f64 total_energy = 0.0;
	
	eng1_qm_mpqc::Compute(p1);
cout << "Eqm = " << energy << "   ";
	total_energy += energy;
	
	eng1_mm_default_mbp::Compute(p1);
cout << "Emm = " << energy << "   ";
	total_energy += energy;
	
	// glue code???
	
	energy = total_energy;
cout << "Etot = " << energy << endl;
}

void eng2_qm_mm_mpqc::SetupPlotting(void)
{
	return eng1_qm_mpqc::SetupPlotting();
}

fGL eng2_qm_mm_mpqc::GetVDWSurf(fGL * pp, fGL * dd)
{
	fGL value = 0.0;
	value += eng1_qm_mpqc::GetVDWSurf(pp, dd);
	value += eng1_mm_default_mbp::GetVDWSurf(pp, dd);
	
	if (dd != NULL)		// numerical gradient...
	{
		fGL old;
		const fGL delta = 0.0001;
		
		old = pp[0]; pp[0] += delta;
		dd[0] = (GetVDWSurf(pp, NULL) - value) / delta;
		pp[0] = old;
		
		old = pp[1]; pp[1] += delta;
		dd[1] = (GetVDWSurf(pp, NULL) - value) / delta;
		pp[1] = old;
		
		old = pp[2]; pp[2] += delta;
		dd[2] = (GetVDWSurf(pp, NULL) - value) / delta;
		pp[2] = old;
	}
	
	return value;
}

fGL eng2_qm_mm_mpqc::GetESP(fGL * pp, fGL * dd)
{
	fGL value = 0.0;
	value += eng1_qm_mpqc::GetESP(pp, dd);
	value += eng1_mm_default_mbp::GetESP(pp, dd);
	
	if (dd != NULL)		// numerical gradient...
	{
		fGL old;
		const fGL delta = 0.0001;
		
		old = pp[0]; pp[0] += delta;
		dd[0] = (GetESP(pp, NULL) - value) / delta;
		pp[0] = old;
		
		old = pp[1]; pp[1] += delta;
		dd[1] = (GetESP(pp, NULL) - value) / delta;
		pp[1] = old;
		
		old = pp[2]; pp[2] += delta;
		dd[2] = (GetESP(pp, NULL) - value) / delta;
		pp[2] = old;
	}
	
	return value;
}

fGL eng2_qm_mm_mpqc::GetElDens(fGL * pp, fGL * dd)
{
	return eng1_qm_mpqc::GetElDens(pp, dd);
}

fGL eng2_qm_mm_mpqc::GetOrbital(fGL * pp, fGL * dd)
{
	return eng1_qm_mpqc::GetOrbital(pp, dd);
}

fGL eng2_qm_mm_mpqc::GetOrbDens(fGL * pp, fGL * dd)
{
	return eng1_qm_mpqc::GetOrbDens(pp, dd);
}

#endif	// ENABLE_MPQC

/*################################################################################################*/

// eof
