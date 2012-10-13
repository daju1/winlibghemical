// RESONANCE.H : determine (topological) resonance structures...

// Copyright (C) 2004 Tommi Hassinen, Juha Jungman

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef RESONANCE_H
#define RESONANCE_H

#include "libconfig.h"

class resonance_structures;

#include "model.h"

/*################################################################################################*/

class resonance_structures
{
	protected:
	
	model * mdl;
	
	public:
	
	resonance_structures(model * );
	~resonance_structures(void);
	
	protected:
	
	void MakeStructures(void);
	
	public:
	
	void CycleStructures(void);
};

/*################################################################################################*/

#endif	// RESONANCE_H

// eof
