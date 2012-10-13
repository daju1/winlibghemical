// ENG1_QM_MPQC.CPP

// Copyright (C) 2000 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "eng1_qm_mpqc.h"	// config.h is here -> we get ENABLE-macros here...

#ifdef ENABLE_MPQC

#include <strstream>
using namespace std;

#include "util/misc/regtime.h"			// RegionTimer
#include "util/misc/formio.h"			// SCFormIO

#include "math/scmat/vector3.h"			// SCVector3

#include "chemistry/molecule/formula.h"		// MolecularFormula

#include "chemistry/qc/basis/obint.h"		// PointChargeData

// force linkages!!! this will make sure that all important stuff will be linked from the libraries...
// force linkages!!! this will make sure that all important stuff will be linked from the libraries...
// force linkages!!! this will make sure that all important stuff will be linked from the libraries...

#include "chemistry/qc/scf/linkage.h"

/*################################################################################################*/

i32u eng1_qm_mpqc::ifile_name_counter = 0;

eng1_qm_mpqc::eng1_qm_mpqc(setup * p1, i32u p2, i32u mode) : eng1_qm(p1, p2), engine(p1, p2)
{
	char filename[32];
	GetInputFileName(filename, sizeof(filename));
	
	cout << "writing MPQC-input file " << filename << endl;
	
	ofstream file(filename, ios::out);
	WriteInputFile(file, mode);
	file.close();
	
	// now we start working with MPQC classes...
	// now we start working with MPQC classes...
	// now we start working with MPQC classes...
	
	// we should call here first ExEnv::init() and pass the command-line arguments to MPQC.
	// however, it seems that we can skip this if we are not doing parallel processing...
	
	grp = MessageGrp::get_default_messagegrp();
	cout << "using " << grp->class_name() << " as MessageGroup..." << endl;
	
	thread = ThreadGrp::get_default_threadgrp();
	cout << "using " << thread->class_name() << " as ThreadGroup..." << endl;
	
	parsedkv = new ParsedKeyVal(filename);
	keyval = new PrefixKeyVal("mpqc", parsedkv.pointer());
	
	static char basename[] = "ghemical";
	SCFormIO::set_default_basename(basename);
	SCFormIO::set_printnode(0);	// how to redirect the output into a logfile???
	
#if (SC_MAJOR_VERSION == 1)
	
	if (keyval->exists("matrixkit"))
	{
		SCMatrixKit::set_default_matrixkit(keyval->describedclassvalue("matrixkit"));
	}
	
	mole = keyval->describedclassvalue("mole");
	obwfn = mole; wfn = mole;
	
#else	// assume SC_MAJOR_VERSION == 2

	if (keyval->exists("matrixkit"))
	{
		SCMatrixKit::set_default_matrixkit(dynamic_cast<SCMatrixKit*>(keyval->describedclassvalue("matrixkit").pointer()));
	}
	
	mole << keyval->describedclassvalue("mole");
	obwfn << mole; wfn << mole;
	
#endif

	MolecularFormula mf(mole->molecule());
	cout << "molecular formula = " << mf.formula() << endl;
	
	// it seems that we have to calculate energy here.
	// it initializes something somewhere (problems in geometry changes).
	
	mole->energy();
}

eng1_qm_mpqc::~eng1_qm_mpqc(void)
{
	MessageGrp::set_default_messagegrp(0);
	ThreadGrp::set_default_threadgrp(0);
	
	SCMatrixKit::set_default_matrixkit(0);
	RegionTimer::set_default_regiontimer(0);
}

void eng1_qm_mpqc::GetInputFileName(char * fn, i32u maxlen)
{
	ostrstream str(fn, maxlen);
	str << "/tmp/mpqc" << setfill('0') << setw(4) << ifile_name_counter++ << ".in" << ends;
}

