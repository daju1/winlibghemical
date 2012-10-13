// CAMERA.CPP

// Copyright (C) 1998 Tommi Hassinen, Jarno Huuskonen, Mike Cruz.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "camera.h"	// config.h is here -> we get ENABLE-macros here...

#include "project.h"
#include "views.h"

#include "appdefine.h"

//#include "time.h"	// for frames-per-second counter...
//#include "sys/time.h"	// for frames-per-second counter...

#include <strstream>
using namespace std;

/*################################################################################################*/

camera::camera(const object_location & p1, fGL p2, project * p3) : dummy_object(p1)
{
	prj = p3;
	
	my_c_number = prj->GetNewCameraID();
	graphics_view_counter = 1;
	
	my_object_name = new char[64];
	ostrstream ons(my_object_name, 64);
	ons << "camera #" << my_c_number << ends;
	
	focus = p2;
	clipping = 0.90;
	
	use_local_lights = true;
	use_global_lights = true;
	
	ortho = false;
	stereo_mode = false;
	stereo_relaxed = false;
	slider = 0.60;
	rang = -0.25;
	
	GetLocDataRW()->crd[2] = -focus;
}

// this is only partial because only camera::OrbitObject() needs this!!!!!!!
// this is only partial because only camera::OrbitObject() needs this!!!!!!!
// this is only partial because only camera::OrbitObject() needs this!!!!!!!

camera::camera(const camera & p1) : dummy_object(* p1.ol)
{
	my_object_name = NULL;
}

camera::~camera(void)
{
	if (my_object_name != NULL) delete[] my_object_name;
}

bool camera::CopySettings(const camera * p1)
{
	ol_static * ref = dynamic_cast<ol_static *>(ol);
	if (ref == NULL) return false;
	
	// now that we have verified that we can modify the ol-object, we will copy the settings...
	
	focus = p1->focus; clipping = p1->clipping;
	for (i32s n1 = 0;n1 < 3;n1++) GetLocDataRW()->crd[n1] = p1->GetLocData()->crd[n1];
	GetLocDataRW()->zdir = p1->GetLocData()->zdir;
	GetLocDataRW()->ydir = p1->GetLocData()->ydir;
	
	// the local lights that are attached to the camera are not moved?!?!?!
	// the local lights that are attached to the camera are not moved?!?!?!
	// the local lights that are attached to the camera are not moved?!?!?!
	
	return true;
}

// when rotating or translating a camera we must make all operations in reversed directions
// and also drag all those local lights along...

void camera::OrbitObject(const fGL * ang, const camera & cam)
{
	fGL tmp_ang[3];
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		tmp_ang[n1] = -ang[n1];
	}
	{
	for (i32u n1 = 0;n1 < prj->light_vector.size();n1++)
	{
		if (prj->light_vector[n1]->owner != this) continue;
		prj->light_vector[n1]->OrbitObject(tmp_ang, cam);
	}
	}
	
	dummy_object::OrbitObject(tmp_ang, cam);
	
	DoCameraEvents();
}

// when rotating a camera, we must also make the local lights to orbit around the camera.
// dummy_object::OrbitObject() will orbit the lights around the camera's focus point, so here
// we must set the camera focus to zero before making the transformation...

void camera::RotateObject(const fGL * ang, const camera & cam)
{
	fGL tmp_ang[3];
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		tmp_ang[n1] = -ang[n1];
	}
	
	camera tmp_cam = cam; tmp_cam.focus = 0.0;{
	for (i32u n1 = 0;n1 < prj->light_vector.size();n1++)
	{
		if (prj->light_vector[n1]->owner != this) continue;
		prj->light_vector[n1]->OrbitObject(tmp_ang, tmp_cam);
	}
	}
	
	dummy_object::RotateObject(tmp_ang, cam);
	
	DoCameraEvents();
}

void camera::TranslateObject(const fGL * dst, const obj_loc_data * data)
{
	fGL tmp_dst[3];
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		tmp_dst[n1] = -dst[n1];
	}
	{
	for (i32u n1 = 0;n1 < prj->light_vector.size();n1++)
	{
		if (prj->light_vector[n1]->owner != this) continue;
		prj->light_vector[n1]->TranslateObject(tmp_dst, data);
	}
	}
	
	dummy_object::TranslateObject(tmp_dst, data);
	
	DoCameraEvents();
}

