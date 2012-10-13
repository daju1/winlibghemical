// ENG1_MM_DEFAULT.H : the default MM force field and computation engine.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef ENG1_MM_DEFAULT_H
#define ENG1_MM_DEFAULT_H

#include "libconfig.h"

struct mm_default_bt1;		// bond stretching
struct mm_default_bt2;		// angle bending
struct mm_default_bt3;		// torsion terms
struct mm_default_bt4;		// out-of-plane

struct mm_default_nbt1;		// nonbonded terms

class eng1_mm_default_bt;		// bonded part
class eng1_mm_default_nbt_mbp;		// nonbonded part; (optional) modified boundary potential
class eng1_mm_default_nbt_mim;		// nonbonded part; minimum image model

class eng1_mm_default_mbp;
class eng1_mm_default_mim;

/*################################################################################################*/

#include "eng1_mm.h"

#include <vector>
using namespace std;

/*################################################################################################*/

struct mm_default_bt1		// bond stretching
{
	i32s atmi[2];
	
	f64 opt;
	f64 fc;
	
	i32s get_atmi(i32s index, bool dir)
	{
		return atmi[dir ? index : !index];
	}
};

struct mm_default_bt2		// angle bending
{
	i32s atmi[3];
	
	i32s index1[2]; bool dir1[2];
	
	f64 opt;
	f64 fc;
	
	i32s get_index(i32s index, bool dir)
	{
		return index1[dir ? index : !index];
	}
	
	bool get_dir(i32s index, bool dir)
	{
		return dir1[dir ? index : !index];
	}
};

struct mm_default_bt3		// torsion terms
{
	i32s atmi[4];
	
	i32s index2[2];
	i32s index1[4]; bool dir1[4];
	
	f64 fc1;
	f64 fc2;
	f64 fc3;
	f64 fc4;	// this is for AMBER parameters only...
	
	bool constraint;
};

struct mm_default_bt4		// out-of-plane
{
	i32s atmi[4];
	
	i32s index2; bool dir2;
	i32s index1[3]; bool dir1[3];
	
	f64 opt;
	f64 fc;
};

// here we use floats to reduce memory consumption... also seems to be slightly faster (at least in intel-machines) than doubles.
// here we use floats to reduce memory consumption... also seems to be slightly faster (at least in intel-machines) than doubles.
// here we use floats to reduce memory consumption... also seems to be slightly faster (at least in intel-machines) than doubles.

struct mm_default_nbt1		// nonbonded terms
{
	i32s atmi[2];
	
	float kr;
	float kd;
	float qq;
};

/*################################################################################################*/

/**	molecular mechanics; bonded part.

	Parameters for this experimental force field come mainly from the Tripos 5.2 
	parameter set. Some extra patameters (like the ones for 4- and 5-membered rings) 
	come from MMFF94 parameters and are only approximate. The system responsible for 
	atomic charges also come from a separate source...
	
	The following is just a useful set of references; not all are used here (at least yet):
	
	Clark M, Cramer III RD, Van Obdenbosch N : "##Validation of the General-Purpose Tripos 
	5.2 Force Field", J. Comp. Chem. 10, 982-1012, (1989)
	
	Bekker H, Berendsen HJC, van Gunsteren WF : "##Force and virial of Torsional-Angle-Dependent 
	Potentials", J. Comp. Chem. 16, 527-533, (1995)
	
	Halgren TA : "##Merck Molecular Force Field. I. Basis, Form, Scope, Parameterization, and 
	Performance of MMFF94", J. Comp. Chem. 17, 490-, (1996) [there are many parts in this paper]
	
	Tuzun RE, Noid DW, Sumpter BG : "##Efficient computation of potential energy first and second 
	derivatives for molecular dynamics, normal coordinate analysis, and molecular mechanics 
	calculations", Macromol. Theory Simul. 5, 771-778, (1996)
	
	Tuzun RE, Noid DW, Sumpter BG : "##Efficient treatment of Out-of-Plane Bend and Improper 
	Torsion Interactions in MM2, MM3, and MM4 Molecular Mechanics Calcultions", J. Comput. Chem. 
	18, 1804-1811, (1997)
	
	Lee SH, Palmo K, Krimm S : "New Out-of-Plane Angle and Bond Angle Internal Coordinates and 
	Related Potential Energy Functions for Molecular Mechanics and Dynamics Simulations", 
	J. Comp. Chem. 20, 1067-1084, (1999)
	
	THE FOLLOWING ARE JUST FUTURE PLANS:
	
	In the long run it would be best to build up a database of QM (and experimental, where 
	available) results, which could then be used to derive (and check!!!) the forcefield 
	parameters. This force field parameter stuff will probably always be inadequate/obsolete 
	somewhat, so when parameters are missing one could gather some extra information, add it 
	to the database, check how it matches with the existing old data, and derive the new parameters...
	
	QM results can be done using MPQC. large amount of molecules/conformations are needed. 
	How to generate the conformations??? Energies must be calculated. What about 1st derivatives??? 
	Or 2nd derivatives??? What is the strategy to derive the parameters?????
*/

