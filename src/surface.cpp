// SURFACE.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "StdAfx.h"
#include "surface.h"	// config.h is here -> we get ENABLE-macros here...

#include "project.h"
#include "appdefine.h"

#include <deque>
#include <strstream>
using namespace std;

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

/*################################################################################################*/

color_surface::color_surface(cs_param & p1)
{
	prj = p1.prj;
	data = p1.data;
	name = p1.name;
	
	transparent = p1.transparent;
	automatic_cv2 = p1.automatic_cv2;
	wireframe = p1.wireframe;
	
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		np[n1] = p1.np[n1];
		dist[n1] = new fGL[np[n1]];
	}
	
	ref = p1.ref;
	GetSurfaceValue = p1.vf1;
	GetColorValue = p1.vf2;
	GetColor = p1.cf;
	
	svalue = p1.svalue;
	cvalue1 = p1.cvalue1;
	cvalue2 = p1.cvalue2;
	alpha = p1.alpha;
	
	tolerance = p1.toler;
	max_cycles = p1.maxc;
	
	grid = vector<bool>(np[0] * np[1] * np[2]);
	
	SetDimension(p1.dim);
	Update();
}

color_surface::~color_surface(void)
{
	if (transparent) prj->RemoveAllTPs((void *) this);
	
	delete[] dist[0];
	delete[] dist[1];
	delete[] dist[2];
}

