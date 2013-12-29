// ATOM.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "atom.h"

#include "bond.h"

/*################################################################################################*/

element element::current_element = element("C");

const char element::string[ELEMENT_SYMBOLS][32] =
{
	"hydrogen", "helium", "lithium", "beryllium", "boron",
	"carbon", "nitrogen", "oxygen", "fluorine", "neon",				// 10
	"sodium", "magnesium", "aluminium", "silicon", "phosphorus",
	"sulphur", "chlorine", "argon", "potassium", "calcium",				// 20
	"scandium", "titanium", "vanadium", "chromium", "manganese",
	"iron", "cobalt", "nickel", "copper", "zinc",					// 30
	"gallium", "germanium", "arsenic", "selenium", "bromine",
	"krypton", "rubidium", "strontium", "yttrium", "zirconium",			// 40
	"niobium", "molybdenum", "technetium", "ruthenium", "rhodium",
	"palladium", "silver", "cadmium", "indium", "tin",				// 50
	"antimony", "tellurium", "iodine", "xenon", "cesium",
	"barium", "lanthanum", "cerium", "praseodymium", "neodymium",			// 60
	"promethium", "samarium", "europium", "gadolinium", "terbium",
	"dysprosium", "holmium", "erbium", "thulium", "ytterbium",			// 70
	"lutetium", "hafnium", "tantalum", "tungsten", "rhenium",
	"osmium", "iridium", "platinum", "gold", "mercury",				// 80
	"thallium", "lead", "bismuth", "polonium", "astatine",
	"radon", "francium", " radium", "actinium", "thorium",				// 90
	"protactinium", "uranium", "neptunium", "plutionium", "americium",
	"curium", "berkelium", "californium", "einsteinium", "fermium",			// 100
	"mendelevium", "nobelium", "lawrencium", "rutherfordium", "dubnium",
	"seaborgium", "bohrium", "hassium", "meitnerium", "ununnilium"			// 110
};

const char element::symbol[ELEMENT_SYMBOLS][4] =
{
	"H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne",
	"Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca",		// 20
	"Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
	"Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr",		// 40
	"Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn",
	"Sb", "Te", "I", "Xe", "Cs", "Ba", "La", "Ce", "Pr", "Nd",		// 60
	"Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb",
	"Lu", "Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg",		// 80
	"Tl", "Pb", "Bi", "Po", "At", "Rn", "Fr", "Ra", "Ac", "Th",
	"Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm",		// 100
	"Md", "No", "Lr", "Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Uun"
};

// These are the defaults--modified via the prefs class.
// REMOVE CONST AND MODIFY THESE DIRECTLY?!?!?!?!

