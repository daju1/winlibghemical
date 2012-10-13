// GETPDB_FTP.CPP

// Copyright (C) 2000 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

////////////////////////////////////////////////////////////////////////////////////////////////////

// this program makes scripts which can be used to fetch big piles of PDB-files...
// to be used in reasonable limits only!!! do not try to download the entire PDB,
// order the CD set instead...

// the script is used for example "ftp -v -n pdb.ccdc.cam.ac.uk < batch.txt"

#include <ctype.h>

#include <string>
#include <fstream>

#include <vector>
#include <algorithm>

#error - please write your email address here for anonymous password!!!
#define EMAIL "unknown"

void main(void)
{
	ifstream ifile("missing.txt", ios::in);
	
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
	
	ofstream ofile("batch.txt", ios::out);
	vector<string>::iterator it1 = unique(files.begin(), files.end());
	files.erase(it1, files.end());
	
	ofile << "user anonymous " << EMAIL << endl;
	
	ofile << "bin" << endl;
	ofile << "cd bnl" << endl;
	ofile << "cd current_release" << endl;
	ofile << "cd compressed_files" << endl;
	
	for (int n1 = 0;n1 < files.size();n1++)
	{
		const char * str = files[n1].c_str();
		ofile << "cd " << str[1] << str[2] << endl;
		ofile << "get pdb" << str << ".ent.Z" << endl;
		ofile << "cd .." << endl;
	}
	
	ofile << "bye" << endl;
	ofile.close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// eof
