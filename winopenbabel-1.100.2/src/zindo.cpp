/**********************************************************************
Copyright (C) 2002-2003 by Geoffrey Hutchison

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
***********************************************************************/

#include <stdlib.h>
#include "mol.h"

using namespace std;

namespace OpenBabel {
  
bool WriteZINDO(ostream &ofs,OBMol &mol)
{
  char buffer[BUFF_SIZE];
  int orbitals, valenceE = 0;
  vector<OBNodeBase*>::iterator i;
  OBAtom *atom;
  bool charged = false;

  for (atom = mol.BeginAtom(i); atom; atom = mol.NextAtom(i))
    {
      switch (atom->GetAtomicNum()) {
      case 1:
	valenceE += 1;
	break;
      case 5: case 13:
	valenceE += 3;
	break;
      case 6: case 14:
	valenceE += 4;
	break;
      case 7: case 15: case 33:
	valenceE += 5;
	break;
      case 8: case 16: case 34:
	valenceE += 6;
	break;
      case 9: case 17: case 35:
	valenceE += 7;
	break;
      default:
	cerr << " tried to get valence electrons for " 
	     << atom->GetAtomicNum() << endl;
      }
    }

  orbitals = valenceE / 2;
  if (charged)
    {
      orbitals = (valenceE / 2) - 1;
      valenceE -= 1;
    }

  ofs << " $TITLEI" << endl;
  ofs << endl;
  ofs << "   " << mol.GetTitle() << endl;
  ofs << endl;
  ofs << " $END" << endl;
  ofs << endl;
  ofs << " $CONTRL" << endl;
  ofs << endl;
  if (charged)
    ofs << " SCFTYP       ROHF   RUNTYP       CI   ENTTYP     COORD" << endl;
  else
    ofs << " SCFTYP        RHF   RUNTYP       CI   ENTTYP     COORD" << endl;
  ofs << " UNITS        ANGS   INTTYP        1   IAPX           3" << endl;
  if (charged)
    {
      ofs << endl;
      ofs << " NOP = 1 " << endl;
      ofs << " NDT = 1 " << endl;
      sprintf(buffer," FOP(1) =% 10.6f% 10.6f",
	      valenceE - 1, 1.0);
      ofs << buffer << endl;
    }

  sprintf(buffer," NAT          %4d   NEL        %4d   MULT           1",
	  mol.NumAtoms(),
	  valenceE);
  ofs << buffer << endl;
  ofs << " IPRINT         -1   ITMAX       100" << endl;
  ofs << endl;
  ofs << "! ***** BASIS SET AND C. I. SIZE INFORMATION ***** " << endl;
  ofs << endl;

  sprintf(buffer," DYNAL(1) =     0%5d%5d    0    0 1200%5d",
	  mol.NumAtoms() - mol.NumHvyAtoms(),
	  mol.NumHvyAtoms(),
	  orbitals + 25);
  ofs << buffer << endl;

  ofs << endl;
  ofs << " INTFA(1) =   1.000000 1.267000  0.680000  1.000000  1.000000 " << endl;
  ofs << endl;
  ofs << "! ***** OUTPUT FILE NAME ***** " << endl;
  ofs << endl;
  ofs << "   ONAME =  zindo " << endl;
  ofs << endl;
  ofs << " $END" << endl;
  ofs << endl;
  ofs << " $DATAIN " << endl;
  ofs << endl;

  for (atom = mol.BeginAtom(i); atom; atom = mol.NextAtom(i))
    {
      sprintf(buffer, "% 10.6f% 10.6f% 10.6f%5d",
	      atom->GetX(),
	      atom->GetY(),
	      atom->GetZ(),
	      atom->GetAtomicNum());
      ofs << buffer << endl;
    }


  ofs << endl;
  ofs << " $END " << endl;
  ofs << endl;
  ofs << " $CIINPU" << endl;
  ofs << endl;
  ofs << "! ***** C. I. SPECIFICATION *****" << endl;
  ofs << endl;
  ofs << "    2    1   25    1    0    0    0    1   10    1   10" << endl;
  ofs << "  -60000.0 0.0000000" << endl;
  ofs << endl;

  if (charged)
    sprintf(buffer, "%5d%5d%5d%5d", 1, orbitals, orbitals, orbitals + 1);
  else
    sprintf(buffer, "%5d%5d%5d", 1, orbitals, orbitals);
  ofs << buffer << endl;
  if (charged)
    sprintf(buffer,"%5d%5d%5d%5d%5d",
	    21, (orbitals - 8), orbitals + 1, orbitals + 1, orbitals + 11);
  else
    sprintf(buffer,"%5d%5d%5d%5d%5d",
	    21, (orbitals - 9), orbitals, orbitals + 1, orbitals + 10);
  ofs << buffer << endl;
  ofs << endl;
  ofs << " $END" << endl;  

  return(true);
}

}

