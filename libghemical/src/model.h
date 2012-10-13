// MODEL.H : the "model" class that stores the "atom" and "bond" objects.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef MODEL_H
#define MODEL_H

#include "libconfig.h"

class model;
class crd_set;

// the readpdb classes here are not fundamental, but are located here for convenience...
// the readpdb classes here are not fundamental, but are located here for convenience...
// the readpdb classes here are not fundamental, but are located here for convenience...

class readpdb_mdata;
struct readpdb_mdata_chain;
struct readpdb_data_atom;
struct readpdb_data_ssbond;

struct ecomp_grp_d;

/*################################################################################################*/

class resonance_structures;	// resonance.h

class setup;			// engine.h
class engine;			// engine.h

class geomopt_param;		// geomopt.h
class moldyn_param;		// moldyn.h

#include "seqbuild.h"

#include <list>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
using namespace std;

#define NOT_FOUND 0x7fffffff   // numeric_limits<i32s>::max()?!?!?!

#define SF_NUM_TYPES 37

bool ReadTargetListFile(char * filename, vector<i32s>& target_list);

/*################################################################################################*/

/// The model class contains information about all atoms and bonds in a model.

class model
{
	protected:

	bool m_bMaxMinCoordCalculed;
	
	setup * current_setup;		///< Must always be a valid pointer!!!
	resonance_structures * rs;	///< This pointer may be either NULL or a valid one.
	
	list<atom> atom_list;
	list<bond> bond_list;
	
	vector<crd_set *> cs_vector;	///< This determines how many crd_sets there are in the model.
	i32u crd_table_size_glob;	///< This determines how big the crd_table arrays are; always >= than above!!!
	
	bool is_index_clean;		///< Some flags that show which information is up-to-date...
	bool is_groups_clean;		///< Some flags that show which information is up-to-date...
	bool is_groups_sorted;		///< Some flags that show which information is up-to-date...
	
	i32s qm_total_charge;
	i32s qm_current_orbital;
	
	bool use_boundary_potential;
	f64 boundary_potential_radius1;		// for solute...
	f64 boundary_potential_radius2;		// for solvent...

	public:
	bool use_periodic_boundary_conditions;
	f64 periodic_box_HALFdim[3];
	f64 maxCRD[3], minCRD[3];

	protected:

	i32s nmol;
	vector<chn_info> * ref_civ;	// vector<chn_info *> ?!?!?!
	
	vector<const char *> ecomp_grp_name_usr;
	
//	int ecomp_ngrps;
//	const char ** ecomp_grp_name;
//	ecomp_grp_d * ecomp_grp_data;
	
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// for thermodynamics stuff, several "topologies" (and therefore also several elements, atom types etc...) are needed.
// plan1) define two different elements, bondtypes for the topologies; make it possible to create MM "engine" objects for
// either of the topologies, so that there are exactly the same terms in both objects. then make a third "engine" object with
// energy term parameters intermediate of those two topologies -> one can softly change 1st topology to the 2nd one.
// plan2) do not really store/maintain two sets of elements/bondtypes/etc but instead make two separate systems, only slightly
// modified. later, it's possible to compare them and find the differences (at least if some "anchor" atom pairs are given).
// then, add dummy atoms to both so that exactly same terms appear in engine classes. make this even in many steps?!?!?!
// still many practical questions remain; do dummy atoms interfere in typerules? how to handle dummy atoms in calculations
// (for example, in a case O-H -> Cl-du an atom will disappear??? do not accept structures like du-du-...???)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	public:
	
	static const char libversion[16];
	static const char libdata_path[256];
	
	static const char sf_symbols[20 + 1];
	static const i32s sf_types[SF_NUM_TYPES];
	static const bool sf_is_polar[SF_NUM_TYPES];
	
	static sequencebuilder amino_builder;
	static sequencebuilder nucleic_builder;
	
	friend class sequencebuilder;

	friend class engine;
	friend class engine_mbp;
	friend class engine_pbc;

	friend class setup1_qm;
	friend class eng1_qm_mopac;
	
	friend class setup1_mm;
	friend class eng1_mm_pbc;
	
	friend class eng1_mm_tripos52_nbt_mbp;
	friend class eng1_mm_tripos52_nbt_mim;
	
	friend class eng1_mm_default_nbt_mbp;
	friend class eng1_mm_default_nbt_mim;
	friend class default_tables;
	
	friend class setup1_sf;
	
	friend class color_mode_element;	// this is for ghemical!!!
	friend class color_mode_secstruct;	// this is for ghemical!!!
	friend class color_mode_hydphob;	// this is for ghemical!!!
	
	friend class setup_druid;	// this is for ghemical!!! // needs access to current_setup...	// for BBB
	friend class setup_dialog;	// this is for ghemical!!! // needs access to current_setup...	// for HEAD
	friend class ribbon;		// this is for ghemical!!!

