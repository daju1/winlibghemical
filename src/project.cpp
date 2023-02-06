// PROJECT.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "StdAfx.h"

#include "project.h"	// config.h is here -> we get ENABLE-macros here...

#include "views.h"

#include "../libghemical/src/v3d.h"

#include "../libghemical/src/atom.h"
#include "../libghemical/src/bond.h"

#include "../libghemical/src/engine.h"
#include "../libghemical/src/eng1_qm.h"
#include "../libghemical/src/eng1_mm.h"
#include "../libghemical/src/eng1_sf.h"
#include "../libghemical/src/eng2_qm_mm.h"
#include "../libghemical/src/eng2_mm_sf.h"

#include "../libghemical/src/geomopt.h"
#include "../libghemical/src/intcrd.h"

#include "../libghemical/src/pop_ana.h"

#include "../libghemical/src/search.h"
#include "../libghemical/src/utility.h"


#include "../consol/filedlg.h"

#include "appdefine.h"

#include "plane.h"
#include "surface.h"

#include "color.h"
#include "views.h"

#include "filetrans.h"

//#include <gdk/gdk.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <iomanip>
#include <algorithm>
#include <fstream>
#include <strstream>
using namespace std;

/*################################################################################################*/

const char project::appversion[16] = APPVERSION;
const char project::appdata_path[256] = APPDATA_PATH;

iGLu project::list_counter = 1;		// zero is not a valid display list number...

color_mode_element project::cm_element = color_mode_element();
color_mode_secstruct project::cm_secstruct = color_mode_secstruct();
color_mode_hydphob project::cm_hydphob = color_mode_hydphob();

project::project(graphics_class_factory & p1) : model()
{
//printf("project::project(graphics_class_factory & p1) : model()\n\n");


	graphics_factory = & p1;
	
	project_path = NULL;
	project_filename = NULL;
	
	SetDefaultProjectFileName();
	
	select_buffer = new iGLu[SB_SIZE];
	selected_object = NULL;
	
	camera_counter = 1;
	object_counter = 1;
	
	pv = NULL;
	
	trajfile = NULL;
	traj_num_atoms = NOT_DEFINED;
	total_traj_frames = NOT_DEFINED;
	current_traj_frame = NOT_DEFINED;
	
	mt_a1 = mt_a2 = mt_a3 = NULL;
}

project::~project(void)
{
	selected_object = NULL;
	
	if (pv != NULL) DestroyProjectView();
	
	while (graphics_view_vector.size() > 0)
	{
		graphics_view * ref;
		ref = graphics_view_vector.back();
		RemoveGraphicsView(ref, true);
	}
	
	while (object_vector.size() > 0)
	{
		smart_object * ref;
		ref = object_vector.back();
		RemoveObject(ref);
	}
	
	if (project_path != NULL) delete[] project_path;
	if (project_filename != NULL) delete[] project_filename;
	
	delete[] select_buffer;
	
	if (trajfile != NULL) WarningMessage("Warning : trajectory file was not closed!");
}

bool project::AddTP(void * owner, transparent_primitive & tp)
{
	if (!tp.TestOwner(owner)) return false;		// this is just a double check, to make
	tp_vector.push_back(tp); return true;		// sure that we have correct "owner"...
}

void project::UpdateMPsForAllTPs(void * owner)
{
	for (i32u n1 = 0;n1 < tp_vector.size();n1++)
	{
		if (tp_vector[n1].TestOwner(owner)) tp_vector[n1].GetData()->UpdateMP();
	}
}

void project::RemoveAllTPs(void * owner)
{
	i32u n1 = 0;
	while (n1 < tp_vector.size())
	{
		vector<transparent_primitive>::iterator iter;
		
		if (!tp_vector[n1].TestOwner(owner))
		{
			n1++;
		}
		else
		{
			delete tp_vector[n1].GetData();		// destroy the data object!!!
			
			iter = (tp_vector.begin() + n1);
			tp_vector.erase(iter);
		}
	}
}

const char * project::GetProjectFileNameExtension(void)
{
	static const char ext[] = "gpr";
	return ext;
}

void project::SetProjectPath(const char * path)
{
	if (project_path != NULL) delete[] project_path;
	
	project_path = new char[strlen(path) + 1];
	strcpy(project_path, path);
}

void project::SetProjectFileName(const char * filename)
{
	if (project_filename != NULL) delete[] project_filename;
	
	project_filename = new char[strlen(filename) + 1];
	strcpy(project_filename, filename);
}

void project::SetDefaultProjectFileName(void)
{
	static i32s id_counter = 1;
	
	char buffer[1024];
	ostrstream str(buffer, sizeof(buffer));
	str << "untitled" << setw(2) << setfill('0') << id_counter++ << ends;
	
	SetProjectFileName(buffer);
}

void project::ParseProjectFileNameAndPath(const char * string)
{
	char * localstring1 = new char[strlen(string) + 1];
	strcpy(localstring1, string);
	
	i32s lastdir = NOT_DEFINED;
	for (i32s n1 = 0;n1 < (i32s) strlen(localstring1);n1++)
	{
		if (localstring1[n1] == DIR_SEPARATOR) lastdir = n1;
	}
	
	char * localstring2 = localstring1;
	
	// set project_path if needed...
	// set project_path if needed...
	// set project_path if needed...
	
	if (lastdir != NOT_DEFINED)
	{
		localstring2[lastdir] = 0;
		SetProjectPath(localstring2);
		
		localstring2 = & localstring2[lastdir + 1];
	}
	
	// and now set project_filename, without extension...
	// and now set project_filename, without extension...
	// and now set project_filename, without extension...
	
	i32s lastext = NOT_DEFINED;
	for (i32s n1 = 0;n1 < (i32s) strlen(localstring2);n1++)
	{
		if (localstring2[n1] == EXT_SEPARATOR) lastext = n1;
	}
	
	if (lastext != NOT_DEFINED)
	{
	  // This only removes an extension if it matches *our* extension,
	  // which makes problems for imported files e.g. nh3.mol.mmg1p (!) instead of nh3.mol or nh3.mm1gp
	  //		char * localstring3 = & localstring2[lastext + 1];
	  //		bool has_extension = !strcmp(localstring3, GetProjectFileNameExtension());
	  //		if (has_extension) localstring2[lastext] = 0;

	  // instead
	  localstring2[lastext] = 0;
	}
	
	SetProjectFileName(localstring2);
	
	delete[] localstring1;
}

void project::GetProjectFileName(char * buffer, int buffer_size, bool with_extension)
{
	ostrstream ostr(buffer, buffer_size);
	
	ostr << project_filename;
	if (with_extension) ostr << EXT_SEPARATOR << GetProjectFileNameExtension();
	ostr << ends;
}

void project::GetFullProjectFileName(char * buffer, int buffer_size)
{
	ostrstream ostr(buffer, buffer_size);
	
	if (project_path != NULL) ostr << project_path << DIR_SEPARATOR;
	ostr << project_filename << EXT_SEPARATOR << GetProjectFileNameExtension() << ends;
}

/*##############################################*/
/*##############################################*/

#ifdef ENABLE_OPENBABEL

bool project::ImportFile(const char * filename, int index)
{
	ifstream ifile;
	ostrstream intermed;
	file_trans translator;
	
	if (index == 0)		// Automatic detection
	{
		if (!translator.CanImport(filename))
		{
			ErrorMessage("Cannot import that file type.");
			return false;
		}

		printf("translator.CanImport(%s)", filename);
		
		ifile.open(filename, ios::in);
		translator.Import(filename, ifile, intermed);
		ifile.close();
	}
	else			// By type picked by the user
	{
		ifile.open(filename, ios::in);
		translator.Import(filename, index - 1, ifile, intermed);
		ifile.close();
	}
	
	istrstream interInput(intermed.str());
	return ReadGPR((* this), interInput, false);
}

bool project::ExportFile(const char * filename, int index)
{
	ofstream ofile;
	strstream intermed;
	file_trans translator;
	
	WriteGPR_v100((* this), intermed);		// this is for openbabel-1.100.2
	istrstream interInput(intermed.str());
	
	if (index == 0) 	// Automatic detection
	{
		if (!translator.CanExport(filename))
		{
			ErrorMessage("Cannot export that file type.");
			return false;
		}
		
		ofile.open(filename, ios::out);
		translator.Export(filename, interInput, ofile);
		ofile.close();
	}
	else			// By type picked by the user
	{
		ofile.open(filename, ios::out);
		translator.Export(filename, index - 1, interInput, ofile);
		ofile.close();
	}
	
	return true;
}

#endif	// ENABLE_OPENBABEL




void project::UnSetTheFlagOnSelectedAtoms(i32u flag_number)
{
	i32u flag = (1 << flag_number);

	for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		if ((* it1).flags & ATOMFLAG_SELECTED)
		{
			(* it1).flags &= (~flag);
		}
	}
}

void project::SetTheFlagOnSelectedAtoms(i32u flag_number)
{
	i32u flag = (1 << flag_number);

	for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		if ((* it1).flags & ATOMFLAG_SELECTED)
		{
			(* it1).flags |= flag;
		}
	}
}

void project::SelectMolecularAxises()
{
	// select none
	iter_al it1 = atom_list.begin();
	while (it1 != atom_list.end()) (* it1++).flags &= (~ATOMFLAG_SELECTED);	

	for (list<molecular_axis>::iterator it2 = molaxis_list.begin();
		it2 != molaxis_list.end(); it2++)
	{
		if ((* it2).atmr[0])
			(* it2).atmr[0]->flags |= ATOMFLAG_SELECTED;
		if ((* it2).atmr[1])
			(* it2).atmr[1]->flags |= ATOMFLAG_SELECTED;
	}
}

void project::SelectAtomsWithTheFlag(i32u flag_number)
{
	// select none
	iter_al it1 = atom_list.begin();
	while (it1 != atom_list.end()) (* it1++).flags &= (~ATOMFLAG_SELECTED);
	
	i32u flag = (1 << flag_number);

	for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		if ((* it1).flags & flag)
		{
			(* it1).flags |= ATOMFLAG_SELECTED;
		}
	}
}

void project::MoveSelectedAtoms(const f64 shift[3])
{
	for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		if (!((* it1).flags & ATOMFLAG_SELECTED))
			continue;

		const fGL * crd = (* it1).GetCRD(0);

		fGL x = crd[0] + shift[0];
		fGL y = crd[1] + shift[1];
		fGL z = crd[2] + shift[2];

		(* it1).SetCRD(0, x, y, z);
	}
}

// todo : move all these trajectory-related things into a separate class? 20040610 TH

void project::EvaluateBFact(void)
{
	if (!trajfile)
	{
		PrintToLog("EvaluateBFact() : trajectory file not open!\n");
		return;
	}
	
	vector<atom *> av;	// pick all selected atoms here...
	for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		if ((* it1).flags & ATOMFLAG_SELECTED) av.push_back(& (* it1));
	}
	
	if (!av.size())
	{
		PrintToLog("EvaluateBFact() : no selected atoms!\n");
		return;
	}
	
	fGL * avrg_str = new fGL[av.size() * 3];
	fGL * b_tab = new fGL[av.size()];
	
	for (i32u ac = 0;ac < av.size();ac++)
	{
		avrg_str[ac * 3 + 0] = 0.0;
		avrg_str[ac * 3 + 1] = 0.0;
		avrg_str[ac * 3 + 2] = 0.0;
		
		b_tab[ac] = 0.0;
	}
	
	for (i32s n1 = 0;n1 < GetTotalFrames();n1++)
	{
		SetCurrentFrame(n1);
		ReadTrajectoryFrame();
		
		for (i32u ac = 0;ac < av.size();ac++)
		{
			const fGL * tmpc = av[ac]->GetCRD(0);
			
			avrg_str[ac * 3 + 0] += tmpc[0];
			avrg_str[ac * 3 + 1] += tmpc[1];
			avrg_str[ac * 3 + 2] += tmpc[2];
		}
	}
	
	for (i32u ac = 0;ac < av.size();ac++)
	{
		avrg_str[ac * 3 + 0] /= (fGL) GetTotalFrames();
		avrg_str[ac * 3 + 1] /= (fGL) GetTotalFrames();
		avrg_str[ac * 3 + 2] /= (fGL) GetTotalFrames();
	}
	
	for (i32s n1 = 0;n1 < GetTotalFrames();n1++)
	{
		SetCurrentFrame(n1);
		ReadTrajectoryFrame();
		
		for (i32u ac = 0;ac < av.size();ac++)
		{
			const fGL * tmpc = av[ac]->GetCRD(0);
			
			fGL dx = avrg_str[ac * 3 + 0] - tmpc[0];
			fGL dy = avrg_str[ac * 3 + 1] - tmpc[1];
			fGL dz = avrg_str[ac * 3 + 2] - tmpc[2];
			
			b_tab[ac] += dx * dx;
			b_tab[ac] += dy * dy;
			b_tab[ac] += dz * dz;
		}
	}
	
	for (i32u ac = 0;ac < av.size();ac++)
	{
		b_tab[ac] /= (fGL) GetTotalFrames();
	}
	
	for (i32u ac = 0;ac < av.size();ac++)
	{
		char txtbuff[1024];
		ostrstream txts(txtbuff, sizeof(txtbuff));
		
		txts << "atom " << av[ac]->index << " ";
		txts << "displacement " << b_tab[ac] << " nm^2 = " << (b_tab[ac] * 100.0) << " Е^2 ";
		txts << " -> Bfact " << 78.957 * (b_tab[ac] * 100.0) << endl << ends;
		
		PrintToLog(txtbuff);
	}
	
	delete[] avrg_str;
	delete[] b_tab;
}

void project::EvaluateDiffConst(double dt)
{
	if (!trajfile)
	{
		PrintToLog("EvaluateDiffConst() : trajectory file not open!\n");
		return;
	}
	
	vector<atom *> av;	// pick all selected atoms here...
	for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		if ((* it1).flags & ATOMFLAG_SELECTED) av.push_back(& (* it1));
	}
	
	if (!av.size())
	{
		PrintToLog("EvaluateDiffConst() : no selected atoms!\n");
		return;
	}
	
	fGL * init_loc = new fGL[av.size() * 3];
	double * dc_tab = new double[av.size()];
	
	SetCurrentFrame(0);
	ReadTrajectoryFrame();
	
	for (i32u ac = 0;ac < av.size();ac++)
	{
		const fGL * tmpc = av[ac]->GetCRD(0);
		
		init_loc[ac * 3 + 0] = tmpc[0];
		init_loc[ac * 3 + 1] = tmpc[1];
		init_loc[ac * 3 + 2] = tmpc[2];
		
		dc_tab[ac] = 0.0;
	}
	
	double time = 0.0;
	for (i32s n1 = 1;n1 < GetTotalFrames();n1++)
	{
		time += dt;
		
		SetCurrentFrame(n1);
		ReadTrajectoryFrame();
		
		for (i32u ac = 0;ac < av.size();ac++)
		{
			const fGL * tmpc = av[ac]->GetCRD(0);
			
			double dist = 0.0; double tmpd;
			tmpd = tmpc[0] - init_loc[ac * 3 + 0]; dist += tmpd * tmpd;
			tmpd = tmpc[1] - init_loc[ac * 3 + 1]; dist += tmpd * tmpd;
			tmpd = tmpc[2] - init_loc[ac * 3 + 2]; dist += tmpd * tmpd;
			
			// convert nm^2 into cm^2 ; 10^-14
			// convert fs into s ; 10^-15
			
			double dc = (dist * 1.0e-14) / (time * 1.0e-15);
			dc_tab[ac] += dc;
		}
	}
	
	for (i32u ac = 0;ac < av.size();ac++)
	{
		dc_tab[ac] /= (double) (GetTotalFrames() - 1);
	}
	
	for (i32u ac = 0;ac < av.size();ac++)
	{
		char txtbuff[1024];
		ostrstream txts(txtbuff, sizeof(txtbuff));
		
		txts << "atom " << av[ac]->index << " ";
		txts << "diffconst " << (dc_tab[ac] * 1.0e+5) << " * 10^-5 cm^2/s" << endl << ends;
		
		PrintToLog(txtbuff);
	}
	
	delete[] init_loc;
	delete[] dc_tab;
}

/*##############################################*/
/*##############################################*/

void project::AddH(void)
{
/*	file_trans ft;
	OBMol * obm = ft.CopyAll(this);
	obm->AddHydrogens(false, false);
	ft.Synchronize();	*/
	
// above is the OpenBabel implementation of hydrogen adding.
// it seems to use bond length information to determine how many H's to add,
// which is problematic for protein X-ray structures (that often are not precise enough).
// TODO : make all these alternative add_h implementations available!!!!!!!!!!!!!!!

	AddHydrogens();		// this is the library implementation...
	
	PrintToLog("Hydrogens added.\n");
}

void project::RemoveH(void)
{
	RemoveHydrogens();
	
	PrintToLog("Hydrogens removed.\n");
}

iGLu project::GetDisplayListIDs(iGLu p1)
{
	iGLu first = list_counter;
	list_counter += p1;
	
	return first;
}

void project::DeleteDisplayLists(iGLu p1, iGLu p2)
{
	for (i32u n1 = 0;n1 < graphics_view_vector.size();n1++)
	{
		graphics_view_vector[n1]->SetCurrent();
		glDeleteLists(p1, p2);
	}
}

/*##############################################*/
/*##############################################*/

void project::AddAtom(atom & p1)
{
	model::AddAtom(p1);
	if (pv != NULL) pv->AtomAdded(& atom_list.back());
}

void project::RemoveAtom(iter_al p1)
{
	if (pv != NULL) pv->AtomRemoved(& (* p1));
	model::RemoveAtom(p1);
}

void project::AddBond(bond & p1)
{
	model::AddBond(p1);
	if (pv != NULL) pv->BondAdded(& bond_list.back());
}

void project::RemoveBond(iter_bl p1)
{
	if (pv != NULL) pv->BondRemoved(& (* p1));
	model::RemoveBond(p1);
}

void project::InvalidateGroups(void)
{
	model::InvalidateGroups();
	if (pv != NULL) pv->ClearChainsView();
}

void project::UpdateChains(void)
{
	model::UpdateChains();
	if (pv != NULL) pv->BuildChainsView();
}

/*##############################################*/
/*##############################################*/

i32s project::IsLight(const dummy_object * p1)
{
	i32s index = NOT_DEFINED;
	for (i32u n1 = 0;n1 < light_vector.size();n1++)
	{
		if (light_vector[n1] == p1) index = n1;
	}
	
	return index;
}

bool project::SelectLight(const dummy_object * p1)
{
	i32s n1 = IsLight(p1);
	if (n1 < 0) return false;
	
	selected_object = light_vector[n1];
	
	return true;
}

bool project::AddGlobalLight(light * p1)
{
	iGLs max_local_size = 0;
	for (i32u n1 = 0;n1 < camera_vector.size();n1++)
	{
		iGLs local_size = CountLocalLights(camera_vector[n1]);
		if (local_size > max_local_size) max_local_size = local_size;
	}
	
	iGLs total_lights = CountGlobalLights() + max_local_size;
	
// we really can't do this test here ; see SetupLights()...
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//iGLs max_lights; glGetIntegerv(GL_MAX_LIGHTS, & max_lights);
//if (total_lights == max_lights) return false;
	
	light_vector.push_back(p1);
	SetGlobalLightNumbers();
	
	for (i32u n1 = 0;n1 < camera_vector.size();n1++)
	{
		SetLocalLightNumbers(camera_vector[n1]);
		SetupLights(camera_vector[n1]);
	}
	
	if (pv != NULL) pv->LightAdded(p1);
	
	selected_object = light_vector.back();
	
	cout << "added global light." << endl;
	return true;
}

bool project::AddLocalLight(light * p1, camera * p2)
{
//printf("project::AddLocalLight(light * p1, camera * p2)\n\n");

	iGLs total_lights = CountGlobalLights() + CountLocalLights(p2);

// we really can't do this test here ; see SetupLights()...
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//iGLs max_lights; glGetIntegerv(GL_MAX_LIGHTS, & max_lights);
//if (total_lights == max_lights) return false;
	
	p1->owner = p2; light_vector.push_back(p1);
	SetLocalLightNumbers(p2); SetupLights(p2);
	
	if (pv != NULL) pv->LightAdded(p1);
	
	selected_object = light_vector.back();
	
	cout << "added local light." << endl;
	return true;
}

bool project::RemoveLight(dummy_object * p1)
{
	i32s n1 = IsLight(p1);
	if (n1 < 0) return false;
	
	if (pv != NULL) pv->LightRemoved(light_vector[n1]);
	
	camera * owner = light_vector[n1]->owner;
	light_vector.erase(light_vector.begin() + n1);
	delete p1;
	
	if (owner != NULL)
	{
		SetLocalLightNumbers(owner);
		SetupLights(owner);
	}
	else
	{
		SetGlobalLightNumbers();
		for (i32u n1 = 0;n1 < camera_vector.size();n1++)
		{
			SetLocalLightNumbers(camera_vector[n1]);
			SetupLights(camera_vector[n1]);
		}
	}
	
	return true;
}

iGLs project::CountGlobalLights(void)
{
	iGLs sum = 0; i32u n1 = 0;
	while (n1 < light_vector.size())
	{
		if (light_vector[n1++]->owner == NULL) sum++;
	}
	
	return sum;
}

iGLs project::CountLocalLights(camera * p1)
{
	iGLs sum = 0; i32u n1 = 0;
	while (n1 < light_vector.size())
	{
		if (light_vector[n1++]->owner == p1) sum++;
	}
	
	return sum;
}

bool project::SetupLights(camera * p1)
{
//printf("project::SetupLights(camera * p1)\n\n");
	
	if (p1->prj != this) return false;
	
	for (i32u n1 = 0;n1 < graphics_view_vector.size();n1++)
	{
		if (graphics_view_vector[n1]->cam != p1) continue;
		
		// the following call to SetCurrent() might work, or might not work.
		// it is really difficult to know, since it depends on whether the graphics
		// system is properly initialized or not (widget realized in GTK+).
		
		// if the call fails, we just skip everything...
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		
		bool failed = !graphics_view_vector[n1]->SetCurrent();
		if (failed) { cout << "GL is not yet initialized -> skipping light setup!" << endl; continue; }
		
		iGLs max_lights;
		glGetIntegerv(GL_MAX_LIGHTS, & max_lights);
		
		for (iGLs n2 = 0;n2 < max_lights;n2++)
		{
			glDisable((GLenum) (GL_LIGHT0 + n2));
		}
		{
		for (i32u n2 = 0;n2 < light_vector.size();n2++)
		{
			bool test1 = (light_vector[n2]->owner != NULL);
			bool test2 = (light_vector[n2]->owner != p1);
			if (test1 && test2) continue;
			
			light_vector[n2]->SetupProperties(); bool test = false;
			if (light_vector[n2]->owner == NULL && p1->use_global_lights) test = true;
			if (light_vector[n2]->owner == p1 && p1->use_local_lights) test = true;
			if (test) glEnable((GLenum) light_vector[n2]->number);
		}
		}
	}
	
	return true;
}

void project::SetGlobalLightNumbers(void)
{
	iGLs counter = 0;
	for (i32u n1 = 0;n1 < light_vector.size();n1++)
	{
		if (light_vector[n1]->owner != NULL) continue;
		light_vector[n1]->number = GL_LIGHT0 + counter++;
	}
}

void project::SetLocalLightNumbers(camera * p1)
{
//printf("project::SetLocalLightNumbers(camera * p1)\n\n");

	iGLs counter = CountGlobalLights();
	for (i32u n1 = 0;n1 < light_vector.size();n1++)
	{
		if (light_vector[n1]->owner != p1) continue;
		light_vector[n1]->number = GL_LIGHT0 + counter++;
	}
}

/*##############################################*/
/*##############################################*/

i32s project::IsObject(const dummy_object * p1)
{
	i32s index = NOT_DEFINED;
	for (i32u n1 = 0;n1 < object_vector.size();n1++)
	{
		if (object_vector[n1] == p1) index = n1;
	}
	
	return index;
}

bool project::SelectObject(const dummy_object * p1)
{
	i32s n1 = IsObject(p1);
	if (n1 < 0) return false;
	
	selected_object = object_vector[n1];
	
	return true;
}

void project::AddObject(smart_object * p1)
{
	object_vector.push_back(p1);
	selected_object = object_vector.back();
	
	if (pv != NULL) pv->ObjectAdded(p1);
}

bool project::RemoveObject(dummy_object * p1)
{
	i32s n1 = IsObject(p1);
	if (n1 < 0) return false;
	
	if (pv != NULL) pv->ObjectRemoved(object_vector[n1]);
	
	object_vector.erase(object_vector.begin() + n1);
	delete p1; return true;
}

// these are the measuring functions, that only take coordinates as input (so they are model-independent)...
// these are the measuring functions, that only take coordinates as input (so they are model-independent)...
// these are the measuring functions, that only take coordinates as input (so they are model-independent)...

float measure_len(float * c1, float * c2)
{
	v3d<float> v1(c1, c2);
	return v1.len();
}

float measure_ang(float * c1, float * c2, float * c3)
{
/*	Vector v1, v2;
	v1 = Vector(c1[0] - c2[0], c1[1] - c2[1], c1[2] - c2[2]);
	v2 = Vector(c3[0] - c2[0], c3[1] - c2[1], c3[2] - c2[2]);
	return VectorAngle(v1, v2);	*/
	
	v3d<float> v1(c2, c1);
	v3d<float> v2(c2, c3);
	return v1.ang(v2) * 180.0 / M_PI;
}

float measure_tor(float * c1, float * c2, float * c3, float * c4)
{
/*	Vector v1, v2, v3, v4;
	v1 = Vector(c1[0], c1[1], c1[2]) * 10.0f;
	v2 = Vector(c2[0], c2[1], c2[2]) * 10.0f;
	v3 = Vector(c3[0], c3[1], c3[2]) * 10.0f;
	v4 = Vector(c4[0], c4[1], c4[2]) * 10.0f;
	return CalcTorsionAngle(v1, v2, v3, v4);	*/
	
	v3d<float> v1(c2, c1);
	v3d<float> v2(c2, c3);
	v3d<float> v3(c3, c4);
	return v1.tor(v2, v3) * 180.0 / M_PI;
}

/*##############################################*/
/*##############################################*/

project_view * project::CreateProjectView(void)
{
//printf("project_view * project::CreateProjectView(void)\n\n");
	if (pv != NULL) return pv;
	else
	{
		pv = GetGraphicsFactory()->ProduceProjectView(this);
		return pv;
	}
}

bool project::DestroyProjectView(void)
{
	if (!pv) return false;
	else
	{
		delete pv;
		pv = NULL;
		
		return true;
	}
}

graphics_view * project::AddGraphicsView(camera * p1, bool detached)
{
//printf("graphics_view * project::AddGraphicsView(camera * p1, bool detached)\n\n");

	camera * cam = p1;
	
	if (!p1)
	{
		fGL focus = GetDefaultFocus();
		camera_vector.push_back(new camera(ol_static(), focus, this));
		cam = camera_vector.back();
		
		if (pv != NULL) pv->CameraAdded(camera_vector.back());
	}
	
	graphics_view_vector.push_back(GetGraphicsFactory()->ProduceGraphicsView(this, cam, detached));
	
	if (pv != NULL) pv->GraphicsViewAdded(graphics_view_vector.back());
	
	if (!p1)
	{
		// also add a default light, so that we can see something...
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		
		AddLocalLight(new directional_light(ol_static(), GetNewObjectID()), cam);
	}
	else
	{
		// setup the old lights that we already have...
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		
		SetupLights(cam);
	}
	
	UpdateAllWindowTitles();
	return graphics_view_vector.back();
}

bool project::RemoveGraphicsView(graphics_view * p1, bool force)
{
	i32s n1 = NOT_DEFINED;
	for (i32u n2 = 0;n2 < camera_vector.size();n2++)
	{
		if (camera_vector[n2] == p1->cam) n1 = n2;
	}
	
	if (n1 == NOT_DEFINED)		// this should never happen...
	{
		cout << "WARNING: could not find the camera at project::RemoveGraphicsView()." << endl;
		return false;
	}
	
	i32s other_views_for_this_cam = 0;	// this is also calculated at IsThisLastGraphicsView().
	{
	i32s n2 = NOT_DEFINED;
	for (i32u n3 = 0;n3 < graphics_view_vector.size();n3++)
	{
		if (graphics_view_vector[n3] == p1) n2 = n3;
		else if (graphics_view_vector[n3]->cam == p1->cam) other_views_for_this_cam++;
	}
	
	if (n2 == NOT_DEFINED)		// this should never happen...
	{
		cout << "WARNING: could not find the view at project::RemoveGraphicsView()." << endl;
		return false;
	}
	
	if (!force && (!other_views_for_this_cam && camera_vector.size() < 2))	// refuse to close the last window!!!
	{
		ErrorMessage("This is the last graphics view for\nthis project - can't close it.");
		return false;
	}
	
	// if this was the last view for a camera, we have to remove also the camera.
	// but before that we must remove all local lights that relate to the camera...
	
	// a light object can also be selected_object, so we must compare the pointers
	// and invalidate selected_object if necessary...
	
	if (!other_views_for_this_cam)
	{
		i32u n3 = 0;
		while (n3 < light_vector.size())
		{
			if (light_vector[n3]->owner == camera_vector[n1])
			{
				if (selected_object != NULL)
				{
					if (selected_object == light_vector[n3])
					{
						selected_object = NULL;
					}
				}
				
				if (pv != NULL) pv->LightRemoved(light_vector[n3]);
				
				delete light_vector[n3];
				light_vector.erase(light_vector.begin() + n3);
			}
			else n3++;
		}
		
		SetGlobalLightNumbers();
		for (n3 = 0;n3 < camera_vector.size();n3++)
		{
			if (n1 == (i32s) n3) continue;
			
			SetLocalLightNumbers(camera_vector[n3]);
			SetupLights(camera_vector[n3]);
		}
	}
	
	// now it's time to remove the window...
	
	if (pv != NULL) pv->GraphicsViewRemoved(graphics_view_vector[n2]);
	
	delete graphics_view_vector[n2];
	graphics_view_vector.erase(graphics_view_vector.begin() + n2);
	
	// and now we finally can remove the camera, if needed...
	
	if (!other_views_for_this_cam)
	{
		if (pv != NULL) pv->CameraRemoved(camera_vector[n1]);
		
		delete camera_vector[n1];
		camera_vector.erase(camera_vector.begin() + n1);
	}
	
	UpdateAllWindowTitles();
	}
	return true;
}

bool project::IsThisLastGraphicsView(graphics_view * p1)
{
	i32s other_views_for_this_cam = 0;
	for (i32u n1 = 0;n1 < graphics_view_vector.size();n1++)
	{
		bool test1 = (graphics_view_vector[n1] == p1);
		bool test2 = (graphics_view_vector[n1]->cam == p1->cam);
		
		if (!test1 && test2) other_views_for_this_cam++;
	}
	
	if (!other_views_for_this_cam && camera_vector.size() < 2) return true;
	else return false;
}

