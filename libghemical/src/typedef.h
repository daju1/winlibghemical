// LIBTYPEDEF.H : some typedefs + other settings here...

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef LIBTYPEDEF_H
#define LIBTYPEDEF_H

#include "libconfig.h"

/*################################################################################################*/

/* added by Robert Williams for Compaq cxx, alpha 11/28/01 */
#define __USE_STD_IOSTREAM

class engine;	// engine.h

#include <iostream>
using namespace std;

/*################################################################################################*/

typedef int i32s;
typedef unsigned int i32u;

typedef double f64;


/*################################################################################################*/

#if HAVE_GL_GLUT_H
#include <GL/glut.h>

typedef GLint iGLs;
typedef GLuint iGLu;

typedef GLfloat fGL;
typedef GLfloat * ref_fGL;

#else

typedef int iGLs;
typedef unsigned int iGLu;

typedef float fGL;
typedef float * ref_fGL;

#endif

/*################################################################################################*/

typedef fGL ValueFunction(engine *, fGL *, fGL *);
typedef void ColorFunction(fGL, fGL, fGL *);

/*################################################################################################*/

#endif	// LIBTYPEDEF_H

// eof
