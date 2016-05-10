// SEQBUILD.H : a sequencebuilder class for building and identifying sequences.

// Copyright (C) 1998 Tommi Hassinen, Geoff Hutchison.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef SEQBUILD_H
#define SEQBUILD_H

#include "libconfig.h"

class chn_info;

class sequencebuilder;

class sb_data_atm;
class sb_data_bnd;
class sb_data_res;

struct sb_tdata;

/*################################################################################################*/

#include "atom.h"
#include "bond.h"

#include "model.h"
#include "typerule.h"

#include "v3d.h"

typedef vector<atom *> atmr_vector;

/*################################################################################################*/

#define PSTATE_CHARGE_mask		0x07	// three least significant bits...

#define PSTATE_SIGN_NEGATIVE		0x08
#define PSTATE_SIGN_POSITIVE		0x10
#define PSTATE_CHARGED_TERMINAL		0x20

/**	A "##chain info"-class. 
	This is used in the context of peptides/proteins and nucleic acids...
*/
istream & operator>>(istream &, sb_data_res &);
istream & operator>>(istream &, sb_data_bnd &);
istream & operator>>(istream &, sb_data_atm &);
ostream & operator<<(ostream &, sb_data_atm &);
class chn_info
{
	public:
	
	enum chn_type
	{
		not_defined = 0, amino_acid = 1, nucleic_acid = 2
	};
	
	protected:
	
	chn_type type;
	
	i32s id_mol;		///< the molecule number (see also atom::id[]).
	i32s id_chn;		///< the chain number (see also atom::id[]).
	
	i32s length;
	char * sequence;
	
	char * ss_state;	// sec-str-state (as determined in DefineSecondaryStructure()).
	char * p_state;		// protonation state (as determined in SF).
	
	char * description;
	
	friend class sequencebuilder;
	friend class setup1_sf;
		
	friend void DefineSecondaryStructure(model *);
	friend f64 HBondEnergy(model *, i32s *, i32s *);
	
	public:
	
	chn_info(void);
	chn_info(chn_type, i32s);
	chn_info(const chn_info &);
	~chn_info(void);
	
	chn_type GetType(void) { return type; }
	i32s GetLength(void) { return length; }
	
	i32s GetIDmol(void) { return id_mol; }
	i32s GetIDchn(void) { return id_chn; }
	
	const char * GetSequence(void) { return sequence; }
	const char * GetSecStrStates(void) { return ss_state; }
	const char * GetProtonationStates(void) { return p_state; }
};

/*################################################################################################*/

/**	A generic sequence builder class. This same code will handle both peptides/proteins 
	and nucleic acids. Only the input file read in ctor is different. The sequence builders 
	can both build sequences and identify them.
	
	Sequence builders will handle only heavy atoms. You must add the hydrogens separately. 
	At least for peptides/proteins this is a complicated (and pH-dependent) job...
	
	How to handle the histidine case with various tautomeric forms???
*/

class sequencebuilder
{
	protected:
	
	chn_info::chn_type type;
	
	vector<sb_data_atm> main_vector;
	vector<sb_data_res> residue_vector;	// res_vector???
	
	vector<typerule> head_vector;
	vector<typerule> tail_vector;
	
	sb_data_res * mod[3];
	
	char buffer[256];
	
	vector<i32s> id_vector;
	atmr_vector ref_vector;
	
	atmr_vector temporary_vector;
	vector<atmr_vector> path_vector;
	
	friend class model;
	friend class setup1_sf;
	
	public:
	
	sequencebuilder(chn_info::chn_type, char *);
	~sequencebuilder(void);
	
	void Build(model *, char *, f64 *);
	void Identify(model *);
	
	private:
	
	void Build(model *, sb_data_res *, f64 *);
	void Convert(atom *[], f64 *, fGL *);
	
	void FindPath(model *, atom *, atom *, i32u = 0);
	void BuildTemplate(vector<sb_tdata> &, i32s, bool, bool);
	void BuildTemplate(vector<sb_tdata> &, vector<sb_data_atm> &);
	bool CheckTemplate(vector<sb_tdata> &, i32s);
};

/*################################################################################################*/

class sb_data_atm
{
	protected:
	
	i32s id[2];
	i32s prev[3];
	
	element el;
	bondtype bt[2];
	
	typerule * tr;
	
	f64 ic1[3];
	i32s ic2;
	
	friend class sequencebuilder;
	
	public:
	
	sb_data_atm(void);
	sb_data_atm(const sb_data_atm &);
	~sb_data_atm(void);
	
	friend istream & operator>>(istream &, sb_data_atm &);
	friend ostream & operator<<(ostream &, sb_data_atm &);
};

/*################################################################################################*/

class sb_data_bnd
{
	protected:
	
	i32s atm[2];
	bondtype bt;

	friend class sequencebuilder;
	
	public:
	
	sb_data_bnd(void);
	~sb_data_bnd(void);
	
	friend istream & operator>>(istream &, sb_data_bnd &);
};

/*################################################################################################*/

class sb_data_res
{
	protected:
	
	i32s id;
	
	char symbol;
	char * description;
	
	vector<sb_data_atm> atm_vector;
	vector<sb_data_bnd> bnd_vector;

	friend class sequencebuilder;
	
	friend class model;			// old???
	friend class setup1_sf;			// new!!!
	
	public:
	
	sb_data_res(void);
	sb_data_res(const sb_data_res &);
	~sb_data_res(void);
	
	void ReadModification(istream &);
	
	friend istream & operator>>(istream &, sb_data_res &);
};

/*################################################################################################*/

struct sb_tdata
{
	element el; bondtype bt;
	i32s id[2]; atom * ref;
};

/*################################################################################################*/

#endif	// SEQBUILD_H

// eof
