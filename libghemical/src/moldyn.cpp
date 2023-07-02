// MOLDYN.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include <strstream>
using namespace std;

#include "long_stream.h"
#include "moldyn.h"

#include "atom.h"
#include "model.h"

#include "eng1_sf.h"	// the langevin stuff needs this...


#define T_BUFF_SIZE	500
#define PI		3.14159265358979323846



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

	nsteps_s = 100000;
	temperature = 200.0;

}
moldyn::moldyn(engine * p1, f64 p2)
{
	eng = p1;
	
	tstep1 = p2;						// [1.0E-15 s]
	tstep2 = tstep1 * tstep1;				// [1.0E-30 s]
	
	vel = new f64[eng->GetAtomCount() * 3];			// [1.0E+3 m/s]
	acc = new f64[eng->GetAtomCount() * 3];			// [1.0E+12 m/s^2]

	cumsum_vel = new f64[eng->GetAtomCount() * 3];			// [1.0e+3 m/s]
	cumsum_acc = new f64[eng->GetAtomCount() * 3];			// [1.0e+12 m/s^2]
	cumsum_f   = new f64[eng->GetAtomCount() * 3];			// [1.0e+3 m/s]
	mass = new f64[eng->GetAtomCount()];
	
	locked = new char[eng->GetAtomCount()];
	
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
		
		for (i32s n1 = 0;n1 < 3;n1++)
		{
			vel[counter * 3 + n1] = 0.0;
			acc[counter * 3 + n1] = 0.0;

			cumsum_vel[counter * 3 + n1] = 0.0;
			cumsum_acc[counter * 3 + n1] = 0.0;
			cumsum_f  [counter * 3 + n1] = 0.0;
		}
		
		counter++;
	}
	
	// the rest are just default values; can be modified...
	// the rest are just default values; can be modified...
	// the rest are just default values; can be modified...
	
	temperature = 300.0;
	
	temperature_coupling = 0.010;
}

moldyn::~moldyn(void)
{
	delete[] vel;
	delete[] acc;
	
	delete[] cumsum_vel;
	delete[] cumsum_acc;
	delete[] cumsum_f;

	delete[] mass;
	
	delete[] locked;
}

void moldyn::TakeMDStep(bool enable_temperature_control)
{
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			f64 tmpA = acc[n1 * 3 + n2];

			f64 tmp1 = tstep1 * vel[n1 * 3 + n2] * 1.0e-3;
			f64 tmp2 = tstep2 * tmpA * 0.5e-9;

			if (!locked[n1])
			{
				eng->crd[n1 * 3 + n2] += tmp1 + tmp2;

				vel[n1 * 3 + n2] += tstep1 * tmpA * 0.5e-6;
			}
		}
	}
	
	eng->DoSHAKE();
	
	eng->Compute(1);
	epot = eng->energy;
	
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		if (locked[n1])
		{
			acc[n1 * 3 + 0] = acc[n1 * 3 + 1] = acc[n1 * 3 + 2] = 0.0;
			vel[n1 * 3 + 0] = vel[n1 * 3 + 1] = vel[n1 * 3 + 2] = 0.0;
		}
		else
		{
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				acc[n1 * 3 + n2] = -eng->d1[n1 * 3 + n2] / mass[n1];
				vel[n1 * 3 + n2] += tstep1 * acc[n1 * 3 + n2] * 0.5e-6;

				cumsum_vel[n1 * 3 + n2] += vel[n1 * 3 + n2];
				cumsum_acc[n1 * 3 + n2] += acc[n1 * 3 + n2];
			}
		}
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			cumsum_f[n1 * 3 + n2] += eng->d1[n1 * 3 + n2];
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


/*################################################################################################*/
moldyn_tst_param::moldyn_tst_param(setup * su)
{
	confirm = false;
	show_dialog = true;
	
	nsteps_h = 5000;
	nsteps_e = 5000;
	nsteps_s = 100000;
	maxwell_init_temperature = temperature = 300.0;
	timestep = 0.5;
	constant_e = false;
	langevin = false;
	recalc_box = false;
	box_optimization = false;
	maxwell_distribution_init = false;

	load_last_frame = false;
	//trajectory_version = 10;

	g[0] = 0.0; //Gx
	g[1] = 0.0; //Gy
	g[2] = 0.0; //Gz
	
	strcpy(filename_traj, "untitled.traj");
	strcpy(filename_input_frame, "input.frame");
	strcpy(filename_output_frame, "output.frame");
	
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

#if GRAVI_OSCILLATOR_WORKING 	
	//len_n_len_g = 0.1;
	//len_n_len_g = 0.33;
	//nsteps_h = 500;
	//nsteps_e = 500;
	//nsteps_s = 1000000;

	cout << "Enter nsteps_h(1000)\n";
	cin >> nsteps_h;
	cout << "Enter nsteps_e(1000)\n";
	cin >> nsteps_e;
	cout << "Enter nsteps_s(1000000)\n";
	cin >> nsteps_s;

	//temperature = 300.0;
	cout << "Enter temperature\n";
	cin >> temperature;
#if !GRAVI_OSCILLATOR_WORKING_LINEAR
	cout << "Enter len_n_len_g\n";
	cin >> len_n_len_g;
	sprintf(filename, "gravi_%0.2f_%0.0f.traj", len_n_len_g, temperature);
#else
	cout << "Enter k_lin_gravi\n";
	cin >> k_lin_gravi;
	sprintf(filename, "lin_gravi_%0.2f_%0.0f_%d.traj", 
		k_lin_gravi, 
		temperature, 
		nsteps_h + nsteps_e);
#endif

#endif
}
double invnormaldistribution(double y0);
#include<math.h>
#include<float.h>

