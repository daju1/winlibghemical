// VIEWS.CPP

// Copyright (C) 1998 Tommi Hassinen, Jarno Huuskonen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "views.h"	// config.h is here -> we get ENABLE-macros here...

#include "../libghemical/src/engine.h"
#include "../libghemical/src//utility.h"

#include "color.h"
#include "camera.h"
#include "project.h"

#include "views.h"

#include "project.h"

#include "appdefine.h"

#include "../../../consol/filedlg.h"



#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <strstream>
#include <algorithm>
using namespace std;


/*################################################################################################*/

view::view(void)
{
}

view::~view(void)
{
}

/*################################################################################################*/

project_view::project_view() : view()
{
}

project_view::~project_view(void)
{
}

/*################################################################################################*/

mouse_tool::state ogl_view::state = mouse_tool::Unknown;
mouse_tool::button ogl_view::button = mouse_tool::None;

bool ogl_view::shift_down = false;
bool ogl_view::ctrl_down = false;

molaxis_tool ogl_view::tool_molaxis;

draw_tool ogl_view::tool_draw;
erase_tool ogl_view::tool_erase;

select_tool ogl_view::tool_select;
select_tool::select_mode ogl_view::select_mode = select_tool::Atom;

zoom_tool ogl_view::tool_zoom;
clipping_tool ogl_view::tool_clipping;

translate_xy_tool ogl_view::tool_translate_xy;
translate_z_tool ogl_view::tool_translate_z;

orbit_xy_tool ogl_view::tool_orbit_xy;
orbit_z_tool ogl_view::tool_orbit_z;

rotate_xy_tool ogl_view::tool_rotate_xy;
rotate_z_tool ogl_view::tool_rotate_z;

measure_tool ogl_view::tool_measure;
measure_tool::measure_mode ogl_view::measure_mode = measure_tool::Bond;

// tool_orbit_xy is the default tool...
// tool_orbit_xy is the default tool...
// tool_orbit_xy is the default tool...

mouse_tool * ogl_view::current_tool = & ogl_view::tool_orbit_xy;

ogl_view::ogl_view() : view()
{
}

ogl_view::~ogl_view(void)
{
}

// later, before stereo views were added, glViewport() was called here...
// but now, this is no longer the practice; now call glViewport() when rendering!!!

void ogl_view::SetSize(i32s p1, i32s p2)
{
	size[0] = p1; size[1] = p2;
	if (!size[0] || !size[1]) size[0] = NOT_DEFINED;
	Update(false);
}

// 2001-12-28 : tried to add support for both bitmap and line-based fonts but it's not ready yet.
// the font scaling and even orientation seems to be very different... there seems to be in use
// some versions of GLUT that lack the bitmap fonts and related functions. hopefully there remains
// at least some sane way to render text using OpenGL/GLUT; at the moment the support for line
// fonts here isveeery broken...  :(

// 2001-12-29 : the line fonts are now more or less readable everywhere, but 3D text still is
// not correctly aligned...

const fGL stroke_font_scale = 0.1;

i32s ogl_view::GetGlutStringWidth(const char * str, void * font)
{
//font = NULL;	// uncomment this to try the line fonts...
	bool use_bitmap_font = (font != NULL ? true : false);
	
	i32u count = 0; i32s width = 0;
	while (count < strlen(str))
	{
		if (use_bitmap_font) width += glutBitmapWidth(font, str[count++]);
		else width += glutStrokeWidth(GLUT_STROKE_ROMAN, str[count++]);
	}
	
	if (!use_bitmap_font) width = (i32s) (width * stroke_font_scale);
	return width;
}

void ogl_view::WriteGlutString2D(const char * str, fGL x, fGL y, void * font)
{
	glPushMatrix();
	glLoadIdentity();
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); glLoadIdentity();
	gluOrtho2D(0, size[0], 0, size[1]);
	
	WriteGlutString3D(str, x, y, 0.0, NULL, font);
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	glPopMatrix();
}

void ogl_view::WriteGlutString3D(const char * str, fGL x, fGL y, fGL z, const obj_loc_data * eye, void * font)
{
//font = NULL;	// uncomment this to try the line fonts...
	bool use_bitmap_font = (font != NULL ? true : false);
	
	// text centering?!?!?! add a new flag???
	// text centering?!?!?! add a new flag???
	// text centering?!?!?! add a new flag???
	
	if (use_bitmap_font)
	{
		glDisable(GL_DEPTH_TEST);
		
		glRasterPos3f(x, y, z);
		
		i32u count = 0;
		while (count < strlen(str))
		{
			glutBitmapCharacter(font, str[count++]);
		}
		
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
		
		glPushMatrix();
		glTranslatef(x, y, z);
		
	// here the text should be oriented to the "eye" coordinates if they are given...
	// it's possible but tricky; something similar than in SetModelView() at objects.cpp
		
		fGL font_scale = stroke_font_scale;		// the 2D and 3D texts must be scaled very
		if (eye != NULL) font_scale /= 500.0;		// differently; detect 3D cases by "eye"...
		glScalef(font_scale, font_scale, font_scale);	// GetGlutStringWidth() is not scaled!!!
		
		i32u count = 0;
		while (count < strlen(str))
		{
			glutStrokeCharacter(GLUT_STROKE_ROMAN, str[count++]);
		}
		
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
	}
}