plot1d_view * project::AddPlot1DView(i32s ud1, const char * s1, const char * sv, bool detached)
{
	plot1d_view * p1dv = graphics_factory->ProducePlot1DView(this, ud1, s1, sv, detached);
	plotting_view_vector.push_back(p1dv);
	
	if (pv != NULL) pv->PlottingViewAdded(plotting_view_vector.back());
	
	return p1dv;
}

plot2d_view * project::AddPlot2DView(i32s ud2, const char * s1, const char * s2, const char * sv, bool detached)
{
	plot2d_view * p2dv = graphics_factory->ProducePlot2DView(this, ud2, s1, s2, sv, detached);
	plotting_view_vector.push_back(p2dv);
	
	if (pv != NULL) pv->PlottingViewAdded(plotting_view_vector.back());
	
	return p2dv;
}

rcp_view * project::AddReactionCoordinatePlotView(i32s ud1, const char * s1, const char * sv, bool detached)
{
	rcp_view * rcpv = graphics_factory->ProduceRCPView(this, ud1, s1, sv, detached);
	plotting_view_vector.push_back(rcpv);
	
	if (pv != NULL) pv->PlottingViewAdded(plotting_view_vector.back());
	
	return rcpv;
}

eld_view * project::AddEnergyLevelDiagramView(bool detached)
{
	eld_view * eldv = graphics_factory->ProduceELDView(this, detached);
	plotting_view_vector.push_back(eldv);
	
	if (pv != NULL) pv->PlottingViewAdded(plotting_view_vector.back());
	
	return eldv;
}

bool project::RemovePlottingView(plotting_view * p1)
{
	i32s n1 = NOT_DEFINED;
	for (i32u n2 = 0;n2 < plotting_view_vector.size();n2++)
	{
		if (plotting_view_vector[n2] == p1) n1 = n2;
	}
	
	if (n1 == NOT_DEFINED)		// this should never happen...
	{
		cout << "WARNING: could not find the view at project::RemovePlottingView()." << endl;
		return false;
	}
	
	if (pv != NULL) pv->PlottingViewRemoved(plotting_view_vector[n1]);
	
	delete plotting_view_vector[n1];
	plotting_view_vector.erase(plotting_view_vector.begin() + n1);
	
	return true;
}

void project::UpdateAllViews(void)
{
	UpdateAllGraphicsViews();
}

void project::UpdateAllWindowTitles(void)	// is still needed in attach/detach stuff...
{
        char buffer[1024];

	for (i32u n1 = 0;n1 < camera_vector.size();n1++)
	{
		for (i32u n2 = 0;n2 < graphics_view_vector.size();n2++)
		{
			if (graphics_view_vector[n2]->cam != camera_vector[n1]) continue;
			
			ostrstream str(buffer, sizeof(buffer));
			str << "camera #" << camera_vector[n1]->my_c_number << " view #" << graphics_view_vector[n2]->my_gv_number << ends;
			graphics_view_vector[n2]->SetTitle(buffer);
		}
	}
	
	if (pv != NULL) pv->UpdateAllWindowTitles();
}

void project::UpdateAllGraphicsViews(bool flag)
{
	for (i32u n1 = 0;n1 < graphics_view_vector.size();n1++)
	{
		graphics_view_vector[n1]->Update(flag);
	}
}

void project::UpdateGraphicsViews(camera * cam, bool flag)
{
	for (i32u n1 = 0;n1 < graphics_view_vector.size();n1++)
	{
		if (graphics_view_vector[n1]->cam != cam) continue;
		graphics_view_vector[n1]->Update(flag);
	}
}

void project::UpdateGraphicsView(graphics_view * gv, bool flag)
{
	gv->Update(flag);
}

void project::UpdatePlottingViews(void)
{
	for (i32u n1 = 0;n1 < plotting_view_vector.size();n1++)
	{
	// energy-level diagrams might require scaling here?!?!?!
	// detect them and call the scaling function... ANY BETTER WAY TO DO THIS???
		eld_view * eldv = dynamic_cast<eld_view *>(plotting_view_vector[n1]);
		if (eldv != NULL) eldv->SetCenterAndScale();
		
		plotting_view_vector[n1]->Update();
	}
}

void project::RenderAllTPs(graphics_view * gv, rmode)
{
	// here we will render those transparent primitives...
	// all models should call this before returning from their versions!!!
	
	// first we must update the distances for all TP's, and sort them...
	
	const fGL * ref1 = gv->cam->GetLocData()->crd;
	const fGL * ref2 = gv->cam->GetLocData()->zdir.data;
	
	for (i32u n1 = 0;n1 < tp_vector.size();n1++)
	{
		tp_vector[n1].UpdateDistance(ref1, ref2);
	}
	
	sort(tp_vector.begin(), tp_vector.end());
	
	// and then we will just render them...
	// it looks better if we disable depth buffer changes...
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(false); glEnable(GL_BLEND);
	
	for (i32u n1 = 0;n1 < tp_vector.size();n1++)
	{
		tp_vector[n1].GetData()->Render();
	}
	
	glDisable(GL_BLEND); glDepthMask(true);
}

/*##############################################*/
/*##############################################*/

