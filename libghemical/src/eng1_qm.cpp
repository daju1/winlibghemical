// ENG1_QM.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "eng1_qm.h"

#include "eng1_qm_mpqc.h"
#include "eng1_qm_mopac.h"

/*################################################################################################*/

const i32u setup1_qm::eng_id_tab[] =
{
	
#ifdef ENABLE_MOPAC7
	
	(ENG1_QM_MOPAC | MOPAC_MNDO),
	(ENG1_QM_MOPAC | MOPAC_MINDO3),
	(ENG1_QM_MOPAC | MOPAC_AM1),
	(ENG1_QM_MOPAC | MOPAC_PM3),
	
#endif	// ENABLE_MOPAC7
	
#ifdef ENABLE_MPQC
	
	(ENG1_QM_MPQC | MPQC_STO3G),
	(ENG1_QM_MPQC | MPQC_STO6G),
	(ENG1_QM_MPQC | MPQC_3_21G),
	(ENG1_QM_MPQC | MPQC_3_21GS),
	(ENG1_QM_MPQC | MPQC_4_31G),
	(ENG1_QM_MPQC | MPQC_4_31GS),
	(ENG1_QM_MPQC | MPQC_4_31GSS),
	(ENG1_QM_MPQC | MPQC_6_31G),
	(ENG1_QM_MPQC | MPQC_6_31GS),
	(ENG1_QM_MPQC | MPQC_6_31GSS),
	(ENG1_QM_MPQC | MPQC_6_311G),
	(ENG1_QM_MPQC | MPQC_6_311GS),
	(ENG1_QM_MPQC | MPQC_6_311GSS),
	
#endif	// ENABLE_MPQC

	(i32u) NOT_DEFINED
};

const char * setup1_qm::eng_name_tab[] =
{
	
#ifdef ENABLE_MOPAC7
	
	"eng1_qm_mopac : MOPAC7 / MNDO",
	"eng1_qm_mopac : MOPAC7 / MINDO/3",
	"eng1_qm_mopac : MOPAC7 / AM1",
	"eng1_qm_mopac : MOPAC7 / PM3",
	
#endif	// ENABLE_MOPAC7
	
#ifdef ENABLE_MPQC

	"eng1_qm_mpqc : MPQC / STO-3G",
	"eng1_qm_mpqc : MPQC / STO-6G",
	"eng1_qm_mpqc : MPQC / 3-21G",
	"eng1_qm_mpqc : MPQC / 3-21G*",
	"eng1_qm_mpqc : MPQC / 4-31G",
	"eng1_qm_mpqc : MPQC / 4-31G*",
	"eng1_qm_mpqc : MPQC / 4-31G**",
	"eng1_qm_mpqc : MPQC / 6-31G",
	"eng1_qm_mpqc : MPQC / 6-31G*",		// 12
	"eng1_qm_mpqc : MPQC / 6-31G**",
	"eng1_qm_mpqc : MPQC / 6-311G",
	"eng1_qm_mpqc : MPQC / 6-311G*",	// 15
	"eng1_qm_mpqc : MPQC / 6-311G**",
	
#endif	// ENABLE_MPQC

	NULL
};

setup1_qm::setup1_qm(model * p1) : setup(p1)
{
}

setup1_qm::~setup1_qm(void)
{
}

void setup1_qm::UpdateAtomFlags(void)
{
	for (iter_al it1 = GetModel()->GetAtomsBegin();it1 != GetModel()->GetAtomsEnd();it1++)
	{
		(* it1).flags |= ATOMFLAG_IS_QM_ATOM;
	}
}

i32u setup1_qm::static_GetEngineCount(void)
{
	i32u count = 0;		// counted by names...
	while (eng_name_tab[count] != NULL) count++;
	
	return count;
}

i32u setup1_qm::static_GetEngineIDNumber(i32u eng_index)
{
	return eng_id_tab[eng_index];
}

const char * setup1_qm::static_GetEngineName(i32u eng_index)
{
	return eng_name_tab[eng_index];
}

const char * setup1_qm::static_GetClassName(void)
{
	static char cn[] = "allqm";
	return cn;
}

i32u setup1_qm::GetEngineCount(void)
{
	return static_GetEngineCount();
}

i32u setup1_qm::GetEngineIDNumber(i32u eng_index)
{
	return static_GetEngineIDNumber(eng_index);
}

const char * setup1_qm::GetEngineName(i32u eng_index)
{
	return static_GetEngineName(eng_index);
}

const char * setup1_qm::GetClassName(void)
{
	return static_GetClassName();
}

