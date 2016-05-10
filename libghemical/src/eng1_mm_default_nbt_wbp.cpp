// ENG1_MM_DEFAULT_NBT_WBP.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "eng1_mm_default.h"

#include "eng1_mm.h"
#include "tab_mm_default.h"

#include <algorithm>
#include <strstream>
using namespace std;

/*################################################################################################*/
eng1_mm_default_nbt_wbp::eng1_mm_default_nbt_wbp(setup * p1, i32u p2) : engine(p1, p2), eng1_mm(p1, p2), engine_wbp(p1, p2), engine_pbc(p1, p2)
{
//	atom ** atmtab = GetSetup()->GetMMAtoms();
//	bond ** bndtab = GetSetup()->GetMMBonds();
	
	ostream * ostr = default_tables::GetInstance()->ostr;

#if WBP_COPIED_FROM_MIM

	fGL mindim = GetSetup()->GetModel()->periodic_box_HALFdim[0];
	if (GetSetup()->GetModel()->periodic_box_HALFdim[1] < mindim) mindim = GetSetup()->GetModel()->periodic_box_HALFdim[1];
	if (GetSetup()->GetModel()->periodic_box_HALFdim[2] < mindim) mindim = GetSetup()->GetModel()->periodic_box_HALFdim[2];
	
	sw1 = 0.6; if (sw1 < (mindim - 0.4)) sw1 = mindim - 0.4;	// will trigger if boxdim < 2.0 nm!!!
	sw2 = shft1 = mindim - 0.2;
	
	limit = mindim;
	update = true;
	
	// calculate the actual values...
	
	sw1 = sw1 * sw1;
	sw2 = sw2 * sw2;
	
	swA = 3.0 * sw1;
	swB = pow(sw2 - sw1, 3.0);
	
	shft3 = pow(shft1, 3.0);
	
	limit = limit * limit;
#else
	update = true;
#endif
	nbt1_vector.reserve(250000);
	
	// also check engine::bp_center!!!
	// also check engine::bp_center!!!
	// also check engine::bp_center!!!
	
	use_upp_wall = true; 
	use_down_wall = true; 

#if SEVERAL_WBP
	//если второй индекс равен нулю, то стенка является абсолютно прозрачной для атомов растворителя
	mN2.insert(map<int,int>::value_type(1,0));//индекс стенки = 1 for Y!!!
	mN2.insert(map<int,int>::value_type(2,0));//индекс стенки = 2 for Z!!!
#else
	N2 = 2;//индекс стенки = 2 for Z!!!
	N2 = 1;//индекс стенки = 1 for Y!!!
#endif
	
#if SEVERAL_WBP
	for (map<int,int>::iterator itm = mN2.begin(); 
		itm != mN2.end(); itm++)
	{
		m_bp_wall_crd.insert(
			map<int,f64>::value_type(
			(*itm).first, 
			GetSetup()->GetModel()->periodic_box_HALFdim[(*itm).first]));//координата стенки
	}
#else
	bp_wall_crd = GetSetup()->GetModel()->periodic_box_HALFdim[N2]; //координата стенки
#endif
	bp_fc_wall = 12500.0;	// силовая константа взаимодействия со стенкой
	bp_fc_wall = 5000.0;	// силовая константа взаимодействия со стенкой
	//bp_fc_solute = 5000.0;		// 50 kJ/(mol*Е^2) = 5000 kJ/(mol*(nm)^2)
	//bp_fc_solvent = 12500.0;	// 125 kJ/(mol*Е^2) = 12500 kJ/(mol*(nm)^2)

	if (ostr != NULL && (use_upp_wall || use_down_wall))
	{
		(* ostr) << "use_upp_wall || use_down_wall ; ";
//		(* ostr) << bp_wall_crd << " " << bp_wall_crd << " ; ";
		(* ostr) << endl;
	}
}

eng1_mm_default_nbt_wbp::~eng1_mm_default_nbt_wbp(void)
{
}

