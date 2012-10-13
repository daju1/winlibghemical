// UTILITY.CPP

// Copyright (C) 1998 Tommi Hassinen, Geoff Hutchison.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "utility.h"

#include "libdefine.h"

#include <string.h>

#include <iomanip>
#include <iostream>
#include <strstream>
using namespace std;

/*################################################################################################*/

superimpose::superimpose(model * p1, i32s p2, i32s p3) : conjugate_gradient(10, 1.0e-5, 1.0e+3)		// 2003-06-08 ok!!!
{
	mdl = p1;
	index[0] = p2;
	index[1] = p3;
	
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		rot[n1] = drot[n1] = 0.0;
		AddVar(& rot[n1], & drot[n1]);
		
		loc[n1] = dloc[n1] = 0.0;
		AddVar(& loc[n1], & dloc[n1]);
	}
}

superimpose::~superimpose(void)
{
}

f64 superimpose::GetRMS(void)
{
	return sqrt(value / (f64) counter);
}

void superimpose::Compare(const f64 * d1, const f64 * d2, bool gradient, f64 * result)
{
	const f64 balance = 0.25;
	
	// translate...
	// translate...
	// translate...
	
	f64 d2a[3];
	d2a[0] = d2[0] + loc[0] * balance;
	d2a[1] = d2[1] + loc[1] * balance;
	d2a[2] = d2[2] + loc[2] * balance;
	
	// rotate x (y,z)...
	// rotate x (y,z)...
	// rotate x (y,z)...
	
	f64 d2b[3];
	d2b[0] = d2a[0];
	d2b[1] = d2a[1] * cos(rot[0]) - d2a[2] * sin(rot[0]);
	d2b[2] = d2a[1] * sin(rot[0]) + d2a[2] * cos(rot[0]);

	// rotate y (z,x)...
	// rotate y (z,x)...
	// rotate y (z,x)...
	
	f64 d2c[3];
	d2c[0] = d2b[2] * sin(rot[1]) + d2b[0] * cos(rot[1]);
	d2c[1] = d2b[1];
	d2c[2] = d2b[2] * cos(rot[1]) - d2b[0] * sin(rot[1]);
	
	// rotate z (x,y)...
	// rotate z (x,y)...
	// rotate z (x,y)...
	
	f64 d2d[3];
	d2d[0] = d2c[0] * cos(rot[2]) - d2c[1] * sin(rot[2]);
	d2d[1] = d2c[0] * sin(rot[2]) + d2c[1] * cos(rot[2]);
	d2d[2] = d2c[2];
	
	// rot-gradients...
	// rot-gradients...
	// rot-gradients...
	
	f64 drz[3];
	drz[0] = -(d2c[0] * sin(rot[2]) + d2c[1] * cos(rot[2]));
	drz[1] = d2c[0] * cos(rot[2]) - d2c[1] * sin(rot[2]);
	drz[2] = 0.0;
	
	f64 dry[3];
	dry[0] = (d2b[2] * cos(rot[1]) - d2b[0] * sin(rot[1])) * cos(rot[2]);
	dry[1] = (d2b[2] * cos(rot[1]) - d2b[0] * sin(rot[1])) * sin(rot[2]);
	dry[2] = -(d2b[2] * sin(rot[1]) + d2b[0] * cos(rot[1]));
	
	f64 drx[3];
	drx[0] = ((d2a[1] * cos(rot[0]) - d2a[2] * sin(rot[0])) * sin(rot[1])) * cos(rot[2]) +
		(d2a[1] * sin(rot[0]) + d2a[2] * cos(rot[0])) * sin(rot[2]);
	drx[1] = ((d2a[1] * cos(rot[0]) - d2a[2] * sin(rot[0])) * sin(rot[1])) * sin(rot[2]) -
		(d2a[1] * sin(rot[0]) + d2a[2] * cos(rot[0])) * cos(rot[2]);
	drx[2] = (d2a[1] * cos(rot[0]) - d2a[2] * sin(rot[0])) * cos(rot[1]);
	
	// loc-gradients...
	// loc-gradients...
	// loc-gradients...
	
	f64 dlx[3];
	dlx[0] = balance * cos(rot[1]) * cos(rot[2]);
	dlx[1] = balance * cos(rot[1]) * sin(rot[2]);
	dlx[2] = -balance * sin(rot[1]);
	
	f64 dly[3];
	dly[0] = balance * sin(rot[0]) * sin(rot[1]) * cos(rot[2]) - balance * cos(rot[0]) * sin(rot[2]);
	dly[1] = balance * sin(rot[0]) * sin(rot[1]) * sin(rot[2]) + balance * cos(rot[0]) * cos(rot[2]);
	dly[2] = balance * sin(rot[0]) * cos(rot[1]);
	
	f64 dlz[3];
	dlz[0] = balance * cos(rot[0]) * sin(rot[1]) * cos(rot[2]) + balance * sin(rot[0]) * sin(rot[2]);
	dlz[1] = balance * cos(rot[0]) * sin(rot[1]) * sin(rot[2]) - balance * sin(rot[0]) * cos(rot[2]);
	dlz[2] = balance * cos(rot[0]) * cos(rot[1]);
	
	// f = (a-b)^2
	// df/db = -2(a-b)*Db
	
	f64 diff[3];
	diff[0] = d1[0] - d2d[0];
	diff[1] = d1[1] - d2d[1];
	diff[2] = d1[2] - d2d[2];
	
	f64 tmp1 = diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2];
	
	value += tmp1;
	
	if (gradient)
	{
		dloc[0] -= 2.0 * diff[0] * dlx[0];
		dloc[0] -= 2.0 * diff[1] * dlx[1];
		dloc[0] -= 2.0 * diff[2] * dlx[2];
		
		dloc[1] -= 2.0 * diff[0] * dly[0];
		dloc[1] -= 2.0 * diff[1] * dly[1];
		dloc[1] -= 2.0 * diff[2] * dly[2];
		
		dloc[2] -= 2.0 * diff[0] * dlz[0];
		dloc[2] -= 2.0 * diff[1] * dlz[1];
		dloc[2] -= 2.0 * diff[2] * dlz[2];
		
		drot[0] -= 2.0 * diff[0] * drx[0];
		drot[0] -= 2.0 * diff[1] * drx[1];
		drot[0] -= 2.0 * diff[2] * drx[2];
		
		drot[1] -= 2.0 * diff[0] * dry[0];
		drot[1] -= 2.0 * diff[1] * dry[1];
		drot[1] -= 2.0 * diff[2] * dry[2];
		
		drot[2] -= 2.0 * diff[0] * drz[0];
		drot[2] -= 2.0 * diff[1] * drz[1];
		drot[2] -= 2.0 * diff[2] * drz[2];
	}
	
	if (result != NULL)
	{
		result[0] = d2d[0];
		result[1] = d2d[1];
		result[2] = d2d[2];
	}
	
	counter++;
}

