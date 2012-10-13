// PLANE.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "plane.h"	// config.h is here -> we get ENABLE-macros here...

#include "project.h"
#include "appdefine.h"

#include <fstream>
#include <strstream>
using namespace std;

/*################################################################################################*/

color_plane::color_plane(cp_param & p1)
{
	prj = p1.prj;
	data = p1.data;
	name = p1.name;
	
	transparent = p1.transparent;
	automatic_cv2 = p1.automatic_cv2;
	
	np = p1.np;
	
	// now we can allocate memory...
	
	dist = new fGL[np];
	
	color4 = new fGL[np * np * 4];
	point3 = new fGL[np * np * 3];
	
	// we must do these settings...
	// we must do these settings...
	// we must do these settings...
	
	ref = p1.ref;
	GetValue = p1.vf;
	GetColor = p1.cf;
	
	cvalue1 = p1.cvalue1;
	cvalue2 = p1.cvalue2;
	alpha = p1.alpha;
	
	// ...before trying to do this!!!
	// ...before trying to do this!!!
	// ...before trying to do this!!!
	
	SetDimension(p1.dim);
	Update();
	
	// add the primitives, if this is a transparent object...
	// add the primitives, if this is a transparent object...
	// add the primitives, if this is a transparent object...
	
	if (transparent)
	{
		for (i32s n1 = 0;n1 < np - 1;n1++)
		{
			for (i32s n2 = 0;n2 < np - 1;n2++)
			{
				i32s tmp1, tmp2, tmp3;
				
				tmp1 = n1 + 0; tmp2 = n2 + 0; tmp3 = tmp1 * np + tmp2;
				fGL * c1 = & color4[tmp3 * 4]; fGL * p1 = & point3[tmp3 * 3];
				
				tmp1 = n1 + 1; tmp2 = n2 + 0; tmp3 = tmp1 * np + tmp2;
				fGL * c2 = & color4[tmp3 * 4]; fGL * p2 = & point3[tmp3 * 3];
				
				tmp1 = n1 + 1; tmp2 = n2 + 1; tmp3 = tmp1 * np + tmp2;
				fGL * c3 = & color4[tmp3 * 4]; fGL * p3 = & point3[tmp3 * 3];
				
				tmp1 = n1 + 0; tmp2 = n2 + 1; tmp3 = tmp1 * np + tmp2;
				fGL * c4 = & color4[tmp3 * 4]; fGL * p4 = & point3[tmp3 * 3];
				
				tpd_quad_4c * tmp4;
				tmp4 = new tpd_quad_4c(c1, p1, c2, p2, c3, p3, c4, p4);
				
				transparent_primitive * tmp5;
				tmp5 = new transparent_primitive((void *) this, * tmp4);
				
				prj->AddTP((void *) this, (* tmp5));
				
				// tmp4, the tpd_quad_4c-object, will be deleted when
				// prj->RemoveAllTPs() is called for this object...
				
				delete tmp5;
			}
		}
	}
}

color_plane::~color_plane(void)
{
	if (transparent) prj->RemoveAllTPs((void *) this);
	
	delete[] dist;
	
	delete[] color4;
	delete[] point3;
}

void color_plane::Update(void)
{
	xdir = data->ydir.vpr(data->zdir);
	
	i32s tmp1[2];				// point indices...
	fGL * tmp2 = new fGL[np * np];		// color values...
	
	for (tmp1[0] = 0;tmp1[0] < np;tmp1[0]++)
	{
		for (tmp1[1] = 0;tmp1[1] < np;tmp1[1]++)
		{
			i32s index = tmp1[0] * np + tmp1[1];
			GetCRD(tmp1, & point3[index * 3]);
			
			tmp2[tmp1[0] * np + tmp1[1]] = GetValue(ref, & point3[index * 3], NULL);
		}
	}
	
	if (automatic_cv2)
	{
		f64 avrg = 0.0;
		for (i32s n1 = 0;n1 < np * np;n1++)
		{
			avrg += tmp2[n1];
		}
		
		// AUTO makes only partial correction -> just looks better and one can estimate the "bias".
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		
		cvalue2 = -0.80 * avrg / (f64) (np * np);
	}
	
	for (tmp1[0] = 0;tmp1[0] < np;tmp1[0]++)
	{
		for (tmp1[1] = 0;tmp1[1] < np;tmp1[1]++)
		{
			i32s index = tmp1[0] * np + tmp1[1];
			fGL tmp3 = tmp2[tmp1[0] * np + tmp1[1]] + cvalue2;
			GetColor((tmp3 / cvalue1), alpha, & color4[index * 4]);
		}
	}
	
	delete[] tmp2;
	
	// update the primitives, if this is a transparent object...
	// update the primitives, if this is a transparent object...
	// update the primitives, if this is a transparent object...
	
	if (transparent) prj->UpdateMPsForAllTPs((void *) this);
}