	friend class win_project_view;

	friend void CalcMaxMinCoordinates(model *, engine *, i32u);
	
	friend void CopyCRD(model *, engine *, i32u);
	friend void CopyCRD(engine *, model *, i32u);
	
	friend fGL plot_GetESPValue(fGL *, model *, fGL *);
	friend fGL plot_GetVDWSValue(fGL *, model *, fGL *);
	
	friend void DefineSecondaryStructure(model *);
	friend f64 HBondEnergy(model *, i32s *, i32s *);
	
	public:
	
	model(void);
	virtual ~model(void);
	
	virtual void ThreadLock(void);
	virtual void ThreadUnlock(void);
	virtual bool SetProgress(double, double *);
	
	virtual void Message(const char *);
	virtual void WarningMessage(const char *);
	virtual void ErrorMessage(const char *);
	
	setup * GetCurrentSetup(void);
	
	resonance_structures * GetRS(void);
	
	void CreateRS(void);
	void DestroyRS(void);
	
	static void OpenLibDataFile(ifstream &, bool, const char *);
	
// what to do for this one???
// what to do for this one???
// what to do for this one???
/// The project-class will override this function...
	virtual void UpdateAllGraphicsViews(bool = false) { }
	
// what to do for this one???
// what to do for this one???
// what to do for this one???
/// Add a message string to the logfile. This is just a default for console...
	virtual void PrintToLog(const char * p1) { cout << "PrintToLog: " << p1 << endl; }
	
/// This will return the number of coordinate sets.
/** It is supposed that at least one coordinate set exists all the time!!! */
	i32u GetCRDSetCount(void);
	
// what to do for this one???
// what to do for this one???
// what to do for this one???
	bool GetCRDSetVisible(i32u);
	void SetCRDSetVisible(i32u, bool);
	
	void PushCRDSets(i32u);
	void PopCRDSets(i32u);
	
	void CopyCRDSet(i32u, i32u);
	void SwapCRDSets(i32u, i32u);
	
	void CenterCRDSet(i32u, bool);
	void ReserveCRDSets(i32u);

	virtual void DiscardCurrEng(void);
	
// rename this!!!
// rename this!!!
// rename this!!!
	void SetupPlotting(void);
	
	// methods for adding new atoms and bonds:
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	virtual void Add_Atom(atom &);		///< This will just push new atom to the atom list.
	virtual void RemoveAtom(iter_al);	///< This will delete all bonds associated with this atom, and erase atom from the list...
	
	virtual void AddBond(bond &);		///< This will add neighbor infos for both atoms and add the new bond into the bond list.
	virtual void RemoveBond(iter_bl);	///< This will remove infos from the atoms and erase bond from the bond list.

	void SystemWasModified(void);	///< This should be called ALWAYS if ANY modification is done to the system. Automatically called by Add/Remove/Atom/Bond. GUI should change if change in element etc...
	
	void ClearModel(void);		///< This will remove all atoms and bonds.
	
	// methods for accessing atom/bond lists:
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// ALL THESE ITERATORS SHOULD BE READ-ONLY!!! is it possible to modify the containers using iterators only???
	
	iter_al GetAtomsBegin(void) { return atom_list.begin(); }	//const;
	iter_al GetAtomsEnd(void) { return atom_list.end(); }		//const;
	
	iter_bl GetBondsBegin(void) { return bond_list.begin(); }	//const;
	iter_bl GetBondsEnd(void) { return bond_list.end(); }		//const;
	
	atom & GetLastAtom(void) { return atom_list.back(); }
	bond & GetLastBond(void) { return bond_list.back(); }
	
	// methods for ???
	// ^^^^^^^^^^^^^^^
	
	i32s GetQMTotalCharge(void) { return qm_total_charge; }
	i32s GetQMCurrentOrbital(void) { return qm_current_orbital; }
	
	i32s GetAtomCount(void) { return atom_list.size(); }	///< This will return atom_list.size().
	i32s GetBondCount(void) { return bond_list.size(); }	///< This will return bond_list.size().
	
	i32s GetMoleculeCount(void) { return nmol; }			///< This will return nmol (see UpdateGroups() for more info).

	bool IsEmpty(void) { return (bond_list.empty() && atom_list.empty()); }		///< This will return whether the system is empty.
	
	bool IsIndexClean(void) { return is_index_clean; }
	bool IsGroupsClean(void) { return is_groups_clean; }
	bool IsGroupsSorted(void) { return is_groups_sorted; }
	
	iter_al FindAtomByIndex(i32s);
	