/// This will draw the contents of a window; also handles selection and some mouse-tool-related stuff.

void camera::RenderWindow(graphics_view * gv, rmode rm, int x, int y)
{
	if (gv->size[0] < 0) return;	// skip rendering if invalid window!!!
	
// A SIMPLE FRAMES-PER-SECOND COUNTER FOR GRAPHICS OPTIMIZATION
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/*	const int numframes = 30;
	static int framecounter = 0;
	static double timecounter = 0.0;
	static long prevtime = 0;
	timeval tv; gettimeofday(& tv, NULL);
	long currtime = tv.tv_usec;		// according to manpages, this should be in microseconds???
	long difftime = currtime - prevtime;	// the CLOCKS_PER_SEC factor works a lot better than 1.0e-06
	prevtime = currtime;
	if (difftime > 0)	// skip the frame in timer overflow situation!!!
	{
		double t_secs = (double) difftime / (double) CLOCKS_PER_SEC;	// time unit???
		framecounter++; timecounter += t_secs;
		if (framecounter >= numframes)
		{
			cout << "frame rate = " << ((double) framecounter / timecounter) << " FPS   (";
			cout << framecounter << " frames in " << timecounter << " seconds)." << endl;
			framecounter = 0; timecounter = 0.0;
		}
	}	*/
// A SIMPLE FRAMES-PER-SECOND COUNTER FOR GRAPHICS OPTIMIZATION
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// projection matrix is now initialized. if this is a selection operation, we must do
	// some furter manipulations (gluPickMatrix() is used to "zoom" into the selection area).
	
	if (rm == Draw || rm == Erase || rm == Select || rm == Measure)
	{
		glRenderMode(GL_SELECT);
		iGLs vp[4]; glGetIntegerv(GL_VIEWPORT, vp);	// is this ok??? why not use the values directly?!?!!?
		
		// is this dependent on screen resolution or what???
		// there are some problems with selection in wireframe models...
		// GRH: This seems to help a bit, but the wireframe 
		// is stil "clipped" short, so you can't pick atoms
		// too far back
		if (gv->render != RENDER_WIREFRAME)
		  gluPickMatrix(x, vp[3] - y, 2.0, 2.0, vp);	// is 2.0 ok???
		else
		  gluPickMatrix(x, vp[3] - y, 5.0, 5.0, vp);
	}
	
	bool accum = gv->accumulate;
	if (accum) { glClear(GL_ACCUM_BUFFER_BIT); /*prj->UpdateAccumValues();*/ }
	else glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// the stereo-modes related stuff start here...
	// the stereo-modes related stuff start here...
	// the stereo-modes related stuff start here...
	
	// the stereo modes are implemented as a loop, that is cycled either once or twice.
	
	i32s width = gv->size[0]; if (stereo_mode && stereo_relaxed) width /= 2;
	i32s height = gv->size[1]; i32s shift = 0;
	
	fGL aspect = (fGL) width / (fGL) height;
	fGL fovy = (aspect > 1.0 ? 45.0 / aspect : 45.0);
	
	gv->range[1] = focus * tan(M_PI * fovy / 360.0);
	gv->range[0] = aspect * gv->range[1];
	
	i32s stereo_count = (stereo_mode ? 2 : 1);
	for (i32s stereo_loop = 0;stereo_loop < stereo_count;stereo_loop++)
	{
		glViewport(shift, 0, width, height);
		if (stereo_mode && stereo_relaxed) shift += width;	// this will shift our viewport right at the next round...
		
		fGL near_ = (1.0 - clipping) * focus;
		fGL far_ = (1.0 + clipping) * focus;
		
		if (!ortho) gluPerspective(fovy, aspect, near_, far_);
		else glOrtho(-gv->range[0], +gv->range[0], -gv->range[1], +gv->range[1], near_, far_);
		
		const obj_loc_data * loc1 = GetLocData();
		
		glMatrixMode(GL_MODELVIEW); glLoadIdentity();
		v3d<fGL> target = v3d<fGL>(loc1->crd) + (loc1->zdir * focus);
		
		const fGL * r1; const fGL * r2; const fGL * r3;
		
		if (!stereo_mode)
		{
			// if not a stereo mode, then just use normal camera settings...
			
			r1 = loc1->crd;		// eye coordinates
			r2 = target.data;	// target coordinates
			r3 = loc1->ydir.data;	// y-direction vector
		}
		else
		{
			// for all stereo modes, translate the camera along camera's x-axis.
			// y-direction will remain constant, but other directions won't. does it matter???
			
			fGL displacement = rang / 20.0;
			if (!stereo_loop) displacement = -displacement;
			v3d<fGL> xdir = loc1->ydir.vpr(loc1->zdir);
			
			// displace each eye coordinate using camera's x-axis -> will be independent on camera's orientation!!!
			// displace each eye coordinate using camera's x-axis -> will be independent on camera's orientation!!!
			// displace each eye coordinate using camera's x-axis -> will be independent on camera's orientation!!!
			
			static fGL tmp_crd[3];
			tmp_crd[0] = loc1->crd[0] + displacement * xdir.data[0];
			tmp_crd[1] = loc1->crd[1] + displacement * xdir.data[1];
			tmp_crd[2] = loc1->crd[2] + displacement * xdir.data[2];
			
			r1 = tmp_crd;		// eye coordinates
			r2 = target.data;	// target coordinates
			r3 = loc1->ydir.data;	// y-direction vector
			
			if (!stereo_relaxed)
			{
				// for the red-blue stereo mode, set the color masks...
				
				if (!stereo_loop) glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_TRUE);	// left_eye - red
				else glColorMask(GL_FALSE,GL_FALSE,GL_TRUE,GL_TRUE);			// right_eye - blue
				
				// ...and clear the depth buffer!!!
				
				glClear(GL_DEPTH_BUFFER_BIT);
			}
		}
		
		gluLookAt(r1[0], r1[1], r1[2], r2[0], r2[1], r2[2], r3[0], r3[1], r3[2]);
		
		if (stereo_mode && stereo_relaxed)
		{
			// this will do the other displacement.
			// i don't understand *how* this works, but it works!  :)  TH
			
			fGL displacement = slider / 10.0;
			if (!stereo_loop) displacement = -displacement;
			else displacement *= 2.0;
			
			glMatrixMode(GL_PROJECTION);
			glTranslatef(displacement, 0.0, 0.0);
			glMatrixMode(GL_MODELVIEW);
		}
		
		// now, it's time to set up the lights...
		
		for (i32u n1 = 0;n1 < gv->GetProject()->light_vector.size();n1++)
		{
			camera * owner = gv->GetProject()->light_vector[n1]->owner;
			if (owner != NULL && owner != this) continue;
			gv->GetProject()->light_vector[n1]->SetupLocation();
		}
		
		// here, we are finally ready to actually render the view. for simple non-stereo views, just erase the background
		// by calling glClear(), and then call prj->Render() to render the view. For relaxed-eye stereo views, two viewports
		// are used and the view is therefore rendered twice, calling glClear only once. TODO(???): there is also a hardware
		// stereo mode (not much supported yet, except SGI) that could be also used; that is a special graphics mode with two
		// color buffers, and glDrawBuffer(???) is called to determine which is active.
		
		// to make things more complicated, there is the "accumulation" effect that relies on glClear() call. to make all this
		// work together, we must take all glClear() and glAccum() calls out from the prj->Render, and do them here.
		
// this "accumulation" stuff (used only in gt2 so far) is not tested after stereo mode changes...
// this "accumulation" stuff (used only in gt2 so far) is not tested after stereo mode changes...
// this "accumulation" stuff (used only in gt2 so far) is not tested after stereo mode changes...

		glInitNames();
		if (transformer::transform_in_progress)
		{
			// do the "transformation" effect: first round renders non-selected atoms/bonds,
			// and the second one renders the selected ones translated/rotated as told by the mouse tool.
			
			// accumulation should be DISABLED here!!!
			
			gv->GetProject()->Render(gv, project::Transform1);
			
			glPushMatrix();
			gv->current_tool->ref_to_object->SetModelView();
			gv->GetProject()->Render(gv, project::Transform2);
			glPopMatrix();
		}
		else
		{
			// do the normal rendering...
			
			gv->GetProject()->Render(gv, project::Normal);
		}
		
		if (stereo_mode && !stereo_relaxed)
		{
			glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);		// reset the color mask...
		}
	}
	
	// end of stereo-modes related stuff...
	// end of stereo-modes related stuff...
	// end of stereo-modes related stuff...
	
	// if this was a selection operation, read the information from the selection buffer.
	
	if (rm == Draw || rm == Erase || rm == Select || rm == Measure)
	{
		i32s tmp1 = glRenderMode(GL_RENDER);
		printf("i32s tmp1(%d) = glRenderMode(GL_RENDER);\n", tmp1);
		i32s tmp2 = NOT_DEFINED;
		
		iGLu tmp3 = 0xffffffff;		// original, lowest z-value...
		//iGLu tmp3 = 0x00000000;	// modified, highest z-value...
	
		// here we examine the selection buffer and select the hit with lowest
		// z-value (since we assume that this object was visible to the user).
		
// HAS THIS SELECTION-LOGIC CHANGED LATELY?!?!?! THIS WAS DONE AT 1998 "BY THE BOOK" AND I THINK
// IT WORKED CORRECTLY. BUT NOW THE OLD CODE SEEMS TO PICK ALWAYS FROM BACK, NOT FROM FRONT?!?!?!

// 2001-june : now it works again wrong -> changed back to original settings. maybe it's now it should be...

		i32s tmp4[2] = { 0, 0 };
		while (tmp4[0] < tmp1)
		{
			if (gv->GetProject()->select_buffer[tmp4[1] + 1] < tmp3)	// original, lowest z-value...
			//if (gv->GetProject()->select_buffer[tmp4[1] + 2] > tmp3)	// modified, highest z-value...
			{
				tmp2 = tmp4[1];
				tmp3 = gv->GetProject()->select_buffer[tmp4[1] + 1];		// original, lowest z-value...
				//tmp3 = gv->GetProject()->select_buffer[tmp4[1] + 2];	// modified, highest z-value...
		printf("tmp2 = %d tmp3 = %d\n", tmp2, tmp3);
			}
			
			tmp4[0]++;
			tmp4[1] += gv->GetProject()->select_buffer[tmp4[1]] + 3;
		printf("tmp4[%d %d]\n", tmp4[0], tmp4[1]);
		}
		
		// now we copy all name records of the selected hit...
		
		vector<iGLu> name_vector;
		if (tmp2 != NOT_DEFINED)
		{
			for (i32u i1 = 0;i1 < gv->GetProject()->select_buffer[tmp2];i1++)
			{
				name_vector.push_back(gv->GetProject()->select_buffer[tmp2 + i1 + 3]);
			}
		}

for(size_t iii = 0; iii < name_vector.size(); iii++)
{
	printf("name_vector[%d] = %d %x", iii, name_vector[iii], name_vector[iii]);
}		
		// "draw"- and "erase"-events are always forwarded to the prj-object:
		// "draw"- and "erase"-events are always forwarded to the prj-object:
		// "draw"- and "erase"-events are always forwarded to the prj-object:
		
		if (rm == Draw) gv->GetProject()->DrawEvent(gv, name_vector);
		if (rm == Erase) gv->GetProject()->EraseEvent(gv, name_vector);
		
		if (rm == Measure && name_vector.size() > 1)
		{
			bool test = (name_vector[0] & GLNAME_MODEL_DEPENDENT);
			if (test)
			{
				if (gv->GetProject()->selected_object != NULL)
				{
					gv->GetProject()->selected_object = NULL;
				}
				
				gv->GetProject()->MeasureEvent(gv, name_vector);
			}
		}

		// "select"-events are forwarded only if they are MODEL_DEPENDENT:
		// "select"-events are forwarded only if they are MODEL_DEPENDENT:
		// "select"-events are forwarded only if they are MODEL_DEPENDENT:
		
		if (rm == Select && name_vector.size() > 1)
		{
			bool test = (name_vector[0] & GLNAME_MODEL_DEPENDENT);
			if (test)
			{
				if (gv->GetProject()->selected_object != NULL)
				{
					gv->GetProject()->selected_object = NULL;
				}
				
				gv->GetProject()->SelectEvent(gv, name_vector);
			}
			else
			{
				switch (name_vector[0])
				{
					case GLNAME_LIGHT:
					gv->GetProject()->SelectLight((const dummy_object *) name_vector[1]);
					break;
					
					case GLNAME_OBJECT:
					gv->GetProject()->SelectObject((const dummy_object *) name_vector[1]);
					break;
				} // switch
			} // test
		} // select
	}
}