void color_plane::Render(void)
{
	glPushMatrix();
	
	glPushName(GLNAME_OBJECT);
	glPushName((iGLu) name);
	
	// if this is a transparent object, we will not render the quads here.
	// instead, the quads get rendered at project::Render(), like all other
	// transparent primitives.
	
	if (!transparent)
	{
		glBegin(GL_QUADS);
		
		for (i32s n1 = 0;n1 < np - 1;n1++)
		{
			for (i32s n2 = 0;n2 < np - 1;n2++)
			{
				i32s tmp1, tmp2, tmp3;
				
				tmp1 = n1 + 0; tmp2 = n2 + 0; tmp3 = tmp1 * np + tmp2;
				glColor4fv(& color4[tmp3 * 4]); glVertex3fv(& point3[tmp3 * 3]);
				
				tmp1 = n1 + 1; tmp2 = n2 + 0; tmp3 = tmp1 * np + tmp2;
				glColor4fv(& color4[tmp3 * 4]); glVertex3fv(& point3[tmp3 * 3]);
				
				tmp1 = n1 + 1; tmp2 = n2 + 1; tmp3 = tmp1 * np + tmp2;
				glColor4fv(& color4[tmp3 * 4]); glVertex3fv(& point3[tmp3 * 3]);
				
				tmp1 = n1 + 0; tmp2 = n2 + 1; tmp3 = tmp1 * np + tmp2;
				glColor4fv(& color4[tmp3 * 4]); glVertex3fv(& point3[tmp3 * 3]);
			}
		}
			
		glEnd();
	}
	
	SetModelView(data);
	
	fGL tmp1 = -0.55 * dim;
	fGL tmp2 = -0.45 * dim;
	
	glBegin(GL_LINES);
	glColor3f(1.0, 1.0, 1.0); glVertex3f(tmp1, tmp1, 0.0);
	glColor3f(0.0, 0.0, 1.0); glVertex3f(tmp2, tmp1, 0.0);
	
	glColor3f(1.0, 1.0, 1.0); glVertex3f(tmp1, tmp1, 0.0);
	glColor3f(0.0, 0.0, 1.0); glVertex3f(tmp1, tmp2, 0.0);
	
	glColor3f(1.0, 1.0, 1.0); glVertex3f(tmp1, tmp1, 0.0);
	glColor3f(0.0, 0.0, 1.0); glVertex3f(tmp1, tmp1, 0.1 * dim);
	glEnd();
	
	glPopName();
	glPopName();
	
	glPopMatrix();
}

void color_plane::SetDimension(fGL p1)
{
	dim = p1; fGL tmp1 = np - 1;
	for (i32s n1 = 0;n1 < np;n1++)
	{
		fGL tmp2 = (fGL) n1 / tmp1;
		dist[n1] = dim * (tmp2 - 0.5);
	}
}

void color_plane::GetCRD(i32s * p1, fGL * p2)
{
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		fGL tmp1 = xdir[n1] * dist[p1[0]];
		fGL tmp2 = data->ydir[n1] * dist[p1[1]];
		p2[n1] = data->crd[n1] + tmp1 + tmp2;
	}
}

/*################################################################################################*/

color_plane_object::color_plane_object(const object_location & p1, cp_param & p2, const char * p3) : smart_object(p1)
{
	p2.data = GetLocData();
	p2.name = (const dummy_object *) this;
	
	cp = new color_plane(p2);
	
	char buffer[1024];
	ostrstream str(buffer, sizeof(buffer));
	str << p3 << "plane" << ends;
	
	copy_of_ref = p2.ref;
	
	object_name = new char[strlen(buffer) + 1];
	strcpy(object_name, buffer);
}

