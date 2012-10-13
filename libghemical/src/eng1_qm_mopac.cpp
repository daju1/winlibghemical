// ENG1_QM_MOPAC.CPP

// Copyright (C) 2001 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "eng1_qm_mopac.h"

#ifdef ENABLE_MOPAC7

#include <iomanip>
#include <strstream>
using namespace std;

//#include <g2c.h>
#include "../../f2c/f2c.h"

static logical c_true = TRUE_;
static logical c_false = FALSE_;

extern "C" void lm7start_(void);	// libmopac7...
extern "C" void lm7stop_(void);		// libmopac7...

extern "C" int compfg_(doublereal *, logical *, doublereal *, logical *, doublereal *, logical *);

extern "C" int initplot_(void);
extern "C" int getesp_(void);
extern "C" int geteldens_(void);
extern "C" int getorb_(void);

// these come from MOPAC7/compfg.c
// these come from MOPAC7/compfg.c
// these come from MOPAC7/compfg.c

struct {
    integer natoms, labels[120], na[120], nb[120], nc[120];
} geokst_;

#define geokst_1 geokst_

struct {
    integer nvar, loc[720]	/* was [2][360] */, idumy;
    doublereal dumy[360];
} geovar_;

#define geovar_1 geovar_

struct {
    doublereal elect;
} elect_;

#define elect_1 elect_

struct {
    doublereal enuclr;
} enuclr_;

#define enuclr_1 enuclr_

struct {
    integer numat, nat[120], nfirst[120], nmidle[120], nlast[120], norbs, 
	    nelecs, nalpha, nbeta, nclose, nopen, ndumy;
    doublereal fract;
} molkst_;

#define molkst_1 molkst_

struct {
    doublereal c__[90000], eigs[300], cbeta[90000], eigb[300];
} vector_;

#define vector_1 vector_

// these come from MOPAC7/force.c
// these come from MOPAC7/force.c
// these come from MOPAC7/force.c

struct {
    doublereal grad[360], gnorm;
} gradnt_;

#define gradnt_1 gradnt_

// these come from MOPAC7/esp.c
// these come from MOPAC7/esp.c
// these come from MOPAC7/esp.c

union {
    struct {
	doublereal potpt[150000]	/* was [3][50000] */, work1d[200000];
    } _1;
    struct {
	doublereal potpt[150000]	/* was [3][50000] */, pad1[100000], 
		rad[50000];
	integer ias[50000];
    } _2;
    struct {
	doublereal potpt[150000]	/* was [3][50000] */, es[50000], esp[
		50000], work1d[100000];
    } _3;
} work1_;

#define work1_1 (work1_._1)
#define work1_2 (work1_._2)
#define work1_3 (work1_._3)

struct {
    doublereal xc, yc, zc, espnuc, espele;
    integer nesp;
} potesp_;

#define potesp_1 potesp_

struct {
    doublereal cespm2[90000]	/* was [300][300] */, sla[10], cespml[90000], 
	    cesp[90000];
    integer inc[1800], nc, npr, is, ip, ipc, isc, icd, iorb;
} plots_;

#define plots_1 plots_

/*################################################################################################*/

eng1_qm_mopac * eng1_qm_mopac::mopac_lock = NULL;

