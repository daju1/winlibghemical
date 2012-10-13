// TYPERULE.CPP

// Copyright (C) 1998 Tommi Hassinen, Geoff Hutchison.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "typerule.h"

#include "libdefine.h"

#include <string.h>

#include <iomanip>
#include <iostream>
#include <strstream>
using namespace std;

/*################################################################################################*/

typerule::typerule(void)
{
	first = NOT_DEFINED;
}

typerule::typerule(istream * istr, ostream * ostr)
{
	first = ReadSubRule(istr, ostr);
}

typerule::typerule(const typerule & p1)
{
	first = p1.first;
	sr_vector = p1.sr_vector;
	
	ring_vector.resize(p1.ring_vector.size());
	for (i32u n1 = 0;n1 < ring_vector.size();n1++)
	{
		ring_vector[n1] = new signed char[strlen((const char *) p1.ring_vector[n1]) + 1];
		strcpy((char *) ring_vector[n1], (const char *) p1.ring_vector[n1]);
	}
}

typerule::~typerule(void)
{
	for (i32u n1 = 0;n1 < ring_vector.size();n1++) delete[] ring_vector[n1];
}

// NOTE!!! this uses bond::flags[FLAG] but also bond::flags[FLAG+1] in "RingSize" and "Ring" tests!!!
// NOTE!!! this uses bond::flags[FLAG] but also bond::flags[FLAG+1] in "RingSize" and "Ring" tests!!!
// NOTE!!! this uses bond::flags[FLAG] but also bond::flags[FLAG+1] in "RingSize" and "Ring" tests!!!

bool typerule::Check(model * mdl, atom * atom, i32s flag)
{
	if (!sr_vector.size()) return true;		// an empty rule always fits...
	return CheckRules(mdl, atom, flag, first);
}

ostream & operator<<(ostream & p1, const typerule & p2)
{
	if (p2.sr_vector.size() != 0) p2.PrintSubRules(p1, p2.first);
	return p1;
}

