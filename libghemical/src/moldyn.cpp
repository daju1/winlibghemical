// MOLDYN.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "moldyn.h"

#include "atom.h"
#include "model.h"

#include "eng1_sf.h"	// the langevin stuff needs this...

#include <strstream>
using namespace std;

#define T_BUFF_SIZE	500

/*################################################################################################*/
moldyn_param::moldyn_param(setup * su)
{
	confirm = false;
	show_dialog = true;
	
	nsteps_h = 5000;
	nsteps_e = 5000;
	nsteps_s = 100000;
	temperature = 300.0;
	timestep = 0.5;
	constant_e = false;
	langevin = false;
	
	strcpy(filename, "untitled.traj");
	
	setup1_sf * susf = dynamic_cast<setup1_sf *>(su);
	if (susf != NULL) timestep = 5.0;			// override...

//		nsteps_s = 100000;
//		temperature = 200.0;
#if SNARJAD_TARGET_WORKING
	nsteps_h = 1000;
	nsteps_e = 2500;
	nsteps_s = 10000;
	constant_e = false;

#if SNARJAD_TARGET_WORKING_T_0
	constant_e = true;
	temperature = 0.0;
	nsteps_h = 0;
	nsteps_e = 0;
	nsteps_s = 10000;
#endif

	start_snarjad_vel[0] = 0.0;
	start_snarjad_vel[1] = 0.0;
	start_snarjad_vel[2] = -10.0;

	strcpy(filename, "target.traj");
#endif
	
#if PROBNIY_ATOM_WORKING
	temperature = 0.0;
	nsteps_h = 0;
	nsteps_e = 0;
	nsteps_s = 1000;
	constant_e = true;
	strcpy(filename, "prob.traj");
#endif
}
moldyn::moldyn(engine * p1, f64 p2)
{
	eng = p1;
	
	tstep1 = p2;						// [1.0E-15 s]
	tstep2 = tstep1 * tstep1;				// [1.0E-30 s]
	
	vel = new f64[eng->GetAtomCount() * 3];			// [1.0E+3 m/s]
	acc = new f64[eng->GetAtomCount() * 3];			// [1.0E+12 m/s^2]
	
	mass = new f64[eng->GetAtomCount()];
	
	locked = new char[eng->GetAtomCount()];
//	target = new char[eng->GetAtomCount()];
	
	step_counter = 0;
	
	atom ** glob_atmtab = eng->GetSetup()->GetAtoms();
	
	num_locked = 0; i32s counter = 0;
	while (counter < eng->GetAtomCount())
	{
		bool lflag = false;
		if (glob_atmtab[counter]->flags & ATOMFLAG_IS_LOCKED)
		{
			lflag = true;
			num_locked++;
		}
		
		mass[counter] = glob_atmtab[counter]->mass;
		mass[counter] *= 1.6605402e-27 * 6.0221367e+23;
		
		locked[counter] = lflag;
		
#if SNARJAD_TARGET_WORKING && !SNARJAD_TARGET_WORKING_T_0
		for (i32s n1 = 0;n1 < 3;n1++)
		{
			i32u rnum = rand(); 
			f64 r1 = ((f64) rnum / (f64) RAND_MAX) - 0.5;
			vel[counter * 3 + n1] = r1;
			acc[counter * 3 + n1] = 0.0;
		}
#else
		for (i32s n1 = 0;n1 < 3;n1++)
		{
			vel[counter * 3 + n1] = 0.0;
			acc[counter * 3 + n1] = 0.0;
		}
#endif
		
		counter++;
	}

	// the rest are just default values; can be modified...
	// the rest are just default values; can be modified...
	// the rest are just default values; can be modified...
	
	temperature = 300.0;
	
	temperature_coupling = 0.010;

#if SNARJAD_TARGET_WORKING
	n_snarjad = -1;
#endif
#if SNARJAD_TARGET_WORKING || PROBNIY_ATOM_WORKING
	SetTarget();
#endif
}

moldyn::~moldyn(void)
{
	delete[] vel;
	delete[] acc;
	
	delete[] mass;
	
	delete[] locked;
//	delete[] target;
}
#if SNARJAD_TARGET_WORKING
void moldyn::ShootSnarjad(i32s nAtom, f64* v, f64* c)
{
	atom ** glob_atmtab = eng->GetSetup()->GetAtoms();
	if (nAtom < eng->GetAtomCount())
	{
		if (locked[nAtom])
		{
			glob_atmtab[nAtom]->flags &= (~ATOMFLAG_IS_LOCKED);
			locked[nAtom] = false;
			num_locked--;
		}

		for (i32s n2 = 0;n2 < 3;n2++)
		{
			if (v)
			{
				vel[nAtom * 3 + n2] = v[n2];
			}
			if (c)
			{
				eng->crd[nAtom * 3 + n2] = c[n2];
			}
		}
		n_snarjad = nAtom;
	}
}

