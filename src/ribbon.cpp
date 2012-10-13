// RIBBON.CPP

// Copyright (C) 1998 Tommi Hassinen, Jarno Huuskonen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "ribbon.h"	// config.h is here -> we get ENABLE-macros here...

#include "spline.h"
#include "../libghemical/src/eng1_sf.h"

#include <strstream>
using namespace std;

/*################################################################################################*/

const i32s ribbon::resol = 10;

const fGL ribbon::width = 0.15;
const fGL ribbon::helix = 0.10;

ribbon::ribbon(project * p1, color_mode * p9, i32s p2, i32s order) : smart_object()
{
	prj = p1; cmode = p9;
	chn = p2; extra_points = order - 2;
	
	vector<chn_info> & ci_vector = (* prj->ref_civ);
	
	if (ci_vector[chn].GetType() != chn_info::amino_acid)
	{
		prj->ErrorMessage("BUG: The chain type is not correct (\"amino_acid\").\nThe program will terminate.");
		exit(EXIT_FAILURE);
	}
	
	if (ci_vector[chn].GetLength() < 3)
	{
		prj->ErrorMessage("BUG: Tried to create a ribbon with less than 3 residues.\nThe program will terminate.");
		exit(EXIT_FAILURE);
	}
	
	const char * tmp_sss = ci_vector[chn].GetSecStrStates();
	
	char * state = new char[ci_vector[chn].GetLength() + 1];
	state[ci_vector[chn].GetLength()] = 0;
	
	// the default state is always loop. strands have the same places as in K&S.
	// helices are shifted: the smallest helical peptide is "4...." -> "LHHHL".
	
	setup * su = prj->GetCurrentSetup();
	setup1_sf * susf = dynamic_cast<setup1_sf *>(su);
	if (susf != NULL)	// optionally, if we use SF, get the constraints...
	{
		bool error = false;
		if (susf->chn_vector.size() != ci_vector.size()) error = true;
		if (susf->chn_vector[chn].res_vector.size() != (i32u) ci_vector[chn].GetLength()) error = true;
		
		if (error)
		{
			cout << "ribbon : failed to use susf." << endl;
			exit(EXIT_FAILURE);
		}
		
		for (i32u n1 = 0;n1 < susf->chn_vector[chn].res_vector.size();n1++)
		{
			state[n1] = 'L';
			if (susf->chn_vector[chn].res_vector[n1].GetState() == SF_STATE_STRAND) state[n1] = 'S';
		}
		{
		for (i32s n1 = 0;n1 < ((i32s) susf->chn_vector[chn].res_vector.size()) - 3;n1++)
		{
			if (susf->chn_vector[chn].res_vector[n1].GetState() == SF_STATE_HELIX4)
			{
				state[n1 + 1] = 'H';
				state[n1 + 2] = 'H';
				state[n1 + 3] = 'H';
			}
		}
		}
	}
	else			// normally, use the DSSP results...
	{
		for (i32s n1 = 0;n1 < ci_vector[chn].GetLength();n1++)
		{
			state[n1] = 'L';
			if (tmp_sss[n1] == 'S') state[n1] = 'S';
		}
		
		for (/*i32s*/ n1 = 0;n1 < ((i32s) ci_vector[chn].GetLength()) - 3;n1++)
		{
			if (tmp_sss[n1] == '4')
			{
				state[n1 + 1] = 'H';
				state[n1 + 2] = 'H';
				state[n1 + 3] = 'H';
			}
		}
	}
	
	iter_al range1[2];
	prj->GetRange(1, chn, range1);
	
	length = ci_vector[chn].GetLength() - 1;
	
	cp1 = new fGL[length * 3];
	cp2 = new fGL[length * 3];
	
	data3 = new fGL[length * 3];
	
	v3d<fGL> old_tmpv7; bool flag = true;
	for (i32s n1 = 0;n1 < length;n1++)
	{
		fGL color1[4]; color1[3] = 1.0;
		fGL color2[4]; color2[3] = 1.0;
		
		if (susf != NULL)	// optional constraint colours...
		{
			color1[0] = 0.5; color1[1] = 0.0; color1[2] = 0.8;
			if (state[n1 + 0] == 'H') { color1[0] = 0.8; color1[1] = 0.5; color1[2] = 0.0; }
			if (state[n1 + 0] == 'S') { color1[0] = 0.0; color1[1] = 0.8; color1[2] = 0.5; }
			
			color2[0] = 0.5; color2[1] = 0.0; color2[2] = 0.8;
			if (state[n1 + 1] == 'H') { color2[0] = 0.8; color2[1] = 0.5; color2[2] = 0.0; }
			if (state[n1 + 1] == 'S') { color2[0] = 0.0; color2[1] = 0.8; color2[2] = 0.5; }
		}
		else			// the normal colours...
		{
			color1[0] = 0.0; color1[1] = 0.0; color1[2] = 1.0;
			if (state[n1 + 0] == 'H') { color1[0] = 1.0; color1[1] = 0.0; color1[2] = 0.0; }
			if (state[n1 + 0] == 'S') { color1[0] = 0.0; color1[1] = 1.0; color1[2] = 0.0; }
			
			color2[0] = 0.0; color2[1] = 0.0; color2[2] = 1.0;
			if (state[n1 + 1] == 'H') { color2[0] = 1.0; color2[1] = 0.0; color2[2] = 0.0; }
			if (state[n1 + 1] == 'S') { color2[0] = 0.0; color2[1] = 1.0; color2[2] = 0.0; }
		}
		
		iter_al c_alpha_1;
		iter_al c_alpha_2;
		
		v3d<fGL> ca_to_ca;
		v3d<fGL> normal(1.0, 0.0, 0.0);
		
		iter_al range2a[2]; prj->GetRange(2, range1, n1 + 0, range2a);
		iter_al range2b[2]; prj->GetRange(2, range1, n1 + 1, range2b);
		
		c_alpha_2 = range2b[0]; while (c_alpha_2 != range2b[1] && ((* c_alpha_2).builder_res_id & 0xFF) != 0x01) c_alpha_2++;
		if (c_alpha_2 == range2b[1]) { cout << "ERROR : search of C_alpha 2 failed." << endl; exit(EXIT_FAILURE); }
		
		c_alpha_1 = range2a[0]; while (c_alpha_1 != range2a[1] && ((* c_alpha_1).builder_res_id & 0xFF) != 0x01) c_alpha_1++;
		if (c_alpha_1 == range2a[1]) { cout << "ERROR : search of C_alpha 1 failed." << endl; exit(EXIT_FAILURE); }
		else
		{
			// this is the normal case, when then c_alpha_1 iterator is ok...
			// search the oxygen and calculate ca_to_ca and normal.
			
			iter_al oxygen = range2a[0]; while (oxygen != range2a[1] && ((* oxygen).builder_res_id & 0xFF) != 0x10) oxygen++;
			if (oxygen == range2a[1]) { cout << "ERROR : search of oxygen failed." << endl; exit(EXIT_FAILURE); }
			
			ca_to_ca = v3d<fGL>((* c_alpha_1).GetCRD(0), (* c_alpha_2).GetCRD(0));
			v3d<fGL> tmpv1 = v3d<fGL>((* c_alpha_1).GetCRD(0), (* oxygen).GetCRD(0));
			normal = ca_to_ca.vpr(tmpv1); normal = normal / normal.len();
		}
		
		v3d<fGL> tmpv7 = normal.vpr(ca_to_ca); tmpv7 = tmpv7 / tmpv7.len();
		
		v3d<fGL> midpoint = v3d<fGL>((* c_alpha_1).GetCRD(0));
		midpoint = midpoint + (ca_to_ca / 2.0);
		
		i32s helix_count = 0;
		if (state[n1 + 0] == 'H') helix_count++;
		if (state[n1 + 1] == 'H') helix_count++;
		midpoint = midpoint + (normal * ((fGL) helix_count * helix));
		
		v3d<fGL> tmpv8 = midpoint + (tmpv7 * width);
		v3d<fGL> tmpv9 = midpoint - (tmpv7 * width);
		
		if (n1 != 0 && tmpv7.ang(old_tmpv7) > M_PI / 2.0) flag = !flag;
		old_tmpv7 = tmpv7;
		
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			if (flag)
			{
				cp1[n1 * 3 + n2] = tmpv8[n2];
				cp2[n1 * 3 + n2] = tmpv9[n2];
			}
			else
			{
				cp1[n1 * 3 + n2] = tmpv9[n2];
				cp2[n1 * 3 + n2] = tmpv8[n2];
			}
			
			data3[n1 * 3 + n2] = (color1[n2] + color2[n2]) / 2.0;
		}
	}
	
	ref1 = new spline(order, length + extra_points * 2);
	
	head_points1 = new fGL[extra_points * 3];
	tail_points1 = new fGL[extra_points * 3];

	head_refs1[0] = & cp1[1 * 3]; head_refs1[1] = & cp1[0 * 3];
	tail_refs1[0] = & cp1[(length - 2) * 3]; tail_refs1[1] = & cp1[(length - 1) * 3];
	
	ref2 = new spline(order, length + extra_points * 2);
	
	head_points2 = new fGL[extra_points * 3];
	tail_points2 = new fGL[extra_points * 3];
	
	head_refs2[0] = & cp2[1 * 3]; head_refs2[1] = & cp2[0 * 3];
	tail_refs2[0] = & cp2[(length - 2) * 3]; tail_refs2[1] = & cp2[(length - 1) * 3];
	
	UpdateExtraPoints(head_refs1, head_points1, tail_refs1, tail_points1);
	UpdateExtraPoints(head_refs2, head_points2, tail_refs2, tail_points2);
	
	for (/*i32s*/ n1 = 0;n1 < extra_points;n1++)
	{
		ref1->SetPoint(n1, & head_points1[(extra_points - (n1 + 1)) * 3]);
		ref2->SetPoint(n1, & head_points2[(extra_points - (n1 + 1)) * 3]);
	}
	
	for (/*i32s*/ n1 = 0;n1 < extra_points;n1++)
	{
		ref1->SetPoint(extra_points + length + n1, & tail_points1[n1 * 3]);
		ref2->SetPoint(extra_points + length + n1, & tail_points2[n1 * 3]);
	}
	
	for (/*i32s*/ n1 = 0;n1 < length;n1++)
	{
		ref1->SetPoint(extra_points + n1, & cp1[n1 * 3]);
		ref2->SetPoint(extra_points + n1, & cp2[n1 * 3]);
	}
	
	for (/*i32s*/ n1 = 0;n1 < length + extra_points * 2 + order;n1++)
	{
		ref1->SetKnot(n1, (fGL) (n1 - extra_points) - (fGL) order / 2.0);
		ref2->SetKnot(n1, (fGL) (n1 - extra_points) - (fGL) order / 2.0);
	}
	
	list_id = prj->GetDisplayListIDs(1);
	
	i32s np1 = resol * (length - 1);
	i32s np2 = np1 + 1;
	
	data1 = new fGL[np2];
	data2a = new fGL[np2 * 3];
	data2b = new fGL[np2 * 3];
	
	for (/*i32s*/ n1 = 0;n1 < np2;n1++)
	{
		data1[n1] = ((fGL) (length - 1) * n1) / (fGL) np1;
		i32s index = (i32s) floor(data1[n1] + 0.5);
		
		fGL width1 = 0.5; if (state[index + 0] == 'H' || state[index + 0] == 'S') width1 = 1.0;
		fGL width2 = 0.5; if (state[index + 1] == 'H' || state[index + 1] == 'S') width2 = 1.0;
		
		fGL ang = M_PI * ((data1[n1] + 0.5) - (fGL) index) / 2.0;
		fGL tmp1 = cos(ang); fGL w1 = width1 * tmp1 * tmp1;
		fGL tmp2 = sin(ang); fGL w2 = width2 * tmp2 * tmp2;
		fGL ribbon_width = w1 + w2;
		
		fGL pos1[3]; ref1->Compute(data1[n1], pos1);
		fGL pos2[3]; ref2->Compute(data1[n1], pos2);
		
		v3d<fGL> v1 = v3d<fGL>(pos1);
		v3d<fGL> v2 = v3d<fGL>(pos1, pos2); v2 = v2 / 2.0;
		
		v3d<fGL> v3 = (v1 + v2) - (v2 * ribbon_width);
		v3d<fGL> v4 = (v1 + v2) + (v2 * ribbon_width);
		
		for (i32s n2 = 0;n2 < 3;n2++)
		{
			data2a[n1 * 3 + n2] = v3[n2];
			data2b[n1 * 3 + n2] = v4[n2];
		}
	}
	
	delete[] state;
	
	my_id_number = prj->GetNewObjectID();
	my_object_name = new char[64]; ostrstream ons(my_object_name, 64);
	ons << "ribbon (object #" << my_id_number << ")" << ends;
}