void project::ProcessCommandString(graphics_view * gv, const char * command)
{
	char mbuff1[512];
	ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << "Processing Command : " << command << endl << ends;
	PrintToLog(mbuff1);
	
	istrstream istr(command);
	char kw1[32]; istr >> kw1;	// the 1st keyword.
	
	if (!strcmp("help", kw1))
	{
		PrintToLog("> AVAILABLE COMMANDS:\n");		// use alphabetical order???
		
		PrintToLog("> add light (local/global) (directional/spotlight) -- add a new light object.\n");
		
		PrintToLog("> add plane <vf> <cf> <cscale1> <AUTO/cscale2> <dim> <res> <tp> <alpha> -- add a plane object.\n");
		PrintToLog(">   where: <vf> = value function : esp vdws eldens mo mod unity\n");
		PrintToLog(">          <cf> = colour function : red green blue rb1 rb2\n");
		PrintToLog(">          <cscale1> = scaling value for calculating the colours\n");
		PrintToLog(">          <cscale2> = scaling offset for calculating the colours\n");
		PrintToLog(">          <dim> = dimension of the plane object (in nm units)\n");
		PrintToLog(">          <res> = resolution of the plane object\n");
		PrintToLog(">          <tp> = 0 or 1 telling if the object is transparent\n");
		PrintToLog(">          <alpha> = transparency alpha value\n");
		
		PrintToLog("> add volrend <vf> <cf> <cscale1> <AUTO/cscale2> <dim> <res> <alpha> -- add a volume-rendering object.\n");
		PrintToLog(">   where: <vf> = value function : esp vdws eldens mo mod unity\n");
		PrintToLog(">          <cf> = colour function : red green blue rb1 rb2\n");
		PrintToLog(">          <cscale1> = scaling value for calculating the colours\n");
		PrintToLog(">          <cscale2> = scaling offset for calculating the colours\n");
		PrintToLog(">          <dim> = dimension of the plane object (in nm units)\n");
		PrintToLog(">          <res> = resolution of the plane object\n");
		PrintToLog(">          <alpha> = transparency alpha value\n");
		
		PrintToLog("> add surf1 <vf1> <vf2> <cf> <sscale> <cscale1> <AUTO/cscale2> <dim> <res> <solid> <tp> <alpha> -- add a single surface object.\n");
		PrintToLog(">   where: <vf1> = value function for calculating the surface : esp vdws eldens mo mod unity\n");
		PrintToLog(">          <vf2> = value function for calculating the colours : esp vdws eldens mo mod unity\n");
		PrintToLog(">          <cf> = colour function : red green blue rb1 rb2\n");
		PrintToLog(">          <sscale> = scaling value for calculating the surface\n");
		PrintToLog(">          <cscale1> = scaling value for calculating the colours\n");
		PrintToLog(">          <cscale2> = scaling offset for calculating the colours\n");
		PrintToLog(">          <dim> = dimension of the plane object (in nm units)\n");
		PrintToLog(">          <res> = resolution of the plane object\n");
		PrintToLog(">          <solid> = 0 or 1 telling if the object is solid\n");
		PrintToLog(">          <tp> = 0 or 1 telling if the object is transparent\n");
		PrintToLog(">          <alpha> = transparency alpha value\n");
		
		PrintToLog("> add surf2 <vf1> <vf2> <cf1> <cf2> <sscale1> <sscale2> <cscale1> <AUTO/cscale2> <dim> <res> <solid> <tp> <alpha> -- add a pair of surface objects.\n");
		PrintToLog(">   where: <vf1> = value function for calculating the surface : esp vdws eldens mo mod unity\n");
		PrintToLog(">          <vf2> = value function for calculating the colours : esp vdws eldens mo mod unity\n");
		PrintToLog(">          <cf1> = colour function for 1st surface : red green blue rb1 rb2\n");
		PrintToLog(">          <cf2> = colour function for 2nd surface : red green blue rb1 rb2\n");
		PrintToLog(">          <sscale1> = scaling value for calculating the surface for 1st surface\n");
		PrintToLog(">          <sscale2> = scaling value for calculating the surface for 2nd surface\n");
		PrintToLog(">          <cscale1> = scaling value for calculating the colours\n");
		PrintToLog(">          <cscale2> = scaling offset for calculating the colours\n");
		PrintToLog(">          <dim> = dimension of the plane object (in nm units)\n");
		PrintToLog(">          <res> = resolution of the plane object\n");
		PrintToLog(">          <solid> = 0 or 1 telling if the object is solid\n");
		PrintToLog(">          <tp> = 0 or 1 telling if the object is transparent\n");
		PrintToLog(">          <alpha> = transparency alpha value\n");
		PrintToLog("> \n");
		
		PrintToLog("> help -- print all available commands in command strings.\n");
		PrintToLog("> \n");
		
		PrintToLog("> energy -- calculate a single-point energy.\n");
		PrintToLog("> test -- my test.\n");
		PrintToLog("> geom_opt -- do a geometry optimization run using default options.\n");
		PrintToLog("> mol_dyn -- do a molecular dynamics run using default options.\n");
		PrintToLog("> \n");
		
		PrintToLog("> random_search <cycles> <optsteps> -- perform a random conformational search.\n");
		PrintToLog("> systematic_search <divisions> <optsteps> -- perform a systematic conformational search.\n");
		PrintToLog("> montecarlo_search <init_cycles> <simul_cycles> <optsteps> -- perform a MonteCarlo search.\n");
		PrintToLog("> \n");
		
		PrintToLog("> sel_atoms_with_flag FLAG_NUM -- selected atoms with any flag\n");
		PrintToLog("> set_flag_on_sel_atoms FLAG_NUM -- set to all selected atoms any flag\n");
		PrintToLog("> unset_flag_on_sel_atoms FLAG_NUM -- unset to all selected atoms any flag\n");
		PrintToLog("> move_sel_atoms DX DY DZ -- Move Selected Atoms\n");
		PrintToLog("> \n");

		PrintToLog("> traj_set_total_frames nframes -- correct total frames in traj file.\n");
		PrintToLog("> \n");

		PrintToLog("> make_plot_crd IND DIM -- create a 1D coordinate vs. step plot on traj file.\n");
		PrintToLog("> make_plot_crd_diff IND1 IND2 DIM -- create a 1D coordinate difference plot on traj file.\n");
		PrintToLog("> make_nematic_plot IA IB -- create a 1D Nematic Coordinate vs. step plot on traj file.\n");
		PrintToLog("> make_plot_dist A B -- create a 1D distance vs. step plot on traj file.\n");
		PrintToLog("> make_plot_dist A B -- create a 1D distance vs. step plot on traj file.\n");
		PrintToLog("> make_plot_ang A B C -- create a 1D angle vs. step plot on traj file.\n");
		PrintToLog("> vel_dist_2d divx divy dt -- create a VeloncityDistribution2D plot on traj file.\n");
		PrintToLog("> make_plot_a A B C <div> <start_ang> <end_ang> <optsteps> -- create a 1D energy vs. angle plot.\n");
		PrintToLog("> make_plot1 A B C D <div> <start_ang> <end_ang> <optsteps> -- create a 1D energy vs. torsion plot.\n");
		PrintToLog("> make_plot2 A B C D <div> <start_ang> <end_ang> I J K L <div> <start_ang> <end_ang> <optsteps> -- create a 2D energy vs. torsions plot.\n");
		PrintToLog("> \n");
		
		PrintToLog("> population_analysis_ESP -- determine atomic charges using an ESP fit (for QM methods only).\n");
		PrintToLog("> \n");
		
		PrintToLog("> transition_state_search <delta_e> <initial_fc> -- perform a transition state search (for QM methods only).\n");
		PrintToLog("> stationary_state_search <steps> -- perform a search for a structure with no forces.\n");
		PrintToLog("> \n");
		
		PrintToLog("> set_current_orbital <orbital_index> -- set the current orbtal index for plotting the orbitals.\n");
		PrintToLog("> \n");

		PrintToLog("> build_amino <sequence> (helix/strand) -- amino acid sequence builder.\n");
		PrintToLog("> build_nucleic <sequence> -- nucleic acid sequence builder.\n");
		PrintToLog("> \n");
		
		PrintToLog("> box (<x-dim> <y-dim> <z-dim>) -- set or get box size.\n");
		PrintToLog("> boxdim (<dim> <val>) -- set or get one box dimention size.\n");
		PrintToLog("> \n");
		
		PrintToLog("> nematic_box <x-dim> <y-dim> <z-dim> (<filename>) -- setup a nematic box ; UNDER_CONSTRUCTION.\n");
		PrintToLog("> solvate_box <x-dim> <y-dim> <z-dim> (<density> <filename> (export)) -- setup a solvation box ; UNDER_CONSTRUCTION.\n");
		PrintToLog("> solvate_sphere <rad_solute> <rad_solvent> (<density> <filename>) -- setup a solvation sphere.\n");
		PrintToLog("> \n");
		
		PrintToLog("> set_formal_charge <index> <charge> -- set formal charges to atoms.\n");
		PrintToLog("> \n");
		
		PrintToLog("> evaluate_Bfact -- evaluate B-factors for selected atoms (a trajectory file must be open).\n");
		PrintToLog("> evaluate_diffconst <dt> -- evaluate diffusion constants for selected atoms (a trajectory file must be open, dt = time difference between frames [fs]).\n");
		PrintToLog("> \n");
		
		return;
	}
	
	if (!strcmp("add", kw1))
	{
		char kw2[32]; istr >> kw2;	// the 2nd keyword; type of the object to add.
		char object_name[128];
		
		if (!strcmp("light", kw2))
		{
			char kw3[32]; istr >> kw3;
			char kw4[32]; istr >> kw4;
			
			bool is_local = true;
			if (kw3[0] == 'g') is_local = false;
			
			bool is_directional = true;
			if (kw4[0] == 's') is_directional = false;
			
			light * new_light;
			if (is_directional) new_light = new directional_light(ol_static(), GetNewObjectID());
			else
			{
				new_light = new spot_light(ol_static(), GetNewObjectID());
				
				const fGL trans[3] = { 0.0, 0.0, -1.0 };
				new_light->TranslateObject((const fGL *) trans, gv->cam->GetLocData());
			}
			
			if (!is_local) AddGlobalLight(new_light);
			else AddLocalLight(new_light, gv->cam);
			
			if (!is_local || graphics_view::draw_info) UpdateAllGraphicsViews();
			else if (is_local) UpdateGraphicsViews(gv->cam);
			
			ostrstream strR(mbuff1, sizeof(mbuff1));
			strR << "Added a new object : light (";
			strR << (is_local ? "local" : "global") << " ";
			strR << (is_directional ? "directional" : "spotlight") << ")." << endl << ends;
			PrintToLog(mbuff1);
			return;
		}
		
		if (!strcmp("plane", kw2))
		{
			// add plane esp rb2 1 1 1 1 1 0.5
			// add plane vdws rb2 1 1 1 1 1 0.5
			char kw3[32]; istr >> kw3;
			char kw4[32]; istr >> kw4;
			char kw5[32]; istr >> kw5;
			char kw6[32]; istr >> kw6;
			char kw7[32]; istr >> kw7;
			char kw8[32]; istr >> kw8;
			char kw9[32]; istr >> kw9;
			char kwA[32]; istr >> kwA;
			char ** endptr = NULL;		

			cp_param cpp;
			cpp.prj = this; 
			
			cpp.ref = GetCurrentSetup()->GetCurrentEngine();			
			if (cpp.ref == NULL){
				GetCurrentSetup()->CreateCurrentEngine();
				cpp.ref = GetCurrentSetup()->GetCurrentEngine();
			}

			if (!strcmp(kw3, "esp")) cpp.vf = (ValueFunction *) value_ESP;
			else if (!strcmp(kw3, "vdws")) cpp.vf = (ValueFunction *) value_VDWSurf;
			else if (!strcmp(kw3, "eldens")) cpp.vf = (ValueFunction *) value_ElDens;
			else if (!strcmp(kw3, "mo")) cpp.vf = (ValueFunction *) value_Orbital;
			else if (!strcmp(kw3, "mod")) cpp.vf = (ValueFunction *) value_OrbDens;
			else if (!strcmp(kw3, "unity")) cpp.vf = (ValueFunction *) GetUnity;
			else { PrintToLog("ERROR : add plane : unknown value function.\n"); return; }
			
			if (!strcmp(kw4, "red")) cpp.cf = (ColorFunction *) GetRedColor;
			else if (!strcmp(kw4, "green")) cpp.cf = (ColorFunction *) GetGreenColor;
			else if (!strcmp(kw4, "blue")) cpp.cf = (ColorFunction *) GetBlueColor;
			else if (!strcmp(kw4, "rb1")) cpp.cf = (ColorFunction *) GetRBRange1;
			else if (!strcmp(kw4, "rb2")) cpp.cf = (ColorFunction *) GetRBRange2;
			else { PrintToLog("ERROR : add plane : unknown colour function.\n"); return; }
			
			f64 cscale1 = strtod(kw5, endptr);
			
			f64 cscale2 = 0.0; bool auto_cv2 = false;
			if (!strcmp(kw6, "AUTO")) auto_cv2 = true;
			else cscale2 = strtod(kw6, endptr);
			
			f64 dim = strtod(kw7, endptr);
			
			i32s res = strtol(kw8, endptr, 10);
			if (res < 2) res = 2;
			
			i32s tp = strtol(kw9, endptr, 10);
			if (tp < 0) tp = 0; if (tp > 1) tp = 1;
			
			f64 alpha = strtod(kwA, endptr);
			
			cpp.dim = dim; cpp.np = res;
			cpp.transparent = tp; cpp.automatic_cv2 = auto_cv2;
			
			cpp.cvalue1 = cscale1;
			cpp.cvalue2 = cscale2;
			cpp.alpha = alpha;
			
			ostrstream strN(object_name, sizeof(object_name));
			strN << kw3 << "-" << ends;
			
			AddObject(new color_plane_object(ol_static(), cpp, object_name));
			UpdateAllGraphicsViews();
			
			ostrstream strR(mbuff1, sizeof(mbuff1));
			strR << "Added a new object : plane (" << kw3 << " " << kw4 << ")." << endl << ends;
			PrintToLog(mbuff1);
			return;
		}
		
		if (!strcmp("volrend", kw2))
		{
			char kw3[32]; istr >> kw3;
			char kw4[32]; istr >> kw4;
			char kw5[32]; istr >> kw5;
			char kw6[32]; istr >> kw6;
			char kw7[32]; istr >> kw7;
			char kw8[32]; istr >> kw8;
			char kw9[32]; istr >> kw9;
			char ** endptr = NULL;
			
			cp_param cpp;
			cpp.prj = this; cpp.ref = GetCurrentSetup()->GetCurrentEngine();
			
			if (!strcmp(kw3, "esp")) cpp.vf = (ValueFunction *) value_ESP;
			else if (!strcmp(kw3, "vdws")) cpp.vf = (ValueFunction *) value_VDWSurf;
			else if (!strcmp(kw3, "eldens")) cpp.vf = (ValueFunction *) value_ElDens;
			else if (!strcmp(kw3, "mo")) cpp.vf = (ValueFunction *) value_Orbital;
			else if (!strcmp(kw3, "mod")) cpp.vf = (ValueFunction *) value_OrbDens;
			else if (!strcmp(kw3, "unity")) cpp.vf = (ValueFunction *) GetUnity;
			else { PrintToLog("ERROR : add volrend : unknown value function.\n"); return; }
			
			if (!strcmp(kw4, "red")) cpp.cf = (ColorFunction *) GetRedColor;
			else if (!strcmp(kw4, "green")) cpp.cf = (ColorFunction *) GetGreenColor;
			else if (!strcmp(kw4, "blue")) cpp.cf = (ColorFunction *) GetBlueColor;
			else if (!strcmp(kw4, "rb1")) cpp.cf = (ColorFunction *) GetRBRange1;
			else if (!strcmp(kw4, "rb2")) cpp.cf = (ColorFunction *) GetRBRange2;
			else { PrintToLog("ERROR : add volrend : unknown colour function.\n"); return; }
			
			f64 cscale1 = strtod(kw5, endptr);
			
			f64 cscale2 = 0.0; bool auto_cv2 = false;
			if (!strcmp(kw6, "AUTO")) auto_cv2 = true;
			else cscale2 = strtod(kw6, endptr);
			
			f64 dim = strtod(kw7, endptr);
			
			i32s res = strtol(kw8, endptr, 10);
			if (res < 4) res = 4;
			
			f64 alpha = strtod(kw9, endptr);
			
			cpp.dim = dim; cpp.np = res;
			cpp.transparent = true; cpp.automatic_cv2 = auto_cv2;
			
			cpp.cvalue1 = cscale1;
			cpp.cvalue2 = cscale2;
			cpp.alpha = alpha;
			
			ostrstream strN(object_name, sizeof(object_name));
			strN << kw3 << "-" << ends;
			
			AddObject(new volume_rendering_object(ol_static(), cpp, res / 2, dim / 2.0, (* gv->cam), object_name));
			UpdateAllGraphicsViews();
			
			ostrstream strR(mbuff1, sizeof(mbuff1));
			strR << "Added a new object : volrend (" << kw3 << " " << kw4 << ")." << endl << ends;
			PrintToLog(mbuff1);
			return;
		}
		
		if (!strcmp("surf1", kw2))
		{
			char kw3[32]; istr >> kw3;	// vf1
			char kw4[32]; istr >> kw4;	// vf2
			char kw5[32]; istr >> kw5;	// cf
			char kw6[32]; istr >> kw6;	// sscale
			char kw7[32]; istr >> kw7;	// cscale1
			char kw8[32]; istr >> kw8;	// AUTO/cscale2
			char kw9[32]; istr >> kw9;	// dim
			char kwA[32]; istr >> kwA;	// res
			char kwB[32]; istr >> kwB;	// solid
			char kwC[32]; istr >> kwC;	// tp
			char kwD[32]; istr >> kwD;	// alpha
			char ** endptr = NULL;
			
			cs_param csp1;

			csp1.prj = this; csp1.ref = GetCurrentSetup()->GetCurrentEngine(); csp1.next = NULL;
			
			if (!strcmp(kw3, "esp")) csp1.vf1 = (ValueFunction *) value_ESP;
			else if (!strcmp(kw3, "vdws")) csp1.vf1 = (ValueFunction *) value_VDWSurf;
			else if (!strcmp(kw3, "eldens")) csp1.vf1 = (ValueFunction *) value_ElDens;
			else if (!strcmp(kw3, "mo")) csp1.vf1 = (ValueFunction *) value_Orbital;
			else if (!strcmp(kw3, "mod")) csp1.vf1 = (ValueFunction *) value_OrbDens;
			else if (!strcmp(kw3, "unity")) csp1.vf1 = (ValueFunction *) GetUnity;
			else { PrintToLog("ERROR : add surf1 : unknown value function 1.\n"); return; }
			
			if (!strcmp(kw4, "esp")) csp1.vf2 = (ValueFunction *) value_ESP;
			else if (!strcmp(kw4, "vdws")) csp1.vf2 = (ValueFunction *) value_VDWSurf;
			else if (!strcmp(kw4, "eldens")) csp1.vf2 = (ValueFunction *) value_ElDens;
			else if (!strcmp(kw4, "mo")) csp1.vf2 = (ValueFunction *) value_Orbital;
			else if (!strcmp(kw4, "mod")) csp1.vf2 = (ValueFunction *) value_OrbDens;
			else if (!strcmp(kw4, "unity")) csp1.vf2 = (ValueFunction *) GetUnity;
			else { PrintToLog("ERROR : add surf1 : unknown value function 2.\n"); return; }
			
			if (!strcmp(kw5, "red")) csp1.cf = (ColorFunction *) GetRedColor;
			else if (!strcmp(kw5, "green")) csp1.cf = (ColorFunction *) GetGreenColor;
			else if (!strcmp(kw5, "blue")) csp1.cf = (ColorFunction *) GetBlueColor;
			else if (!strcmp(kw5, "rb1")) csp1.cf = (ColorFunction *) GetRBRange1;
			else if (!strcmp(kw5, "rb2")) csp1.cf = (ColorFunction *) GetRBRange2;
			else { PrintToLog("ERROR : add surf1 : unknown colour function.\n"); return; }
			
			f64 sscale = strtod(kw6, endptr);
			f64 cscale1 = strtod(kw7, endptr);
			
			f64 cscale2 = 0.0; bool auto_cv2 = false;
			if (!strcmp(kw8, "AUTO")) auto_cv2 = true;
			else cscale2 = strtod(kw8, endptr);
			
			f64 dim = strtod(kw9, endptr);
			
			i32s res = strtol(kwA, endptr, 10);
			if (res < 4) res = 4;
			
			i32s solid = strtol(kwB, endptr, 10);
			if (solid < 0) solid = 0; if (solid > 1) solid = 1;

			i32s tp = strtol(kwC, endptr, 10);
			if (tp < 0) tp = 0; if (tp > 1) tp = 1;
			
			f64 alpha = strtod(kwD, endptr);
			
			static fGL dim_arr[3];
			dim_arr[0] = dim_arr[1] = dim_arr[2] = dim;
			
			static i32s res_arr[3];
			res_arr[0] = res_arr[1] = res_arr[2] = res;
			
			csp1.dim = dim_arr; csp1.np = res_arr;
			csp1.transparent = tp; csp1.automatic_cv2 = auto_cv2; csp1.wireframe = !solid;
			
			csp1.svalue = sscale;
			csp1.cvalue1 = cscale1;
			csp1.cvalue2 = cscale2;
			csp1.alpha = alpha;
			
			csp1.toler = fabs(1.0e-6 * sscale); csp1.maxc = 250;
			
			ostrstream strN(object_name, sizeof(object_name));
			strN << kw3 << "-" << kw4 << "-" << ends;
			
			AddObject(new color_surface_object(ol_static(), csp1, object_name));
			UpdateAllGraphicsViews();
			
			ostrstream strR(mbuff1, sizeof(mbuff1));
			strR << "Added a new object : surf1 (" << kw3 << " " << kw4 << " " << kw5 << ")." << endl << ends;
			PrintToLog(mbuff1);
			return;
		}
		
		if (!strcmp("surf2", kw2))
		{
			char kw3[32]; istr >> kw3;	// vf1
			char kw4[32]; istr >> kw4;	// vf2
			char kw5[32]; istr >> kw5;	// cf1
			char kw6[32]; istr >> kw6;	// cf2
			char kw7[32]; istr >> kw7;	// sscale1
			char kw8[32]; istr >> kw8;	// sscale2
			char kw9[32]; istr >> kw9;	// cscale1
			char kwA[32]; istr >> kwA;	// AUTO/cscale2
			char kwB[32]; istr >> kwB;	// dim
			char kwC[32]; istr >> kwC;	// res
			char kwD[32]; istr >> kwD;	// solid
			char kwE[32]; istr >> kwE;	// tp
			char kwF[32]; istr >> kwF;	// alpha
			char ** endptr = NULL;
			
			cs_param csp2a; cs_param csp2b;
			
			csp2a.prj = this; csp2a.ref = GetCurrentSetup()->GetCurrentEngine(); csp2a.next = & csp2b;

			csp2b.prj = this; csp2b.ref = GetCurrentSetup()->GetCurrentEngine(); csp2b.next = NULL;
			
			if (!strcmp(kw3, "esp")) csp2a.vf1 = csp2b.vf1 = (ValueFunction *) value_ESP;
			else if (!strcmp(kw3, "vdws")) csp2a.vf1 = csp2b.vf1 = (ValueFunction *) value_VDWSurf;
			else if (!strcmp(kw3, "eldens")) csp2a.vf1 = csp2b.vf1 = (ValueFunction *) value_ElDens;
			else if (!strcmp(kw3, "mo")) csp2a.vf1 = csp2b.vf1 = (ValueFunction *) value_Orbital;
			else if (!strcmp(kw3, "mod")) csp2a.vf1 = csp2b.vf1 = (ValueFunction *) value_OrbDens;
			else if (!strcmp(kw3, "unity")) csp2a.vf1 = csp2b.vf1 = (ValueFunction *) GetUnity;
			else { PrintToLog("ERROR : add surf2 : unknown value function 1.\n"); return; }
			
			if (!strcmp(kw4, "esp")) csp2a.vf2 = csp2b.vf2 = (ValueFunction *) value_ESP;
			else if (!strcmp(kw4, "vdws")) csp2a.vf2 = csp2b.vf2 = (ValueFunction *) value_VDWSurf;
			else if (!strcmp(kw4, "eldens")) csp2a.vf2 = csp2b.vf2 = (ValueFunction *) value_ElDens;
			else if (!strcmp(kw4, "mo")) csp2a.vf2 = csp2b.vf2 = (ValueFunction *) value_Orbital;
			else if (!strcmp(kw4, "mod")) csp2a.vf2 = csp2b.vf2 = (ValueFunction *) value_OrbDens;
			else if (!strcmp(kw4, "unity")) csp2a.vf2 = csp2b.vf2 = (ValueFunction *) GetUnity;
			else { PrintToLog("ERROR : add surf2 : unknown value function 2.\n"); return; }
			
			if (!strcmp(kw5, "red")) csp2a.cf = (ColorFunction *) GetRedColor;
			else if (!strcmp(kw5, "green")) csp2a.cf = (ColorFunction *) GetGreenColor;
			else if (!strcmp(kw5, "blue")) csp2a.cf = (ColorFunction *) GetBlueColor;
			else if (!strcmp(kw5, "rb1")) csp2a.cf = (ColorFunction *) GetRBRange1;
			else if (!strcmp(kw5, "rb2")) csp2a.cf = (ColorFunction *) GetRBRange2;
			else { PrintToLog("ERROR : add surf2 : unknown colour function 1.\n"); return; }
			
			if (!strcmp(kw6, "red")) csp2b.cf = (ColorFunction *) GetRedColor;
			else if (!strcmp(kw6, "green")) csp2b.cf = (ColorFunction *) GetGreenColor;
			else if (!strcmp(kw6, "blue")) csp2b.cf = (ColorFunction *) GetBlueColor;
			else if (!strcmp(kw6, "rb1")) csp2b.cf = (ColorFunction *) GetRBRange1;
			else if (!strcmp(kw6, "rb2")) csp2b.cf = (ColorFunction *) GetRBRange2;
			else { PrintToLog("ERROR : add surf2 : unknown colour function 2.\n"); return; }
			
			f64 sscale1 = strtod(kw7, endptr);
			f64 sscale2 = strtod(kw8, endptr);
			f64 cscale1 = strtod(kw9, endptr);
			
			f64 cscale2 = 0.0; bool auto_cv2 = false;
			if (!strcmp(kwA, "AUTO")) auto_cv2 = true;
			else cscale2 = strtod(kwA, endptr);
			
			f64 dim = strtod(kwB, endptr);
			
			i32s res = strtol(kwC, endptr, 10);
			if (res < 4) res = 4;
			
			i32s solid = strtol(kwD, endptr, 10);
			if (solid < 0) solid = 0; if (solid > 1) solid = 1;

			i32s tp = strtol(kwE, endptr, 10);
			if (tp < 0) tp = 0; if (tp > 1) tp = 1;
			
			f64 alpha = strtod(kwF, endptr);
			
			static fGL dim_arr[3];
			dim_arr[0] = dim_arr[1] = dim_arr[2] = dim;
			
			static i32s res_arr[3];
			res_arr[0] = res_arr[1] = res_arr[2] = res;
			
			csp2a.dim = dim_arr; csp2a.np = res_arr;
			csp2a.transparent = tp; csp2a.automatic_cv2 = auto_cv2; csp2a.wireframe = !solid;
			
			csp2a.svalue = sscale1;
			csp2a.cvalue1 = cscale1;
			csp2a.cvalue2 = cscale2;
			csp2a.alpha = alpha;
			
			csp2a.toler = fabs(1.0e-6 * sscale1); csp2a.maxc = 250;
			
			csp2b.dim = dim_arr; csp2b.np = res_arr;
			csp2b.transparent = tp; csp2b.automatic_cv2 = auto_cv2; csp2b.wireframe = !solid;
			
			csp2b.svalue = sscale2;
			csp2b.cvalue1 = cscale1;
			csp2b.cvalue2 = cscale2;
			csp2b.alpha = alpha;
			
			csp2b.toler = fabs(1.0e-6 * sscale2); csp2b.maxc = 250;
			
			ostrstream strN(object_name, sizeof(object_name));
			strN << kw3 << "-" << kw4 << "-" << ends;
			
			AddObject(new color_surface_object(ol_static(), csp2a, object_name));
			UpdateAllGraphicsViews();
			
			ostrstream strR(mbuff1, sizeof(mbuff1));
			strR << "Added a new object : surf2 (" << kw3 << " " << kw4 << " " << kw5 << " " << kw6 << ")." << endl << ends;
			PrintToLog(mbuff1);
			return;
		}
		
		PrintToLog("ERROR : could not process a command : \"add\".\n");
		return;
	}
	
	if (!strcmp("energy", kw1))
	{
		DoEnergy();
		return;
	}
	
	if (!strcmp("test", kw1))
	{
		//DoEnergy();

		//void model::DoEnergy(void)
		//{
		#if USE_ORIGINAL_ENGINE_SELECTION
			//#####################################################################
			// original code 
			//#####################################################################
			engine * eng = GetCurrentSetup()->GetCurrentEngine();
			if (eng == NULL) GetCurrentSetup()->CreateCurrentEngine();
			eng = GetCurrentSetup()->GetCurrentEngine();
		#else
			//test code
			engine * eng = GetCurrentSetup()->CreateEngineByIDNumber(CURRENT_ENG1_MM);
		#endif
			//#####################################################################
			if (eng == NULL) return;
			
			char mbuff1[256];
			ostrstream str1(mbuff1, sizeof(mbuff1));
			str1 << "Calculating Energy ";
			str1 << "(setup = " << GetCurrentSetup()->GetClassName();
			str1 << ", engine = " << GetCurrentSetup()->GetEngineName(GetCurrentSetup()->GetCurrEngIndex());
			str1 << ")." << endl << ends;
			PrintToLog(mbuff1);
			
			CopyCRD(this, eng, 0);
			eng->Compute(0);
						


			if (dynamic_cast<eng1_sf *>(eng) != NULL) CopyCRD(eng, this, 0);	// for ribbons...
			
			char buffer[128];
			ostrstream str(buffer, sizeof(buffer)); str.setf(ios::fixed); str.precision(8);
			
			str << "Energy = " << eng->energy << " kJ/mol" << endl << ends;
			PrintToLog(buffer);
			
		// we will not delete current_eng here, so that we can draw plots using it...
		// we will not delete current_eng here, so that we can draw plots using it...
		// we will not delete current_eng here, so that we can draw plots using it...
			
			SetupPlotting();


			//#############################################
			eng1_mm_default_nbt_mim * eng_mm = dynamic_cast<eng1_mm_default_nbt_mim *>(eng);
			if (eng_mm)
			{
				atom ** atmtab = eng_mm->GetSetup()->GetMMAtoms();
							
			while (!bond_list.empty())
			{
				iter_bl it1 = bond_list.begin();
				RemoveBond(it1);
			}
				// the nonbonded terms begin...
				// the nonbonded terms begin...
				// the nonbonded terms begin...
				
				for (i32s n1 = 0;n1 < (i32s) eng_mm->nbt1_vector.size();n1++)
				{
					i32s * atmi = eng_mm->nbt1_vector[n1].atmi;
					if (atmtab[atmi[0]] != atmtab[atmi[1]])
					{
						bond newbond( atmtab[atmi[0]],  atmtab[atmi[1]], bondtype('S'));
						AddBond(newbond);
					}
				}
			}
			//#############################################

		//}


	

		return;
	}
	
	if (!strcmp("geom_opt", kw1))				// todo: how to set the options here?
	{
		setup * su = GetCurrentSetup();
		static jobinfo_GeomOpt ji;
		
		ji.prj = this;
		ji.go = geomopt_param(su);
		ji.go.show_dialog = false;
		
		start_job_GeomOpt(& ji);
		return;
	}
	
	if (!strcmp("mol_dyn", kw1))				// todo: how to set the options here?
	{
		setup * su = GetCurrentSetup();
		static jobinfo_MolDyn ji;
		
		ji.prj = this;
		ji.md = moldyn_param(su);
		ji.md.show_dialog = false;
		
		start_job_MolDyn(& ji);
		return;
	}
	
	if (!strcmp("random_search", kw1))
	{
		char kw2[32]; istr >> kw2;	// the 2nd keyword; cycles.
		char kw3[32]; istr >> kw3;	// the 3rd keyword; optsteps.
		
		static jobinfo_RandomSearch ji;
		char ** endptr = NULL;
		
		ji.prj = this;
		ji.cycles = strtol(kw2, endptr, 10);
		ji.optsteps = strtol(kw3, endptr, 10);
		
#ifdef ENABLE_THREADS
		
		CreateProgressDialog("Random Search", true, NOT_DEFINED, NOT_DEFINED);
		GThread * t = g_thread_create(pcs_job_RandomSearch, (gpointer) & ji, FALSE, NULL);
		if (t == NULL) { DestroyProgressDialog(); ErrorMessage("Thread failed!"); }
		
#else	// ENABLE_THREADS
		
//		pcs_job_RandomSearch((gpointer) & ji);
		pcs_job_RandomSearch((void*) & ji);
		
#endif	// ENABLE_THREADS
		
		return;
	}

	if (!strcmp("systematic_search", kw1))
	{
		char kw2[32]; istr >> kw2;	// the 2nd keyword; divisions.
		char kw3[32]; istr >> kw3;	// the 3rd keyword; optsteps.
		
		char ** endptr = NULL;
		i32s divisions = strtol(kw2, endptr, 10);
		i32s optsteps = strtol(kw3, endptr, 10);
		
		DoSystematicSearch(divisions, optsteps, true);
		return;
	}

	if (!strcmp("montecarlo_search", kw1))
	{
		char kw2[32]; istr >> kw2;	// the 2nd keyword; n_init_steps.
		char kw3[32]; istr >> kw3;	// the 3rd keyword; n_simul_steps.
		char kw4[32]; istr >> kw4;	// the 4th keyword; optsteps.
		
		char ** endptr = NULL;
		i32s n_init_steps = strtol(kw2, endptr, 10);
		i32s n_simul_steps = strtol(kw3, endptr, 10);
		i32s optsteps = strtol(kw4, endptr, 10);
		
		DoMonteCarloSearch(n_init_steps, n_simul_steps, optsteps, true);
		return;
	}

	
	if (!strcmp("make_plot_ang", kw1))
	{
		char kw2[32]; istr >> kw2;	// A
		char kw3[32]; istr >> kw3;	// B
		char kw4[32]; istr >> kw4;	// C

		char ** endptr = NULL;
		
		i32s ia = strtol(kw2, endptr, 10);
		i32s ib = strtol(kw3, endptr, 10);
		i32s ic = strtol(kw4, endptr, 10);
		
		TrajView_AnglePlot(ia, ib, ic);
		return;
	}


	if (!strcmp("make_plot_dist", kw1))
	{
		char kw2[32]; istr >> kw2;	// A
		char kw3[32]; istr >> kw3;	// B

		char ** endptr = NULL;
		
		i32s ia = strtol(kw2, endptr, 10);
		i32s ib = strtol(kw3, endptr, 10);
		
		TrajView_DistancePlot(ia, ib);
		return;
	}

	if (!strcmp("vel_dist_2d", kw1))
	{
		char kw2[32]; istr >> kw2;	// divx
		char kw3[32]; istr >> kw3;	// divy
		char kw4[32]; istr >> kw4;	// dt

		char ** endptr = NULL;
		
		i32s divx = strtol(kw2, endptr, 10);
		i32s divy = strtol(kw3, endptr, 10);
		f64 dt = strtod(kw4, endptr);
		
		TrajView_VeloncityDistribution2D(divx, divy, dt);
		return;
	}

	if (!strcmp("make_plot_crd_diff", kw1))
	{
		char kw2[32]; istr >> kw2;	// ind1
		char kw3[32]; istr >> kw3;	// ind2
		char kw4[32]; istr >> kw4;	// dim

		char ** endptr = NULL;
		
		i32s ind1 = strtol(kw2, endptr, 10);
		i32s ind2 = strtol(kw3, endptr, 10);
		i32s dim = strtol(kw4, endptr, 10);
		
		TrajView_CoordinateDifferencePlot(ind1, ind2, dim);
		return;
	}

	if (!strcmp("make_plot_crd", kw1))
	{
		char kw2[32]; istr >> kw2;	// A
		char kw3[32]; istr >> kw3;	// B

		char ** endptr = NULL;
		
		i32s ia = strtol(kw2, endptr, 10);
		i32s ib = strtol(kw3, endptr, 10);
		
		TrajView_CoordinatePlot(ia, ib);
		return;
	}

	if (!strcmp("make_nematic_plot", kw1))
	{
		char kw2[32]; istr >> kw2;	// A
		char kw3[32]; istr >> kw3;	// B

		char ** endptr = NULL;
		
		i32s ia = strtol(kw2, endptr, 10);
		i32s ib = strtol(kw3, endptr, 10);
		
		TrajView_NematicCoordinatePlot(ia, ib);
		return;
	}

	
	if (!strcmp("sel_atoms_with_flag", kw1))
	{
		char kw2[32]; istr >> kw2;	
		char ** endptr = NULL;		
		i32u flag_num = strtoul(kw2, endptr, 10);		
		SelectAtomsWithTheFlag(flag_num);
		return;
	}

	if (!strcmp("set_flag_on_sel_atoms", kw1))
	{
		char kw2[32]; istr >> kw2;	
		char ** endptr = NULL;		
		i32u flag_num = strtoul(kw2, endptr, 10);		
		SetTheFlagOnSelectedAtoms(flag_num);
		return;
	}

	if (!strcmp("unset_flag_on_sel_atoms", kw1))
	{
		char kw2[32]; istr >> kw2;	
		char ** endptr = NULL;		
		i32u flag_num = strtoul(kw2, endptr, 10);		
		UnSetTheFlagOnSelectedAtoms(flag_num);
		return;
	}

	if (!strcmp("move_sel_atoms", kw1))
	{
		char kw2[32]; istr >> kw2;//dx
		char kw3[32]; istr >> kw3;//dy
		char kw4[32]; istr >> kw4;//dz
		char ** endptr = NULL;
		f64 dx = strtod(kw2, endptr);
		f64 dy = strtod(kw3, endptr);
		f64 dz = strtod(kw4, endptr);
		const f64 shift[3] = {dx, dy, dz};
		MoveSelectedAtoms(shift);
		return;
	}
	
	if (!strcmp("traj_set_total_frames", kw1))
	{
		char kw2[32]; istr >> kw2;	// A
		char ** endptr = NULL;
		

		if ( strlen(kw2) > 0)
		{
			i32s ia = strtol(kw2, endptr, 10);
			char filename[512];
			DWORD nFilterIndex;
			if (OpenFileDlg(NULL, "Ghemical Trajectory File (*.traj)\0*.traj\0All files \0*.*\0", filename, nFilterIndex) == S_OK)
			{	
				this->TrajectorySetTotalFrames(filename, ia);
			}
		}
		return;
	}


	
	if (!strcmp("make_plot_a", kw1))
	{
		char kw2[32]; istr >> kw2;	// A
		char kw3[32]; istr >> kw3;	// B
		char kw4[32]; istr >> kw4;	// C
		//char kw5[32]; istr >> kw5;	// D
		char kw6[32]; istr >> kw6;	// div
		char kw7[32]; istr >> kw7;	// start_ang
		char kw8[32]; istr >> kw8;	// end_ang
		char kw9[32]; istr >> kw9;	// optsteps
		char kwA[256]; istr >> kwA;	// filename
		char ** endptr = NULL;
		
		i32s ia = strtol(kw2, endptr, 10);
		i32s ib = strtol(kw3, endptr, 10);
		i32s ic = strtol(kw4, endptr, 10);
		//i32s id = strtol(kw5, endptr, 10);
		i32s div1 = strtol(kw6, endptr, 10);
		fGL start1 = strtod(kw7, endptr);
		fGL end1 = strtod(kw8, endptr);
		
		i32s optsteps = strtol(kw9, endptr, 10);
		
		DoEnergyPlot1D(ia, ib, ic, div1, start1, end1, optsteps, kwA);
		return;
	}
	if (!strcmp("make_plot1", kw1))
	{
		char kw2[32]; istr >> kw2;	// A
		char kw3[32]; istr >> kw3;	// B
		char kw4[32]; istr >> kw4;	// C
		char kw5[32]; istr >> kw5;	// D
		char kw6[32]; istr >> kw6;	// div
		char kw7[32]; istr >> kw7;	// start_ang
		char kw8[32]; istr >> kw8;	// end_ang
		char kw9[32]; istr >> kw9;	// optsteps
		char ** endptr = NULL;
		
		i32s ia = strtol(kw2, endptr, 10);
		i32s ib = strtol(kw3, endptr, 10);
		i32s ic = strtol(kw4, endptr, 10);
		i32s id = strtol(kw5, endptr, 10);
		i32s div1 = strtol(kw6, endptr, 10);
		fGL start1 = strtod(kw7, endptr);
		fGL end1 = strtod(kw8, endptr);
		
		i32s optsteps = strtol(kw9, endptr, 10);
		
		DoEnergyPlot1D(ia, ib, ic, id, div1, start1, end1, optsteps);
		return;
	}
	if (!strcmp("make_plot2", kw1))
	{
		char kw2[32]; istr >> kw2;	// A
		char kw3[32]; istr >> kw3;	// B
		char kw4[32]; istr >> kw4;	// C
		char kw5[32]; istr >> kw5;	// D
		char kw6[32]; istr >> kw6;	// div
		char kw7[32]; istr >> kw7;	// start_ang
		char kw8[32]; istr >> kw8;	// end_ang
		char kw9[32]; istr >> kw9;	// I
		char kwA[32]; istr >> kwA;	// J
		char kwB[32]; istr >> kwB;	// K
		char kwC[32]; istr >> kwC;	// L
		char kwD[32]; istr >> kwD;	// div
		char kwE[32]; istr >> kwE;	// start_ang
		char kwF[32]; istr >> kwF;	// end_ang
		char kwG[32]; istr >> kwG;	// optsteps
		char ** endptr = NULL;
		
		i32s ia = strtol(kw2, endptr, 10);
		i32s ib = strtol(kw3, endptr, 10);
		i32s ic = strtol(kw4, endptr, 10);
		i32s id = strtol(kw5, endptr, 10);
		i32s div1 = strtol(kw6, endptr, 10);
		fGL start1 = strtod(kw7, endptr);
		fGL end1 = strtod(kw8, endptr);
		
		i32s ii = strtol(kw9, endptr, 10);
		i32s ij = strtol(kwA, endptr, 10);
		i32s ik = strtol(kwB, endptr, 10);
		i32s il = strtol(kwC, endptr, 10);
		i32s div2 = strtol(kwD, endptr, 10);
		fGL start2 = strtod(kwE, endptr);
		fGL end2 = strtod(kwF, endptr);
		
		i32s optsteps = strtol(kwG, endptr, 10);
		
		DoEnergyPlot2D(ia, ib, ic, id, div1, start1, end1, ii, ij, ik, il, div2, start2, end2, optsteps);
		return;
	}
	
	if (!strcmp("population_analysis_ESP", kw1))
	{
		setup1_qm * suqm = dynamic_cast<setup1_qm *>(current_setup);
		if (suqm == NULL) Message("Sorry, this is for QM models only!");
		else
		{
			pop_ana_electrostatic pa(suqm);
			pa.DoPopAna();
			
			// how to set the charge labels on in graphics?
		}
		
		return;
	}
	
	if (!strcmp("transition_state_search", kw1))
	{
		char kw2[32]; istr >> kw2;	// the 2nd keyword; delta-E per step.
		char kw3[32]; istr >> kw3;	// the 3rd keyword; initial force constant.
		char ** endptr = NULL;
		
		fGL deltae = strtod(kw2, endptr);
		fGL initfc = strtod(kw3, endptr);
		
		DoTransitionStateSearch(deltae, initfc);
		return;
	}
	if (!strcmp("stationary_state_search", kw1))
	{
		char kw2[32]; istr >> kw2;	// the 2nd keyword; steps.
		char ** endptr = NULL;
		
		i32s steps = strtol(kw2, endptr, 10);
		
		DoStationaryStateSearch(steps);
		return;
	}
	
	if (!strcmp("set_current_orbital", kw1))
	{
		char kw2[32]; istr >> kw2;	// the 2nd keyword; the orbital index.
		
		char ** endptr = NULL;
		int index = strtol(kw2, endptr, 10);
		if (index < 0) index = 0;
		
		qm_current_orbital = index;
		
		ostrstream strR(mbuff1, sizeof(mbuff1));
		strR << "The current orbital is now " << qm_current_orbital << "." << endl << ends;
		PrintToLog(mbuff1);
		return;
	}
	
	if (!strcmp("build_amino", kw1))
	{
		char kw2[4096]; istr >> kw2;	// sequence
		char kw3[32]; istr >> kw3;	// helix/sheet (optional)
		
		f64 aab_ah[] = { 302.0 * M_PI / 180.0, 313.0 * M_PI / 180.0, M_PI };
		f64 aab_bs[] = { 180.0 * M_PI / 180.0, 180.0 * M_PI / 180.0, M_PI };
		
		model::amino_builder.Build(this, kw2, kw3[0] == 'h' || kw3[0] == 'H' ? aab_ah : aab_bs);
		UpdateAllGraphicsViews();
		
		ostrstream strR(mbuff1, sizeof(mbuff1));
		strR << "built a sequence : " << kw2 << endl << ends;
		PrintToLog(mbuff1);
		return;
	}
	if (!strcmp("build_nucleic", kw1))
	{
		char kw2[4096]; istr >> kw2;	// sequence
		
		f64 nab[] =
		{
			44.6 * M_PI / 180.0, 261.0 * M_PI / 180.0, 320.8 * M_PI / 180.0, 208.6 * M_PI / 180.0,
			273.8 * M_PI / 180.0, 105.6 * M_PI / 180.0, 356.0 * M_PI / 180.0, 24.7 * M_PI / 180.0,
			88.7 * M_PI / 180.0, 264.6 * M_PI / 180.0
		};
		
		model::nucleic_builder.Build(this, kw2, nab);
		UpdateAllGraphicsViews();
		
		ostrstream strR(mbuff1, sizeof(mbuff1));
		strR << "built a sequence : " << kw2 << endl << ends;
		PrintToLog(mbuff1);
		return;
	}

	
	if (!strcmp("box", kw1))
	{
		char kw2[32]; istr >> kw2;		// xdim
		char kw3[32]; istr >> kw3;		// ydim
		char kw4[32]; istr >> kw4;		// zdim
		if ( strlen(kw2) > 0 && strlen(kw3) > 0 && strlen(kw4) > 0)
		{		
			char ** endptr = NULL;
			fGL xdim = strtod(kw2, endptr);
			fGL ydim = strtod(kw3, endptr);
			fGL zdim = strtod(kw4, endptr);		
			
			use_periodic_boundary_conditions = true;	// DO NOT SET THIS!!! this is broken apparently...
			periodic_box_HALFdim[0] = xdim;
			periodic_box_HALFdim[1] = ydim;
			periodic_box_HALFdim[2] = zdim;

			UpdateAllGraphicsViews();
		}
		if ( strlen(kw2) > 0 && strlen(kw3) == 0 && strlen(kw4) == 0)
		{	
			use_periodic_boundary_conditions = atoi(kw2);
			UpdateAllGraphicsViews();
		}
		printf("use_periodic_boundary_conditions = %d\n", use_periodic_boundary_conditions);
		printf("periodic_box_HALFdim[0] = %f\n", periodic_box_HALFdim[0]);
		printf("periodic_box_HALFdim[1] = %f\n", periodic_box_HALFdim[1]);
		printf("periodic_box_HALFdim[2] = %f\n", periodic_box_HALFdim[2]);
		return;
	}

	if (!strcmp("boxdim", kw1))
	{
		char kw2[32]; istr >> kw2;		// dim
		char kw3[32]; istr >> kw3;		// val
		if ( strlen(kw2) > 0 && strlen(kw3) > 0)
		{		
			char ** endptr = NULL;
			int dim = atoi(kw2);
			fGL val = strtod(kw3, endptr);
			
			periodic_box_HALFdim[dim] = val;

			UpdateAllGraphicsViews();
		}

		printf("use_periodic_boundary_conditions = %d\n", use_periodic_boundary_conditions);
		printf("periodic_box_HALFdim[0] = %f\n", periodic_box_HALFdim[0]);
		printf("periodic_box_HALFdim[1] = %f\n", periodic_box_HALFdim[1]);
		printf("periodic_box_HALFdim[2] = %f\n", periodic_box_HALFdim[2]);
		return;
	}	
	
	if (!strcmp("nematic_box", kw1))
	{
		char kw2[32]; istr >> kw2;		// xdim
		char kw3[32]; istr >> kw3;		// ydim
		char kw4[32]; istr >> kw4;		// zdim
		char kw6[256] = ""; istr >> kw6;	// filename (optional)
		
		char ** endptr = NULL;
		fGL xdim = strtod(kw2, endptr);
		fGL ydim = strtod(kw3, endptr);
		fGL zdim = strtod(kw4, endptr);
		
		//fGL density = 1.00; if (strlen(kw5) > 0) density = strtod(kw5, endptr);
		

		dummy_project * solvent = NULL;
		if (strlen(kw6) > 0)
		{			
			char fn[256];
			ostrstream fns(fn, sizeof(fn));
			fns << kw6 << ".gpr" << ends;

			DWORD nFilterIndex;
			if (OpenFileDlg(0, "Ghemical Project File (*.gpr)\0*.gpr\0All files \0*.*\0", 
				fn, nFilterIndex) 
				== S_OK)
			{
				solvent = new dummy_project(NULL);
				ifstream ifile(fn, ios::in);
				ReadGPR(* solvent, ifile, false);
				ifile.close();
			}			
		}
		
		NematicBox(xdim, ydim, zdim, solvent);
		UpdateAllGraphicsViews();
		return;
	}

	if (!strcmp("solvate_box", kw1))
	{
		char kw2[32]; istr >> kw2;		// xdim
		char kw3[32]; istr >> kw3;		// ydim
		char kw4[32]; istr >> kw4;		// zdim
		char kw5[32] = ""; istr >> kw5;		// density (optional)
		char kw5_[32] = ""; istr >> kw5_;	// atom number (optional)
		char kw6[256] = ""; istr >> kw6;	// filename (optional)
		char kw7[64] = ""; istr >> kw7;		// export (optional)
		
		char ** endptr = NULL;
		fGL xdim = strtod(kw2, endptr);
		fGL ydim = strtod(kw3, endptr);
		fGL zdim = strtod(kw4, endptr);
		
		fGL density = 1.00; if (strlen(kw5) > 0) density = strtod(kw5, endptr);
		
		char * export_fn = NULL;		
		if (!strcmp(kw7, "export")) export_fn = kw6;

		int element_number = 2; if (strlen(kw5_) > 0) element_number = atoi(kw5_);

		dummy_project * solvent = NULL;
		if (strlen(kw6) > 0)
		{
			
			char fn[256];
			ostrstream fns(fn, sizeof(fn));
			fns << kw6 << ".gpr" << ends;

			DWORD nFilterIndex;
			if (OpenFileDlg(0, "Ghemical Project File (*.gpr)\0*.gpr\0All files \0*.*\0", 
				fn, nFilterIndex) 
				== S_OK)
			{
				solvent = new dummy_project(NULL);
				ifstream ifile(fn, ios::in);
				ReadGPR(* solvent, ifile, false);
				ifile.close();

				element_number = 0;
			}
			
		}
		
		SolvateBox(xdim, ydim, zdim, density, element_number, solvent, export_fn);
		UpdateAllGraphicsViews();
		return;
	}
	if (!strcmp("solvate_sphere", kw1))
	{
		char kw2[32]; istr >> kw2;		// rad_solute
		char kw3[32]; istr >> kw3;		// rad_solvent
		char kw4[32] = ""; istr >> kw4;		// density (optional)
		char kw5[256] = ""; istr >> kw5;	// filename (optional)
		
		char ** endptr = NULL;
		fGL rad1 = strtod(kw2, endptr);
		fGL rad2 = strtod(kw3, endptr);
		
		fGL density = 1.00;	// in kg/liter as usual...
		if (strlen(kw4) > 0) density = strtod(kw4, endptr);
		
		dummy_project * solvent = NULL;
		if (strlen(kw5) > 0)
		{
			char fn[256];
			ostrstream fns(fn, sizeof(fn));
			fns << kw5 << ".gpr" << ends;

			DWORD nFilterIndex;
			if (OpenFileDlg(0, "Ghemical Project File (*.gpr)\0*.gpr\0All files \0*.*\0", 
				fn, nFilterIndex) 
				== S_OK)
			{
				solvent = new dummy_project(NULL);
				ifstream ifile(fn, ios::in);
				ReadGPR(* solvent, ifile, false);
				ifile.close();
			}
		}
		
		SolvateSphere(rad1, rad2, density, solvent);
		UpdateAllGraphicsViews();
		return;
	}
	
	if (!strcmp("set_formal_charge", kw1))
	{
		char kw2[32]; istr >> kw2;	// the 2nd keyword; index.
		char kw3[32]; istr >> kw3;	// the 3rd keyword; charge.
		char ** endptr = NULL;
		
		i32s index = strtol(kw2, endptr, 10);
		i32s charge = strtol(kw3, endptr, 10);
		
		if (!IsIndexClean()) UpdateIndex();
		
		atom * atmr = NULL;
		for (iter_al it1 = GetAtomsBegin();it1 != GetAtomsEnd();it1++)
		{
			if ((* it1).index == index) { atmr = & (* it1); break; }
		}
		
		if (atmr != NULL)
		{
			atmr->formal_charge = charge;
			UpdateAllGraphicsViews();	// update the labels...
		}
		else
		{
			PrintToLog("Sorry, atom not found!\n");
		}
		return;
	}
	
	if (!strcmp("evaluate_Bfact", kw1))
	{
		EvaluateBFact();
		return;
	}
	if (!strcmp("evaluate_diffconst", kw1))
	{
		char kw2[32]; istr >> kw2;	// the 2nd keyword; dt.
		
		char ** endptr = NULL;
		fGL dt = strtod(kw2, endptr);
		
		EvaluateDiffConst(dt);
		return;
	}
	
	// if the command is not recognized above, we will print out an error message here.
	
	ostrstream strE(mbuff1, sizeof(mbuff1)); strE << "ERROR : Unknown command : " << command << endl << ends;
	PrintToLog(mbuff1); PrintToLog("The \"help\" command will give more information about command strings.\n");
}
#if PROBNIY_ATOM_GEOMOPT
HANDLE  project::start_job_work_prob_atom_GeomOpt(jobinfo_work_prob_atom_GeomOpt * ji)
{
#ifdef ENABLE_THREADS
	
	CreateProgressDialog("Geometry Optimization", true, 1, 20);
	GThread * t = g_thread_create(pcs_job_GeomOpt, (gpointer) ji, FALSE, NULL);
	if (t == NULL) { DestroyProgressDialog(); ErrorMessage("Thread failed!"); }
	
#else	// ENABLE_THREADS

	DWORD dwChildID;
	HANDLE hThread;
	hThread = ::CreateThread(NULL, 0, 
		pcs_job_work_prob_atom_GeomOpt, 
		reinterpret_cast<LPVOID>(ji), 
		0,
		&dwChildID );
	
//	pcs_job_GeomOpt((gpointer) ji);
//	pcs_job_GeomOpt((void*) ji);

	return hThread;
	
#endif	// ENABLE_THREADS
}

//gpointer project::pcs_job_GeomOpt(gpointer p)
DWORD WINAPI project::pcs_job_work_prob_atom_GeomOpt(void* p)
{
	jobinfo_work_prob_atom_GeomOpt * ji = (jobinfo_work_prob_atom_GeomOpt *) p;
	
#ifdef ENABLE_THREADS
	const bool updt = false;
#else	// ENABLE_THREADS
	const bool updt = true;
#endif	// ENABLE_THREADS
	
	ji->prj->work_prob_atom_GeomOpt(*ji->go, ji->infile_name, ji->trgtlst_name, ji->box_name, ji->fixed_name, ji->total_frames);
	
#ifdef ENABLE_THREADS
	ji->prj->ThreadLock();
	ji->prj->DestroyProgressDialog();
	ji->prj->ThreadUnlock();
#endif	// ENABLE_THREADS
	
	return NULL;
}



