// EXAMPLE3.CPP

// Copyright (C) 2004 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "example3.h"

#include "..\notice.h"

#include "..\eng1_mm_tripos52.h"
#include "..\eng1_mm_default.h"

#include "..\geomopt.h"
#include "..\moldyn.h"

#include <strstream>
using namespace std;

/*################################################################################################*/

silent_model::silent_model(void) : model()
{
}

silent_model::~silent_model(void)
{
}

void silent_model::Message(const char *)
{
}

void silent_model::WarningMessage(const char *)
{
}

void silent_model::ErrorMessage(const char *)
{
}

void silent_model::PrintToLog(const char *)
{
}

/*################################################################################################*/

int method = -1;
int request = -1;

int main(int argc, char ** argv)
{
	if (argc > 1)
	{
		if (!strcmp(argv[1], "--copyright"))
		{
			print_lib_intro_notice(cout);
			print_copyright_notice(cout);
			return 0;
		}
		
		if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "--version"))
		{
			cout << "example3 ; a CGI test program for libghemical-" << LIBVERSION << endl;
			cout << endl;
			cout << "command-line options:" << endl;
			cout << "\t" << "--copyright" << "\t\t" << "display a copyright notice." << endl;
			return 0;
		}
	}
	
	cout << "Content-type: text/plain" << endl;	// this tells to the web server that output is plain-text formatted...
	cout << endl;
	
	const char * clstring = getenv("CONTENT_LENGTH");
	if (clstring == NULL)
	{
		cout << "%Error unknown_content_length" << endl;
		exit(EXIT_FAILURE);
	}
	
	char ** endptr = NULL;
	int clength = strtol(clstring, endptr, 10);
	
	char * buffer = new char[clength + 1];
	
	// first separate the different fields...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	char * buff_job = NULL;
	char * buff_params = NULL;
	char * buff_data = NULL;
	
	while (true)
	{
		cin.getline(buffer, clength + 1, '=');
		clength -= strlen(buffer) + 1;
		
		char ** ref_to_buff = NULL;
		if (!strcmp(buffer, "job")) ref_to_buff = & buff_job;
		if (!strcmp(buffer, "params")) ref_to_buff = & buff_params;
		if (!strcmp(buffer, "data")) ref_to_buff = & buff_data;
		
		if (ref_to_buff == NULL)
		{
			cout << "%Error unknown_field " << buffer << endl;
			exit(EXIT_FAILURE);
		}
		
		cin.getline(buffer, clength + 1, '&');
		clength -= strlen(buffer) + 1;
		
		(* ref_to_buff) = new char[strlen(buffer) + 1];
		strcpy((* ref_to_buff), buffer);
		
		if (buff_job != NULL && buff_params != NULL && buff_data != NULL) break;
		if (cin.eof())
		{
			cout << "%Error end_of_stream_reached" << endl;
			exit(EXIT_FAILURE);
		}
	}
	
	// then do the decoding...
	// ^^^^^^^^^^^^^^^^^^^^^^^
	
	for (int n1 = 0;n1 < 3;n1++)
	{
		char ** ref_to_buff = NULL;
		
		switch (n1)
		{
			case 0:		ref_to_buff = & buff_job; break;
			case 1:		ref_to_buff = & buff_params; break;
			case 2:		ref_to_buff = & buff_data; break;
		}
		
		strcpy(buffer, (* ref_to_buff));
		
		int i1 = 0; int i2 = 0;
		while (buffer[i1] != 0)
		{
			bool remains_same = false;
			if (buffer[i1] >= 'a' && buffer[i1] <= 'z') remains_same = true;
			if (buffer[i1] >= 'A' && buffer[i1] <= 'Z') remains_same = true;
			if (buffer[i1] >= '0' && buffer[i1] <= '9') remains_same = true;
			if (buffer[i1] == '.' || buffer[i1] == '-' || buffer[i1] == '*' || buffer[i1] =='_') remains_same = true;
			
			if (remains_same)
			{
				(* ref_to_buff)[i2++] = buffer[i1++];
			}
			else if (buffer[i1] == '+')
			{
				(* ref_to_buff)[i2++] = ' ';
				i1++;
			}
			else
			{
				if (buffer[i1 + 0] != '%')
				{
					cout << "%Error decoding_error" << endl;
					exit(EXIT_FAILURE);
				}
				
				char hex[3];
				hex[0] = buffer[i1 + 1];
				hex[1] = buffer[i1 + 2];
				hex[2] = 0;
				
				char ** endptr = NULL;
				int code = strtol(hex, endptr, 16);
				
				(* ref_to_buff)[i2++] = (char) code;
				i1 += 3;
			}
		}
		
		(* ref_to_buff)[i2++] = 0;
	}
	
	//cout << "%Debug job=" << buff_job << endl;
	//cout << "%Debug params=" << buff_params << endl;
	//cout << "%Debug data=" << buff_data << endl;
	
	// create a model object, and read in the data from a string buffer:
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	silent_model * mdl = new silent_model();
	
	istrstream iss(buff_data);
	ReadCGI(* mdl, iss, false);
	
	// check that we get everything that is needed ; parse "job" and "params"...
	// check that we get everything that is needed ; parse "job" and "params"...
	// check that we get everything that is needed ; parse "job" and "params"...
	
/*	if (method < 0)
	{
		cout << "%Error method_record_missing" << endl;
		exit(EXIT_FAILURE);
	}
	
	if (request < 0)
	{
		cout << "%Error request_record_missing" << endl;
		exit(EXIT_FAILURE);
	}	*/
	
	// if everything went fine, then create a computation engine object and initialize it.
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	engine * eng = mdl->GetCurrentSetup()->CreateEngineByIDNumber(ENG1_MM_TRIPOS52);
//	engine * eng = mdl->GetCurrentSetup()->CreateEngineByIDNumber(ENG1_MM_DEFAULT);
	
	CopyCRD(mdl, eng, 0);
	
	// then just handle the different job types...
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	if (!strcmp(buff_job, "CompEne"))
	{
		eng->Compute(0);
		double ene = eng->GetEnergy();
		
		cout << "%CompEne " << ene << endl;
	}
	else if (!strcmp(buff_job, "DoGeomOpt"))
	{
		geomopt * opt = new geomopt(eng, 100, 0.025, 10.0);
		
		int n1;
		for (n1 = 0;n1 < 250;n1++)
		{
			opt->TakeCGStep(conjugate_gradient::Newton2An);
			
			if ((n1 + 1) % 10 == 0)
			{
				cout << "%DoGeomOpt_i ";
				cout << (n1 + 1) << " ";	// step
				cout << opt->optval << " ";	// energy in kJ/mol
				cout << opt->optstp << " ";	// steplength
				cout << endl;
				
				CopyCRD(eng, mdl, 0);
				WriteCGI_Coord(* mdl, cout, 0);
			}
		}
		
		// print out the final results...
		
		cout << "%DoGeomOpt_f ";
		cout << (n1 + 1) << " ";	// step
		cout << opt->optval << " ";	// energy in kJ/mol
		cout << opt->optstp << " ";	// steplength
		cout << endl;
		
		CopyCRD(eng, mdl, 0);
		WriteCGI_Coord(* mdl, cout, 0);
		
		delete opt; opt = NULL;
	}
	else
	{
		cout << "%Error unknown_job " << buff_job << endl;
	}
	
	// finally release all allocated memory and leave:
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	delete eng; eng = NULL;
	delete mdl; mdl = NULL;
	
	return 0;
}

/*################################################################################################*/

// eof