double invnormaldistribution(double y0)
{
    double result;
    double expm2;
    double s2pi;
    double big;
    double x;
    double y;
    double z;
    double y2;
    double x0;
    double x1;
    int code;
    double p0;
    double q0;
    double p1;
    double q1;
    double p2;
    double q2;

    expm2 = 0.13533528323661269189;
    s2pi = 2.50662827463100050242;
    big = 10000000000000;
    if( y0<=0 )
    {
        result = -big;
        return result;
    }
    if( y0>=1 )
    {
        result = big;
        return result;
    }
    code = 1;
    y = y0;
    if( y>1.0-expm2 )
    {
        y = 1.0-y;
        code = 0;
    }
    if( y>expm2 )
    {
        y = y-0.5;
        y2 = y*y;
        p0 = -59.9633501014107895267;
        p0 = 98.0010754185999661536+y2*p0;
        p0 = -56.6762857469070293439+y2*p0;
        p0 = 13.9312609387279679503+y2*p0;
        p0 = -1.23916583867381258016+y2*p0;
        q0 = 1;
        q0 = 1.95448858338141759834+y2*q0;
        q0 = 4.67627912898881538453+y2*q0;
        q0 = 86.3602421390890590575+y2*q0;
        q0 = -225.462687854119370527+y2*q0;
        q0 = 200.260212380060660359+y2*q0;
        q0 = -82.0372256168333339912+y2*q0;
        q0 = 15.9056225126211695515+y2*q0;
        q0 = -1.18331621121330003142+y2*q0;
        x = y+y*y2*p0/q0;
        x = x*s2pi;
        result = x;
        return result;
    }
    x = sqrt(-2.0*log(y));
    x0 = x-log(x)/x;
    z = 1.0/x;
    if( x<8.0 )
    {
        p1 = 4.05544892305962419923;
        p1 = 31.5251094599893866154+z*p1;
        p1 = 57.1628192246421288162+z*p1;
        p1 = 44.0805073893200834700+z*p1;
        p1 = 14.6849561928858024014+z*p1;
        p1 = 2.18663306850790267539+z*p1;
        p1 = -1.40256079171354495875*0.1+z*p1;
        p1 = -3.50424626827848203418*0.01+z*p1;
        p1 = -8.57456785154685413611*0.0001+z*p1;
        q1 = 1;
        q1 = 15.7799883256466749731+z*q1;
        q1 = 45.3907635128879210584+z*q1;
        q1 = 41.3172038254672030440+z*q1;
        q1 = 15.0425385692907503408+z*q1;
        q1 = 2.50464946208309415979+z*q1;
        q1 = -1.42182922854787788574*0.1+z*q1;
        q1 = -3.80806407691578277194*0.01+z*q1;
        q1 = -9.33259480895457427372*0.0001+z*q1;
        x1 = z*p1/q1;
    }
    else
    {
        p2 = 3.23774891776946035970;
        p2 = 6.91522889068984211695+z*p2;
        p2 = 3.93881025292474443415+z*p2;
        p2 = 1.33303460815807542389+z*p2;
        p2 = 2.01485389549179081538*0.1+z*p2;
        p2 = 1.23716634817820021358*0.01+z*p2;
        p2 = 3.01581553508235416007*0.0001+z*p2;
        p2 = 2.65806974686737550832*0.000001+z*p2;
        p2 = 6.23974539184983293730*0.000000001+z*p2;
        q2 = 1;
        q2 = 6.02427039364742014255+z*q2;
        q2 = 3.67983563856160859403+z*q2;
        q2 = 1.37702099489081330271+z*q2;
        q2 = 2.16236993594496635890*0.1+z*q2;
        q2 = 1.34204006088543189037*0.01+z*q2;
        q2 = 3.28014464682127739104*0.0001+z*q2;
        q2 = 2.89247864745380683936*0.000001+z*q2;
        q2 = 6.79019408009981274425*0.000000001+z*q2;
        x1 = z*p2/q2;
    }
    x = x0-x1;
    if( code!=0 )
    {
        x = -x;
    }
    result = x;
    return result;
}

moldyn_tst::moldyn_tst(engine * p1, f64 p2) : moldyn (p1, p2)
{
	eng = p1;
	
	tstep1 = p2;						// [1.0E-15 s]
	tstep2 = tstep1 * tstep1;				// [1.0E-30 s]
	
	vel = new f64[eng->GetAtomCount() * 3];			// [1.0E+3 m/s]
	acc = new f64[eng->GetAtomCount() * 3];			// [1.0E+12 m/s^2]
	
	mass = new f64[eng->GetAtomCount()];
	
	locked = new char[eng->GetAtomCount()];
	worked = new char[eng->GetAtomCount()];
//	target = new char[eng->GetAtomCount()];
	gravi = new char[eng->GetAtomCount()];
	
	step_counter = 0;
	
	atom ** glob_atmtab = eng->GetSetup()->GetAtoms();

	num_gravi = 0;	num_worked = 0; 
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
		// умножаем на массу единицы атомного веса углеродной шкалы в кг и на число Авогадро
		mass[counter] *= 1.6605402e-27 * 6.0221367e+23;
		if (glob_atmtab[counter]->flags & ATOMFLAG_IS_SPECIAL_ATOM)
		{
			mass[counter] *= 20;
		}		
		
		locked[counter] = lflag;

		bool wflag = false;
		if (glob_atmtab[counter]->flags & ATOMFLAG_IS_WORKING_ATOM)
		{
			wflag = true;
			num_worked++;
		}		
		worked[counter] = wflag;

		bool grflag = false;
		if (glob_atmtab[counter]->flags & ATOMFLAG_IS_GRAVI)
		{
			grflag = true;
			num_gravi++;
		}	

		gravi[counter] = grflag;

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

	for (i32s dim = 0; dim < 3; dim++)
	{
		this->m_g[dim] = 0.0;
	}

	// the rest are just default values; can be modified...
	// the rest are just default values; can be modified...
	// the rest are just default values; can be modified...
	
	temperature = 300.0;
	
	temperature_coupling = 0.010;

	psi = 0.0;
	relax_rate_2 = 0.00001;





#if GRAVI_OSCILLATOR_WORKING 
// режим работы с наложением броуновского осциллятора 
// в виде гравитационной осцилляции
	v_pract = 0.0; // m/s
	v_theor = 0.0;
    v0 = 0.0; // m/s
	g0 = 0;//амплитуда гравитационного осциллятора [m/s^2]
	omega = 0;//круговая частота гравитационного осциллятора [s^-1]
	start_gravi_step_counter = 0;
	to_start_gravi = false;
#endif

#if SNARJAD_TARGET_WORKING
	n_snarjad = -1;
#endif
#if SNARJAD_TARGET_WORKING || PROBNIY_ATOM_WORKING
	SetTarget();
#endif
}

moldyn_tst::~moldyn_tst(void)
{
	delete[] vel;
	delete[] acc;
	
	delete[] mass;
	
	delete[] worked;
	delete[] locked;
//	delete[] target;
	delete[] gravi;
}