ribbon::~ribbon(void)
{
	delete[] cp1;
	delete[] cp2;
	
	delete ref1;
	delete[] head_points1;
	delete[] tail_points1;
	
	delete ref2;
	delete[] head_points2;
	delete[] tail_points2;
	
	delete[] data1;
	delete[] data2a;
	delete[] data2b;
	delete[] data3;
	
	prj->DeleteDisplayLists(list_id, 1);
}

void ribbon::UpdateExtraPoints(fGL ** hr, fGL * hp, fGL ** tr, fGL * tp)
{
	v3d<fGL> tmpv1; v3d<fGL> tmpv2;
	
	tmpv1 = v3d<fGL>(hr[1]);
	tmpv2 = v3d<fGL>(hr[0], hr[1]);
	for (i32s n1 = 0;n1 < extra_points;n1++)
	{
		v3d<fGL> tmpv3 = tmpv1 + (tmpv2 * ((fGL) n1 + 1));
		for (i32s n2 = 0;n2 < 3;n2++) hp[n1 * 3 + n2] = tmpv3[n2];
	}
	
	tmpv1 = v3d<fGL>(tr[1]);
	tmpv2 = v3d<fGL>(tr[0], tr[1]);
	for (/*i32s*/ n1 = 0;n1 < extra_points;n1++)
	{
		v3d<fGL> tmpv3 = tmpv1 + (tmpv2 * ((fGL) n1 + 1));
		for (i32s n2 = 0;n2 < 3;n2++) tp[n1 * 3 + n2] = tmpv3[n2];
	}
}

