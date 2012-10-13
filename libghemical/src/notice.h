// NOTICE.H : copyright notices...

// Copyright (C) 2000 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef NOTICE_H
#define NOTICE_H

#include "libconfig.h"

/* added by Robert Williams for Compaq cxx, alpha 11/28/01 */
#define __USE_STD_IOSTREAM

#include <iostream>
using namespace std;

/*################################################################################################*/

#define LIB_INTRO_NOTICE_LINES 4
#define COPYRIGHT_NOTICE_LINES 21

const char * get_lib_intro_notice_line(int);
const char * get_copyright_notice_line(int);

void print_lib_intro_notice(ostream &);
void print_copyright_notice(ostream &);

/*################################################################################################*/

#endif	// NOTICE_H

// eof