void moldyn_tst::MaxwellDistribution(f64 Temp)
{
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			i32u rnum = rand(); 
			f64 r1 = ((f64) rnum / (f64) RAND_MAX);
			vel[n1 * 3 + n2] = invnormaldistribution(r1) * sqrt(1.0*8.314*Temp/mass[n1]) / 1000.;
		}
	}
}
f64 moldyn_tst::GetGravi(i32s dim)
{
	if (dim >= 0 && dim < 3)
		return this->m_g[dim];
	else
		return 0.0;
}
void moldyn_tst::SetGraviG(f64 *p)
{
	if (p)
	{
		for (i32s dim = 0; dim < 3; dim++)
		{
			this->m_g[dim] = p[dim];
		}
	}
}
#if GRAVI_OSCILLATOR_WORKING 
// режим работы с наложением броуновского осциллятора 
// в виде гравитационной осцилляции
//функция инициализации гравитационного осциллятора, вызывается перед началом работы
void moldyn_tst::InitGraviOscillator(moldyn_tst_param& param, engine_pbc2 * eng_pbc)
{
	this->len_n_len_g = param.len_n_len_g;
	this->len_n = 2.0 * eng->GetSetup()->GetModel()->periodic_box_HALFdim[2];
	this->len_g = this->len_n / this->len_n_len_g;
	this->N_mol = eng_pbc->n_solvent_molecules;
	this->M = eng_pbc->M_solvent;
	printf("M = %f\n", this->M);
	gamma = 5.0 / 3.0;

	double RT_M = 8.314510 * param.temperature / M; // Дж/кг = m^2/s^2
	g0 = ( RT_M * sqrt(2.0) * PI / (len_g * 1.0e-9) ) * sqrt(gamma * len_n_len_g / N_mol); // m/s^2
	omega = ( PI / (len_g * 1.0e-9) ) * sqrt(gamma * RT_M); // s^-1
	v0 = sqrt(2.0 * RT_M * len_n_len_g / N_mol);

	printf("omega = %e\tg0 = %e\tv0 = %e\n", omega, g0, v0);

	omega *= 1.0e-15;
	g0 *= 1.0e-12;
	v0 *= 1.0e-3;

	printf("omega = %e\tg0 = %e\tv0 = %e\n", omega, g0, v0);

	to_start_gravi = true;
	start_gravi_step_counter = step_counter;
#if !GRAVI_OSCILLATOR_WORKING_LINEAR
#else
		this->k_lin_gravi = param.k_lin_gravi;
#endif


}
// на каждом шаге вычисляется значение амплитуды g в центре осцилляторе
void moldyn_tst::TakeGraviStep()
{
	if(to_start_gravi)
	{
		double t = tstep1 * (step_counter - start_gravi_step_counter)/* * 1.0e-15*/;//момент времени в секундах
#if !GRAVI_OSCILLATOR_WORKING_LINEAR
		m_g[2] = g0 * cos(omega * t)/* * 1.0e-12*/; //амплитуда в центре осциллятора
		v_theor = v0 * sin(omega * t);
#else
		m_g[2] = k_lin_gravi * t/* * 1.0e-12*/; //амплитуда в центре осциллятора
		//printf("m_g[2] %f = k_lin_gravi %f  * t %f\n", m_g[2], k_lin_gravi, t);
		v_theor = 0.0;
#endif
		v_pract += tstep1 * m_g[2] * 1.0e-6;
	}
}
double moldyn_tst::GetTheorV()
{
	return v_theor;
}
double moldyn_tst::GetPractV()
{
	return v_pract;
}

#endif

#if SOUND_GRAVI_OSCILLATOR
// на каждом шаге вычисляется значение амплитуды g
void moldyn_tst::TakeGraviStep()
{
	if(to_start_gravi)
	{
		double t = tstep1 * (step_counter - start_gravi_step_counter)/* * 1.0e-15*/;//момент времени в секундах
		m_g_gravi_dim = g0 * sin(omega * t)/* * 1.0e-12*/; //амплитуда в центре осциллятора
	}
}
void moldyn_tst::InitGraviOscillator(moldyn_tst_param& param)
{
	cout << "SOUND_GRAVI_OSCILLATOR\n";

	cout << "Enter gravi_dim: 0-x, 1-y, 2-z\n";
	cin >> gravi_dim;

	cout << "Enter g0, m/s^2\n";
	cin >> g0;

	cout << "Enter m_T period in pikaseconds\n";
	cout << "One pikasecond correspond 10^12 Hz\n";
	cout << "Hypersound 10^9 - 10^13 Hz\n";
	cout << "correspond interval 0.1 pikaseconds (10^13 Hz) - 1000 pikaseconds (10^9 Hz)\n";
	cout << "So, m_T period in pikaseconds?\n";
	cin >> m_T;

	to_start_gravi = true;
	start_gravi_step_counter = step_counter;
	//gravi_dim = param.gravi_dim;
	//g0 = param.g0;
	//пеерводим период из пикасекунд в фемтосекунды
	//и получаем круговую частоту в обратных фемтосекунды
	omega = 2.0 * M_PI / (1000.0 * m_T);
	m_g_gravi_dim = 0.0;


}
#endif

void moldyn_tst::LockAtoms(char * fn)
{
	vector<i32s> fixed_list;
	ReadTargetListFile(fn, fixed_list);
	this->LockAtoms(fixed_list);
}

