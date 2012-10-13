// POP_ANA.H : population analysis classes for QM methods (to determine atomic charges).

// Copyright (C) 2004 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef POP_ANA_H
#define POP_ANA_H

#include "libconfig.h"

class pop_ana;

struct pop_ana_es_data;
class pop_ana_electrostatic;

/*################################################################################################*/

#include "eng1_qm.h"
#include "conjgrad.h"

/*################################################################################################*/

class pop_ana
{
	protected:
	
	setup1_qm * su;
	
	public:
	
	pop_ana(setup1_qm *);
	virtual ~pop_ana(void);
	
	virtual void DoPopAna(void) = 0;
};

/*################################################################################################*/

struct pop_ana_es_data
{
	fGL x; fGL y; fGL z;
	fGL ESP;
};

class pop_ana_electrostatic : public pop_ana, public conjugate_gradient
{
	protected:
	
	vector<pop_ana_es_data> data_vector;
	
	f64 * charge;
	f64 * dchg;
	
	f64 value;
	
	public:
	
	pop_ana_electrostatic(setup1_qm *);
	~pop_ana_electrostatic(void);
	
	void DoPopAna(void);	// virtual
	
	void Check(i32s);
	f64 GetValue(void);	// virtual
	f64 GetGradient(void);	// virtual
	
	void Calculate(i32s);
};

/*################################################################################################*/

#endif	// POP_ANA_H

// eof