i32s typerule::ReadSubRule(istream * istr, ostream * ostr)
{
	vector<i32s> index_vector;
	if (istr->peek() == '(') istr->get();
	else
	{
	//	cout << "WARNING : typerule::ReadSubRule() failed." << endl;	// why so many of these at start???
		return NOT_DEFINED;
	}
	
	while (true)
	{
		if (istr->peek() == ')') break;
		else if (istr->peek() == ',') istr->get();
		else
		{
			tr_subrule newsr;
			newsr.data = NOT_DEFINED;
			newsr.next = NOT_DEFINED;
			newsr.sub = NOT_DEFINED;
			
			if (istr->peek() == 'v')		// vl
			{
				istr->get();
				if (istr->peek() != 'l')
				{
					cout << "WARNING : typerule::ReadSubRule() failed (vl)." << endl;
					return NOT_DEFINED;
				}
				
				newsr.type = tr_subrule::Valence;
				istr->getline(buffer, sizeof(buffer), '=');
				(* istr) >> newsr.data;
			}
			else if (istr->peek() == 'f')		// fc
			{
				istr->get();
				if (istr->peek() != 'c')
				{
					cout << "WARNING : typerule::ReadSubRule() failed (fc)." << endl;
					return NOT_DEFINED;
				}
				
				newsr.type = tr_subrule::FormalCharge;
				istr->getline(buffer, sizeof(buffer), '=');
				(* istr) >> newsr.data;
			}
			else if (istr->peek() == 'b')
			{
				istr->get();
				switch (istr->peek())
				{
					case 'S': newsr.type = tr_subrule::NumBondsSingle; break;
					case 'C': newsr.type = tr_subrule::NumBondsCnjgtd; break;
					case 'D': newsr.type = tr_subrule::NumBondsDouble; break;
					case 'T': newsr.type = tr_subrule::NumBondsTriple; break;
					default:
					cout << "WARNING : typerule::ReadSubRule() failed (b?)." << endl;
					return NOT_DEFINED;
				}
				
				istr->getline(buffer, sizeof(buffer), '=');
				(* istr) >> newsr.data;
			}
			else if (istr->peek() == 'n')
			{
				istr->get();
				switch (istr->peek())
				{
					case 'A': newsr.type = tr_subrule::NeighborsAll; break;
					case 'B': newsr.type = tr_subrule::NeighborsHeavy; break;
					case 'H': newsr.type = tr_subrule::NeighborsHydrogen; break;
					case 'X': newsr.type = tr_subrule::NeighborsHighElNeg; break;
					default:
					cout << "WARNING : typerule::ReadSubRule() failed (n?)." << endl;
					return NOT_DEFINED;
				}
				
				istr->getline(buffer, sizeof(buffer), '=');
				(* istr) >> newsr.data;
			}
			else if (istr->peek() == 'r')
			{
				newsr.type = tr_subrule::RingSize;
				istr->getline(buffer, sizeof(buffer), '=');
				(* istr) >> newsr.data;
			}
			else if (istr->peek() == '[')
			{
				newsr.type = tr_subrule::Ring; istr->get();
				for (i32u n1 = 0;n1 < sizeof(buffer);n1++) buffer[n1] = 0;
				istr->getline(buffer, sizeof(buffer), ']');
				
				i32s tmp1[2] = { strlen(buffer), 0 };
				{
				for (i32s n1 = 0;n1 < tmp1[0];n1++)
				{
					if (buffer[n1] == '-') tmp1[1]++;
					if (buffer[n1] == '~') tmp1[1]++;
					if (buffer[n1] == '=') tmp1[1]++;
					if (buffer[n1] == '#') tmp1[1]++;
					if (buffer[n1] == '?') tmp1[1]++;	// wildcard
				}} tmp1[1] += (tmp1[1] - 1);
				
				signed char * newring = new signed char[tmp1[1] + 1];
				newring[tmp1[1]] = 0;
				
				tmp1[0] = 0; char tmp2[3] = { 0, 0, 0 };
				{
				for (i32s n1 = 0;n1 < tmp1[1];n1++)
				{
					if (!(n1 % 2)) newring[n1] = buffer[tmp1[0]++];
					else
					{
						tmp2[0] = buffer[tmp1[0]++];
						if (buffer[tmp1[0]] < 'a' || buffer[tmp1[0]] > 'z') tmp2[1] = 0;
						else tmp2[1] = buffer[tmp1[0]++]; element el = element(tmp2);
						
						bool problem = (el.GetAtomicNumber() == NOT_DEFINED && tmp2[0] != '*');
						if (problem && ostr != NULL)
						{
							(* ostr) << "WARNING !!! Unknown element " << tmp2;
							(* ostr) << " -> using wildcard instead..." << endl;
						}
						
						newring[n1] = (char) el.GetAtomicNumber();
					}
				}
				}
				
				newsr.data = ring_vector.size();
				ring_vector.push_back(newring);
			}
			else
			{
				newsr.type = tr_subrule::BondedTo;
				char tmp1 = (char) istr->get(); newsr.bt = bondtype(tmp1);
				
				bool problem1 = (newsr.bt.GetValue() == NOT_DEFINED && tmp1 != '?');
				if (problem1 && ostr != NULL)
				{
					(* ostr) << "WARNING !!! Unknown bondtype " << tmp1;
					(* ostr) << " -> using wildcard instead..." << endl;
				}
				
				char tmp2[3] = { 0, 0, 0 }; tmp2[0] = (char) istr->get();
				if (istr->peek() >= 'a' && istr->peek() <= 'z') tmp2[1] = (char) istr->get();
				newsr.el = element(tmp2);
				
				bool problem2 = (newsr.el.GetAtomicNumber() == NOT_DEFINED && tmp2[0] != '*');
				if (problem2 && ostr != NULL)
				{
					(* ostr) << "WARNING !!! Unknown element " << tmp2;
					(* ostr) << " -> using wildcard instead..." << endl;
				}
				
				newsr.sub = ReadSubRule(istr, ostr);
			}
			
			index_vector.push_back(sr_vector.size());
			sr_vector.push_back(newsr);
		}
	}
	
	istr->get(); if (!index_vector.size()) return NOT_DEFINED;	// empty rule...
	for (i32s n1 = 0;n1 < ((i32s) index_vector.size()) - 1;n1++) sr_vector[index_vector[n1]].next = index_vector[n1 + 1];
	return index_vector[0];
}

// here we must use flags "flag" and "flag + 1" because the general typerule testing and
// testing for ring rules must be independent of each other!!!!!

// NOTE!!! this uses bond::flags[FLAG] but also bond::flags[FLAG+1] in "RingSize" and "Ring" tests!!!
// NOTE!!! this uses bond::flags[FLAG] but also bond::flags[FLAG+1] in "RingSize" and "Ring" tests!!!
// NOTE!!! this uses bond::flags[FLAG] but also bond::flags[FLAG+1] in "RingSize" and "Ring" tests!!!