const fGL element::color[ELEMENT_SYMBOLS][3] =
{
    { 1.00, 1.00, 1.00 }, { 0.85, 1.00, 1.00 }, { 0.80, 0.50, 1.00 },
    { 0.76, 1.00, 0.00 }, { 1.00, 0.71, 0.71 }, { 0.56, 0.56, 0.56 },	// 6
    { 0.18, 0.31, 0.97 }, { 1.00, 0.05, 0.05 }, { 0.56, 0.87, 0.31 },
    { 0.70, 0.89, 0.96 }, { 0.67, 0.36, 0.95 }, { 0.54, 1.00, 0.00 },	// 12
    { 0.75, 0.65, 0.65 }, { 0.94, 0.78, 0.62 }, { 1.00, 0.50, 0.00 },
    { 1.00, 1.00, 0.19 }, { 0.12, 0.94, 0.12 }, { 0.50, 0.82, 0.89 },	// 18
    { 0.56, 0.25, 0.83 }, { 0.24, 1.00, 0.00 }, { 0.90, 0.90, 0.90 },
    { 0.75, 0.76, 0.78 }, { 0.65, 0.65, 0.67 }, { 0.54, 0.60, 0.78 },	// 24
    { 0.61, 0.48, 0.78 }, { 0.87, 0.40, 0.20 }, { 0.94, 0.56, 0.62 },
    { 0.31, 0.81, 0.31 }, { 0.78, 0.50, 0.20 }, { 0.49, 0.50, 0.69 },	// 30
    { 0.76, 0.56, 0.56 }, { 0.40, 0.56, 0.56 }, { 0.74, 0.50, 0.89 },
    { 1.00, 0.63, 0.00 }, { 0.65, 0.16, 0.16 }, { 0.36, 0.72, 0.82 },	// 36
    { 0.44, 0.18, 0.69 }, { 0.00, 1.00, 0.00 }, { 0.58, 1.00, 1.00 },
    { 0.58, 0.88, 0.88 }, { 0.45, 0.76, 0.79 }, { 0.33, 0.71, 0.71 },	// 42
    { 0.23, 0.62, 0.62 }, { 0.14, 0.56, 0.56 }, { 0.04, 0.49, 0.55 },
    { 0.00, 0.41, 0.52 }, { 0.75, 0.75, 0.75 }, { 1.00, 0.85, 0.56 },	// 48
    { 0.65, 0.46, 0.45 }, { 0.40, 0.50, 0.50 }, { 0.62, 0.39, 0.71 },
    { 0.83, 0.48, 0.00 }, { 0.58, 0.00, 0.58 }, { 0.26, 0.62, 0.69 },	// 54
    { 0.34, 0.09, 0.56 }, { 0.00, 0.79, 0.00 }, { 0.44, 0.83, 1.00 },
    { 1.00, 1.00, 0.78 }, { 0.85, 1.00, 0.78 }, { 0.78, 1.00, 0.78 },	// 60
    { 0.64, 1.00, 0.78 }, { 0.56, 1.00, 0.78 }, { 0.38, 1.00, 0.78 },
    { 0.27, 1.00, 0.78 }, { 0.19, 1.00, 0.78 }, { 0.12, 1.00, 0.78 },	// 66
    { 0.00, 1.00, 0.61 }, { 0.00, 0.90, 0.46 }, { 0.00, 0.83, 0.32 },
    { 0.00, 0.75, 0.22 }, { 0.00, 0.67, 0.14 }, { 0.30, 0.76, 1.00 },	// 72
    { 0.30, 0.65, 1.00 }, { 0.13, 0.58, 0.84 }, { 0.15, 0.49, 0.67 },
    { 0.15, 0.40, 0.59 }, { 0.09, 0.33, 0.53 }, { 0.81, 0.81, 0.87 },	// 78
    { 1.00, 0.81, 0.13 }, { 0.72, 0.72, 0.81 }, { 0.65, 0.33, 0.30 },
    { 0.34, 0.35, 0.38 }, { 0.62, 0.31, 0.71 }, { 0.67, 0.36, 0.00 },	// 84
    { 0.46, 0.31, 0.27 }, { 0.26, 0.51, 0.59 }, { 0.26, 0.00, 0.40 },
    { 0.00, 0.49, 0.00 }, { 0.44, 0.67, 0.98 }, { 0.00, 0.73, 1.00 },	// 90
    { 0.00, 0.63, 1.00 }, { 0.00, 0.56, 1.00 }, { 0.00, 0.50, 1.00 },
    { 0.00, 0.42, 1.00 }, { 0.33, 0.36, 0.95 }, { 0.47, 0.36, 0.89 },	// 96
    { 0.54, 0.31, 0.89 }, { 0.63, 0.21, 0.83 }, { 0.70, 0.12, 0.83 },
    { 0.70, 0.12, 0.73 }, { 0.70, 0.05, 0.65 }, { 0.74, 0.05, 0.53 },	// 102
    { 0.78, 0.00, 0.40 }, { 0.80, 0.00, 0.35 }, { 0.82, 0.00, 0.31 },
    { 0.85, 0.00, 0.27 }, { 0.88, 0.00, 0.22 }, { 0.90, 0.00, 0.18 },	// 108
    { 0.92, 0.00, 0.15 }, { 0.95, 0.00, 0.11 }
};

