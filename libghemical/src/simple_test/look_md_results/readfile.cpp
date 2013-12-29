#include "StdAfx.h"
#include "readfile.h"
//#include "../datenum.h"
extern HINSTANCE hInst;   // текущий экземпляр
void ErrorExit(LPTSTR lpszFunction, bool toExitProcess = false);
/*void ErrorExit(LPTSTR lpszFunction, bool toExitProcess = false) 
{ 
    TCHAR szBuf[200]; 
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    wsprintf(szBuf, 
        "%s failed with error %d: %s", 
        lpszFunction, dw, lpMsgBuf); 
 
    MessageBox(NULL, szBuf, "Error", MB_OK); 

    LocalFree(lpMsgBuf);
	if(toExitProcess)
		ExitProcess(dw); 
}*/
#pragma warning (disable:4786)

#define USE_HWND 0

int GetDelimiterFromID(int delimID)
{		
	int delim;
	switch(delimID)
	{
	case 0:
		delim = '\t';
		break;
	case 1:
		delim = ' ';
		break;
	case 2:
		delim = ',';
		break;
	case 3:
		delim = ';';
		break;
	}
	return delim;
}

int ReadDatFile(HWND hWnd, char * fpath, char* filename, vector<vector<double> > *pvectors, vector<string> * pnames )
{
	int delim = ',';
	bool bUse_Header = true;
	double value;
	const int n=16384; // length of buffer string line
	char* szBuff; // buffer string line
	if ((szBuff = (char *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,n*sizeof(char))) == NULL)
	{
		MessageBox(0, "vX - Not enough memory",
			"importExcelCommonFileParam()", 0);
		return -1;
	}
	char *p1, *p2, *p;
	
	char ext[255];
	p = strrchr(fpath,'.');
	if (p)
		strcpy(ext, p+1);
	char * ch;

	if (p)
	{
		char * pp;
		// sscan year from filename
		p1 = strrchr(fpath,'\\');
		p2 = strrchr(fpath,'/');

		if (p1 && p2)
			pp = p1>p2 ? p1 : p2;
		if (!p1 && p2)
			pp = p2;
		if (p1 && !p2)
			pp = p1;
		if (!p1 && !p2)
			pp = fpath;
		else
		{
			strncpy(filename, pp+1, p-pp-1);
			filename[p-pp-1] = '\0';
		}



		FILE *stream;
		if ((stream = fopen(fpath,"rt")) == NULL)
		{
			char errstr[2048];
			sprintf(errstr, "ReadDatFile:fopen(%s)", fpath);
			ErrorExit(errstr);
			return -1;
		}


#if USE_HWND
		RECT rcClient;  // Client area of parent window 
		int cyVScroll;  // Height of scroll bar arrow 
		HWND hwndPB;    // Handle of progress bar 
		size_t cb,		// Size of file and count of
		  cb_read = 0,  // bytes read 
		  cb_disp = 0;  // bytes displayed


		if (hWnd != NULL)
		{

			// Ensure that the common control DLL is loaded
			// and create a progress bar along the bottom of
			// the client area of the parent window. 
			// Base the height of the progress bar on the
			// height of a scroll bar arrow. 
			InitCommonControls(); 
			GetClientRect(hWnd, &rcClient); 
			cyVScroll = GetSystemMetrics(SM_CYVSCROLL); 
			hwndPB = CreateWindowEx(0, PROGRESS_CLASS,
					 (LPSTR) NULL, WS_CHILD | WS_VISIBLE,
					 0, 0,
					 //rcClient.left, rcClient.bottom,
					 rcClient.right, cyVScroll, 
					 hWnd, (HMENU) 0, hInst, NULL); 

			// Gets the file descriptor associated with a stream.
			int fd = _fileno(stream);
			// Get the length of a file in bytes
			cb = _filelength(fd);



			// Set the range and increment of the progress
			// bar. 

			SendMessage(hwndPB, PBM_SETRANGE, 0,
						MAKELPARAM(0, cb / 2048)); 
			SendMessage(hwndPB, PBM_SETSTEP, (WPARAM) 1, 0); 
		}
#endif

		bool have_header = false;
		bool have_tsyfra = false;


		// read and ignore first nLinesToIgnore lines
		int line = 0;
		char title[512];
		int nParams = 0;
		size_t bytes_line;
		size_t bytes_param;
		// read and ignore first nLinesToIgnore lines
		line = 0;
		// read the first line and define memory and titles for plots
		int ncol = 0;
		if (!feof(stream))
		{
			ch = fgets(szBuff,n,stream);
			if( ch != NULL && strlen(szBuff) > 1)
			{
				bytes_line = strlen(ch);
				//MessageBox(0, szBuff, "", 0);
#if USE_HWND
				// calculate progress
				if(hWnd != NULL)
				{
					cb_read += bytes_line;
					if (cb_read - cb_disp > 2048) 
					{
					   // Advance the current position of the
					   // progress bar by the increment. 
						size_t todisp = (cb_read - cb_disp) / 2048;
						cb_disp += todisp*2048;
						for (size_t idisp = 0; idisp < todisp; idisp++)
							SendMessage(hwndPB, PBM_STEPIT, 0, 0); 
					}
				}
#endif
				p1 = szBuff;

				// подбор разделителя
				if (strchr(p1,',') != NULL)
				{
					delim = ',';
				}
				else
				if (strchr(p1,'\t') != NULL)
				{
					delim = '\t';
				}
				else
				if (strchr(p1,';') != NULL)
				{
					delim = ';';
				}
				else
				if (strchr(p1,' ') != NULL)
				{
					delim = ' ';
				}


				size_t first_line_len = strlen(szBuff);
				bool isnot_comment_char = true;
				for (size_t j = 0; j < first_line_len; j++)
				{
					int cur_char = szBuff[j];
					// принадлежит ли данный знак цифровому интервалу?
					if (isnot_comment_char)
					{
						if (cur_char == '"')
						{
							isnot_comment_char = false;
						}
						else
						{
							bool is_tsyfra = cur_char >= 48 && cur_char <= 57;
							bool is_dot = cur_char == '.';
							bool is_delim = cur_char == delim;
							bool is_plus_minus = cur_char == '+' || cur_char == '-';

							if (is_tsyfra)
								have_tsyfra = true;


							if (!is_tsyfra && !is_delim && !is_dot && !is_plus_minus && cur_char != 10 && cur_char != 32)
								have_header = true;
						}
					}
					else
					{
						if (cur_char == '"')
						{
							isnot_comment_char = true;
						}
					}
				}


				ncol = 0;
				while ((p2 = strchr(p1,delim)) != NULL || p1-szBuff < (long)bytes_line )
				{
					ncol++;
					if ((p2 = strchr(p1,delim)) != NULL)
					{
						bytes_param = p2-p1+1;
					}
					else
					{
						bytes_param = strlen(p1);
					}
					strncpy(title, p1, bytes_param);
					title[bytes_param-1] = '\0';

					string s;
					s = title;
					if (pnames)
						pnames->push_back(s);

					if ((p2 = strchr(p1,delim)) != NULL)
					{
						p1 = p2+1;
					}
					else
						p1 = szBuff + bytes_line;
				}
			}
		}

		printf ("have_tsyfra = %d have_header = %d \"%s\"\n", have_tsyfra, have_header, szBuff);

		if (have_tsyfra && !have_header)
		{
		printf ("fclose(stream)\n");
			fclose(stream);
			if ((stream = fopen(fpath,"rt")) == NULL)
			{
				char errstr[2048];
				sprintf(errstr, "ReadDatFile:fopen(%s)", fpath);
				ErrorExit(errstr);
				return -1;
			}
		}

		// alloc memory for ncols vectors
		int ncols = ncol;

		// read all lines and remember data in plot vectors
		int i = 0;
		while (!feof(stream))
		{
			if ((i == 0 && bUse_Header) || i > 0)
				ch = fgets(szBuff,n,stream);
			if( ch != NULL && strlen(szBuff) > 1)
			{

				bytes_line = strlen(ch);
				// calculate progress
#if USE_HWND
				if (hWnd != NULL)
				{
					cb_read += bytes_line;
					if (cb_read - cb_disp > 2048) 
					{
					   // Advance the current position of the
					   // progress bar by the increment. 
						size_t todisp = (cb_read - cb_disp) / 2048;
						cb_disp += todisp*2048;
						for (size_t idisp = 0; idisp < todisp; idisp++)
							SendMessage(hwndPB, PBM_STEPIT, 0, 0); 
					}
				}
#endif
				p1 = szBuff;

				vector<double> temp_row;

				int ncol = 0;

				while ((p2 = strchr(p1,delim)) != NULL || p1-szBuff < (long)bytes_line)
				{
					ncol++;
					if ((p2 = strchr(p1,delim)) != NULL)
					{
						bytes_param = p2-p1+1;
					}
					else
					{
						bytes_param = strlen(p1);
					}
					strncpy(title, p1, bytes_param);
					title[bytes_param-1] = '\0';

					value = atof(title);
					if (value == 0.0)
						value = double(atoi(title));

					temp_row.push_back(value);

					if ((p2 = strchr(p1,delim)) != NULL)
					{
						p1 = p2+1;
					}
					else
						p1 = szBuff + bytes_line;
				}
				// add new row to the massive
				(*pvectors).push_back(temp_row);
				//
				i++;
			}
		}
		fclose(stream);
#if USE_HWND
		if (hWnd && hwndPB)
			DestroyWindow(hwndPB);
#endif
	}
	else
		MessageBox(0, ext, "Расширение файла не подходит", 
			MB_OK | MB_ICONINFORMATION);
	if(szBuff)
		HeapFree(GetProcessHeap(), 0,szBuff);
	return 0;
}