void color_surface::Update(void)
{
	char mbuff1[256];
	
	xdir = data->ydir.vpr(data->zdir);
	
	vdata.resize(0);
	tdata.resize(0);
	ldata.resize(0);
	
	prj->PrintToLog("evaluating grid");
	for (i32s n1 = 0;n1 < np[0];n1++)
	{
		prj->PrintToLog("*");	// slow to update???
		
		for (i32s n2 = 0;n2 < np[1];n2++)
		{
			for (i32s n3 = 0;n3 < np[2];n3++)
			{
				i32s tmp1[3] = { n1, n2, n3 }; fGL tmp2[3]; GetCRD(tmp1, tmp2);
				grid[n1 * np[1] * np[2] + n2 * np[2] + n3] = (GetSurfaceValue(ref, tmp2, NULL) < svalue);
			}
		}
	}
	
	prj->PrintToLog(" done\n");
	
	avrg = 0.0; acnt = 0;
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		i32s tmp1[2]; i32s tmp2[2] = { 0, 0 };
		prj->PrintToLog("interpolating in ");
		switch (n1)
		{
			case 0:
			tmp1[0] = 1; tmp1[1] = 2;
			prj->PrintToLog("x");
			break;
			
			case 1:
			tmp1[0] = 2; tmp1[1] = 0;
			prj->PrintToLog("y");
			break;
			
			case 2:
			tmp1[0] = 0; tmp1[1] = 1;
			prj->PrintToLog("z");
			break;
		}
		
		prj->PrintToLog("-direction... ");
		
		range[n1][0] = vdata.size();
		Interpolate(n1, tmp1, tmp2);
		range[n1][1] = vdata.size();
		
		ostrstream str1(mbuff1, sizeof(mbuff1));
		str1 << tmp2[0] << " points, " << tmp2[1] << " cycles" << endl << ends;
		prj->PrintToLog(mbuff1);
	}
	
	if (automatic_cv2)
	{
		// AUTO makes only partial correction -> just looks better and one can estimate the "bias".
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		
		cvalue2 = -0.80 * avrg / (f64) acnt;
	}
	{
	for (i32u n1 = 0;n1 < vdata.size();n1++)
	{
		GetColor((vdata[n1].cvalue + cvalue2) / cvalue1, alpha, vdata[n1].color);
	}
	}
	prj->PrintToLog("generating image... ");
	{
	for (i32s n1 = 0;n1 < np[0] - 1;n1++)
	{
		for (i32s n2 = 0;n2 < np[1] - 1;n2++)
		{
			for (i32s n3 = 0;n3 < np[2] - 1;n3++)
			{
				vector<cs_line> line_vector;
				
				for (i32s n4 = 0;n4 < 2;n4++)
				{
					for (i32s n5 = 0;n5 < 2;n5++)
					{
						for (i32s n6 = 0;n6 < 2;n6++)
						{
							i32s id1[4];
							id1[0] = (n1 + n4) * np[1] * np[2] + (n2 + n5) * np[2] + (n3 + n6);
							id1[1] = (n1 + !n4) * np[1] * np[2] + (n2 + n5) * np[2] + (n3 + n6);
							id1[2] = (n1 + n4) * np[1] * np[2] + (n2 + !n5) * np[2] + (n3 + n6);
							id1[3] = (n1 + n4) * np[1] * np[2] + (n2 + n5) * np[2] + (n3 + !n6);
							
							i32s id2[3] = { NOT_DEFINED, NOT_DEFINED, NOT_DEFINED };
							
							for (i32s n7 = 0;n7 < 3;n7++)
							{
								if (grid[id1[0]] != grid[id1[n7 + 1]])
								{
					i32s tmp1 = min(id1[0], id1[n7 + 1]); id2[n7] = range[n7][0];
					while (id2[n7] < range[n7][1] && vdata[id2[n7]].id != tmp1) id2[n7]++;
					if (id2[n7] == range[n7][1]) cout << "color_surface problem #1" << endl;
								}
							}
							
							for (i32s n7 = 0;n7 < 3;n7++)
							{
								cs_line newline;
								newline.index[0] = id2[n7];
								newline.index[1] = id2[(n7 + 1) % 3];
								newline.Arrange();
								
								if (newline.index[0] < 0) continue;
								if (newline.index[1] < 0) continue;
								
								line_vector.push_back(newline);
							}
						}
					}
				}
				
				for (i32s n4 = 0;n4 < 3;n4++)
				{
					i32s id1a[4]; i32s id1b[4];
					i32s tmp1[2] = { (n4 + 1) % 3, (n4 + 2) % 3 };
					
					i32s t1a[3] = { n1, n2, n3 };
					id1a[0] = t1a[0] * np[1] * np[2] + t1a[1] * np[2] + t1a[2];
					i32s t2a[3] = { n1, n2, n3 }; t2a[tmp1[0]]++;
					id1a[1] = t2a[0] * np[1] * np[2] + t2a[1] * np[2] + t2a[2];
					i32s t3a[3] = { n1, n2, n3 }; t3a[tmp1[0]]++; t3a[tmp1[1]]++;
					id1a[2] = t3a[0] * np[1] * np[2] + t3a[1] * np[2] + t3a[2];
					i32s t4a[3] = { n1, n2, n3 }; t4a[tmp1[1]]++;
					id1a[3] = t4a[0] * np[1] * np[2] + t4a[1] * np[2] + t4a[2];
					
					i32s t1b[3] = { n1, n2, n3 }; t1b[n4]++;
					id1b[0] = t1b[0] * np[1] * np[2] + t1b[1] * np[2] + t1b[2];
					i32s t2b[3] = { n1, n2, n3 }; t2b[n4]++; t2b[tmp1[0]]++;
					id1b[1] = t2b[0] * np[1] * np[2] + t2b[1] * np[2] + t2b[2];
					i32s t3b[3] = { n1, n2, n3 }; t3b[n4]++; t3b[tmp1[0]]++; t3b[tmp1[1]]++;
					id1b[2] = t3b[0] * np[1] * np[2] + t3b[1] * np[2] + t3b[2];
					i32s t4b[3] = { n1, n2, n3 }; t4b[n4]++; t4b[tmp1[1]]++;
					id1b[3] = t4b[0] * np[1] * np[2] + t4b[1] * np[2] + t4b[2];
					
					i32s id2[4] = { NOT_DEFINED, NOT_DEFINED, NOT_DEFINED, NOT_DEFINED };
					
					for (i32s n5 = 0;n5 < 4;n5++)
					{
						if (grid[id1a[n5]] != grid[id1b[n5]])
						{
			id2[n5] = range[n4][0]; while (id2[n5] < range[n4][1] && vdata[id2[n5]].id != id1a[n5]) id2[n5]++;
			if (id2[n5] == range[n4][1]) cout << "color_surface problem #2" << endl;
						}
					}
					
					for (i32s n5 = 0;n5 < 4;n5++)
					{
						cs_line newline;
						newline.index[0] = id2[n5];
						newline.index[1] = id2[(n5 + 1) % 4];
						newline.Arrange();
						
						if (newline.index[0] < 0) continue;
						if (newline.index[1] < 0) continue;
						
						line_vector.push_back(newline);
					}
				}
				
				ldata.insert(ldata.end(), line_vector.begin(), line_vector.end());
								
				while (!line_vector.empty())
				{
					deque<i32s> polygon;
					
					polygon.push_front(line_vector.back().index[0]);
					polygon.push_back(line_vector.back().index[1]);
					line_vector.pop_back();
					
					i32s counter = 0;
					while (counter < (i32s) line_vector.size())
					{
						for (i32s dir = 0;dir < 2;dir++)
						{
							if (line_vector[counter].index[dir] == polygon.front())
							{
								polygon.push_front(line_vector[counter].index[!dir]);
								line_vector.erase(line_vector.begin() + counter);
								counter = -1; break;
							}
							
							if (line_vector[counter].index[dir] == polygon.back())
							{
								polygon.push_back(line_vector[counter].index[!dir]);
								line_vector.erase(line_vector.begin() + counter);
								counter = -1; break;
							}
						}
						
						counter++;
					}
					
					for (i32s n4 = 1;n4 < ((i32s) polygon.size()) - 2;n4++)
					{
						cs_triangle newtriangle;
						newtriangle.index[0] = polygon[0];
						newtriangle.index[1] = polygon[n4];
						newtriangle.index[2] = polygon[n4 + 1];
						
						if (newtriangle.index[0] == newtriangle.index[1]) cout << "E";
						if (newtriangle.index[0] == newtriangle.index[2]) cout << "E";
						if (newtriangle.index[1] == newtriangle.index[2]) cout << "E";
						tdata.push_back(newtriangle);
					}
				}
			}
		}
	}
	}
	// there are some duplicated lines -> make unique !!!
	// there are some duplicated lines -> make unique !!!
	// there are some duplicated lines -> make unique !!!
	
	sort(ldata.begin(), ldata.end());
	vector<cs_line>::iterator it1 = unique(ldata.begin(), ldata.end());
	if (it1 != ldata.end()) ldata.erase(it1, ldata.end());
	
	// check the counterclockwise winding of triangles !!!
	// check the counterclockwise winding of triangles !!!
	// check the counterclockwise winding of triangles !!!
	
	for (i32s n1 = 0;n1 < (i32s) tdata.size();n1++)
	{
		v3d<fGL> nv1 = v3d<fGL>(vdata[tdata[n1].index[0]].normal);
		v3d<fGL> nv2 = v3d<fGL>(vdata[tdata[n1].index[1]].normal);
		v3d<fGL> nv3 = v3d<fGL>(vdata[tdata[n1].index[2]].normal);
		
		v3d<fGL> v1 = nv1 + nv2; v1 = v1 + nv3;
		v3d<fGL> v2 = v3d<fGL>(vdata[tdata[n1].index[0]].crd, vdata[tdata[n1].index[1]].crd);
		v3d<fGL> v3 = v3d<fGL>(vdata[tdata[n1].index[1]].crd, vdata[tdata[n1].index[2]].crd);
		bool flag = !(v1.tor(v2, v3) < 0.0);
		
		if (flag)
		{
			i32s tmp1 = tdata[n1].index[0];
			tdata[n1].index[0] = tdata[n1].index[1];
			tdata[n1].index[1] = tmp1;
		}
	}
	
	ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << ldata.size() << " lines, " << tdata.size() << " triangles." << endl << ends;
	prj->PrintToLog(mbuff1);
	
	// add/update the primitives, if this is a transparent object...
	// add/update the primitives, if this is a transparent object...
	// add/update the primitives, if this is a transparent object...
	
	if (transparent && !wireframe)
	{
		prj->RemoveAllTPs((void *) this);
		
		for (i32s n1 = 0;n1 < (i32s) tdata.size();n1++)
		{
			fGL * c1 = vdata[tdata[n1].index[0]].color;
			fGL * p1 = vdata[tdata[n1].index[0]].crd;
			
			fGL * c2 = vdata[tdata[n1].index[1]].color;
			fGL * p2 = vdata[tdata[n1].index[1]].crd;
			
			fGL * c3 = vdata[tdata[n1].index[2]].color;
			fGL * p3 = vdata[tdata[n1].index[2]].crd;
			
			tpd_tri_3c * tmp4;
			tmp4 = new tpd_tri_3c(c1, p1, c2, p2, c3, p3);
			
			transparent_primitive * tmp5;
			tmp5 = new transparent_primitive((void *) this, * tmp4);
			
			prj->AddTP((void *) this, (* tmp5));
			
			// tmp4, the tpd_tri_3c-object, will be deleted when
			// prj->RemoveAllTPs() is called for this object...
			
			delete tmp5;
		}
	}
}