void eng1_qm_mpqc::WriteInputFile(ofstream & file, i32s mode)
{
	const fGL cf1 = 10.0;		// conversion factor for [nm] -> [Å]
	
	file.setf(ios::fixed);
	
	// molecule-record...
	// molecule-record...
	// molecule-record...
	
	file << "molecule<Molecule>: (" << endl;
	file << "  symmetry = C1" << endl;
	file << "  unit = angstrom" << endl;
	file << "  { atoms geometry } = {" << endl;
	
	// we will write the first coordinate set here by default...
	
	atom ** atmtab = GetSetup()->GetQMAtoms();
	for (i32s index = 0;index < GetSetup()->GetQMAtomCount();index++)
	{
		const fGL * defcrd = atmtab[index]->GetCRD(0);
		
		file << "    " << atmtab[index]->el.GetSymbol() << "\t[ ";
		file << setprecision(6) << setw(12) << (defcrd[0] * cf1) << " ";
		file << setprecision(6) << setw(12) << (defcrd[1] * cf1) << " ";
		file << setprecision(6) << setw(12) << (defcrd[2] * cf1) << " ";
		file << "]" << endl;
	}
	
	file << "  }" << endl;
	file << ")" << endl;
	
	// basis-record...
	// basis-record...
	// basis-record...
	
	file << "basis<GaussianBasisSet>: (" << endl;
	
	switch (mode)
	{
		case MPQC_STO6G:
		file << "  name = \"STO-6G\"" << endl;
		break;
		
		case MPQC_3_21G:
		file << "  name = \"3-21G\"" << endl;
		break;

		case MPQC_3_21GS:
		file << "  name = \"3-21G*\"" << endl;
		break;
		
		case MPQC_4_31G:
		file << "  name = \"4-31G\"" << endl;
		break;

		case MPQC_4_31GS:
		file << "  name = \"4-31G*\"" << endl;
		break;

		case MPQC_4_31GSS:
		file << "  name = \"4-31G**\"" << endl;
		break;

		case MPQC_6_31G:
		file << "  name = \"6-31G\"" << endl;
		break;

		case MPQC_6_31GS:
		file << "  name = \"6-31G*\"" << endl;
		break;

		case MPQC_6_31GSS:
		file << "  name = \"6-31G**\"" << endl;
		break;
		
		case MPQC_6_311G:
		file << "  name = \"6-311G\"" << endl;
		break;
		
		case MPQC_6_311GS:
		file << "  name = \"6-311G*\"" << endl;
		break;

		case MPQC_6_311GSS:
		file << "  name = \"6-311G**\"" << endl;
		break;
		
		default:	// MPQC_STO3G
		file << "  name = \"STO-3G\"" << endl;
	}
	file << "  molecule = $:molecule" << endl;
	file << ")" << endl;
	
	// mpqc-record...
	// mpqc-record...
	// mpqc-record...
	
	file << "mpqc: (" << endl;
	file << "  print_all_evals = no" << endl;
	file << "  print_occ_evals = no" << endl;
	file << "  mole<CLHF>: (" << endl;
	file << "    total_charge = " << GetSetup()->GetModel()->GetQMTotalCharge() << endl;
	file << "    print_molecule_when_changed = no" << endl;
	file << "    molecule = $:molecule" << endl;
	file << "    basis = $:basis" << endl;
	file << "  )" << endl;
	file << ")" << endl;
}

i32s eng1_qm_mpqc::GetOrbitalCount(void)
{
	RefDiagSCMatrix eigval = obwfn->eigenvalues();
	return eigval->n();
}

f64 eng1_qm_mpqc::GetOrbitalEnergy(i32s index)
{
	RefDiagSCMatrix eigval = obwfn->eigenvalues();
	return eigval->get_element(index);
}

i32s eng1_qm_mpqc::GetElectronCount(void)
{
	return wfn->nelectron();
}

void eng1_qm_mpqc::Compute(i32u p1)
{
	E_solute = 0.0;		// re-think this for eng2_xx_yy classes!!!
	E_solvent = 0.0;	// re-think this for eng2_xx_yy classes!!!
	E_solusolv = 0.0;	// re-think this for eng2_xx_yy classes!!!
	
	const double cf1 = 18.897162;	// conversion factor for [nm] -> [bohr]
	const double cf2 = 2625.5;	// conversion factor for [Hartree] -> [kJ/mol]
	
	RefSCVector newcrd(mole->moldim(), mole->matrixkit());
	for (i32s n1 = 0;n1 < GetSetup()->GetQMAtomCount();n1++)
	{
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			newcrd(n1 * 3 + n2) = crd[l2g_qm[n1] * 3 + n2] * cf1;
		}
	}
	
	mole->set_x(newcrd);
	
	if (p1 == 0)
	{
		double e = mole->energy();
		energy = e * cf2;
	}
	else if (p1 == 1)
	{
		double e = mole->energy();
		energy = e * cf2;
		
		RefSCVector gv = mole->get_cartesian_gradient();
		for (i32s n1 = 0;n1 < GetSetup()->GetQMAtomCount();n1++)
		{
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				d1[l2g_qm[n1] * 3 + n2] = gv[n1 * 3 + n2] * cf2 * cf1;
			}
		}
	}
	else	// can't calculate higher derivatives just yet...
	{
		cout << "BUG: no d2 available at eng1_qm_mpqc::Compute()" << endl;
		exit(EXIT_FAILURE);
	}
}

void eng1_qm_mpqc::SetupPlotting(void)
{
	// we don't need to do anything here...
}

