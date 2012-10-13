// ATOM.H : the atom-related classes.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef ATOM_H
#define ATOM_H

#include "libconfig.h"

class element;

class crec;
class atom;

/*################################################################################################*/

class bond;	// bond.h
class model;	// model.h

#include "libdefine.h"
#include "typedef.h"

#include <list>
#include <vector>
using namespace std;

/*################################################################################################*/

#define ELEMENT_SYMBOLS 110

/// An element class.

class element
{
	private:
	
	i32s atomic_number;			///< This is the atomic number, or NOT_DEFINED (equal to -1) for a dummy atom.
	
	static const char string[ELEMENT_SYMBOLS][32];
	static const char symbol[ELEMENT_SYMBOLS][4];
	
	static const fGL color[ELEMENT_SYMBOLS][3];
	
	static const fGL mass[ELEMENT_SYMBOLS];
	static const fGL vdwr[ELEMENT_SYMBOLS];

	public:
	
	static element current_element;		///< This is the current element that user has selected.
	
	public:
	
	element(void);
	element(char *);	///< By symbols.
	element(i32s);		///< By atomic numbers.
	~element(void);
	
	const i32s GetAtomicNumber(void) const;
	const char * GetString(void) const;
	const char * GetSymbol(void) const;	
	const fGL * GetColor(void) const;
	fGL GetAtomicMass(void) const;
	fGL GetVDWRadius(void) const;

// JUST DISABLED FOR A WHILE; remove the "const" settings from tables and write prefs directly there??? QUICKER!!!
// JUST DISABLED FOR A WHILE; remove the "const" settings from tables and write prefs directly there??? QUICKER!!!
// JUST DISABLED FOR A WHILE; remove the "const" settings from tables and write prefs directly there??? QUICKER!!!
//	fGL GetVDWRadius(prefs *) const;
//	const fGL * GetColor(prefs *) const;
	
	void operator++(void);
	void operator--(void);
};

/*################################################################################################*/

/// A connectivity record class.
/** The connectivity records are maintained in the atom objects, to make quick browising of 
bond networks possible. */

class crec
{
//	protected:
	public:		// TODO : not properly divided in public/protected data.
	
	atom * atmr;
	bond * bndr;
	
	public:
	
	crec(void);
	crec(atom *, bond *);
	~crec(void);
	
//	bool operator<(const crec &) const { return false; }		// just for STL...
	bool operator==(const crec &) const;
};

/*################################################################################################*/

#define ATOMFLAG_SELECTED		(1 << 0)	// 0x00000001
#define ATOMFLAG_REQUEST_VISIBLE	(1 << 1)	// 0x00000002
#define ATOMFLAG_REQUEST_QUANTUM_ATOM	(1 << 2)	// 0x00000002
#define ATOMFLAG_REQUEST_UNITED_ATOM	(1 << 3)	// 0x00000004
#define ATOMFLAG_REQUEST_SF_ATOM	(1 << 4)	// 0x00000004

#define ATOMFLAG_IS_SOLVENT_ATOM	(1 << 8)	// 0x00000008	16 ja yli???
#define ATOMFLAG_MEASURE_ND_RDF		(1 << 9)	// 0x00000008	16 ja yli???
#define ATOMFLAG_COUNT_IN_RDF		(1 << 10)	// 0x00000008	16 ja yli???

#define ATOMFLAG_IS_QM_ATOM		(1 << 11)	// 0x00000008	16 ja yli???
#define ATOMFLAG_IS_MM_ATOM		(1 << 12)	// 0x00000008	16 ja yli???
#define ATOMFLAG_IS_SF_ATOM		(1 << 13)	// 0x00000008	16 ja yli???
#define ATOMFLAG_IS_HIDDEN		(1 << 14)	// 0x00000008	16 ja yli???

#define ATOMFLAG_IS_LOCKED		(1 << 15)	// 0x00000008	16 ja yli???
// the LOCKED flag means : 1) DoSHAKE() should not move it 2) in MD acc/vel always 0.0, and is not included when calculating Ekin/T.
// THIS IS STILL AN INCOMPLETE IMPLEMENTATION!!! the flag is IGNORED for example in geometry optimization ; see the sf-model...
#define ATOMFLAG_IS_GRAVI		(1 << 16)	// 0x00000008	16 ja yli???

// - protected??? not needed?

/// An atom class.
/** Will store all data about an atom, including type, coordinates, and a connectivity 
record list for quick access to neighbor atoms. */

class atom
{
	protected:
	
	model * mdl;
	
//	protected:
	public:		// TODO : not properly divided in public/protected data.
	
	element el;		// is saved into files!!!
	int formal_charge;	// is saved into files!!! NOT YET BUT SHOULD!!!
	
	i32s atmtp;		///< The normal MM atomtype.
	i32s atmtp_E;		///< MM atomtype "exception"; related to macromolecules...
	char atmtp_s[4];	///< MM atomtype "exception"; related to macromolecules...
	
	f64 charge;		// is saved into files???
	f64 mass;
	
	f64 vdwr;		// important in SF!!!
	
	list<crec> cr_list;		///< Connectivity records.
	
protected:
	fGL * crd_table;		///< The actual coordinate data; see also model::PushCRDSets().
	i32u crd_table_size_loc;
public:

/// Molecule, chain, residue and atom ID-numbers.
/** model::GatherGroups() will maintain molecule numbers, and sequence builders will handle the rest. 
mol/chn id's are unique, but res/atm numbers are repeated in different chains and molecules.*/
	i32s id[4];
	
/// Residue-ID numbers related to sequence-builder. Not sensitive to protonation states etc...
	i32s builder_res_id;
	
	i32s index;		///< Index of this entry in model::atom_list. Updated by model::UpdateIndex().
	i32s varind;		///< Index of this entry in setup::atmtab. Updated by setup::UpdateSetupInfo().
	
	i32u flags;		///< The flags that carry various information. These are saved into files.
	i32s ecomp_grp_i;
	
	friend class model;	// model::PushCRDSets() needs this...
	
	public:
	
	atom(void);
	atom(element, const fGL *, i32u);
	atom(const atom &);
	~atom(void);
	
	model * GetModel(void) { return mdl; }
	
	const fGL * GetCRD(i32u);
	void SetCRD(i32s, fGL, fGL, fGL);
	
	bool operator<(const atom &) const;	///< Using id-numbers.
	bool operator==(const atom &) const;	///< Using pointers.
};

/*################################################################################################*/

typedef list<crec>::iterator iter_cl;		// cl = connectivity list
typedef list<atom>::iterator iter_al;		// al = atom list

typedef atom * ref_atom;

/*################################################################################################*/

#endif	// ATOM_H

// eof