void color_surface::Render(void)
{
	glPushName(GLNAME_OBJECT);
	glPushName((iGLu) name);
	
	if (wireframe)
	{
		glBegin(GL_LINES);
		for (i32s n1 = 0;n1 < (i32s) ldata.size();n1++)
		{
			i32s ind1 = ldata[n1].index[0]; i32s ind2 = ldata[n1].index[1];
			glColor3fv(vdata[ind1].color); glVertex3fv(vdata[ind1].crd);
			glColor3fv(vdata[ind2].color); glVertex3fv(vdata[ind2].crd);
		}
		
		glEnd();	// GL_LINES
	}
	else if (!transparent)
	{
		glEnable(GL_LIGHTING);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, true);
		
		glBegin(GL_TRIANGLES);
		for (i32s n1 = 0;n1 < (i32s) tdata.size();n1++)
		{
			vdata[tdata[n1].index[0]].Render(true);		// aina TRUE!??!?!?!
			vdata[tdata[n1].index[1]].Render(true);
			vdata[tdata[n1].index[2]].Render(true);
		}
		
		glEnd();	// GL_TRIANGLES
		
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, false);
		glDisable(GL_LIGHTING);
	}
	
	glPopName();
	glPopName();
}

void color_surface::SetDimension(fGL * p1)
{
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		dim[n1] = p1[n1];
		fGL tmp1 = np[n1] - 1;
		
		for (i32s n2 = 0;n2 < np[n1];n2++)
		{
			fGL tmp2 = (fGL) n2 / tmp1;
			dist[n1][n2] = dim[n1] * (tmp2 - 0.5);
		}
	}
}