void moldyn_tst::LockAtoms(vector<i32s>& nAtoms)
{
	atom ** glob_atmtab = eng->GetSetup()->GetAtoms();

	for(int i = 0; i < nAtoms.size(); i++)
	{	
		i32s nAtom = nAtoms[i];
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
}


#if SNARJAD_TARGET_WORKING
void moldyn_tst::ShootSnarjad(i32s nAtom, f64* v, f64* c)
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

bool moldyn_tst::GetSnarjadVelCrd(f64* v, f64* c, f64* f)
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
void moldyn_tst::LockAtom(i32s nAtom)
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

void moldyn_tst::SetTarget()
{
	num_target = 0;// i32s counter = 0;
//	while (counter < eng->GetAtomCount())
//	{
//		target[counter] = false;
//		counter++;
//	}
	this->m_target_list.resize(0);
}

void moldyn_tst::SetTarget(vector<i32s>& target_list)
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

void moldyn_tst::ComputeTargetCrd()
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

bool moldyn_tst::GetProbAtomForce(f64* f)
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

void moldyn_tst::SetProbAtom(i32s nAtom, f64* c)
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
#if WRITE_LOCKED_FORCES
void moldyn_tst::WriteLockedForcesHeader()
{
	SYSTEMTIME time;
	GetLocalTime(&time);

	WORD y = time.wYear;
	WORD mo = time.wMonth;
	WORD d = time.wDay;

	WORD h = time.wHour;
	WORD mi = time.wMinute;
	WORD s = time.wSecond;

	sprintf(locked_forces_fn, "D://locked_forces_%d.%d.%d_%d.%d.%d.txt", y,mo,d,h,mi,s);
	FILE * stream = fopen(locked_forces_fn, "wt");
	if (stream)
	{
		fprintf(stream,"t");
		for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
		{
			if (locked[n1])
			{
				//fprintf(stream,",x%d", n1);
				//fprintf(stream,",y%d", n1);
				//fprintf(stream,",z%d", n1);
			}
		}
#if SOUND_GRAVI_OSCILLATOR
		fprintf(stream,",m_g_gravi_dim\n");
#endif
		fprintf(stream, ",temperature");
		fprintf(stream, ",ConvEKinTemp(ekin)");

		fprintf(stream, ",sum_mom_xy");

		fprintf(stream, ",sum_vel[0]");
		fprintf(stream, ",sum_vel[1]");
		fprintf(stream, ",sum_vel[2]");

		fprintf(stream, ",sum_vel_up[0]");
		fprintf(stream, ",sum_vel_up[1]");
		fprintf(stream, ",sum_vel_up[2]");

		fprintf(stream, ",sum_vel_dw[0]");
		fprintf(stream, ",sum_vel_dw[1]");
		fprintf(stream, ",sum_vel_dw[2]");

		fprintf(stream, ",sum_fup[0]");
		fprintf(stream, ",sum_fup[1]");
		fprintf(stream, ",sum_fup[2]");

		fprintf(stream, ",sum_fdw[0]");
		fprintf(stream, ",sum_fdw[1]");
		fprintf(stream, ",sum_fdw[2]");

		fprintf(stream, "\n");
		fclose(stream);
	}
}
#endif
#if WRITE_WORKED_FORCES
void moldyn_tst::WriteWorkedForcesHeader()
{
	SYSTEMTIME time;
	GetLocalTime(&time);

	WORD y = time.wYear;
	WORD mo = time.wMonth;
	WORD d = time.wDay;

	WORD h = time.wHour;
	WORD mi = time.wMinute;
	WORD s = time.wSecond;

	sprintf(worked_forces_fn, "D://worked_forces_%d.%d.%d_%d.%d.%d.txt", y,mo,d,h,mi,s);
	FILE * stream = fopen(worked_forces_fn, "wt");
	if (stream)
	{
		fprintf(stream,"t");
#if SOUND_GRAVI_OSCILLATOR
		fprintf(stream,",m_g_gravi_dim\n");
#endif
		fprintf(stream, ",temperature");
		fprintf(stream, ",ConvEKinTemp(ekin)");

		fprintf(stream, ",sum_mom_xy");

		fprintf(stream, ",sum_vel[0]");
		fprintf(stream, ",sum_vel[1]");
		fprintf(stream, ",sum_vel[2]");

		fprintf(stream, ",sum_vel_wk_up[0]");
		fprintf(stream, ",sum_vel_wk_up[1]");
		fprintf(stream, ",sum_vel_wk_up[2]");

		fprintf(stream, ",sum_vel_wk_dw[0]");
		fprintf(stream, ",sum_vel_wk_dw[1]");
		fprintf(stream, ",sum_vel_wk_dw[2]");

		fprintf(stream, ",sum_vel_up[0]");
		fprintf(stream, ",sum_vel_up[1]");
		fprintf(stream, ",sum_vel_up[2]");

		fprintf(stream, ",sum_vel_dw[0]");
		fprintf(stream, ",sum_vel_dw[1]");
		fprintf(stream, ",sum_vel_dw[2]");

		fprintf(stream, ",sum_f_wk_up[0]");
		fprintf(stream, ",sum_f_wk_up[1]");
		fprintf(stream, ",sum_f_wk_up[2]");

		fprintf(stream, ",sum_f_wk_dw[0]");
		fprintf(stream, ",sum_f_wk_dw[1]");
		fprintf(stream, ",sum_f_wk_dw[2]");

		fprintf(stream, "\n");
		fclose(stream);
	}
}
#endif
void moldyn_tst::TakeMDStep(bool enable_temperature_control)
{
#if WRITE_LOCKED_FORCES
	double sum_mom_xy = 0.0;

	double sum_vel[3];
	sum_vel[0] = 0.0;
	sum_vel[1] = 0.0;
	sum_vel[2] = 0.0;

	double sum_vel_up[3];
	sum_vel_up[0] = 0.0;
	sum_vel_up[1] = 0.0;
	sum_vel_up[2] = 0.0;

	double sum_vel_dw[3];
	sum_vel_dw[0] = 0.0;
	sum_vel_dw[1] = 0.0;
	sum_vel_dw[2] = 0.0;

	double sum_fup[3];
	sum_fup[0] = 0.0;
	sum_fup[1] = 0.0;
	sum_fup[2] = 0.0;
	double sum_fdw[3];
	sum_fdw[0] = 0.0;
	sum_fdw[1] = 0.0;
	sum_fdw[2] = 0.0;
#endif
#if WRITE_WORKED_FORCES
	double sum_mom_xy = 0.0;

	double sum_vel[3];
	sum_vel[0] = 0.0;
	sum_vel[1] = 0.0;
	sum_vel[2] = 0.0;

	double sum_vel_wk_up[3];
	sum_vel_wk_up[0] = 0.0;
	sum_vel_wk_up[1] = 0.0;
	sum_vel_wk_up[2] = 0.0;

	double sum_vel_wk_dw[3];
	sum_vel_wk_dw[0] = 0.0;
	sum_vel_wk_dw[1] = 0.0;
	sum_vel_wk_dw[2] = 0.0;


	double sum_vel_up[3];
	sum_vel_up[0] = 0.0;
	sum_vel_up[1] = 0.0;
	sum_vel_up[2] = 0.0;

	double sum_vel_dw[3];
	sum_vel_dw[0] = 0.0;
	sum_vel_dw[1] = 0.0;
	sum_vel_dw[2] = 0.0;

	double sum_f_wk_up[3];
	sum_f_wk_up[0] = 0.0;
	sum_f_wk_up[1] = 0.0;
	sum_f_wk_up[2] = 0.0;
	double sum_f_wk_dw[3];
	sum_f_wk_dw[0] = 0.0;
	sum_f_wk_dw[1] = 0.0;
	sum_f_wk_dw[2] = 0.0;
#endif
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			f64 tmpA = acc[n1 * 3 + n2];
			f64 tmp1 = tstep1 * vel[n1 * 3 + n2] * 1.0e-3;
			f64 tmp2 = tstep2 * tmpA * 0.5e-9;
			
#if MOLDYN_LOCK_ATOMS_ONLY_ON_Z_DIM 
			if (n2 == 2 && locked[n1])
#else
			if (locked[n1])
#endif
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
#if WRITE_LOCKED_FORCES
			sum_vel[n2] += vel[n1 * 3 + n2];
			if (eng->crd[n1 * 3 + 1] > 0.0)
			{
				sum_vel_up[n2] += vel[n1 * 3 + n2];
			}
			else
			{
				sum_vel_dw[n2] += vel[n1 * 3 + n2];
			}
#endif
#if WRITE_WORKED_FORCES
			if (worked[n1])
			{
				if (eng->crd[n1 * 3 + 1] > 0.0)
				{
					sum_vel_wk_up[n2] += vel[n1 * 3 + n2];
				}
				else
				{
					sum_vel_wk_dw[n2] += vel[n1 * 3 + n2];
				}
			}
			//else
			{
				sum_vel[n2] += vel[n1 * 3 + n2];
				if (eng->crd[n1 * 3 + 1] > 0.0)
				{
					sum_vel_up[n2] += vel[n1 * 3 + n2];
				}
				else
				{
					sum_vel_dw[n2] += vel[n1 * 3 + n2];
				}
			}
#endif
		}
#if WRITE_LOCKED_FORCES
		// это неверно
		// момент по часовой стрелке
		// y*vx-x*vy
		sum_mom_xy += 
			eng->crd[n1 * 3 + 1] * vel[n1 * 3 + 0] - 
			eng->crd[n1 * 3 + 0] * vel[n1 * 3 + 1];
#endif
#if WRITE_WORKED_FORCES
		if (!worked[n1])
		{
			// момент против часовой стрелки
			// x*vy-y*vx
			sum_mom_xy += mass[n1] * 
				(eng->crd[n1 * 3 + 0] * vel[n1 * 3 + 1] - 
				eng->crd[n1 * 3 + 1] * vel[n1 * 3 + 0]);
		}
#endif
	}
	
	eng->DoSHAKE();
	
	eng->Compute(1);
	epot = eng->energy;