bool moldyn::GetSnarjadVelCrd(f64* v, f64* c, f64* f)
{
	if (this->n_snarjad >= 0 && this->n_snarjad < eng->GetAtomCount())
	{
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			if (v)
			{
				v[n2] = vel[this->n_snarjad * 3 + n2];
			}
			if (c)
			{
				c[n2] = eng->crd[this->n_snarjad * 3 + n2];
			}
			if (f)
			{
				f[n2] = eng->d1[this->n_snarjad * 3 + n2];
			}
		}
		return true;
	}
	return false;
}

#endif
#if SNARJAD_TARGET_WORKING || PROBNIY_ATOM_WORKING
void moldyn::LockAtom(i32s nAtom)
{
	atom ** glob_atmtab = eng->GetSetup()->GetAtoms();
	
	if (nAtom < eng->GetAtomCount())
	{
		bool lflag = false;
		glob_atmtab[nAtom]->flags |= ATOMFLAG_IS_LOCKED;
		if (glob_atmtab[nAtom]->flags & ATOMFLAG_IS_LOCKED)
		{
			lflag = true;
			num_locked++;
		}		
		locked[nAtom] = lflag;		
	}
}

void moldyn::SetTarget()
{
	num_target = 0;// i32s counter = 0;
//	while (counter < eng->GetAtomCount())
//	{
//		target[counter] = false;
//		counter++;
//	}
	this->m_target_list.resize(0);
}

void moldyn::SetTarget(vector<i32s>& target_list)
{
	SetTarget();
	for(vector<i32s>::iterator it = target_list.begin(); it != target_list.end(); it++)
	{
		if((*it) >=0 && (*it) < eng->GetAtomCount())
		{			
			num_target++;			
			//target[(*it)] = true;
			this->m_target_list.push_back(*it);
		}
	}
	target_distanse_matrix.resize(num_target);
	for(size_t i1 = 0; i1 < num_target; i1++)
	{
		target_distanse_matrix[i1].resize(num_target);
		for(size_t i2 = 0; i2 < num_target; i2++)
		{
			target_distanse_matrix[i1][i2] = 0.0;
		}
	}
	for (i32s n2 = 0;n2 < 3;n2++)
	{
		for(size_t i1 = 0; i1 < num_target; i1++)
		{
			for(size_t i2 = 0; i2 < num_target; i2++)
			{
				target_distanse_matrix[i1][i2] = 
					eng->crd[this->m_target_list[i2] * 3 + n2] -
					eng->crd[this->m_target_list[i1] * 3 + n2];

				printf("%c target_distanse_matrix[%d][%d] = %f\n", (char)('x' + n2), i1, i2, target_distanse_matrix[i1][i2]);
			}
		}
	}
}

void moldyn::ComputeTargetCrd()
{
	for (i32s n2 = 0;n2 < 3;n2++)
	{
		target_crd[n2] = 0.0;
#if 1
		for(size_t i1 = 0; i1 < num_target; i1++)
		{
			// здесь не учтены пока периодические условия
			// если атомы цели разрежутся периодической границей, 
			// возникнет ошибка
			target_crd[n2] += eng->crd[this->m_target_list[i1] * 3 + n2];
		}
#else
		for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
		{			
			if (target[n1])
			{
				// здесь не учтены пока периодические условия
				// если атомы цели разрежутся периодической границей, 
				// возникнет ошибка
				target_crd[n2] += eng->crd[n1 * 3 + n2];
			}
		}
#endif
	}
	if(num_target)
	{
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			target_crd[n2] /= num_target;
		}
	}
}
#endif /*SNARJAD_TARGET_WORKING || PROBNIY_ATOM_WORKING*/
#if PROBNIY_ATOM_WORKING

bool moldyn::GetProbAtomForce(f64* f)
{
	if (this->n_prob_atom >= 0 && this->n_prob_atom < eng->GetAtomCount())
	{
		for (i32s n2 = 0; n2 < 3; n2++)
		{
			if (f)
			{
				f[n2] = eng->d1[this->n_prob_atom * 3 + n2];
			}
		}
		return true;
	}
	return false;
}