/*################################################################################################*/

bool graphics_view::quick_update = false;
bool graphics_view::draw_info = true;

graphics_view::graphics_view(camera * p1) : ogl_view()
{
	cam = p1;
	my_gv_number = cam->graphics_view_counter++;
	
//	render = RENDER_WIREFRAME;		// make this model-dependent like the coloring...
	render = RENDER_BALL_AND_STICK;		// make this model-dependent like the coloring...
//	render = RENDER_VAN_DER_WAALS;
	label = LABEL_NOTHING;
	
	colormode = cam->prj->GetDefaultColorMode();
	
	enable_fog = false;
	accumulate = false;
}

graphics_view::~graphics_view(void)
{
}

void graphics_view::GetCRD(i32s * p1, fGL * p2)
{
	v3d<fGL> xdir = (cam->GetLocData()->ydir).vpr(cam->GetLocData()->zdir); xdir = xdir / xdir.len();
	v3d<fGL> tmpv = v3d<fGL>(cam->GetLocData()->crd); tmpv = tmpv + (cam->GetLocData()->zdir * cam->focus);
	tmpv = tmpv + xdir * (2.0 * range[0] * (fGL) (size[0] / 2 - p1[0]) / (fGL) size[0]);
	tmpv = tmpv + cam->GetLocData()->ydir * (2.0 * range[1] * (fGL) (size[1] / 2 - p1[1]) / (fGL) size[1]);
	for (i32s n1 = 0;n1 < 3;n1++) p2[n1] = tmpv[n1];
}

void graphics_view::InitGL(void)
{
	SetCurrent();
	GetProject()->InitGL();
}

void graphics_view::Render(void)
{
	SetCurrent();
	cam->RenderWindow(this);
}

/*################################################################################################*/

// the "udata" handling functions (for creating and using the data) are here. allocate memory with malloc!!!
// the "udata" handling functions (for creating and using the data) are here. allocate memory with malloc!!!
// the "udata" handling functions (for creating and using the data) are here. allocate memory with malloc!!!

void * convert_cset_to_plotting_udata(project * mdl, i32s cset)
{
	void * udata = malloc(sizeof(i32s) + sizeof(fGL) * mdl->GetAtomCount() * 3);
	
	// first, store the number of atoms in the model, as a simple way to test data validity...
	
	i32s * ptr1 = (i32s *) udata;
	(* ptr1) = mdl->GetAtomCount();
	
	// then just store the coordinates...
	
	fGL * ptr2 = (fGL *) ((i32u) udata + sizeof(i32s)); i32s counter = 0;
	for (iter_al it1 = mdl->GetAtomsBegin();it1 != mdl->GetAtomsEnd();it1++)
	{
		const fGL * cdata = (* it1).GetCRD(cset);
		ptr2[counter++] = cdata[0];
		ptr2[counter++] = cdata[1];
		ptr2[counter++] = cdata[2];
	}
	
	return udata;
}

void apply_udata_as_cset(project * prj, void * udata)
{
	i32s * ptr1 = (i32s *) udata;
	if (prj->GetAtomCount() != (* ptr1))
	{
		cout << "oops!!! the atom counts are different." << endl;
		return;
	}
	
	fGL * ptr2 = (fGL *) ((i32u) udata + sizeof(i32s)); i32s counter = 0;
	for (iter_al it1 = prj->GetAtomsBegin();it1 != prj->GetAtomsEnd();it1++)
	{
		const i32s cset = 0;	// how to set this?!?!?!
		fGL x = ptr2[counter++];
		fGL y = ptr2[counter++];
		fGL z = ptr2[counter++];
		(* it1).SetCRD(cset, x, y, z);
	}
	
	prj->UpdateAllGraphicsViews();
	
	// also make sure that if user calculates any results, the new structure will be used!
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	prj->GetCurrentSetup()->DiscardCurrentEngine();
}

// the base class...
// the base class...
// the base class...

plotting_view::plotting_view(project * p1)
{
	mdl = p1;
	select_buffer = new iGLu[SB_SIZE];
}

plotting_view::~plotting_view(void)
{
	delete[] select_buffer;
}

