// TAB_MM_DEFAULT.H : tables for the molecular mechanics force field.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef TAB_MM_DEFAULT_H
#define TAB_MM_DEFAULT_H

#include "libconfig.h"

struct default_at;		// atomtype

struct default_bs;		// bond stretching
struct default_ab;		// angle bending
struct default_tr;		// torsion
struct default_op;		// out of plane

struct default_bs_query;
struct default_ab_query;
struct default_tr_query;
struct default_op_query;

class default_tables;

struct e_atom_name;
struct e_vdw_param;

/*################################################################################################*/

class model;		// model.h
class typerule;		// typerule.h

#include "eng1_mm_default.h"

#include <vector>
using namespace std;

/*################################################################################################*/

struct default_at		// atomtype
{
	i32s atomtype[2];
	
	f64 vdw_R; f64 vdw_E;
	f64 formal_charge;
	i32u flags;
	
	typerule * tr;
	char * description;
};

struct default_bs		// bond stretching
{
	i32s atmtp[2];
	bondtype bndtp;
	
	f64 opt;
	f64 fc;
	
	f64 ci;
};

struct default_ab		// angle bending
{
	i32s atmtp[3];
	bondtype bndtp[2];
	
	f64 opt;
	f64 fc;
};

struct default_tr		// torsion
{
	i32s atmtp[4];
	bondtype bndtp[3];
	
	f64 fc1;
	f64 fc2;
	f64 fc3;
};

struct default_op		// out of plane
{
	i32s atmtp[4];
	bondtype bndtp[3];
	
	f64 opt;
	f64 fc;
};

struct default_bs_query
{
	i32s atmtp[2];		// filled by client!!!
	bondtype bndtp;		// filled by client!!!

	bool strict;		// filled by client!!!
	
	i32s index;
	bool dir;
	
	f64 opt;
	f64 fc;
	
	f64 ci;
};

struct default_ab_query
{
	i32s atmtp[3];		// filled by client!!!
	bondtype bndtp[2];	// filled by client!!!

	bool strict;		// filled by client!!!

	i32s index;
	bool dir;
	
	f64 opt;
	f64 fc;
};

struct default_tr_query
{
	i32s atmtp[4];		// filled by client!!!
	bondtype bndtp[3];	// filled by client!!!

	bool strict;		// filled by client!!!

	i32s index;
	bool dir;
	
	f64 fc1;
	f64 fc2;
	f64 fc3;
};

struct default_op_query
{
	i32s atmtp[4];		// filled by client!!!
	bondtype bndtp[3];	// filled by client!!!

	bool strict;		// filled by client!!!

	i32s index;
	
	f64 opt;
	f64 fc;
};

/*################################################################################################*/

#define RES_NAME_TAB_SIZE (27*3+8*3)
#define ATM_NAME_TAB_SIZE (4+83+53)

/// Molecular mechanics force field parameter tables.

class default_tables
{
	private:
	
	static default_tables * instance;
	
i32s secondary_types_depth;	// DANGEROUS!!! keep this NOT_DEFINED in normal use!
bool use_strict_query;		// DANGEROUS!!! keep this false in normal use!
bool def_params_only;		// DANGEROUS!!! keep this false in normal use!
	
	vector<default_at> at_vector;
	
	vector<default_bs> bs_vector;
	vector<default_ab> ab_vector;
	vector<default_tr> tr_vector;
	vector<default_op> op_vector;
	
	public:
	
	ostream * ostr;		///< an optional output stream, defaults to (& cout); may be set to NULL.
	
	private:
	
	default_tables(void);
	
	public:
	
	~default_tables(void);
	static default_tables * GetInstance(void);

	void PrintAllTypeRules(ostream &);

	i32u UpdateTypes(setup *);
	i32u UpdateCharges(setup *);
	
	const default_at * GetAtomType(i32s);
	
	void DoParamSearch(default_bs_query *);
	void DoParamSearch(default_ab_query *);
	void DoParamSearch(default_tr_query *);
	void DoParamSearch(default_op_query *);
	