f64 superimpose::GetValue(void)
{
	value = 0.0; counter = 0;
	
	for (iter_al it1 = mdl->GetAtomsBegin();it1 != mdl->GetAtomsEnd();it1++)
	{
		// skip the atoms with ATOMFLAG_IS_HIDDEN set, because those are not displayed either and might have invalid coordinates!!!
		if ((* it1).flags & ATOMFLAG_IS_HIDDEN) continue;
		
		// also skip the solvent atoms ; 20050120
		if ((* it1).flags & ATOMFLAG_IS_SOLVENT_ATOM) continue;
		
		const fGL * crd1 = (* it1).GetCRD(index[0]);
		f64 d1[3] = { crd1[0], crd1[1], crd1[2] };
		
		const fGL * crd2 = (* it1).GetCRD(index[1]);
		f64 d2[3] = { crd2[0], crd2[1], crd2[2] };
		
		Compare(d1, d2, false);
	}
	
	return value;
}

f64 superimpose::GetGradient(void)
{
	value = 0.0; counter = 0;
	dloc[0] = dloc[1] = dloc[2] = 0.0;
	drot[0] = drot[1] = drot[2] = 0.0;
	
	for (iter_al it1 = mdl->GetAtomsBegin();it1 != mdl->GetAtomsEnd();it1++)
	{
		// skip the atoms with ATOMFLAG_IS_HIDDEN set, because those are not displayed either and might have invalid coordinates!!!
		if ((* it1).flags & ATOMFLAG_IS_HIDDEN) continue;
		
		// also skip the solvent atoms ; 20050120
		if ((* it1).flags & ATOMFLAG_IS_SOLVENT_ATOM) continue;
		
		const fGL * crd1 = (* it1).GetCRD(index[0]);
		f64 d1[3] = { crd1[0], crd1[1], crd1[2] };
		
		const fGL * crd2 = (* it1).GetCRD(index[1]);
		f64 d2[3] = { crd2[0], crd2[1], crd2[2] };
		
		Compare(d1, d2, true);
	}
	
	return value;
}