eng1_qm_mopac::eng1_qm_mopac(setup * p1, i32u p2, i32u mode) : engine(p1, p2), eng1_qm(p1, p2)
{
	if (mopac_lock != NULL)
	{
		// the main MOPAC locking mechanism is now in model::CreateDefaultEngine().
		// if the locking fails in this stage, take it as a serious error and shutdown...
		
		GetSetup()->GetModel()->ErrorMessage("locking error at eng1_qm_mopac ctor.");
		exit(EXIT_FAILURE);
	}
	else mopac_lock = this;
	
	char fn_FOR005[256] = "FOR005";
	if (getenv("FOR005") != NULL) strcpy(fn_FOR005, getenv("FOR005"));
	
	cout << "writing MOPAC-input file " << fn_FOR005 << endl;
	
	ofstream file;
	file.open(fn_FOR005, ios::out);
	
	file << "XYZ NOLOG ";
	file << "SCFCRT=0.000001 ";	// slightly less strict than default, but seems to stabilize geomopt.
	
	switch (mode)
	{
		case MOPAC_MINDO3:
		file << "MINDO ";
		break;
		
		case MOPAC_AM1:
		file << "AM1 ";
		break;
		
		case MOPAC_PM3:
		file << "PM3 ";
		break;
		
		// if we don't write anything here, the default MNDO method will be used...
		// if we don't write anything here, the default MNDO method will be used...
		// if we don't write anything here, the default MNDO method will be used...
	};
	
// one should use UHF for transition state searching???
// but it will break the energy level diagrams...
// and unfortunately is quite slow...

//file << "UHF ";
//file << "TRIPLET ";

	file << "CHARGE=" << GetSetup()->GetModel()->GetQMTotalCharge() << " ";
	file << "GEO-OK MMOK ";
	file << endl;
	
	file << "automatically generated MOPAC input file." << endl << endl;
	
// add one dummy atom, to avoid program crash with molecules that have 3 first atoms linearly arranged.
// using this dummy atom won't have any other effect, except it cheats a test at GETGEO.F around line 357.
// removing this dummy atom and the test at GETGEO.F seems to produce exactly similar results...

// this problem can be studied with a molecule CH2=C=CH2 that can be drawn in either "right" way (carbons
// are the first three atoms) or "wrong" way... produces slightly higher energies and different MOs.

// HOW TO AVOID THIS PROBLEM: draw the molecule in 1-3-2 order instead of 1-2-3 order!!!
// HOW TO AVOID THIS PROBLEM: draw the molecule in 1-3-2 order instead of 1-2-3 order!!!
// HOW TO AVOID THIS PROBLEM: draw the molecule in 1-3-2 order instead of 1-2-3 order!!!

////////////////////////////////////////////////////////////////////////////////
//file << "XX     +1.00 1   +1.00 1   +1.00 1" << endl;	// this line can be commented out...
////////////////////////////////////////////////////////////////////////////////

	atom ** atmtab = GetSetup()->GetQMAtoms();
	for (i32s index = 0;index < GetSetup()->GetQMAtomCount();index++)
	{
		const fGL * defcrd = atmtab[index]->GetCRD(0);
		const fGL cf = 10.0;	// conversion factor for [nm] -> [Å]
		
		file << atmtab[index]->el.GetSymbol() << "\t";
		file << setprecision(6) << setw(12) << (defcrd[0] * cf) << " 1 ";
		file << setprecision(6) << setw(12) << (defcrd[1] * cf) << " 1 ";
		file << setprecision(6) << setw(12) << (defcrd[2] * cf) << " 1 ";
		file << endl;
	}
	
	file << endl;	// add an empty line to mark end of this input???
	file.close();
	
	// the MOPAC output is now directed to console; that's because at the beginning of LM7START()
	// at m7MAIN.c the unit 6 file is not opened. to send output to a logfile, open the unit 6 file.
	
//	cout << "start MOPAC... ";
	lm7start_();
//	cout << "OK!" << endl;
	
	// by default, MOPAC changes the molecule orientation, and limits the available variables.
	// we will change it here, by writing our own variable table. THIS IS FOR XYZ MODE ONLY!!!
	
	geovar_1.nvar = geokst_1.natoms * 3;
	
	for (int zzz = 0;zzz < geovar_1.nvar;zzz++)
	{
		geovar_1.loc[zzz * 2 + 0] = (zzz / 3) + 1;
	}
	
	for (int zzz = 0;zzz < geovar_1.nvar;zzz++)
	{
		geovar_1.loc[zzz * 2 + 1] = (zzz % 3) + 1;
	}
	
	// ok, but now we still have to update the geometry before doing calculations...
	// that will happen on each call of the energy functions separately.
}

eng1_qm_mopac::~eng1_qm_mopac(void)
{
	if (mopac_lock != this) return;		// LOCK!!!
	
//	cout << "stop MOPAC... ";
	lm7stop_();
//	cout << "OK!" << endl;
	
	char fn_FOR005[256] = "FOR005";
	if (getenv("FOR005") != NULL) strcpy(fn_FOR005, getenv("FOR005"));
	
//	char fn_FOR006[256] = "FOR006";
//	if (getenv("FOR006") != NULL) strcpy(fn_FOR006, getenv("FOR006"));
	
	char fn_SHUTDOWN[256] = "SHUTDOWN";
	if (getenv("SHUTDOWN") != NULL) strcpy(fn_SHUTDOWN, getenv("SHUTDOWN"));
	
	char command[256];
	cout << "removing intermediate files... ";
	ostrstream cstr1(command, sizeof(command)); cstr1 << "rm " << fn_FOR005 << ends; system(command);
//	ostrstream cstr2(command, sizeof(command)); cstr2 << "rm " << fn_FOR006 << ends; system(command);
	ostrstream cstr3(command, sizeof(command)); cstr3 << "rm " << fn_SHUTDOWN << ends; system(command);
	cout << "OK!" << endl;
	
	mopac_lock = NULL;
}

