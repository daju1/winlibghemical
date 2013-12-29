#ifndef READ_FILE_H_
#define READ_FILE_H_
#include "stdafx.h"
#include <windows.h>
#include <vector>
#include <string>
using namespace std;

#define USE_STRING_IN_THE_CELL	1
#define CELL_S_N_CHARS			8

struct DelimiterData
{
	bool bTab;
	bool bSpace;
	bool bComma;
	bool bCommaPoint;
	bool bOther;
	char charOther;

	bool bUseManyDelimitersAsOne;
};
int ReadDatFile(HWND hWnd, char * fpath, char* filename, vector<vector<double> > *pvectors, vector<string> * pnames);
int GetDelimiterFromID(int delimID);

int ReadDatFile(const char * filename, vector<double> & x, vector<double> & y, vector<double> & z);
int ReadBlnFile(const char * filename, vector<double>& x, vector<double>& y, int& inside);
int ReadBlnFile(const char * filename, vector<double>& x, vector<double>& y, vector<double>& z, int& inside);
int ReadBlnFile(const char * filename, vector<vector<vector<double> > > * vvvd, vector<int> * vinside);

int ReadCoordinateFromFile(const char * filename, double& f);


#endif