void eng1_mm_default_nbt_wbp::ComputeNBT1(i32u p1)
{
	energy_nbt1a = 0.0;
	energy_nbt1b = 0.0;
	energy_nbt1c = 0.0;
	energy_nbt1d = 0.0;
	
	atom ** atmtab = GetSetup()->GetMMAtoms();
	
	// an additional pass for the boundary potential (optional).
	// an additional pass for the boundary potential (optional).
	// an additional pass for the boundary potential (optional).
#if SEVERAL_WBP
	for (map<int,int>::iterator itm = mN2.begin(); 
		itm != mN2.end(); itm++)
	{
		int N2 = (*itm).first;    
		int free_solvent = (*itm).second;    
		f64 bp_wall_crd;
		//GetSetup()->GetModel()->periodic_box_HALFdim[(*itm).second];//координата стенки
#endif	
	bp_wall_crd = GetSetup()->GetModel()->periodic_box_HALFdim[N2]; //координата стенки

	if (use_down_wall)
	{
		for (i32s n1 = 0;n1 < GetSetup()->GetMMAtomCount();n1++)
		{
#if SEVERAL_WBP
			if (free_solvent && atmtab[n1]->flags & ATOMFLAG_IS_SOLVENT_ATOM)
				continue;
#endif	

			//f64 radius = bp_radius_solute;
			f64 fc = bp_fc_wall;
			
			/*if (atmtab[n1]->flags & ATOMFLAG_IS_SOLVENT_ATOM)
			{
				radius = bp_radius_solvent;
				fc = bp_fc_solvent;
			}*/
			
			/*f64 t1a[3]; f64 t1b = 0.0;
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t9a = bp_center[n2];
				f64 t9b = crd[l2g_mm[n1] * 3 + n2];
				
				t1a[n2] = t9a - t9b;
				t1b += t1a[n2] * t1a[n2];
			}
			
			f64 t1c = sqrt(t1b);*/
			f64 t1c = crd[l2g_mm[n1] * 3 + N2];
			
			if (t1c > - bp_wall_crd) continue;
			
			// f = a(x-b)^2
			// df/dx = 2a(x-b)
			
			//глубина проникновения атома в зону действия потенциала стенки
			f64 t2a = - bp_wall_crd - t1c;
			//энергия на которую увеличилась потенци альная энергия атома
			f64 t2b = fc * t2a * t2a;
			
			energy_bt1 += t2b;
if (atmtab[n1]->flags & ATOMFLAG_IS_SOLVENT_ATOM) E_solvent += t2b; else E_solute += t2b;
			
			if (p1 > 0)
			{
				// сила, направленная от стенки
				f64 t2c = 2.0 * fc * t2a;
				
				i32s n2 = N2;
				//{
					// проекция силы на координатную ось
					f64 t2d = t2c;
					
					d1[l2g_mm[n1] * 3 + n2] -= t2d;
				//}
			}
		}
	}
	
	// the nonbonded terms begin...
	// the nonbonded terms begin...
	// the nonbonded terms begin...

	if (use_upp_wall)
	{
		for (i32s n1 = 0;n1 < GetSetup()->GetMMAtomCount();n1++)
		{
#if SEVERAL_WBP
			if (free_solvent && atmtab[n1]->flags & ATOMFLAG_IS_SOLVENT_ATOM)
				continue;
#endif
			//f64 radius = bp_radius_solute;
			f64 fc = bp_fc_wall;
			
			/*if (atmtab[n1]->flags & ATOMFLAG_IS_SOLVENT_ATOM)
			{
				radius = bp_radius_solvent;
				fc = bp_fc_solvent;
			}*/
			
			/*f64 t1a[3]; f64 t1b = 0.0;
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t9a = bp_center[n2];
				f64 t9b = crd[l2g_mm[n1] * 3 + n2];
				
				t1a[n2] = t9a - t9b;
				t1b += t1a[n2] * t1a[n2];
			}
			
			f64 t1c = sqrt(t1b);*/
			f64 t1c = crd[l2g_mm[n1] * 3 + N2];
			
			
			if (t1c < bp_wall_crd) continue;
			
			// f = a(x-b)^2
			// df/dx = 2a(x-b)
			
			//глубина проникновения атома в зону действия потенциала стенки
			f64 t2a = t1c - bp_wall_crd;
			//энергия на которую увеличилась потенци альная энергия атома
			f64 t2b = fc * t2a * t2a;
			
			energy_bt1 += t2b;
if (atmtab[n1]->flags & ATOMFLAG_IS_SOLVENT_ATOM) E_solvent += t2b; else E_solute += t2b;
			
			if (p1 > 0)
			{
				// сила, направленная от стенки
				f64 t2c = 2.0 * fc * t2a;
				
				i32s n2 = N2;
				//{
					// проекция силы на координатную ось
					f64 t2d = -t2c;
					
					d1[l2g_mm[n1] * 3 + n2] -= t2d;
				//}		

			}
		}
	}

#if SEVERAL_WBP
	}
#endif
	if (update) UpdateTerms();
	
	// the nonbonded terms begin...
	// the nonbonded terms begin...
	// the nonbonded terms begin...
	
	for (i32s n1 = 0;n1 < (i32s) nbt1_vector.size();n1++)
	{
		i32s * atmi = nbt1_vector[n1].atmi;
		
		f64 t1a[3]; f64 t1b = 0.0;
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			f64 t2a = crd[l2g_mm[atmi[0]] * 3 + n2];
			f64 t2b = crd[l2g_mm[atmi[1]] * 3 + n2];
			
			t1a[n2] = t2a - t2b;
			// периодические условия для тех измерений где нет стенки
#if SEVERAL_WBP
			map<int, int>::iterator found = mN2.find(n2);
			if (found == mN2.end())
#else
			if (n2 != N2)
#endif
			{
				if (t1a[n2] < -GetSetup()->GetModel()->periodic_box_HALFdim[n2])
				{
					t1a[n2] += 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n2];
				}
				else if (t1a[n2] > +GetSetup()->GetModel()->periodic_box_HALFdim[n2])
				{
					t1a[n2] -= 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n2];
				}
			}
			
			t1b += t1a[n2] * t1a[n2];
		}
		
		f64 t1c = sqrt(t1b);
		
		// f1 = (r/a)^-12 - (r/b)^-6
		// df1/dr = -12/a(r/a)^-13 + 6/b(r/b)^-7
		
		f64 t3a = t1c / nbt1_vector[n1].kr;
		f64 t3b = t1c / nbt1_vector[n1].kd;
		
		f64 t4a = t3a * t3a * t3a; f64 t4b = t4a * t4a; f64 t4c = t4b * t4b;	// ^3 ^6 ^12
		f64 t5a = t3b * t3b * t3b; f64 t5b = t5a * t5a;				// ^3 ^6
		
		f64 t6a = 1.0 / (t4c) - 1.0 / (t5b);
		
