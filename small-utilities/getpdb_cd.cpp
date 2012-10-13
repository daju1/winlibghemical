// GETPDB_CD.CPP

// Copyright (C) 2000 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

////////////////////////////////////////////////////////////////////////////////////////////////////

// this program makes scripts which can be used to read big piles of PDB-files from CD...
// redhat linux contains "rename" utility : add .ent extension using "rename .gz .ent.gz pdb????.gz"

#include <ctype.h>

#include <string>
#include <fstream>

#include <vector>
#include <algorithm>

#define PATH "/cdrom/distr/"

void main(void)
{
	ifstream ifile("cd_files.txt", ios::in);
	
	char buffer[65536];
	
	vector<string> files;
	while (ifile.peek() != '#')			// remember the "#end"-line!!!
	{
		if (ifile.peek() != '/')		// entries can be commented out...
		{
			int thrsh;
		//	ifile >> thrsh;		// this is for pdb_select format...
			ifile >> buffer;
			
			buffer[4] = 0;
			buffer[0] = tolower(buffer[0]);
			buffer[1] = tolower(buffer[1]);
			buffer[2] = tolower(buffer[2]);
			buffer[3] = tolower(buffer[3]);
			
			cout << buffer << endl;
			
			string str = buffer; files.push_back(str);
		}
		
		ifile.getline(buffer, sizeof(buffer));
	}
	
	ifile.close();
	
	ofstream ofile("script", ios::out);
	vector<string>::iterator it1 = unique(files.begin(), files.end());
	files.erase(it1, files.end());
	
	ofile << "#!/bin/sh" << endl;
	
	for (int n1 = 0;n1 < files.size();n1++)
	{
		const char * str = files[n1].c_str();
		ofile << "cp " << PATH << str[1] << str[2] << "/pdb" << str << ".gz ." << endl;
	}
	
	ofile.close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// eof
