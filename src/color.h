// COLOR.H : some common (inline-)functions for color selections.

// Copyright (C) 1999 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef COLOR_H
#define COLOR_H

#include "appconfig.h"

/*################################################################################################*/

#include "../libghemical/src/typedef.h"

class engine;	// engine.h

/*################################################################################################*/

// these should be done generic, to give any desired color range...
// these should be done generic, to give any desired color range...
// these should be done generic, to give any desired color range...

void GetRBRange1(fGL, fGL, fGL *);
void GetRBRange2(fGL, fGL, fGL *);

void GetRedColor(fGL, fGL, fGL *);
void GetGreenColor(fGL, fGL, fGL *);
void GetBlueColor(fGL, fGL, fGL *);

fGL GetUnity(engine *, fGL *, fGL *);

/*################################################################################################*/

/**	A color function: white -> red -> green -> blue -> white, constant alpha. 
	This is best suited for opaque/transparent planes...
*/

inline void GetRBRange1(fGL value1, fGL alpha1, fGL * rgb)
{
	fGL value2 = fabs(value1);
	
	if (value2 < 1.0)
	{
		if (value1 < 0.0)
		{
			rgb[0] = 0.0;
			rgb[2] = value2;
		}
		else
		{
			rgb[0] = value2;
			rgb[2] = 0.0;
		}
		
		rgb[1] = 1.0 - value2;
		rgb[3] = alpha1;
	}
	else
	{
		value2 = 1.0 - 1.0 / value2;
		
		if (value1 < 0.0)
		{
			rgb[0] = value2;
			rgb[2] = 1.0;
		}
		else
		{
			rgb[0] = 1.0;
			rgb[2] = value2;
		}
		
		rgb[1] = value2;
		rgb[3] = alpha1;
	}
}

/**	A color function: white -> red -> [SHARP CHANGE] -> blue -> white, variable alpha. 
	This is best suited for transparent objects (volume rendering)...
*/

inline void GetRBRange2(fGL value1, fGL alpha1, fGL * rgb)
{
	fGL value2 = fabs(value1);
	
	fGL alpha2 = alpha1 * value2;
	if (alpha2 > 1.0) alpha2 = 1.0;
	
	if (value2 < 1.0)
	{
		if (value1 < 0.0)
		{
			rgb[0] = 0.0;
			rgb[2] = 1.0;
		}
		else
		{
			rgb[0] = 1.0;
			rgb[2] = 0.0;
		}
		
		rgb[1] = 0.0;
		rgb[3] = alpha2;
	}
	else
	{
		value2 = 1.0 - 1.0 / value2;
		
		if (value1 < 0.0)
		{
			rgb[0] = value2;
			rgb[2] = 1.0;
		}
		else
		{
			rgb[0] = 1.0;
			rgb[2] = value2;
		}
		
		rgb[1] = value2;
		rgb[3] = alpha2;
	}
}

inline void GetRedColor(fGL value, fGL alpha, fGL * rgb)
{
	rgb[0] = value;
	rgb[1] = rgb[2] = 0.0;
	
	rgb[3] = alpha;
}

inline void GetGreenColor(fGL value, fGL alpha, fGL * rgb)
{
	rgb[1] = value;
	rgb[0] = rgb[2] = 0.0;
	
	rgb[3] = alpha;
}

inline void GetBlueColor(fGL value, fGL alpha, fGL * rgb)
{
	rgb[2] = value;
	rgb[0] = rgb[1] = 0.0;
	
	rgb[3] = alpha;
}

inline fGL GetUnity(engine *, fGL *, fGL *)
{
	return 1.0;
}

/*################################################################################################*/

#endif	// COLOR_H

// eof