#if WBP_COPIED_FROM_MIM
		// s1 = (rE^2 - r^2)^2 * (rE^2 + 2r^2 - 3rB^2) / (rE^2 - rB^2)^3
		// ds1/dr = [this will yield 2 terms quite easily...]
		
		f64 t3x;		// value
		f64 t3y; f64 t3z;	// derivative
		if (t1b < sw1)
		{
			t3x = 1.0;
			t3y = t3z = 0.0;
		}
		else if (t1b > sw2)
		{
			t3x = 0.0;
			t3y = t3z = 0.0;
		}
		else
		{
			f64 t3c = sw2 - t1b; f64 t3d = t3c * t3c;
			f64 t3e = sw2 + 2.0 * t1b - swA;
			
			t3x = t3d * t3e / swB;
			t3y = 4.0 * t1c * t3d / swB;
			t3z = 4.0 * t1c * t3c * t3e / swB;
		}
		
		energy_nbt1a += t6a * t3x;
#else // copied from mbp
		energy_nbt1a += t6a;
#endif
		
		// f2 = Q/r
		// df2/dr = -Q/r^2
		
		f64 t6b = nbt1_vector[n1].qq / t1c;
		
#if WBP_COPIED_FROM_MIM
		// s2 = (1 - (r/rE)^3)^2
		// ds2/dr = -6r^2 * (1 - (r/rE)^3) / rE^3
		
		f64 t4x;		// value
		f64 t4y;		// derivative
		if (t1c > shft1)
		{
			t4x = 0.0;
			t4y = 0.0;
		}
		else
		{
			f64 t4d = t1b * t1c / shft3;
			f64 t4e = 1.0 - t4d;
			
			t4x = t4e * t4e;
			t4y = 6.0 * t1b * t4e / shft3;
		}
		
		energy_nbt1b += t6b * t4x;
		
		f64 tote = t6a * t3x + t6b * t4x;
#else // copied from mbp
		
		energy_nbt1b += t6b;
		
		f64 tote = t6a + t6b;
#endif

int class1 = (atmtab[atmi[0]]->flags & ATOMFLAG_IS_SOLVENT_ATOM);
int class2 = (atmtab[atmi[1]]->flags & ATOMFLAG_IS_SOLVENT_ATOM);
if (class1 == class2) { if (class1) E_solvent += tote; else E_solute += tote; }
else E_solusolv += tote;
		
#if !WBP_COPIED_FROM_MIM
		// copied from mbp
		if (p1 > 0)
		{
			f64 t7a = 12.0 / (nbt1_vector[n1].kr * t4c * t3a);
			f64 t7b = 6.0 / (nbt1_vector[n1].kd * t5b * t3b);
			
			f64 t8a = nbt1_vector[n1].qq / t1b;
			
			f64 t9a = t7b - t7a - t8a;
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t9b = (t1a[n2] / t1c) * t9a;
				
				d1[l2g_mm[atmi[0]] * 3 + n2] += t9b;
				d1[l2g_mm[atmi[1]] * 3 + n2] -= t9b;
			}
		}
#else // from mim
		if (p1 > 0)
		{
			f64 t7a = 12.0 / (nbt1_vector[n1].kr * t4c * t3a);
			f64 t7b = 6.0 / (nbt1_vector[n1].kd * t5b * t3b);
			
			f64 t8a = nbt1_vector[n1].qq / t1b;
			
			f64 t9a = (t7b - t7a) * t3x + t6a * (t3y - t3z);
			f64 t9b = t8a * t4x + t6b * t4y;
			
			f64 t9c = t9a - t9b;
			
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				f64 t9d = (t1a[n2] / t1c) * t9c;
				
				d1[l2g_mm[atmi[0]] * 3 + n2] += t9d;
				d1[l2g_mm[atmi[1]] * 3 + n2] -= t9d;
			}
		}