color_plane_object::~color_plane_object(void)
{
	delete cp;
	delete[] object_name;
}

void color_plane_object::OrbitObject(const fGL * p1, const camera & p2)
{
	dummy_object::RotateObject(p1, p2);
	Update();
}

void color_plane_object::RotateObject(const fGL * p1, const camera & p2)
{
	dummy_object::OrbitObject(p1, p2);
	Update();
}

void color_plane_object::TranslateObject(const fGL * p1, const obj_loc_data * p2)
{
	fGL tmp1[3] = { p1[0], p1[1], p1[2] };
	
	if (p2 == GetLocData())
	{
		tmp1[0] = -tmp1[0];
		tmp1[2] = -tmp1[2];
	}
	
	dummy_object::TranslateObject(tmp1, p2);
	Update();
}

/*################################################################################################*/

volume_rendering_object::volume_rendering_object(const object_location & p1,
	cp_param & p2, i32s p3, fGL p4, camera & p5, const char * p6) : smart_object(p1)
{
	for (i32s n1 = 0;n1 < p3;n1++)
	{
		data_vector.push_back(new obj_loc_data);
		
		p2.data = data_vector.back();
		p2.name = (const dummy_object *) this;
		
		cp_vector.push_back(new color_plane(p2));
	}
	
	thickness = p4;
	ConnectCamera(p5);
	
	char buffer[1024];
	ostrstream str(buffer, sizeof(buffer));
	str << p6 << "VR" << ends;
	
	copy_of_ref = p2.ref;
	
	object_name = new char[strlen(buffer) + 1];
	strcpy(object_name, buffer);
	
	CameraEvent(p5);
}

volume_rendering_object::~volume_rendering_object(void)
{
	for (i32u n1 = 0;n1 < cp_vector.size();n1++)
	{
		delete cp_vector[n1];
		delete data_vector[n1];
	}
	
	delete[] object_name;
}

void volume_rendering_object::CameraEvent(const camera & p1)
{
	v3d<fGL> newz = v3d<fGL>(GetLocData()->crd, p1.GetLocData()->crd);
	newz = newz / newz.len();
	
	v3d<fGL> newx = newz.vpr(p1.GetLocData()->ydir);
	newx = newx / newx.len();
	
	v3d<fGL> newy = newx.vpr(newz);
	newy = newy / newy.len();
	
	GetLocDataRW()->zdir = newz;
	GetLocDataRW()->ydir = newy;
	
	for (i32u n1 = 0;n1 < cp_vector.size();n1++)
	{
		fGL z1 = (fGL) n1 / (fGL) (cp_vector.size() - 1);
		fGL z2 = 2.0 * (z1 - 0.5) * thickness;
		
		v3d<fGL> pv = v3d<fGL>(GetLocData()->crd);
		pv = pv + (GetLocData()->zdir * z2);
		
		data_vector[n1]->crd[0] = pv.data[0];
		data_vector[n1]->crd[1] = pv.data[1];
		data_vector[n1]->crd[2] = pv.data[2];
		
		data_vector[n1]->zdir = GetLocData()->zdir;
		data_vector[n1]->ydir = GetLocData()->ydir;
	}
	
	Update();	// update the planes!!!!!
}

void volume_rendering_object::OrbitObject(const fGL *, const camera &)
{
}

void volume_rendering_object::RotateObject(const fGL *, const camera &)
{
}

void volume_rendering_object::TranslateObject(const fGL * p1, const obj_loc_data * p2)
{
	fGL tmp1[3] = { p1[0], p1[1], p1[2] };
	
	if (p2 == GetLocData())
	{
		tmp1[0] = -tmp1[0];
		tmp1[2] = -tmp1[2];
	}
	
	dummy_object::TranslateObject(tmp1, p2);
	Update();
}

void volume_rendering_object::Render(void)
{
	for (i32u n1 = 0;n1 < cp_vector.size();n1++)
	{
		cp_vector[n1]->Render();
	}
}

void volume_rendering_object::Update(void)
{
	for (i32u n1 = 0;n1 < cp_vector.size();n1++)
	{
		cp_vector[n1]->Update();
	}
}

/*################################################################################################*/

// eof