// * how to use clipping planes (to make comparison easier) ???
// * how to use clipping planes (to make comparison easier) ???
// * how to use clipping planes (to make comparison easier) ???

void camera::RenderObjects(graphics_view * gv)
{
	for (i32u n1 = 0;n1 < gv->GetProject()->light_vector.size();n1++)
	{
		camera * owner = gv->GetProject()->light_vector[n1]->owner;
		if (owner != NULL && owner != this) continue;
		gv->GetProject()->light_vector[n1]->Render();
	}
	
	for (/*i32u*/ n1 = 0;n1 < gv->GetProject()->object_vector.size();n1++)
	{
		if (gv->GetProject()->object_vector[n1]->transparent) continue;
		gv->GetProject()->object_vector[n1]->Render();
	}
}

void camera::DoCameraEvents(void)
{
	list<smart_object *>::iterator so_it;
	for (so_it = obj_list.begin();so_it != obj_list.end();so_it++)
	{
		(* so_it)->CameraEvent(* this);
	}
}

/*################################################################################################*/

const fGL light::def_amb_comp[4] = { 0.1, 0.1, 0.1 };
const fGL light::def_diff_comp[4] = { 0.8, 0.8, 0.8 };
const fGL light::def_spec_comp[4] = { 0.8, 0.8, 0.8 };