void moldyn::SetProbAtom(i32s nAtom, f64* c)
{
	atom ** glob_atmtab = eng->GetSetup()->GetAtoms();
	if (nAtom < eng->GetAtomCount())
	{
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			if (c)
			{
				eng->crd[nAtom * 3 + n2] = c[n2];
			}
		}
		n_prob_atom = nAtom;
	}
}

#endif

void moldyn::TakeMDStep(bool enable_temperature_control)
{
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			f64 tmpA = acc[n1 * 3 + n2];
			f64 tmp1 = tstep1 * vel[n1 * 3 + n2] * 1.0e-3;
			f64 tmp2 = tstep2 * tmpA * 0.5e-9;
			
			if (locked[n1])
			{
				tmpA = 0.0;	// make sure that locked atoms remain locked!
				tmp1 = 0.0;	// make sure that locked atoms remain locked!
				tmp2 = 0.0;	// make sure that locked atoms remain locked!
				
				// the engine class really cannot compute and return zero forces
				// for the locked atoms.
				
				// then how to ensure translational invariance and stuff like that?
			}
			
			eng->crd[n1 * 3 + n2] += tmp1 + tmp2;
			
			vel[n1 * 3 + n2] += tstep1 * tmpA * 0.5e-6;
		}
	}
	
	eng->DoSHAKE();
	
	eng->Compute(1);
	epot = eng->energy;

	{	
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		if (locked[n1]) continue;
		//a = -F/m
		//dv = a*dt
		
		acc[n1 * 3 + 0] = -eng->d1[n1 * 3 + 0] / mass[n1];
		acc[n1 * 3 + 1] = -eng->d1[n1 * 3 + 1] / mass[n1];
		acc[n1 * 3 + 2] = -eng->d1[n1 * 3 + 2] / mass[n1];
		
		vel[n1 * 3 + 0] += tstep1 * acc[n1 * 3 + 0] * 0.5e-6;
		vel[n1 * 3 + 1] += tstep1 * acc[n1 * 3 + 1] * 0.5e-6;
		vel[n1 * 3 + 2] += tstep1 * acc[n1 * 3 + 2] * 0.5e-6;
	}
	}
	
	ekin = KineticEnergy();
	if (enable_temperature_control)
	{
		f64 delta = (temperature / ConvEKinTemp(ekin)) - 1.0;
		
		f64 tc = sqrt(1.0 + temperature_coupling * delta);
		SetEKin(ekin * tc);
	}
	
	step_counter++;
}

f64 moldyn::KineticEnergy(void)
{
	f64 energy = 0.0;
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		f64 sum = 0.0;
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			f64 tmp = vel[n1 * 3 + n2];
			sum += tmp * tmp;
		}
		
		energy += 500.0 * mass[n1] * sum;
	}
	
	return energy;
}

// <Ekin> = 3/2 * k * T				// <Ekin> = <0.5 * m * v^2) = average kinetic energy of a molecule.
// EKin / N = 3/2 * k * T			// Ekin = total kinetic energy, N = number of molecules.
// EKin / N = 3/2 * R * T			// if Ekin is expressed "per mole", change k to R.

f64 moldyn::ConvTempEKin(f64 p1)
{
	return (3.0 / 2.0) * p1 * ((eng->GetAtomCount() - num_locked) * 8.314510) / 1000.0;
}

f64 moldyn::ConvEKinTemp(f64 p1)
{
	return (2.0 / 3.0) * p1 * 1000.0 / ((eng->GetAtomCount() - num_locked) * 8.314510);
}

void moldyn::SetEKin(f64 p1)
{
	f64 tmp1 = p1 / KineticEnergy();
	f64 tmp2 = (tmp1 < 0.0 ? 0.0 : sqrt(tmp1));
	
	i32u tmp3 = eng->GetAtomCount() * 3;
	for (i32u n1 = 0;n1 < tmp3;n1++) vel[n1] *= tmp2;
}

/*################################################################################################*/