void color_surface::GetCRD(i32s * p1, fGL * p2)
{
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		fGL tmp1 = xdir[n1] * dist[0][p1[0]];
		fGL tmp2 = data->ydir[n1] * dist[1][p1[1]];
		fGL tmp3 = data->zdir[n1] * dist[2][p1[2]];
		p2[n1] = data->crd[n1] + tmp1 + tmp2 + tmp3;
	}
}

void color_surface::Interpolate(i32s p1, i32s * p2, i32s * p3)
{
	i32s tmp1[3]; i32s tmp2[3];
	for (i32s n1 = 0;n1 < np[p2[0]];n1++)
	{
		tmp1[p2[0]] = tmp2[p2[0]] = n1;
		
		for (i32s n2 = 0;n2 < np[p2[1]];n2++)
		{
			tmp1[p2[1]] = tmp2[p2[1]] = n2;
			
			for (i32s n3 = 0;n3 < np[p1] - 1;n3++)
			{
				tmp1[p1] = n3; tmp2[p1] = n3 + 1;
				bool test1 = grid[tmp1[0] * np[1] * np[2] + tmp1[1] * np[2] + tmp1[2]];
				bool test2 = grid[tmp2[0] * np[1] * np[2] + tmp2[1] * np[2] + tmp2[2]];
				if (test1 == test2) continue;
				
				fGL tmp3a[3]; GetCRD(tmp1, tmp3a);
				fGL tmp3b[3]; GetCRD(tmp2, tmp3b);
				
				v3d<fGL> tmpv1 = v3d<fGL>(tmp3a);
				v3d<fGL> tmpv2 = v3d<fGL>(tmp3a, tmp3b);
				
				v3d<fGL> tmpv3;
				
				fGL tmp4[2]; tmp4[0] = 0.0; tmpv3 = tmpv1 + (tmpv2 * tmp4[0]);
				tmp4[1] = GetSurfaceValue(ref, tmpv3.data, NULL) - svalue;
				
				fGL tmp5[2]; tmp5[0] = 1.0; tmpv3 = tmpv1 + (tmpv2 * tmp5[0]);
				tmp5[1] = GetSurfaceValue(ref, tmpv3.data, NULL) - svalue;
				
				// this is a "regula falsi"-type search...
				
				fGL tmp6 = 0.0; i32s tmp7 = 0;
				while (tmp7++ < max_cycles)
				{
					fGL tmp8 = tmp5[0] - tmp5[1] * (tmp5[0] - tmp4[0]) / (tmp5[1] - tmp4[1]);
					
					tmpv3 = tmpv1 + (tmpv2 * tmp8);
					tmp6 = GetSurfaceValue(ref, tmpv3.data, NULL) - svalue;
					
					if ((tmp6 < 0.0) ^ (tmp4[1] < 0.0))
					{
						tmp5[0] = tmp8;
						tmp5[1] = tmp6;
					}
					else
					{
						tmp4[0] = tmp8;
						tmp4[1] = tmp6;
					}
					
					if (fabs(tmp4[1]) < fabs(tmp5[1]))
					{
						tmp6 = tmp4[0];
						tmp8 = fabs(tmp4[1]);
					}
					else
					{
						tmp6 = tmp5[0];
						tmp8 = fabs(tmp5[1]);
					}
					
					if (tmp8 < tolerance) break;
					else p3[1]++;
				}
				
				cs_vertex vertex; tmpv3 = tmpv1 + (tmpv2 * tmp6);
				vertex.cvalue = GetColorValue(ref, tmpv3.data, NULL);
				
				if (automatic_cv2)
				{
					avrg += vertex.cvalue;
					acnt++;
				}
				
				fGL tmp9[4];
				
				if (!wireframe)
				{
					tmp9[3] = 0.0; GetSurfaceValue(ref, tmpv3.data, tmp9);
					for (i32s n4 = 0;n4 < 3;n4++) tmp9[3] += tmp9[n4] * tmp9[n4];
					tmp9[3] = sqrt(tmp9[3]);
				}
				
				for (i32s n4 = 0;n4 < 3;n4++)
				{
					vertex.crd[n4] = tmpv3[n4];
					if (!wireframe) vertex.normal[n4] = tmp9[n4] / tmp9[3];
				}
				
				vertex.id = (tmp1[0] * np[1] * np[2] + tmp1[1] * np[2] + tmp1[2]);
				vdata.push_back(vertex); p3[0]++;
			}
		}
	}
}

