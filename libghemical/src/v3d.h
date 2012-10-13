// V3D.H : easy-to-use 3D-vector class and coordinate transformations.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef V3D_H
#define V3D_H

#include "libconfig.h"

/*################################################################################################*/

#include "typedef.h"

#include <math.h>

#include <iostream>
using namespace std;

/*################################################################################################*/

/**	A three-dimensional vector class.
	
	Parameters to functions should be chosen this way:
	
	len	<-this-=
	ang	<-this-= =-prm1->
	tor	<-this-= =-prm1-> =-prm2->
	
	tor: direction of the angle is in accordance with the IUPAC convention 
	(clockwise rotation of "this" to eclipse "v2" is the positive direction) 
	and the range of the results is [-pi,+pi].
	
	vpr: cross product "this * v1 = v2" is right-handed.
*/

// use the try->throw->catch -system here??? is it a reasonable solution here at all???
// use the try->throw->catch -system here??? is it a reasonable solution here at all???
// use the try->throw->catch -system here??? is it a reasonable solution here at all???

template <class TYPE1> class v3d
{
	public:
	
	TYPE1 data[3];		// public is ok here, this is used everywhere...
	
	public:
	
	v3d(void) { }
	
	v3d(const TYPE1 * p1)
	{
		for (i32s n1 = 0;n1 < 3;n1++) data[n1] = p1[n1];
	}
	
	v3d(const TYPE1 * p1, const TYPE1 * p2)
	{
		for (i32s n1 = 0;n1 < 3;n1++) data[n1] = p2[n1] - p1[n1];
	}
	
	v3d(TYPE1 p1, TYPE1 p2, TYPE1 p3)
	{
		data[0] = p1;
		data[1] = p2;
		data[2] = p3;
	}
	
	~v3d(void) { }
	
	TYPE1 len(void) const
	{
		TYPE1 sum = 0.0;
		for (i32s n1 = 0;n1 < 3;n1++) sum += data[n1] * data[n1];
		return sqrt(sum);
	}
	
	TYPE1 ang(const v3d<TYPE1> & v1) const
	{
		TYPE1 denom = len() * v1.len();
		if (denom != 0.0)
		{
			TYPE1 cosine = spr(v1) / denom;
			
			if (cosine < -1.0) cosine = -1.0;	// domain check...
			if (cosine > +1.0) cosine = +1.0;	// domain check...
			
			return acos(cosine);
		}
		else
		{
			cout << "problems: zero division in v3d<TYPE1>::ang !!!" << endl;
			return 0.0;
		}
	}
	
	TYPE1 tor(const v3d<TYPE1> & v1, const v3d<TYPE1> & v2) const
	{
		TYPE1 temp = v1.len();
		TYPE1 denom = temp * temp;
		if (denom != 0.0)
		{
			v3d<TYPE1> v3 = v2 - (v1 * (v1.spr(v2) / denom));
			v3d<TYPE1> v4 = (* this) - (v1 * (v1.spr(* this) / denom));
			return (v1.vpr(v4)).spr(v3) < 0.0 ? -v3.ang(v4) : +v3.ang(v4);
		}
		else
		{
			cout << "problems: zero division in v3d<TYPE1>::tor !!!" << endl;
			return 0.0;
		}
	}
	
	TYPE1 spr(const v3d<TYPE1> & v1) const
	{
		TYPE1 sum = 0.0;
		for (i32s n1 = 0;n1 < 3;n1++) sum += data[n1] * v1.data[n1];
		return sum;
	}
	
	v3d<TYPE1> vpr(const v3d<TYPE1> & v1) const
	{
		v3d<TYPE1> v2;
		v2.data[0] = data[1] * v1.data[2] - data[2] * v1.data[1];
		v2.data[1] = data[2] * v1.data[0] - data[0] * v1.data[2];
		v2.data[2] = data[0] * v1.data[1] - data[1] * v1.data[0];
		return v2;
	}
	
	v3d<TYPE1> operator+(const v3d<TYPE1> & v1) const
	{
		v3d<TYPE1> v2;
		for (i32s n1 = 0;n1 < 3;n1++) v2.data[n1] = data[n1] + v1.data[n1];
		return v2;
	}
	
	v3d<TYPE1> operator-(const v3d<TYPE1> & v1) const
	{
		v3d<TYPE1> v2;
		for (i32s n1 = 0;n1 < 3;n1++) v2.data[n1] = data[n1] - v1.data[n1];
		return v2;
	}
	
	v3d<TYPE1> operator*(TYPE1 p1) const
	{
		v3d<TYPE1> v2;
		for (i32s n1 = 0;n1 < 3;n1++) v2.data[n1] = data[n1] * p1;
		return v2;
	}
	
	v3d<TYPE1> operator/(TYPE1 p1) const
	{
		v3d<TYPE1> v2;
		for (i32s n1 = 0;n1 < 3;n1++) v2.data[n1] = data[n1] / p1;
		return v2;
	}
	
	TYPE1 & operator[](i32s p1) const
	{
		return (TYPE1 &) data[p1];
	}
	
	friend ostream & operator<<(ostream & p1, const v3d<TYPE1> & p2)
	{
		p1 << "x = " << p2.data[0] << ", y = " << p2.data[1] << ", z = " << p2.data[2];
		return p1;
	}
};

/*################################################################################################*/

/**	A function to convert cartesian coordinates into polar ones.
	
	ang1: 0.0 - 180.0 degrees, starting from the z-axis.
	ang2: 0.0 - 360.0 degrees, from the x-axis, to the direction of y-axis.
*/

// those domain checks are quite useful here...
// those domain checks are quite useful here...
// those domain checks are quite useful here...

template <class TYPE1> void crt2pol(TYPE1 * crt, TYPE1 * pol)
{
	pol[0] = sqrt((crt[0] * crt[0]) + (crt[1] * crt[1]) + (crt[2] * crt[2]));
	TYPE1 tmp1 = crt[2] / pol[0];
	
	if (tmp1 < -1.0) tmp1 = -1.0;		// domain check...
	if (tmp1 > +1.0) tmp1 = +1.0;		// domain check...
	
	pol[1] = acos(tmp1);
	TYPE1 tmp2 = sin(pol[1]);
	if (tmp2 != 0.0)
	{
		TYPE1 tmp3 = pol[0] * tmp2;
		tmp1 = crt[0] / (tmp3);
		
		if (tmp1 < -1.0) tmp1 = -1.0;	// domain check...
		if (tmp1 > +1.0) tmp1 = +1.0;	// domain check...
		
		TYPE1 ang1 = acos(tmp1);
		tmp1 = crt[1] / (tmp3);
		
		if (tmp1 < -1.0) tmp1 = -1.0;	// domain check...
		if (tmp1 > +1.0) tmp1 = +1.0;	// domain check...
		
		TYPE1 ang2 = asin(tmp1);
		pol[2] = (ang2 < 0.0 ? 2.0 * M_PI - ang1 : ang1);
	}
	else pol[2] = 0.0;	// actually this is not defined???
}

/**	A function to convert polar coordinates into cartesian ones.
	
	ang1: 0.0 - 180.0 degrees, starting from the z-axis.
	ang2: 0.0 - 360.0 degrees, from the x-axis, to the direction of y-axis.
*/

template <class TYPE1> void pol2crt(TYPE1 * pol, TYPE1 * crt)
{
	crt[0] = pol[0] * sin(pol[1]) * cos(pol[2]);
	crt[1] = pol[0] * sin(pol[1]) * sin(pol[2]);
	crt[2] = pol[0] * cos(pol[1]);
}

/*################################################################################################*/

#endif	// V3D_H

// eof