void superimpose::Transform(void)
{
	value = 0.0; counter = 0;
	
	for (iter_al it1 = mdl->GetAtomsBegin();it1 != mdl->GetAtomsEnd();it1++)
	{
		// transform all atoms, no matter what flags are set...
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		
		const fGL * crd1 = (* it1).GetCRD(index[0]);
		f64 d1[3] = { crd1[0], crd1[1], crd1[2] };
		
		const fGL * crd2 = (* it1).GetCRD(index[1]);
		f64 d2[3] = { crd2[0], crd2[1], crd2[2] };
		
		f64 d3[3];
		Compare(d1, d2, false, d3);
		
		(* it1).SetCRD(index[1], d3[0], d3[1], d3[2]);
	}
}

/*################################################################################################*/

void TransformVector(v3d<fGL> & p1, const fGL * p2)
{
	i32s n1; i32s n2; fGL pv[4]; pv[3] = 1.0;
	for (n1 = 0;n1 < 3;n1++) pv[n1] = p1.data[n1];
	
	fGL rv[4] = { 0.0, 0.0, 0.0, 0.0 };
	for (n1 = 0;n1 < 4;n1++) for (n2 = 0;n2 < 4;n2++) rv[n1] += pv[n2] * p2[n2 * 4 + n1];
	for (n1 = 0;n1 < 3;n1++) p1.data[n1] = rv[n1] / rv[3];
}

/*################################################################################################*/

#define HBOND_TRESHOLD -1.0	// h-bond energy treshold kcal/mol (K&S used -0.5 kcal/mol?)

#define HELIX_CHECK (M_PI * 45.0 / 180.0)	// geometry tresholds for helix torsions...
#define HELIX4_REF (M_PI * +50.3 / 180.0)