bool typerule::CheckRules(model * mdl, atom * atom, i32s flag, i32s rule)
{
	i32s n1; iter_cl it1;
	switch (sr_vector[rule].type)
	{
		case tr_subrule::BondedTo:	n1 = false;	// the result...
		for (it1 = atom->cr_list.begin();it1 != atom->cr_list.end();it1++)
		{
			if ((* it1).bndr->flags[flag]) continue;
			if (sr_vector[rule].el.GetAtomicNumber() != NOT_DEFINED && sr_vector[rule].el.GetAtomicNumber() != (* it1).atmr->el.GetAtomicNumber()) continue;
			if (sr_vector[rule].bt.GetValue() != NOT_DEFINED && sr_vector[rule].bt.GetValue() != (* it1).bndr->bt.GetValue()) continue;
			
			n1 = true;	// passed the "BondedTo"-rule; now check the subrules and other rules...
			
			(* it1).bndr->flags[flag] = true;
			if (n1 && (sr_vector[rule].sub != NOT_DEFINED)) n1 = CheckRules(mdl, (* it1).atmr, flag, sr_vector[rule].sub);
			if (n1 && (sr_vector[rule].next != NOT_DEFINED)) n1 = CheckRules(mdl, atom, flag, sr_vector[rule].next);
			(* it1).bndr->flags[flag] = false;
			
			if (n1) return true;
		}
		return false;
		
		case tr_subrule::Valence:			// count the valences as electrons ; easier to use int!!!
		n1 = 0; it1 = atom->cr_list.begin();
		while (it1 != atom->cr_list.end())
		{
			switch ((* it1++).bndr->bt.GetValue())
			{
				case BONDTYPE_SINGLE:	n1 += 2; break;
				case BONDTYPE_CNJGTD:	n1 += 3; break;
				case BONDTYPE_DOUBLE:	n1 += 4; break;
				case BONDTYPE_TRIPLE:	n1 += 6; break;
			}
		}
		if (n1 != sr_vector[rule].data) return false;
		else if (sr_vector[rule].next != NOT_DEFINED) return CheckRules(mdl, atom, flag, sr_vector[rule].next);
		else return true;
		
		case tr_subrule::FormalCharge:
		if (atom->formal_charge != sr_vector[rule].data) return false;
		else if (sr_vector[rule].next != NOT_DEFINED) return CheckRules(mdl, atom, flag, sr_vector[rule].next);
		else return true;
		
		case tr_subrule::NeighborsAll:
		if (atom->cr_list.size() != (i32u) sr_vector[rule].data) return false;
		else if (sr_vector[rule].next != NOT_DEFINED) return CheckRules(mdl, atom, flag, sr_vector[rule].next);
		else return true;
		
		case tr_subrule::NeighborsHeavy:
		n1 = 0; it1 = atom->cr_list.begin();
		while (it1 != atom->cr_list.end())
		{
			if ((* it1++).atmr->el.GetAtomicNumber() != 1) n1++;
		}
		if (n1 != sr_vector[rule].data) return false;
		else if (sr_vector[rule].next != NOT_DEFINED) return CheckRules(mdl, atom, flag, sr_vector[rule].next);
		else return true;
		
		case tr_subrule::NeighborsHydrogen:
		n1 = 0; it1 = atom->cr_list.begin();
		while (it1 != atom->cr_list.end())
		{
			if ((* it1++).atmr->el.GetAtomicNumber() == 1) n1++;
		}
		if (n1 != sr_vector[rule].data) return false;
		else if (sr_vector[rule].next != NOT_DEFINED) return CheckRules(mdl, atom, flag, sr_vector[rule].next);
		else return true;
		
		case tr_subrule::NeighborsHighElNeg:
		n1 = 0; it1 = atom->cr_list.begin();
		while (it1 != atom->cr_list.end())
		{
			switch ((* it1++).atmr->el.GetAtomicNumber())
			{
				case 8:		n1++; break;	// O
				case 9:		n1++; break;	// F
				case 17:	n1++; break;	// Cl
				case 35:	n1++; break;	// Br
				case 53:	n1++; break;	// I
				case 85:	n1++; break;	// At
			}
		}
//		if (n1 != sr_vector[rule].data) return false;
		if (n1 < sr_vector[rule].data) return false;	// "inexact" test ; use "<" instead of "!="...
		else if (sr_vector[rule].next != NOT_DEFINED) return CheckRules(mdl, atom, flag, sr_vector[rule].next);
		else return true;
		
		case tr_subrule::NumBondsSingle:
		n1 = 0; it1 = atom->cr_list.begin();
		while (it1 != atom->cr_list.end())
		{
			if ((* it1++).bndr->bt.GetValue() == BONDTYPE_SINGLE) n1++;
		}
		if (n1 != sr_vector[rule].data) return false;
		else if (sr_vector[rule].next != NOT_DEFINED) return CheckRules(mdl, atom, flag, sr_vector[rule].next);
		else return true;
		
		case tr_subrule::NumBondsCnjgtd:
		n1 = 0; it1 = atom->cr_list.begin();
		while (it1 != atom->cr_list.end())
		{
			if ((* it1++).bndr->bt.GetValue() == BONDTYPE_CNJGTD) n1++;
		}
		if (n1 != sr_vector[rule].data) return false;
		else if (sr_vector[rule].next != NOT_DEFINED) return CheckRules(mdl, atom, flag, sr_vector[rule].next);
		else return true;
		
		case tr_subrule::NumBondsDouble:
		n1 = 0; it1 = atom->cr_list.begin();
		while (it1 != atom->cr_list.end())
		{
			if ((* it1++).bndr->bt.GetValue() == BONDTYPE_DOUBLE) n1++;
		}
		if (n1 != sr_vector[rule].data) return false;
		else if (sr_vector[rule].next != NOT_DEFINED) return CheckRules(mdl, atom, flag, sr_vector[rule].next);
		else return true;
		
		case tr_subrule::NumBondsTriple:
		n1 = 0; it1 = atom->cr_list.begin();
		while (it1 != atom->cr_list.end())
		{
			if ((* it1++).bndr->bt.GetValue() == BONDTYPE_TRIPLE) n1++;
		}
		if (n1 != sr_vector[rule].data) return false;
		else if (sr_vector[rule].next != NOT_DEFINED) return CheckRules(mdl, atom, flag, sr_vector[rule].next);
		else return true;
		
		case tr_subrule::RingSize:
		if (!mdl->FindRing(atom, atom, NULL, sr_vector[rule].data, flag + 1)) return false;
		else if (sr_vector[rule].next != NOT_DEFINED) return CheckRules(mdl, atom, flag, sr_vector[rule].next);
		else return true;
		
		case tr_subrule::Ring:
		n1 = strlen((const char *) ring_vector[sr_vector[rule].data]); n1 = (n1 + 1) / 2;
		if (!mdl->FindRing(atom, atom, ring_vector[sr_vector[rule].data], n1, flag + 1)) return false;
		else if (sr_vector[rule].next != NOT_DEFINED) return CheckRules(mdl, atom, flag, sr_vector[rule].next);
		else return true;
		
		default:
		cout << "fatal error : unknown typerule!" << endl; exit(EXIT_FAILURE);
		return false;
	}
}

