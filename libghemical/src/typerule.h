// TYPERULE.H : a class for atomtype detection...

// Copyright (C) 1998 Tommi Hassinen, Geoff Hutchison.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "libconfig.h"

#ifndef TYPERULE_H
#define TYPERULE_H

class typerule;
struct tr_subrule;

/*################################################################################################*/

#include "atom.h"
#include "bond.h"

#include "model.h"

/*################################################################################################*/

/**	A typerule class for atom types.

	Will parse, store and check atom type rulesets written as text. For example a typerule 
	for formaldehyde carbon is written (=O,-H,-H) which means "##bonded to oxygen with a 
	double bond and to 2 hydrogens with single bonds". The trick is that typerules can be 
	recursive: for the acetaldehyde carbon 2 we can write a rule (=O,-C(-H,-H,-H),-H) which 
	means "##bonded to oxygen with a double bond, and with single bonds to a hydrogen and a 
	carbon, which is further bonded to 3 hydrogens with single bonds". Typerules can also 
	contain other rules like number of bonds, ring sizes and rings. For example we can write 
	the following typerule for phenolic hydrogen (-O(-C([~C~C~C~C~C~]))) which means 
	"##single-bonded to oxygen, which is single-bonded to carbon, which is part of a 
	6-member carbon ring connected with aromatic bonds". It is also possible to use 
	wildcards for both atoms and bonds.
	
	The "##wildcard"-mark for elements is '*' and for bondtypes '?'. Does it work for bonds??? 
	More exact rules must be tested before less exact rules; (-C,-*) but NOT (-*,-C)!!!
	
	Here is no test for the element of the atom itself; just test that separately...
*/

class typerule
{
	protected:
	public:
	
	i32s first;
	char buffer[256];
	
	vector<signed char *> ring_vector;
	vector<tr_subrule> sr_vector;
	
	public:
	
	typerule(void);
	typerule(istream *, ostream *);
	typerule(const typerule &);
	~typerule(void);
	
	i32s GetFirst(void) { return first; }
	
	bool Check(model *, atom *, i32s);
	friend ostream & operator<<(ostream &, const typerule &);
	
	private:
	
	i32s ReadSubRule(istream *, ostream *);
	bool CheckRules(model *, atom *, i32s, i32s);
	public:
	
	void PrintSubRules(ostream &, i32s) const;
	void PrintRing(ostream &, signed char *) const;
};

struct tr_subrule
{
	enum
	{
		BondedTo = 0,			// -*, ~*, =*, #*	the main typerule!!!
		Valence = 1,			// vl=???		valence electrons of all bonds; integer.
		FormalCharge = 2,		// fc=???		formal charge set by user; integer.
		NumBondsSingle = 3,		// bS=???		the count of S-bonds.
		NumBondsCnjgtd = 4,		// bC=???		the count of C-bonds.
		NumBondsDouble = 5,		// bD=???		the count of D-bonds.
		NumBondsTriple = 6,		// bT=???		the count of T-bonds.
		NeighborsAll = 7,		// nA=???		the count of neighbor atoms.
		NeighborsHeavy = 8,		// nB=???		the count of neighbor heavy-atoms.
		NeighborsHydrogen = 9,		// nH=???		the count of neighbor H-atoms.
		NeighborsHighElNeg = 10,	// nX=???		the count of neighbor halogen/oxygen-atoms.
		RingSize = 11,			// rs=???		is part of a ring system of certain size.
		Ring = 12			// [-*-*-]		is part of a described ring system.
	} type;
	
	bondtype bt;
	element el;
	
	i32s data;
	i32s next;
	i32s sub;
};

/*################################################################################################*/

#endif	// TYPERULE_H

// eof