#if WRITE_LOCKED_FORCES
	FILE * locked_forces_stream = fopen(locked_forces_fn, "at");
	if (locked_forces_stream)
		fprintf(locked_forces_stream, "%f", tstep1*step_counter);
#endif /*WRITE_LOCKED_FORCES*/
#if WRITE_WORKED_FORCES
	FILE * worked_forces_stream = fopen(worked_forces_fn, "at");
	if (worked_forces_stream)
		fprintf(worked_forces_stream, "%f", tstep1*step_counter);
#endif /*WRITE_LOCKED_FORCES*/
	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
#if WRITE_LOCKED_FORCES
		if (locked[n1])
		{
			if (locked_forces_stream)
			{
				if (eng->crd[n1 * 3 + 1] > 0.0)
				{
	sum_fup[0] += -eng->d1[n1 * 3 + 0];
	sum_fup[1] += -eng->d1[n1 * 3 + 1];
	sum_fup[2] += -eng->d1[n1 * 3 + 2];
				}
				else
				{
	sum_fdw[0] += -eng->d1[n1 * 3 + 0];
	sum_fdw[1] += -eng->d1[n1 * 3 + 1];
	sum_fdw[2] += -eng->d1[n1 * 3 + 2];
				}
			}
		}
#endif /*WRITE_LOCKED_FORCES*/
#if WRITE_WORKED_FORCES
		if (worked[n1])
		{
				if (eng->crd[n1 * 3 + 1] > 0.0)
				{
	sum_f_wk_up[0] += -eng->d1[n1 * 3 + 0];
	sum_f_wk_up[1] += -eng->d1[n1 * 3 + 1];
	sum_f_wk_up[2] += -eng->d1[n1 * 3 + 2];
				}
				else
				{
	sum_f_wk_dw[0] += -eng->d1[n1 * 3 + 0];
	sum_f_wk_dw[1] += -eng->d1[n1 * 3 + 1];
	sum_f_wk_dw[2] += -eng->d1[n1 * 3 + 2];
				}
			
		}
#endif /*WRITE_WORKED_FORCES*/

#if MOLDYN_LOCK_ATOMS_ONLY_ON_Z_DIM 
		if (n2 == 2 && locked[n1]) {
#else
		if (locked[n1]) {
#endif
			acc[n1 * 3 + 0] = acc[n1 * 3 + 1] = acc[n1 * 3 + 2] = 0.0;
			vel[n1 * 3 + 0] = vel[n1 * 3 + 1] = vel[n1 * 3 + 2] = 0.0;
		}
		else //not locked
		{
		//a = -F/m
		//dv = a*dt
		
		acc[n1 * 3 + 0] = -eng->d1[n1 * 3 + 0] / mass[n1];
		acc[n1 * 3 + 1] = -eng->d1[n1 * 3 + 1] / mass[n1];
		acc[n1 * 3 + 2] = -eng->d1[n1 * 3 + 2] / mass[n1];

#if SOUND_GRAVI_OSCILLATOR
		acc[n1 * 3 + gravi_dim] += m_g_gravi_dim;
#endif

		if (gravi[n1])
		{
#if GRAVI_OSCILLATOR_WORKING 
#if !GRAVI_OSCILLATOR_WORKING_LINEAR
			acc[n1 * 3 + 2] += this->m_g[2] * cos (eng->crd[n1 * 3 + 2] / len_g);
#else
			acc[n1 * 3 + 2] += this->m_g[2]);
#endif
#else
			acc[n1 * 3 + 0] += this->m_g[0];
			acc[n1 * 3 + 1] += this->m_g[1];
			acc[n1 * 3 + 2] += this->m_g[2];
#endif /* GRAVI_OSCILLATOR_WORKING */
		}
		
		vel[n1 * 3 + 0] += tstep1 * acc[n1 * 3 + 0] * 0.5e-6;
		vel[n1 * 3 + 1] += tstep1 * acc[n1 * 3 + 1] * 0.5e-6;
		vel[n1 * 3 + 2] += tstep1 * acc[n1 * 3 + 2] * 0.5e-6;

		for (i32s n2 = 0;n2 < 3;n2++)
		{
			cumsum_vel[n1 * 3 + n2] += vel[n1 * 3 + n2];
			cumsum_acc[n1 * 3 + n2] += acc[n1 * 3 + n2];
		}
		}//not locked
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			cumsum_f[n1 * 3 + n2] += eng->d1[n1 * 3 + n2];
		}
	}

