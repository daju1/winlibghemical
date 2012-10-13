// BOND.H : the bond-related classes.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef BOND_H
#define BOND_H

#include "libconfig.h"

class bondtype;

class bond;

/*################################################################################################*/

class atom;	// atom.h

#include "libdefine.h"
#include "typedef.h"

#include <list>
#include <vector>
using namespace std;

/*################################################################################################*/

#define BONDTYPE_CNJGTD 0
#define BONDTYPE_SINGLE 1
#define BONDTYPE_DOUBLE 2
#define BONDTYPE_TRIPLE 3
#define BONDTYPE_QUADRP 4	// never needed!!!

/// A bondtype class.

class bondtype
{
	private:
	public:
	
	i32s type;
	
	static const char * string[5];
	
	static const char symbol1[5];
	static const char symbol2[5];
	
	public:
	
	static bondtype current_bondtype;	///< This is the current element that user has selected.
	
	public:
	
	bondtype(void);
	bondtype(char);		///< By symbols.
	bondtype(i32s);		///< By number codes.
	~bondtype(void);
	
	const char * GetString(void) const;
	
	i32s GetValue(void) const;
	char GetSymbol1(void) const;
	char GetSymbol2(void) const;
	
	void operator++(void);
	void operator--(void);
	
	friend ostream & operator<<(ostream &, bondtype &);
};

/*################################################################################################*/

#define BOND_NFLAGS 3	// the number of flags can be increased if needed...

// ABOUT FLAGS:
// ^^^^^^^^^^^^
// 0 is used generally in typerule tests.
// 1 is also used in "RingSize" and "Ring" typerule tests.
// 2 is used by sequencebuilder::FindPath() at least...

/// A bond class.
/** Will store information about a bond, including the atoms that the bond connects, 
and bondtype information. */

class bond
{
//	protected:
	public:		// TODO : not properly divided in public/protected data.
	
	atom * atmr[2];
	bondtype bt;
	
	vector<bool> flags;
	
/// A temporary variable used when creating molecular mechanics energy terms.
/** This variable is not otherwise used or initialized, and it is not saved. */
	i32s tmp_bt1_index;
	
	bool do_not_render_TSS_fixmelater;	///< this is only for visualize breaking bonds in TSS!!!
	
	public:
	
	bond(void);
	bond(atom *, atom *, bondtype);
	bond(const bond &);
	~bond(void);
	
	bool operator<(const bond &) const;	///< Using molecule numbers.
	bool operator==(const bond &) const;	///< Using atom pointers.
};

/*################################################################################################*/

typedef list<bond>::iterator iter_bl;		// bl = bond list

typedef bond * ref_bond;

/*################################################################################################*/

#endif	// BOND_H

// eof
