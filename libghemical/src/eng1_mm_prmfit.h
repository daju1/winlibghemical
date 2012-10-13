// ENG1_MM_PRMFIT.H : a slightly modified FF class for FF parameter fitting.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef ENG1_MM_PRMFIT_H
#define ENG1_MM_PRMFIT_H

#include "libconfig.h"

struct mm_prmfit_bt1;		// bond stretching
struct mm_prmfit_bt2;		// angle bending
struct mm_prmfit_bt3;		// torsion terms
struct mm_prmfit_bt4;		// out-of-plane

struct mm_prmfit_nbt1;		// nonbonded terms

class eng1_mm_prmfit;

/*################################################################################################*/

#include "eng1_mm.h"
#include "tab_mm_prmfit.h"

#include <vector>
using namespace std;

/*################################################################################################*/

struct mm_prmfit_bt1		// bond stretching
{
	i32s atmi[2];
	
	f64 opt;
	f64 fc;
	
	i32s get_atmi(i32s index, bool dir)
	{
		return atmi[dir ? index : !index];
	}
	
	// these records contain information that is used in parameter derivation process...
	// these records contain information that is used in parameter derivation process...
	// these records contain information that is used in parameter derivation process...
	
	i32s bt;
};

struct mm_prmfit_bt2		// angle bending
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
	
	// these records contain information that is used in parameter derivation process...
	// these records contain information that is used in parameter derivation process...
	// these records contain information that is used in parameter derivation process...

	i32s bt[2];
};

struct mm_prmfit_bt3		// torsion terms
{
	i32s atmi[4];
	
	i32s index2[2];
	i32s index1[4]; bool dir1[4];
	
	f64 fc1;
	f64 fc2;
	f64 fc3;
	
	bool constraint;
	
	// these records contain information that is used in parameter derivation process...
	// these records contain information that is used in parameter derivation process...
	// these records contain information that is used in parameter derivation process...

	i32s bt[3];
};

struct mm_prmfit_bt4		// out-of-plane
{
	i32s atmi[4];
	
	i32s index2; bool dir2;
	i32s index1[3]; bool dir1[3];
	
	f64 opt;
	f64 fc;
	
	// these records contain information that is used in parameter derivation process...
	// these records contain information that is used in parameter derivation process...
	// these records contain information that is used in parameter derivation process...
	
	i32s bt[3];
};

struct mm_prmfit_nbt1		// nonbonded terms
{
	i32s atmi[2];
	
	f64 kr;
	f64 kd;
	f64 qq;
	
	// these records contain information that is used in parameter derivation process...
	// these records contain information that is used in parameter derivation process...
	// these records contain information that is used in parameter derivation process...
};

/*################################################################################################*/

/// This is a slightly modified and patched eng-class for parameter fitting.

class eng1_mm_prmfit : public eng1_mm
{
	protected:
	
	mm_bt1_data * bt1data;
	mm_bt2_data * bt2data;
	
	vector<mm_prmfit_bt1> bt1_vector;
	vector<mm_prmfit_bt2> bt2_vector;
	vector<mm_prmfit_bt3> bt3_vector;
	vector<mm_prmfit_bt4> bt4_vector;
	
	vector<mm_prmfit_nbt1> nbt1_vector;
	
	friend class prmfit_tables;
	friend class prmfit_cg_optim;
	
	public:
	
	eng1_mm_prmfit(setup *, i32u , prmfit_tables &);
	virtual ~eng1_mm_prmfit(void);
	
// strategy: the tab_mm_prmfit/eng1_mm_prmfit classes are different from the default ones,
// ^^^^^^^^^ and it seems to be OK; compatibility at parameter-file-level seems to be enough!
	
	i32s FindTorsion(atom *, atom *, atom *, atom *);	// virtual
	bool SetTorsionConstraint(i32s, f64, f64, bool);	// virtual
	
	protected:
	
	void ComputeBT1(i32u);		// virtual
	void ComputeBT2(i32u);		// virtual
	void ComputeBT3(i32u);		// virtual
	void ComputeBT4(i32u);		// virtual
	
	void ComputeNBT1(i32u);		// virtual
};

/*################################################################################################*/

#endif	// ENG1_MM_PRMFIT_H

// eof
