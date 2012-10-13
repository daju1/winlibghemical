// INTCRD.H : generic internal coordinates for quick manipulation of structures.

// Copyright (C) 2001 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef INTCRD_H
#define INTCRD_H

#include "libconfig.h"

class ic_data;
class intcrd;

#include "typedef.h"

#include "model.h"

#include <vector>
using namespace std;

/*################################################################################################*/

class ic_data
{
	protected:
	
	ic_data * previous;
	
	fGL len; fGL ang; fGL torc; fGL torv;
	atom * atmr;	// the cartesian coordinates are stored here!!!
	
	bool is_variable;
	
	friend class intcrd;
	
	public:
	
	ic_data(void);
	~ic_data(void);
};

/*################################################################################################*/

class intcrd
{
	protected:
	
	model * mdl;
	i32s molnum;
	i32s crdset;
	
	ic_data base[3];
	
	vector<ic_data *> ic_vector;
	vector<i32u> variable_index_vector;
	
	void AddNewPoint(atom *, ic_data *, bool);
	
	public:
	
	intcrd(model &, i32s, i32s);
	~intcrd(void);
	
	void UpdateCartesian(void);
	
/// the "##variable" functions try to retain the torsions of non-rotating bonds constant.
	i32s GetVariableCount(void);
	i32s FindVariable(atom *, atom *);
	void SetVariable(i32s, fGL);
	fGL GetVariable(i32s);
	
/// the "##torsion" functions give access to all torsions. USE VERY CAREFULLY!!!
	i32s GetTorsionCount(void);
	i32s FindTorsion(atom *, atom *);
	void SetTorsion(i32s, fGL);
	fGL GetTorsion(i32s);
};

/*################################################################################################*/

#endif	// INTCRD_H

// eof
