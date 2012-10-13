// TAB_MM_PRMFIT.H : a slightly modified tables class for FF parameter fitting.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef TAB_MM_PRMFIT_H
#define TAB_MM_PRMFIT_H

#include "libconfig.h"

struct prmfit_at;		// atomtype

struct prmfit_bs;		// bond stretching
struct prmfit_ab;		// angle bending
struct prmfit_tr;		// torsion
struct prmfit_op;		// out of plane

struct prmfit_bs_query;
struct prmfit_ab_query;
struct prmfit_tr_query;
struct prmfit_op_query;

class prmfit_tables;

/*################################################################################################*/

#include "eng1_mm_prmfit.h"
#include "conjgrad.h"

#include <vector>
using namespace std;

/*################################################################################################*/

struct prmfit_at		// atomtype
{
	i32s atomtype[2];
	
	f64 vdw_R; f64 vdw_E;
	f64 formal_charge;
	i32u flags;
	
	typerule * tr;
	char * description;
	
	/*##########################*/
	/*##########################*/
	
};

struct prmfit_bs		// bond stretching
{
	i32s atmtp[2];
	bondtype bndtp;
	
	f64 opt;
	f64 fc;
	
	f64 ci;
	
	/*##########################*/
	/*##########################*/
	
	bool operator<(const prmfit_bs & p1) const
	{
		if (atmtp[0] != p1.atmtp[0]) return (atmtp[0] < p1.atmtp[0]);
		else if (atmtp[1] != p1.atmtp[1]) return (atmtp[1] < p1.atmtp[1]);
		else return (bndtp.GetValue() < p1.bndtp.GetValue());
	}
};

struct prmfit_ab		// angle bending
{
	i32s atmtp[3];
	bondtype bndtp[2];
	
	f64 opt;
	f64 fc;
	
	/*##########################*/
	/*##########################*/
	
	bool operator<(const prmfit_ab & p1) const
	{
		if (atmtp[1] != p1.atmtp[1]) return (atmtp[1] < p1.atmtp[1]);
		else if (atmtp[0] != p1.atmtp[0]) return (atmtp[0] < p1.atmtp[0]);
		else if (atmtp[2] != p1.atmtp[2]) return (atmtp[2] < p1.atmtp[2]);
		else if (bndtp[0].GetValue() != p1.bndtp[0].GetValue()) return (bndtp[0].GetValue() < p1.bndtp[0].GetValue());
		else return (bndtp[1].GetValue() < p1.bndtp[1].GetValue());
	}
};

struct prmfit_tr		// torsion
{
	i32s atmtp[4];
	bondtype bndtp[3];
	
	f64 fc1;
	f64 fc2;
	f64 fc3;
	
	/*##########################*/
	/*##########################*/
	
	bool operator<(const prmfit_tr & p1) const
	{
		if (atmtp[1] != p1.atmtp[1]) return (atmtp[1] < p1.atmtp[1]);
		else if (atmtp[2] != p1.atmtp[2]) return (atmtp[2] < p1.atmtp[2]);
		else if (atmtp[0] != p1.atmtp[0]) return (atmtp[0] < p1.atmtp[0]);
		else if (atmtp[3] != p1.atmtp[3]) return (atmtp[3] < p1.atmtp[3]);
		else if (bndtp[1].GetValue() != p1.bndtp[1].GetValue()) return (bndtp[1].GetValue() < p1.bndtp[1].GetValue());
		else if (bndtp[0].GetValue() != p1.bndtp[0].GetValue()) return (bndtp[0].GetValue() < p1.bndtp[0].GetValue());
		else return (bndtp[2].GetValue() < p1.bndtp[2].GetValue());
	}
};

struct prmfit_op		// out of plane
{
	i32s atmtp[4];
	bondtype bndtp[3];
	
	f64 opt;
	f64 fc;
	
	/*##########################*/
	/*##########################*/
	
	bool operator<(const prmfit_op & p1) const
	{
		if (atmtp[1] != p1.atmtp[1]) return (atmtp[1] < p1.atmtp[1]);
		else if (atmtp[2] != p1.atmtp[2]) return (atmtp[2] < p1.atmtp[2]);
		else if (atmtp[0] != p1.atmtp[0]) return (atmtp[0] < p1.atmtp[0]);
		else if (atmtp[3] != p1.atmtp[3]) return (atmtp[3] < p1.atmtp[3]);
		else if (bndtp[0].GetValue() != p1.bndtp[0].GetValue()) return (bndtp[0].GetValue() < p1.bndtp[0].GetValue());
		else if (bndtp[1].GetValue() != p1.bndtp[1].GetValue()) return (bndtp[1].GetValue() < p1.bndtp[1].GetValue());
		else return (bndtp[2].GetValue() < p1.bndtp[2].GetValue());
	}
};

struct prmfit_bs_query
{
	i32s atmtp[2];		// filled by client!!!
	bondtype bndtp;		// filled by client!!!

	bool strict;		// filled by client!!!
	
	i32s index;
	bool dir;
	
	f64 opt;
	f64 fc;
	
	f64 ci;
};

struct prmfit_ab_query
{
	i32s atmtp[3];		// filled by client!!!
	bondtype bndtp[2];	// filled by client!!!

	bool strict;		// filled by client!!!

	i32s index;
	bool dir;
	
	f64 opt;
	f64 fc;
};

struct prmfit_tr_query
{
	i32s atmtp[4];		// filled by client!!!
	bondtype bndtp[3];	// filled by client!!!

	bool strict;		// filled by client!!!

	i32s index;
	bool dir;
	
	f64 fc1;
	f64 fc2;
	f64 fc3;
};

struct prmfit_op_query
{
	i32s atmtp[4];		// filled by client!!!
	bondtype bndtp[3];	// filled by client!!!

	bool strict;		// filled by client!!!

	i32s index;
	
	f64 opt;
	f64 fc;
};

/*################################################################################################*/

/// force field parameter tables.

class prmfit_tables
{
	protected:
	
	char * path;
	
	vector<prmfit_at> at2_vector;
	
	vector<prmfit_bs> bs_vector;
	vector<prmfit_ab> ab_vector;
	vector<prmfit_tr> tr_vector;
	vector<prmfit_op> op_vector;

	public:
	
	ostream * ostr;		///< an optional output stream, defaults to (& cout); may be set to NULL.
	
	public:
	
	prmfit_tables(const char *);
	virtual ~prmfit_tables(void);
	
// strategy: the tab_mm_prmfit/eng1_mm_prmfit classes are different from the default ones,
// ^^^^^^^^^ and it seems to be OK; compatibility at parameter-file-level seems to be enough!
	
	void PrintAllTypeRules(ostream &);
	
	i32u UpdateTypes(setup *);
	i32u UpdateCharges(setup *);
	
	const prmfit_at * GetAtomType(i32s);
	
	void DoParamSearch(prmfit_bs_query *);
	void DoParamSearch(prmfit_ab_query *);
	void DoParamSearch(prmfit_tr_query *);
	void DoParamSearch(prmfit_op_query *);
};

/*################################################################################################*/

#endif	// TAB_MM_PRMFIT_H

// eof