const light_components light::def_components =
{
	(fGL *) light::def_amb_comp,
	(fGL *) light::def_diff_comp,
	(fGL *) light::def_spec_comp
};

light::light(const object_location & p1, const light_components & p2) : dummy_object(p1)
{
	owner = NULL;
	number = NOT_DEFINED;
	InitComponents(& p2);
}

light::~light(void)
{
	delete[] amb_comp;
	delete[] diff_comp;
	delete[] spec_comp;
}

void light::InitComponents(const light_components * p1)
{
	amb_comp = new fGL[4];
	diff_comp = new fGL[4];
	spec_comp = new fGL[4];
	
	for (i32s n1 = 0;n1 < 4;n1++)
	{
		amb_comp[n1] = p1->amb_comp[n1];
		diff_comp[n1] = p1->diff_comp[n1];
		spec_comp[n1] = p1->spec_comp[n1];
	}
}

/*################################################################################################*/

fGL spot_light::size = 0.05;

fGL spot_light::shade1[3] = { 0.0, 0.0, 0.7 };
fGL spot_light::shade2[3] = { 0.0, 0.0, 0.9 };

fGL spot_light::bulb_on[3] = { 1.0, 1.0, 0.5 };
fGL spot_light::bulb_off[3] = { 0.5, 0.5, 0.5 };

