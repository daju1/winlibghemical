// FILETRANS.CPP : Handle import/export through use of the Babel external lib.

// Copyright (C) 2000 Geoffrey Hutchison.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "filetrans.h"

#ifdef ENABLE_OPENBABEL

//#include <ghemical/libdefine.h>
#include "../libghemical/src/libdefine.h"

#include "project.h"

#include <fstream>
#include <strstream>
using namespace std;

// Babel lib includes
// ^^^^^^^^^^^^^^^^^^

#include "mol.h"
#include "obutil.h"
#include "data.h"
#include "typer.h"

using namespace std;
using namespace OpenBabel;

#ifndef FORMAT_PATH
#define FORMAT_PATH (char *) project::appdata_path
#endif	// FORMAT_PATH

namespace OpenBabel
{
	extern OBAromaticTyper aromtyper;
	extern OBAtomTyper atomtyper;
	extern OBExtensionTable extab;
	extern OBElementTable etab;
	extern OBTypeTable ettab;
}

/*################################################################################################*/

// Constructor
// Requires: None
// Provides: import, export vectors for future use
file_trans::file_trans()
{
  format_record current;

// it seems that under OELib these objects (aromtyper, atomtyper,
// extab, etab, ttab) are global, so here we could initialize them
// multiple times is case we create multiple file_trans objects.
// could this be risky and/or problematic??? 2001-05-28 TH

  // Certainly not risky, though it's maybe a bit problematic -GRH

  aromtyper.SetReadDirectory(FORMAT_PATH);
  aromtyper.SetEnvironmentVariable("GHEMICAL_DIR");
  atomtyper.SetReadDirectory(FORMAT_PATH);		// why 2nd time?
  atomtyper.SetEnvironmentVariable("GHEMICAL_DIR");	// why 2nd time?
  extab.SetReadDirectory(FORMAT_PATH);
  extab.SetEnvironmentVariable("GHEMICAL_DIR");
  etab.SetReadDirectory(FORMAT_PATH);
  etab.SetEnvironmentVariable("GHEMICAL_DIR");
  ttab.SetReadDirectory(FORMAT_PATH);
  ttab.SetEnvironmentVariable("GHEMICAL_DIR");

  for (unsigned int i = 0; i < extab.Count(); i++)
    {
      current.offset = i;
      current.format = extab.GetDescription(i);
      current.format += " file -- .";
      current.format += extab.GetExtension(i);
      if (extab.IsReadable(i))
	imports.push_back(current);
      if (extab.IsWritable(i))
	exports.push_back(current);
    }
  
  // the rest is for compatibility part...
  // the rest is for compatibility part...
  // the rest is for compatibility part...
  
  obm = NULL;
  prj = NULL;
  
  name_tag_count = -1;
  tagtab = NULL;
}

file_trans::~file_trans()
{
  // we will delete the OBMol and name tags!!!
  
  if (obm != NULL) delete obm;
  if (tagtab != NULL) delete [] tagtab;
}

// Import
// Requires: filename (for typing) and an istream to read
// Provides: 
int file_trans::Import(const char *filename, istream &input, ostream &output)
{
  io_type inFileType;

  char *tmp = new char[strlen(filename) + 1];
  strcpy(tmp, filename);
  inFileType = extab.FilenameToType(tmp);

  if (!extab.CanReadExtension(tmp))
    {
      delete [] tmp;
      return(-1);
    }

  OBMol mol(inFileType, GHEMICAL);
  OBFileFormat ff;
  ff.ReadMolecule(input,mol);
  ff.WriteMolecule(output,mol);
  delete [] tmp;

  printf("file_trans::Import end\n");

  return(0);
}

int file_trans::Export(const char *filename, istream &input, ostream &output)
{
  io_type outFileType;

  char *tmp = new char[strlen(filename) + 1];
  strcpy(tmp, filename);
  outFileType = extab.FilenameToType(tmp);

  if (!extab.CanWriteExtension(tmp))
    {
      delete [] tmp;
      return(-1);
    }

  OBMol mol(GHEMICAL, outFileType);
  OBFileFormat ff;
  ff.ReadMolecule(input,mol,tmp);
  ff.WriteMolecule(output,mol);
  delete [] tmp;

  return(0);
}

bool file_trans::CanImport(const char *filename)
{
  char *tmp = new char[strlen(filename) + 1];
  bool canRead;
  strcpy(tmp, filename);
  canRead = extab.CanReadExtension(tmp);
  delete [] tmp;
  return canRead;
}

bool file_trans::CanExport(const char *filename)
{
  char *tmp = new char[strlen(filename) + 1];
  bool canWrite;
  strcpy(tmp, filename);
  canWrite = extab.CanWriteExtension(tmp);
  delete [] tmp;
  return canWrite;
}

string file_trans::GetExportDescription(unsigned int index)
{
  string temp;
  if (index < exports.size())
    temp = exports[index].format;

  return temp;
}