int ReadDatFile(const char * filename, vector<double> & x, vector<double> & y, vector<double> & z)
{
	const char *p;
	char ext[255];
	FILE *stream;
	const unsigned int n = 1023;
	char szBuff[n];

	p = strrchr(filename,'.');
	if (p)
		strcpy(ext, p+1);


	double X,Y,Z;

	if ((stream = fopen(filename,"rt")) == NULL)
	{
		MessageBox(0, "Cannot open input file.\n", "Import", 
			MB_OK | MB_ICONINFORMATION);
		return -1;
	}
	while (!feof(stream))
	{
		char * ch;
		ch = fgets(szBuff,n,stream);
		if( ch != NULL && strlen(szBuff) > 1)
		{
			int rez = sscanf(szBuff,"%lf\t%lf\t%lf",
							&X, &Y, &Z);
			if (rez == 3 && rez != EOF)
			{
				x.push_back(X);
				y.push_back(Y);
				z.push_back(Z);
			}
			else
			{
				rez = sscanf(szBuff,"%lf,%lf,%lf",
								&X, &Y, &Z);
				if (rez == 3 && rez != EOF)
				{
					x.push_back(X);
					y.push_back(Y);
					z.push_back(Z);
				}
			}
		}
	}
	fclose(stream);
	return 0;

}