	private:
	
	// exceptions...
	// exceptions...
	// exceptions...
	
	atom * e_UT_FindAtom(iter_al *, i32s);
	void e_UT_FindHydrogens(iter_al *, i32s, vector<atom *> &);
	
	i32u e_UpdateTypes(setup *);
	i32u e_UpdateCharges(setup *);
	
	public:
	
	bool e_Init(eng1_mm *, mm_default_bt1 *, i32s);		///< only eng1_mm_default_??? classes should call these...
	bool e_Init(eng1_mm *, mm_default_bt2 *, i32s *);	///< only eng1_mm_default_??? classes should call these...
	bool e_Init(eng1_mm *, mm_default_bt3 *, i32s *);	///< only eng1_mm_default_??? classes should call these...
	bool e_Init(eng1_mm *, mm_default_bt4 *, i32s *);	///< only eng1_mm_default_??? classes should call these...
	bool e_Init(eng1_mm *, mm_default_nbt1 *, bool);	///< only eng1_mm_default_??? classes should call these...
	
	public:
	
	static const e_atom_name res_name_tab[RES_NAME_TAB_SIZE];
	static const e_atom_name atm_name_tab[ATM_NAME_TAB_SIZE];
};

/*################################################################################################*/
/*################################################################################################*/
/*################################################################################################*/

#define ATE_type_mask		0xff0000
#define ATE_resn_mask		0xff00
#define ATE_atmn_mask		0xff

// define "residue types"; types for solvents/ions are also included.
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#define ATE_type_XT		0x100000

#define ATE_type_AA		0x200000
#define ATE_type_AANT		0x210000
#define ATE_type_AACT		0x220000

#define ATE_type_NA		0x300000
#define ATE_type_NA5T		0x310000
#define ATE_type_NA3T		0x320000

// define residue names.
// ^^^^^^^^^^^^^^^^^^^^^

#define ATE_resn_aa_ALA		0x0000
#define ATE_resn_aa_ARG		0x0100
#define ATE_resn_aa_ASN		0x0200
#define ATE_resn_aa_ASP		0x0300		// ASP charged
#define ATE_resn_aa_ASH		0x0400		// ASP neutral
#define ATE_resn_aa_CYS		0x0500		// CYS neutral
#define ATE_resn_aa_CYM		0x0600		// CYS charged
#define ATE_resn_aa_CYX		0x0700		// CYS bridged
#define ATE_resn_aa_GLN		0x0800
#define ATE_resn_aa_GLU		0x0900		// GLU charged
#define ATE_resn_aa_GLH		0x0a00		// GLU neutral
#define ATE_resn_aa_GLY		0x0b00
#define ATE_resn_aa_HID		0x0c00		// HIS delta-hydrogen
#define ATE_resn_aa_HIE		0x0d00		// HIS epsilon-hydrogen
#define ATE_resn_aa_HIP		0x0e00		// HIS charged
#define ATE_resn_aa_ILE		0x0f00
#define ATE_resn_aa_LEU		0x1000
#define ATE_resn_aa_LYS		0x1100		// LYS charged
#define ATE_resn_aa_LYN		0x1200		// LYS neutral
#define ATE_resn_aa_MET		0x1300
#define ATE_resn_aa_PHE		0x1400
#define ATE_resn_aa_PRO		0x1500
#define ATE_resn_aa_SER		0x1600
#define ATE_resn_aa_THR		0x1700
#define ATE_resn_aa_TRP		0x1800
#define ATE_resn_aa_TYR		0x1900
#define ATE_resn_aa_VAL		0x1a00

#define ATE_resn_na_DA		0x0000
#define ATE_resn_na_DC		0x0100
#define ATE_resn_na_DG		0x0200
#define ATE_resn_na_DT		0x0300
#define ATE_resn_na_RA		0x0400
#define ATE_resn_na_RC		0x0500
#define ATE_resn_na_RG		0x0600
#define ATE_resn_na_RU		0x0700

