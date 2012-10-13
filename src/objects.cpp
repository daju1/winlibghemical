// OBJECTS.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "StdAfx.h"

#include "objects.h"	// config.h is here -> we get ENABLE-macros here...

#include "../libghemical/src/utility.h"

#include "camera.h"
#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glut.h>

#include <algorithm>
using namespace std;

/*################################################################################################*/

void SetModelView(const obj_loc_data * data)
{
	glTranslatef(data->crd[0], data->crd[1], data->crd[2]);
	
	v3d<fGL> gzd(0.0, 0.0, 1.0);
	const v3d<fGL> gyd(0.0, 1.0, 0.0);
	
	// first the global y-direction is rotated so that it is parallel to the local
	// y-direction. this rotation is done using the cross product vector of the global
	// and local y-directions, or if it has zero length, using the global x-direction.
	
	v3d<fGL> tmpv1 = gyd.vpr(data->ydir); fGL len = tmpv1.len();
	if (len == 0.0) tmpv1 = v3d<fGL>(1.0, 0.0, 0.0);
	else tmpv1 = tmpv1 / len;
	
	fGL ang1 = 180.0 * gyd.ang(data->ydir) / M_PI;
	
	glPushMatrix(); glLoadIdentity();
	glRotatef(ang1, tmpv1[0], tmpv1[1], tmpv1[2]);
	fGL rotm[16]; glGetFloatv(GL_MODELVIEW_MATRIX, rotm);
	glPopMatrix();
	
	glMultMatrixf(rotm);
	TransformVector(gzd, rotm);
	
	// after that the global z-direction is rotated so that it is parallel to the local
	// z-direction. this rotation is done using the common y-direction. direction of the
	// rotation must be checked because the range is now a full circle, 360.0 degrees...
	
	v3d<fGL> xdir = data->ydir.vpr(data->zdir);
	fGL ang2 = 180.0 * gzd.ang(data->zdir) / M_PI;
	if (gzd.spr(xdir) > 0.0) ang2 = 360.0 - ang2;
	glRotatef(ang2, 0.0, 1.0, 0.0);
}

/*################################################################################################*/

object_location::object_location(void)
{
	data = new obj_loc_data;
	data->lock_count = 1;
}

object_location::object_location(const object_location & p1)
{
	data = p1.data;
	data->lock_count++;
}

object_location::~object_location(void)
{
	data->lock_count--;
	if (!data->lock_count) delete data;
}

void object_location::SetModelView(void) const
{
	UpdateLocation();		// update the location if it's dynamic!!!
	::SetModelView(data);
}

const obj_loc_data * object_location::GetLocData(void) const
{
	return data;
}

/*################################################################################################*/

ol_static::ol_static(void) : object_location()
{
	for (i32s n1 = 0;n1 < 3;n1++) data->crd[n1] = 0.0;
	data->zdir = v3d<fGL>(0.0, 0.0, 1.0);
	data->ydir = v3d<fGL>(0.0, 1.0, 0.0);
}

ol_static::ol_static(const obj_loc_data * p1)
{
	for (i32s n1 = 0;n1 < 3;n1++) data->crd[n1] = p1->crd[n1];
	data->zdir = p1->zdir;
	data->ydir = p1->ydir;
}

ol_static::~ol_static(void)
{
}

void ol_static::UpdateLocation(void) const
{
}

object_location * ol_static::MakeCopy(void) const
{
	return new ol_static(* this);
}

/*################################################################################################*/

ol_dynamic_l1::ol_dynamic_l1(void) : object_location()
{
	UpdateLocation();
}

ol_dynamic_l1::~ol_dynamic_l1(void)
{
}

// just move the object somewhere... a dummy function for testing purposes...
// just move the object somewhere... a dummy function for testing purposes...
// just move the object somewhere... a dummy function for testing purposes...

void ol_dynamic_l1::UpdateLocation(void) const
{
	data->crd[0] += 0.01;
}

object_location * ol_dynamic_l1::MakeCopy(void) const
{
	return new ol_dynamic_l1(* this);
}

/*################################################################################################*/

dummy_object::dummy_object(bool p1)
{
	if (!p1) ol = NULL;
	else ol = new ol_static();
	
	my_id_number = NOT_DEFINED;
}

dummy_object::dummy_object(const object_location & p1)
{
	ol = p1.MakeCopy();
	
	my_id_number = NOT_DEFINED;
}

dummy_object::~dummy_object(void)
{
	if (ol != NULL) delete ol;
}

void dummy_object::SetModelView(void) const
{
	if (ol != NULL) ol->SetModelView();
}

const obj_loc_data * dummy_object::GetLocData(void) const
{
	return ol != NULL ? ol->GetLocData() : NULL;
}

obj_loc_data * dummy_object::GetLocDataRW(void) const
{
	return ol != NULL ? ol->data : NULL;
}

