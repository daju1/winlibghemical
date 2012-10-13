// EXAMPLE3.H : a CGI test/example program for libghemical.

// Copyright (C) 2004 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "..\libconfig.h"

#ifndef EXAMPLE3_H
#define EXAMPLE3_H

#include "..\model.h"

#include <iostream>
using namespace std;

#define METHOD_MM		0
#define REQUEST_CompEne		0

/*################################################################################################*/

// the only purpose of this class is to block those functions that might want to
// print something to standard output ; that would interfere with stream I/O.

class silent_model : public model
{
	protected:
	
	public:
	
	silent_model(void);
	~silent_model(void);
	
	virtual void Message(const char *);			// virtual
	virtual void WarningMessage(const char *);		// virtual
	virtual void ErrorMessage(const char *);		// virtual
	
	virtual void PrintToLog(const char *);			// virtual
};

/*################################################################################################*/

int main(int, char **);

/*################################################################################################*/

bool ReadCGI(silent_model &, istream &, bool);			// see streamio.cpp!!!
void WriteCGI_Coord(silent_model &, ostream &, i32u);		// see streamio.cpp!!!

/*################################################################################################*/

#endif	// EXAMPLE3_H

// eof