// define atom names for "extra" parameters; solvents and ions (n = 4).
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#define ATE_atmn_xt_Hh2o	0x00
#define ATE_atmn_xt_Oh2o	0x01
#define ATE_atmn_xt_Na		0x02
#define ATE_atmn_xt_Cl		0x03

// define atom names for amino acids (n = 83).
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#define ATE_atmn_aa_H		0x2a
#define ATE_atmn_aa_H1		0x2b
#define ATE_atmn_aa_H2		0x2c
#define ATE_atmn_aa_H3		0x2d
#define ATE_atmn_aa_HA		0x2e
#define ATE_atmn_aa_HA2		0x2f
#define ATE_atmn_aa_HA3		0x30
#define ATE_atmn_aa_HB		0x31
#define ATE_atmn_aa_HB1		0x32
#define ATE_atmn_aa_HB2		0x33
#define ATE_atmn_aa_HB3		0x34
#define ATE_atmn_aa_HG		0x35
#define ATE_atmn_aa_HG1		0x36
#define ATE_atmn_aa_HG11	0x37
#define ATE_atmn_aa_HG12	0x38
#define ATE_atmn_aa_HG13	0x39
#define ATE_atmn_aa_HG2		0x3a
#define ATE_atmn_aa_HG21	0x3b
#define ATE_atmn_aa_HG22	0x3c
#define ATE_atmn_aa_HG23	0x3d
#define ATE_atmn_aa_HG3		0x3e
#define ATE_atmn_aa_HD1		0x3f
#define ATE_atmn_aa_HD11	0x40
#define ATE_atmn_aa_HD12	0x41
#define ATE_atmn_aa_HD13	0x42
#define ATE_atmn_aa_HD2		0x43
#define ATE_atmn_aa_HD21	0x44
#define ATE_atmn_aa_HD22	0x45
#define ATE_atmn_aa_HD23	0x46
#define ATE_atmn_aa_HD3		0x47
#define ATE_atmn_aa_HE		0x48
#define ATE_atmn_aa_HE1		0x49
#define ATE_atmn_aa_HE2		0x4a
#define ATE_atmn_aa_HE21	0x4b
#define ATE_atmn_aa_HE22	0x4c
#define ATE_atmn_aa_HE3		0x4d
#define ATE_atmn_aa_HZ		0x4e
#define ATE_atmn_aa_HZ1		0x4f
#define ATE_atmn_aa_HZ2		0x50
#define ATE_atmn_aa_HZ3		0x51
#define ATE_atmn_aa_HH		0x52
#define ATE_atmn_aa_HH11	0x53
#define ATE_atmn_aa_HH12	0x54
#define ATE_atmn_aa_HH2		0x55
#define ATE_atmn_aa_HH21	0x56
#define ATE_atmn_aa_HH22	0x57
#define ATE_atmn_aa_C		0x58
#define ATE_atmn_aa_CA		0x59
#define ATE_atmn_aa_CB		0x5a
#define ATE_atmn_aa_CG		0x5b
#define ATE_atmn_aa_CG1		0x5c
#define ATE_atmn_aa_CG2		0x5d
#define ATE_atmn_aa_CD		0x5e
#define ATE_atmn_aa_CD1		0x5f
#define ATE_atmn_aa_CD2		0x60
#define ATE_atmn_aa_CE		0x61
#define ATE_atmn_aa_CE1		0x62
#define ATE_atmn_aa_CE2		0x63
#define ATE_atmn_aa_CE3		0x64
#define ATE_atmn_aa_CZ		0x65
#define ATE_atmn_aa_CZ2		0x66
#define ATE_atmn_aa_CZ3		0x67
#define ATE_atmn_aa_CH2		0x68
#define ATE_atmn_aa_N		0x69
#define ATE_atmn_aa_ND1		0x6a
#define ATE_atmn_aa_ND2		0x6b
#define ATE_atmn_aa_NE		0x6c
#define ATE_atmn_aa_NE1		0x6d
#define ATE_atmn_aa_NE2		0x6e
#define ATE_atmn_aa_NZ		0x6f
#define ATE_atmn_aa_NH1		0x70
#define ATE_atmn_aa_NH2		0x71
#define ATE_atmn_aa_O		0x72
#define ATE_atmn_aa_OG		0x73
#define ATE_atmn_aa_OG1		0x74
#define ATE_atmn_aa_OD1		0x75
#define ATE_atmn_aa_OD2		0x76
#define ATE_atmn_aa_OE1		0x77
#define ATE_atmn_aa_OE2		0x78
#define ATE_atmn_aa_OH		0x79
#define ATE_atmn_aa_OXT		0x7a
#define ATE_atmn_aa_SG		0x7b
#define ATE_atmn_aa_SD		0x7c

