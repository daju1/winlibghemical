// FILETRANS.H : Handle import/export through use of the Babel external lib.

// Copyright (C) 2000 Geoffrey Hutchison.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef FILETRANS_H
#define FILETRANS_H

#include "../libghemical/src/libconfig.h"		// ENABLE_OPENBABEL
#ifdef ENABLE_OPENBABEL

#include "appconfig.h"

/*################################################################################################*/

#include "project.h"

// OpenBabel includes...
// OpenBabel includes...
// OpenBabel includes...

#include "mol.h"
//#include "oeutil.h"
//#include "data.h"
//#include "typer.h"
using namespace OpenBabel;

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

/*################################################################################################*/

typedef struct {
  atom * atmr;
  OBAtom * oba;
} atom_name_tag;

typedef struct {
  string   format;
  unsigned int offset;
} format_record;

class file_trans
{
        private:

  vector<format_record> imports;
  vector<format_record> exports;
  
  int name_tag_count;
  atom_name_tag * tagtab;
  project * prj; OBMol * obm;
  iter_al itb; iter_al ite;
  
  	public:
  
  file_trans();
  virtual ~file_trans();
  
  // these expect to be passed the filename to pick the file format
  int Import(const char *, istream &, ostream &);
  int Export(const char *, istream &, ostream &);

  // these check whether the format picked by the filename are valid
  bool CanImport(const char *);
  bool CanExport(const char *);

  // Get a listing of the number of valid import/export types
  unsigned int NumImports() { return imports.size(); }
  unsigned int NumExports() { return exports.size(); }

  // Get the description associated with this type
  string GetExportDescription(unsigned int);
  string GetImportDescription(unsigned int);
  
  // These take the unsigned int as a format specifier
  int Import(const char *, unsigned int, istream &, ostream &);
  int Export(const char *, unsigned int, istream &, ostream &);
  
  	// some Babel compatibility methods:
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
  /// This takes a pair of atom list iterators, and copies all atoms/bonds in this range to OBMol.
  OBMol * Copy(project *, iter_al, iter_al);
  
  /// This will copy a molecule (defined by the index) to OBMol.
  OBMol * CopyMolecule(project *, int);
  
  /// This will copy everything in this project to OBMol.
  OBMol * CopyAll(project *);
  
  /// This will syncronize the project and the OBMol.
  void Synchronize(void);
  
};

/*################################################################################################*/

#endif	// ENABLE_OPENBABEL
#endif	// FILETRANS_H

// eof