// IUPAC recognized masses rounded to 0.001 where appropriate
// From _Pure Appl. Chem., 71 (1999) 1593-1607.
// these are in normal atomic mass units... TH

const fGL element::mass[ELEMENT_SYMBOLS] =
{
	1.008, 4.003, 6.941, 9.012, 10.812,
	12.011, 14.007, 15.999, 18.998, 20.180,			// 10
	22.990, 24.305, 26.982, 28.086, 30.974,
	32.067, 35.453, 39.948, 39.098, 40.078,			// 20
	44.956, 47.867, 50.942, 51.996, 54.938,
	55.845, 58.933, 58.693, 63.546, 65.39,			// 30
	69.723, 72.61, 74.922, 78.96, 79.904,
	83.80, 85.468, 87.62, 88.906, 91.224,			// 40
	92.906, 95.94, 98.0, 101.07, 102.906,
	106.42, 107.868, 112.412, 114.818, 118.711,		// 50
	121.760, 127.60, 126.904, 131.29, 132.905,
	137.328, 138.906, 140.116, 140.908, 144.24,		// 60
	145.0, 150.36, 151.964, 157.25, 158.925,
	162.50, 164.930, 167.26, 168.934, 173.04,		// 70
	174.967, 178.49, 180.948, 183.84, 186.207,
	190.23, 192.217, 195.078, 196.967, 200.59,		// 80
	204.383, 207.2, 208.980, 209.0, 210.0,
	222.0, 223.0, 226.0, 227.0, 232.038,			// 90
	231.036, 238.029, 237.0, 244.0, 243.0,
	247.0, 247.0, 251.0, 252.0, 257.0,			// 100
	258.0, 259.0, 262.0, 261.0, 262.0,
	263.0, 264.0, 265.0, 268.0, 269.0			// 110
};

// van der Waals radii for graphics--these determine the size of the spheres
// These were collected from the elements list in Open Babel
// (0.170 nm seems to be an "unknown" value

const fGL element::vdwr[ELEMENT_SYMBOLS] =
{
	0.120, 0.140, 0.180, 0.170, 0.208, 0.170, 0.155, 0.150, 0.155, 0.160,
	0.231, 0.170, 0.205, 0.210, 0.185, 0.180, 0.181, 0.191, 0.280, 0.197,		// 20
	0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.160, 0.140, 0.140,
	0.190, 0.170, 0.185, 0.190, 0.190, 0.200, 0.244, 0.170, 0.170, 0.170,		// 40
	0.170, 0.170, 0.170, 0.170, 0.170, 0.163, 0.172, 0.160, 0.190, 0.220,
	0.220, 0.210, 0.198, 0.220, 0.262, 0.170, 0.170, 0.170, 0.170, 0.170,		// 60
	0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170,
	0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.175, 0.166, 0.155,		// 80
	0.200, 0.202, 0.240, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170,
	0.170, 0.190, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170,		// 100
	0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170, 0.170
};

element::element(void)
{
	atomic_number = NOT_DEFINED;	// set to a dummy atom by default!!!
}

// unknown symbols are thought to be "wildcard elements" which MUST have their atomic numbers
// equal to minus one (that is, NOT_DEFINED). if this logic is changed, some bad problems will
// emerge in the ring search and in typerules...

element::element(char * p1)
{
	i32s n1 = 0;
	while (n1 < ELEMENT_SYMBOLS && strcmp(symbol[n1], p1)) n1++;
	
	if (n1 < ELEMENT_SYMBOLS) atomic_number = n1 + 1;
	else atomic_number = NOT_DEFINED;
}

element::element(i32s p1)
{
	if (p1 < 1 || p1 > ELEMENT_SYMBOLS) atomic_number = NOT_DEFINED;
	else atomic_number = p1;
}

