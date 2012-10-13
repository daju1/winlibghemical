// TAB_MM_TRIPOS52.H : tables for the molecular mechanics force field.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef TAB_MM_TRIPOS52_H
#define TAB_MM_TRIPOS52_H

#include "libconfig.h"

class tripos52_tables;

/*################################################################################################*/

class model;		// model.h
class typerule;		// typerule.h

#include "eng1_mm_tripos52.h"

#include <vector>
using namespace std;

/*################################################################################################*/

struct tripos52_at		// atomtype
{
	i32s atomtype;
	
	typerule * tr;
	char * description;
};

struct tripos52_bs		// bond stretching
{
	i32s atmtp[2];
	bondtype bndtp;
	f64 param[2];
};

struct tripos52_ab		// angle bending
{
	i32s atmtp[3];
	bondtype bndtp[2];
	f64 param[2];
};

struct tripos52_tr		// torsion
{
	i32s atmtp[4];
	bondtype bndtp[3];
	f64 k; f64 s;
};

struct tripos52_lj		// lennard-jones
{
	i32s type;
	f64 r; f64 k;
};

struct tripos52_ci		// charge increment
{
	i32s atmtp[2];
	bondtype bndtp;
	f64 delta;
};

/*################################################################################################*/

/// Molecular mechanics force field parameter tables.

class tripos52_tables
{
	private:
	
	static tripos52_tables * instance;
	
	vector<tripos52_at> at_vector;
	
	vector<tripos52_bs> bs_vector;
	vector<tripos52_ab> ab_vector;
	vector<tripos52_tr> tr_vector;
	vector<tripos52_lj> lj_vector;
	vector<tripos52_ci> ci_vector;
	
	public:
	
	ostream * ostr;		///< an optional output stream, defaults to (& cout); may be set to NULL.
	
	private:
	
	tripos52_tables(void);
	
	public:
	
	~tripos52_tables(void);
	static tripos52_tables * GetInstance(void);
	
	void PrintAllTypeRules(ostream &);
	
	i32u UpdateTypes(setup *);
	i32u UpdateCharges(setup *);
	
	const tripos52_at * GetAtomType(i32s);
	
	f64 GetChargeInc(bond *);
	bool Init(eng1_mm *, mm_tripos52_bt1 *, i32s);
	bool Init(eng1_mm *, mm_tripos52_bt2 *, i32s *);
	bool Init(eng1_mm *, mm_tripos52_bt3 *, i32s *);
	bool Init(eng1_mm *, mm_tripos52_nbt1 *, bool);
};

/*################################################################################################*/

#endif	// TAB_MM_TRIPOS52_H

// eof
