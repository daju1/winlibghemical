// ENG2_MM_SF.CPP

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "eng2_mm_sf.h"

#include "engine.h"

/*################################################################################################*/

const i32u setup2_mm_sf::eng_id_tab[] =
{
// nothing really usable is ready here ; this is more like a demonstration of the setup class
// design that allows nested engine classes, if needed ; but this is not a top priority right now.
	
//	0x010001,
	(i32u) NOT_DEFINED
};

const char * setup2_mm_sf::eng_name_tab[] =
{
// nothing really usable is ready here ; this is more like a demonstration of the setup class
// design that allows nested engine classes, if needed ; but this is not a top priority right now.
	
//	"eng2_mm_sf : under construction",
	NULL
};

setup2_mm_sf::setup2_mm_sf(model * p1, bool convert_crd) : setup(p1), setup1_mm(p1), setup1_sf(p1, convert_crd)
{
}

setup2_mm_sf::~setup2_mm_sf(void)
{
}

void setup2_mm_sf::UpdateAtomFlags(void)
{
cout << "not yet ready!" << endl;
exit(EXIT_FAILURE);
}

i32u setup2_mm_sf::static_GetEngineCount(void)
{
	i32u count = 0;		// counted by names...
	while (eng_name_tab[count] != NULL) count++;
	
	return count;
}

i32u setup2_mm_sf::static_GetEngineIDNumber(i32u eng_index)
{
	return eng_id_tab[eng_index];
}

const char * setup2_mm_sf::static_GetEngineName(i32u eng_index)
{
	return eng_name_tab[eng_index];
}

const char * setup2_mm_sf::static_GetClassName(void)
{
	static char cn[] = "mm_sf";
	return cn;
}

i32u setup2_mm_sf::GetEngineCount(void)
{
	return static_GetEngineCount();
}

i32u setup2_mm_sf::GetEngineIDNumber(i32u eng_index)
{
	return static_GetEngineIDNumber(eng_index);
}

const char * setup2_mm_sf::GetEngineName(i32u eng_index)
{
	return static_GetEngineName(eng_index);
}

const char * setup2_mm_sf::GetClassName(void)
{
	return static_GetClassName();
}

engine * setup2_mm_sf::CreateEngineByIndex(i32u eng_index)
{
	if (eng_index >= GetEngineCount())
	{
		cout << "setup2_mm_sf::CreateEngineByIndex() failed!" << endl;
		return NULL;
	}
	
	GetModel()->UpdateIndex();
	UpdateSetupInfo();
	
	return new eng2_mm_sf(this, 1);		// the only choice...
}

/*################################################################################################*/

eng2_mm_sf::eng2_mm_sf(setup * p1, i32u p2) : engine(p1, p2), eng1_mm(p1, p2),
	engine_mbp(p1, p2), eng1_mm_default_mbp(p1, p2), eng1_sf(p1, p2, true, true)
{
}

eng2_mm_sf::~eng2_mm_sf(void)
{
}

i32s eng2_mm_sf::GetOrbitalCount(void)
{
	return 0;
}

f64 eng2_mm_sf::GetOrbitalEnergy(i32s)
{
	return 0.0;
}

i32s eng2_mm_sf::GetElectronCount(void)
{
	return 0;
}

void eng2_mm_sf::Compute(i32u p1)
{
	f64 total_energy = 0.0;
	
	eng1_mm_default_mbp::Compute(p1);
	total_energy += energy;
	
	eng1_sf::Compute(p1);
	total_energy += energy;
	
	// glue code???
	
	energy = total_energy;
}

void eng2_mm_sf::SetupPlotting(void)
{
}

fGL eng2_mm_sf::GetVDWSurf(fGL * pp, fGL * dd)
{
	fGL value = 0.0;
	value += eng1_mm_default_mbp::GetVDWSurf(pp, dd);
	value += eng1_sf::GetVDWSurf(pp, dd);
	
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

fGL eng2_mm_sf::GetESP(fGL * pp, fGL * dd)
{
	fGL value = 0.0;
	value += eng1_mm_default_mbp::GetESP(pp, dd);
	value += eng1_sf::GetESP(pp, dd);
	
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

fGL eng2_mm_sf::GetElDens(fGL *, fGL *)
{
	return 0.0;
}

fGL eng2_mm_sf::GetOrbital(fGL *, fGL *)
{
	return 0.0;
}

fGL eng2_mm_sf::GetOrbDens(fGL *, fGL *)
{
	return 0.0;
}

/*################################################################################################*/

// eof