const fGL spot_light::def_cutoff = 45.0;
const fGL spot_light::def_exponent = 32.0;

spot_light::spot_light(const object_location & p1, i32s myid, const light_components & p2, fGL p3, fGL p4) : light(p1, p2)
{
	GetLocDataRW()->crd[3] = 1.0;		// make this a spotlight!!!
	cutoff = p3; exponent = p4;
	
	my_object_name = new char[64];
	ostrstream ons(my_object_name, 64);
	ons << "light (spotlight, object #" << myid << ")" << ends;
}

spot_light::~spot_light(void)
{
	delete[] my_object_name;
}

void spot_light::SetupProperties(void)
{
	glLightf((GLenum) number, GL_SPOT_CUTOFF, cutoff);
	glLightf((GLenum) number, GL_SPOT_EXPONENT, exponent);
	
	glLightfv((GLenum) number, GL_AMBIENT, amb_comp);
	glLightfv((GLenum) number, GL_DIFFUSE, diff_comp);
	glLightfv((GLenum) number, GL_SPECULAR, spec_comp);
}

void spot_light::SetupLocation(void)
{
	glLightfv((GLenum) number, GL_POSITION, GetLocData()->crd);
	glLightfv((GLenum) number, GL_SPOT_DIRECTION, GetLocData()->zdir.data);
}

#define SHADE_ANGLE (22.5 / 180.0 * M_PI)