i32s eng1_qm_mopac::GetOrbitalCount(void)
{
	return molkst_1.norbs;		// for RHF case...
	
//cout << "norbs = " << molkst_1.norbs << endl;
//cout << "nalpha = " << molkst_1.nalpha << endl;
//	return 3;
}

f64 eng1_qm_mopac::GetOrbitalEnergy(i32s index)
{
	return vector_1.eigs[index];
}

i32s eng1_qm_mopac::GetElectronCount(void)
{
	return molkst_1.nelecs;
}

void eng1_qm_mopac::Compute(i32u p1)
{
	E_solute = 0.0;		// re-think this for eng2_xx_yy classes!!!
	E_solvent = 0.0;	// re-think this for eng2_xx_yy classes!!!
	E_solusolv = 0.0;	// re-think this for eng2_xx_yy classes!!!
	
	if (mopac_lock != this) return;		// LOCK!!!
	
	double e; double hf;
	int i; static doublereal escf;
	
	const double cf1 = 10.0;	// conversion factor for [nm] -> [Å]
	const double cf2 = 96.4853;	// conversion factor for [eV] -> [kJ/mol]
	const double cf3 = 4.1868;	// conversion factor for [cal] -> [J]
	
	for (i = 0;i < geokst_1.natoms;i++)
	{
		geovar_1.dumy[i * 3 + 0] = crd[l2g_qm[i] * 3 + 0] * cf1;
		geovar_1.dumy[i * 3 + 1] = crd[l2g_qm[i] * 3 + 1] * cf1;
		geovar_1.dumy[i * 3 + 2] = crd[l2g_qm[i] * 3 + 2] * cf1;
	}
	
	if (p1 == 0)		// energy was requested...
	{
		compfg_(geovar_1.dumy, &c_true, &escf, &c_true, gradnt_1.grad, &c_false);
		
		e = (elect_1.elect + enuclr_1.enuclr) * cf2;
		hf = escf;
	}
	else if (p1 == 1)	// energy and gradient was requested...
	{
		compfg_(geovar_1.dumy, &c_true, &escf, &c_true, gradnt_1.grad, &c_true);
		
		e = (elect_1.elect + enuclr_1.enuclr) * cf2;
		hf = escf;
		
		for (i = 0;i < geokst_1.natoms;i++)
		{
			d1[l2g_qm[i] * 3 + 0] = gradnt_1.grad[i * 3 + 0] * cf3 * cf1;
			d1[l2g_qm[i] * 3 + 1] = gradnt_1.grad[i * 3 + 1] * cf3 * cf1;
			d1[l2g_qm[i] * 3 + 2] = gradnt_1.grad[i * 3 + 2] * cf3 * cf1;
		}
	}
	else	// can't calculate higher derivatives just yet...
	{
		cout << "BUG: no d2 available at eng1_qm_mopac::Compute()" << endl;
		exit(EXIT_FAILURE);
	}
	
//cout << "energy = " << e << " kJ/mol" << endl;
//cout << "heat of formation = " << hf << " kcal/mol" << endl;

	energy = e;
	
	// the rest is for transition state search only!!!
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	if (tss_ref_str == NULL) return;
	
	// this is currently implemented for MOPAC only; there is no problems in
	// making it for MPQC as well, but since semi-empirical methods are quicker
	// this seems to be enough. ONE CAN USE AB INITIO IN STATIONARY STATE SEARCH!
	
	tss_delta_ene = 0.0;
	for (i = 0;i < geokst_1.natoms;i++)
	{
		f64 t1a[3]; f64 t1b = 0.0;
		for (i32s n5 = 0;n5 < 3;n5++)
		{
			f64 t9a = crd[l2g_qm[i] * 3 + n5];
			f64 t9b = tss_ref_str[i * 3 + n5];
			
			t1a[n5] = t9a - t9b;
			t1b += t1a[n5] * t1a[n5];
		}
		
		f64 t1c = sqrt(t1b);
		for (i32s n5 = 0;n5 < 3;n5++)
		{
			f64 t9a = t1a[n5] / t1c;
			
			t1a[n5] = +t9a;
		}
		
		// f = a(x)^2
		// df/dx = 2a(x)
		
		f64 t2a = tss_force_const * t1b;
		energy += t2a;
		
		// df/fa = (x)^2
		
		tss_delta_ene += t1b;
		
		if (p1 > 0)
		{
			f64 t2b = 2.0 * tss_force_const * t1c;
			
			for (i32s n5 = 0;n5 < 3;n5++)
			{
				f64 t2c = t1a[n5] * t2b;
				
				d1[i * 3 + n5] += t2c;
			}
		}
	}
}