#endif
HANDLE  project::start_job_GeomOpt(jobinfo_GeomOpt * ji)
{
#ifdef ENABLE_THREADS
	
	CreateProgressDialog("Geometry Optimization", true, 1, 20);
	GThread * t = g_thread_create(pcs_job_GeomOpt, (gpointer) ji, FALSE, NULL);
	if (t == NULL) { DestroyProgressDialog(); ErrorMessage("Thread failed!"); }
	
#else	// ENABLE_THREADS

	DWORD dwChildID;
	HANDLE hThread;
	hThread = ::CreateThread(NULL, 0, 
		pcs_job_GeomOpt, 
		reinterpret_cast<LPVOID>(ji), 
		0,
		&dwChildID );
	
//	pcs_job_GeomOpt((gpointer) ji);
//	pcs_job_GeomOpt((void*) ji);

	return hThread;
	
#endif	// ENABLE_THREADS
}

//gpointer project::pcs_job_GeomOpt(gpointer p)
DWORD WINAPI project::pcs_job_GeomOpt(void* p)
{
	jobinfo_GeomOpt * ji = (jobinfo_GeomOpt *) p;
	
#ifdef ENABLE_THREADS
	const bool updt = false;
#else	// ENABLE_THREADS
	const bool updt = true;
#endif	// ENABLE_THREADS
	
	ji->prj->DoGeomOpt(ji->go, updt);
	
#ifdef ENABLE_THREADS
	ji->prj->ThreadLock();
	ji->prj->DestroyProgressDialog();
	ji->prj->ThreadUnlock();
#endif	// ENABLE_THREADS
	
	return NULL;
}

HANDLE project::start_job_MolDyn(jobinfo_MolDyn * ji)
{
#ifdef ENABLE_THREADS
	
	CreateProgressDialog("Molecular Dynamics", true, NOT_DEFINED, NOT_DEFINED);
	GThread * t = g_thread_create(pcs_job_MolDyn, (gpointer) ji, FALSE, NULL);
	if (t == NULL) { DestroyProgressDialog(); ErrorMessage("Thread failed!"); }
	
#else	// ENABLE_THREADS
	
//	pcs_job_MolDyn((gpointer) ji);
//	pcs_job_MolDyn((void*) ji);
	DWORD dwChildID;
	HANDLE hThread;
	hThread = ::CreateThread(NULL, 0, 
		pcs_job_MolDyn, 
		reinterpret_cast<LPVOID>(ji), 
		0,
		&dwChildID );

	return hThread;
	
#endif	// ENABLE_THREADS
}

HANDLE project::start_job_MolDyn_tst(jobinfo_MolDyn_tst * ji)
{
#ifdef ENABLE_THREADS
	
	CreateProgressDialog("Molecular Dynamics", true, NOT_DEFINED, NOT_DEFINED);
	GThread * t = g_thread_create(pcs_job_MolDyn, (gpointer) ji, FALSE, NULL);
	if (t == NULL) { DestroyProgressDialog(); ErrorMessage("Thread failed!"); }
	
#else	// ENABLE_THREADS
	
//	pcs_job_MolDyn((gpointer) ji);
//	pcs_job_MolDyn((void*) ji);
	DWORD dwChildID;
	HANDLE hThread;
	hThread = ::CreateThread(NULL, 0, 
		pcs_job_MolDyn_tst, 
		reinterpret_cast<LPVOID>(ji), 
		0,
		&dwChildID );

	return hThread;
	
#endif	// ENABLE_THREADS
}

//gpointer project::pcs_job_MolDyn(gpointer p)
DWORD WINAPI project::pcs_job_MolDyn(void* p)
{
	jobinfo_MolDyn * ji = (jobinfo_MolDyn *) p;
	
#ifdef ENABLE_THREADS
	const bool updt = false;
#else	// ENABLE_THREADS
	const bool updt = true;
#endif	// ENABLE_THREADS
	
	ji->prj->DoMolDyn(ji->md, updt);
	
#ifdef ENABLE_THREADS
	ji->prj->ThreadLock();
	ji->prj->DestroyProgressDialog();
	ji->prj->ThreadUnlock();
#endif	// ENABLE_THREADS
	
	return NULL;
}

DWORD WINAPI project::pcs_job_MolDyn_tst(void* p)
{
	jobinfo_MolDyn_tst * ji = (jobinfo_MolDyn_tst *) p;
	
#ifdef ENABLE_THREADS
	const bool updt = false;
#else	// ENABLE_THREADS
	const bool updt = true;
#endif	// ENABLE_THREADS
	
	ji->prj->DoMolDyn_tst(ji->md, updt);
	
#ifdef ENABLE_THREADS
	ji->prj->ThreadLock();
	ji->prj->DestroyProgressDialog();
	ji->prj->ThreadUnlock();
#endif	// ENABLE_THREADS
	
	return NULL;
}

//gpointer project::pcs_job_RandomSearch(gpointer p)
void* project::pcs_job_RandomSearch(void* p)
{
	jobinfo_RandomSearch * ji = (jobinfo_RandomSearch *) p;
	
#ifdef ENABLE_THREADS
	const bool updt = false;
#else	// ENABLE_THREADS
	const bool updt = true;
#endif	// ENABLE_THREADS
	
	ji->prj->DoRandomSearch(ji->cycles, ji->optsteps, updt);
	
#ifdef ENABLE_THREADS
	ji->prj->ThreadLock();
	ji->prj->DestroyProgressDialog();
	ji->prj->ThreadUnlock();
#endif	// ENABLE_THREADS
	
	return NULL;
}

/*##############################################*/
/*##############################################*/

void project::DoDeleteCurrentObject(void)
{
	if (selected_object != NULL)
	{
		bool test1 = RemoveLight(selected_object);
		bool test2 = test1; if (!test1) test2 = RemoveObject(selected_object);
		
		if (test2)
		{
			selected_object = NULL;
			UpdateAllGraphicsViews();
		}
	}
}

void project::DoSwitchLocalLights(camera * cam, bool report)
{
	cam->use_local_lights = !cam->use_local_lights;
	if (report) cout << "local lights = " << (cam->use_local_lights ? "on" : "off") << endl;
	SetupLights(cam); UpdateGraphicsViews(cam);
}

void project::DoSwitchGlobalLights(camera * cam, bool report)
{
	cam->use_global_lights = !cam->use_global_lights;
	if (report) cout << "global lights = " << (cam->use_global_lights ? "on" : "off") << endl;
	SetupLights(cam); UpdateGraphicsViews(cam);
}


void project::InitGL(void)
{
//	printf("project::InitGL(void)\n\n");
//    const fGL background[4] = { 0.0, 0.0, 0.0, 1.0};
//    const fGL background[4] = { 0.5, 1.0, 0.5, 0.1};
    const fGL background[4] = { 1.0, 1.0, 1.0, 0.0 };
//	fGL *background = model_prefs->ColorRGBA("Graphics/BkgndColor", default_background);
	glClearColor(background[0], background[1], background[2], background[3]);
//	delete [] background;
	
	glDepthFunc(GL_LESS); glEnable(GL_DEPTH_TEST);
	glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 64);
	
	const fGL specular_reflectance[4] = { 0.9, 0.9, 0.9, 1.0 };
//	fGL *specular_reflectance = model_prefs->ColorRGBA("Graphics/SpecularReflectance", default_reflectance);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular_reflectance);
//	delete [] specular_reflectance;
	
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	
	const fGL ambient_intensity[4] = { 0.2, 0.2, 0.2, 1.0 };
//	fGL *ambient_intensity = model_prefs->ColorRGBA("Graphics/AmbientIntensity", default_ambient);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_intensity);
//	delete [] ambient_intensity;
	
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, true);
	
	glFogi(GL_FOG_MODE, GL_EXP);
//	glFogf(GL_FOG_DENSITY, model_prefs->Double("Graphics/FogDensity",0.15));
	glFogf(GL_FOG_DENSITY, 0.15);
	
	const fGL fog_color[4] = { 0.0, 0.0, 0.0, 0.0 };
//	fGL *fog_color = model_prefs->ColorRGBA("Graphics/FogColor", default_fog);
	glFogfv(GL_FOG_COLOR, fog_color);
//	delete [] fog_color;
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glSelectBuffer(SB_SIZE, select_buffer);
	
	// also setup the lights, just to make sure it always happens...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	for (i32u n1 = 0;n1 < camera_vector.size();n1++)
	{
		cout << "InitGL : setting up lights for cam " << n1 << endl;
		SetupLights(camera_vector[n1]);
	}
}

fGL project::GetDefaultFocus(void)
{
	return 2.0;
}

color_mode * project::GetDefaultColorMode(void)
{
	return & project::cm_element;
}

void project::SelectAll(void)
{
	if (selected_object != NULL)
	{
		selected_object = NULL;
	}
	
	iter_al it1 = atom_list.begin();
	while (it1 != atom_list.end()) (* it1++).flags |= ATOMFLAG_SELECTED;
	
	UpdateAllGraphicsViews();
}

void project::InvertSelection(void)
{
	if (selected_object != NULL)
	{
		selected_object = NULL;
	}
	
	iter_al it1 = atom_list.begin();
	while (it1 != atom_list.end()) (* it1++).flags ^= ATOMFLAG_SELECTED;
	
	UpdateAllGraphicsViews();
}

void project::DeleteSelection(void)
{
	if (selected_object != NULL)
	{
		selected_object = NULL;
	}
	
	iter_al it1 = atom_list.begin();
	while (it1 != atom_list.end())
	{
		if ((* it1).flags & ATOMFLAG_SELECTED)
		{
			RemoveAtom(it1);
			it1 = atom_list.begin();	// reset the search!!!
		}
		else it1++;
	}
	
	UpdateAllGraphicsViews();
}

bool project::TestAtom(atom * ref, rmode rm)
{
	if (ref->flags & ATOMFLAG_IS_HIDDEN) return false;
	
	if (rm == Transform1 && (ref->flags & ATOMFLAG_SELECTED)) return false;
	if (rm == Transform2 && !(ref->flags & ATOMFLAG_SELECTED)) return false;
	
	return true;
}

bool project::TestBond(bond * ref, rmode rm)
{
	if (ref->atmr[0]->flags & ATOMFLAG_IS_HIDDEN) return false;
	if (ref->atmr[1]->flags & ATOMFLAG_IS_HIDDEN) return false;
	
	if (rm == Transform1 && (ref->atmr[0]->flags & ATOMFLAG_SELECTED)) return false;	// no need to study the another...
	if (rm == Transform2 && !(ref->atmr[0]->flags & ATOMFLAG_SELECTED)) return false;	// ...atom due to the test below?
	
	bool test1 = (ref->atmr[0]->flags & ATOMFLAG_SELECTED) ? true : false;
	bool test2 = (ref->atmr[1]->flags & ATOMFLAG_SELECTED) ? true : false;
	if (rm != Normal && test1 != test2) return false;
	
if (ref->do_not_render_TSS_fixmelater) return false;	// temporary, for transition_state_search only...
	
	return true;
}

void project::SetColor(color_mode * cm, atom * ref, bool black_and_white)
{
	fGL select_color[3] = { 1.0, 0.0, 1.0 };	// this can no longer be const...
//	fGL *select_color = model_prefs->ColorRGB("Graphics/SelectColor", default_sel_color);

	if (ref->flags & ATOMFLAG_SELECTED)
	{
		if (black_and_white)	// if we have a red/blue stereo mode, average the colours to shades of gray!
		{
			fGL average = (select_color[0] + select_color[1] + select_color[2]) / 3.0;
			select_color[0] = select_color[1] = select_color[2] = average;
		}
		
		glColor3f(select_color[0], select_color[1], select_color[2]);
	}
	else
	{
		static fGL color[4];
		cm->GetColor4(ref, -1, color);
		
		if (black_and_white)	// if we have a red/blue stereo mode, average the colours to shades of gray!
		{
			fGL average = (color[0] + color[1] + color[2]) / 3.0;
			color[0] = color[1] = color[2] = average;
		}
		
		glColor3fv(color);
	}
	
//	delete [] select_color;
}

void project::DrawCylinder1(const fGL ** crd, const fGL ** col, const fGL * rad)
{
	fGL rsum = rad[0] + rad[1];
	
	for (i32s n1 = 0;n1 < 2;n1++)
	{
		glColor3fv(col[n1]);
		
		v3d<fGL> crt = v3d<fGL>(crd[n1], crd[!n1]);
		fGL pol[3]; crt2pol(crt.data, pol);
		
	//	const float radius = 0.1;	//model_prefs->Double("MM2Graphics/CylinderSize", 0.1);
		const int resolution = 10;	//model_prefs->Value("MM2Graphics/CylinderResolution", 10);
		
		GLUquadricObj * qo = gluNewQuadric();
		gluQuadricDrawStyle(qo, (GLenum) GLU_FILL); glPushMatrix();
		
		glTranslated(crd[n1][0], crd[n1][1], crd[n1][2]);
		
		glRotated(180.0 * pol[1] / M_PI, 0.0, 1.0, 0.0);
		glRotated(180.0 * pol[2] / M_PI, sin(-pol[1]), 0.0, cos(-pol[1]));
		
		fGL length = crt.len() * rad[n1] / rsum;
		gluCylinder(qo, 0.1*rad[n1], 0.1*rad[!n1], length, resolution, resolution / 2);
		
		glPopMatrix(); gluDeleteQuadric(qo);
	}
}

void project::Render(graphics_view * gv, rmode rm)
{
	const fGL label_color[3] = { 0.0, 1.0, 1.0 };	// looks bad but won't fade easily into other colours...
//	fGL *label_color = model_prefs->ColorRGB("Graphics/LabelColor", default_label_color);
	
	bool accum = gv->accumulate; if (rm != Normal) accum = false;
//if (accum) { glClear(GL_ACCUM_BUFFER_BIT); UpdateAccumValues(); }
//else if (rm != Transform2) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (use_boundary_potential && rm == Normal)
	{
		for (int loop = 0;loop < 2;loop++)
		{
			fGL radius;
			
			if (!loop) { glColor3f(0.5, 0.0, 0.5); radius = boundary_potential_radius1; }
			else { glColor3f(1.0, 0.0, 1.0); radius = boundary_potential_radius2; }
			
			glPushMatrix();
			glTranslated(0.0, 0.0, 0.0);	// TODO : set the engine::bp_center[] here!!!
			
			glBegin(GL_LINES);
			
			fGL ang1 = 0.0;
			fGL ca1 = radius * cos(ang1);
			fGL sa1 = radius * sin(ang1);
			
			const i32s divisions = 12;
			for (i32s n1 = 0;n1 < divisions;n1++)
			{
				fGL ang2 = 2.0 * M_PI * ((fGL) (n1 + 1)) / (fGL) divisions;
				fGL ca2 = radius * cos(ang2);
				fGL sa2 = radius * sin(ang2);
				
				glVertex3f(ca1, sa1, 0.0);
				glVertex3f(ca2, sa2, 0.0);
				
				glVertex3f(ca1, 0.0, sa1);
				glVertex3f(ca2, 0.0, sa2);
				
				glVertex3f(0.0, ca1, sa1);
				glVertex3f(0.0, ca2, sa2);
				
				ang1 = ang2; ca1 = ca2; sa1 = sa2;
			}
			
			glEnd();
			
			glPopMatrix();
		}
	}
	
	if (use_periodic_boundary_conditions && 
		rm == Normal)
	{
		glLineWidth(1.0);
		glColor3f(1.0, 0.0, 1.0);
		glBegin(GL_LINES);
		
		glVertex3f(-periodic_box_HALFdim[0], -periodic_box_HALFdim[1], +periodic_box_HALFdim[2]);
		glVertex3f(+periodic_box_HALFdim[0], -periodic_box_HALFdim[1], +periodic_box_HALFdim[2]);
		
		glVertex3f(-periodic_box_HALFdim[0], +periodic_box_HALFdim[1], +periodic_box_HALFdim[2]);
		glVertex3f(+periodic_box_HALFdim[0], +periodic_box_HALFdim[1], +periodic_box_HALFdim[2]);
		
		glVertex3f(-periodic_box_HALFdim[0], -periodic_box_HALFdim[1], -periodic_box_HALFdim[2]);
		glVertex3f(+periodic_box_HALFdim[0], -periodic_box_HALFdim[1], -periodic_box_HALFdim[2]);
		
		glVertex3f(-periodic_box_HALFdim[0], +periodic_box_HALFdim[1], -periodic_box_HALFdim[2]);
		glVertex3f(+periodic_box_HALFdim[0], +periodic_box_HALFdim[1], -periodic_box_HALFdim[2]);
		
		glVertex3f(-periodic_box_HALFdim[0], -periodic_box_HALFdim[1], -periodic_box_HALFdim[2]);
		glVertex3f(-periodic_box_HALFdim[0], -periodic_box_HALFdim[1], +periodic_box_HALFdim[2]);
		
		glVertex3f(-periodic_box_HALFdim[0], +periodic_box_HALFdim[1], -periodic_box_HALFdim[2]);
		glVertex3f(-periodic_box_HALFdim[0], +periodic_box_HALFdim[1], +periodic_box_HALFdim[2]);
		
		glVertex3f(+periodic_box_HALFdim[0], -periodic_box_HALFdim[1], -periodic_box_HALFdim[2]);
		glVertex3f(+periodic_box_HALFdim[0], -periodic_box_HALFdim[1], +periodic_box_HALFdim[2]);
		
		glVertex3f(+periodic_box_HALFdim[0], +periodic_box_HALFdim[1], -periodic_box_HALFdim[2]);
		glVertex3f(+periodic_box_HALFdim[0], +periodic_box_HALFdim[1], +periodic_box_HALFdim[2]);
		
		glVertex3f(-periodic_box_HALFdim[0], -periodic_box_HALFdim[1], -periodic_box_HALFdim[2]);
		glVertex3f(-periodic_box_HALFdim[0], +periodic_box_HALFdim[1], -periodic_box_HALFdim[2]);
		
		glVertex3f(-periodic_box_HALFdim[0], -periodic_box_HALFdim[1], +periodic_box_HALFdim[2]);
		glVertex3f(-periodic_box_HALFdim[0], +periodic_box_HALFdim[1], +periodic_box_HALFdim[2]);
		
		glVertex3f(+periodic_box_HALFdim[0], -periodic_box_HALFdim[1], -periodic_box_HALFdim[2]);
		glVertex3f(+periodic_box_HALFdim[0], +periodic_box_HALFdim[1], -periodic_box_HALFdim[2]);
		
		glVertex3f(+periodic_box_HALFdim[0], -periodic_box_HALFdim[1], +periodic_box_HALFdim[2]);
		glVertex3f(+periodic_box_HALFdim[0], +periodic_box_HALFdim[1], +periodic_box_HALFdim[2]);
		
		glEnd();
	}
	
	if (gv->enable_fog) glEnable(GL_FOG);
	
	i32s layers = 0;
//	if (use_periodic_boundary_conditions && rm == Normal) layers = 1;	// un-comment this to render the periodic images...
	
	for (i32s r1 = -layers;r1 < (layers + 1);r1++)
	{
		for (i32s r2 = -layers;r2 < (layers + 1);r2++)
		{
			for (i32s r3 = -layers;r3 < (layers + 1);r3++)
			{
				glPushMatrix();
				
				fGL trans1 = r1 * 2.0 * periodic_box_HALFdim[0];
				fGL trans2 = r2 * 2.0 * periodic_box_HALFdim[1];
				fGL trans3 = r3 * 2.0 * periodic_box_HALFdim[2];
				
				glTranslated(trans1, trans2, trans3);
				
				RenderScene(gv, rm, accum);
				
				glPopMatrix();
			}
		}
	}
	
	if (accum) glAccum(GL_RETURN, 1.0);
	else if (rm != Transform2) gv->cam->RenderObjects(gv);
	
	if (gv->label == LABEL_INDEX)
	{
		char string[32]; i32s tmp1 = 0;
		
		glColor3f(label_color[0], label_color[1], label_color[2]);
		for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
		{
			if ((* it1).flags & ATOMFLAG_IS_HIDDEN) { tmp1++; continue; }
			
			ostrstream str(string, sizeof(string));
			str << tmp1++ << ends;
			
			const fGL * cdata = (* it1).GetCRD(0);
			fGL x = cdata[0]; fGL y = cdata[1]; fGL z = cdata[2];
			
			gv->WriteGlutString3D(string, x, y, z, gv->cam->GetLocData(), GLUT_BITMAP_9_BY_15);
		}
	}
	else if (gv->label == LABEL_F_CHARGE)
	{
		char string[32];
		
		glColor3f(label_color[0], label_color[1], label_color[2]);
		for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
		{
			if ((* it1).flags & ATOMFLAG_IS_HIDDEN) continue;
			
			ostrstream str(string, sizeof(string));
			str.setf(ios::fixed | ios::showpos); str << (* it1).formal_charge << ends;

			const fGL * cdata = (* it1).GetCRD(0);
			fGL x = cdata[0]; fGL y = cdata[1]; fGL z = cdata[2];
			
			gv->WriteGlutString3D(string, x, y, z, gv->cam->GetLocData(), GLUT_BITMAP_9_BY_15);
		}
	}
	else if (gv->label == LABEL_P_CHARGE)
	{
		char string[32];
		
		glColor3f(label_color[0], label_color[1], label_color[2]);
		for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
		{
			if ((* it1).flags & ATOMFLAG_IS_HIDDEN) continue;
			
			ostrstream str(string, sizeof(string));
			str.setf(ios::fixed | ios::showpos); str.precision(4); str << (* it1).charge << ends;

			const fGL * cdata = (* it1).GetCRD(0);
			fGL x = cdata[0]; fGL y = cdata[1]; fGL z = cdata[2];
			
			gv->WriteGlutString3D(string, x, y, z, gv->cam->GetLocData(), GLUT_BITMAP_9_BY_15);
		}
	}
	else if (gv->label == LABEL_ELEMENT)
	{
		char string[32];
		
		glColor3f(label_color[0], label_color[1], label_color[2]);
		for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
		{
			if ((* it1).flags & ATOMFLAG_IS_HIDDEN) continue;
			
			ostrstream str(string, sizeof(string));
			str << (* it1).el.GetSymbol() << ends;

			const fGL * cdata = (* it1).GetCRD(0);
			fGL x = cdata[0]; fGL y = cdata[1]; fGL z = cdata[2];
			
			gv->WriteGlutString3D(string, x, y, z, gv->cam->GetLocData(), GLUT_BITMAP_9_BY_15);
		}
	}
	else if (gv->label == LABEL_BUILDER_ID)
	{
		char string[32];
		
		glColor3f(label_color[0], label_color[1], label_color[2]);
		for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
		{
			if ((* it1).flags & ATOMFLAG_IS_HIDDEN) continue;
			
			ostrstream str(string, sizeof(string));
			str << "0x" << hex << (* it1).builder_res_id << ends;

			const fGL * cdata = (* it1).GetCRD(0);
			fGL x = cdata[0]; fGL y = cdata[1]; fGL z = cdata[2];
			
			gv->WriteGlutString3D(string, x, y, z, gv->cam->GetLocData(), GLUT_BITMAP_9_BY_15);
		}
	}
	else if (gv->label == LABEL_ATOMTYPE)
	{
		char string[32];
		
		glColor3f(label_color[0], label_color[1], label_color[2]);
		for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
		{
			if ((* it1).flags & ATOMFLAG_IS_HIDDEN) continue;
			
			ostrstream str(string, sizeof(string));
			str << "0x" << hex << (* it1).atmtp << ends;
			
		//	str << "0x" << hex << (* it1).atmtp_E << ends;		// debug...
		//	str << (* it1).atmtp_s << ends;				// debug...
			
		/*	if (!(* it1).atRS) str << "none" << ends;
			else
			{
				atomtype_mmRS * atmtp = (* it1).atRS;
				if (!atmtp) str << "0x" << hex << (* it1).atmtp << ends;
				
				for (int n1 = 0;n1 < atmtp->GetSize();n1++)
				{
					str << "0x" << hex << atmtp->GetAtomType(n1) << " (" << atmtp->GetWeight(n1) << ")" << endl;
				}	str << ends;
			}	*/
			
			const fGL * cdata = (* it1).GetCRD(0);
			fGL x = cdata[0]; fGL y = cdata[1]; fGL z = cdata[2];
			
			gv->WriteGlutString3D(string, x, y, z, gv->cam->GetLocData(), GLUT_BITMAP_9_BY_15);
		}
	}
	else if (gv->label == LABEL_BONDTYPE)
	{
		char string[32];
		
		glColor3f(label_color[0], label_color[1], label_color[2]);
		for (iter_bl it1 = bond_list.begin();it1 != bond_list.end();it1++)
		{
			if ((* it1).atmr[0]->flags & ATOMFLAG_IS_HIDDEN) continue;
			if ((* it1).atmr[1]->flags & ATOMFLAG_IS_HIDDEN) continue;
			
			ostrstream str(string, sizeof(string));
			str << (* it1).bt.GetSymbol1() << ends;
			
			const fGL * cd1 = (* it1).atmr[0]->GetCRD(0); const fGL * cd2 = (* it1).atmr[1]->GetCRD(0);
			fGL x = (cd1[0] + cd2[0]) / 2.0; fGL y = (cd1[1] + cd2[1]) / 2.0; fGL z = (cd1[2] + cd2[2]) / 2.0;
			
			gv->WriteGlutString3D(string, x, y, z, gv->cam->GetLocData(), GLUT_BITMAP_9_BY_15);
		}
	}
	else if (gv->label == LABEL_RESIDUE)
	{
		if (ref_civ != NULL)
		{
			char string[32];
			glColor3f(label_color[0], label_color[1], label_color[2]);
			
			vector<chn_info> & ci_vector = (* ref_civ);
			for (i32u chn = 0;chn < ci_vector.size();chn++)
			{
				iter_al range1[2]; GetRange(1, chn, range1);
				const char * tmp_seq = ci_vector[chn].GetSequence();
				
				for (i32s res = 0;res < ci_vector[chn].GetLength();res++)
				{
					iter_al range2[2]; GetRange(2, range1, res, range2);
					fGL rescrd[3] = { 0.0, 0.0, 0.0 }; i32s counter = 0;
					
				// SLOW because coordinates calculated on-the-fly!!! save them somewhere???
				// SLOW because coordinates calculated on-the-fly!!! save them somewhere???
				// SLOW because coordinates calculated on-the-fly!!! save them somewhere???
					
					for (iter_al it1 = range2[0];it1 != range2[1];it1++)
					{
						const fGL * atmcrd = (* it1).GetCRD(0);
						rescrd[0] += atmcrd[0]; rescrd[1] += atmcrd[1]; rescrd[2] += atmcrd[2];
						counter++;
					}
					
					fGL x = rescrd[0] / (fGL) counter;
					fGL y = rescrd[1] / (fGL) counter;
					fGL z = rescrd[2] / (fGL) counter;
					
					ostrstream str(string, sizeof(string));
					str << tmp_seq[res] << " (" << (chn + 1) << "/" << (res + 1) << ")" << ends;
					
					gv->WriteGlutString3D(string, x, y, z, gv->cam->GetLocData(), GLUT_BITMAP_9_BY_15);
				}
			}
		}
	}
	else if (gv->label == LABEL_SEC_STRUCT)
	{
		if (ref_civ != NULL)
		{
			char string[32];
			glColor3f(label_color[0], label_color[1], label_color[2]);
			
			vector<chn_info> & ci_vector = (* ref_civ);
			for (i32u chn = 0;chn < ci_vector.size();chn++)
			{
				iter_al range1[2]; GetRange(1, chn, range1);
				const char * tmp_states = ci_vector[chn].GetSecStrStates();
				
				for (i32s res = 0;res < ci_vector[chn].GetLength();res++)
				{
					iter_al range2[2]; GetRange(2, range1, res, range2);
					fGL rescrd[3] = { 0.0, 0.0, 0.0 }; i32s counter = 0;
					
				// SLOW because coordinates calculated on-the-fly!!! save them somewhere???
				// SLOW because coordinates calculated on-the-fly!!! save them somewhere???
				// SLOW because coordinates calculated on-the-fly!!! save them somewhere???
					
					for (iter_al it1 = range2[0];it1 != range2[1];it1++)
					{
						const fGL * atmcrd = (* it1).GetCRD(0);
						rescrd[0] += atmcrd[0]; rescrd[1] += atmcrd[1]; rescrd[2] += atmcrd[2];
						counter++;
					}
					
					fGL x = rescrd[0] / (fGL) counter;
					fGL y = rescrd[1] / (fGL) counter;
					fGL z = rescrd[2] / (fGL) counter;
					
					ostrstream str(string, sizeof(string));
					str << tmp_states[res] << ends;
					
					gv->WriteGlutString3D(string, x, y, z, gv->cam->GetLocData(), GLUT_BITMAP_9_BY_15);
				}
			}
		}
	}
	
	if (use_periodic_boundary_conditions && 
		rm == Normal)
	{
		glLineWidth(2.0);
		glColor3f(1.0, 0.0, 1.0);
		glBegin(GL_LINES);

		char string[32]; 
		double len = 0.25;
		fGL x, y, z;

		glVertex3f(-periodic_box_HALFdim[0]-len, -periodic_box_HALFdim[1]-len, -periodic_box_HALFdim[2]-len);
		glVertex3f(-periodic_box_HALFdim[0], -periodic_box_HALFdim[1]-len, -periodic_box_HALFdim[2]-len);
		
		glVertex3f(-periodic_box_HALFdim[0]-len, -periodic_box_HALFdim[1]-len, -periodic_box_HALFdim[2]-len);
		glVertex3f(-periodic_box_HALFdim[0]-len, -periodic_box_HALFdim[1]-len, -periodic_box_HALFdim[2]);
		
		glVertex3f(-periodic_box_HALFdim[0]-len, -periodic_box_HALFdim[1]-len, -periodic_box_HALFdim[2]-len);
		glVertex3f(-periodic_box_HALFdim[0]-len, -periodic_box_HALFdim[1], -periodic_box_HALFdim[2]-len);
		glEnd();
		
		sprintf(string, "X\0");
		x = -periodic_box_HALFdim[0]; y = -periodic_box_HALFdim[1]-len; z = -periodic_box_HALFdim[2]-len;
		gv->WriteGlutString3D(string, x, y, z, gv->cam->GetLocData(), GLUT_BITMAP_9_BY_15);
		
		sprintf(string, "Y\0");
		x = -periodic_box_HALFdim[0]-len; y = -periodic_box_HALFdim[1]; z = -periodic_box_HALFdim[2]-len;
		gv->WriteGlutString3D(string, x, y, z, gv->cam->GetLocData(), GLUT_BITMAP_9_BY_15);
		
		sprintf(string, "Z\0");
		x = -periodic_box_HALFdim[0]-len; y = -periodic_box_HALFdim[1]-len; z = -periodic_box_HALFdim[2];
		gv->WriteGlutString3D(string, x, y, z, gv->cam->GetLocData(), GLUT_BITMAP_9_BY_15);
	}
	if (gv->enable_fog) glDisable(GL_FOG);
	
	// finally call this to handle transparency...
	// finally call this to handle transparency...
	// finally call this to handle transparency...
	
	RenderAllTPs(gv, rm);

//	delete [] label_color;
}

