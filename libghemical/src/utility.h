// UTILITY.H : many utility functions and classes...

// Copyright (C) 1998 Tommi Hassinen, Geoff Hutchison.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef UTILITY_H
#define UTILITY_H

#include "libconfig.h"

class superimpose;

/*################################################################################################*/

#include "atom.h"
#include "bond.h"

#include "model.h"

#include "conjgrad.h"

#include "v3d.h"

/*################################################################################################*/

/// This class can be used to superimpose coordinate sets.

class superimpose : public conjugate_gradient
{
	protected:
	
	model * mdl;
	
	i32s index[2];
	i32s counter;
	
	f64 value;
	f64 rot[3]; f64 drot[3];
	f64 loc[3]; f64 dloc[3];
	
	public:
	
	superimpose(model *, i32s, i32s);
	~superimpose(void);
	
	f64 GetRMS(void);
	void Compare(const f64 *, const f64 *, bool, f64 * = NULL);
	
	f64 GetValue(void);		// virtual
	f64 GetGradient(void);		// virtual
	
	void Transform(void);
};

/*################################################################################################*/

/**	An utility class (template) to release memory when program closes.
	
	If you have, say a table that you have to create when program starts, 
	and to destroy it when program closes. For example
	
	        int * table = new int[100];
	
	The memory will be automatically released if you attach an "cleaner" for it:
	
	        int * table = new int[100];
	        singleton_cleaner<int> table_cleaner(table);
*/

template<class TYPE1> class singleton_cleaner
{
	private:
	
	TYPE1 * instance;
	
	public:
	
	singleton_cleaner(TYPE1 * p1 = NULL) { instance = p1; }
	~singleton_cleaner(void) { if (instance) delete instance; }
	
	bool SetInstance(TYPE1 * p1) { if (instance) return false; instance = p1; return true; }
};

/*################################################################################################*/

/**	This function can be used to modify a 3D-vector object using a 4x4 transformation 
	matrix. Will first make a [1*4]x[4x4]-type matrix product and after that bring the 
	result back to 3D.
	
	For more complete documentation search for "##homogeneous coordinates"...
*/

void TransformVector(v3d<fGL> &, const fGL *);

/*################################################################################################*/

#define HBOND_TRESHOLD -1.0			// h-bond energy treshold kcal/mol (K&S used -0.5 kcal/mol???)

#define HELIX_CHECK (M_PI * 45.0 / 180.0)	// geometry tresholds for helix torsions...
#define HELIX4_REF (M_PI * +50.3 / 180.0)

void DefineSecondaryStructure(model *);

f64 HBondEnergy(model *, i32s *, i32s *);
bool TorsionCheck(model *, i32s, i32s *, fGL);

/*################################################################################################*/

#endif	// UTILITY_H

// eof