engine * setup1_qm::CreateEngineByIndex(i32u eng_index)
{
	if (eng_index >= GetEngineCount())
	{
		cout << "setup1_qm::CreateEngineByIndex() failed!" << endl;
		return NULL;
	}
	
	GetModel()->UpdateIndex();
	UpdateSetupInfo();
	
	bool test = CheckSettings(this);
	if (!test) return NULL;
	
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
		else return new eng1_qm_mopac(this, 1, engtype2);
		
#endif	// ENABLE_MOPAC7
		
#ifdef ENABLE_MPQC
		
		case ENG1_QM_MPQC:
		return new eng1_qm_mpqc(this, 1, engtype2);
		
#endif	// ENABLE_MPQC
		
		default:
		cout << "fatal error at setup1_qm::CreateEngineByIndex()" << endl;
		return NULL;
	}
}

bool setup1_qm::CheckSettings(setup * ref)	// engine IDNumber might be needed later???
{
	i32s electron_count = 0;
	
	atom ** atmtab = ref->GetQMAtoms();
	for (i32s n1 = 0;n1 < ref->GetQMAtomCount();n1++)
	{
		electron_count += atmtab[n1]->el.GetAtomicNumber();
	}
	
	// calculate the total number of electrons in the QM-subsystem.
	
	i32s total_electron_count = electron_count - ref->GetModel()->qm_total_charge;
cout << total_electron_count << " = " << electron_count << " - " << ref->GetModel()->qm_total_charge << endl;
	
	// checks start now...
	// checks start now...
	// checks start now...
	
	if (total_electron_count < 1)
	{
		ref->GetModel()->ErrorMessage("Less than one electron in the system!\nPlease check the \"total charge\" setting.");
		return false;
	}
	
	if (total_electron_count % 2)
	{
		ref->GetModel()->ErrorMessage("Odd number of electrons in the system!\nOnly singlet states with an even number\nof electrons are supported at the moment.\nPlease check the \"total charge\" setting.");
		return false;
	}
	
	// passed the tests...
	// passed the tests...
	// passed the tests...
	
	return true;
}

/*################################################################################################*/

eng1_qm::eng1_qm(setup * p1, i32u p2) : engine(p1, p2)
{
	// create the local-to-global lookup table...
	// create the local-to-global lookup table...
	// create the local-to-global lookup table...
	
	l2g_qm = new i32u[GetSetup()->GetQMAtomCount()];
	
	atom ** atmtab = GetSetup()->GetQMAtoms();
	atom ** glob_atmtab = GetSetup()->GetAtoms();
	for (i32s n1 = 0;n1 < GetSetup()->GetQMAtomCount();n1++)
	{
		i32s index = 0;
		while (index < GetSetup()->GetAtomCount())
		{
			if (atmtab[n1] == glob_atmtab[index]) break;
			else index++;
		}
		
		if (index >= GetSetup()->GetAtomCount())
		{
			cout << "BUG: eng1_qm ctor failed to create the l2g lookup table." << endl;
			exit(EXIT_FAILURE);
		}
		
		l2g_qm[n1] = index;
	}
	
	// transition_state_search will set this if needed...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	tss_ref_str = NULL;
	tss_force_const = 0.0;
	tss_delta_ene = 0.0;
}

eng1_qm::~eng1_qm(void)
{
	delete[] l2g_qm;
}

// f = sum[(r/a)^-3] = sum[a^3 * r^-3]		// now seems to be r^-12
// df/dr = -3 * sum[a^3 * r^-4]

fGL eng1_qm::GetVDWSurf(fGL * pp, fGL * dd)
{
	fGL vdwsv = 0.0;
	if (dd != NULL) dd[0] = dd[1] = dd[2] = 0.0;
	
	atom ** atmtab = GetSetup()->GetQMAtoms();
	for (i32s n1 = 0;n1 < GetSetup()->GetQMAtomCount();n1++)
	{
		fGL tmp1[3]; fGL r2 = 0.0;
		const f64 * cdata = & crd[l2g_qm[n1] * 3];
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			tmp1[n2] = pp[n2] - cdata[n2];
			r2 += tmp1[n2] * tmp1[n2];
		}
		
		if (r2 == 0.0) return +1.0e+35;		// numeric_limits<fGL>::max()?!?!?!
		fGL r1 = sqrt(r2);
		
		fGL tmp2 = r1 / (atmtab[n1]->vdwr + 0.0);	// solvent radius??? 0.15
		fGL qqq = tmp2 * tmp2 * tmp2 * tmp2;
		
		fGL tmp3 = 1.0 / (qqq * qqq * qqq);
		vdwsv += tmp3;
		
		if (dd != NULL)		// sign ??? constant ???
		{
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				fGL tmp4 = tmp1[n2] / r1;
				fGL tmp5 = tmp4 * tmp3 / tmp2;
				dd[n2] += tmp5;
			}
		}
	}
	
	return vdwsv;
}

/*################################################################################################*/

// eof