void plotting_view::InitGL(void)
{
	SetCurrent();
	
//	prefs * model_prefs = mdl->model_prefs;		// needed elsewhere???

	const fGL background[4] = { 0.0, 0.0, 0.0, 1.0};
//    const fGL background[4] = { 0.5, 1.0, 0.5, 0.1};
//	fGL * background = model_prefs->ColorRGBA("Graphics/BkgndColor", default_background);
	glClearColor(background[0], background[1], background[2], background[3]);
//delete[] background;
	glDepthFunc(GL_LESS); glEnable(GL_DEPTH_TEST);
	
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 64);
	const fGL specular_reflectance[4] = { 0.9, 0.9, 0.9, 1.0 };
//	fGL * specular_reflectance = model_prefs->ColorRGBA("Graphics/SpecularReflectance", default_reflectance);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular_reflectance);
//delete[] specular_reflectance;
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glSelectBuffer(SB_SIZE, select_buffer);
}

/*################################################################################################*/

plot1d_view::plot1d_view(project * p1, i32s p2, const char * s1, const char * sv, const char * title) : plotting_view(p1)
{
	plot_userdata1 = p2;

	if (!s1 || !strlen(s1)) { cout << "invalid s1 passed to plot1d_view!" << endl; exit(EXIT_FAILURE); }
	else { name1 = new char[strlen(s1) + 1]; strcpy(name1, s1); }

	if (!sv || !strlen(sv)) { cout << "invalid sv passed to plot1d_view!" << endl; exit(EXIT_FAILURE); }
	else { namev = new char[strlen(sv) + 1]; strcpy(namev, sv); }

	if (!title || !strlen(title)) { cout << "invalid title passed to plot1d_view!" << endl; exit(EXIT_FAILURE); }
	else { name_title = new char[strlen(title) + 1]; strcpy(name_title, title); }
}

plot1d_view::~plot1d_view(void)
{
	delete[] name1;
	delete[] namev;
	delete[] name_title;

	cout << "mem leak!!! free() the non-NULL udata blocks for all data points!!!" << endl;		// ???
}

void plot1d_view::SaveAs()
{
	TCHAR filter[] =     
		TEXT("Text (*.txt)\0*.txt\0")
		TEXT("All Files (*.*)\0*.*\0");

	DWORD nFilterIndex;
	char outfile_name[256];

	sprintf(outfile_name, "\0");
	if (SaveFileDlg(0, outfile_name, filter, nFilterIndex) == S_OK)
	{
		FILE * stream;
		stream  = fopen(outfile_name,"wt");
		fprintf(stream, "%s,%s\n", this->name1, this->namev);
		for(size_t i = 0; i < this->dv.size(); i++)
		{
			fprintf(stream, "%f,%f\n", this->dv[i].c1, this->dv[i].v);
		}
		fclose(stream);
	}
}


void plot1d_view::AddData(f64 c1, f64 v, void * udata)
{
	plot1d_data newdata;
	newdata.c1 = c1; newdata.v = v; newdata.udata = udata;
	
	dv.push_back(newdata);
}

void plot1d_view::SetCenterAndScale(void)
{
	if (dv.empty()) return;
	
	min1 = max1 = dv[0].c1;
	minv = maxv = dv[0].v;
	
	for (i32u n1 = 1;n1 < dv.size();n1++)
	{
		if (dv[n1].c1 < min1) min1 = dv[n1].c1;
		if (dv[n1].c1 > max1) max1 = dv[n1].c1;
		
		if (dv[n1].v < minv) minv = dv[n1].v;
		if (dv[n1].v > maxv) maxv = dv[n1].v;
	}
}