void ribbon::Render(void)
{
	if (glIsList(list_id) == GL_TRUE) glCallList(list_id);
	else
	{
		glNewList(list_id, GL_COMPILE_AND_EXECUTE);
		
		i32s np1 = resol * (length - 1);
		
		glEnable(GL_LIGHTING);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, true);
		
		glBegin(GL_TRIANGLES);
		for (i32s n1 = 0;n1 < np1;n1++)
		{
			v3d<fGL> va = v3d<fGL>(& data2a[(n1 + 0) * 3], & data2b[(n1 + 0) * 3]);
			v3d<fGL> vb = v3d<fGL>(& data2a[(n1 + 1) * 3], & data2b[(n1 + 1) * 3]);
			
			v3d<fGL> vc = v3d<fGL>(& data2a[(n1 + 0) * 3], & data2a[(n1 + 1) * 3]);
			v3d<fGL> vd = v3d<fGL>(& data2b[(n1 + 0) * 3], & data2b[(n1 + 1) * 3]);
			
			v3d<fGL> vA1 = va.vpr(vc); v3d<fGL> vA2 = vb.vpr(vc);
			v3d<fGL> vB1 = va.vpr(vd); v3d<fGL> vB2 = vb.vpr(vd);
			
			if (n1 != 0)
			{
				v3d<fGL> ve = v3d<fGL>(& data2a[(n1 + 0) * 3], & data2a[(n1 - 1) * 3]);
				v3d<fGL> vf = v3d<fGL>(& data2b[(n1 + 0) * 3], & data2b[(n1 - 1) * 3]);
				vA1 = vA1 + (ve.vpr(va)); vB1 = vB1 + (vf.vpr(va));
			}
			
			if (n1 != (np1 - 1))
			{
				v3d<fGL> vg = v3d<fGL>(& data2a[(n1 + 1) * 3], & data2a[(n1 + 2) * 3]);
				v3d<fGL> vh = v3d<fGL>(& data2b[(n1 + 1) * 3], & data2b[(n1 + 2) * 3]);
				vA2 = vA2 + (vb.vpr(vg)); vB2 = vB2 + (vb.vpr(vh));
			}
			
			vA1 = vA1 / vA1.len(); vA2 = vA2 / vA2.len();
			vB1 = vB1 / vB1.len(); vB2 = vB2 / vB2.len();
			
			i32s index = (i32s) floor(data1[n1]);
			fGL color1[3]; fGL mod1 = data1[n1 + 0] - (fGL) index;
			fGL color2[3]; fGL mod2 = data1[n1 + 1] - (fGL) index;
			
			for (i32s n2 = 0;n2 < 3;n2++)
			{
				color1[n2] = data3[index * 3 + n2] * (1.0 - mod1) + data3[(index + 1) * 3 + n2] * mod1;
				color2[n2] = data3[index * 3 + n2] * (1.0 - mod2) + data3[(index + 1) * 3 + n2] * mod2;
			}
			
			glColor3fv(color2);
			glNormal3fv(vA2.data); glVertex3fv(& data2a[(n1 + 1) * 3]);
			
			glColor3fv(color1);
			glNormal3fv(vA1.data); glVertex3fv(& data2a[(n1 + 0) * 3]);
			glNormal3fv(vB1.data); glVertex3fv(& data2b[(n1 + 0) * 3]);
			
			glColor3fv(color1);
			glNormal3fv(vB1.data); glVertex3fv(& data2b[(n1 + 0) * 3]);
			
			glColor3fv(color2);
			glNormal3fv(vB2.data); glVertex3fv(& data2b[(n1 + 1) * 3]);
			glNormal3fv(vA2.data); glVertex3fv(& data2a[(n1 + 1) * 3]);
		}
		
		glEnd();	// GL_TRIANGLES
		
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, false);
		glDisable(GL_LIGHTING);
		
		glEndList();
	}
}

/*################################################################################################*/

// eof