void project::RenderScene(graphics_view * gv, rmode rm, bool accum)
{
	bool do_bw = (gv->cam->stereo_mode && !gv->cam->stereo_relaxed);
	
	for (i32u n1 = 0;n1 < cs_vector.size();n1++)
	{
		if (!GetCRDSetVisible(n1)) continue;
if (accum) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// FIXME!!!

		if (gv->render == RENDER_WIREFRAME)
		{
			glPointSize(3.0); glLineWidth(1.0);
			for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)		// wireframe atoms
			{
				if (!TestAtom(& (* it1), rm)) continue;
				glPushName(GLNAME_MD_TYPE1); glPushName((i32u) & (* it1));
				
				glBegin(GL_POINTS);
				SetColor(gv->colormode, & (* it1), do_bw);
				glVertex3fv((* it1).GetCRD(n1));
				glEnd();
				
				glPopName(); glPopName();
			}
			
			glEnable(GL_LINE_STIPPLE);
			for (iter_bl it2 = bond_list.begin();it2 != bond_list.end();it2++)		// wireframe bonds
			{
				if (!TestBond(& (* it2), rm)) continue;
				
				switch ((* it2).bt.GetSymbol1())
				{
					case 'S': glLineStipple(1, 0xFFFF); break;
					case 'C': glLineStipple(1, 0x3FFF); break;
					case 'D': glLineStipple(1, 0x3F3F); break;
					case 'T': glLineStipple(1, 0x3333); break;
				}
				
				glBegin(GL_LINES);
				SetColor(gv->colormode, (* it2).atmr[0], do_bw);
				glVertex3fv((* it2).atmr[0]->GetCRD(n1));
				SetColor(gv->colormode, (* it2).atmr[1], do_bw);
				glVertex3fv((* it2).atmr[1]->GetCRD(n1));
				glEnd();
			}
			glDisable(GL_LINE_STIPPLE);
		}
		
		if (gv->render != RENDER_WIREFRAME && gv->render != RENDER_NOTHING)
		{
			glEnable(GL_LIGHTING);
			
			for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)		// atoms as spheres
			{
				if (!TestAtom(& (* it1), rm)) continue;
				
				SetColor(gv->colormode, & (* it1), do_bw);
				
				float rad = 0.0; int res = 0;
				switch (gv->render)
				{
					case RENDER_BALL_AND_STICK:
					rad = 0.035;	//model_prefs->Double("MM1Graphics/BallSize", 0.035);
					//if (model_prefs->Boolean("MM1Graphics/BallVdWScale", false)) rad *= (* it1).vdwr * 4.0;
					res = 12;	//model_prefs->Value("MM1Graphics/BallResolution", 12);
					break;
					
					case RENDER_VAN_DER_WAALS:
					rad = (* it1).vdwr;
					res = 22;	//model_prefs->Value("MM1Graphics/VdWResolution", 22);
					break;
					
					case RENDER_CYLINDERS:
					rad = 0.035;	//model_prefs->Double("MM1Graphics/CylinderSize", 0.035);
					res = 12;	//model_prefs->Value("MM1Graphics/CylinderResolution", 12);
					break;
				}
				
				glPushName(GLNAME_MD_TYPE1); glPushName((i32u) & (* it1));
				
				GLUquadricObj * qo = gluNewQuadric();
				gluQuadricDrawStyle(qo, (GLenum) GLU_FILL);
				
				glPushMatrix();
				const fGL * cdata = (* it1).GetCRD(n1);
				glTranslated(cdata[0], cdata[1], cdata[2]);
				gluSphere(qo, rad, res, res / 2);
				glPopMatrix();
				gluDeleteQuadric(qo);
				
				glPopName(); glPopName();
			}
			
			glDisable(GL_LIGHTING);
		}
		
		if (gv->render == RENDER_BALL_AND_STICK || gv->render == RENDER_CYLINDERS)
		{
			glEnable(GL_LIGHTING);
			
			for (iter_bl it1 = bond_list.begin();it1 != bond_list.end();it1++)		// bonds as cylinders
			{
				if (!TestBond(& (* it1), rm)) continue;
				
				fGL vdwr[2] =
				{
					(* it1).atmr[0]->vdwr,
					(* it1).atmr[1]->vdwr
				};
				
				fGL vdwrsum = vdwr[0] + vdwr[1];
				
				for (i32s n2 = 0;n2 < 2;n2++)
				{
					const fGL * crd1 = (* it1).atmr[n2]->GetCRD(n1);
					const fGL * crd2 = (* it1).atmr[!n2]->GetCRD(n1);
					v3d<fGL> crt1 = v3d<fGL>(crd1);
					v3d<fGL> crt2 = v3d<fGL>(crd2);
					v3d<fGL> crt = crt2 - crt1;
					
					fGL pol[3]; crt2pol(crt.data, pol);
					
					SetColor(gv->colormode, (* it1).atmr[n2], do_bw);
					
					float trans, rad = 0.0; int res = 0;
					switch (gv->render)
					{
						case RENDER_BALL_AND_STICK:
						rad = 0.01;	//model_prefs->Double("MM1Graphics/StickSize", 0.01);
						res = 6;	//model_prefs->Value("MM1Graphics/StickResolution", 6);
						break;
						
						case RENDER_CYLINDERS:
						rad = 0.035;	//model_prefs->Double("MM1Graphics/CylinderSize", 0.035);
						res = 12;	//model_prefs->Value("MM1Graphics/CylinderResolution", 12);
						break;
					}
					
					glPushName(GLNAME_MD_TYPE1); glPushName((i32u) (* it1).atmr[n2]);
					
					GLUquadricObj * qo = gluNewQuadric();
					gluQuadricDrawStyle(qo, (GLenum) GLU_FILL);
					glPushMatrix();
					
					glTranslated(crd1[0], crd1[1], crd1[2]);
					
					glRotated(180.0 * pol[1] / M_PI, 0.0, 1.0, 0.0);
					glRotated(180.0 * pol[2] / M_PI, sin(-pol[1]), 0.0, cos(-pol[1]));
					
					// any chance to further define the orientation of, for example, double bonds???
					// one more rotation would be needed. but what is the axis, and how much to rotate???
					
					fGL length = crt.len() * vdwr[n2] / vdwrsum;
					
					if (gv->render == RENDER_BALL_AND_STICK)
					switch ((* it1).bt.GetValue())
					{
						case BONDTYPE_DOUBLE:
						trans = rad;
						rad = rad / 1.5;
						
						if (n2)
							glTranslated(0.0, trans, 0.0);
						else
							glTranslated(0.0, -trans, 0.0);
						gluCylinder(qo, rad, rad, length, res, 1);					
						if (n2)
							glTranslated(0.0, -2.0 * trans, 0.0);
						else
							glTranslated(0.0, 2.0 * trans, 0.0);
						gluCylinder(qo, rad, rad, length, res, 1);
						break;
						
						case BONDTYPE_CNJGTD:
						trans = rad;
						rad = rad / 1.5;
						
						if (n2)
							glTranslated(0.0, trans, 0.0);
						else
							glTranslated(0.0, -trans, 0.0);
						gluCylinder(qo, rad, rad, length, res, 1);
						if (n2)
							glTranslated(0.0, -2.0 * trans, 0.0);
						else
							glTranslated(0.0, 2.0 * trans, 0.0);
						
						glEnable(GL_LINE_STIPPLE);
						glLineStipple(1, 0x3F3F);
						gluQuadricDrawStyle(qo, (GLenum) GLU_LINE);
						gluCylinder(qo, rad, rad, length, res, 1);
						glDisable(GL_LINE_STIPPLE);
						break;
						
						case BONDTYPE_TRIPLE:
						trans = rad;
						rad = rad / 2.0;
						
						if (n2)
							glTranslated(0.0, trans, 0.0);
						else
							glTranslated(0.0, -trans, 0.0);
						gluCylinder(qo, rad, rad, length, res, 1);
						if (n2)
							glTranslated(0.0, -trans, 0.0);
						else
							glTranslated(0.0, trans, 0.0);
						gluCylinder(qo, rad, rad, length, res, 1);
						if (n2)
							glTranslated(0.0, -trans, 0.0);
						else
							glTranslated(0.0, trans, 0.0);
						gluCylinder(qo, rad, rad, length, res,1);
						break;
						
						default:
						gluCylinder(qo, rad, rad, length, res, 1);
					}
					else
						gluCylinder(qo, rad, rad, length, res, 1);
					
					glPopMatrix();
					gluDeleteQuadric(qo);
					
					glPopName(); glPopName();
				}
			}
			
			glDisable(GL_LIGHTING);
		}
		
		// render the additional stuff related to SF.
		// render the additional stuff related to SF.
		// render the additional stuff related to SF.
		
		setup1_sf * susf = dynamic_cast<setup1_sf *>(current_setup);
		if (susf != NULL)
		{
			for (i32u n2 = 0;n2 < susf->hi_vector.size();n2++)	// visualize the helix4 hbonds...
			{
				vector<atom *> * vect = & susf->hi_vector[n2].cg_donacc;
				if (susf->united_atoms) vect = & susf->hi_vector[n2].ua_donacc;
				
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1, 0x3333);
				
				glBegin(GL_LINES);
				glColor3f(0.20, 1.00, 0.10);
				
				for (i32u n3 = 0;n3 < (* vect).size() / 2;n3++)
				{
					const fGL * crd1 = (* vect)[n3 * 2 + 0]->GetCRD(n1);
					const fGL * crd2 = (* vect)[n3 * 2 + 1]->GetCRD(n1);
					
					glVertex3fv(crd1);
					glVertex3fv(crd2);
				}
				
				glEnd();
				glDisable(GL_LINE_STIPPLE);
			}
			
			if (susf->united_atoms)		// the UA-specific rendering starts here...
			{
				for (i32u n2 = 0;n2 < susf->sp_vector.size();n2++)
				{
					glEnable(GL_LINE_STIPPLE);
					glLineStipple(1, 0x3333);
					
					glBegin(GL_LINES);
					glColor3f(0.20, 1.00, 0.10);
					
					for (i32u n3 = 0;n3 < susf->sp_vector[n2].ua_donacc.size() / 2;n3++)
					{
						const fGL * crd1 = susf->sp_vector[n2].ua_donacc[n3 * 2 + 0]->GetCRD(n1);
						const fGL * crd2 = susf->sp_vector[n2].ua_donacc[n3 * 2 + 1]->GetCRD(n1);
						
						glVertex3fv(crd1);
						glVertex3fv(crd2);
					}
					
					glEnd();
					glDisable(GL_LINE_STIPPLE);
				}
			}
			else	// the CG-specific rendering starts here...
			{
				for (i32u n2 = 0;n2 < susf->sp_vector.size();n2++)
				{
					glEnable(GL_LINE_STIPPLE);
					glLineStipple(1, 0x3333);
					
					glBegin(GL_LINES);
					glColor3f(0.20, 1.00, 0.10);
					
					for (i32u n3 = 0;n3 < susf->sp_vector[n2].cg_straight.size() / 2;n3++)
					{
						const fGL * crd1 = susf->sp_vector[n2].cg_straight[n3 * 2 + 0]->GetCRD(n1);
						const fGL * crd2 = susf->sp_vector[n2].cg_straight[n3 * 2 + 1]->GetCRD(n1);
						
						glVertex3fv(crd1);
						glVertex3fv(crd2);
					}
					
					glEnd();
					glDisable(GL_LINE_STIPPLE);
				}
				
				for (i32u n2 = 0;n2 < susf->sp_vector.size();n2++)
				{
					glEnable(GL_LINE_STIPPLE);
					glLineStipple(1, 0x1111);
					
					glBegin(GL_LINES);
					glColor3f(0.20, 1.00, 0.10);
					
					for (i32u n3 = 0;n3 < susf->sp_vector[n2].cg_crossed.size() / 2;n3++)
					{
						const fGL * crd1 = susf->sp_vector[n2].cg_crossed[n3 * 2 + 0]->GetCRD(n1);
						const fGL * crd2 = susf->sp_vector[n2].cg_crossed[n3 * 2 + 1]->GetCRD(n1);
						
						glVertex3fv(crd1);
						glVertex3fv(crd2);
					}
					
					glEnd();
					glDisable(GL_LINE_STIPPLE);
				}
				
				// protein chains...
				
				for (i32u n2 = 0;n2 < susf->chn_vector.size();n2++)
				{
					for (i32s n3 = 0;n3 < ((i32s) susf->chn_vector[n2].res_vector.size()) - 1;n3++)
					{
						i32s ind1[3] = { n2, n3 + 0, 0 };
						i32s ind2[3] = { n2, n3 + 1, 0 };
						
						const fGL * crd1 = susf->chn_vector[ind1[0]].res_vector[ind1[1]].GetRefA(ind1[2])->GetCRD(n1);
						const fGL * crd2 = susf->chn_vector[ind2[0]].res_vector[ind2[1]].GetRefA(ind2[2])->GetCRD(n1);
						const fGL * crd[2] = { crd1, crd2 };
						
						fGL col1[4] = { 0.8, 0.8, 0.6, 1.0 };	// todo...
						fGL col2[4] = { 0.8, 0.8, 0.6, 1.0 };	// todo...
						const fGL * col[2] = { col1, col2 };
						
						fGL rad[2] =
						{
							susf->chn_vector[ind1[0]].res_vector[ind1[1]].GetRefA(ind1[2])->vdwr,
							susf->chn_vector[ind2[0]].res_vector[ind2[1]].GetRefA(ind2[2])->vdwr
						};
						
						if (gv->render == RENDER_WIREFRAME)
						{
							glBegin(GL_LINES);
							glColor3fv(col1); glVertex3fv(crd1);
							glColor3fv(col2); glVertex3fv(crd2);
							glEnd();
						}
						else if (gv->render != RENDER_NOTHING)
						{
							glEnable(GL_LIGHTING);
							DrawCylinder1(crd, col, rad);
							glDisable(GL_LIGHTING);
						}
					}
					{
					for (i32u n3 = 0;n3 < susf->chn_vector[n2].res_vector.size();n3++)
					{
						for (i32s n4 = 0;n4 < susf->chn_vector[n2].res_vector[n3].GetNumA() - 1;n4++)
						{
							i32s ind1[3] = { n2, n3, n4 + 0 };
							i32s ind2[3] = { n2, n3, n4 + 1 };
							
							const fGL * crd1 = susf->chn_vector[ind1[0]].res_vector[ind1[1]].GetRefA(ind1[2])->GetCRD(n1);
							const fGL * crd2 = susf->chn_vector[ind2[0]].res_vector[ind2[1]].GetRefA(ind2[2])->GetCRD(n1);
							const fGL * crd[2] = { crd1, crd2 };
							
							fGL col1[4] = { 0.6, 0.8, 0.8, 1.0 };	// todo...
							fGL col2[4] = { 0.6, 0.8, 0.8, 1.0 };	// todo...
							const fGL * col[2] = { col1, col2 };
							
							fGL rad[2] =
							{
								susf->chn_vector[ind1[0]].res_vector[ind1[1]].GetRefA(ind1[2])->vdwr,
								susf->chn_vector[ind2[0]].res_vector[ind2[1]].GetRefA(ind2[2])->vdwr
							};
							
							if (gv->render == RENDER_WIREFRAME)
							{
								glBegin(GL_LINES);
								glColor3fv(col1); glVertex3fv(crd1);
								glColor3fv(col2); glVertex3fv(crd2);
								glEnd();
							}
							else if (gv->render != RENDER_NOTHING)
							{
								glEnable(GL_LIGHTING);
								DrawCylinder1(crd, col, rad);
								glDisable(GL_LIGHTING);
							}
						}
					}
					}
				}
				
				// disulphide bridges.
				{
				for (i32s n2 = 0;n2 < (i32s) susf->dsb_vector.size();n2++)
				{
					i32s ind1[3] = { susf->dsb_vector[n2].chn[0], susf->dsb_vector[n2].res[0], 1 };
					i32s ind2[3] = { susf->dsb_vector[n2].chn[1], susf->dsb_vector[n2].res[1], 1 };
					
					const fGL * crd1 = susf->chn_vector[ind1[0]].res_vector[ind1[1]].GetRefA(ind1[2])->GetCRD(n1);
					const fGL * crd2 = susf->chn_vector[ind2[0]].res_vector[ind2[1]].GetRefA(ind2[2])->GetCRD(n1);
					const fGL * crd[2] = { crd1, crd2 };
					
					fGL col1[4] = { 1.0, 1.0, 0.0, 1.0 };	// todo...
					fGL col2[4] = { 1.0, 1.0, 0.0, 1.0 };	// todo...
					const fGL * col[2] = { col1, col2 };
					
					fGL rad[2] =
					{
						susf->chn_vector[ind1[0]].res_vector[ind1[1]].GetRefA(ind1[2])->vdwr,
						susf->chn_vector[ind2[0]].res_vector[ind2[1]].GetRefA(ind2[2])->vdwr
					};
					
					if (gv->render == RENDER_WIREFRAME)
					{
						glBegin(GL_LINES);
						glColor3fv(col1); glVertex3fv(crd1);
						glColor3fv(col2); glVertex3fv(crd2);
						glEnd();
					}
					else
					{
						glEnable(GL_LIGHTING);
						DrawCylinder1(crd, col, rad);
						glDisable(GL_LIGHTING);
					}
				}
				}
			}
		}
		
/*//////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
	glEnable(GL_LIGHTING); glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, true); glBegin(GL_QUADS);
	// do not take the direction from any array, but calculate it using N/C/O???
	for (iter_bl it1 = bond_list.begin();it1 != bond_list.end();it1++)	// sf peptide dipoles...
	{
		if ((* it1).sf_pbdd < -1000.0) continue;
		
	//char symbol2 = chn_vector[n2].res_vector[n3 + 1].symbol;
	//if (symbol2 == 'P') continue;	// skip all X-pro cases !!!
		
		atom * prev = NULL;
		atom * curr = (* it1).atmr[0];
		atom * next = (* it1).atmr[1];
		
		// WARNING!!! this is pretty slow!!! need to find the previous c-alpha.
		iter_cl it2;
		for (it2 = curr->cr_list.begin();it2 != curr->cr_list.end();it2++)
		{
			if ((* it2).atmr == next) continue;
			
			if ((* it2).atmr->el.GetAtomicNumber() > 0) continue;
			if ((* it2).atmr->sf_atmtp & 0xFF) continue;
			
			prev = (* it2).atmr;
			break;
		}
		
		if (!prev) continue;
		
		v3d<fGL> v1(prev->GetCRD(n1), curr->GetCRD(n1));
		v3d<fGL> v2(curr->GetCRD(n1), next->GetCRD(n1));
		
		v3d<fGL> v3 = v1.vpr(v2); v3 = v3 * (0.075 / v3.len());
		v3d<fGL> v4 = v3.vpr(v2); v4 = v4 * (0.075 / v4.len());
		
		fGL peptide = (* it1).sf_pbdd;	// this is the same for all crd_sets!!!
		v3d<fGL> v5 = (v3 * sin(peptide)) + (v4 * cos(peptide));
		
		fGL peptnorm = peptide - M_PI / 2.0;
		v3d<fGL> normal = (v3 * sin(peptnorm)) + (v4 * cos(peptnorm));
		normal = normal / normal.len(); glNormal3fv(normal.data);
		
		v3d<fGL> pvc(curr->GetCRD(n1));
		v3d<fGL> pv1 = pvc + (v2 * 0.5) + v5; v3d<fGL> pv2 = pvc + (v2 * 0.90);
		v3d<fGL> pv3 = pvc + (v2 * 0.5) - v5; v3d<fGL> pv4 = pvc + (v2 * 0.10);
		
		glColor3f(1.0, 0.0, 0.0); glVertex3fv(pv1.data);
		glColor3f(0.0, 1.0, 0.0); glVertex3fv(pv2.data);
		glColor3f(0.0, 0.0, 1.0); glVertex3fv(pv3.data);
		glColor3f(0.0, 1.0, 0.0); glVertex3fv(pv4.data);
	}
	glEnd();	// GL_QUADS
	glDisable(GL_LIGHTING); glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, false);
////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////*/
		
		if (accum)
		{
			gv->cam->RenderObjects(gv);
			glAccum(GL_ACCUM, cs_vector[n1]->accum_value);
		}
	}
}

void project::Center(transformer * p1)
{
	i32s sum = 0;
	p1->GetLocDataRW()->crd[0] = 0.0;
	p1->GetLocDataRW()->crd[1] = 0.0;
	p1->GetLocDataRW()->crd[2] = 0.0;
	
	for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		if (!((* it1).flags & ATOMFLAG_SELECTED)) continue;
		for (i32u n1 = 0;n1 < cs_vector.size();n1++)
		{
			sum++;
			const fGL * cdata = (* it1).GetCRD(n1);
			p1->GetLocDataRW()->crd[0] += cdata[0];
			p1->GetLocDataRW()->crd[1] += cdata[1];
			p1->GetLocDataRW()->crd[2] += cdata[2];
		}
	}
	
	if (!sum) return;
	
	p1->GetLocDataRW()->crd[0] /= (fGL) sum;
	p1->GetLocDataRW()->crd[1] /= (fGL) sum;
	p1->GetLocDataRW()->crd[2] /= (fGL) sum;
	
	for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		if (!((* it1).flags & ATOMFLAG_SELECTED)) continue;
		for (i32u n1 = 0;n1 < cs_vector.size();n1++)
		{
			const fGL * cdata = (* it1).GetCRD(n1);
			
			fGL x = cdata[0] - p1->GetLocData()->crd[0];
			fGL y = cdata[1] - p1->GetLocData()->crd[1];
			fGL z = cdata[2] - p1->GetLocData()->crd[2];
			
			(* it1).SetCRD(n1, x, y, z);
		}
	}
}

void project::Transform(transformer * p1)
{
	fGL matrix[16]; p1->GetMatrix(matrix);
	
	for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
	{
		if (!((* it1).flags & ATOMFLAG_SELECTED)) continue;
		
		for (i32u n1 = 0;n1 < cs_vector.size();n1++)
		{
			v3d<fGL> posv = v3d<fGL>((* it1).GetCRD(n1));
			TransformVector(posv, matrix);
			
			(* it1).SetCRD(n1, posv[0], posv[1], posv[2]);
		}
	}
	
	UpdateAllGraphicsViews();	// re-draw the bonds across selection boundary!!!
}

void project::MolAxisEvent(graphics_view * gv, vector<iGLu> & names)
{
	if (ogl_view::button == mouse_tool::Right) return;	// the right button is for popup menus...
	
	i32s mouse[2] =
	{
		gv->current_tool->latest_x,
		gv->current_tool->latest_y
	};
	if (ogl_view::state == mouse_tool::Down)
	{
		if (names.size() > 1 && names[0] == GLNAME_MD_TYPE1)
		{
			draw_data[0] = (atom *) names[1];
		}
		else
		{
			/*fGL tmp1[3]; gv->GetCRD(mouse, tmp1);
			atom newatom(element::current_element, tmp1, cs_vector.size());
			AddAtom(newatom); draw_data[0] = & atom_list.back();*/
		}
	}
	else
	{
		if (names.size() > 1 && names[0] == GLNAME_MD_TYPE1)
		{
			draw_data[1] = (atom *) names[1];

			if (draw_data[0] != draw_data[1])
			{
				//bond newbond(draw_data[0], draw_data[1], bondtype::current_bondtype);
				molecular_axis new_mol_axis(draw_data[0], draw_data[1]);

				draw_data[0]->flags |= ATOMFLAG_SELECTED;
				draw_data[1]->flags |= ATOMFLAG_SELECTED;

				SystemWasModified();

				this->AddMolAxis(new_mol_axis);

		
				UpdateAllGraphicsViews();

			}
			else
			{
				/*SystemWasModified();
				
				draw_data[0]->el = element::current_element;
				draw_data[0]->mass = element::current_element.GetAtomicMass();		// also need to update these...
				draw_data[0]->vdwr = element::current_element.GetVDWRadius();		// also need to update these...
				
				if (pv != NULL) pv->AtomUpdateItem(draw_data[0]);*/
			}
		}
		else
		{
			/*fGL tmp1[3]; gv->GetCRD(mouse, tmp1);
			atom newatom(element::current_element, tmp1, cs_vector.size());
			AddAtom(newatom); draw_data[1] = & atom_list.back();*/
		}
		
		// if different: update bondtype or add a new bond.
		// if not different: change atom to different element.
		

		
	}
}

void project::DrawEvent(graphics_view * gv, vector<iGLu> & names)
{
	if (ogl_view::button == mouse_tool::Right) return;	// the right button is for popup menus...
	
	i32s mouse[2] =
	{
		gv->current_tool->latest_x,
		gv->current_tool->latest_y
	};
	if (ogl_view::state == mouse_tool::Down)
	{
		if (names.size() > 1 && names[0] == GLNAME_MD_TYPE1)
		{
			draw_data[0] = (atom *) names[1];
		}
		else
		{
			fGL tmp1[3]; gv->GetCRD(mouse, tmp1);
			atom newatom(element::current_element, tmp1, cs_vector.size());
			AddAtom(newatom); draw_data[0] = & atom_list.back();
		}
	}
	else
	{
		if (names.size() > 1 && names[0] == GLNAME_MD_TYPE1)
		{
			draw_data[1] = (atom *) names[1];
		}
		else
		{
			fGL tmp1[3]; gv->GetCRD(mouse, tmp1);
			atom newatom(element::current_element, tmp1, cs_vector.size());
			AddAtom(newatom); draw_data[1] = & atom_list.back();
		}
		
		// if different: update bondtype or add a new bond.
		// if not different: change atom to different element.
		
		if (draw_data[0] != draw_data[1])
		{
			bond newbond(draw_data[0], draw_data[1], bondtype::current_bondtype);
			iter_bl it1 = find(bond_list.begin(), bond_list.end(), newbond);
			if (it1 != bond_list.end())
			{
				SystemWasModified();
				
				(* it1).bt = bondtype::current_bondtype;
				
				if (pv != NULL) pv->BondUpdateItem(& (* it1));
			}
			else AddBond(newbond);
		}
		else
		{
			SystemWasModified();
			
			draw_data[0]->el = element::current_element;
			draw_data[0]->mass = element::current_element.GetAtomicMass();		// also need to update these...
			draw_data[0]->vdwr = element::current_element.GetVDWRadius();		// also need to update these...
			
			if (pv != NULL) pv->AtomUpdateItem(draw_data[0]);
		}
		
		UpdateAllGraphicsViews();
	}
}

void project::EraseEvent(graphics_view * gv, vector<iGLu> & names)
{
	if (ogl_view::button == mouse_tool::Right) return;	// the right button is for popup menus...
	
	if (ogl_view::state == mouse_tool::Down)
	{
		if (names.size() > 1 && names[0] == GLNAME_MD_TYPE1)
		{
			draw_data[0] = (atom *) names[1];
		}
		else
		{
			draw_data[0] = NULL;
		}
	}
	else
	{
		if (names.size() > 1 && names[0] == GLNAME_MD_TYPE1)
		{
			draw_data[1] = (atom *) names[1];
		}
		else
		{
			draw_data[1] = NULL;
		}
		
		if (!draw_data[0] || !draw_data[1]) return;
		
		// if different: try to find and remove a bond.
		// if not different: remove atom.
		
		if (draw_data[0] != draw_data[1])
		{
			bond tmpbond(draw_data[0], draw_data[1], bondtype::current_bondtype);
			iter_bl it1 = find(bond_list.begin(), bond_list.end(), tmpbond);
			if (it1 != bond_list.end()) RemoveBond(it1); else return;
		}
		else
		{
			iter_al it1 = find(atom_list.begin(), atom_list.end(), (* draw_data[0]));
			if (it1 != atom_list.end())
			{
				RemoveAtom(it1);
				
				// removing an atom will cause changes in atom indexing -> must update all atoms and bonds in pv!!!
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
				
				for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++) if (pv != NULL) pv->AtomUpdateItem(& (* it1));
				{
				for (iter_bl it1 = bond_list.begin();it1 != bond_list.end();it1++) if (pv != NULL) pv->BondUpdateItem(& (* it1));
				}
			}
			else
			{
				cout << "BUG: project::EraseEvent()." << endl;
				exit(EXIT_FAILURE);
			}
		}
		
		UpdateAllGraphicsViews();
	}
}

void project::SelectEvent(graphics_view *, vector<iGLu> & names)
{
	if (names[0] == GLNAME_MD_TYPE1)
	{
		atom * ref = (atom *) names[1];
		bool selected = (ref->flags & ATOMFLAG_SELECTED);
		
		if (ogl_view::select_mode == select_tool::Residue || ogl_view::select_mode == select_tool::Chain)
		{
			if (ref_civ == NULL)
			{
				bool update = Question("Group information about chains/residues is needed for this operation.\nIs it OK to update group information?\n");
				if (!update) return;
				
				UpdateChains();
			}
			
			bool no_info = false;
			if (ref->id[1] == NOT_DEFINED) no_info = true;
			if (ogl_view::select_mode == select_tool::Residue && ref->id[2] == NOT_DEFINED) no_info = true;
			
			if (no_info)
			{
				Message("Sorry, no chain/residue information available for this atom.");
				return;
			}
		}
		
		if (ogl_view::select_mode == select_tool::Molecule)
		{
			if (!IsGroupsClean()) UpdateGroups();
		}
		
		iter_al it1;
		iter_al range1[2];
		iter_al range2[2];
		
		if (selected) cout << "de";
		switch (ogl_view::select_mode)
		{
			case select_tool::Atom:
			ref->flags ^= ATOMFLAG_SELECTED;
			cout << "selected atom " << ref->index << " (atomtype = " << hex << ref->atmtp << dec << ")." << endl;
			break;
			
			case select_tool::Residue:
			GetRange(1, ref->id[1], range1);		// get the chain!!!
			GetRange(2, range1, ref->id[2], range2);	// get the residue!!!
			for (it1 = range2[0];it1 != range2[1];it1++)
			{
				if (selected) (* it1).flags &= (~ATOMFLAG_SELECTED);
				else (* it1).flags |= (ATOMFLAG_SELECTED);
			}
			
			cout << "selected residue " << ref->id[2] << " from chain " << ref->id[1] << "." << endl;
			break;
			
			case select_tool::Chain:
			GetRange(1, ref->id[1], range1);		// get the chain!!!
			for (it1 = range1[0];it1 != range1[1];it1++)
			{
				if (selected) (* it1).flags &= (~ATOMFLAG_SELECTED);
				else (* it1).flags |= (ATOMFLAG_SELECTED);
			}
			
			cout << "selected chain " << ref->id[1] << "." << endl;
			break;
			
			case select_tool::Molecule:
			if (IsGroupsSorted())	// if atom_list is sorted, a quicker method based on model::GetRange() is used.
			{
				GetRange(0, ref->id[0], range1);		// get the molecule!!!
				for (it1 = range1[0];it1 != range1[1];it1++)
				{
					if (selected) (* it1).flags &= (~ATOMFLAG_SELECTED);
					else (* it1).flags |= (ATOMFLAG_SELECTED);
				}
			}
			else
			{
				for (it1 = GetAtomsBegin();it1 != GetAtomsEnd();it1++)
				{
					if ((* it1).id[0] != ref->id[0]) continue;
					
					if (selected) (* it1).flags &= (~ATOMFLAG_SELECTED);
					else (* it1).flags |= (ATOMFLAG_SELECTED);
				}
			}
			
			cout << "selected molecule " << ref->id[0] << "." << endl;
			break;
		}
		
		UpdateAllGraphicsViews();
	}
}