void DefineSecondaryStructure(model * mdl)
{
	vector<chn_info> & ci_vector = (* mdl->ref_civ);
	
	for (i32u n1 = 0;n1 < ci_vector.size();n1++)
	{
		if (ci_vector[n1].type != chn_info::amino_acid) continue;
		
		if (ci_vector[n1].ss_state != NULL) delete[] ci_vector[n1].ss_state;
		ci_vector[n1].ss_state = new char[ci_vector[n1].length + 1];
		
		f64 * energy = new f64[ci_vector[n1].length];
		
		ci_vector[n1].ss_state[ci_vector[n1].length] = 0;	// make it a null-terminated string for convenience...
		for (i32s n2 = 0;n2 < ci_vector[n1].length;n2++)
		{
			ci_vector[n1].ss_state[n2] = '.';
			energy[n2] = HBOND_TRESHOLD;
		}
		
		// the 3-turns and 5-turns are disabled (so far) ; 20050527
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		
	/*	for (i32s n2 = 0;n2 < ci_vector[n1].length - 3;n2++)		// 3-turns...
		{
			i32s tmp1[2] = { n1, n2 };
			i32s tmp2[2] = { n1, n2 + 3 };
			
			f64 tmp3 = HBondEnergy(mdl, tmp1, tmp2);
			if (tmp3 < energy[n2])
			{
				ci_vector[n1].ss_state[n2] = '3';
				energy[n2] = tmp3;
			}
		}	*/
		
		for (/*i32s*/ n2 = 0;n2 < ci_vector[n1].length - 4;n2++)		// 4-turns...
		{
			i32s tmp1[2] = { n1, n2 };
			i32s tmp2[2] = { n1, n2 + 4 };
			
			f64 tmp3 = HBondEnergy(mdl, tmp1, tmp2);
			if (tmp3 < energy[n2])
			{
				bool tor_check = true;
	i32s tc1[4] = { n2+0, n2+1, n2+2, n2+3 }; if (!TorsionCheck(mdl, n1, tc1, HELIX4_REF)) tor_check = false;
	i32s tc2[4] = { n2+1, n2+2, n2+3, n2+4 }; if (!TorsionCheck(mdl, n1, tc2, HELIX4_REF)) tor_check = false;
				
				if (tor_check)
				{
					ci_vector[n1].ss_state[n2] = '4';
					energy[n2] = tmp3;
				}
			}
		}
		
	/*	for (i32s n2 = 0;n2 < ci_vector[n1].length - 5;n2++)		// 5-turns...
		{
			i32s tmp1[2] = { n1, n2 };
			i32s tmp2[2] = { n1, n2 + 5 };
			
			f64 tmp3 = HBondEnergy(mdl, tmp1, tmp2);
			if (tmp3 < energy[n2])
			{
				ci_vector[n1].ss_state[n2] = '5';
				energy[n2] = tmp3;
			}
		}	*/
		
		delete[] energy;
		
		cout << "looking for intrachain strands for chain " << (n1 + 1) << endl;
		
		for (/*i32s*/ n2 = 1;n2 < ci_vector[n1].length - 4;n2++)
		{
			cout << "." << flush;
			
			for (i32s n3 = n2 + 3;n3 < ci_vector[n1].length - 1;n3++)
			{
				i32s t1a[2] = { n1, n2 - 1 }; i32s t1b[2] = { n1, n3 };
				f64 t1c = HBondEnergy(mdl, t1a, t1b);
				
				i32s t2a[2] = { n1, n3 }; i32s t2b[2] = { n1, n2 + 1 };
				f64 t2c = HBondEnergy(mdl, t2a, t2b);
				
				i32s t3a[2] = { n1, n3 - 1 }; i32s t3b[2] = { n1, n2 };
				f64 t3c = HBondEnergy(mdl, t3a, t3b);
				
				i32s t4a[2] = { n1, n2 }; i32s t4b[2] = { n1, n3 + 1 };
				f64 t4c = HBondEnergy(mdl, t4a, t4b);
				
				i32s t5a[2] = { n1, n2 }; i32s t5b[2] = { n1, n3 };
				f64 t5c = HBondEnergy(mdl, t5a, t5b);
				
				i32s t6a[2] = { n1, n3 }; i32s t6b[2] = { n1, n2 };
				f64 t6c = HBondEnergy(mdl, t6a, t6b);
				
				i32s t7a[2] = { n1, n2 - 1 }; i32s t7b[2] = { n1, n3 + 1 };
				f64 t7c = HBondEnergy(mdl, t7a, t7b);
				
				i32s t8a[2] = { n1, n3 - 1 }; i32s t8b[2] = { n1, n2 + 1 };
				f64 t8c = HBondEnergy(mdl, t8a, t8b);
				
				bool bridge = false;
				if (t1c < HBOND_TRESHOLD && t2c < HBOND_TRESHOLD) bridge = true;	// parallel
				if (t3c < HBOND_TRESHOLD && t4c < HBOND_TRESHOLD) bridge = true;	// parallel
				if (t5c < HBOND_TRESHOLD && t6c < HBOND_TRESHOLD) bridge = true;	// antiparallel
				if (t7c < HBOND_TRESHOLD && t8c < HBOND_TRESHOLD) bridge = true;	// antiparallel
				
				if (bridge)
				{
					ci_vector[n1].ss_state[n2] = 'S';
					ci_vector[n1].ss_state[n3] = 'S';
				}
			}
		}
		
		cout << endl;
	}
	
	cout << "looking for interchain strands";
	
	{
	for (i32s n1 = 0;n1 < ((i32s) ci_vector.size()) - 1;n1++)
	{
		for (i32s n2 = n1 + 1;n2 < (i32s) ci_vector.size();n2++)
		{
			cout << "." << flush;
			
			for (i32s n3 = 1;n3 < ci_vector[n1].length - 1;n3++)
			{
				for (i32s n4 = 1;n4 < ci_vector[n2].length - 1;n4++)
				{
					i32s t1a[2] = { n1, n3 - 1 }; i32s t1b[2] = { n2, n4 };
					f64 t1c = HBondEnergy(mdl, t1a, t1b);
					
					i32s t2a[2] = { n2, n4 }; i32s t2b[2] = { n1, n3 + 1 };
					f64 t2c = HBondEnergy(mdl, t2a, t2b);
					
					i32s t3a[2] = { n2, n4 - 1 }; i32s t3b[2] = { n1, n3 };
					f64 t3c = HBondEnergy(mdl, t3a, t3b);
					
					i32s t4a[2] = { n1, n3 }; i32s t4b[2] = { n2, n4 + 1 };
					f64 t4c = HBondEnergy(mdl, t4a, t4b);
					
					i32s t5a[2] = { n1, n3 }; i32s t5b[2] = { n2, n4 };
					f64 t5c = HBondEnergy(mdl, t5a, t5b);
					
					i32s t6a[2] = { n2, n4 }; i32s t6b[2] = { n1, n3 };
					f64 t6c = HBondEnergy(mdl, t6a, t6b);
					
					i32s t7a[2] = { n1, n3 - 1 }; i32s t7b[2] = { n2, n4 + 1 };
					f64 t7c = HBondEnergy(mdl, t7a, t7b);
					
					i32s t8a[2] = { n2, n4 - 1 }; i32s t8b[2] = { n1, n3 + 1 };
					f64 t8c = HBondEnergy(mdl, t8a, t8b);
					
					bool bridge = false;
					if (t1c < HBOND_TRESHOLD && t2c < HBOND_TRESHOLD) bridge = true;	// see above???
					if (t3c < HBOND_TRESHOLD && t4c < HBOND_TRESHOLD) bridge = true;	// see above???
					if (t5c < HBOND_TRESHOLD && t6c < HBOND_TRESHOLD) bridge = true;	// see above???
					if (t7c < HBOND_TRESHOLD && t8c < HBOND_TRESHOLD) bridge = true;	// see above???
					
					if (bridge)
					{
						ci_vector[n1].ss_state[n3] = 'S';
						ci_vector[n2].ss_state[n4] = 'S';
					}
				}
			}
		}
	}
	}
	cout << endl;
	
	for (/*i32u*/ n1 = 0;n1 < ci_vector.size();n1++)
	{
		cout << "// chain " << n1 << ":" << endl;
		
		i32u length = strlen(ci_vector[n1].ss_state);
		for (i32u n2 = 0;n2 < length;n2++)
		{
			cout << ci_vector[n1].ss_state[n2];
			
			bool is_break = !((n2 + 1) % 50);
			bool is_end = ((n2 + 1) == length);
			
			if (is_break || is_end) cout << endl;
		}
		
		cout << endl;
	}
	
	cout << "DefineSecondaryStructure() is ready." << endl;
}

