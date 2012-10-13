// GLADE_DIALOG.H : write a short description here...

// Copyright (C) 2002 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef GLADE_DIALOG_H
#define GLADE_DIALOG_H

#include "appconfig.h"

//#include <glade/glade.h>

/*################################################################################################*/

class glade_dialog
{
	protected:
	
//	GladeXML * xml;
	
	public:
	
	glade_dialog(const char *);
	virtual ~glade_dialog(void);
};

/*################################################################################################*/

#endif	// GLADE_DIALOG_H

// eof