void project::MeasureEvent(graphics_view *, vector<iGLu> & names)
{
	char mbuff1[256];
	ostrstream str1(mbuff1, sizeof(mbuff1));
	
	// PLEASE NOTE!!! we use always the 1st coordinate set here...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	// we can be sure that "ref" is always up-to-date but the stored pointers
	// mt_a1/2/3 can be invalid ; so check them before use. reset if problems.
	
	if (names[0] == GLNAME_MD_TYPE1)
	{
		atom * ref = (atom *) names[1];
		ref->flags ^= ATOMFLAG_SELECTED;
		UpdateAllGraphicsViews();
		
		if (mt_a1 == NULL)
		{
			mt_a1 = ref;	// this must be OK.
			str1 << "charge: " << ref->charge << endl << ends;
// my measure
const fGL * p1 = mt_a1->GetCRD(0);
cout << "el = " << mt_a1->el.GetSymbol() << " " << mt_a1->el.GetAtomicNumber() << " x = " << p1[0] << " y = " << p1[1] << " z = " << p1[2] << endl;
cout << "Index of this entry in model::atom_list i32s index  = " << mt_a1->index << endl;
cout << "Index of this entry in setup::atmtab    i32s varind = " << mt_a1->varind << endl;

			PrintToLog(mbuff1);


		}
		else if (mt_a1 != NULL && mt_a2 == NULL)
		{
			if (mt_a1 == ref) { mt_a1->flags &= (~ATOMFLAG_SELECTED); mt_a1 = NULL; return; }
			
			mt_a2 = ref;	// this must be OK.
			
			iter_al itX;
for (itX = atom_list.begin();itX != atom_list.end();itX++) if (& (* itX) == mt_a1) break; if (itX == atom_list.end()) goto reset_all;
			
			const fGL * p1 = mt_a1->GetCRD(0);
			const fGL * p2 = mt_a2->GetCRD(0);

			

// my measure
cout  << "el = " << mt_a2->el.GetSymbol() << " " << mt_a2->el.GetAtomicNumber() << " x = " << p2[0] << " y = " << p2[1] << " z = " << p2[2] << endl;
			
			v3d<fGL> v1(p1, p2);
			fGL len = v1.len();
			
			str1 << "distance: " << len << " nm" << endl << ends;
			PrintToLog(mbuff1);
		}
		else if (mt_a1 != NULL && mt_a2 != NULL && mt_a3 == NULL)
		{
			if (mt_a1 == ref) { mt_a1->flags &= (~ATOMFLAG_SELECTED); mt_a1 = mt_a2; mt_a2 = NULL; return; }
			else if (mt_a2 == ref) { mt_a2->flags &= (~ATOMFLAG_SELECTED); mt_a2 = NULL; return; }
			
			mt_a3 = ref;	// this must be OK.
			
			iter_al itX;
for (itX = atom_list.begin();itX != atom_list.end();itX++) if (& (* itX) == mt_a1) break; if (itX == atom_list.end()) goto reset_all;
for (itX = atom_list.begin();itX != atom_list.end();itX++) if (& (* itX) == mt_a2) break; if (itX == atom_list.end()) goto reset_all;
			
			const fGL * p1 = mt_a1->GetCRD(0);
			const fGL * p2 = mt_a2->GetCRD(0);
			const fGL * p3 = mt_a3->GetCRD(0);
			
// my measure
cout  << "el = " << mt_a3->el.GetSymbol() << " " << mt_a3->el.GetAtomicNumber() << " x = " << p3[0] << " y = " << p3[1] << " z = " << p3[2] << endl;
			v3d<fGL> v1(p2, p1);
			v3d<fGL> v2(p2, p3);
			fGL ang = v1.ang(v2) * 180.0 / M_PI;
			
			str1 << "angle: " << ang << " deg" << endl << ends;
			PrintToLog(mbuff1);
		}
		else
		{
			if (mt_a1 == ref) { mt_a1->flags &= (~ATOMFLAG_SELECTED); mt_a1 = mt_a2; mt_a2 = mt_a3; mt_a3 = NULL; return; }
			else if (mt_a2 == ref) { mt_a2->flags &= (~ATOMFLAG_SELECTED); mt_a2 = mt_a3; mt_a3 = NULL; return; }
			else if (mt_a3 == ref) { mt_a3->flags &= (~ATOMFLAG_SELECTED); mt_a3 = NULL; return; }
			
			const fGL * p1 = mt_a1->GetCRD(0);
			const fGL * p2 = mt_a2->GetCRD(0);
			const fGL * p3 = mt_a3->GetCRD(0);
			const fGL * p4 = ref->GetCRD(0);
			
			v3d<fGL> v1(p2, p1);
			v3d<fGL> v2(p2, p3);
			v3d<fGL> v3(p3, p4);
			fGL tor = v1.tor(v2, v3) * 180.0 / M_PI;
			
// my measure
cout  << "el = " << ref->el.GetSymbol() << " " << ref->el.GetAtomicNumber() << " x = " << p4[0] << " y = " << p4[1] << " z = " << p4[2] << endl;

			str1 << "torsion: " << tor << " deg " << endl << ends;
			PrintToLog(mbuff1);
			
			mt_a1->flags &= (~ATOMFLAG_SELECTED);
			mt_a2->flags &= (~ATOMFLAG_SELECTED);
			mt_a3->flags &= (~ATOMFLAG_SELECTED);
			ref->flags &= (~ATOMFLAG_SELECTED);
			
			goto reset_all;
		}
		
		return;
	}
	
	reset_all:
	
	mt_a1 = mt_a2 = mt_a3 = NULL;
	UpdateAllGraphicsViews();
}

double project::DoFormula(bool msg)
{
  double molweight = 0.0;
  int i;
  char buffer[1024];
  ostrstream str(buffer, sizeof(buffer));

  int count[ELEMENT_SYMBOLS];
  // These are the atomic numbers of the elements in alphabetical order.
  const int alphabetical[ELEMENT_SYMBOLS] = {
   89, 47, 13, 95, 18, 33, 85, 79, 5, 56, 4, 107, 83, 97, 35, 6, 20, 48,
   58, 98, 17, 96, 27, 24, 55, 29, 105, 66, 68, 99, 63, 9, 26, 100, 87, 31,
   64, 32, 1, 2, 72, 80, 67, 108, 53, 49, 77, 19, 36, 57, 3, 103, 71, 101,
   12, 25, 42, 109, 7, 11, 41, 60, 10, 28, 102, 93, 8, 76, 15, 91, 82, 46, 
   61, 84, 59, 78, 94, 88, 37, 75, 104, 45, 86, 44, 16, 51, 21, 34, 106, 14, 
   62, 50, 38, 73, 65, 43, 52, 90, 22, 81, 69, 92, 110, 23, 74, 54, 39, 70, 
   30, 40 };
  int index;

  for (i = 0; i < ELEMENT_SYMBOLS; i++)
    {
      count[i] = 0;
    }

  iter_al it2 = atom_list.begin();
  while (it2 != atom_list.end())
    {
      iter_al it3 = it2++;
      count[(* it3).el.GetAtomicNumber() - 1]++;
      molweight += (* it3).mass;
    }
  
  for (i = 0; i < ELEMENT_SYMBOLS; i++)
    {
      index = alphabetical[i] - 1;
      if (count[index] > 1)
	{
	  str << (element(index + 1)).GetSymbol() << count[index] << " ";
	}
      else if (count[index] == 1)
	{
	  str << (element(index + 1)).GetSymbol();
	}
    }

  str << endl;
  str << "MW: " << molweight << ends;
  if (msg)
	  Message(buffer);

  return molweight;
}
void project::DoDensity(void)
{
	double molweight = this->DoFormula(false);// г/моль
	double V = 
		this->periodic_box_HALFdim[0] * 
		this->periodic_box_HALFdim[1] * 
		this->periodic_box_HALFdim[2] * 
		8.0;// нм^3

	double Na = 6.0221367e+23;
	double m = molweight / Na;
	double density = 1.0e+21 * m / V;

  char buffer[1024];
  ostrstream str(buffer, sizeof(buffer));
  str << endl;
  str << "density: " << density << " г/см^3" << ends;

  Message(buffer);
}

void project::TrajView_CoordinatePlot(i32s inda, i32s dim)
{
	if (this->GetTrajectoryFile())
	{
		this->ErrorMessage("Trajectory already open?!?!?!");
		return;
	}

	char filename[512];
	DWORD nFilterIndex;
	if (OpenFileDlg(NULL, "Ghemical Trajectory File (*.traj)\0*.traj\0All files \0*.*\0", filename, nFilterIndex) != S_OK)
	{
		return;
	}

	cout << "trying to open \"" << filename << "\"." << endl;
	this->OpenTrajectory(filename);
	// check if there were problems with OpenTrajectory()?!?!?!
	// check if there were problems with OpenTrajectory()?!?!?!
	// check if there were problems with OpenTrajectory()?!?!?!

	const char * s1 = "frame(num)"; const char * sv = "distance (nm)";
	plot1d_view * plot = AddPlot1DView(PLOT_USERDATA_STRUCTURE, s1, sv, true);

	float ekin;
	float epot;
	float tmp;

	i32s shift = 0;
	fGL previouse_coordinate = 0.0;

	i32s max_frames = this->GetTotalFrames();
	for (i32s loop = 0;loop < max_frames;loop++)
	{
		this->SetCurrentFrame(loop);
		//this->ReadTrajectoryFrame();
		//void project::ReadTrajectoryFrame(void)
		//{
		i32s place = GetTrajectoryHeaderSize();						// skip the header...
		place += GetTrajectoryFrameSize() * current_traj_frame;		// get the correct frame...
		//place += GetTrajectoryEnergySize();							// skip epot and ekin...
		
		trajfile->seekg(place, ios::beg);

		trajfile->read((char *) & ekin, sizeof(ekin));
		trajfile->read((char *) & epot, sizeof(epot));

		float boundary[3];
		if (trajectory_version > 10)
		{
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[0] = tmp;
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[1] = tmp;
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[2] = tmp;
		}

		i32s ind = 0;
		mt_a1 = mt_a2 = mt_a3 = NULL;

		for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
		{
		//	if ((* it1).flags & ATOMFLAG_IS_HIDDEN) continue;	// currently all coordinates are written...
			float t1a;
			fGL cdata[3];
			for (i32s t4 = 0;t4 < 3;t4++)
			{
				trajfile->read((char *) & t1a, sizeof(t1a));
				cdata[t4] = t1a;
			}

			if (trajectory_version > 12)
			{
				fGL vdata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					vdata[t4] = tmp;
				}
			}

			if (trajectory_version > 13)
			{
				fGL adata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					adata[t4] = tmp;
				}
			}

			if (trajectory_version > 11)
			{
				fGL fdata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					fdata[t4] = tmp;
				}
			}

			(* it1).SetCRD(0, cdata[0], cdata[1], cdata[2]);

			if (ind == inda)
			{
				mt_a1 = &(* it1);
			}

			/*if (ind == indb)
			{
				mt_a2 = &(* it1);
			}

			if (ind == indc)
			{
				mt_a3 = &(* it1);
			}*/

			ind++;
		}
		//this->UpdateAllGraphicsViews();

		fGL coordinate;

		if (mt_a1 && dim >=0 && dim < 3)
		{
			const fGL * p1 = mt_a1->GetCRD(0);
			//const fGL * p2 = mt_a2->GetCRD(0);
			// my measure
			//cout  << "el = " << mt_a1->el.GetSymbol() << " " << mt_a1->el.GetAtomicNumber() << " x = " << p1[0] << " y = " << p1[1] << " z = " << p1[2] << endl;
			coordinate = p1[dim];
		}
		else {
			coordinate = 0;
		}

		if (coordinate - previouse_coordinate > boundary[dim])
		{
			shift -= 1;
			printf("coordinate %f- previouse_coordinate %f > boundary[dim] %f shift %d\n", coordinate, previouse_coordinate, boundary[dim], shift);
		}
		if (coordinate - previouse_coordinate < -boundary[dim])
		{
			shift += 1;
			printf("coordinate %f- previouse_coordinate %f < boundary[dim] %f shift %d\n", coordinate, previouse_coordinate, boundary[dim], shift);
		}

		//ref->this->UpdateAllGraphicsViews(true);
		//::Sleep(100);
		void * udata = convert_cset_to_plotting_udata(this, 0);
		f64 value = coordinate + 2*shift*boundary[dim];
		plot->AddData(loop, value, udata);

		mt_a1 = mt_a2 = mt_a3 = NULL;
		previouse_coordinate = coordinate;
	}

	this->CloseTrajectory();

//				static trajview_dialog * tvd = NULL;

//				if (tvd != NULL) delete tvd;		// how to safely release the memory...
	//tvd =
	//	new trajview_dialog(this);		// ...right after the dialog is closed?
	// the dialog will call this->CloseTrajectory() itself when closed!!!
	// the dialog will call this->CloseTrajectory() itself when closed!!!
	// the dialog will call this->CloseTrajectory() itself when closed!!!
	plot->SetCenterAndScale();
	plot->Update();
}

void project::TrajView_CoordinateDifferencePlot(i32s ind1, i32s ind2, i32s dim)
{
	//win_graphics_view * gv = win_graphics_view::GetGV(widget);
	//win_project * prj = dynamic_cast<win_project *>(gv->prj);
	//if (prj)// new trajfile_dialog(prj);	// will call delete itself...
	//{
		if (!this->GetTrajectoryFile())
		{

			char filename[512];
			DWORD nFilterIndex;
			if (OpenFileDlg(NULL, "Ghemical Trajectory File (*.traj)\0*.traj\0All files \0*.*\0", filename, nFilterIndex) == S_OK)
			{
				cout << "trying to open \"" << filename << "\"." << endl;
				this->OpenTrajectory(filename);
				// check if there were problems with OpenTrajectory()?!?!?!
				// check if there were problems with OpenTrajectory()?!?!?!
				// check if there were problems with OpenTrajectory()?!?!?!
					
				const char * s1 = "frame(num)"; const char * sv = "distance (nm)";
				plot1d_view * plot = AddPlot1DView(PLOT_USERDATA_STRUCTURE, s1, sv, true);

				float ekin;
				float epot;
				float tmp;


				i32s max_frames = this->GetTotalFrames();
				for (i32s loop = 0;loop < max_frames;loop++)
				{
					this->SetCurrentFrame(loop);
					//this->ReadTrajectoryFrame();
					//void project::ReadTrajectoryFrame(void)
					//{
						i32s place = GetTrajectoryHeaderSize();						// skip the header...
						place += GetTrajectoryFrameSize() * current_traj_frame;		// get the correct frame...
						//place += GetTrajectoryEnergySize();							// skip epot and ekin...
						
						trajfile->seekg(place, ios::beg);

						trajfile->read((char *) & ekin, sizeof(ekin));
						trajfile->read((char *) & epot, sizeof(epot));

		if (trajectory_version > 10)
		{
			float boundary[3];
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[0] = tmp;
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[1] = tmp;
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[2] = tmp;
		}

		i32s ind = 0;
		mt_a1 = mt_a2 = mt_a3 = NULL;

		for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
		{
		//	if ((* it1).flags & ATOMFLAG_IS_HIDDEN) continue;	// currently all coordinates are written...
			float t1a;
			fGL cdata[3];
			for (i32s t4 = 0;t4 < 3;t4++)
			{
				trajfile->read((char *) & t1a, sizeof(t1a));
				cdata[t4] = t1a;
			}

			if (trajectory_version > 12)
			{
				fGL vdata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					vdata[t4] = tmp;
				}
			}

			if (trajectory_version > 13)
			{
				fGL adata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					adata[t4] = tmp;
				}
			}

			if (trajectory_version > 11)
			{
				fGL fdata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					fdata[t4] = tmp;
				}
			}

							(* it1).SetCRD(0, cdata[0], cdata[1], cdata[2]);

							if (ind == ind1)
							{
								mt_a1 = &(* it1);
							}

							if (ind == ind2)
							{
								mt_a2 = &(* it1);
							}

							/*if (ind == indc)
							{
								mt_a3 = &(* it1);
							}*/


							ind++;
						}
						//this->UpdateAllGraphicsViews();
					//}

						fGL coordinate;

						if (mt_a1 && mt_a2 && dim >=0 && dim < 3)
						{
							const fGL * p1 = mt_a1->GetCRD(0);
							const fGL * p2 = mt_a2->GetCRD(0);
// my measure
//cout  << "el = " << mt_a1->el.GetSymbol() << " " << mt_a1->el.GetAtomicNumber() << " x = " << p1[0] << " y = " << p1[1] << " z = " << p1[2] << endl;
							coordinate = p1[dim]-p2[dim];
						}
						else
							coordinate = 0;

					//ref->this->UpdateAllGraphicsViews(true);
					//::Sleep(100);
					void * udata = convert_cset_to_plotting_udata(this, 0);
					f64 value = coordinate;
					plot->AddData(loop, value, udata);

					mt_a1 = mt_a2 = mt_a3 = NULL;
				}
				
				this->CloseTrajectory();

//				static trajview_dialog * tvd = NULL;
				
//				if (tvd != NULL) delete tvd;		// how to safely release the memory...
				//tvd =
				//	new trajview_dialog(this);		// ...right after the dialog is closed?
				
				// the dialog will call this->CloseTrajectory() itself when closed!!!
				// the dialog will call this->CloseTrajectory() itself when closed!!!
				// the dialog will call this->CloseTrajectory() itself when closed!!!
				plot->SetCenterAndScale();
				plot->Update();
			}
		}
		else this->ErrorMessage("Trajectory already open?!?!?!");
	//}
}
struct xyz
{
	f64 x,y,z;
	xyz()
	{
		x = y = z = 0.0;
	}
	xyz(f64 _x, f64 _y, f64 _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
};
void project::TrajView_VeloncityDistribution2D(i32s divx, i32s divy, f64 dt)
{
	dt *= 100 * 1e-15;
	if (!this->GetTrajectoryFile())
	{

		char filename[512];
		DWORD nFilterIndex;
		if (OpenFileDlg(NULL, "Ghemical Trajectory File (*.traj)\0*.traj\0All files \0*.*\0", filename, nFilterIndex) == S_OK)
		{
			cout << "trying to open \"" << filename << "\"." << endl;
			this->OpenTrajectory(filename);
			// check if there were problems with OpenTrajectory()?!?!?!
			// check if there were problems with OpenTrajectory()?!?!?!
			// check if there were problems with OpenTrajectory()?!?!?!
				
			const char * s1 = "tor1(deg)"; const char * s2 = "tor2(deg)"; const char * sv = "E(kJ/mol)";
			plot2d_view * plot = AddPlot2DView(PLOT_USERDATA_STRUCTURE, s1, s2, sv, true);
			
			//const char * s1 = "frame(num)"; const char * sv = "distance (nm)";
			//plot1d_view * plot = AddPlot1DView(PLOT_USERDATA_STRUCTURE, s1, sv, true);
				//const char * s1 = "frame(num)"; const char * sv = "distance (nm)";
				plot1d_view * plot1 = AddPlot1DView(PLOT_USERDATA_STRUCTURE, s1, sv, true);
				plot1d_view * plot11 = AddPlot1DView(PLOT_USERDATA_STRUCTURE, s1, sv, true);

			float ekin;
			float epot;
			float tmp;

			vector<xyz> coordinates_xyz;


			for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
			{
				if (!((* it1).flags & ATOMFLAG_SELECTED)) continue;	
				coordinates_xyz.push_back(xyz());
			}

			vector<xyz> veloncities_xyz;

			xyz mean_traj_coordinate = xyz();

			xyz minvel, maxvel;

			f64 sum_mom_xy = 0.0;

			i32s max_frames = this->GetTotalFrames();
			for (i32s loop = 0;loop < max_frames;loop++)
			{
				this->SetCurrentFrame(loop);
				//this->ReadTrajectoryFrame();
				//void project::ReadTrajectoryFrame(void)
				//{
					i32s place = GetTrajectoryHeaderSize();						// skip the header...
					place += GetTrajectoryFrameSize() * current_traj_frame;		// get the correct frame...
					//place += GetTrajectoryEnergySize();							// skip epot and ekin...
					
					trajfile->seekg(place, ios::beg);

		trajfile->read((char *) & ekin, sizeof(ekin));
		trajfile->read((char *) & epot, sizeof(epot));

		if (trajectory_version > 10)
		{
			float boundary[3];
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[0] = tmp;
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[1] = tmp;
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[2] = tmp;
		}

		i32s ind = 0;
		xyz mean_shift = xyz();
		for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
		{
			float t1a;
			fGL cdata[3];
			for (i32s t4 = 0;t4 < 3;t4++)
			{
				trajfile->read((char *) & t1a, sizeof(t1a));
				cdata[t4] = t1a;
			}

			if (trajectory_version > 12)
			{
				fGL vdata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					vdata[t4] = tmp;
				}
			}

			if (trajectory_version > 13)
			{
				fGL adata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					adata[t4] = tmp;
				}
			}

			if (trajectory_version > 11)
			{
				fGL fdata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					fdata[t4] = tmp;
				}
			}

						(* it1).SetCRD(0, cdata[0], cdata[1], cdata[2]);

						if (!((* it1).flags & ATOMFLAG_SELECTED)) continue;	

						if (loop)
						{
							xyz dist;

							dist.x = cdata[0] - coordinates_xyz[ind].x;
							dist.y = cdata[1] - coordinates_xyz[ind].y;
							dist.z = cdata[2] - coordinates_xyz[ind].z;

							i32s n1 = 0;
							{
								if (dist.x < -this->periodic_box_HALFdim[n1])
								{
									dist.x += 2.0*this->periodic_box_HALFdim[n1];
								}
								else if (dist.x > this->periodic_box_HALFdim[n1])
								{
									dist.x -= 2.0*this->periodic_box_HALFdim[n1];
								}
							}


							f64 vx = dist.x / dt;
							f64 vy = dist.y / dt;
							f64 vz = dist.z / dt;

							//printf("vx = %f vy = %f vz = %f\n", vx,vy,vz);

							veloncities_xyz.push_back(xyz(vx,vy,vz));

							if (minvel.x > vx) minvel.x = vx;
							if (minvel.y > vy) minvel.y = vy;
							if (minvel.z > vz) minvel.z = vz;

							if (maxvel.x < vx) maxvel.x = vx;
							if (maxvel.y < vy) maxvel.y = vy;
							if (maxvel.z < vz) maxvel.z = vz;

							mean_shift.x += dist.x;
							mean_shift.y += dist.y;
							mean_shift.z += dist.z;
						}

						coordinates_xyz[ind].x = cdata[0];
						coordinates_xyz[ind].y = cdata[1];
						coordinates_xyz[ind].z = cdata[2];


						ind++;
					}
					//this->UpdateAllGraphicsViews();

					mean_shift.x /= ind;
					mean_shift.y /= ind;
					mean_shift.z /= ind;

					mean_traj_coordinate.x += mean_shift.x;
					mean_traj_coordinate.y += mean_shift.y;
					mean_traj_coordinate.z += mean_shift.z;
				//}

				// момент против часовой стрелки
				// x*vy-y*vx
				sum_mom_xy +=  
					(mean_traj_coordinate.x * mean_shift.y - 
					mean_traj_coordinate.y * mean_shift.x);

				plot1->AddData(mean_traj_coordinate.x, mean_traj_coordinate.y,  NULL);
				void * udata = convert_cset_to_plotting_udata(this, 0);
				plot11->AddData(loop, sum_mom_xy, udata);
			}
			
			this->CloseTrajectory();

			f64 range_x[2];
			range_x[0] = minvel.x;
			range_x[1] = maxvel.x;

			f64 range_y[2];
			range_y[0] = minvel.y;
			range_y[1] = maxvel.y;

			vector<vector<f64> > m;
			m.resize(divx);

			for (i32s sx = 0; sx < divx; sx++)
			{
				m[sx].resize(divy, 0.0);
			}


			for (vector<xyz>::iterator it_vel = veloncities_xyz.begin();
				it_vel != veloncities_xyz.end(); it_vel++)
			{

				i32s sx = i32s((((*it_vel).x - minvel.x) / (maxvel.x - minvel.x)) * f64(divx));
				if (sx == divx) sx--;

				i32s sy = i32s((((*it_vel).y - minvel.y) / (maxvel.y - minvel.y)) * f64(divy));
				if (sy == divy) sy--;

						
				
				//printf("(*it_vel).x = %f (*it_vel).y = %f\n", (*it_vel).x,(*it_vel).y);
				//printf("sx = %d sy = %d\n", sx,sy);
				

				m[sx][sy] += 1.0;			
			}


			f64 vx = range_x[0];

			for (i32s sx = 0; sx < divx; sx++)
			{
				f64 vy = range_y[0];
				for (i32s sy = 0; sy < divy; sy++)
				{

					// ...and add it to the plot.			
					void * udata = NULL;// convert_cset_to_plotting_udata(this, 0);
					plot->AddData(vx, vy, m[sx][sy], udata);
					//printf("vx = %f, vy = %f, m[sx][sy] = %f\n", vx, vy, m[sx][sy]);
					vy += (range_y[1] - range_y[0]) / (f64) divy;
				}
				
				vx += (range_x[1] - range_x[0]) / (f64) divx;
			}


			
			// the "eng" object is the setup->current_eng object, so there's no need to delete it...
			
			plot1->SetCenterAndScale();
			plot1->Update();
			
			plot11->SetCenterAndScale();
			plot11->Update();
			
			plot->SetCenterAndScale();
			plot->Update();

		}
	}
	else this->ErrorMessage("Trajectory already open?!?!?!");
}


#include "ap.h"
#include "float.h"

bool jacobyeigenvaluesandvectors(ap::real_2d_array a,
     int n,
     ap::real_1d_array& d,
     ap::real_2d_array& v);

bool jacobyeigenvaluesandvectors(ap::real_2d_array a,
     int n,
     ap::real_1d_array& d,
     ap::real_2d_array& v)
{
    bool result;
    int j;
    int iq;
    int ip;
    int i;
    double tresh;
    double theta;
    double tau;
    double t;
    double sm;
    double s;
    double h;
    double g;
    double c;
    ap::real_1d_array b;
    ap::real_1d_array z;

    d.setbounds(1, n);
    v.setbounds(1, n, 1, n);
    b.setbounds(1, n);
    z.setbounds(1, n);
    result = true;
    for(ip = 1; ip <= n; ip++)
    {
        for(iq = 1; iq <= n; iq++)
        {
            v(ip,iq) = 0.0;
        }
        v(ip,ip) = 1.0;
    }
    for(ip = 1; ip <= n; ip++)
    {
        b(ip) = a(ip,ip);
        d(ip) = b(ip);
        z(ip) = 0.0;
    }
    for(i = 1; i <= 50; i++)
    {
        result = i!=50;
        sm = 0.0;
        for(ip = 1; ip <= n-1; ip++)
        {
            for(iq = ip+1; iq <= n; iq++)
            {
                sm = sm+fabs(a(ip,iq));
            }
        }
        if( sm==0.0||!result )
        {
            return result;
        }
        if( i<4 )
        {
            tresh = 0.2*sm/ap::sqr(n);
        }
        else
        {
            tresh = 0.0;
        }
        for(ip = 1; ip <= n-1; ip++)
        {
            for(iq = ip+1; iq <= n; iq++)
            {
                g = fabs(a(ip,iq));
                if( i>4&&100*fabs(d(ip))*ap::machineepsilon>=g&&100*fabs(d(iq))*ap::machineepsilon>=g )
                {
                    a(ip,iq) = 0.0;
                }
                else
                {
                    if( fabs(a(ip,iq))>tresh )
                    {
                        h = d(iq)-d(ip);
                        if( 100*fabs(h)*ap::machineepsilon>=g )
                        {
                            t = a(ip,iq)/h;
                        }
                        else
                        {
                            theta = 0.5*h/a(ip,iq);
                            t = 1.0/(fabs(theta)+sqrt(1.0+ap::sqr(theta)));
                            if( theta<0.0 )
                            {
                                t = -t;
                            }
                        }
                        c = 1.0/sqrt(1+ap::sqr(t));
                        s = t*c;
                        tau = s/(1.0+c);
                        h = t*a(ip,iq);
                        z(ip) = z(ip)-h;
                        z(iq) = z(iq)+h;
                        d(ip) = d(ip)-h;
                        d(iq) = d(iq)+h;
                        a(ip,iq) = 0.0;
                        for(j = 1; j <= ip-1; j++)
                        {
                            g = a(j,ip);
                            h = a(j,iq);
                            a(j,ip) = g-s*(h+g*tau);
                            a(j,iq) = h+s*(g-h*tau);
                        }
                        for(j = ip+1; j <= iq-1; j++)
                        {
                            g = a(ip,j);
                            h = a(j,iq);
                            a(ip,j) = g-s*(h+g*tau);
                            a(j,iq) = h+s*(g-h*tau);
                        }
                        for(j = iq+1; j <= n; j++)
                        {
                            g = a(ip,j);
                            h = a(iq,j);
                            a(ip,j) = g-s*(h+g*tau);
                            a(iq,j) = h+s*(g-h*tau);
                        }
                        for(j = 1; j <= n; j++)
                        {
                            g = v(j,ip);
                            h = v(j,iq);
                            v(j,ip) = g-s*(h+g*tau);
                            v(j,iq) = h+s*(g-h*tau);
                        }
                    }
                }
            }
        }
    }
    for(ip = 1; ip <= n; ip++)
    {
        b(ip) = b(ip)+z(ip);
        d(ip) = b(ip);
        z(ip) = 0.0;
    }
    return result;
}