fGL eng1_qm_mpqc::GetESP(fGL * pp, fGL * dd)
{
	const double cnv = 18.897162;	// conversion factor for [nm] -> [bohr]
	const double cf = 2625.5;	// conversion factor for [Hartree] -> [kJ/mol]
	
	fGL value = 0.0;
	
	// first, calculate negative potential due to electrons:
	// the code comes from mpqc-1.2.5/src/lib/chemistry/qc/wfn/solvent.cc
	
	double charge = +1.0;
	double pos_xyz[] = { pp[0] * cnv, pp[1] * cnv, pp[2] * cnv }; double * position = pos_xyz;
	
#if (SC_MAJOR_VERSION == 1)
	
	RefPointChargeData pc_dat = new PointChargeData(1, & position, & charge);
	
	RefOneBodyInt pc = wfn->integral()->point_charge(pc_dat);
	RefSCElementOp pc_op = new OneBodyIntOp(pc);
	RefSymmSCMatrix ao_density = wfn->ao_density()->copy();
	RefSymmSCMatrix pc_mat(ao_density->dim(), ao_density->kit());
	
	ao_density->scale(2.0);			// SCElementScalarProduct computes with unique matrix elements only -> in
	ao_density->scale_diagonal(0.5);	// symmetric matrices the off-diagonal elements are doubled to compensate this.
	
	RefSCElementScalarProduct sp = new SCElementScalarProduct;
	RefSCElementOp2 generic_sp(sp.pointer());
	
#else	// assume SC_MAJOR_VERSION == 2
	
	Ref<PointChargeData> pc_dat = new PointChargeData(1, & position, & charge);
	
	Ref<OneBodyInt> pc = wfn->integral()->point_charge(pc_dat);
	Ref<SCElementOp> pc_op = new OneBodyIntOp(pc);
	RefSymmSCMatrix ao_density = wfn->ao_density()->copy();			// written in the "old way"...
	RefSymmSCMatrix pc_mat(ao_density->dim(), ao_density->kit());		// written in the "old way"...
	
	ao_density->scale(2.0);			// SCElementScalarProduct computes with unique matrix elements only -> in
	ao_density->scale_diagonal(0.5);	// symmetric matrices the off-diagonal elements are doubled to compensate this.
	
	Ref<SCElementScalarProduct> sp = new SCElementScalarProduct;
	Ref<SCElementOp2> generic_sp(sp.pointer());
	
#endif

	pc_mat->assign(0.0);
	pc_mat->element_op(pc_op);
	sp->init();
	pc_mat->element_op(generic_sp, ao_density);
	value = cf * sp->result();
	
	// then, calculate positive potential due to nuclei:

	atom ** atmtab = GetSetup()->GetQMAtoms();
	for (i32s n1 = 0;n1 < GetSetup()->GetQMAtomCount();n1++)
	{
		f64 t1a[3]; f64 t1b = 0.0;
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			f64 t2a = pp[n2];
			f64 t2b = crd[l2g_qm[n1] * 3 + n2];
			
			t1a[n2] = t2a - t2b;
			
			t1b += t1a[n2] * t1a[n2];
		}
		
		f64 t1c = sqrt(t1b);
		
		// f1 = Zr^-1
		// df1/dr = -Zr^-2
		
		const f64 t9a = 138.9354518;
		f64 atnum = atmtab[n1]->el.GetAtomicNumber();
		if (atnum < 0.5) { cout << "BUG: bad atnum!!!" << endl; exit(EXIT_FAILURE); }
		value += t9a * atnum / t1c;
	}
	
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

fGL eng1_qm_mpqc::GetElDens(fGL * pp, fGL * dd)
{
	const double cnv = 18.897162;	// conversion factor for [nm] -> [bohr]
	SCVector3 v1((double) pp[0] * cnv, (double) pp[1] * cnv, (double) pp[2] * cnv);
	
	if (dd != NULL)
	{
		double dd2[3];
		double value = wfn->density_gradient(v1, dd2);
		for (i32s n1 = 0;n1 < 3;n1++) dd[n1] = dd2[n1];
		return value;
	}
	else return wfn->density(v1);
}

// natural orbitals seem to be much faster, but I'm not familiar with those...
// natural orbitals seem to be much faster, but I'm not familiar with those...
// natural orbitals seem to be much faster, but I'm not familiar with those...

fGL eng1_qm_mpqc::GetOrbital(fGL * pp, fGL * dd)
{
	const double cnv = 18.897162;	// conversion factor for [nm] -> [bohr]
	SCVector3 v1((double) pp[0] * cnv, (double) pp[1] * cnv, (double) pp[2] * cnv);
	
	fGL value = obwfn->orbital(v1, GetSetup()->GetModel()->GetQMCurrentOrbital());
	
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

fGL eng1_qm_mpqc::GetOrbDens(fGL * pp, fGL * dd)
{
	const double cnv = 18.897162;	// conversion factor for [nm] -> [bohr]
	SCVector3 v1((double) pp[0] * cnv, (double) pp[1] * cnv, (double) pp[2] * cnv);
	
	fGL value = obwfn->orbital_density(v1, GetSetup()->GetModel()->GetQMCurrentOrbital());

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

#endif	// ENABLE_MPQC

// eof
