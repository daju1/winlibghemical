// LIBDEFINE.H : just some macros #defined here...

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef LIBDEFINE_H
#define LIBDEFINE_H

#include "libconfig.h"

/*################################################################################################*/

#define WEBSITE "http://www.bioinformatics.org/ghemical"

/*################################################################################################*/

/// NOT_DEFINED is defined as -1; therefore negative values often mean "##not defined".

#define NOT_DEFINED -1

/*################################################################################################*/

// some additional macros here...

#define AMINO_BUILDER_FILE "builder/amino.txt"
#define NUCLEIC_BUILDER_FILE "builder/nucleic.txt"

// directory and extension separators in filenames...

#define DIR_SEPARATOR '/'		// directory separator
#define EXT_SEPARATOR '.'		// extension separator

/*################################################################################################*/

#endif	// LIBDEFINE_H

// eof