moldyn_langevin::moldyn_langevin(engine * p1, f64 p2) : moldyn(p1, p2)
{
	if (num_locked != 0)
	{
		cout << "locked atoms not supported in moldyn_langevin!" << endl;
		exit(EXIT_FAILURE);
	}
	
	eng1_sf * engsf = dynamic_cast<eng1_sf *>(eng);
	
	// at the moment this code is limited to SF implicit solvation motion damping...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	if (engsf == NULL) { cout << "engsf cast failed!" << endl; exit(EXIT_FAILURE); }
	if (!engsf->use_implicit_solvent) { cout << "engsf must use implicit solvation!" << endl; exit(EXIT_FAILURE); }
	
	langevin_r_forces = new f64[eng->GetAtomCount() * 3];
	
	langevin_weight = new f64[eng->GetAtomCount()];
	langevin_frict = new f64[eng->GetAtomCount()];
	
	i32s counter = 0;
	while (counter < eng->GetAtomCount())
	{
		langevin_weight[counter] = NOT_DEFINED;		// this is -1.0 -> disabled!!!
		langevin_frict[counter] = 0.0;
		
		counter++;
	}
	
	// the rest are just default values; can be modified...
	// the rest are just default values; can be modified...
	// the rest are just default values; can be modified...
	
// Pastor RW, Brooks BR, Szabo A : "An analysis of the accuracy of Langevin
// and molecular dynamics algorithm", Mol Phys, 65, 1409-1419 (1988)

// the above article says frict should be 50 ps^-1 for H2O???
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// not sure what it means but seems that the constant here should be much more than that...
// setting langevin_frict_fc = 4.0e-6 1/s will yield v_loss/stp = 1.0% ; roughly at range then.

//	langevin_frict_fc = 8.0e-6;	// see model::DoMolDyn() ; v_loss/stp = 2.0% ; Bfact ??? ; 20050124
	langevin_frict_fc = 16.0e-6;	// see model::DoMolDyn() ; v_loss/stp = 4.0% ; Bfact OK ; 20050124
	
	langevin_random_fc = 0.00;	// see model::DoMolDyn() ; 3450.0
	langevin_coupling = 0.00;	// see model::DoMolDyn() ; 0.0005
	
	for (i32s n1 = 0;n1 < engsf->GetSetup()->GetSFAtomCount() - engsf->num_solvent;n1++)
	{
		f64 weight = 1.0;	// make this depend on SA??? update dynamically below then.
		
		langevin_weight[n1] = weight;
		langevin_frict[n1] = weight * langevin_frict_fc;
	}
}

moldyn_langevin::~moldyn_langevin(void)
{
	delete[] langevin_r_forces;
	
	delete[] langevin_weight;
	delete[] langevin_frict;
}

void moldyn_langevin::TakeMDStep(bool enable_temperature_control)
{
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			f64 tmp1 = tstep1 * vel[n1 * 3 + n2] * 1.0e-3;
			f64 tmp2 = tstep2 * acc[n1 * 3 + n2] * 0.5e-9;
			eng->crd[n1 * 3 + n2] += tmp1 + tmp2;
			
			vel[n1 * 3 + n2] += tstep1 * acc[n1 * 3 + n2] * 0.5e-6;
		}
	}
	
	eng->DoSHAKE();
	
	eng->Compute(1);
	epot = eng->energy;
	
// maintaining the correct temperature in langevin dynamics seems to be a bit problematic.
// constant energy simulations are very difficult, but it's quite easy to calculate the energy loss.

// the friction term will gain strength when T is increased, while the random term only depends on
// the force constant -> for each pair of friction/random settings, there is an equilibrium temperature
// that will be maintained without any heating.

