// EXAMPLE1.H : a simple test/example program for libghemical.

// Copyright (C) 2003 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "..\libconfig.h"

#ifndef EXAMPLE1_H
#define EXAMPLE1_H

#include "..\model.h"

#include <iostream>
using namespace std;

/*################################################################################################*/

int main(int, char **);

/*################################################################################################*/

bool ReadGPR_OLD(model &, istream &, bool, bool);		// see fileio.cpp!!!
bool ReadGPR_v100(model &, istream &, bool, bool);		// see fileio.cpp!!!
bool ReadGPR(model &, istream &, bool, bool);			// see fileio.cpp!!!
void WriteGPR(model &, ostream &);				// see fileio.cpp!!!

/*################################################################################################*/

#endif	// EXAMPLE1_H

// eof