string file_trans::GetImportDescription(unsigned int index)
{
  string temp;
  if (index < imports.size())
    temp = imports[index].format;

  return temp;
}

// Import
// Requires: filename, a type and an istream to read
// Provides: an imported file using Babel
int file_trans::Import(const char *filename, unsigned int type, 
		       istream &input, ostream &output)
{
  io_type inFileType;

  if (type < imports.size())
    inFileType = extab.GetType(imports[type].offset);
  else
    {
      char *tmp = new char[strlen(filename) + 1];
      strcpy(tmp, filename);
      inFileType = extab.FilenameToType(tmp);
      if (!extab.CanReadExtension(tmp))
	{
	  delete [] tmp;
	  return(-1);
	}
      delete [] tmp;
    }

  OBMol mol(inFileType, GHEMICAL);
  OBFileFormat ff;
  ff.ReadMolecule(input,mol);
  ff.WriteMolecule(output,mol);

  printf("file_trans::Import end\n");

  return(0);
}

// Export
// Requires: filename, a type and an istream to read
// Provides: an exported file using Babel
int file_trans::Export(const char *filename, unsigned int type,
		       istream &input, ostream &output)
{
  io_type outFileType;
  char *tmp = new char[strlen(filename) + 1];
  strcpy(tmp, filename);

  if (type < exports.size())
    outFileType = extab.GetType(exports[type].offset);
  else
    {
      outFileType = extab.FilenameToType(tmp);
      if (!extab.CanWriteExtension(tmp))
	{
	  delete [] tmp;
	  return(-1);
	}
    }

  OBMol mol(GHEMICAL, outFileType);
  OBFileFormat ff;
  ff.ReadMolecule(input,mol,tmp);
  ff.WriteMolecule(output,mol);

  delete [] tmp;
  return(0);
}

/*################*/
/*################*/

OBMol * file_trans::Copy(project * p1, iter_al p2, iter_al p3)
{
	prj = p1; obm = new OBMol(); itb = p2; ite = p3;
	
	// count the atoms and reserve memory for the name tags.
	// count the atoms and reserve memory for the name tags.
	// count the atoms and reserve memory for the name tags.
	
	name_tag_count = 0;
	for (iter_al it1 = itb;it1 != ite;it1++)
	{
		name_tag_count++;
	}
	
	tagtab = new atom_name_tag[name_tag_count];
	
	// copy the atoms/bonds to OBMol...
	
	obm->BeginModify();
	
	// copy the atoms.
	
	int tag_counter = 0;
	for (iter_al it1 = itb;it1 != ite;it1++)
	{
		OBAtom * oba = obm->NewAtom();
		oba->SetAtomicNum((* it1).el.GetAtomicNumber());
		
		const fGL * cdata = (* it1).GetCRD(0);
		float xcrd = cdata[0] * 10.0;
		float ycrd = cdata[1] * 10.0;
		float zcrd = cdata[2] * 10.0;
		
		oba->SetVector(xcrd, ycrd, zcrd);
		
		// now set the "name tag" for this atom...
		// now set the "name tag" for this atom...
		// now set the "name tag" for this atom...
		
		tagtab[tag_counter].atmr = & (* it1);
		tagtab[tag_counter].oba = oba;
		tag_counter++;
	}
	
	// copy the bonds.
	
// WE ASSUME HERE THAT ATOM ITERATORS COVER THE WHOLE SYSTEM!!!
prj->UpdateIndex();

	for (iter_bl it1 = prj->GetBondsBegin();it1 != prj->GetBondsEnd();it1++)
	{
		i32s ind1 = (* it1).atmr[0]->index + 1;
		i32s ind2 = (* it1).atmr[1]->index + 1;
		
		i32s bt;
		switch ((* it1).bt.GetValue())
		{
			case BONDTYPE_SINGLE:	bt = 1; break;
			case BONDTYPE_DOUBLE:	bt = 2; break;
			case BONDTYPE_TRIPLE:	bt = 3; break;
		        case BONDTYPE_CNJGTD:   bt = 5; break;
			default:		bt = 1;
		}
		
		obm->AddBond(ind1, ind2, bt);
	}
	
	// ok, it's ready!
	
	obm->EndModify();
	return obm;
}

OBMol * file_trans::CopyMolecule(project *, int)
{
	return NULL;
}

OBMol * file_trans::CopyAll(project * prj)
{
	return Copy(prj, prj->GetAtomsBegin(), prj->GetAtomsEnd());
}