void plot1d_view::Render(void)
{
	if (size[0] < 0) return;			// skip rendering if invalid window!!!
//	prefs * model_prefs = mdl->model_prefs;		// needed elsewhere???

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// set projection and call gluLookAt() to set up our "eye".
	
	i32s width = size[0]; i32s height = size[1]; glViewport(0, 0, width, height);
	glOrtho(-0.1, +1.1, -0.1, +1.1, -0.5, +1.5);
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	
	gluLookAt(0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	
//	const fGL default_label_color[3] = { 1.0, 1.0, 1.0 };
//	fGL * label_color = model_prefs->ColorRGB("Graphics/LabelColor", default_label_color);

	glInitNames();
	
	for (i32s n1 = 0;n1 < ((i32s) dv.size()) - 1;n1++)
	{
		glColor3f(0.0, 1.0, 0.0);	// green
		glBegin(GL_LINES); fGL x; fGL y;
		
		x = (dv[n1].c1 - min1) / (max1 - min1);
		y = (dv[n1].v - minv) / (maxv - minv);
		glVertex3f(x, y, 0.1);
		
		x = (dv[n1 + 1].c1 - min1) / (max1 - min1);
		y = (dv[n1 + 1].v - minv) / (maxv - minv);
		glVertex3f(x, y, 0.1);
		
		glEnd();
	}
}

void plot1d_view::ZoomEvent(i32s)
{
}

void plot1d_view::TransEvent(i32s)
{
}

void plot1d_view::UserEvent(i32s x, i32s)
{
	if (!(size[0] > 1)) return;
	fGL sx = 1.2 * (fGL) x / (fGL) size[0] - 0.1;
	
	if (sx < 0.0) return;
	if (sx > 1.0) return;
	
	fGL s1 = sx * (max1 - min1);
	
	i32s index = NOT_DEFINED;
	fGL mindiff = max1 - min1;
	for (i32u n1 = 0;n1 < dv.size();n1++)
	{
		fGL diff = fabs(s1 - dv[n1].c1);
		if (diff < mindiff)
		{
			index = n1;
			mindiff = diff;
		}
	}
	
	if (index < 0 || index >= (i32s) dv.size()) return;
	if (!dv[index].udata) return;
	
	// ok, we have a valid selection...
	
	char mbuff1[256];
	ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << name1 << " = " << dv[index].c1 << " " << namev << " = " << dv[index].v << endl << ends;
	mdl->PrintToLog(mbuff1);
	
	if (plot_userdata1 == PLOT_USERDATA_STRUCTURE)
	{
		apply_udata_as_cset(mdl, dv[index].udata);
	}
}

void plot1d_view::ImportData(plot1d_view * p)
{
	plot_userdata1 = p->plot_userdata1;
	p->plot_userdata1 = PLOT_USERDATA_IGNORE;	// transfer the data instead of copying!
	
	for (i32u n1 = 0;n1 < p->dv.size();n1++)
	{
		plot1d_data nd;
		nd.c1 = p->dv[n1].c1;
		nd.v = p->dv[n1].v;
		
		nd.udata = p->dv[n1].udata;
		p->dv[n1].udata = NULL;			// transfer the data instead of copying!
		
		dv.push_back(nd);
	}
	
	SetCenterAndScale();
}

/*################################################################################################*/

plot2d_view::plot2d_view(project * p1, i32s p2, const char * s1, const char * s2, const char * sv, const char * title) : plotting_view(p1)
{
	plot_userdata2 = p2;
	
	if (!s1 || !strlen(s1)) { cout << "invalid s1 passed to plot2d_view!" << endl; exit(EXIT_FAILURE); }
	else { name1 = new char[strlen(s1) + 1]; strcpy(name1, s1); }
	
	if (!s2 || !strlen(s2)) { cout << "invalid s2 passed to plot2d_view!" << endl; exit(EXIT_FAILURE); }
	else { name2 = new char[strlen(s2) + 1]; strcpy(name2, s2); }
	
	if (!sv || !strlen(sv)) { cout << "invalid sv passed to plot2d_view!" << endl; exit(EXIT_FAILURE); }
	else { namev = new char[strlen(sv) + 1]; strcpy(namev, sv); }

	if (!title || !strlen(title)) { cout << "invalid title passed to plot2d_view!" << endl; exit(EXIT_FAILURE); }
	else { name_title = new char[strlen(title) + 1]; strcpy(name_title, title); }
}

plot2d_view::~plot2d_view(void)
{
	delete[] name1;
	delete[] name2;
	delete[] namev;
	delete[] name_title;

	cout << "mem leak!!! free() the non-NULL udata blocks for all data points!!!" << endl;		// ???
}

void plot2d_view::AddData(f64 c1, f64 c2, f64 v, void * udata)
{
	plot2d_data newdata;
	newdata.c1 = c1; newdata.c2 = c2; newdata.v = v; newdata.udata = udata;
	
	dv.push_back(newdata);
}

void plot2d_view::SetCenterAndScale(void)
{
	if (dv.empty()) return;
	
	min1 = max1 = dv[0].c1;
	min2 = max2 = dv[0].c2;
	minv = maxv = dv[0].v;
	
	for (i32u n1 = 1;n1 < dv.size();n1++)
	{
		if (dv[n1].c1 < min1) min1 = dv[n1].c1;
		if (dv[n1].c1 > max1) max1 = dv[n1].c1;
		
		if (dv[n1].c2 < min2) min2 = dv[n1].c2;
		if (dv[n1].c2 > max2) max2 = dv[n1].c2;
		
		if (dv[n1].v < minv) minv = dv[n1].v;
		
		// for "maxv" we actually calculate the average value instead of maximum value.
		// the max value can be very high especially if geomopt is skipped -> unreasonable scaling.
		
		maxv += dv[n1].v;	// average!
	}
	
	maxv /= dv.size();	// average!
}

void plot2d_view::Render(void)
{
	if (size[0] < 0) return;			// skip rendering if invalid window!!!
//	prefs * model_prefs = mdl->model_prefs;		// needed elsewhere???

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// set projection and call gluLookAt() to set up our "eye".
	
	i32s width = size[0]; i32s height = size[1]; glViewport(0, 0, width, height);
	glOrtho(-0.1, +1.1, -0.1, +1.1, -0.5, +1.5);
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	
	gluLookAt(0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	
//	const fGL default_label_color[3] = { 1.0, 1.0, 1.0 };
//	fGL * label_color = model_prefs->ColorRGB("Graphics/LabelColor", default_label_color);

	glInitNames();
	
	i32s zzz = (i32s) sqrt((f64) dv.size());	// support square datasets only!!!
	for (i32s n1 = 0;n1 < zzz - 1;n1++)
	{
		for (i32s n2 = 0;n2 < zzz - 1;n2++)
		{
			glBegin(GL_QUADS);
			
			fGL x1 = (dv[(n1 + 0) * zzz + (n2 + 0)].c1 - min1) / (max1 - min1);
			fGL y1 = (dv[(n1 + 0) * zzz + (n2 + 0)].c2 - min2) / (max2 - min2);
			fGL x2 = (dv[(n1 + 1) * zzz + (n2 + 1)].c1 - min1) / (max1 - min1);
			fGL y2 = (dv[(n1 + 1) * zzz + (n2 + 1)].c2 - min2) / (max2 - min2);
			
			SetColor(dv[(n1 + 0) * zzz + (n2 + 0)].v); glVertex3f(x1, y1, 0.1);
			SetColor(dv[(n1 + 1) * zzz + (n2 + 0)].v); glVertex3f(x2, y1, 0.1);
			SetColor(dv[(n1 + 1) * zzz + (n2 + 1)].v); glVertex3f(x2, y2, 0.1);
			SetColor(dv[(n1 + 0) * zzz + (n2 + 1)].v); glVertex3f(x1, y2, 0.1);
			
			glEnd();
		}
	}
}

void plot2d_view::SetColor(f64 v)
{
	fGL color[4];
	f64 cv = (v - minv) / (maxv - minv);
	GetRBRange1(cv * cv, 1.0, color);
	glColor3fv(color);
}

void plot2d_view::ZoomEvent(i32s dy)
{
	f64 zzz = maxv - minv;
	maxv += mouse_tool::dist_sensitivity * zzz * (fGL) dy / (fGL) size[1];
	
	char mbuff1[256];
	ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << "now scaling the colours to range " << (maxv - minv) << " kJ/mol" << endl << ends;
	mdl->PrintToLog(mbuff1);
	
	Update();
}

void plot2d_view::TransEvent(i32s)
{
}

void plot2d_view::UserEvent(i32s x, i32s y)
{
	if (!(size[0] > 1)) return;
	fGL sx = 1.2 * (fGL) x / (fGL) size[0] - 0.1;
	
	if (sx < 0.0) return;
	if (sx > 1.0) return;
	
	if (!(size[1] > 1)) return;
	fGL sy = 1.2 * (fGL) y / (fGL) size[1] - 0.1;
	
	if (sy < 0.0) return;
	if (sy > 1.0) return;
	
	// WARNING!!!
	// ^^^^^^^^^^
	// this is not yet a generic implementation ; it is assumed that the data points
	// have been added in a certain order; see project::DoEnergyPlot2D() how to add them...
	
	i32s zzz = (i32s) sqrt((f64) dv.size());	// support square datasets only!!!
	i32s index = (i32s) (((i32s) (sx * zzz)) * zzz + sy * zzz);
	if (index >= (i32s) dv.size()) return;
	
	if (!dv[index].udata) return;
	
	// ok, we have a valid selection...
	
	char mbuff1[256];
	ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << name1 << " = " << dv[index].c1 << " " << name2 << " = " << dv[index].c2 << " " << namev << " = " << dv[index].v << endl << ends;
	mdl->PrintToLog(mbuff1);
	
	if (plot_userdata2 == PLOT_USERDATA_STRUCTURE)
	{
		apply_udata_as_cset(mdl, dv[index].udata);
	}
}

void plot2d_view::ImportData(plot2d_view * p)
{
	plot_userdata2 = p->plot_userdata2;
	p->plot_userdata2 = PLOT_USERDATA_IGNORE;	// transfer the data instead of copying!
	
	for (i32u n1 = 0;n1 < p->dv.size();n1++)
	{
		plot2d_data nd;
		nd.c1 = p->dv[n1].c1;
		nd.c2 = p->dv[n1].c2;
		nd.v = p->dv[n1].v;
		
		nd.udata = p->dv[n1].udata;
		p->dv[n1].udata = NULL;			// transfer the data instead of copying!
		
		dv.push_back(nd);
	}
	
	SetCenterAndScale();
}

/*################################################################################################*/

rcp_view::rcp_view(project * p1, i32s p2, const char * s1, const char * sv, const char * title) : plot1d_view(p1, p2, s1, sv, title)
{
	ready = false;
	
	r_treshold = 0.65;	// default for range 0<->1...
	p_treshold = 0.35;	// default for range 0<->1...
}

rcp_view::~rcp_view(void)
{
}

void rcp_view::SortDataAndCalcRC(void)
{
	sort(dv.begin(), dv.end());
	
	i32s index = 0; fGL ets = dv.front().v;
	for (i32s n1 = 1;n1 < (i32s) dv.size();n1++)
	{
		if (dv[n1].v > ets)
		{
			index = n1;
			ets = dv[n1].v;
		}
	}
	
	// first calculate the final reaction coordinates...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	fGL * c_ts = (fGL *) (((char *) dv[index].udata) + sizeof(i32s));
	
	fGL minrc = 0.0;
	fGL maxrc = 0.0;
	{
	for (i32u n1 = 0;n1 < dv.size();n1++)
	{
		if (n1 == (i32u) index)
		{
			dv[n1].c1 = 0.0;
			continue;
		}
		
		fGL * c_xx = (fGL *) (((char *) dv[n1].udata) + sizeof(i32s));
		
		fGL rc = 0.0;
		for (i32u n5 = 0;n5 < patoms.size();n5++)
		{
			const fGL * crd1 = & c_ts[patoms[n5] * 3];
			const fGL * crd2 = & c_xx[patoms[n5] * 3];
			
			for (i32u n6 = 0;n6 < 3;n6++)
			{
				fGL tmp1 = crd2[n6] - crd1[n6];
				rc += tmp1 * tmp1;
			}
		}
		
		rc = sqrt(rc);
		if (n1 < (i32u) index) rc = -rc;
		
if (rc < minrc) minrc = rc;
if (rc > maxrc) maxrc = rc;
		dv[n1].c1 = rc;
	}
	}
	
// what the heck?!?!?! plot1d_view is unable to display negative values???
// then, just re-scale them form 0 to 1. THIS IS A BIT STRANGE...
	{
	for (i32u n1 = 0;n1 < dv.size();n1++)
	{
		//fGL rc = ((fGL) n1) / ((fGL) dv.size());		// debug ; just use the sort order...
		fGL rc = (dv[n1].c1 - minrc) / (maxrc - minrc);		// calculate RC using the distances...
		
		dv[n1].c1 = rc;
	}
	}
	
	// ...and then update the display tresholds.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	const f64 eri = dv.front().v;
	const f64 epi = dv.back().v;
	
	f64 limit_r = (epi + ets) / 2.0;
	{
	for (i32s n1 = index;n1 < (i32s) dv.size();n1++)
	{
		if (dv[n1].v < limit_r)
		{
			r_treshold = (dv[n1].c1 + dv[n1 - 1].c1) / 2.0;
			break;
		}
	}
	}
	
	f64 limit_p = (eri + ets) / 2.0;
	{
	for (i32s n1 = index;n1 >= 0;n1--)
	{
		if (dv[n1].v < limit_p)
		{
			p_treshold = (dv[n1].c1 + dv[n1 + 1].c1) / 2.0;
			break;
		}
	}
	}
	
	char txtbuff[256]; ostrstream txts(txtbuff, sizeof(txtbuff));
	txts << "the tresholds were set to " << r_treshold << " and " << p_treshold << endl << ends;
	mdl->PrintToLog(txtbuff);
	
	ready = true;
}

void rcp_view::UserEvent(i32s x, i32s y)
{
	if (!ready) return;
	
	// first setup the bonds...
	// ^^^^^^^^^^^^^^^^^^^^^^^^
	
	if (!(size[0] > 1)) return;
	fGL sx = 1.2 * (fGL) x / (fGL) size[0] - 0.1;
	
	if (sx < 0.0) return;
	if (sx > 1.0) return;
	
	fGL s1 = sx * (max1 - min1);
	
	bool show_r_bonds = (s1 < r_treshold);
	for (i32u n1 = 0;n1 < rbonds.size();n1++)
	{
		rbonds[n1]->do_not_render_TSS_fixmelater = !show_r_bonds;
	}
	
	bool show_p_bonds = (s1 > p_treshold);{
	for (i32u n1 = 0;n1 < pbonds.size();n1++)
	{
		pbonds[n1]->do_not_render_TSS_fixmelater = !show_p_bonds;
	}
	}
	
	// ...and finally call base class UserEvent().
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	plot1d_view::UserEvent(x, y);
}

/*################################################################################################*/

eld_view::eld_view(project * p1) : plotting_view(p1)
{
	mdl = p1;
	SetCenterAndScale();
}

eld_view::~eld_view(void)
{
}

void eld_view::SetCenterAndScale(void)
{
	engine * current_eng = mdl->GetCurrentSetup()->GetCurrentEngine();
	
	if (current_eng != NULL)
	{
		f64 mine = current_eng->GetOrbitalEnergy(0);
		f64 maxe = current_eng->GetOrbitalEnergy(current_eng->GetOrbitalCount() - 1);
		
		// above we assumed that the first orbital has lowest and the last orbital has highest energy...
		
		center = (mine + maxe) / 2.0;
		scale = (maxe - mine) * 1.5;
	}
	else
	{
		center = 0.0;
		scale = 1.0;
	}
}

void eld_view::Render(void)
{
	if (size[0] < 0) return;			// skip rendering if invalid window!!!
//	prefs * model_prefs = mdl->model_prefs;		// needed elsewhere???

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// set projection and call gluLookAt() to set up our "eye".
	
// PREFS: making this value bigger will make the left margin (for displaying text) bigger.
	fGL diagram_left_margin = 0.2 + 0.5;
	
	i32s width = size[0]; i32s height = size[1]; glViewport(0, 0, width, height);
	glOrtho(-diagram_left_margin, +1.2, center - scale * 0.5, center + scale * 0.5, -0.5, +1.5);
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	
	gluLookAt(0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	
// render...
// render...
// render...

	const fGL label_color[3] = { 1.0, 1.0, 1.0 };
//	fGL *label_color = model_prefs->ColorRGB("Graphics/LabelColor", default_label_color);

	glInitNames();
	if (!mdl->GetCurrentSetup()->GetCurrentEngine())	// print out a message if we have no data...
	{
		glColor3f(label_color[0], label_color[1], label_color[2]);
		
		fGL xpos; fGL ypos;
		void * font = GLUT_BITMAP_TIMES_ROMAN_24;
		
		const char * txt1 = "No data available,";
		xpos = (size[0] - GetGlutStringWidth(txt1, font)) / 2.0;
		ypos = (size[1] - 24) / 2.0 + 14;
		WriteGlutString2D(txt1, xpos, ypos, font);
		
		const char * txt2 = "please calculate energy!";
		xpos = (size[0] - GetGlutStringWidth(txt2, font)) / 2.0;
		ypos = (size[1] - 24) / 2.0 - 14;
		WriteGlutString2D(txt2, xpos, ypos, font);
	}
	else	// ...or otherwise render the diagram.
	{
		fGL one_pixel_height = scale / (fGL) height;
		
// PREFS: making this value bigger will make lines thicker (the value is in pixels).
		fGL diagram_line_thickness = 2.0;	//model_prefs->Double("ELGraphics/LineWidth", 2.0);
// PREFS: making this value bigger will make triangles taller (the value is in pixels).
		fGL diagram_triangle_height = 30.0;	//model_prefs->Double("ELGraphics/TriangleHeight", 30.0);
// PREFS: making this value bigger will make lines wider.
		fGL diagram_triangle_width = 0.1;	//model_prefs->Double("ELGraphics/TriangleWidth", 0.1);
		
// the functions:
// GetOrbitalCount()
// GetOrbitalEnergy()
// need to separate between alpha and beta electrons.
// also a simple way to determine occupation (and whether is alpha/beta) is needed.

		// gather the degenerate energy levels (del) into groups...
		
		const fGL del_tolerance = 0.001;
		i32s * del_first = new i32s[mdl->GetCurrentSetup()->GetCurrentEngine()->GetOrbitalCount()];
		
		del_first[0] = 0; i32s dd = 1;
		while (dd < mdl->GetCurrentSetup()->GetCurrentEngine()->GetOrbitalCount())
		{
			i32s ind_first = del_first[dd - 1];
			
			fGL ene_first = mdl->GetCurrentSetup()->GetCurrentEngine()->GetOrbitalEnergy(ind_first);
			fGL ene_current = mdl->GetCurrentSetup()->GetCurrentEngine()->GetOrbitalEnergy(dd);
			
			if (ene_current - ene_first < del_tolerance)
			{
				del_first[dd] = ind_first;	// belongs to the previous set.
			}
			else
			{
				del_first[dd] = dd;		// is the first of a new set.
			}
			
			dd++;
		}
		
		i32s * del_count = new i32s[mdl->GetCurrentSetup()->GetCurrentEngine()->GetOrbitalCount()];
		for (dd = 0;dd < mdl->GetCurrentSetup()->GetCurrentEngine()->GetOrbitalCount();dd++) del_count[dd] = 0;
		
		dd = 0; i32s first = 0;
		while (dd < mdl->GetCurrentSetup()->GetCurrentEngine()->GetOrbitalCount())	// calculate the group sizes...
		{
			if (del_first[dd] != del_first[first]) first = dd;
			del_count[first]++;
			dd++;
		}
		
		dd = 0; first = 0;
		while (dd < mdl->GetCurrentSetup()->GetCurrentEngine()->GetOrbitalCount())	// ...and copy them to all group members.
		{
			if (del_first[dd] != del_first[first]) first = dd;
			if (first != dd) del_count[dd] = del_count[first];
			dd++;
		}
		
		// render...
		
		for (i32s n1 = 0;n1 < mdl->GetCurrentSetup()->GetCurrentEngine()->GetOrbitalCount();n1++)
		{
			fGL energy = mdl->GetCurrentSetup()->GetCurrentEngine()->GetOrbitalEnergy(n1);
			
		//fGL left = 0.0; fGL right = 1.0; fGL width = right - left;	// OBSOLETE...
		
		fGL width = 1.0 / (fGL) del_count[n1];
		fGL left = 0.0 + (fGL) (n1 - del_first[n1]) * width;
		fGL right = 0.0 + (fGL) (n1 - del_first[n1] + 1) * width;
		
			// draw the line...
			// draw the line...
			// draw the line...
			
			glColor3f(0.0, 1.0, 0.0);	// green
			glBegin(GL_QUADS);
			
			glVertex3f(right - 0.05 * width, energy, 0.1);
			glVertex3f(left + 0.05 * width, energy, 0.1);
			glVertex3f(left + 0.05 * width, energy + one_pixel_height * diagram_line_thickness, 0.1);
			glVertex3f(right - 0.05 * width, energy + one_pixel_height * diagram_line_thickness, 0.1);
			
			glEnd();
			
			// draw the electrons...
			// draw the electrons...
			// draw the electrons...
			
			if (n1 < mdl->GetCurrentSetup()->GetCurrentEngine()->GetElectronCount() / 2)
			{
				glColor3f(1.0, 1.0, 0.0);	// yellow
				glBegin(GL_TRIANGLES);
				
glVertex3f(left + 0.3 * width - diagram_triangle_width, energy - one_pixel_height * diagram_triangle_height / 3.0, 0.0);
glVertex3f(left + 0.3 * width, energy + one_pixel_height * diagram_triangle_height / 1.5, 0.0);
glVertex3f(left + 0.3 * width + diagram_triangle_width, energy - one_pixel_height * diagram_triangle_height / 3.0, 0.0);

glVertex3f(right - 0.3 * width - diagram_triangle_width, energy + one_pixel_height * diagram_triangle_height / 3.0, 0.0);
glVertex3f(right - 0.3 * width, energy - one_pixel_height * diagram_triangle_height / 1.5, 0.0);
glVertex3f(right - 0.3 * width + diagram_triangle_width, energy + one_pixel_height * diagram_triangle_height / 3.0, 0.0);

				glEnd();
			}
			
			// print out the text...
			// print out the text...
			// print out the text...
			
			glColor3f(label_color[0], label_color[1], label_color[2]);
			
			fGL xpos; fGL ypos;
			void * font = GLUT_BITMAP_TIMES_ROMAN_10;
			
			char txt[256];
			ostrstream str(txt, sizeof(txt));
			str << "i = " << n1 << " e = " << energy << ends;
			
			xpos = 10.0;
			ypos = ((energy - center) / scale + 0.5) * size[1] - 5.0;	// 5.0 comes from font height 10...
			ypos += (fGL) (n1 - del_first[n1]) * 10.0;			// 10.0 comes from font height 10...
			
			WriteGlutString2D(txt, xpos, ypos, font);
		}
		
		delete[] del_first;
		delete[] del_count;
	}
}

void eld_view::ZoomEvent(i32s dy)
{
	scale += mouse_tool::dist_sensitivity * scale * (fGL) dy / (fGL) size[1];
	Update();
}

void eld_view::TransEvent(i32s dy)
{
	center -= 0.5 * scale * (fGL) dy / (fGL) size[1];
	Update();
}

// figure out the selections using coordinates instead of OpenGL selection mode?!?!?! might be simpler in this 2-dimensional case...
// figure out the selections using coordinates instead of OpenGL selection mode?!?!?! might be simpler in this 2-dimensional case...
// figure out the selections using coordinates instead of OpenGL selection mode?!?!?! might be simpler in this 2-dimensional case...

void eld_view::UserEvent(i32s, i32s)
{
	cout << "UserEvent() was called!" << endl;
}

/*################################################################################################*/

// eof