element::~element(void)
{
}

const char * element::GetString(void) const
{
	if (atomic_number == NOT_DEFINED) return "DUMMY_ATOM";
	else return string[atomic_number - 1];
}

const i32s element::GetAtomicNumber(void) const
{
	return atomic_number;
}

// the symbol for atomic_number NOT_DEFINED (= -1) might affect the atomtype rule class "typerule",
// but it seems to be OK to return symbol "Du" (a "wildcard" setting is selected for all unknown ones).

const char * element::GetSymbol(void) const
{
	if (atomic_number == NOT_DEFINED) return "Du";
	else return symbol[atomic_number - 1];
}

const fGL * element::GetColor(void) const
{
	static fGL not_defined_color[3] = { 0.5, 0.4, 0.3 };
	
	if (atomic_number == NOT_DEFINED) return not_defined_color;	// just return something usable also in this case...
	else return color[atomic_number - 1];
}

fGL element::GetAtomicMass(void) const
{
	if (atomic_number == NOT_DEFINED) return -1.0;
	else return mass[atomic_number - 1];
}

fGL element::GetVDWRadius(void) const
{
	if (atomic_number == NOT_DEFINED) return -1.0;
	else return vdwr[atomic_number - 1];
}

/*
const fGL * element::GetColor(prefs *user_prefs) const
{
	static fGL not_defined_color[3] = { 0.1, 0.1, 0.1 };
	
	if (atomic_number == NOT_DEFINED) return not_defined_color;

	if (user_prefs == NULL)
	  return color[atomic_number - 1];
	else
	  {
	    char keybuffer[32];
	    ostrstream fullkey(keybuffer, sizeof(keybuffer));
	    fullkey << "ElementColor/" << atomic_number << ends;
	    return user_prefs->ColorRGB(keybuffer, color[atomic_number - 1]);
	  }
}*/

/*
fGL element::GetVDWRadius(prefs *user_prefs) const
{
	if (atomic_number == NOT_DEFINED) return -1.0;

	if (user_prefs == NULL)
	  return vdwr[atomic_number - 1];
	else
	  {
	    char keybuffer[32];
	    ostrstream fullkey(keybuffer, sizeof(keybuffer));
	    fullkey << "ElementMass/" << atomic_number - 1 << ends;
	    return user_prefs->Double(keybuffer, vdwr[atomic_number - 1]);
	  }
}*/

void element::operator++(void)
{
	if (++atomic_number > ELEMENT_SYMBOLS) atomic_number = 1;
}

void element::operator--(void)
{
	if (--atomic_number < 1) atomic_number = ELEMENT_SYMBOLS;
}

/*################################################################################################*/

crec::crec(void)
{
	atmr = NULL;
	bndr = NULL;
}

crec::crec(atom * p1, bond * p2)
{
	atmr = p1;
	bndr = p2;
}

crec::~crec(void)
{
}

bool crec::operator==(const crec & p1) const
{
	return (bndr == p1.bndr);
}

/*################################################################################################*/

atom::atom(void)
{
	mdl = NULL;
}

atom::atom(element p1, const fGL * p2, i32u p3)
{
	mdl = NULL;
	
	el = p1;
	formal_charge = 0;
	
	atmtp = NOT_DEFINED;
	atmtp_E = NOT_DEFINED;
	atmtp_s[0] = 0;			// initialize to a zero-length string.
	
	charge = 0.0;
	mass = el.GetAtomicMass();
	
	vdwr = el.GetVDWRadius();
	
	fGL data[3] = { 0.0, 0.0, 0.0 };	// default coordinates...
	if (p2 != NULL) { data[0] = p2[0]; data[1] = p2[1]; data[2] = p2[2]; }
	
	crd_table_size_loc = p3;
	if (crd_table_size_loc < 1) { cout << "BUG: invalid crd_table_size_loc found." << endl; exit(EXIT_FAILURE); }
	
	crd_table = new fGL[crd_table_size_loc * 3];
	for (i32u n1 = 0;n1 < crd_table_size_loc;n1++)		// be sure to initialize the whole array -> valid floats!!!
	{
		crd_table[n1 * 3 + 0] = data[0];
		crd_table[n1 * 3 + 1] = data[1];
		crd_table[n1 * 3 + 2] = data[2];
	}
	
	for (i32u i1 = 0;i1 < 4;i1++) id[i1] = NOT_DEFINED;
	builder_res_id = NOT_DEFINED;
	
	index = NOT_DEFINED;
	varind = NOT_DEFINED;
	
	flags = 0;
	ecomp_grp_i = NOT_DEFINED;
}