// define atom names for nucleic acids (n = 53).
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#define ATE_atmn_na_H1		0xa2
#define ATE_atmn_na_H1p		0xa3
#define ATE_atmn_na_H2		0xa4
#define ATE_atmn_na_H21		0xa5
#define ATE_atmn_na_H22		0xa6
#define ATE_atmn_na_H2p1	0xa7
#define ATE_atmn_na_H2p2	0xa8
#define ATE_atmn_na_H3		0xa9
#define ATE_atmn_na_H3T		0xaa
#define ATE_atmn_na_H3p		0xab
#define ATE_atmn_na_H41		0xac
#define ATE_atmn_na_H42		0xad
#define ATE_atmn_na_H4p		0xae
#define ATE_atmn_na_H5		0xaf
#define ATE_atmn_na_H5T		0xb0
#define ATE_atmn_na_H5p1	0xb1
#define ATE_atmn_na_H5p2	0xb2
#define ATE_atmn_na_H6		0xb3
#define ATE_atmn_na_H61		0xb4
#define ATE_atmn_na_H62		0xb5
#define ATE_atmn_na_H71		0xb6
#define ATE_atmn_na_H72		0xb7
#define ATE_atmn_na_H73		0xb8
#define ATE_atmn_na_H8		0xb9
#define ATE_atmn_na_HOp2	0xba
#define ATE_atmn_na_C1p		0xbb
#define ATE_atmn_na_C2		0xbc
#define ATE_atmn_na_C2p		0xbd
#define ATE_atmn_na_C3p		0xbe
#define ATE_atmn_na_C4		0xbf
#define ATE_atmn_na_C4p		0xc0
#define ATE_atmn_na_C5		0xc1
#define ATE_atmn_na_C5p		0xc2
#define ATE_atmn_na_C6		0xc3
#define ATE_atmn_na_C7		0xc4
#define ATE_atmn_na_C8		0xc5
#define ATE_atmn_na_N1		0xc6
#define ATE_atmn_na_N2		0xc7
#define ATE_atmn_na_N3		0xc8
#define ATE_atmn_na_N4		0xc9
#define ATE_atmn_na_N6		0xca
#define ATE_atmn_na_N7		0xcb
#define ATE_atmn_na_N9		0xcc
#define ATE_atmn_na_O1P		0xcd
#define ATE_atmn_na_O2		0xce
#define ATE_atmn_na_O2P		0xcf
#define ATE_atmn_na_O2p		0xd0
#define ATE_atmn_na_O3p		0xd1
#define ATE_atmn_na_O4		0xd2
#define ATE_atmn_na_O4p		0xd3
#define ATE_atmn_na_O5p		0xd4
#define ATE_atmn_na_O6		0xd5
#define ATE_atmn_na_P		0xd6

/// e_atom_name is used to map the residue/atom name codes to text strings used in AMBER param files.

struct e_atom_name
{
	int id;
	char * name;
};

struct e_vdw_param
{
	const char * name;
	f64 param_r;
	f64 param_e;
};

/*################################################################################################*/

#endif	// TAB_MM_DEFAULT_H

// eof