struct mol_axis_vector
{
	fGL e_mlax[3];
	fGL len;
};

void project::TrajView_NematicCoordinatePlot(i32s _type, i32s _dim)
{
	//win_graphics_view * gv = win_graphics_view::GetGV(widget);
	//win_project * prj = dynamic_cast<win_project *>(gv->prj);
	//if (prj)// new trajfile_dialog(prj);	// will call delete itself...
	//{
		if (!this->GetTrajectoryFile())
		{

			char filename[512];
			DWORD nFilterIndex;
			if (OpenFileDlg(NULL, "Ghemical Trajectory File (*.traj)\0*.traj\0All files \0*.*\0", filename, nFilterIndex) == S_OK)
			{
				cout << "trying to open \"" << filename << "\"." << endl;
				this->OpenTrajectory(filename);
				// check if there were problems with OpenTrajectory()?!?!?!
				// check if there were problems with OpenTrajectory()?!?!?!
				// check if there were problems with OpenTrajectory()?!?!?!
					
				const char * s1 = "frame(num)"; const char * sv = "distance (nm)";
				plot1d_view * plot = AddPlot1DView(PLOT_USERDATA_STRUCTURE, s1, sv, true);

				float ekin;
				float epot;
				float tmp;


				i32s max_frames = this->GetTotalFrames();
				for (i32s loop = 0;loop < max_frames;loop++)
				{
					this->SetCurrentFrame(loop);
					//this->ReadTrajectoryFrame();
					//void project::ReadTrajectoryFrame(void)
					//{
						i32s place = GetTrajectoryHeaderSize();						// skip the header...
						place += GetTrajectoryFrameSize() * current_traj_frame;		// get the correct frame...
						//place += GetTrajectoryEnergySize();							// skip epot and ekin...
						
						trajfile->seekg(place, ios::beg);

						trajfile->read((char *) & ekin, sizeof(ekin));
						trajfile->read((char *) & epot, sizeof(epot));

		if (trajectory_version > 10)
		{
			float boundary[3];
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[0] = tmp;
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[1] = tmp;
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[2] = tmp;
		}

						i32s ind = 0;
						mt_a1 = mt_a2 = mt_a3 = NULL;	


						vector<mol_axis_vector> vmav;

						vmav.clear();




						for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
						{
						//	if ((* it1).flags & ATOMFLAG_IS_HIDDEN) continue;	// currently all coordinates are written...
							float t1a;
							fGL cdata[3];
							for (i32s t4 = 0;t4 < 3;t4++)
							{
								trajfile->read((char *) & t1a, sizeof(t1a));
								cdata[t4] = t1a;
							}

			if (trajectory_version > 12)
			{
				fGL vdata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					vdata[t4] = tmp;
				}
			}

			if (trajectory_version > 13)
			{
				fGL adata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					adata[t4] = tmp;
				}
			}

			if (trajectory_version > 11)
			{
				fGL fdata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					fdata[t4] = tmp;
				}
			}

							(* it1).SetCRD(0, cdata[0], cdata[1], cdata[2]);

#if 0						
							if (ind == ind_)
							{
								mt_a1 = &(* it1);
							}
#endif

							/*if (ind == indb)
							{
								mt_a2 = &(* it1);
							}

							if (ind == indc)
							{
								mt_a3 = &(* it1);
							}*/


							ind++;
						}
						//this->UpdateAllGraphicsViews();
					//}

						//Allen, Tildesley. Computer simulations of liquids p 305 (11.19)
						ap::real_2d_array Q;
						Q.setbounds(1,3,1,3);
						for (i32s n1 = 1; n1 <= 3; n1++)
						{
							for (i32s n2 = 1; n2 <= 3; n2++)
							{
								Q(n1,n2) = 0.0;
							}
						}
						for (list<molecular_axis>::iterator it = this->molaxis_list.begin();
							it != this->molaxis_list.end(); it++)
						{	
							const fGL * p1 = (*it).atmr[0]->GetCRD(0);
							const fGL * p2 = (*it).atmr[1]->GetCRD(0);

							mol_axis_vector mav;



							

							for (i32s n1 = 0; n1 < 3; n1++)
							{
								mav.e_mlax[n1] = p2[n1] - p1[n1];
								if (n1 == 0)
								{
									if (mav.e_mlax[n1] < -this->periodic_box_HALFdim[n1])
									{
										mav.e_mlax[n1] += 2.0*this->periodic_box_HALFdim[n1];
									}
									else if (mav.e_mlax[n1] > this->periodic_box_HALFdim[n1])
									{
										mav.e_mlax[n1] -= 2.0*this->periodic_box_HALFdim[n1];
									}
								}
							}

							mav.len = 0.0;
							for (i32s n1 = 0; n1 < 3; n1++)
							{
								mav.len += mav.e_mlax[n1] * mav.e_mlax[n1];
							}
							mav.len = sqrt(mav.len);
							vmav.push_back(mav);

							for (i32s n1 = 1; n1 <= 3; n1++)
							{
								for (i32s n2 = 1; n2 <= 3; n2++)
								{
									Q(n1,n2) += 3.0 * mav.e_mlax[n1-1]*mav.e_mlax[n2-1] - 1.0 * (n1 == n2);
								}
							}
						}
						int two_N = 2.0*this->molaxis_list.size();
						for (i32s n1 = 1; n1 <= 3; n1++)
						{
							for (i32s n2 = 1; n2 <= 3; n2++)
							{
								Q(n1,n2) /= two_N;
							}
						}

						int N = vmav.size();
						f64 sumP2 = 0.0;
						int nn = 0;
						for (i32s i1 = 0; i1 < N; i1++)
						{
							for (i32s i2 = i1+1; i2 < N; i2++)
							{
								if (i1 != i2)
								{
									f64 scalar_product = 0.0;
									for (i32s n1 = 0; n1 < 3; n1++)
									{
										scalar_product += 
											vmav[i1].e_mlax[n1] * vmav[i2].e_mlax[n1];
									}
									f64 cosine = scalar_product / (vmav[i1].len * vmav[i2].len);
							//printf("cosine(%d,%d) = %f\n",i1,i2, cosine);
									f64 P2 = 0.5 * (3.0 * cosine * cosine - 1);
									sumP2 += P2;
									nn++;
								}
							}
						}
						f64 meanP2 = sumP2 / nn;


						//собственные значения и собственные вектора
						ap::real_1d_array d;
						ap::real_2d_array v;

						fGL coordinate;
						if (false)
						{
							coordinate = -DBL_MAX;

							if (jacobyeigenvaluesandvectors(Q, 3, d, v))
							{

								//printf("d(1) = %f, d(2) = %f, d(3) = %f\n", d(1), d(2), d(3));
								for (i32s n1 = 1; n1 <= 3; n1++)	
								{
									if (coordinate < d(n1))
										coordinate = d(n1);
								}
							}
						}
						else
						{
							coordinate = meanP2;
						}

#if 0						
						if (mt_a1 && dim >=0 && dim < 3)
						{
							const fGL * p1 = mt_a1->GetCRD(0);
							//const fGL * p2 = mt_a2->GetCRD(0);
// my measure
//cout  << "el = " << mt_a1->el.GetSymbol() << " " << mt_a1->el.GetAtomicNumber() << " x = " << p1[0] << " y = " << p1[1] << " z = " << p1[2] << endl;
							coordinate = p1[dim];
						}
						else
							coordinate = 0;
#endif
					//ref->this->UpdateAllGraphicsViews(true);
					//::Sleep(100);
					void * udata = convert_cset_to_plotting_udata(this, 0);
					f64 value = coordinate;
					plot->AddData(loop, value, udata);

					mt_a1 = mt_a2 = mt_a3 = NULL;
				}
				
				this->CloseTrajectory();

//				static trajview_dialog * tvd = NULL;
				
//				if (tvd != NULL) delete tvd;		// how to safely release the memory...
				//tvd =
				//	new trajview_dialog(this);		// ...right after the dialog is closed?
				
				// the dialog will call this->CloseTrajectory() itself when closed!!!
				// the dialog will call this->CloseTrajectory() itself when closed!!!
				// the dialog will call this->CloseTrajectory() itself when closed!!!
				plot->SetCenterAndScale();
				plot->Update();
			}
		}
		else this->ErrorMessage("Trajectory already open?!?!?!");
	//}
}



void project::TrajView_DistancePlot(i32s inda, i32s indb)
{
	//win_graphics_view * gv = win_graphics_view::GetGV(widget);
	//win_project * prj = dynamic_cast<win_project *>(gv->prj);
	//if (prj)// new trajfile_dialog(prj);	// will call delete itself...
	//{
		if (!this->GetTrajectoryFile())
		{

			char filename[512];
			DWORD nFilterIndex;
			if (OpenFileDlg(NULL, "Ghemical Trajectory File (*.traj)\0*.traj\0All files \0*.*\0", filename, nFilterIndex) == S_OK)
			{
				cout << "trying to open \"" << filename << "\"." << endl;
				this->OpenTrajectory(filename);
				// check if there were problems with OpenTrajectory()?!?!?!
				// check if there were problems with OpenTrajectory()?!?!?!
				// check if there were problems with OpenTrajectory()?!?!?!
					
				const char * s1 = "frame(num)"; const char * sv = "distance (nm)";
				plot1d_view * plot = AddPlot1DView(PLOT_USERDATA_STRUCTURE, s1, sv, true);

				float ekin;
				float epot;
				float tmp;


				i32s max_frames = this->GetTotalFrames();
				for (i32s loop = 0;loop < max_frames;loop++)
				{
					this->SetCurrentFrame(loop);
					//this->ReadTrajectoryFrame();
					//void project::ReadTrajectoryFrame(void)
					//{
						i32s place = GetTrajectoryHeaderSize();						// skip the header...
						place += GetTrajectoryFrameSize() * current_traj_frame;		// get the correct frame...
						//place += GetTrajectoryEnergySize();							// skip epot and ekin...
						
						trajfile->seekg(place, ios::beg);

						trajfile->read((char *) & ekin, sizeof(ekin));
						trajfile->read((char *) & epot, sizeof(epot));

		if (trajectory_version > 10)
		{
			float boundary[3];
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[0] = tmp;
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[1] = tmp;
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[2] = tmp;
		}

						i32s ind = 0;
						mt_a1 = mt_a2 = mt_a3 = NULL;



						for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
						{
						//	if ((* it1).flags & ATOMFLAG_IS_HIDDEN) continue;	// currently all coordinates are written...
							float t1a;
							fGL cdata[3];
							for (i32s t4 = 0;t4 < 3;t4++)
							{
								trajfile->read((char *) & t1a, sizeof(t1a));
								cdata[t4] = t1a;
							}

			if (trajectory_version > 12)
			{
				fGL vdata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					vdata[t4] = tmp;
				}
			}

			if (trajectory_version > 13)
			{
				fGL adata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					adata[t4] = tmp;
				}
			}

			if (trajectory_version > 11)
			{
				fGL fdata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					fdata[t4] = tmp;
				}
			}

							(* it1).SetCRD(0, cdata[0], cdata[1], cdata[2]);

							if (ind == inda)
							{
								mt_a1 = &(* it1);
							}

							if (ind == indb)
							{
								mt_a2 = &(* it1);
							}

							/*if (ind == indc)
							{
								mt_a3 = &(* it1);
							}*/


							ind++;
						}
						//this->UpdateAllGraphicsViews();
					//}

						fGL dist;

						if (mt_a1 && mt_a2)
						{
							const fGL * p1 = mt_a1->GetCRD(0);
							const fGL * p2 = mt_a2->GetCRD(0);
// my measure
cout  << "el = " << mt_a1->el.GetSymbol() << " " << mt_a1->el.GetAtomicNumber() << " x = " << p1[0] << " y = " << p1[1] << " z = " << p1[2] << endl;
cout  << "el = " << mt_a2->el.GetSymbol() << " " << mt_a2->el.GetAtomicNumber() << " x = " << p2[0] << " y = " << p2[1] << " z = " << p2[2] << endl;
							v3d<fGL> v1(p2, p1);
							dist = v1.len();
						}
						else
							dist = 0;

					//ref->this->UpdateAllGraphicsViews(true);
					//::Sleep(100);
					void * udata = convert_cset_to_plotting_udata(this, 0);
					f64 value = dist;
					plot->AddData(loop, value, udata);

					mt_a1 = mt_a2 = mt_a3 = NULL;
				}
				
				this->CloseTrajectory();

//				static trajview_dialog * tvd = NULL;
				
//				if (tvd != NULL) delete tvd;		// how to safely release the memory...
				//tvd =
				//	new trajview_dialog(this);		// ...right after the dialog is closed?
				
				// the dialog will call this->CloseTrajectory() itself when closed!!!
				// the dialog will call this->CloseTrajectory() itself when closed!!!
				// the dialog will call this->CloseTrajectory() itself when closed!!!
				plot->SetCenterAndScale();
				plot->Update();
			}
		}
		else this->ErrorMessage("Trajectory already open?!?!?!");
	//}
}



void project::TrajView_AnglePlot(i32s inda, i32s indb, i32s indc)
{
	//win_graphics_view * gv = win_graphics_view::GetGV(widget);
	//win_project * prj = dynamic_cast<win_project *>(gv->prj);
	//if (prj)// new trajfile_dialog(prj);	// will call delete itself...
	//{
		if (!this->GetTrajectoryFile())
		{

			char filename[512];
			DWORD nFilterIndex;
			if (OpenFileDlg(NULL, "Ghemical Trajectory File (*.traj)\0*.traj\0All files \0*.*\0", filename, nFilterIndex) == S_OK)
			{
				cout << "trying to open \"" << filename << "\"." << endl;
				this->OpenTrajectory(filename);
				// check if there were problems with OpenTrajectory()?!?!?!
				// check if there were problems with OpenTrajectory()?!?!?!
				// check if there were problems with OpenTrajectory()?!?!?!
					
				const char * s1 = "frame(num)"; const char * sv = "angle (degree)";
				plot1d_view * plot = AddPlot1DView(PLOT_USERDATA_STRUCTURE, s1, sv, true);

				float ekin;
				float epot;
				float tmp;


				i32s max_frames = this->GetTotalFrames();
				for (i32s loop = 0;loop < max_frames;loop++)
				{
					this->SetCurrentFrame(loop);
					//this->ReadTrajectoryFrame();
					//void project::ReadTrajectoryFrame(void)
					//{
						i32s place = GetTrajectoryHeaderSize();						// skip the header...
						place += GetTrajectoryFrameSize() * current_traj_frame;		// get the correct frame...
						//place += GetTrajectoryEnergySize();							// skip epot and ekin...
						
						trajfile->seekg(place, ios::beg);

						trajfile->read((char *) & ekin, sizeof(ekin));
						trajfile->read((char *) & epot, sizeof(epot));

		if (trajectory_version > 10)
		{
			float boundary[3];
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[0] = tmp;
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[1] = tmp;
			trajfile->read((char *) & tmp, sizeof(tmp)); boundary[2] = tmp;
		}

		i32s ind = 0;
		mt_a1 = mt_a2 = mt_a3 = NULL;

		for (iter_al it1 = atom_list.begin();it1 != atom_list.end();it1++)
		{
		//	if ((* it1).flags & ATOMFLAG_IS_HIDDEN) continue;	// currently all coordinates are written...
			float t1a;
			fGL cdata[3];
			for (i32s t4 = 0;t4 < 3;t4++)
			{
				trajfile->read((char *) & t1a, sizeof(t1a));
				cdata[t4] = t1a;
			}

			if (trajectory_version > 12)
			{
				fGL vdata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					vdata[t4] = tmp;
				}
			}

			if (trajectory_version > 13)
			{
				fGL adata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					adata[t4] = tmp;
				}
			}

			if (trajectory_version > 11)
			{
				fGL fdata[3];
				for (i32s t4 = 0; t4 < 3; t4++)
				{
					trajfile->read((char *) & tmp, sizeof(tmp));
					fdata[t4] = tmp;
				}
			}

			(* it1).SetCRD(0, cdata[0], cdata[1], cdata[2]);

							if (ind == inda)
							{
								mt_a1 = &(* it1);
							}

							if (ind == indb)
							{
								mt_a2 = &(* it1);
							}

							if (ind == indc)
							{
								mt_a3 = &(* it1);
							}


							ind++;
						}
						//this->UpdateAllGraphicsViews();
					//}

						fGL ang;

						if (mt_a1 && mt_a2 && mt_a3)
						{
							const fGL * p1 = mt_a1->GetCRD(0);
							const fGL * p2 = mt_a2->GetCRD(0);
							const fGL * p3 = mt_a3->GetCRD(0);			
// my measure
cout  << "el = " << mt_a1->el.GetSymbol() << " " << mt_a1->el.GetAtomicNumber() << " x = " << p1[0] << " y = " << p1[1] << " z = " << p1[2] << endl;
cout  << "el = " << mt_a2->el.GetSymbol() << " " << mt_a2->el.GetAtomicNumber() << " x = " << p2[0] << " y = " << p2[1] << " z = " << p2[2] << endl;
cout  << "el = " << mt_a3->el.GetSymbol() << " " << mt_a3->el.GetAtomicNumber() << " x = " << p3[0] << " y = " << p3[1] << " z = " << p3[2] << endl;
							v3d<fGL> v1(p2, p1);
							v3d<fGL> v2(p2, p3);
							ang = v1.ang(v2) * 180.0 / M_PI;
						}
						else
							ang = 0;

					//ref->this->UpdateAllGraphicsViews(true);
					//::Sleep(100);
					void * udata = convert_cset_to_plotting_udata(this, 0);
					f64 value = ang;
					plot->AddData(loop, value, udata);

					mt_a1 = mt_a2 = mt_a3 = NULL;
				}
				
				this->CloseTrajectory();

//				static trajview_dialog * tvd = NULL;
				
//				if (tvd != NULL) delete tvd;		// how to safely release the memory...
				//tvd =
				//	new trajview_dialog(this);		// ...right after the dialog is closed?
				
				// the dialog will call this->CloseTrajectory() itself when closed!!!
				// the dialog will call this->CloseTrajectory() itself when closed!!!
				// the dialog will call this->CloseTrajectory() itself when closed!!!
				plot->SetCenterAndScale();
				plot->Update();
			}
		}
		else this->ErrorMessage("Trajectory already open?!?!?!");
	//}
}



void project::DoEnergyPlot1D(i32s inda, i32s indb, i32s indc, i32s div1, fGL start1, fGL end1, i32s optsteps, char *filename)
{
	// 2003-11-17 : for IC modification and structure refinement, make a temporary molecular mechanics model.
	
	i32s curr_eng_index = 0;	// if current setup is molecular mechanics, get the eng class...
	setup * tmpsu1 = GetCurrentSetup(); setup1_mm * tmpsu2 = dynamic_cast<setup1_mm *>(tmpsu1);
	if (tmpsu2 != NULL) curr_eng_index = GetCurrentSetup()->GetCurrEngIndex();
	
	model * tmpmdl = new model();	// the default setup here will be molecular mechanics!
	vector<atom *> av; vector<atom *> av_tmp;
	
	for (iter_al it1 = GetAtomsBegin();it1 != GetAtomsEnd();it1++)
	{
		atom newatm((* it1).el, (* it1).GetCRD(0), tmpmdl->GetCRDSetCount());
		tmpmdl->AddAtom(newatm);
		
		av.push_back(& (* it1));
		av_tmp.push_back(& tmpmdl->GetLastAtom());
	}
	{
	for (iter_bl it1 = GetBondsBegin();it1 != GetBondsEnd();it1++)
	{
		i32u ind1 = 0;
		while (ind1 < av.size() && av[ind1] != (* it1).atmr[0]) ind1++;
		if (ind1 == av.size()) { cout << "fatal error! atom #1 not found." << endl; exit(EXIT_FAILURE); }
		
		i32u ind2 = 0;
		while (ind2 < av.size() && av[ind2] != (* it1).atmr[1]) ind2++;
		if (ind2 == av.size()) { cout << "fatal error! atom #2 not found." << endl; exit(EXIT_FAILURE); }
		
		bond newbnd(av_tmp[ind1], av_tmp[ind2], (* it1).bt);
		tmpmdl->AddBond(newbnd);
	}
	}

#if USE_ORIGINAL_ENGINE_SELECTION
	//#####################################################################
	// original code 
	//#####################################################################
	engine * tmpeng = tmpmdl->GetCurrentSetup()->CreateEngineByIndex(curr_eng_index);
	//#####################################################################
#else
	//test code
	engine * tmpeng = tmpmdl->GetCurrentSetup()->CreateEngineByIDNumber(CURRENT_ENG1_MM);
#endif
	
	// the temporary model is now ok, continue...
	
#if USE_ORIGINAL_ENGINE_SELECTION
	//#####################################################################
	// original code 
	//#####################################################################
	engine * eng = GetCurrentSetup()->GetCurrentEngine();
	if (!eng) eng = GetCurrentSetup()->CreateEngineByIndex(GetCurrentSetup()->GetCurrEngIndex());
	//#####################################################################
#else
	//test code
	engine * eng = GetCurrentSetup()->CreateEngineByIDNumber(CURRENT_ENG1_MM);
#endif
	
	i32s molnum = 0; i32s in_crdset = 0;
	
	i32s atmi1[3] = { inda, indb, indc};
	atom * atmr1[3]; f64 range1[2];
	range1[0] = M_PI * start1 / 180.0;
	range1[1] = M_PI * end1 / 180.0;

	for (i32s n1 = 0; n1 < 3; n1++)
	{
		iter_al it1;
		
		it1 = tmpmdl->FindAtomByIndex(atmi1[n1]);
		if (it1 == tmpmdl->GetAtomsEnd()) { PrintToLog("ERROR : atom not found!\n"); return; }
		atmr1[n1] = & (* it1);
	}
	
// must call SortGroups() here because intcrd needs it ; however that might change the indices?!?!?! note that we convert to pointers above...
// must call SortGroups() here because intcrd needs it ; however that might change the indices?!?!?! note that we convert to pointers above...
// must call SortGroups() here because intcrd needs it ; however that might change the indices?!?!?! note that we convert to pointers above...
//	if (!tmpmdl->IsGroupsClean()) tmpmdl->UpdateGroups();		// for internal coordinates...
//	if (!tmpmdl->IsGroupsSorted()) tmpmdl->SortGroups(true);	// for internal coordinates...
	
	//intcrd * tmpic = new intcrd((* tmpmdl), molnum, in_crdset);
	//i32s ict1 = tmpic->FindTorsion(atmr1[1], atmr1[2]);
//if (ict1 < 0) { PrintToLog("ERROR : could not find ic.\n"); return; }
	
	v3d<fGL> v1a(atmr1[1]->GetCRD(in_crdset), atmr1[0]->GetCRD(in_crdset));
	v3d<fGL> v1b(atmr1[1]->GetCRD(in_crdset), atmr1[2]->GetCRD(in_crdset));
//	v3d<fGL> v1c(atmr1[2]->GetCRD(in_crdset), atmr1[3]->GetCRD(in_crdset));
//	f64 oldt1 = v1a.tor(v1b, v1c);

	f64 oldang = v1a.ang(v1b);
	fGL len =  v1a.len();

	// ось вдоль начального направления исследуемой связи
	v3d<fGL> v1(atmr1[1]->GetCRD(in_crdset), atmr1[0]->GetCRD(in_crdset));
	// ось вращения
	v3d<fGL> v2 = v1.vpr(v1b);
	// ось проекции
	v3d<fGL> v3 = v1.vpr(v2);

	printf("v1 = [%f %f %f]\n", v1.data[0],v1.data[1],v1.data[2]); 
	printf("v2 = [%f %f %f]\n", v2.data[0],v2.data[1],v2.data[2]); 
	printf("v3 = [%f %f %f]\n", v3.data[0],v3.data[1],v3.data[2]); 

	//переводим в единичные векторы - имеем ортогональный базис внутренних координат
	v1 = v1 / v1.len();
	v2 = v2 / v2.len();
	v3 = v3 / v3.len();


	printf("v1 = [%f %f %f]\n", v1.data[0],v1.data[1],v1.data[2]); 
	printf("v2 = [%f %f %f]\n", v2.data[0],v2.data[1],v2.data[2]); 
	printf("v3 = [%f %f %f]\n", v3.data[0],v3.data[1],v3.data[2]); 

		// compute energy for final structure...
		
		f64 value;
		CopyCRD(this, eng, 0);
		eng->Compute(0); value = eng->energy;


	cout << "oldang = " << (180.0 * oldang / M_PI) << " deg, energy = " << value << " kJ/mol." << endl << ends;

//	i32s ffa1 = tmpeng->FindAngle(atmr1[0], atmr1[1], atmr1[2]);
//if (ffa1 < 0) { PrintToLog("ERROR : could not find angle-term.\n"); return; }
	
	const char * s1 = "ang(deg)"; const char * sv = "E(kJ/mol)";
	plot1d_view * plot = AddPlot1DView(PLOT_USERDATA_STRUCTURE, s1, sv, true);

	FILE *stream = NULL;
	if (filename) stream = fopen(filename,"wt");
	int delim = '\t';
	if(stream)
	{
		fprintf(stream, 
			"\"%s\"%c"
			"\"%s\"%c"
			"\"%s\"%c"
			"\"%s\"%c"

			"\"%s\"%c"
			"\"%s\"%c"
			"\"%s\"%c"
			"\"%s\"%c"

			"\"%s\"%c"
			"\"%s\"%c"
			"\"%s\"%c"
			"\"%s\""
			,
			"Angle", delim,
			"Energy kJ/mol", delim, 
			"BondedTermsEnergy", delim, 
			"NonBondedTermsEnergy", delim, 

			"BondStretchEnergy", delim, 
			"AngleStretchEnergy", delim, 
			"TorsionEnergy", delim, 
			"OutOfPlaneEnergy", delim, 

			"DispersionEnergy", delim, 
			"ElectrostaticEnergy", delim, 
			"NonBondedEnergyC", delim, 
			"NonBondedEnergyD"

			);

		eng->WriteAnglesHeader(stream, delim);
		fprintf(stream, "\n");


	}

	f64 ang1 = range1[0];
	{
	for (i32s s1 = 0; s1 < (div1 + 1);s1++)
	{
		//tmpic->SetTorsion(ict1, tor1 - oldt1);
		//tmpic->UpdateCartesian();
		{
			// вычисляем проекции текущего направления исследуемой связи на наш ортогональный базис внутренних координат 
			v3d<fGL> V1 = v1 * (len * cos(ang1));
			v3d<fGL> V2 = v2 * 0.0;
			v3d<fGL> V3 = v3 * (len * sin(ang1));

            // складываем проециии в результирующий вектор			
			v3d<fGL> tmpvC = ((V1 + V2) + V3);

			// координата атома, который является осью вращения связи			
			const fGL * cdata = atmr1[1]->GetCRD(in_crdset);
			// вычисляем координаты конца исследуемой связи
			fGL x = cdata[0] + tmpvC[0];
			fGL y = cdata[1] + tmpvC[1];
			fGL z = cdata[2] + tmpvC[2];

			// заносим эти координаты в двигающийся атом			
			atmr1[0]->SetCRD(in_crdset, x, y, z);
		}
#if 0
		CopyCRD(tmpmdl, tmpeng, 0);					// lock_local_structure needs coordinates!!!
		//tmpeng->SetAngleConstraint(ffa1, ang1, 5000.0, true);
		
		// optimize...
		
		geomopt * opt = new geomopt(tmpeng, 100, 0.025, 10.0);		// optimal settings?!?!?
		
		for (i32s n1 = 0;n1 < optsteps;n1++)
		{
			opt->TakeCGStep(conjugate_gradient::Newton2An);
			if (!(n1 % 50)) cout << n1 << " " << opt->optval << " " << opt->optstp << endl;
		}
		
		CopyCRD(tmpeng, tmpmdl, 0);
		tmpmdl->CenterCRDSet(0, true);
		delete opt;
		
#endif
		for (i32u n1 = 0;n1 < av_tmp.size();n1++)
		{
			const fGL * tmpcrd = av_tmp[n1]->GetCRD(0);
			av[n1]->SetCRD(0, tmpcrd[0], tmpcrd[1], tmpcrd[2]);
		}
		// compute energy for final structure...
		
		f64 value;
		CopyCRD(this, eng, 0);
		eng->Compute(0); value = eng->energy;

		eng1_mm * eng_mm = dynamic_cast<eng1_mm *>(eng);

		if(eng_mm && stream)
		{
			fprintf(stream, 
				"%f%c"
				"%f%c"
				"%f%c"
				"%f%c"

				"%f%c"
				"%f%c"
				"%f%c"
				"%f%c"

				"%f%c"
				"%f%c"
				"%f%c"
				"%f"
				,
				(180.0 * ang1 / M_PI), delim,
				value, delim, 
				eng_mm->GetBondedTermsEnergy(), delim, 
				eng_mm->GetNonBondedTermsEnergy(), delim, 

				eng_mm->GetBondStretchEnergy(), delim, 
				eng_mm->GetAngleStretchEnergy(), delim, 
				eng_mm->GetTorsionEnergy(), delim, 
				eng_mm->GetOutOfPlaneEnergy(), delim, 

				eng_mm->GetDispersionEnergy(), delim, 
				eng_mm->GetElectrostaticEnergy(), delim, 
				eng_mm->GetNonBondedEnergyC(), delim, 
				eng_mm->GetNonBondedEnergyD()

				);

			eng->WriteAngles(stream, delim);
			fprintf(stream, "\n");

		}

		
		// ...and add it to the plot.
		
		void * udata = convert_cset_to_plotting_udata(this, 0);
		plot->AddData(180.0 * (ang1-range1[0]) / M_PI, value, udata);
		
		char mbuff1[256];
		strstream str1(mbuff1, sizeof(mbuff1));
		str1 << "ang = " << (180.0 * ang1 / M_PI) << " deg, energy = " << value << " kJ/mol." << endl << ends;
		PrintToLog(mbuff1);
		
		ang1 += (range1[1] - range1[0]) / (f64) div1;
	}
	}

	if (stream) fclose(stream);
	//delete tmpic;
	delete tmpeng;
	delete tmpmdl;
	
	// the "eng" object is the setup->current_eng object, so there's no need to delete it...
	
	plot->SetCenterAndScale();
	plot->Update();
}


