// PLANE.H : a classes for drawing colored 3D-planes and doing volume rendering.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef PLANE_H
#define PLANE_H

#include "appconfig.h"

struct cp_param;
class color_plane;

class color_plane_object;
class volume_rendering_object;

/*################################################################################################*/

#include "camera.h"

/*################################################################################################*/

struct cp_param
{
	project * prj;
	const obj_loc_data * data;	// for color_plane only!!!
	const dummy_object * name;	// for color_plane only!!!
	
	bool transparent;
	bool automatic_cv2;
	
	i32s np; fGL dim;
	
	engine * ref;
	ValueFunction * vf;
	ColorFunction * cf;
	
	fGL cvalue1;
	fGL cvalue2;
	fGL alpha;
};

class color_plane
{
	private:
	
	project * prj;
	const obj_loc_data * data;
	const dummy_object * name;
	
	bool transparent;
	bool automatic_cv2;
	
	i32s np; fGL dim;
	
	engine * ref;
	ValueFunction * GetValue;
	ColorFunction * GetColor;
	
	fGL cvalue1;
	fGL cvalue2;
	fGL alpha;
	
	/*################*/
	/*################*/
	
	fGL * dist;
	
	fGL * color4;
	fGL * point3;
	
	v3d<fGL> xdir;
	
	public:
	
	color_plane(cp_param &);
	~color_plane(void);
	
	void Update(void);
	void Render(void);
	
	protected:
	
	void SetDimension(fGL);
	void GetCRD(i32s *, fGL *);
};

/*################################################################################################*/

class color_plane_object : public smart_object
{
	private:
	
	color_plane * cp;
	
	engine * copy_of_ref;
	char * object_name;
	
	public:
	
	color_plane_object(const object_location &, cp_param &, const char *);
	~color_plane_object(void);
	
	engine * GetRef(void) { return copy_of_ref; }
	const char * GetObjectName(void) { return object_name; }	// virtual
	
	void CameraEvent(const camera &) { }	// virtual
	
	bool BeginTransformation(void) { return false; }	// virtual
	bool EndTransformation(void) { return false; }		// virtual
	
	void OrbitObject(const fGL *, const camera &);			// virtual
	void RotateObject(const fGL *, const camera &);			// virtual
	
	void TranslateObject(const fGL *, const obj_loc_data *);	// virtual
	
	void Render(void) { cp->Render(); }	// virtual
	void Update(void) { cp->Update(); }
};

/*################################################################################################*/

class volume_rendering_object : public smart_object
{
	private:
	
	fGL thickness;
	vector<color_plane *> cp_vector;
	vector<obj_loc_data *> data_vector;
	
	void * copy_of_ref;
	char * object_name;
	
	public:
	
	volume_rendering_object(const object_location &, cp_param &, i32s, fGL, camera &, const char *);
	~volume_rendering_object(void);
	
	void * GetRef(void) { return copy_of_ref; }
	const char * GetObjectName(void) { return object_name; }	// virtual

	void CameraEvent(const camera &);	// virtual
	
	bool BeginTransformation(void) { return false; }	// virtual
	bool EndTransformation(void) { return false; }		// virtual
	
	void OrbitObject(const fGL *, const camera &);			// virtual
	void RotateObject(const fGL *, const camera &);			// virtual
	
	void TranslateObject(const fGL *, const obj_loc_data *);	// virtual
	
	void Render(void);	// virtual
	void Update(void);
};

/*################################################################################################*/

#endif	// PLANE_H

// eof