	void GetRange(i32s, i32s, iter_al *);	///< This is just a default version of GetRange() using the full range of atom list iterators...
	
/// GetRange is used to get a range of atoms that form molecules, residues etc...
/** This will reduce the initial range of two atom list iterators to some subrange
with certain values in certain atom::id[]-fields. Before using this you MUST call 
model::UpdateGroups() to arrange the atom list! What the above explanation really tries 
to say, is that using this function you can pick up a certain part of the model; for 
example a molecule, or a chain in a macromolecule, or a range of residue in a chain. */
	void GetRange(i32s, iter_al *, i32s, iter_al *);
	
	void GetRange(i32s, iter_bl *);		///< This GetRange() gives a range of bond iterators for a molecule...
	
	i32s FindPath(atom *, atom *, i32s, i32s, i32s = 0);
	vector<bond *> * FindPathV(atom *, atom *, i32s, i32s, i32s = 0);
	bool FindRing(atom *, atom *, signed char *, i32s, i32s, i32s = 0);
	
/** Adding or removing atoms or bonds will generally scramble the grouping, 
and when this happens one should call this function to discard all grouping information. */
	virtual void InvalidateGroups(void);
	
	void UpdateIndex(void);
	
/** This will set the atom ID numbers so that molecules form groups. Will also validate model::nmol 
but will not yet permit the use of model::GetRange()-functions since atom list is not sorted. */
	void UpdateGroups(void);
	
/** This will group the atom list so that molecules (and chains/residues if defined) will form 
continuous groups. Will permit the use of model::GetRange()-functions. */
	void SortGroups(bool);
	
/** This will validate ref_civ and fill it with chain descriptions using all sequencebuilder objects. 
Finally it will call model::SortGroups() so that chains and residues in the atom list will be ordered 
sequentially in contiguous blocks. */
	virtual void UpdateChains(void);
	
	private:
	
/** This will set molecule numbers quickly using a recursive search algorithm. 
This is private because only model::UpdateGroups() should use this... */
	void GatherAtoms(atom *, i32s);
	
	atom * cp_FindAtom(iter_al *, i32s);		///< this is just for CheckProtonation()...
	
/** This will create tables of non-standard valences using ref_civ->p_state information. 
This is private because only model::AddHydrogens() should use this... */
	void CheckProtonation(vector<atom *> &, vector<float> &);
	
	public:
	
	void AddHydrogens(void);
	void RemoveHydrogens(void);
	
	void SolvateBox(fGL, fGL, fGL, fGL = 1.0, int element_number = 2, model * = NULL, const char * = NULL);
	void SolvateSphere(fGL, fGL, fGL = 1.0,  model * = NULL);
	fGL S_Initialize(fGL, model **);
	fGL S_Initialize(fGL, model **, int);
	
	// here we have a set of Do???()-functions. the idea is that there is a set
	// of features that we wish to behave EXACTLY same way in each target/platform.
	// we then create a Do???()-function for the feature, and hide the details of
	// the user interface in a set of virtual functions.
	
/** This will perform an energy calculation, and report the result. */
	void DoEnergy(void);
	
/** This will perform geometry optimization. */
	void DoGeomOpt(geomopt_param &, bool);
	
/** This is used to ask user the GO options; this default function will just silently accept the defaults. */
	virtual void GeomOptGetParam(geomopt_param &);
	
/** This will perform molecular dynamics. */
	void DoMolDyn(moldyn_param &, bool);
	
/** This is used to ask user the MD options; this default function will just silently accept the defaults. */
	virtual void MolDynGetParam(moldyn_param &);
	
/** This will perform a random search using torsions as variables. */
	void DoRandomSearch(i32s, i32s, bool);
	
/** This will perform a systematic search using torsions as variables. */
	void DoSystematicSearch(i32s, i32s, bool);
	
/** This will perform a MonteCarlo search using torsions as variables. */
	void DoMonteCarloSearch(i32s, i32s, i32s, bool);
	
	public:
	
/// A function for reading in Brookhaven PDB/ENT files.
/** The readpdb functions here are used to import PDB files as correctly as possible. 
The PDB files represent experimental results, and in many cases the structures in files 
have gapped and/or incomplete sequences, incomplete residues, and so on.

The readpdb functions do the import in two stages: in first stage read in the "metadata" 
(all headers and remarks about the data including the original sequence), and in second stage 
read in the data as correctly as possible. later, results from these two can be compared, for 
example to evaluate quality of the data or to match the data with records in other databases. */
	readpdb_mdata * readpdb_ReadMData(const char *);
	
	void readpdb_ReadData(const char *, readpdb_mdata *, i32s);
	i32s readpdb_ReadData_sub1(vector<readpdb_data_atom> &, i32s *, const char *, bool);
	void readpdb_ReadData_sub2(vector<readpdb_data_atom> &, i32s *, const char *, const char *, char);
	