// strategy: find friction/random settings with equilibrium T a bit below simulation temperature,
// and use mild temperature control to maintain the simulation temperature???
	
	{
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		if (langevin_weight[n1] >= 0.0)
		{
			// generate gaussian random numbers using a (modified???) Box-Muller transformation:
			// y = sqrt(-2*ln(x1))*cos(2*pi*x2)	// x1 and x2 = linear random numbers ]0..1].
			
			i32u rnum;	// return value 0 from rand() would lead to ln(0.0)...
			do { rnum = rand(); } while (rnum == 0); f64 r1 = (f64) rnum / (f64) RAND_MAX;
			do { rnum = rand(); } while (rnum == 0); f64 r2 = (f64) rnum / (f64) RAND_MAX;
			do { rnum = rand(); } while (rnum == 0); f64 r3 = (f64) rnum / (f64) RAND_MAX;
			
			f64 rA = sqrt(-2.0 * log(r1)) * cos(2.0 * M_PI * r2);
			f64 rB = sqrt(-2.0 * log(r2)) * cos(2.0 * M_PI * r3);
			f64 rC = sqrt(-2.0 * log(r3)) * cos(2.0 * M_PI * r1);
			
			langevin_r_forces[n1 * 3 + 0] = rA * langevin_weight[n1];
			langevin_r_forces[n1 * 3 + 1] = rB * langevin_weight[n1];
			langevin_r_forces[n1 * 3 + 2] = rC * langevin_weight[n1];
		}
	}
	}
	
	f64 net_random_x = 0.0;
	f64 net_random_y = 0.0;
	f64 net_random_z = 0.0;
	
	i32s langevin_counter = 0;
	{
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		if (langevin_weight[n1] >= 0.0)
		{
			net_random_x += langevin_r_forces[n1 * 3 + 0];
			net_random_y += langevin_r_forces[n1 * 3 + 1];
			net_random_z += langevin_r_forces[n1 * 3 + 2];
			
			langevin_counter++;
		}
	}
	}
	
	if (langevin_counter > 0)
	{
		net_random_x /= (f64) langevin_counter;
		net_random_y /= (f64) langevin_counter;
		net_random_z /= (f64) langevin_counter;
	}
	{
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		acc[n1 * 3 + 0] = -eng->d1[n1 * 3 + 0] / mass[n1];
		acc[n1 * 3 + 1] = -eng->d1[n1 * 3 + 1] / mass[n1];
		acc[n1 * 3 + 2] = -eng->d1[n1 * 3 + 2] / mass[n1];
		
		if (langevin_weight[n1] >= 0.0)
		{
			// ma = -grad - bmv + f(t)	;; b = frictional constant
			// a = -grad/m - bv + f(t)/m	;; b = frictional constant
			
			// the frictional constant b must have a unit 1/s ; [b] = 1/s
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			// is the effect independent of MD timestep??? should be...
			
			// friction...
			// friction...
			// friction...
			
			f64 frict_const = langevin_frict[n1] * 1.0e+9;
			acc[n1 * 3 + 0] -= frict_const * vel[n1 * 3 + 0];
			acc[n1 * 3 + 1] -= frict_const * vel[n1 * 3 + 1];
			acc[n1 * 3 + 2] -= frict_const * vel[n1 * 3 + 2];
			
//static int lcnt = 0; if (n1 == 15 && !(lcnt++ % 1000)) {
//cout << "ATOM " << n1 << " velocity losses = ";
//cout << (frict_const * vel[n1 * 3 + 0] * tstep1 * 0.5e-6) / vel[n1 * 3 + 0] << " ";
//cout << (frict_const * vel[n1 * 3 + 1] * tstep1 * 0.5e-6) / vel[n1 * 3 + 1] << " ";
//cout << (frict_const * vel[n1 * 3 + 2] * tstep1 * 0.5e-6) / vel[n1 * 3 + 2] << endl; }
			
			// random forces...
			// random forces... NOT WORKING CORRECTLY YET???
			// random forces...
			
			f64 random_fc = langevin_random_fc / (tstep1 * mass[n1]);
			acc[n1 * 3 + 0] += (langevin_r_forces[n1 * 3 + 0] - net_random_x) * random_fc;
			acc[n1 * 3 + 1] += (langevin_r_forces[n1 * 3 + 1] - net_random_y) * random_fc;
			acc[n1 * 3 + 2] += (langevin_r_forces[n1 * 3 + 2] - net_random_z) * random_fc;
		}
		
		vel[n1 * 3 + 0] += tstep1 * acc[n1 * 3 + 0] * 0.5e-6;
		vel[n1 * 3 + 1] += tstep1 * acc[n1 * 3 + 1] * 0.5e-6;
		vel[n1 * 3 + 2] += tstep1 * acc[n1 * 3 + 2] * 0.5e-6;
	}
	}
	
	ekin = KineticEnergy();
	if (enable_temperature_control)
	{
		f64 delta = (temperature / ConvEKinTemp(ekin)) - 1.0;
		
		f64 tc = sqrt(1.0 + temperature_coupling * delta);
		SetEKin(ekin * tc);
		
		if (langevin_coupling > 0.0)
		{
			f64 lc = sqrt(1.0 + langevin_coupling * delta);
			langevin_random_fc *= lc;
			
			if (!(step_counter%100))
			{
				cout << "langevin_random_fc = " << langevin_random_fc << endl;
			}
		}
	}
	
	step_counter++;
}

/*################################################################################################*/

// eof