void typerule::PrintSubRules(ostream & p1, i32s p2) const
{
	switch (sr_vector[p2].type)
	{
		case tr_subrule::BondedTo:
		if (sr_vector[p2].bt.GetValue() == NOT_DEFINED) p1 << "?";
		else p1 << sr_vector[p2].bt.GetSymbol2();
		if (sr_vector[p2].el.GetAtomicNumber() == NOT_DEFINED) p1 << "*";
		else p1 << sr_vector[p2].el.GetSymbol();
		if (sr_vector[p2].sub == NOT_DEFINED) break;
		p1 << "("; PrintSubRules(p1, sr_vector[p2].sub); p1 << ")";
		break;
		
		case tr_subrule::Valence:
		p1 << "vl=" << sr_vector[p2].data;
		break;
		
		case tr_subrule::FormalCharge:
		p1.setf(ios::showpos); p1 << "fc=" << sr_vector[p2].data; p1.unsetf(ios::showpos);
		break;
		
		case tr_subrule::NumBondsSingle:
		p1 << "bS=" << sr_vector[p2].data;
		break;
		
		case tr_subrule::NumBondsCnjgtd:
		p1 << "bC=" << sr_vector[p2].data;
		break;
		
		case tr_subrule::NumBondsDouble:
		p1 << "bD=" << sr_vector[p2].data;
		break;
		
		case tr_subrule::NumBondsTriple:
		p1 << "bT=" << sr_vector[p2].data;
		break;
		
		case tr_subrule::NeighborsAll:
		p1 << "nA=" << sr_vector[p2].data;
		break;
		
		case tr_subrule::NeighborsHeavy:
		p1 << "nB=" << sr_vector[p2].data;
		break;
		
		case tr_subrule::NeighborsHydrogen:
		p1 << "nH=" << sr_vector[p2].data;
		break;
		
		case tr_subrule::NeighborsHighElNeg:
		p1 << "nX=" << sr_vector[p2].data;
		break;
		
		case tr_subrule::RingSize:
		p1 << "rs=" << sr_vector[p2].data;
		break;
		
		case tr_subrule::Ring:
		p1 << "["; PrintRing(p1, ring_vector[sr_vector[p2].data]); p1 << "]";
		break;
	}
	
	if (sr_vector[p2].next == NOT_DEFINED) return;
	p1 << ","; PrintSubRules(p1, sr_vector[p2].next);
}

void typerule::PrintRing(ostream & p1, signed char * p2) const
{
	for (i32u n1 = 0;n1 < strlen((const char *) p2);n1++)
	{
		if (!(n1 % 2)) p1 << (char) p2[n1];
		else
		{
			i32s n2 = p2[n1];
			if (n2 == NOT_DEFINED) p1 << "*";
			else
			{
				element el = element(n2);
				p1 << el.GetSymbol();
			}
		}
	}
}

/*################################################################################################*/

// eof