// here can be some problems with the carboxyl terminus where
// we can accidentally pick "wrong" oxygen...

// for simplified residues, no changes are needed here?!?!?!
// in sf cases 0.0 is just returned since no O/N atoms are found.

f64 HBondEnergy(model * mdl, i32s * res1, i32s * res2)
{
	vector<chn_info> & ci_vector = (* mdl->ref_civ);
	if (ci_vector[res2[0]].sequence[res2[1]] == 'P') return 0.0;	// handle the PRO case...
	
	iter_al r1a[2]; mdl->GetRange(1, res1[0], r1a);
	iter_al r1b[2]; mdl->GetRange(2, r1a, res1[1], r1b);
	
	iter_al r2a[2]; mdl->GetRange(1, res2[0], r2a);
	iter_al r2b[2]; mdl->GetRange(2, r2a, res2[1], r2b);
	
	iter_al it_o = r1b[0];
	while (it_o != r1b[1] && ((* it_o).builder_res_id & 0xFF) != 0x10) it_o++;
	if (it_o == r1b[1]) return 0.0;
	
	iter_al it_n = r2b[0];
	while (it_n != r2b[1] && ((* it_n).builder_res_id & 0xFF) != 0x00) it_n++;
	if (it_n == r2b[1]) return 0.0;
	
	// if the distance between the residues is large enough we can be sure that there
	// is no h-bond and skip this time-consumig process. according to K&S this treshold
	// NO-distance is about 6.0 Å, but let's be careful and use a bit larger value...
	
	v3d<fGL> sv = v3d<fGL>((* it_o).GetCRD(0), (* it_n).GetCRD(0));
	if (sv.len() > 0.75) return 0.0;
	
// the above check is not enough to make this fast. problem is that the check is done in
// too late stage to be efficient. calculate first distances between all residues and
// use those results for sanity checking in earlier stage???

	iter_al it_c = r1b[0];
	while (it_c != r1b[1] && ((* it_c).builder_res_id & 0xFF) != 0x02) it_c++;
	if (it_c == r1b[1]) return 0.0;
	
	atom * ref_h = NULL;
	list<crec>::iterator it1 = (* it_n).cr_list.begin();
/*	while (it1 != (* it_n).cr_list.end() && ref_h == NULL)
	{
		if ((* it1).atmr->el.GetAtomicNumber() != 1) it1++;
		else ref_h = (* it1).atmr;
	}	*/
	
	// the sequence builder currently ignores hydrogen atoms, so those
	// are not always available. also PRO does not have the proton (see ABOVE).
	
	// therefore we don't use the real hydrogens here at all, but we "reconstruct"
	// the amide hydrogen using the main-chain heavy atoms...
	
it1 = (* it_n).cr_list.end();	// pretend that we couldn't find the amide hydrogen...
	if (it1 == (* it_n).cr_list.end())
	{
		if (!res2[1]) return 0.0;	// skip if N-terminal residue...
		iter_al r2c[2]; mdl->GetRange(2, r2a, res2[1] - 1, r2c);
		
		iter_al it_c1 = r2b[0];
		while (it_c1 != r2b[1] && ((* it_c1).builder_res_id & 0xFF) != 0x01) it_c1++;
		if (it_c1 == r2b[1]) return 0.0;
		
		iter_al it_c2 = r2c[0];
		while (it_c2 != r2c[1] && ((* it_c2).builder_res_id & 0xFF) != 0x02) it_c2++;
		if (it_c2 == r2c[1]) return 0.0;
		
		v3d<fGL> v1 = v3d<fGL>((* it_n).GetCRD(0), (* it_c1).GetCRD(0));
		v3d<fGL> v2 = v3d<fGL>((* it_n).GetCRD(0), (* it_c2).GetCRD(0));
		v3d<fGL> v3 = v1.vpr(v2); v3d<fGL> v4 = v1.vpr(v3);
		
		const fGL len = 0.1024;
		const fGL ang = M_PI * 121.0 / 180.0;
		v1 = v1 * (len * cos(ang) / v1.len());
		v4 = v4 * (len * sin(ang) / v4.len());
		v3d<fGL> v5 = v3d<fGL>((* it_n).GetCRD(0)) + (v1 + v4);
		ref_h = new atom(element(1), v5.data, 1);
		
///////////////////////////////////////////////////////////////////////////
//v3d<fGL> tv1 = v3d<fGL>((* it_n).crd[0], v5.comp);
//cout << "the H bond length = " << tv1.len() << endl;
//cout << "the H bond angle = " << (tv1.ang(v2) * 180.0 / M_PI) << endl;
//i32s zzz; cin >> zzz;
///////////////////////////////////////////////////////////////////////////
	}
	
	v3d<fGL> on_v = v3d<fGL>((* it_o).GetCRD(0), (* it_n).GetCRD(0));
	v3d<fGL> ch_v = v3d<fGL>((* it_c).GetCRD(0), ref_h->GetCRD(0));
	
	v3d<fGL> oh_v = v3d<fGL>((* it_o).GetCRD(0), ref_h->GetCRD(0));
	v3d<fGL> cn_v = v3d<fGL>((* it_c).GetCRD(0), (* it_n).GetCRD(0));
	
	if (it1 == (* it_n).cr_list.end()) delete ref_h;
	
	f64 tmp1 = 1.0 / on_v.len() + 1.0 / ch_v.len();
	f64 tmp2 = 1.0 / oh_v.len() + 1.0 / cn_v.len();
	f64 tmp3 = 0.42 * 0.20 * (tmp1 - tmp2);
	return 0.1 * 332.0 * tmp3;
}