#if WRITE_LOCKED_FORCES
	if (locked_forces_stream)
	{
#if SOUND_GRAVI_OSCILLATOR
		fprintf(locked_forces_stream, ",%f", m_g_gravi_dim);
#endif
		fprintf(locked_forces_stream, ",%f", temperature);
		fprintf(locked_forces_stream, ",%f", ConvEKinTemp(ekin));

		fprintf(locked_forces_stream, ",%f", sum_mom_xy);

		fprintf(locked_forces_stream, ",%f", sum_vel[0]);
		fprintf(locked_forces_stream, ",%f", sum_vel[1]);
		fprintf(locked_forces_stream, ",%f", sum_vel[2]);

		fprintf(locked_forces_stream, ",%f", sum_vel_up[0]);
		fprintf(locked_forces_stream, ",%f", sum_vel_up[1]);
		fprintf(locked_forces_stream, ",%f", sum_vel_up[2]);

		fprintf(locked_forces_stream, ",%f", sum_vel_dw[0]);
		fprintf(locked_forces_stream, ",%f", sum_vel_dw[1]);
		fprintf(locked_forces_stream, ",%f", sum_vel_dw[2]);

		fprintf(locked_forces_stream, ",%f", sum_fup[0]);
		fprintf(locked_forces_stream, ",%f", sum_fup[1]);
		fprintf(locked_forces_stream, ",%f", sum_fup[2]);

		fprintf(locked_forces_stream, ",%f", sum_fdw[0]);
		fprintf(locked_forces_stream, ",%f", sum_fdw[1]);
		fprintf(locked_forces_stream, ",%f", sum_fdw[2]);

		fprintf(locked_forces_stream, "\n");
		fclose(locked_forces_stream);
	}
#endif /*WRITE_LOCKED_FORCES*/
#if WRITE_WORKED_FORCES
	if (worked_forces_stream)
	{
#if SOUND_GRAVI_OSCILLATOR
		fprintf(worked_forces_stream, ",%f", m_g_gravi_dim);
#endif
		fprintf(worked_forces_stream, ",%f", temperature);
		fprintf(worked_forces_stream, ",%f", ConvEKinTemp(ekin));

		fprintf(worked_forces_stream, ",%f", sum_mom_xy);

		fprintf(worked_forces_stream, ",%f", sum_vel[0]);
		fprintf(worked_forces_stream, ",%f", sum_vel[1]);
		fprintf(worked_forces_stream, ",%f", sum_vel[2]);

		fprintf(worked_forces_stream, ",%f", sum_vel_wk_up[0]);
		fprintf(worked_forces_stream, ",%f", sum_vel_wk_up[1]);
		fprintf(worked_forces_stream, ",%f", sum_vel_wk_up[2]);

		fprintf(worked_forces_stream, ",%f", sum_vel_wk_dw[0]);
		fprintf(worked_forces_stream, ",%f", sum_vel_wk_dw[1]);
		fprintf(worked_forces_stream, ",%f", sum_vel_wk_dw[2]);

		fprintf(worked_forces_stream, ",%f", sum_vel_up[0]);
		fprintf(worked_forces_stream, ",%f", sum_vel_up[1]);
		fprintf(worked_forces_stream, ",%f", sum_vel_up[2]);

		fprintf(worked_forces_stream, ",%f", sum_vel_dw[0]);
		fprintf(worked_forces_stream, ",%f", sum_vel_dw[1]);
		fprintf(worked_forces_stream, ",%f", sum_vel_dw[2]);

		fprintf(worked_forces_stream, ",%f", sum_f_wk_up[0]);
		fprintf(worked_forces_stream, ",%f", sum_f_wk_up[1]);
		fprintf(worked_forces_stream, ",%f", sum_f_wk_up[2]);

		fprintf(worked_forces_stream, ",%f", sum_f_wk_dw[0]);
		fprintf(worked_forces_stream, ",%f", sum_f_wk_dw[1]);
		fprintf(worked_forces_stream, ",%f", sum_f_wk_dw[2]);

		fprintf(worked_forces_stream, "\n");
		fclose(worked_forces_stream);
	}
#endif /*WRITE_WORKED_FORCES*/

	ekin = KineticEnergy();

	if (enable_temperature_control)
	{
		f64 delta = (temperature / ConvEKinTemp(ekin)) - 1.0;

		f64 tc = sqrt(1.0 + temperature_coupling * delta);

		bool with_constant_impuls = false;
		bool and_with_zero_impuls = true;

		SetEKin(ekin * tc, with_constant_impuls, and_with_zero_impuls);
	}

	step_counter++;
}

f64 moldyn_tst::KineticEnergy(void)
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

f64 moldyn_tst::ConvTempEKin(f64 p1)
{
	return (3.0 / 2.0) * p1 * ((eng->GetAtomCount() - num_locked) * 8.314510) / 1000.0;
}

f64 moldyn_tst::ConvEKinTemp(f64 p1)
{
	return (2.0 / 3.0) * p1 * 1000.0 / ((eng->GetAtomCount() - num_locked) * 8.314510);
}

void moldyn_tst::SetEKin(f64 p1, bool with_constant_impuls, bool and_with_zero_impuls)
{
	// определяем суммарный импульс системы до применения термостата
	f64 sum_p1[3];
	if (with_constant_impuls)
	{
		if (and_with_zero_impuls)
		{
			sum_p1[0] = 0.0;
			sum_p1[1] = 0.0;
			sum_p1[2] = 0.0;
		}
		else
			SumModelImpuls(sum_p1);
	}

	//№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№
	// оригинальный код, не учитывающий необходимость соблюдения закона сохранения импульса
	//f64 tmp1 = p1 / KineticEnergy();
	f64 tmp1 = p1 / ekin;//заменили для ускорения поскольку ekin было вычислено непосредственно перед вызовом данной функции
	f64 tmp2 = (tmp1 < 0.0 ? 0.0 : sqrt(tmp1));
	
	i32u tmp3 = eng->GetAtomCount() * 3;
	for (i32u n1 = 0;n1 < tmp3;n1++) vel[n1] *= tmp2;
	//№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№№
	if (with_constant_impuls)
	{
		// определяем суммарный импульс системы после применения термостата
		f64 sum_p2[3];
		SumModelImpuls(sum_p2);
		// производим такую корректировку, чтобы суммарный импульс в результате действия термостата не изменился 
		f64 dp[3];//поправка к импульсу в расчёте на каждый атом
		for (i32s n2 = 0;n2 < 3;n2++) 
			dp[n2] = (sum_p1[n2] - sum_p2[n2]) / (eng->GetAtomCount() - num_locked);

		for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
		{
			if (!locked[n1])
			{
				for (i32s n2 = 0;n2 < 3;n2++)
				{
					vel[n1 * 3 + n2] += dp[n2] / mass[n1];
				}	
			}
		}
#if 0
		//Отладка
		f64 sum_p3[3];
		SumModelImpuls(sum_p3);

		for (i32s n2 = 0;n2 < 3;n2++) 
			printf ("sum_p[%d]\t%f\t%f\t%f\t\t%f\n", n2,  sum_p1[n2], sum_p2[n2], sum_p3[n2], sum_p1[n2]-sum_p3[n2]);
#endif
	}
#if 1 
	//Отладка
	f64 ekin_tst = KineticEnergy();
	if (fabs(p1 - ekin_tst) > 1e-6)
	{
		printf("p1 - ekin_tst = %f\n", p1 - ekin_tst);
	}
#endif
	ekin = p1;
}

