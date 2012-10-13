// SURFACE.H : a class for drawing colored 3D-surfaces.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef SURFACE_H
#define SURFACE_H

#include "appconfig.h"

struct cs_line;
struct cs_triangle;

struct cs_vertex;

struct cs_param;
class color_surface;

/*################################################################################################*/

#include "camera.h"

#include <GL/gl.h>

#include <vector>
#include <algorithm>
using namespace std;

/*################################################################################################*/

struct cs_line
{
	i32s index[2];
	
	void Arrange(void)
	{
		sort(index, index + 2);
	}
	
	bool operator==(const cs_line & p1) const
	{
		if (index[0] != p1.index[0]) return false;
		if (index[1] != p1.index[1]) return false;
		return true;
	}
	
	bool operator<(const cs_line & p1) const
	{
		if (index[0] != p1.index[0]) return index[0] < p1.index[0];
		if (index[1] != p1.index[1]) return index[1] < p1.index[1];
		return false;
	}
};

struct cs_triangle
{
	i32s index[3];
};

struct cs_vertex
{
	fGL crd[3]; fGL normal[3];
	fGL color[4]; fGL cvalue; i32s id;
	
	void Render(bool normal_flag)
	{
		glColor4fv(color);
		if (normal_flag) glNormal3fv(normal);
		glVertex3fv(crd);
	}
};

/*################################################################################################*/

struct cs_param
{
	project * prj;
	const obj_loc_data * data;	// for color_surface only!!!
	const dummy_object * name;	// for color_surface only!!!
	
	bool transparent;
	bool automatic_cv2;
	bool wireframe;
	
	i32s * np; fGL * dim;
	
	engine * ref;
	ValueFunction * vf1;
	ValueFunction * vf2;
	ColorFunction * cf;
	
	fGL svalue;
	fGL cvalue1;
	fGL cvalue2;
	fGL alpha;
	
	fGL toler;
	i32s maxc;
	
	cs_param * next;
};

class color_surface
{
	private:
	
	project * prj;
	const obj_loc_data * data;
	const dummy_object * name;
	
	bool transparent;
	bool automatic_cv2;
	bool wireframe;
	
	i32s np[3]; fGL dim[3];
	
	engine * ref;
	ValueFunction * GetSurfaceValue;
	ValueFunction * GetColorValue;
	ColorFunction * GetColor;
	
	fGL svalue;
	fGL cvalue1;
	fGL cvalue2;
	fGL alpha;
	
	fGL tolerance;
	i32s max_cycles;
	
	/*################*/
	/*################*/
	
	fGL * dist[3];
	vector<bool> grid;
	
	i32s range[3][2];
	vector<cs_vertex> vdata;
	vector<cs_triangle> tdata;
	vector<cs_line> ldata;
	
	v3d<fGL> xdir;
	
	f64 avrg; i32u acnt;
	
	public:
	
	color_surface(cs_param &);
	~color_surface(void);
	
	void Update(void);
	void Render(void);
	
	protected:
	
	void SetDimension(fGL *);
	void GetCRD(i32s *, fGL *);
	
	void Interpolate(i32s, i32s *, i32s *);
};

/*################################################################################################*/

class color_surface_object : public smart_object
{
	private:
	
	project * prj; fGL dim[3];
	vector<color_surface *> cs_vector;
	
	bool transform_in_progress;
	
	engine * copy_of_ref;
	char * object_name;
	
	public:
	
	color_surface_object(const object_location &, cs_param &, const char *);
	~color_surface_object(void);
	
	engine * GetRef(void) { return copy_of_ref; }
	const char * GetObjectName(void) { return object_name; }	// virtual
	
	void CameraEvent(const camera &) { }	// virtual
	
	bool BeginTransformation(void);			// virtual
	bool EndTransformation(void);			// virtual
	
	void OrbitObject(const fGL *, const camera &);			// virtual
	void RotateObject(const fGL *, const camera &);			// virtual
	
	void TranslateObject(const fGL *, const obj_loc_data *);	// virtual
	
	void Render(void);	// virtual
	void Update(void);
};

/*################################################################################################*/

#endif	// SURFACE_H

// eof