/*################################################################################################*/

color_surface_object::color_surface_object(const object_location & p1, cs_param & p2, const char * p3) : smart_object(p1)
{
	transform_in_progress = false;
	
	dim[0] = p2.dim[0];	// now we have several copies
	dim[1] = p2.dim[1];	// of dimension, but it should
	dim[2] = p2.dim[2];	// be the same everywhere...
	
	cs_param * param = & p2;
	
	while (true)
	{
		param->data = GetLocData();
		param->name = (const dummy_object *) this;
		
		cs_vector.push_back(new color_surface(* param));
		
		if (param->next != NULL) param = param->next;
		else break;
	}
	
	char buffer[1024];
	ostrstream str(buffer, sizeof(buffer));
	str << p3 << "surface" << ends;
	
	copy_of_ref = p2.ref;
	
	object_name = new char[strlen(buffer) + 1];
	strcpy(object_name, buffer);
}

color_surface_object::~color_surface_object(void)
{
	for (i32u n1 = 0;n1 < cs_vector.size();n1++)
	{
		delete cs_vector[n1];
	}
	
	delete[] object_name;
}

bool color_surface_object::BeginTransformation(void)
{
	transform_in_progress = true;
	return true;
}

bool color_surface_object::EndTransformation(void)
{
	transform_in_progress = false;
	
	Update();	// update the surfaces!!!!!
	
	return true;
}