molgroup::molgroup(enum molgrouptype _molgrouptype)
{
	molgrouptype = _molgrouptype;
	for (i32s n2 = 0;n2 < 3;n2++) sum_p[n2] = 0.0;
	num_locked = 0;
}

molgroup::molgroup(enum molgrouptype _molgrouptype, std::list<i32s> _natoms)
{
	molgrouptype = _molgrouptype;
	num_locked = 0;
	for (std::list<i32s>::iterator it = _natoms.begin(); it != _natoms.end(); ++it)
	{
		natoms.push_back(*it);
	}
	
	for (i32s n2 = 0;n2 < 3;n2++) sum_p[n2] = 0.0;
}

void molgroup::ForceMoleculesMomentumToZero(moldyn * mld)
{
	for (i32s n2 = 0;n2 < 3;n2++) sum_p[n2] = 0.0;
	num_locked = 0;

	for (std::list<i32s>::iterator it = natoms.begin(); it != natoms.end(); ++it)
	{
		i32s n1 = *it;

		if (mld->locked[n1])
		{
			num_locked++;
			continue;
		}

		for (i32s n2 = 0;n2 < 3;n2++)
		{
			sum_p[n2] += mld->mass[n1] * mld->vel[n1 * 3 + n2];
		}
	}

	f64 dp[3];
	for (i32s n2 = 0;n2 < 3;n2++)
		dp[n2] = (/*sum_p1[n2]*/ - sum_p[n2]) / (natoms.size() - num_locked);

	for (std::list<i32s>::iterator it = natoms.begin(); it != natoms.end(); ++it)
	{
		i32s n1 = *it;

		if (mld->locked[n1]) continue;

		for (i32s n2 = 0; n2 < 3; n2++)
		{
			mld->vel[n1 * 3 + n2] += dp[n2] / mld->mass[n1];
		}
	}

	for (i32s n2 = 0;n2 < 3;n2++) sum_p[n2] = 0.0;

	for (std::list<i32s>::iterator it = natoms.begin(); it != natoms.end(); ++it)
	{
		i32s n1 = *it;

		if (mld->locked[n1]) continue;

		for (i32s n2 = 0;n2 < 3;n2++)
		{
			sum_p[n2] += mld->mass[n1] * mld->vel[n1 * 3 + n2];
		}
	}

	for (i32s n2 = 0; n2 < 3; n2++)
		printf ("sum_p[%d] %e\n", n2,  sum_p[n2]);
}

void moldyn_tst::ForceMoleculesMomentumToZero()
{
	model * mdl = eng->GetSetup()->GetModel();
	std::list<struct molgroup> molgroups;
	mdl->MakeMoleculesGroups(molgroups);
	for ( std::list<molgroup>::iterator it_mlgr = molgroups.begin();
		it_mlgr != molgroups.end(); ++it_mlgr)
	{
		it_mlgr->ForceMoleculesMomentumToZero(this);
	}
}

void moldyn_tst::SumModelImpuls(f64 * sum_p)
{		
	for (i32s n2 = 0;n2 < 3;n2++) sum_p[n2] = 0.0;

	for (i32s n1 = 0;n1 < eng->GetAtomCount();n1++)
	{
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			sum_p[n2] += mass[n1] * vel[n1 * 3 + n2];
		}		
	}	
}
void moldyn_tst::SaveLastFrame(char * fn)
{
	const int number_of_atoms = eng->GetAtomCount();
	const char file_id[10] = "framev01";

	ofstream ofile;
	ofile.open(fn, ios_base::out | ios_base::trunc | ios_base::binary);
	ofile.write((char *) file_id, 8);					// file id, 8 chars.
	ofile.write((char *) & number_of_atoms, sizeof(number_of_atoms));	// number of atoms, int.
	ofile.write((char *) & step_counter, sizeof(step_counter));	// step_counter, int.
	ofile.write((char *) & tstep1, sizeof(tstep1));	// step_counter, int.
	
	SaveLastFrame(ofile);
	ofile.close();
}

void moldyn_tst::SaveLastFrame(ofstream& ofile)
{
	ofile.write((char *) & ekin, sizeof(ekin));	// kinetic energy, float.
	ofile.write((char *) & epot, sizeof(epot));	// potential energy, float.
	
	double t1a; 
	const int number_of_atoms = eng->GetAtomCount();
	for (i32s tt1 = 0;tt1 < number_of_atoms;tt1++)
	{
		for (i32s tt2 = 0;tt2 < 3;tt2++)
		{
			/*eng->crd[tt1 * 3 + tt2];
			vel[tt1 * 3 + tt2];
			eng->d1[tt1 * 3 + tt2];
			acc[tt1 * 3 + tt2];*/

			t1a = eng->crd[tt1 * 3 + tt2];
			ofile.write((char *) & t1a, sizeof(t1a));

			t1a = vel[tt1 * 3 + tt2];
			ofile.write((char *) & t1a, sizeof(t1a));

			t1a = acc[tt1 * 3 + tt2];
			ofile.write((char *) & t1a, sizeof(t1a));

			t1a = eng->d1[tt1 * 3 + tt2];
			ofile.write((char *) & t1a, sizeof(t1a));
		}
	}
}