#endif
	}
}

void eng1_mm_default_nbt_wbp::UpdateTerms(void)
{
	atom ** atmtab = GetSetup()->GetMMAtoms();
//	bond ** bndtab = GetSetup()->GetMMBonds();
	
	
	ostream * ostr = default_tables::GetInstance()->ostr;
	
	update = false;
	nbt1_vector.resize(0);
	
	if (ostr != NULL) (* ostr) << "creating nbt1-terms: ";
	i32s nbt1_err = 0;
	
	for (i32s ind1 = 0;ind1 < GetSetup()->GetMMAtomCount() - 1;ind1++)
	{
		for (i32s ind2 = ind1 + 1;ind2 < GetSetup()->GetMMAtomCount();ind2++)
		{
			i32s test = range_cr1[ind1];
			while (test < range_cr1[ind1 + 1])
			{
				if (cr1[test] == atmtab[ind2]) break;
				else test++;
			}
			
			if (test == range_cr1[ind1 + 1])
			{
				f64 t1a; f64 t1b = 0.0;
				for (i32s n1 = 0;n1 < 3;n1++)
				{
					f64 t2a = crd[l2g_mm[ind1] * 3 + n1];
					f64 t2b = crd[l2g_mm[ind2] * 3 + n1];
					
					t1a = t2a - t2b;
					
#if SEVERAL_WBP
					map<int, int>::iterator found = mN2.find(n1);
					if (found == mN2.end())
#else
					if (n1 != N2)
#endif
					{
						if (t1a < -GetSetup()->GetModel()->periodic_box_HALFdim[n1])
						{
							t1a += 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n1];
						}
						else if (t1a > +GetSetup()->GetModel()->periodic_box_HALFdim[n1])
						{
							t1a -= 2.0 * GetSetup()->GetModel()->periodic_box_HALFdim[n1];
						}
					}
					
					t1b += t1a * t1a;
				}
				
#if WBP_COPIED_FROM_MIM
				if (t1b > limit) continue;
#endif
				
				test = range_cr2[ind1];
				while (test < range_cr2[ind1 + 1])
				{
					if (cr2[test] == atmtab[ind2]) break;
					else test++;
				}
				
				bool is14 = (test != range_cr2[ind1 + 1]);
				
				mm_default_nbt1 newnbt1;
				newnbt1.atmi[0] = ind1;
				newnbt1.atmi[1] = ind2;
				
				bool success = false;
				if (dynamic_cast<setup1_mm *>(GetSetup())->EnableExceptions())
				{
					success = default_tables::GetInstance()->e_Init(this, & newnbt1, is14);
				}
				else
				{
					// see also eng1_mm_default_nbt_mbp ctor ; should be the same!!!
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
					
					bool errors = false;
					const default_at * at;
					
					f64 r1 = 0.150; f64 e1 = 0.175;			// default...
					at = default_tables::GetInstance()->GetAtomType(atmtab[ind1]->atmtp);
					if (at != NULL) { r1 = at->vdw_R; e1 = at->vdw_E; }
					else errors = true;
					
					f64 r2 = 0.150; f64 e2 = 0.175;			// default...
					at = default_tables::GetInstance()->GetAtomType(atmtab[ind2]->atmtp);
					if (at != NULL) { r2 = at->vdw_R; e2 = at->vdw_E; }
					else errors = true;
					
					f64 optdist = r1 + r2;
					f64 energy = sqrt(e1 * e2);
					
					f64 charge1 = atmtab[ind1]->charge;
					f64 charge2 = atmtab[ind2]->charge;
					newnbt1.qq = 138.9354518 * charge1 * charge2;
					
					if (is14)
					{
						energy *= 0.5;
						newnbt1.qq *= 0.75;
					}
					
					f64 tmp1 = optdist * pow(1.0 * energy, 1.0 / 12.0);
					f64 tmp2 = optdist * pow(2.0 * energy, 1.0 / 6.0);
					
					newnbt1.kr = tmp1;
					newnbt1.kd = tmp2;
					
					if (!errors) success = true;
				}
				
				nbt1_err += !success;
				nbt1_vector.push_back(newnbt1);
			}
		}
	}
	
	// report possible errors...
	
	i32s total_err = nbt1_err;
	if (total_err)
	{
		char mbuff1[256];
		ostrstream str1(mbuff1, sizeof(mbuff1));
		str1 << "WARNING : there were " << total_err << " missing parameters in the nonbonded terms." << endl << ends;
		GetSetup()->GetModel()->PrintToLog(mbuff1);
	}
}
/*################################################################################################*/