void color_surface_object::OrbitObject(const fGL * p1, const camera & p2)
{
	dummy_object::RotateObject(p1, p2);
}

void color_surface_object::RotateObject(const fGL * p1, const camera & p2)
{
	dummy_object::OrbitObject(p1, p2);
}

void color_surface_object::TranslateObject(const fGL * p1, const obj_loc_data * p2)
{
	fGL tmp1[3] = { p1[0], p1[1], p1[2] };
	
	if (p2 == GetLocData())
	{
		tmp1[0] = -tmp1[0];
		tmp1[2] = -tmp1[2];
	}
	
	dummy_object::TranslateObject(tmp1, p2);
}

void color_surface_object::Render(void)
{
	if (transform_in_progress)
	{
		glPushMatrix(); SetModelView();
		fGL dx[2]; dx[0] = dim[0] / 2.0; dx[1] = -dx[0];
		fGL dy[2]; dy[0] = dim[1] / 2.0; dy[1] = -dy[0];
		fGL dz[2]; dz[0] = dim[2] / 2.0; dz[1] = -dz[0];
		
		glBegin(GL_LINES);
		
		glColor3f(0.0, 0.0, 1.0);
		
		glVertex3f(dx[0], dy[0], dz[0]); glVertex3f(dx[1], dy[0], dz[0]);
		glVertex3f(dx[0], dy[1], dz[0]); glVertex3f(dx[1], dy[1], dz[0]);
		glVertex3f(dx[0], dy[1], dz[1]); glVertex3f(dx[1], dy[1], dz[1]);
		glVertex3f(dx[0], dy[0], dz[1]); glVertex3f(dx[1], dy[0], dz[1]);
		
		glVertex3f(dx[0], dy[0], dz[0]); glVertex3f(dx[0], dy[1], dz[0]);
		glVertex3f(dx[1], dy[0], dz[0]); glVertex3f(dx[1], dy[1], dz[0]);
		glVertex3f(dx[1], dy[0], dz[1]); glVertex3f(dx[1], dy[1], dz[1]);
		glVertex3f(dx[0], dy[0], dz[1]); glVertex3f(dx[0], dy[1], dz[1]);
		
		glVertex3f(dx[0], dy[0], dz[0]); glVertex3f(dx[0], dy[0], dz[1]);
		glVertex3f(dx[1], dy[0], dz[0]); glVertex3f(dx[1], dy[0], dz[1]);
		glVertex3f(dx[1], dy[1], dz[0]); glVertex3f(dx[1], dy[1], dz[1]);
		glVertex3f(dx[0], dy[1], dz[0]); glVertex3f(dx[0], dy[1], dz[1]);
		
		glColor3f(1.0, 1.0, 1.0);
		
		glVertex3f(dx[0] / 2.0, 0.0, 0.0); glVertex3f(dx[1] / 2.0, 0.0, 0.0);
		glVertex3f(0.0, dy[0] / 2.0, 0.0); glVertex3f(0.0, dy[1] / 2.0, 0.0);
		glVertex3f(0.0, 0.0, dz[0] / 2.0); glVertex3f(0.0, 0.0, dz[1] / 2.0);
		
		glEnd(); glPopMatrix();
	}
	else
	{
		for (i32u n1 = 0;n1 < cs_vector.size();n1++)
		{
			cs_vector[n1]->Render();
		}
	}
}

void color_surface_object::Update(void)
{
	for (i32u n1 = 0;n1 < cs_vector.size();n1++)
	{
		cs_vector[n1]->Update();
	}
}

/*################################################################################################*/

// eof