atom::atom(const atom & p1)
{
	mdl = NULL;	// strict...
	
	el = p1.el;
	formal_charge = p1.formal_charge;
	
	atmtp = p1.atmtp;
	atmtp_E = p1.atmtp_E;
	strcpy(atmtp_s, p1.atmtp_s);
	
	charge = p1.charge;
	mass = p1.mass;
	
	vdwr = p1.vdwr;
	
	cr_list = p1.cr_list;
	
	crd_table_size_loc = p1.crd_table_size_loc;
	if (crd_table_size_loc < 1) { cout << "BUG: invalid crd_table_size_loc found." << endl; exit(EXIT_FAILURE); }
	
	crd_table = new fGL[crd_table_size_loc * 3];
	for (i32u n1 = 0;n1 < crd_table_size_loc;n1++)		// copy as floats -> the arrays always should contain valid data!!!
	{
		crd_table[n1 * 3 + 0] = p1.crd_table[n1 * 3 + 0];
		crd_table[n1 * 3 + 1] = p1.crd_table[n1 * 3 + 1];
		crd_table[n1 * 3 + 2] = p1.crd_table[n1 * 3 + 2];
	}
	
	for (i32u n1 = 0;n1 < 4;n1++) id[n1] = p1.id[n1];
	builder_res_id = p1.builder_res_id;
	
	index = p1.index;
	varind = p1.varind;
	
	flags = p1.flags;
	ecomp_grp_i = p1.ecomp_grp_i;
}

atom::~atom(void)
{
	delete[] crd_table;
}

const fGL * atom::GetCRD(i32u cs)
{
	if (cs >= crd_table_size_loc)
	{
		cout << "BUG: cs overflow at atom::GetCRD()." << endl;
		exit(EXIT_FAILURE);
	}
	
	return & (crd_table[cs * 3]);
}

void atom::SetCRD(i32s cs, fGL x, fGL y, fGL z)
{
	if (cs < 0)
	{
		for (i32u n1 = 0;n1 < crd_table_size_loc;n1++)		// NOT_DEFINED -> set for all csets...
		{
			crd_table[n1 * 3 + 0] = x;
			crd_table[n1 * 3 + 1] = y;
			crd_table[n1 * 3 + 2] = z;
		}
	}
	else
	{
		if (cs >= (i32s) crd_table_size_loc)
		{
			cout << "BUG: cs overflow at atom::SetCRD()" << endl;
			exit(EXIT_FAILURE);
		}
		
		crd_table[cs * 3 + 0] = x;
		crd_table[cs * 3 + 1] = y;
		crd_table[cs * 3 + 2] = z;
	}
}

// atoms will be sorted according to their molecule, chain and residue numbers.
// sorting the container will then arrange molecules/chains/residues in continuous blocks.

bool atom::operator<(const atom & p1) const
{
	for (i32s n1 = 0;n1 < 3;n1++)
	{
		if (id[n1] != p1.id[n1]) return (id[n1] < p1.id[n1]);
	}
	
	return false;
}

// for atoms, equality is tested using pointers -> you can find the
// iterator (using STL's find-function) if the pointer is known.

bool atom::operator==(const atom & p1) const
{
	return (this == (& p1));
}

/*################################################################################################*/

// eof
