// NOTICE.CPP

// Copyright (C) 2000 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "notice.h"

#include "libdefine.h"

#include <strstream>
using namespace std;

/*################################################################################################*/

const char * get_lib_intro_notice_line(int line)
{
	static char buffer[256];
	ostrstream str(buffer, sizeof(buffer));
	
	switch (line)
	{
		case 0:		str << "libghemical-" << LIBVERSION << " released on " << LIBRELEASEDATE << ends; break;
		case 1:		str << " " << ends; break;
		case 2:		str << "For more information please visit " << WEBSITE << ends; break;
		case 3:		str << " " << ends; break;
		
		default:	str << ends;
	}
	
	return buffer;
};

const char * get_copyright_notice_line(int line)
{
	static char buffer[256];
	ostrstream str(buffer, sizeof(buffer));
	
	switch (line)
	{
		case 0:		str << "Copyright (C) 1998 Tommi Hassinen and others." << ends; break;
		case 1:		str << " " << ends; break;
		case 2:		str << "OpenBabel Copyright (C) 1998 OpenEye Scientific and others." << ends; break;
		case 3:		str << "OpenBabel homepage is http://openbabel.sourceforge.net/" << ends; break;
		case 4:		str << " " << ends; break;
		case 5:		str << "MOPAC7 by James J.P. Stewart and others is in Public Domain." << ends; break;
		case 6:		str << "The MOPAC7 based code (libmopac7) included in this program" << ends; break;
		case 7:		str << "is also in Public Domain." << ends; break;
		case 8:		str << " " << ends; break;
		case 9:		str << "MPQC Copyright (C) 1997 Limit Point Systems, Inc. and others." << ends; break;
		case 10:	str << "MPQC homepage is http://www.mpqc.org/" << ends; break;
		case 11:	str << " " << ends; break;
		case 12:	str << "This program is free software; you can redistribute it and/or" << ends; break;
		case 13:	str << "modify it under the terms of the GNU General Public License" << ends; break;
		case 14:	str << "as published by the Free Software Foundation; either version" << ends; break;
		case 15:	str << "2 of the License, or any later version." << ends; break;
		case 16:	str << " " << ends; break;
		case 17:	str << "This program is distributed in the hope that it will be useful," << ends; break;
		case 18:	str << "but WITHOUT ANY WARRANTY; without even the implied warranty of" << ends; break;
		case 19:	str << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the" << ends; break;
		case 20:	str << "GNU General Public License for more details." << ends; break;
		
		default:	str << ends;
	}
	
	return buffer;
}

void print_lib_intro_notice(ostream & p1)
{
	for (int line = 0;line < LIB_INTRO_NOTICE_LINES;line++)
	{
		p1 << get_lib_intro_notice_line(line) << endl;
	}
}

void print_copyright_notice(ostream & p1)
{
	for (int line = 0;line < COPYRIGHT_NOTICE_LINES;line++)
	{
		p1 << get_copyright_notice_line(line) << endl;
	}
}

/*################################################################################################*/

// eof
