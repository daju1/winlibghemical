/**********************************************************************
Copyright (C) 1998-2001 by OpenEye Scientific Software, Inc.
Some portions Copyright (c) 2001-2003 by Geoffrey R. Hutchison

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
***********************************************************************/

#include "mol.h"

using namespace std;

namespace OpenBabel
{

bool WriteChemDraw(ostream &ofs,OBMol &mol)
{ 
  char buffer[BUFF_SIZE];

  ofs << mol.GetTitle() << endl;
  sprintf(buffer," %d %d",mol.NumAtoms(),mol.NumBonds());
  ofs << buffer << endl;
  
  OBAtom *atom;
  vector<OBNodeBase*>::iterator i;

  for(atom = mol.BeginAtom(i);atom;atom = mol.NextAtom(i))
  {
    sprintf(buffer," %9.4f %9.4f    0.0000 %-1s",
	    atom->x(),
	    atom->y(),
	    etab.GetSymbol(atom->GetAtomicNum()));
    ofs << buffer << endl;
  }

  OBBond *bond;
  vector<OBEdgeBase*>::iterator j;

  for(bond = mol.BeginBond(j);bond;bond = mol.NextBond(j))
  {
    sprintf(buffer,"%3d%3d%3d%3d",
	    bond->GetBeginAtomIdx(),
	    bond->GetEndAtomIdx(),
	    bond->GetBO(),1);
    ofs << buffer << endl;
  }
  return(true);
}

}