void moldyn_tst::SaveTrajectoryFrame(long_ofstream& ofile, i32s trajectory_version)
{
	if (trajectory_version < 15 || 16 == trajectory_version)
	{
		float t1a;
		const int number_of_atoms = eng->GetAtomCount();
		for (i32s tt1 = 0;tt1 < number_of_atoms;tt1++)
		{
			for (i32s tt2 = 0;tt2 < 3;tt2++)
			{
				t1a = eng->crd[tt1 * 3 + tt2];
				ofile.write((char *) & t1a, sizeof(t1a));
			}

			if (trajectory_version > 12)
			{
				for (i32s tt2 = 0;tt2 < 3;tt2++)
				{
					t1a = vel[tt1 * 3 + tt2];
					ofile.write((char *) & t1a, sizeof(t1a));
				}
			}

			if (trajectory_version > 13)
			{
				for (i32s tt2 = 0;tt2 < 3;tt2++)
				{
					t1a = acc[tt1 * 3 + tt2];
					ofile.write((char *) & t1a, sizeof(t1a));
				}
			}

			if (trajectory_version > 11)
			{
				for (i32s tt2 = 0;tt2 < 3;tt2++)
				{
					t1a = eng->d1[tt1 * 3 + tt2];
					ofile.write((char *) & t1a, sizeof(t1a));
				}
			}

			if (16 == trajectory_version)
			{
				for (i32s tt2 = 0;tt2 < 3;tt2++)
				{
					t1a = cumsum_vel[tt1 * 3 + tt2];
					ofile.write((char *) & t1a, sizeof(t1a));
				}

				for (i32s tt2 = 0;tt2 < 3;tt2++)
				{
					t1a = cumsum_acc[tt1 * 3 + tt2];
					ofile.write((char *) & t1a, sizeof(t1a));
				}

				for (i32s tt2 = 0;tt2 < 3;tt2++)
				{
					t1a = cumsum_f[tt1 * 3 + tt2];
					ofile.write((char *) & t1a, sizeof(t1a));
				}
			}
		}
	}
	else if (15 == trajectory_version || 17 == trajectory_version)
	{
		f64 t1a;
		const int number_of_atoms = eng->GetAtomCount();
		for (i32s tt1 = 0;tt1 < number_of_atoms;tt1++)
		{
			for (i32s tt2 = 0;tt2 < 3;tt2++)
			{
				t1a = eng->crd[tt1 * 3 + tt2];
				ofile.write((char *) & t1a, sizeof(t1a));
			}

			for (i32s tt2 = 0;tt2 < 3;tt2++)
			{
				t1a = vel[tt1 * 3 + tt2];
				ofile.write((char *) & t1a, sizeof(t1a));
			}

			for (i32s tt2 = 0;tt2 < 3;tt2++)
			{
				t1a = acc[tt1 * 3 + tt2];
				ofile.write((char *) & t1a, sizeof(t1a));
			}

			for (i32s tt2 = 0;tt2 < 3;tt2++)
			{
				t1a = eng->d1[tt1 * 3 + tt2];
				ofile.write((char *) & t1a, sizeof(t1a));
			}

			if (17 == trajectory_version)
			{
				for (i32s tt2 = 0;tt2 < 3;tt2++)
				{
					t1a = cumsum_vel[tt1 * 3 + tt2];
					ofile.write((char *) & t1a, sizeof(t1a));
				}

				for (i32s tt2 = 0;tt2 < 3;tt2++)
				{
					t1a = cumsum_acc[tt1 * 3 + tt2];
					ofile.write((char *) & t1a, sizeof(t1a));
				}

				for (i32s tt2 = 0;tt2 < 3;tt2++)
				{
					t1a = cumsum_f[tt1 * 3 + tt2];
					ofile.write((char *) & t1a, sizeof(t1a));
				}
			}
		}
	}
}

void moldyn_tst::SaveLastFrameTxt(char * fn)
{
	const int number_of_atoms = eng->GetAtomCount();
	const char file_id[10] = "framev01";

	ofstream ofile;
	ofile.open(fn, ios_base::trunc);
	ofile << file_id << endl;// file id, 8 chars.
	ofile << number_of_atoms << endl;// number of atoms, int.
	ofile << step_counter << endl;// step_counter, int.
	ofile << tstep1 << endl;// step_counter, int.


	ofile << ekin << endl;// kinetic energy, float.
	ofile << epot << endl << endl;// potential energy, float.

	double t1a; 
	for (i32s tt1 = 0;tt1 < number_of_atoms;tt1++)
	{
		for (i32s tt2 = 0;tt2 < 3;tt2++)
		{
			/*eng->crd[tt1 * 3 + tt2];
			vel[tt1 * 3 + tt2];
			eng->d1[tt1 * 3 + tt2];
			acc[tt1 * 3 + tt2];*/

			t1a = eng->crd[tt1 * 3 + tt2];
			ofile << t1a << " ";

			t1a = vel[tt1 * 3 + tt2];
			ofile << t1a << " ";

			t1a = acc[tt1 * 3 + tt2];
			ofile << t1a << " ";

			t1a = eng->d1[tt1 * 3 + tt2];
			ofile << t1a << endl;
		}
		ofile << endl;
	}
	ofile.close();
}

void moldyn_tst::ReadLastFrame(char * fn)
{
	ifstream * framefile;
	framefile = new ifstream(fn, ios::in | ios::binary);
	framefile->seekg(8, ios::beg);	// skip the file id...
	int natoms;
	framefile->read((char *) & natoms, sizeof(natoms));
	
	if (natoms != eng->GetAtomCount())
	{
		printf("natoms %d != traj_num_atoms %d\n", natoms, eng->GetAtomCount());
		printf("The LastFrame is incompatible with the current structure/setup!!!");
		printf("incompatible file : different number of atoms!\n");
		if (framefile != NULL)
		{
			framefile->close();
			delete framefile;
			
			framefile = NULL;
		}
		return;
	}
	
	framefile->read((char *) & step_counter, sizeof(step_counter));
	f64 dt;
	framefile->read((char *) & dt, sizeof(dt));
	tstep1 = dt;
	
	framefile->read((char *) & ekin, sizeof(ekin));
	framefile->read((char *) & epot, sizeof(epot));
	
	double t1a; 
	for (i32s tt1 = 0;tt1 < natoms;tt1++)
	{
		for (i32s tt2 = 0;tt2 < 3;tt2++)
		{
			/*eng->crd[tt1 * 3 + tt2];
			vel[tt1 * 3 + tt2];
			eng->d1[tt1 * 3 + tt2];
			acc[tt1 * 3 + tt2];*/

			framefile->read((char *) & t1a, sizeof(t1a));
			eng->crd[tt1 * 3 + tt2] = t1a;

			framefile->read((char *) & t1a, sizeof(t1a));
			vel[tt1 * 3 + tt2] = t1a;

			framefile->read((char *) & t1a, sizeof(t1a));
			acc[tt1 * 3 + tt2] = t1a;

			framefile->read((char *) & t1a, sizeof(t1a));
			eng->d1[tt1 * 3 + tt2] = t1a;
		}
	}	
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
	
	f64 net_random_x = 0.0;
	f64 net_random_y = 0.0;
	f64 net_random_z = 0.0;
	
	i32s langevin_counter = 0;
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
	
	if (langevin_counter > 0)
	{
		net_random_x /= (f64) langevin_counter;
		net_random_y /= (f64) langevin_counter;
		net_random_z /= (f64) langevin_counter;
	}
	
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