void project::DoEnergyPlot1D(i32s inda, i32s indb, i32s indc, i32s indd, i32s div1, fGL start1, fGL end1, i32s optsteps)
{
printf("project::DoEnergyPlot1D 0\n");
	// 2003-11-17 : for IC modification and structure refinement, make a temporary molecular mechanics model.
	
	i32s curr_eng_index = 0;	// if current setup is molecular mechanics, get the eng class...
	setup * tmpsu1 = GetCurrentSetup(); setup1_mm * tmpsu2 = dynamic_cast<setup1_mm *>(tmpsu1);
	if (tmpsu2 != NULL) curr_eng_index = GetCurrentSetup()->GetCurrEngIndex();
	
	model * tmpmdl = new model();	// the default setup here will be molecular mechanics!
	vector<atom *> av; vector<atom *> av_tmp;
	
	for (iter_al it1 = GetAtomsBegin();it1 != GetAtomsEnd();it1++)
	{
		atom newatm((* it1).el, (* it1).GetCRD(0), tmpmdl->GetCRDSetCount());
		tmpmdl->AddAtom(newatm);
		
		av.push_back(& (* it1));
		av_tmp.push_back(& tmpmdl->GetLastAtom());
	}
printf("project::DoEnergyPlot1D 1\n");
	{
	for (iter_bl it1 = GetBondsBegin();it1 != GetBondsEnd();it1++)
	{
		i32u ind1 = 0;
		while (ind1 < av.size() && av[ind1] != (* it1).atmr[0]) ind1++;
		if (ind1 == av.size()) { cout << "fatal error! atom #1 not found." << endl; exit(EXIT_FAILURE); }
		
		i32u ind2 = 0;
		while (ind2 < av.size() && av[ind2] != (* it1).atmr[1]) ind2++;
		if (ind2 == av.size()) { cout << "fatal error! atom #2 not found." << endl; exit(EXIT_FAILURE); }
		
		bond newbnd(av_tmp[ind1], av_tmp[ind2], (* it1).bt);
		tmpmdl->AddBond(newbnd);
	}
	}
	engine * tmpeng = tmpmdl->GetCurrentSetup()->CreateEngineByIndex(curr_eng_index);
printf("project::DoEnergyPlot1D 2\n");
	
	// the temporary model is now ok, continue...
	
	engine * eng = GetCurrentSetup()->GetCurrentEngine();
	if (!eng) eng = GetCurrentSetup()->CreateEngineByIndex(GetCurrentSetup()->GetCurrEngIndex());
	
	i32s molnum = 0; i32s in_crdset = 0;
	
	i32s atmi1[4] = { inda, indb, indc, indd };
	atom * atmr1[4]; f64 range1[2];
	range1[0] = M_PI * start1 / 180.0;
	range1[1] = M_PI * end1 / 180.0;
printf("project::DoEnergyPlot1D 3\n");

	for (i32s n1 = 0;n1 < 4;n1++)
	{
		iter_al it1;
		
		it1 = tmpmdl->FindAtomByIndex(atmi1[n1]);
		if (it1 == tmpmdl->GetAtomsEnd()) { PrintToLog("ERROR : atom not found!\n"); return; }
		atmr1[n1] = & (* it1);
	}
printf("project::DoEnergyPlot1D 4\n");
// must call SortGroups() here because intcrd needs it ; however that might change the indices?!?!?! note that we convert to pointers above...
// must call SortGroups() here because intcrd needs it ; however that might change the indices?!?!?! note that we convert to pointers above...
// must call SortGroups() here because intcrd needs it ; however that might change the indices?!?!?! note that we convert to pointers above...
	if (!tmpmdl->IsGroupsClean()) tmpmdl->UpdateGroups();		// for internal coordinates...
	if (!tmpmdl->IsGroupsSorted()) tmpmdl->SortGroups(true);	// for internal coordinates...
printf("project::DoEnergyPlot1D 5 molnum = %d\n", molnum);
	intcrd * tmpic = new intcrd((* tmpmdl), molnum, in_crdset);
printf("project::DoEnergyPlot1D 5 1\n");
	i32s ict1 = tmpic->FindTorsion(atmr1[1], atmr1[2]);
if (ict1 < 0) { PrintToLog("ERROR : could not find ic.\n"); return; }
printf("project::DoEnergyPlot1D 6\n");
	v3d<fGL> v1a(atmr1[1]->GetCRD(in_crdset), atmr1[0]->GetCRD(in_crdset));
	v3d<fGL> v1b(atmr1[1]->GetCRD(in_crdset), atmr1[2]->GetCRD(in_crdset));
	v3d<fGL> v1c(atmr1[2]->GetCRD(in_crdset), atmr1[3]->GetCRD(in_crdset));
	f64 oldt1 = v1a.tor(v1b, v1c);
printf("project::DoEnergyPlot1D 7\n");
	i32s fft1 = tmpeng->FindTorsion(atmr1[0], atmr1[1], atmr1[2], atmr1[3]);
if (fft1 < 0) { PrintToLog("ERROR : could not find tor-term.\n"); return; }
printf("project::DoEnergyPlot1D 8\n");
	const char * s1 = "tor(deg)"; const char * sv = "E(kJ/mol)";
	plot1d_view * plot = AddPlot1DView(PLOT_USERDATA_STRUCTURE, s1, sv, true);
printf("project::DoEnergyPlot1D 9\n");
	f64 tor1 = range1[0];
	{
	for (i32s s1 = 0; s1 < (div1 + 1);s1++)
	{
		tmpic->SetTorsion(ict1, tor1 - oldt1);
		tmpic->UpdateCartesian();
		
		CopyCRD(tmpmdl, tmpeng, 0);					// lock_local_structure needs coordinates!!!
		tmpeng->SetTorsionConstraint(fft1, tor1, 5000.0, true);
		
		// optimize...
		
		geomopt * opt = new geomopt(tmpeng, 100, 0.025, 10.0);		// optimal settings?!?!?
		
		for (i32s n1 = 0;n1 < optsteps;n1++)
		{
			opt->TakeCGStep(conjugate_gradient::Newton2An);
			if (!(n1 % 50)) cout << n1 << " " << opt->optval << " " << opt->optstp << endl;
		}
		
		CopyCRD(tmpeng, tmpmdl, 0);
		tmpmdl->CenterCRDSet(0, true);
		delete opt;
		
		for (i32u n1 = 0;n1 < av_tmp.size();n1++)
		{
			const fGL * tmpcrd = av_tmp[n1]->GetCRD(0);
			av[n1]->SetCRD(0, tmpcrd[0], tmpcrd[1], tmpcrd[2]);
		}
		
		// compute energy for final structure...
		
		f64 value;
		CopyCRD(this, eng, 0);
		eng->Compute(0); value = eng->energy;
		
		// ...and add it to the plot.
		
		void * udata = convert_cset_to_plotting_udata(this, 0);
		plot->AddData(180.0 * tor1 / M_PI, value, udata);
		
		char mbuff1[256];
		strstream str1(mbuff1, sizeof(mbuff1));
		str1 << "tor = " << (180.0 * tor1 / M_PI) << " deg, energy = " << value << " kJ/mol." << endl << ends;
		PrintToLog(mbuff1);
		
		tor1 += (range1[1] - range1[0]) / (f64) div1;
	}
	}
	delete tmpic;
	delete tmpeng;
	delete tmpmdl;
	
	// the "eng" object is the setup->current_eng object, so there's no need to delete it...
	
	plot->SetCenterAndScale();
	plot->Update();
}

void project::DoEnergyPlot2D(i32s inda, i32s indb, i32s indc, i32s indd, i32s div1, fGL start1, fGL end1, i32s indi, i32s indj, i32s indk, i32s indl, i32s div2, fGL start2, fGL end2, i32s optsteps)
{
	// 2003-11-17 : for IC modification and structure refinement, make a temporary molecular mechanics model.
	
	i32s curr_eng_index = 0;	// if current setup is molecular mechanics, get the eng class...
	setup * tmpsu1 = GetCurrentSetup(); setup1_mm * tmpsu2 = dynamic_cast<setup1_mm *>(tmpsu1);
	if (tmpsu2 != NULL) curr_eng_index = GetCurrentSetup()->GetCurrEngIndex();
	
	model * tmpmdl = new model();	// the default setup here will be molecular mechanics!
	vector<atom *> av; vector<atom *> av_tmp;
	
	for (iter_al it1 = GetAtomsBegin();it1 != GetAtomsEnd();it1++)
	{
		atom newatm((* it1).el, (* it1).GetCRD(0), tmpmdl->GetCRDSetCount());
		tmpmdl->AddAtom(newatm);
		
		av.push_back(& (* it1));
		av_tmp.push_back(& tmpmdl->GetLastAtom());
	}
	{
	for (iter_bl it1 = GetBondsBegin();it1 != GetBondsEnd();it1++)
	{
		i32u ind1 = 0;
		while (ind1 < av.size() && av[ind1] != (* it1).atmr[0]) ind1++;
		if (ind1 == av.size()) { cout << "fatal error! atom #1 not found." << endl; exit(EXIT_FAILURE); }
		
		i32u ind2 = 0;
		while (ind2 < av.size() && av[ind2] != (* it1).atmr[1]) ind2++;
		if (ind2 == av.size()) { cout << "fatal error! atom #2 not found." << endl; exit(EXIT_FAILURE); }
		
		bond newbnd(av_tmp[ind1], av_tmp[ind2], (* it1).bt);
		tmpmdl->AddBond(newbnd);
	}
	}
	engine * tmpeng = tmpmdl->GetCurrentSetup()->CreateEngineByIndex(curr_eng_index);
	
	// the temporary model is now ok, continue...
	
	engine * eng = GetCurrentSetup()->GetCurrentEngine();
	if (!eng) eng = GetCurrentSetup()->CreateEngineByIndex(GetCurrentSetup()->GetCurrEngIndex());
	
	i32s molnum = 0; i32s in_crdset = 0;
	
	i32s atmi1[4] = { inda, indb, indc, indd };
	atom * atmr1[4]; f64 range1[2];
	range1[0] = M_PI * start1 / 180.0;
	range1[1] = M_PI * end1 / 180.0;
	
	i32s atmi2[4] = { indi, indj, indk, indl };
	atom * atmr2[4]; f64 range2[2];
	range2[0] = M_PI * start2 / 180.0;
	range2[1] = M_PI * end2 / 180.0;
	
	for (i32s n1 = 0;n1 < 4;n1++)
	{
		iter_al it1;
		
		it1 = tmpmdl->FindAtomByIndex(atmi1[n1]);
		if (it1 == tmpmdl->GetAtomsEnd()) { PrintToLog("ERROR : tor1 atom not found!\n"); return; }
		atmr1[n1] = & (* it1);
		
		it1 = tmpmdl->FindAtomByIndex(atmi2[n1]);
		if (it1 == tmpmdl->GetAtomsEnd()) { PrintToLog("ERROR : tor2 atom not found!\n"); return; }
		atmr2[n1] = & (* it1);
	}
	
// must call SortGroups() here because intcrd needs it ; however that might change the indices?!?!?! note that we convert to pointers above...
// must call SortGroups() here because intcrd needs it ; however that might change the indices?!?!?! note that we convert to pointers above...
// must call SortGroups() here because intcrd needs it ; however that might change the indices?!?!?! note that we convert to pointers above...
	if (!tmpmdl->IsGroupsClean()) tmpmdl->UpdateGroups();		// for internal coordinates...
	if (!tmpmdl->IsGroupsSorted()) tmpmdl->SortGroups(true);	// for internal coordinates...
	
	intcrd * tmpic = new intcrd((* tmpmdl), molnum, in_crdset);
	i32s ict1 = tmpic->FindTorsion(atmr1[1], atmr1[2]);
if (ict1 < 0) { PrintToLog("ERROR : could not find ic for tor1.\n"); return; }
	i32s ict2 = tmpic->FindTorsion(atmr2[1], atmr2[2]);
if (ict2 < 0) { PrintToLog("ERROR : could not find ic for tor2.\n"); return; }
	
	v3d<fGL> v1a(atmr1[1]->GetCRD(in_crdset), atmr1[0]->GetCRD(in_crdset));
	v3d<fGL> v1b(atmr1[1]->GetCRD(in_crdset), atmr1[2]->GetCRD(in_crdset));
	v3d<fGL> v1c(atmr1[2]->GetCRD(in_crdset), atmr1[3]->GetCRD(in_crdset));
	f64 oldt1 = v1a.tor(v1b, v1c);
	
	v3d<fGL> v2a(atmr2[1]->GetCRD(in_crdset), atmr2[0]->GetCRD(in_crdset));
	v3d<fGL> v2b(atmr2[1]->GetCRD(in_crdset), atmr2[2]->GetCRD(in_crdset));
	v3d<fGL> v2c(atmr2[2]->GetCRD(in_crdset), atmr2[3]->GetCRD(in_crdset));
	f64 oldt2 = v2a.tor(v2b, v2c);
	
	i32s fft1 = tmpeng->FindTorsion(atmr1[0], atmr1[1], atmr1[2], atmr1[3]);
if (ict1 < 0) { PrintToLog("ERROR : could not find tor-term for tor1.\n"); return; }
	i32s fft2 = tmpeng->FindTorsion(atmr2[0], atmr2[1], atmr2[2], atmr2[3]);
if (ict2 < 0) { PrintToLog("ERROR : could not find tor-term for tor2.\n"); return; }
	
	const char * s1 = "tor1(deg)"; const char * s2 = "tor2(deg)"; const char * sv = "E(kJ/mol)";
	plot2d_view * plot = AddPlot2DView(PLOT_USERDATA_STRUCTURE, s1, s2, sv, true);
	
	f64 tor1 = range1[0];
	{
	for (i32s s1 = 0;s1 < (div1 + 1);s1++)
	{
		f64 tor2 = range2[0];
		for (i32s s2 = 0;s2 < (div2 + 1);s2++)
		{
			tmpic->SetTorsion(ict1, tor1 - oldt1);
			tmpic->SetTorsion(ict2, tor2 - oldt2);
			tmpic->UpdateCartesian();
			
			CopyCRD(tmpmdl, tmpeng, 0);					// lock_local_structure needs coordinates!!!
			tmpeng->SetTorsionConstraint(fft1, tor1, 5000.0, true);
			tmpeng->SetTorsionConstraint(fft2, tor2, 5000.0, true);
			
			// optimize...
			
			geomopt * opt = new geomopt(tmpeng, 100, 0.025, 10.0);		// optimal settings?!?!?
			{
			for (i32s n1 = 0;n1 < optsteps;n1++)
			{
				opt->TakeCGStep(conjugate_gradient::Newton2An);
				if (!(n1 % 50)) cout << n1 << " " << opt->optval << " " << opt->optstp << endl;
			}
			}
			CopyCRD(tmpeng, tmpmdl, 0);
			tmpmdl->CenterCRDSet(0, true);
			delete opt;
			
			for (i32u n1 = 0;n1 < av_tmp.size();n1++)
			{
				const fGL * tmpcrd = av_tmp[n1]->GetCRD(0);
				av[n1]->SetCRD(0, tmpcrd[0], tmpcrd[1], tmpcrd[2]);
			}
			
			// compute energy for final structure...
			
			f64 value;
			CopyCRD(this, eng, 0);
			eng->Compute(0); value = eng->energy;
			
			// ...and add it to the plot.
			
			void * udata = convert_cset_to_plotting_udata(this, 0);
			plot->AddData(180.0 * tor1 / M_PI, 180.0 * tor2 / M_PI, value, udata);
			
			char mbuff1[256];
			strstream str1(mbuff1, sizeof(mbuff1));
			str1 << "tor1 = " << (180.0 * tor1 / M_PI) << " deg, tor2 = " << (180.0 * tor2 / M_PI) << " deg, energy = " << value << " kJ/mol." << endl << ends;
			PrintToLog(mbuff1);
			
			tor2 += (range2[1] - range2[0]) / (f64) div2;
		}
		
		tor1 += (range1[1] - range1[0]) / (f64) div1;
	}
	}
	delete tmpic;
	delete tmpeng;
	delete tmpmdl;
	
	// the "eng" object is the setup->current_eng object, so there's no need to delete it...
	
	plot->SetCenterAndScale();
	plot->Update();
}

void project::DoTransitionStateSearch(f64 deltae, f64 initfc)
{
Message("BUG notice : TSS and libmopac7 seem to have\nsome compatibility problems in versions >= 1.90\nsince untraceable segmentation faults occur...\n\ngcc-3.3 or libf2c changes??? 20050608 TH");
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	transition_state_search * tss = new transition_state_search(this, deltae, initfc);
	if (tss->InitFailed()) { delete tss; return; }
	
	char txtbuff[256]; ostrstream txts1(txtbuff, sizeof(txtbuff));
	txts1 << "r-energy = " << tss->GetE(0) << "   " << "p-energy = " << tss->GetE(1) << "   ";
	txts1 << (tss->GetE(0) < tss->GetE(1) ? "r" : "p") << " is lower " << fabs(tss->GetE(0) - tss->GetE(1));
	txts1 << endl << ends; PrintToLog(txtbuff); cout << txtbuff;
	
	f64 erl = tss->GetE(0); f64 epl = tss->GetE(1);
	
	const char * s1 = "rc"; const char * sv = "E(kJ/mol)";
	rcp_view * plot = AddReactionCoordinatePlotView(PLOT_USERDATA_STRUCTURE, s1, sv, true);
	
	for (i32u n1 = 0;n1 < tss->patoms.size();n1++) plot->AddPAtom(tss->patoms[n1]);
	for (i32u n1 = 0;n1 < tss->rbonds.size();n1++) plot->AddRBond(tss->rbonds[n1]);
	for (i32u n1 = 0;n1 < tss->pbonds.size();n1++) plot->AddPBond(tss->pbonds[n1]);
	
	void * udata;
	
	// add the initial structures...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	udata = convert_cset_to_plotting_udata(this, 0);
	plot->AddData(tss->GetP(0), tss->GetE(0), udata);
	
	udata = convert_cset_to_plotting_udata(this, 1);
	plot->AddData(tss->GetP(1), tss->GetE(1), udata);
	
	// loop...
	// ^^^^^^^
	
	i32s prev_not_stored[2] = { false, false };
	while (true)
	{
		tss->Run(prev_not_stored);
		
		ostrstream txts2(txtbuff, sizeof(txtbuff));
		txts2 << "r-energy = " << tss->GetE(0) << "   " << "p-energy = " << tss->GetE(1) << "   ";
		txts2 << (tss->GetE(0) < tss->GetE(1) ? "r" : "p") << " is lower " << fabs(tss->GetE(0) - tss->GetE(1)) << "   ";
		
		if (tss->GetR(0) && tss->GetR(1))
		{
			txts2 << "READY!" << endl << ends; PrintToLog(txtbuff);
			break;
		}
		
		bool update[2] = { !tss->GetR(0), !tss->GetR(1) };
		if (tss->GetE(1) < erl) update[0] = false;
		if (tss->GetE(0) < epl) update[1] = false;
		
		if (!update[0] && !update[1])	// this is a deadlock situation, fix it...
		{
////////////////////////////////////////////////////////////////
//cout << (i32s) update[0] << (i32s) update[1] << " ";
//cout << (i32s) tss->GetR(0) << (i32s) tss->GetR(1) << "   ";
//cout << "DEADLOCK!!!" << endl; int rr;cin>>rr;
////////////////////////////////////////////////////////////////
			if (!tss->GetR(0) && tss->GetR(1)) update[0] = true;
			if (tss->GetR(0) && !tss->GetR(1)) update[1] = true;
			if (!update[0] && !update[1])
			{
				f64 delta1 = erl - tss->GetE(1);
				f64 delta2 = epl - tss->GetE(0);
				i32s uuu = (delta1 > delta2 ? 0 : 1);	// update the bigger one...
				update[uuu] = true;
			}
		}
		
		txts2 << (i32s) update[0] << (i32s) update[1] << " ";
		txts2 << (i32s) tss->GetR(0) << (i32s) tss->GetR(1);
		txts2 << endl << ends; PrintToLog(txtbuff); cout << txtbuff;
		
		tss->UpdateTargets(update);
		
		if (update[0])
		{
			udata = convert_cset_to_plotting_udata(this, 0);
			plot->AddData(tss->GetP(0), tss->GetE(0), udata);
			
			erl = tss->GetE(0);
		}
		
		if (update[1])
		{
			udata = convert_cset_to_plotting_udata(this, 1);
			plot->AddData(tss->GetP(1), tss->GetE(1), udata);
			
			epl = tss->GetE(1);
		}
		
		prev_not_stored[0] = !update[0];
		prev_not_stored[1] = !update[1];
	}
	
	delete tss;
	
	// create an approximate TS as an average of the two structures.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	for (iter_al it1 = GetAtomsBegin();it1 != GetAtomsEnd();it1++)
	{
		const fGL * crdr = (* it1).GetCRD(0);
		const fGL * crdp = (* it1).GetCRD(1);
		
		fGL x = (crdr[0] + crdp[0]) / 2.0;
		fGL y = (crdr[1] + crdp[1]) / 2.0;
		fGL z = (crdr[2] + crdp[2]) / 2.0;
		
		(* it1).SetCRD(0, x, y, z);
	}
	
	PopCRDSets(1);		// remove the 2nd crd-set that is no longer needed.
	
	// refine the approximate TS using stationary state search...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	DoStationaryStateSearch(100);
	f64 ts_e = GetCurrentSetup()->GetCurrentEngine()->energy;
	
	// add the final estimate of TS, and finish the plot.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	udata = convert_cset_to_plotting_udata(this, 0);
	plot->AddData(0, ts_e, udata);
	
	plot->SortDataAndCalcRC();
	plot->SetCenterAndScale();
	plot->Update();
}

void project::DoStationaryStateSearch(i32s steps)
{
	engine * eng = GetCurrentSetup()->GetCurrentEngine();
	if (eng == NULL) GetCurrentSetup()->CreateCurrentEngine();
	eng = GetCurrentSetup()->GetCurrentEngine();
	if (eng == NULL) return;
	
	char mbuff1[256];
	ostrstream str1(mbuff1, sizeof(mbuff1));
	str1 << "Starting Stationary State Search ";
	str1 << "(setup = " << GetCurrentSetup()->GetClassName();
	str1 << ", engine = " << GetCurrentSetup()->GetEngineName(GetCurrentSetup()->GetCurrEngIndex());
	str1 << ")." << endl << ends;
	PrintToLog(mbuff1);
	
	CopyCRD(this, eng, 0);
	
	// use a small default steplength; also setting maximum steplength is important!!!
	stationary_state_search * sss = new stationary_state_search(eng, 25, 1.0e-7, 1.0e-5);
	
	char buffer[1024];
	PrintToLog("Cycle    Gradient       Step\n");
	
	i32s n1 = 0;	// n1 counts the number of steps...
	while (true)
	{
		sss->TakeCGStep(conjugate_gradient::Simple);
		
		sprintf(buffer, "%4d %10.4e %10.4e \n", n1, sss->optval, sss->optstp);
		
		PrintToLog(buffer);
		
		bool terminate = false;
		if (n1 >= steps)
		{
			terminate = true;
			PrintToLog("the nsteps termination test was passed.\n");
		}
		
		if (!(n1 % 10) || terminate)
		{
			CopyCRD(eng, this, 0);
			CenterCRDSet(0, true);
			
			UpdateAllGraphicsViews(true);
		}
		
		if (terminate) break;		// exit the loop here!!!
		
		n1++;	// update the number of steps...
	}
	
	delete sss;
	
// we will not delete current_eng here, so that we can draw plots using it...
	
	// above, CopyCRD was done eng->mdl and then CenterCRDSet() was done for mdl.
	// this might cause that old coordinates remain in eng object, possibly affecting plots.
	// here we sync the coordinates and other plotting data in the eng object.
	
	CopyCRD(this, eng, 0);
	SetupPlotting();
}

/*################################################################################################*/

dummy_project::dummy_project(graphics_class_factory * p1) : project(* p1)
{
}

dummy_project::~dummy_project(void)
{
}

/*################################################################################################*/

void color_mode_element::GetColor4(const void * dd, i32s cs, fGL * pp)
{
	atom * ref = (atom *) dd;
	const fGL * color = ref->el.GetColor();
	pp[0] = color[0]; pp[1] = color[1]; pp[2] = color[2]; pp[3] = 1.0;
}

void color_mode_secstruct::GetColor4(const void * dd, i32s cs, fGL * pp)
{
	atom * ref = (atom *) dd;
	model * mdl = ref->GetModel();
	
	pp[0] = 0.0; pp[1] = 0.0; pp[2] = 1.0; pp[3] = 0;	// loop
	
	if (mdl == NULL || mdl->ref_civ == NULL) return;
	if (ref->id[1] < 0 || ref->id[2] < 0) return;
	
	vector<chn_info> & ci_vector = (* mdl->ref_civ);
	const char * tmptab = ci_vector[ref->id[1]].GetSecStrStates();
	
	if (tmptab == NULL) return;
	char state = tmptab[ref->id[2]];
	
	switch (state)
	{
		case '4':
		pp[0] = 1.0; pp[1] = 0.0; pp[2] = 0.0;		// helix
		return;
		
		case 'S':
		pp[0] = 0.0; pp[1] = 1.0; pp[2] = 0.0;		// strand
		return;
	}
}

void color_mode_hydphob::GetColor4(const void * dd, i32s cs, fGL * pp)
{
	atom * ref = (atom *) dd;
	model * mdl = ref->GetModel();
	
	pp[0] = 0.0; pp[1] = 0.5; pp[2] = 0.0; pp[3] = 0;	// default...
	
	if (mdl == NULL || mdl->ref_civ == NULL) return;
	if (ref->id[1] < 0 || ref->id[2] < 0) return;
	
	vector<chn_info> & ci_vector = (* mdl->ref_civ);
	const char * tmptab = ci_vector[ref->id[1]].GetSequence();
	
	if (tmptab == NULL) return;
	char state = tmptab[ref->id[2]];
	
	
	switch (state)
	{
		case 'A':
		case 'G':
		pp[0] = 0.0; pp[1] = 1.0; pp[2] = 0.0;		// ala/gly
		return;
		
		case 'V':
		case 'F':
		case 'I':
		case 'L':
		case 'P':
		case 'M':
		pp[0] = 1.0; pp[1] = 0.0; pp[2] = 0.0;		// hydrophobic
		return;
		
		case 'D':
		case 'E':
		case 'K':
		case 'R':
		pp[0] = 0.2; pp[1] = 0.2; pp[2] = 1.0;		// charged
		return;
		
		case 'S':
		case 'T':
		case 'Y':
		case 'C':
		case 'N':
		case 'Q':
		case 'H':
		case 'W':
		pp[0] = 0.0; pp[1] = 1.0; pp[2] = 2.0;		// polar
		return;
	}
}

/*################################################################################################*/

transparent_primitive::transparent_primitive(void)
{
	owner = NULL; data = NULL;
	z_distance = 0.0;
}

transparent_primitive::transparent_primitive(void * p1, transparent_primitive_data & p2)
{
	owner = p1; data = & p2;
	z_distance = 0.0;
}

transparent_primitive::transparent_primitive(const transparent_primitive & p1)
{
	owner = p1.owner; data = p1.data;
	z_distance = p1.z_distance;
}

transparent_primitive::~transparent_primitive(void)
{
}

bool transparent_primitive::TestOwner(void * p1) const
{
	return (owner == p1);
}

transparent_primitive_data * transparent_primitive::GetData(void) const
{
	return data;
}

void transparent_primitive::UpdateDistance(const fGL * crd_c, const fGL * zdir)
{
	fGL crd_p[3];
	crd_p[0] = data->midpoint[0] - crd_c[0];
	crd_p[1] = data->midpoint[1] - crd_c[1];
	crd_p[2] = data->midpoint[2] - crd_c[2];
	
	// if we mark the vector from crd_c to crd_p as v1, and angle between v1 and zdir as angle
	// alpha, we have
	//
	//	zdist = cos(alpha) * |v1|			, where |v1| = length of v1. since
	//
	//	cos(alpha) = ip(v1,zdir) / (|zdir| * |v1|)	, we have
	//
	//	zdist = ip(v1,zdir) / |zdir|			, and if zdir is a unit vector,
	//
	//	zdist = ip(v1,zdir)
	
	z_distance = crd_p[0] * zdir[0] + crd_p[1] * zdir[1] + crd_p[2] * zdir[2];
}

bool transparent_primitive::operator<(const transparent_primitive & p1) const
{
	return (z_distance > p1.z_distance);	// inverted order...
}

/*################################################################################################*/

transparent_primitive_data::transparent_primitive_data(void)
{
}

transparent_primitive_data::~transparent_primitive_data(void)
{
}

/*################################################################################################*/

tpd_tri_3c::tpd_tri_3c(fGL * c1, fGL * p1, fGL * c2, fGL * p2, fGL * c3, fGL * p3)
{
	color[0] = c1;
	color[1] = c2;
	color[2] = c3;
	
	point[0] = p1;
	point[1] = p2;
	point[2] = p3;
	
	UpdateMP();
}

tpd_tri_3c::~tpd_tri_3c(void)
{
}

void tpd_tri_3c::Render(void)
{
	glBegin(GL_TRIANGLES);
	
	glColor4fv(color[0]);
	glVertex3fv(point[0]);
	
	glColor4fv(color[1]);
	glVertex3fv(point[1]);
	
	glColor4fv(color[2]);
	glVertex3fv(point[2]);
	
	glEnd();	// GL_TRIANGLES
}

void tpd_tri_3c::UpdateMP(void)
{
	midpoint[0] = (point[0][0] + point[1][0] + point[2][0]) / 3.0;
	midpoint[1] = (point[0][1] + point[1][1] + point[2][1]) / 3.0;
	midpoint[2] = (point[0][2] + point[1][2] + point[2][2]) / 3.0;
}

/*################################################################################################*/

tpd_quad_4c::tpd_quad_4c(fGL * c1, fGL * p1, fGL * c2, fGL * p2, fGL * c3, fGL * p3, fGL * c4, fGL * p4)
{
	color[0] = c1;
	color[1] = c2;
	color[2] = c3;
	color[3] = c4;
	
	point[0] = p1;
	point[1] = p2;
	point[2] = p3;
	point[3] = p4;
	
	UpdateMP();
}

tpd_quad_4c::~tpd_quad_4c(void)
{
}

void tpd_quad_4c::Render(void)
{
	glBegin(GL_QUADS);
	
	glColor4fv(color[0]);
	glVertex3fv(point[0]);
	
	glColor4fv(color[1]);
	glVertex3fv(point[1]);
	
	glColor4fv(color[2]);
	glVertex3fv(point[2]);
	
	glColor4fv(color[3]);
	glVertex3fv(point[3]);
	
	glEnd();	// GL_QUADS
}

void tpd_quad_4c::UpdateMP(void)
{
	midpoint[0] = (point[0][0] + point[1][0] + point[2][0] + point[3][0]) / 4.0;
	midpoint[1] = (point[0][1] + point[1][1] + point[2][1] + point[3][1]) / 4.0;
	midpoint[2] = (point[0][2] + point[1][2] + point[2][2] + point[3][2]) / 4.0;
}

/*################################################################################################*/

// eof