// this will orbit the object around the camera's focus point. the rotation is done relatively
// to the camera's direction vectors. basic idea is to make first a 4x4-transformation matrix and
// use it to manipulate the object's direction and position vectors.

void dummy_object::OrbitObject(const fGL * ang, const camera & cam)
{
	if (ol == NULL) return;
	
	const obj_loc_data * loc1 = cam.GetLocData();
	v3d<fGL> cam_xdir = loc1->ydir.vpr(loc1->zdir);
	
	v3d<fGL> tmpv1 = v3d<fGL>((fGL *) loc1->crd, GetLocData()->crd);
	v3d<fGL> tmpv2 = loc1->zdir * cam.focus;
	v3d<fGL> tmpv3 = tmpv1 - tmpv2;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); glLoadIdentity();
	glRotatef(ang[0], cam_xdir[0], cam_xdir[1], cam_xdir[2]);
	glRotatef(ang[1], loc1->ydir[0], loc1->ydir[1], loc1->ydir[2]);
	glRotatef(ang[2], loc1->zdir[0], loc1->zdir[1], loc1->zdir[2]);
	fGL rotm[16]; glGetFloatv(GL_MODELVIEW_MATRIX, rotm); glPopMatrix();
	
	obj_loc_data * loc2 = GetLocDataRW();
	v3d<fGL> xdir = loc2->ydir.vpr(loc2->zdir);
	
	TransformVector(xdir, rotm);
	TransformVector(loc2->ydir, rotm);
	
	loc2->zdir = xdir.vpr(loc2->ydir);
	loc2->zdir = loc2->zdir / loc2->zdir.len();
	loc2->ydir = loc2->ydir / loc2->ydir.len();
	
	TransformVector(tmpv3, rotm);
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		loc2->crd[n1] = loc1->crd[n1] + tmpv2[n1] + tmpv3[n1];
	}
}

// this will rotate the object relatively to the camera's direction vectors. the basic idea
// is similar to that in dummy_object::OrbitObject().

void dummy_object::RotateObject(const fGL * ang, const camera & cam)
{
	if (ol == NULL) return;
	
	const obj_loc_data * loc1 = cam.GetLocData();
	v3d<fGL> cam_xdir = loc1->ydir.vpr(loc1->zdir);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); glLoadIdentity();
	glRotatef(ang[0], cam_xdir[0], cam_xdir[1], cam_xdir[2]);
	glRotatef(ang[1], loc1->ydir[0], loc1->ydir[1], loc1->ydir[2]);
	glRotatef(ang[2], loc1->zdir[0], loc1->zdir[1], loc1->zdir[2]);
	fGL rotm[16]; glGetFloatv(GL_MODELVIEW_MATRIX, rotm); glPopMatrix();
	
	obj_loc_data * loc2 = GetLocDataRW();
	v3d<fGL> xdir = loc2->ydir.vpr(loc2->zdir);
	
	TransformVector(xdir, rotm);
	TransformVector(loc2->ydir, rotm);
	
	loc2->zdir = xdir.vpr(loc2->ydir);
	loc2->zdir = loc2->zdir / loc2->zdir.len();
	loc2->ydir = loc2->ydir / loc2->ydir.len();
}

// this will translate the object relatively to given direction vectors. this is just a simple
// summing, no more strange matrix tricks here...

void dummy_object::TranslateObject(const fGL * dst, const obj_loc_data * data)
{
	if (ol == NULL || data == NULL) return;
	
	v3d<fGL> obj_xdir = data->ydir.vpr(data->zdir);
	obj_loc_data * loc1 = GetLocDataRW();
	
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		fGL tmp1 = dst[0] * obj_xdir[n1];
		fGL tmp2 = dst[1] * data->ydir[n1];
		fGL tmp3 = dst[2] * data->zdir[n1];
		loc1->crd[n1] += tmp1 + tmp2 + tmp3;
	}
}

/*################################################################################################*/

smart_object::smart_object(void) : dummy_object(false)
{
	transparent = false;
}

smart_object::smart_object(const object_location & p1) : dummy_object(p1)
{
	transparent = false;
}

smart_object::~smart_object(void)
{
	list<camera *>::iterator c_it;
	for (c_it = cam_list.begin();c_it != cam_list.end();c_it++)
	{
		camera * cam = (* c_it);
		
		bool flag = true;
		while (flag)
		{
			list<smart_object *>::iterator so_it;
			so_it = find(cam->obj_list.begin(), cam->obj_list.end(), this);
			if (so_it != cam->obj_list.end()) cam->obj_list.erase(so_it);
			else flag = false;
		}
	}
}

void smart_object::ConnectCamera(camera & p1)
{
	camera * cam = & p1;
	
	cam_list.push_back(cam);
	cam->obj_list.push_back(this);
}

/*################################################################################################*/

// eof