class eng1_mm_default_bt : virtual public eng1_mm
{
	protected:
	
	mm_bt1_data * bt1data;
	mm_bt2_data * bt2data;
	
	vector<mm_default_bt1> bt1_vector;
	vector<mm_default_bt2> bt2_vector;
	vector<mm_default_bt3> bt3_vector;
	vector<mm_default_bt4> bt4_vector;
	
	friend class model;	// export_gromacs needs the above tables...
	
	public:
	
	eng1_mm_default_bt(setup *, i32u);
	virtual ~eng1_mm_default_bt(void);
	
	i32s FindTorsion(atom *, atom *, atom *, atom *);	// virtual
	bool SetTorsionConstraint(i32s, f64, f64, bool);	// virtual
	
	protected:
	
	void ComputeBT1(i32u);		// virtual
	void ComputeBT2(i32u);		// virtual
	void ComputeBT3(i32u);		// virtual
	void ComputeBT4(i32u);		// virtual

	virtual void WriteAnglesHeader(FILE * stream, int delim);
	virtual void WriteAngles(FILE * stream, int delim);
};

class eng1_mm_default_bt2 : public eng1_mm_default_bt
{

	public:
	
	eng1_mm_default_bt2(setup *, i32u);
	virtual ~eng1_mm_default_bt2(void);
	
	protected:
	
	void ComputeBT1(i32u);		// virtual
	void ComputeBT2(i32u);		// virtual
	void ComputeBT3(i32u);		// virtual
	void ComputeBT4(i32u);		// virtual


};

/*################################################################################################*/

/// molecular mechanics; nonbonded part, modified boundary potential.

// TODO : how to measure the radial density of the solvent sphere??? what about pressure???
// TODO : how to measure the radial density of the solvent sphere??? what about pressure???
// TODO : how to measure the radial density of the solvent sphere??? what about pressure???

class eng1_mm_default_nbt_mbp : virtual public eng1_mm, virtual public engine_mbp
{
	protected:
	
	vector<mm_default_nbt1> nbt1_vector;
	
	public:
	
	eng1_mm_default_nbt_mbp(setup *, i32u);
	virtual ~eng1_mm_default_nbt_mbp(void);
	
	protected:
	
	void ComputeNBT1(i32u);		// virtual
};

/// molecular mechanics; nonbonded part, minimum image model.

// what is the most reasonable objective or strategy here???
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// there are some very advanced programs (gromacs at least, and possibly others) to run
// the simulations efficiently. only make simple implementations of these techniques here???
// keep the focus in boundary potential methods since those can be applied with SF...

// TODO : how to calculate pressure in periodic boxes???
// TODO : how to calculate pressure in periodic boxes???
// TODO : how to calculate pressure in periodic boxes???
#define USE_ENGINE_PBC2 1

#if USE_ENGINE_PBC2
class eng1_mm_default_nbt_mim : virtual public eng1_mm, public engine_pbc2
#else
class eng1_mm_default_nbt_mim : virtual public eng1_mm, public engine_pbc
#endif /*USE_ENGINE_PBC2*/
{
	friend class project;
	protected:
	
	vector<mm_default_nbt1> nbt1_vector;
	
	f64 sw1; f64 sw2;
	f64 swA; f64 swB;
	
	f64 shft1;
	f64 shft3;
	
	public:		// why public!?!?!?!?
	
	f64 limit;
	
	public:
	
	eng1_mm_default_nbt_mim(setup *, i32u);
	virtual ~eng1_mm_default_nbt_mim(void);
	
	protected:
	
	void UpdateTerms(void);
	
	void ComputeNBT1(i32u);		// virtual
};

/*################################################################################################*/

class eng1_mm_default_mbp : public eng1_mm_default_bt, public eng1_mm_default_nbt_mbp
{
	public:
	
	eng1_mm_default_mbp(setup *, i32u);
	~eng1_mm_default_mbp(void);
};

class eng1_mm_default_mim : public eng1_mm_default_bt, public eng1_mm_default_nbt_mim
{
	public:
	
	eng1_mm_default_mim(setup *, i32u);
	~eng1_mm_default_mim(void);
};

class eng1_mm_default_mim2 : public eng1_mm_default_bt2, public eng1_mm_default_nbt_mim
{
	public:
	
	eng1_mm_default_mim2(setup *, i32u);
	~eng1_mm_default_mim2(void);
};

/*################################################################################################*/

#endif	// ENG1_MM_DEFAULT_H

// eof