void eng1_qm_mopac::SetupPlotting(void)
{
	initplot_();
}

fGL eng1_qm_mopac::GetESP(fGL * pp, fGL * dd)
{
	if (mopac_lock != this) return 0.0;	// LOCK!!!
	
	const double cf1 = 10.0;	// conversion factor for [nm] -> [Å]
	const double cf2 = 2625.5;	// conversion factor for [Hartree] -> [kJ/mol]
	
	potesp_1.nesp = 1;
	work1_3.potpt[0] = pp[0] * cf1;
	work1_3.potpt[1] = pp[1] * cf1;
	work1_3.potpt[2] = pp[2] * cf1;
	
	getesp_();
	
	fGL value = work1_3.esp[0] * cf2;
	
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

fGL eng1_qm_mopac::GetElDens(fGL * pp, fGL * dd)
{
	if (mopac_lock != this) return 0.0;	// LOCK!!!
	
	const double cf1 = 18.897162;	// conversion factor for [nm] -> [bohr]
	
	potesp_1.nesp = 1;
	work1_3.potpt[0] = pp[0] * cf1;
	work1_3.potpt[1] = pp[1] * cf1;
	work1_3.potpt[2] = pp[2] * cf1;
	
	geteldens_();
	
	fGL value = work1_3.esp[0];
	
	if (dd != NULL)		// numerical gradient...
	{
		fGL old;
		const fGL delta = 0.0001;
		
		old = pp[0]; pp[0] += delta;
		dd[0] = (GetElDens(pp, NULL) - value) / delta;
		pp[0] = old;
		
		old = pp[1]; pp[1] += delta;
		dd[1] = (GetElDens(pp, NULL) - value) / delta;
		pp[1] = old;
		
		old = pp[2]; pp[2] += delta;
		dd[2] = (GetElDens(pp, NULL) - value) / delta;
		pp[2] = old;
	}
	
	return value;
}

fGL eng1_qm_mopac::GetOrbital(fGL * pp, fGL * dd)
{
	if (mopac_lock != this) return 0.0;	// LOCK!!!
	
	const double cf1 = 18.897162;	// conversion factor for [nm] -> [bohr]
	
	potesp_1.nesp = 1;
	work1_3.potpt[0] = pp[0] * cf1;
	work1_3.potpt[1] = pp[1] * cf1;
	work1_3.potpt[2] = pp[2] * cf1;
	plots_1.iorb = (GetSetup()->GetModel()->qm_current_orbital + 1);
	
	getorb_();
	
	fGL value = work1_3.esp[0];
	
	if (dd != NULL)		// numerical gradient...
	{
		fGL old;
		const fGL delta = 0.0001;
		
		old = pp[0]; pp[0] += delta;
		dd[0] = (GetOrbital(pp, NULL) - value) / delta;
		pp[0] = old;
		
		old = pp[1]; pp[1] += delta;
		dd[1] = (GetOrbital(pp, NULL) - value) / delta;
		pp[1] = old;
		
		old = pp[2]; pp[2] += delta;
		dd[2] = (GetOrbital(pp, NULL) - value) / delta;
		pp[2] = old;
	}
	
	return value;
}

fGL eng1_qm_mopac::GetOrbDens(fGL * pp, fGL * dd)
{
	if (mopac_lock != this) return 0.0;	// LOCK!!!
	
	// this orbital density plot is closely related to the orbital plot;
	// we just square the orbital function and multiply it with 2 (assuming 2 electrons per orbital, true for RHF).
	
	fGL orb = GetOrbital(pp, dd);
	fGL value = 2.0 * orb * orb;
	
	if (dd != NULL)		// numerical gradient...
	{
		fGL old;
		const fGL delta = 0.0001;
		
		old = pp[0]; pp[0] += delta;
		dd[0] = (GetOrbDens(pp, NULL) - value) / delta;
		pp[0] = old;
		
		old = pp[1]; pp[1] += delta;
		dd[1] = (GetOrbDens(pp, NULL) - value) / delta;
		pp[1] = old;
		
		old = pp[2]; pp[2] += delta;
		dd[2] = (GetOrbDens(pp, NULL) - value) / delta;
		pp[2] = old;
	}
	
	return value;
}

/*################################################################################################*/

#endif	// ENABLE_MOPAC7

// eof
