// POP_ANA.CPP

// Copyright (C) 2004 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "pop_ana.h"

/*################################################################################################*/

pop_ana::pop_ana(setup1_qm * p1)
{
	su = p1;
}

pop_ana::~pop_ana(void)
{
}

/*################################################################################################*/

pop_ana_electrostatic::pop_ana_electrostatic(setup1_qm * p1) : pop_ana(p1), conjugate_gradient(50, 0.001, 10.0)
{
	charge = NULL;
	dchg = NULL;
}

pop_ana_electrostatic::~pop_ana_electrostatic(void)
{
}

void pop_ana_electrostatic::DoPopAna(void)
{
	engine * eng = su->GetCurrentEngine();
	if (eng == NULL) su->CreateCurrentEngine();
	eng = su->GetCurrentEngine();
	if (eng == NULL) return;
	
	CopyCRD(su->GetModel(), eng, 0);
eng->Compute(0);//miksei vaikuta mit‰‰n???
	
	// create an array of grid points, and calculate ESP values.
	// OBS!!! radii for the atoms/shells are taken from element::vdwr[] table!!!
	// OBS!!! radii for the atoms/shells are taken from element::vdwr[] table!!!
	// OBS!!! radii for the atoms/shells are taken from element::vdwr[] table!!!
	
	if (data_vector.size() != 0) { cout << "ooops!!! data_vector not empty." << endl; }
	
	int atom_counter = 0;
	for (iter_al it1 = su->GetModel()->GetAtomsBegin();it1 != su->GetModel()->GetAtomsEnd();it1++)
	{
		const fGL * crd1 = (* it1).GetCRD(0);
		const fGL r1 = (* it1).el.GetVDWRadius();
		
		int point_counter = 0;
		for (i32s shell = 0;shell < 4;shell++)
		{
			f64 rr = NOT_DEFINED;
			switch (shell)
			{
				case 0:	rr = 1.4; break;
				case 1:	rr = 1.6; break;
				case 2:	rr = 1.8; break;
				case 3:	rr = 2.0; break;
			}
			
			if (rr == NOT_DEFINED)
			{
				cout << "bad rr!!!" << endl;
				exit(EXIT_FAILURE);
			}
			
			const fGL rsh1 = r1 * rr;
			
			fGL alpha = (0.1 / rsh1) * 2.0 * M_PI;				// longer-range limit...
			if (alpha > 20.0 * M_PI / 180.0) alpha = 20.0 * M_PI / 180.0;	// short-range limit!!!
			
			const int num1 = ((int) (M_PI / alpha)) + 1;
			const fGL d1 = M_PI / (fGL) num1;
			
			fGL a1 = 0.0;
			for (i32s n1 = 0;n1 < num1;n1++)
			{
				const int num2 = ((int) (sin(a1) * 2.0 * M_PI / alpha)) + 1;
				const fGL d2 = 2.0 * M_PI / (fGL) num2;
				
				fGL a2 = 0.0;
				for (i32s n2 = 0;n2 < num2;n2++)
				{
					fGL crd2[3];
					crd2[0] = rsh1 * sin(a1) * cos(a2);
					crd2[1] = rsh1 * sin(a1) * sin(a2);
					crd2[2] = rsh1 * cos(a1);
					
					crd2[0] += crd1[0];
					crd2[1] += crd1[1];
					crd2[2] += crd1[2];
					
					// test that none of the other atoms/shells overlap; if not, calculate and store data.
					
					bool skip = false;
					for (iter_al it5 = su->GetModel()->GetAtomsBegin();it5 != su->GetModel()->GetAtomsEnd();it5++)
					{
						if (it5 == it1) continue;
						
						const fGL * crd5 = (* it5).GetCRD(0);
						const fGL r5 = (* it5).el.GetVDWRadius();
						const fGL rsh5 = r5 * rr;
						
						v3d<fGL> v1(crd2, crd5);
						fGL dist = v1.len();
						
						if (dist < rsh5)
						{
							skip = true;
							break;
						}
					}
					
					if (!skip)
					{
						fGL dESP[3];
						fGL ESP = eng->GetESP(crd2, dESP);
						
						pop_ana_es_data newdata;
						
						newdata.x = crd2[0];
						newdata.y = crd2[1];
						newdata.z = crd2[2];
						newdata.ESP = ESP;
						
						data_vector.push_back(newdata);
						point_counter++;
					}
					
					a2 += d2;	// update angles...
				}
				
				a1 += d1;	// update angles...
			}
		}
		
		cout << "calculated " << point_counter << " data points for atom " << atom_counter++ << "." << endl;
	}
	
	// then do the fitting...
	
	charge = new f64[atom_counter];
	dchg = new f64[atom_counter];
	
	for (i32s n1 = 0;n1 < atom_counter;n1++)
	{
		charge[n1] = su->GetModel()->GetQMTotalCharge() / (fGL) atom_counter;
		dchg[n1] = 0.0;
		
		AddVar(& charge[n1], & dchg[n1]);
	}
	
//Check(1);return;
	for (i32s n1 = 0;n1 < 250;n1++)
	{
		TakeCGStep(conjugate_gradient::Newton2An);
		
		cout << "step = " << n1 << " ";
		cout << "value = " << optval << " ";
		cout << "(optstp = " << optstp << ") ";
		cout << endl;
	}
	
	// set the values...
	
	atom_counter = 0;
	for (iter_al it1 = su->GetModel()->GetAtomsBegin();it1 != su->GetModel()->GetAtomsEnd();it1++)
	{
		(* it1).charge = charge[atom_counter++];
	}
	
	delete[] charge;
	delete[] dchg;
}