// for helices, simply checking the above test seem to give false positives.
// perform additional main-chain torsion checks here...

bool TorsionCheck(model * mdl, i32s chn, i32s * res, fGL ref)
{
	iter_al rc[2]; mdl->GetRange(1, chn, rc);
	iter_al rr1[2]; mdl->GetRange(2, rc, res[0], rr1);
	iter_al rr2[2]; mdl->GetRange(2, rc, res[1], rr2);
	iter_al rr3[2]; mdl->GetRange(2, rc, res[2], rr3);
	iter_al rr4[2]; mdl->GetRange(2, rc, res[3], rr4);
	
	iter_al it_c1 = rr1[0];
	while (it_c1 != rr1[1] && ((* it_c1).builder_res_id & 0xFF) != 0x02) it_c1++;
	if (it_c1 == rr1[1]) { cout << "TC ; c_alpha #1 not found!" << endl; exit(EXIT_FAILURE); }
	
	iter_al it_c2 = rr2[0];
	while (it_c2 != rr2[1] && ((* it_c2).builder_res_id & 0xFF) != 0x02) it_c2++;
	if (it_c2 == rr2[1]) { cout << "TC ; c_alpha #2 not found!" << endl; exit(EXIT_FAILURE); }
	
	iter_al it_c3 = rr3[0];
	while (it_c3 != rr3[1] && ((* it_c3).builder_res_id & 0xFF) != 0x02) it_c3++;
	if (it_c3 == rr3[1]) { cout << "TC ; c_alpha #3 not found!" << endl; exit(EXIT_FAILURE); }
	
	iter_al it_c4 = rr4[0];
	while (it_c4 != rr4[1] && ((* it_c4).builder_res_id & 0xFF) != 0x02) it_c4++;
	if (it_c4 == rr4[1]) { cout << "TC ; c_alpha #4 not found!" << endl; exit(EXIT_FAILURE); }
	
	v3d<fGL> v1 = v3d<fGL>((* it_c2).GetCRD(0), (* it_c1).GetCRD(0));
	v3d<fGL> v2 = v3d<fGL>((* it_c2).GetCRD(0), (* it_c3).GetCRD(0));
	v3d<fGL> v3 = v3d<fGL>((* it_c3).GetCRD(0), (* it_c4).GetCRD(0));
	fGL tor = v1.tor(v2, v3);
	
	fGL dev = tor - ref;
	if (dev > +M_PI) dev = 2.0 * M_PI - dev;
	else if (dev < -M_PI) dev = 2.0 * M_PI + dev;
	
	if (fabs(dev) < HELIX_CHECK) return true;
	else
	{
		cout << "HELIX CHECK FAILED : " << dev << endl;
	//	int pause; cin >> pause;
		
		return false;
	}
}

/*################################################################################################*/

// eof
