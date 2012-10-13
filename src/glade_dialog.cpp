// GLADE_DIALOG.CPP

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "glade_dialog.h"

#include "project.h"

#include <iostream>
#include <strstream>
using namespace std;

/*################################################################################################*/

glade_dialog::glade_dialog(const char * xmlfile)
{
	char filename[256];
	ostrstream str(filename, sizeof(filename));
	str << project::appdata_path << DIR_SEPARATOR << project::appversion << DIR_SEPARATOR << xmlfile << ends;
	
//	xml = glade_xml_new(filename, NULL, NULL);
	
/*	if (xml == NULL)
	{
		cout << "ERROR : could not read glade XML file : " << filename << endl;
		exit(EXIT_FAILURE);
	}*/
}

glade_dialog::~glade_dialog(void)
{
	// need to do anything here to clean the xml object???
	// need to do anything here to clean the xml object???
	// need to do anything here to clean the xml object???
}

/*################################################################################################*/

// eof