int ReadCoordinateFromFile(const char * filename, double& f)
{
	FILE *stream;
	const unsigned int n = 1023;
	char szBuff[n];

	if ((stream = fopen(filename,"rt")) == NULL)
	{
		MessageBox(0, "Cannot open input file.\n", "Import", 
			MB_OK | MB_ICONINFORMATION);
		return -1;
	}
	if (!feof(stream))
	{
		char * ch;
		ch = fgets(szBuff,n,stream);
		if( ch != NULL && strlen(szBuff) > 1)
		{
			f = atof(ch);
			fclose(stream);
			return 0;
		}
	}
	fclose(stream);
	return -1;
}


int ReadBlnFile(const char * filename, vector<double>& x, vector<double>& y, int& inside)
{
	const char *p;
	char ext[255];
	FILE *stream;
	const unsigned int n = 1023;
	char szBuff[n];

	p = strrchr(filename,'.');
	if (p)
		strcpy(ext, p+1);

	if (p && strcmp(ext, "bln") == 0)
	{
		//MessageBox(0, ext, "dat",0);
		// open the file

		if ((stream = fopen(filename,"rt")) == NULL)
		{
			MessageBox(0, "Cannot open input file.\n", "Import", 
				MB_OK | MB_ICONINFORMATION);
			return -1;
		}
		int len = 0;
		double X,Y;
		if (!feof(stream))
		{
			char * ch;
			ch = fgets(szBuff,n,stream);
			if( ch != NULL && strlen(szBuff) > 1)
			{
				//len = atoi(ch);
				//int inside;
				int rez = sscanf(szBuff,"%d,%d",
										&len, &inside);

				if (rez != 2 || rez == EOF || len == 0)
				{
					fclose(stream);
					return -1;
				}				
			}
		}
		if (len == 0)
		{
			fclose(stream);
			return -1;
		}
		int i = 0;
		x.resize(len);
		y.resize(len);
		while (!feof(stream))
		{
			char * ch;
			ch = fgets(szBuff,n,stream);
			if( ch != NULL && strlen(szBuff) > 1)
			{
				int rez = sscanf(szBuff,"%lf,%lf",
								&X, &Y);
				if (i < len)
				{
					if (rez == 2 && rez != EOF)
					{
						x[i] = X;
						y[i] = Y;
						i++;
					}
					else
					{
						rez = sscanf(szBuff,"%lf\t%lf",
										&X, &Y);
						if (rez == 2 && rez != EOF)
						{
							x[i] = X;
							y[i] = Y;
							i++;
						}
					}
				}
			}
		}
		fclose(stream);
		return 0;
	}
	return -1;
}