void file_trans::Synchronize(void)
{
	// create a new tagtab for the current situation (for bonds)...
	
	atom_name_tag * tagtab2 = new atom_name_tag[obm->NumAtoms()];
	
	for (i32u n1 = 1;n1 <= obm->NumAtoms();n1++)
	{
		OBAtom * oba = obm->GetAtom(n1);
		atom * atmr = NULL;
		
		// try to find the matching name tag; if you find it, get it and remove the original.
		
		for (i32s n2 = 0;n2 < name_tag_count;n2++)
		{
			if (tagtab[n2].oba != oba) continue;
			
			// match found!!!
			// match found!!!
			// match found!!!
			
			atmr = tagtab[n2].atmr;
			tagtab[n2].atmr = NULL;
			tagtab[n2].oba = NULL;
			break;
		}
		
		// if atmr is still NULL, we did not have a matching tag -> this must be a new atom!
		// otherwise, we have this pair of corresponding objects and we can synchronize.
		
		if (!atmr)
		{
			element el(oba->GetAtomicNum());
			
			fGL crd[3] =
			{
				oba->GetX() / 10.0,
				oba->GetY() / 10.0,
				oba->GetZ() / 10.0
			};
			
			atom newatom(el, crd, prj->GetCRDSetCount());
			prj->AddAtom(newatom); atmr = & prj->GetLastAtom();
			
	// this seems to crash -> disabled!!!!!!!!!!
	//		atmr->charge = oba->GetPartialCharge();
		}
		else
		{
			atmr->el = element(oba->GetAtomicNum());
			atmr->SetCRD(0, oba->GetX() / 10.0, oba->GetY() / 10.0, oba->GetZ() / 10.0);
			
	// this seems to crash -> disabled!!!!!!!!!!
	//		atmr->charge = oba->GetPartialCharge();
		}
		
		// save the new tagtab entry...
		
		tagtab2[n1 - 1].atmr = atmr;
		tagtab2[n1 - 1].oba = oba;
	}
	
	// in the above loop, we discarded the name tags that had corresponding atoms.
	// so if there still are name tags left, it means we should remove those atoms!
	// removing atoms will also remove bonds connected to them automagically...
	
	for (i32s n1 = 0;n1 < name_tag_count;n1++)
	{
		if (!tagtab[n1].atmr) continue;
		
		// ok, remove this atom.
		// ok, remove this atom.
		// ok, remove this atom.
		
		iter_al it1 = itb;
		while (it1 != ite) if (& (* it1) == tagtab[n1].atmr) break;
		if (it1 == itb) { cout << "BUG: iterator lost!!!" << endl; exit(EXIT_FAILURE); }
		
		tagtab[n1].atmr = NULL;
		tagtab[n1].oba = NULL;
		
		prj->RemoveAtom(it1);
	}
	
	// now we should have the atoms in sync; do the same for bonds.
	// for each OBBond, find or create the equivalent, and check the type.
	// we assume that for a pair of atoms, there is no more than 1 bond!!!
	
	for (i32u n1 = 0;n1 < obm->NumBonds();n1++)
	{
		OBBond * obb = obm->GetBond(n1);
		i32u ind1 = obb->GetBeginAtomIdx() - 1;
		i32u ind2 = obb->GetEndAtomIdx() - 1;
		
		atom * atmr1 = tagtab2[ind1].atmr;
		atom * atmr2 = tagtab2[ind2].atmr;
		
		bond * bndr = NULL;
		
		for (iter_bl it1 = prj->GetBondsBegin();it1 != prj->GetBondsEnd();it1++)
		{
			bool match = false;
			if ((* it1).atmr[0] == atmr1 && (* it1).atmr[1] == atmr2) match = true;
			if ((* it1).atmr[1] == atmr1 && (* it1).atmr[0] == atmr2) match = true;
			if (!match) continue;
			
			// match found; mark it and check the bondtype.
			// match found; mark it and check the bondtype.
			// match found; mark it and check the bondtype.
			
			bndr = & (* it1);
			
			i32s bt;
			switch (obb->GetBO())
			{
				case 1:		bt = BONDTYPE_SINGLE; break;
				case 2:		bt = BONDTYPE_DOUBLE; break;
				case 3:		bt = BONDTYPE_TRIPLE; break;
				default:	bt = BONDTYPE_CNJGTD;
			}

			if (obb->IsAromatic())	bt = BONDTYPE_CNJGTD;
			
			bndr->bt = bondtype(bt);
			
			break;
		}
		
		// if there were no match, then create the bond!
		
		if (!bndr)
		{
			i32s bt;
			switch (obb->GetBO())
			{
				case 1:		bt = BONDTYPE_SINGLE; break;
				case 2:		bt = BONDTYPE_DOUBLE; break;
				case 3:		bt = BONDTYPE_TRIPLE; break;
				default:	bt = BONDTYPE_CNJGTD;
			}
			
			if (obb->IsAromatic())	bt = BONDTYPE_CNJGTD;

			bond newbond(atmr1, atmr2, bondtype(bt));
			prj->AddBond(newbond);
		}
	}
	
	// get rid of the new tagtab...
	
	delete[] tagtab2;
}

#endif	// ENABLE_OPENBABEL

/*################################################################################################*/

// eof
