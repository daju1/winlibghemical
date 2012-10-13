// BOND.CPP

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "bond.h"

#include "atom.h"

/*################################################################################################*/

bondtype bondtype::current_bondtype = bondtype('S');

const char * bondtype::string[5] =
{
	"Conjugated", "Single", "Double", "Triple", "Quadruple"
};

const char bondtype::symbol1[5] = { 'C', 'S', 'D', 'T', 'Q' };
const char bondtype::symbol2[5] = { '~', '-', '=', '#', '%' };

bondtype::bondtype(void)
{
	type = NOT_DEFINED;
}

bondtype::bondtype(char p1)
{
	type = NOT_DEFINED;
	while (type++ < 3)
	{
		if (symbol1[type] == p1) return;
		if (symbol2[type] == p1) return;
	}
	
	type = NOT_DEFINED;
}

bondtype::bondtype(i32s p1)
{
	type = p1;
}

bondtype::~bondtype(void)
{
}

const char * bondtype::GetString(void) const
{
	return string[type];
}

i32s bondtype::GetValue(void) const
{
	return type;
}

char bondtype::GetSymbol1(void) const
{
	if (type < 0)
	{
		cout << "trying to use invalid bondtype..." << endl;
		return 'S';
	}
	
	return symbol1[type];
}

char bondtype::GetSymbol2(void) const
{
	if (type < 0)
	{
		cout << "trying to use invalid bondtype..." << endl;
		return '-';
	}
	
	return symbol2[type];
}

void bondtype::operator++(void)
{
	if (++type > 3) type = 0;
}

void bondtype::operator--(void)
{
	if (--type < 0) type = 3;
}

ostream & operator<<(ostream & p1, bondtype & p2)
{
	switch (p2.type)
	{
		case BONDTYPE_CNJGTD:	p1 << "conjugated"; break;
		case BONDTYPE_SINGLE:	p1 << "single"; break;
		case BONDTYPE_DOUBLE:	p1 << "double"; break;
		case BONDTYPE_TRIPLE:	p1 << "triple"; break;
	        case BONDTYPE_QUADRP:	p1 << "quadruple"; break;
	        default:		p1 << "???";
	}
	
	return p1;
}

/*################################################################################################*/

bond::bond(void)
{
	atmr[0] = atmr[1] = NULL;
	for (i32s n1 = 0;n1 < BOND_NFLAGS;n1++) flags.push_back(false);
	
do_not_render_TSS_fixmelater = false;					// a temporary setup, for transition_state_search only...
}

bond::bond(atom * p1, atom * p2, bondtype p3)
{
	atmr[0] = p1; atmr[1] = p2; bt = p3;
	for (i32s n1 = 0;n1 < BOND_NFLAGS;n1++) flags.push_back(false);
	
do_not_render_TSS_fixmelater = false;					// a temporary setup, for transition_state_search only...
}

bond::bond(const bond & p1)
{
	atmr[0] = p1.atmr[0]; atmr[1] = p1.atmr[1]; bt = p1.bt;
	for (i32s n1 = 0;n1 < BOND_NFLAGS;n1++) flags.push_back(p1.flags[n1]);
	
do_not_render_TSS_fixmelater = p1.do_not_render_TSS_fixmelater;		// a temporary setup, for transition_state_search only...
}

bond::~bond(void)
{
}

// bonds will be sorted according to the molecule numbers (should be the same for both atoms).
// sorting the container will then arrange the bonds of a molecule in a continuous blocks.

bool bond::operator<(const bond & p1) const
{
	// assume that the molecule numbers of both atoms in a bond object are identical!!!
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	return (atmr[0]->id[0] < p1.atmr[0]->id[0]);
}

// for bonds, equality is tested using atom pointers. if you need to find a certain bond,
// just make a temporary bond and use that to find the original one (using STL's find-function).

bool bond::operator==(const bond & p1) const
{
	if (atmr[0] == p1.atmr[0] && atmr[1] == p1.atmr[1]) return true;
	if (atmr[0] == p1.atmr[1] && atmr[1] == p1.atmr[0]) return true;
	
	return false;
}

/*################################################################################################*/

// eof