void spot_light::Render(void)
{
	fGL r1 = tan(SHADE_ANGLE) * size;
	fGL tmp1 = 1.0 / (sin(SHADE_ANGLE) + 1.0);
	fGL tmp2 = 1.2 * size;
	
	fGL r2 = (1.0 - tmp1) * tmp2;
	fGL r3 = tmp1 * tmp2;
	
	const dummy_object * name = this;
	glPushName(GLNAME_LIGHT); glPushName((iGLu) name);
	
	glPushMatrix(); SetModelView();
	
	if (cutoff < 180.0)
	{
		glPushMatrix();
		glTranslatef(0.0, 0.0, -r3);
		glBegin(GL_TRIANGLES);
		
		for (i32s n1 = 0;n1 < 8;n1++)
		{
			if (n1 & 1) glColor3fv(shade1);
			else glColor3fv(shade2);
			
			fGL ang1 = M_PI * (fGL) n1 / 4.0;
			fGL ang2 = M_PI * (fGL) (n1 + 1) / 4.0;
			
			glVertex3f(r1 * cos(ang1), r1 * sin(ang1), size);
			glVertex3f(r1 * cos(ang2), r1 * sin(ang2), size);
			glVertex3f(0.0, 0.0, 0.0);
		}
		
		glEnd();	// GL_TRIANGLES
		glPopMatrix();
	}
	
	if (glIsEnabled((GLenum) number)) glColor3fv(bulb_on);
	else glColor3fv(bulb_off);
	
	GLUquadricObj * qo = gluNewQuadric();
	gluQuadricDrawStyle(qo, (GLenum) GLU_FILL);
	gluSphere(qo, 0.95 * r2, 8, 4);
	gluDeleteQuadric(qo);
	
	glPopMatrix();
	
	glPopName();
	glPopName();
}

/*################################################################################################*/

directional_light::directional_light(const object_location & p1, i32s myid, const light_components & p2) : light(p1, p2)
{
	my_object_name = new char[64];
	ostrstream ons(my_object_name, 64);
	ons << "light (directional, object #" << myid << ")" << ends;
}

directional_light::~directional_light(void)
{
	delete[] my_object_name;
}

void directional_light::SetupProperties(void)
{
	glLightf((GLenum) number, GL_SPOT_CUTOFF, 180.0);
	glLightf((GLenum) number, GL_SPOT_EXPONENT, 0.0);
	
	glLightfv((GLenum) number, GL_AMBIENT, amb_comp);
	glLightfv((GLenum) number, GL_DIFFUSE, diff_comp);
	glLightfv((GLenum) number, GL_SPECULAR, spec_comp);
}

// the direction of a directional light is the reversed z-axis direction!!!
// the direction of a directional light is the reversed z-axis direction!!!
// the direction of a directional light is the reversed z-axis direction!!!

void directional_light::SetupLocation(void)
{
	fGL tmp_crd[4]; tmp_crd[3] = 0.0;	// make this a directional light!!!
	
	for (i32s n1 = 0;n1 < 3;n1++) tmp_crd[n1] = -GetLocData()->zdir[n1];
	glLightfv((GLenum) number, GL_POSITION, tmp_crd);
}

void directional_light::Render(void)
{
}

/*################################################################################################*/

bool transformer::transform_in_progress = false;

transformer::transformer() : dummy_object(true)
{
	target_prj = NULL;
}

transformer::~transformer(void)
{
}

void transformer::Init(project * p1)
{
	target_prj = p1;
	
	obj_loc_data * data = GetLocDataRW();
	for (i32s n1 = 0;n1 < 3;n1++) data->crd[n1] = 0.0;
	data->zdir = v3d<fGL>(0.0, 0.0, 1.0);
	data->ydir = v3d<fGL>(0.0, 1.0, 0.0);
}

void transformer::GetMatrix(fGL * p1) const
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix(); glLoadIdentity();
	SetModelView(); glGetFloatv(GL_MODELVIEW_MATRIX, p1);
	glPopMatrix();
}

bool transformer::BeginTransformation(void)
{
	transform_in_progress = true;
	target_prj->Center(this);
	return true;
}

bool transformer::EndTransformation(void)
{
	transform_in_progress = false;
	target_prj->Transform(this);
	return true;
}

void transformer::OrbitObject(const fGL * p1, const camera & p2)
{
	dummy_object::RotateObject(p1, p2);
}

void transformer::RotateObject(const fGL * p1, const camera & p2)
{
	dummy_object::OrbitObject(p1, p2);
}

void transformer::Render(void)
{
}

/*################################################################################################*/

// eof