void pop_ana_electrostatic::Check(i32s)
{
	const f64 delta = 0.0001;	// the finite difference step...
	f64 tmp1 = GetGradient();
	
	for (i32s n1 = 0;n1 < su->GetQMAtomCount();n1++)
	{
		f64 old = charge[n1];
		charge[n1] = old + delta;
		f64 tmp2 = (GetValue() - tmp1) / delta;
		charge[n1] = old;
		
		cout << "var " << n1 << " : ";
		cout << "a = " << dchg[n1] << " ";
		cout << "n = " << tmp2 << endl;
	}
}

f64 pop_ana_electrostatic::GetValue(void)
{
	Calculate(0);
	return value;
}

f64 pop_ana_electrostatic::GetGradient(void)
{
	Calculate(1);
	return value;
}

void pop_ana_electrostatic::Calculate(i32s p1)
{
	value = 0.0;
	
	if (p1)
	{
		for (i32s n1 = 0;n1 < su->GetQMAtomCount();n1++)
		{
			dchg[n1] = 0.0;
		}
	}
	
	atom ** atmtab = su->GetQMAtoms();
	for (i32u n1 = 0;n1 < data_vector.size();n1++)
	{
		fGL pos[3] = { data_vector[n1].x, data_vector[n1].y, data_vector[n1].z };
		fGL ESP_qm = data_vector[n1].ESP;
		
		fGL ESP_mm = 0.0;
		for (i32s n2 = 0;n2 < su->GetQMAtomCount();n2++)
		{
			const fGL nc = (4.1868 * 33.20716);
			const fGL * crd = atmtab[n2]->GetCRD(0);
			v3d<fGL> v1(pos, crd);
			
			ESP_mm += nc * charge[n2] / v1.len();
		}
		
		f64 delta = ESP_mm - ESP_qm;
		value += delta * delta;
		
		if (p1)
		{
			fGL dvalue = 2.0 * delta;
			for (i32s n2 = 0;n2 < su->GetQMAtomCount();n2++)
			{
				const fGL nc = (4.1868 * 33.20716);
				const fGL * crd = atmtab[n2]->GetCRD(0);
				v3d<fGL> v1(pos, crd);
				
				dchg[n2] += dvalue * nc / v1.len();
			}
		}
	}
}

/*################################################################################################*/

// eof