	void ecomp_AddGroupU(const char *);
	bool ecomp_DeleteGroupU(int);
	
/*	void ecomp_UnRegisterAll(void);
	void ecomp_RegisterNoAuto(void);
	void ecomp_RegisterWithAutoSolv(void);
	void ecomp_RegisterWithAutoSolvChn1(void);
	void ecomp_RegisterWithAutoSolvChn2(void);	*/
	
	private:
	
	void ecomp_Register(void);

	//void Correct_periodic_box_HALFdim(engine * eng);
#if SNARJAD_TARGET_WORKING
	i32s n_snarjad_through_membrana;
	f64 m_pre_snarjad_crd_z;
	f64 m_pre_target_crd_z;
	//f64 m_pre_snarjad_membrana_dist_z;
	bool b_snarjad_through_membrana_started;
#endif

public:

	bool Read_GPR(char * infile_name);


#if DIFFUSE_WORKING
	void work_diffuse(char *infile_name);
#endif

#if SNARJAD_TARGET_WORKING
	void work(double start, double step, double fin, char *infile_name, char * trgtlst_name);//my experiment
	void working(double vel, char *infile_name, char * trgtlst_name);
#endif

#if PROBNIY_ATOM_WORKING
	void work_prob_atom(char *infile_name, char * trgtlst_name);//my experiment
	void working_prob_atom(char *infile_name, char * trgtlst_name);
#endif

#if PROBNIY_ATOM_WORKING || PROBNIY_ATOM_GEOMOPT
	bool prob_atom_move_direction;
#endif

#if PROBNIY_ATOM_GEOMOPT
	void work_prob_atom_GeomOpt(char *infile_name, char * trgtlst_name, char * box_name, char * fixed_name);//my experiment
	void working_prob_atom_GeomOpt(char *infile_name, char * trgtlst_name, char * box_name, char * fixed_name);//my experiment
#endif

	void SaveBox(char * boxfilename);
	bool LoadBox(char * boxfilename);
	
	void SaveSelected(char * filename);
	void LoadSelected(char * filename);

};

/*################################################################################################*/

/// A coordinate-set class.

class crd_set
{
	protected:
	
	char * description;
	
// bring also the coloring information here??? -> would allow different colors for different crdsets!!!
// bring also the coloring information here??? -> would allow different colors for different crdsets!!!
// bring also the coloring information here??? -> would allow different colors for different crdsets!!!

	public:
	
// these are tricky to protect, since these are still developing and are not yet used much.
// so, these are public as long as some reasonable strategy is found...

	fGL accum_weight;
	fGL accum_value;
	bool visible;
	
	public:
	
	crd_set(void);
	crd_set(const crd_set &);
	~crd_set(void);
	
	void SetDescription(const char *);
};

/*################################################################################################*/

// define struct readpdb_mdata_chain before class readpdb_mdata, since the latter uses
// former in some inline functions...

// how to best relate readpdb_mdata_chain and chn_info !??!?!?!?!?
// maybe just by storing the alpha-carbon pointers here...

struct readpdb_mdata_chain
{
	char chn_id;
	char * seqres;
	
	vector<i32s> missing_residues;
	vector<atom *> alpha_carbons;
};

// class readpdb_mdata is a class just to make the memory management easier. the data members in
// the class are filled in model::readpdb_ReadMData(), and at end the object is just to be deleted.

class readpdb_mdata
{
	public:
	
	vector<readpdb_mdata_chain *> chn_vector;
	
	public:
	
	readpdb_mdata(void)
	{
	}
	
	~readpdb_mdata(void)
	{
		for (i32u n1 = 0;n1 < chn_vector.size();n1++)
		{
			delete[] chn_vector[n1]->seqres;	// delete the sequence...
			delete chn_vector[n1];			// delete the whole record...
		}
	}
};

// READPDB_MAX_CRDSETS is relevant only if READPDB_ENABLE_MULTIPLE_CRDSETS is defined...
// READPDB_MAX_CRDSETS is relevant only if READPDB_ENABLE_MULTIPLE_CRDSETS is defined...
// READPDB_MAX_CRDSETS is relevant only if READPDB_ENABLE_MULTIPLE_CRDSETS is defined...

#define READPDB_MAX_CRDSETS 10

struct readpdb_data_atom
{
	char chn_id;
	
	i32s res_num;
	char res_name[5];
	char atm_name[5];
	fGL crd[READPDB_MAX_CRDSETS][3];
	
	atom * ref;
};

struct readpdb_data_ssbond
{
	char chn_id;
	i32s res_num;
	
	atom * ref;
};

struct ecomp_grp_d
{
	double ECOMP_b;
	double ECOMP_nbA;
	double ECOMP_nbB;
};

/*################################################################################################*/

#endif	// MODEL_H

// eof