int ReadBlnFile(const char * filename, vector<double>& x, vector<double>& y, vector<double>& z, int& inside)
{
	const char *p;
	char ext[255];
	FILE *stream;
	const unsigned int n = 1023;
	char szBuff[n];

	p = strrchr(filename,'.');
	if (p)
		strcpy(ext, p+1);

	if (p && strcmp(ext, "bln") == 0)
	{
		//MessageBox(0, ext, "dat",0);
		// open the file

		if ((stream = fopen(filename,"rt")) == NULL)
		{
			MessageBox(0, "Cannot open input file.\n", "Import", 
				MB_OK | MB_ICONINFORMATION);
			return -1;
		}
		int len = 0;
		double X,Y,Z;
		if (!feof(stream))
		{
			char * ch;
			ch = fgets(szBuff,n,stream);
			if( ch != NULL && strlen(szBuff) > 1)
			{
				//len = atoi(ch);
				int inside;
				int rez = sscanf(szBuff,"%d,%d",
										&len, &inside);

				if (rez != 2 || rez == EOF || len == 0)
				{
					fclose(stream);
					return -1;
				}			
			}
		}
		if (len == 0)
		{
			fclose(stream);
			return -1;
		}
		int i = 0;
		x.resize(len);
		y.resize(len);
		z.resize(len);
		while (!feof(stream))
		{
			char * ch;
			ch = fgets(szBuff,n,stream);
			if( ch != NULL && strlen(szBuff) > 1)
			{
				int rez = sscanf(szBuff,"%lf,%lf,%lf",
								&X, &Y, &Z);
				if (i < len)
				{
					if (rez == 3 && rez != EOF)
					{
						x[i] = X;
						y[i] = Y;
						y[i] = Z;
						i++;
					}
					else
					{
						rez = sscanf(szBuff,"%lf\t%lf\t%lf",
										&X, &Y, &Z);
						if (rez == 3 && rez != EOF)
						{
							x[i] = X;
							y[i] = Y;
							z[i] = Z;
							i++;
						}
					}
				}
			}
		}
		fclose(stream);
		return 0;
	}
	return -1;
}

int ReadBlnFile(const char * filename, vector<vector<vector<double> > > * vvvd, vector<int> * vinside)
{
	const char *p;
	char ext[255];
	FILE *stream;
	const unsigned int n = 1023;
	char szBuff[n];

	p = strrchr(filename,'.');
	if (p)
		strcpy(ext, p+1);

	if (p && strcmp(ext, "bln") == 0)
	{
		//MessageBox(0, ext, "dat",0);
		// open the file

		if ((stream = fopen(filename,"rt")) == NULL)
		{
			MessageBox(0, "Cannot open input file.\n", "Import", 
				MB_OK | MB_ICONINFORMATION);
			return -1;
		}
		int len = 0;
		double X,Y,Z;
		while (!feof(stream))
		{
			char * ch;
			ch = fgets(szBuff,n,stream);
			if( ch != NULL && strlen(szBuff) > 1)
			{
				//len = atoi(ch);
				int inside;
				char name[255];
				int rez1 = sscanf(szBuff,"%d,%d \"%s\"",
										&len, &inside, name);

				if (rez1 < 2 || rez1 == EOF || len == 0)
				{
					fclose(stream);
					return -1;
				}
				if ( rez1 == 3) Z = atof(name);
				if (vinside) vinside->push_back(inside);
				int i = 0;
				// begin new line
				vvvd->push_back(vector<vector<double> >());
				vector<double> point;
				while (!feof(stream) && i < len)
				{
					char * ch;
					ch = fgets(szBuff,n,stream);
					if( ch != NULL && strlen(szBuff) > 1)
					{
						point.clear();
						int rez = sscanf(szBuff,"%lf,%lf,%lf",
										&X, &Y, &Z);
						if (i < len)
						{
							if (rez == 3 && rez != EOF)
							{
								point.push_back(X);
								point.push_back(Y);
								point.push_back(Z);
								vvvd->back().push_back(point);
								i++;
							}
							else
							if (rez == 2 && rez != EOF)
							{
								point.push_back(X);
								point.push_back(Y);
								if (rez1 == 3 && Z != 0.0)
									point.push_back(Z);
								vvvd->back().push_back(point);
								i++;
							}
							else
							{
								rez = sscanf(szBuff,"%lf\t%lf\t%lf",
												&X, &Y, &Z);
								if (rez == 3 && rez != EOF)
								{
									point.push_back(X);
									point.push_back(Y);
									point.push_back(Z);
									vvvd->back().push_back(point);
									i++;
								}
								else
								if (rez == 2 && rez != EOF)
								{
									point.push_back(X);
									point.push_back(Y);
									vvvd->back().push_back(point);
									i++;
								}
							}
						}
					}
				}
			}
		}
		fclose(stream);
		return 0;
	}
	return -1;
}





