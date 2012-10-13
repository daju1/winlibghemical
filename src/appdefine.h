// APPDEFINE.H : just some macros #defined here...

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef APPDEFINE_H
#define APPDEFINE_H

#include "appconfig.h"

/*################################################################################################*/

#define GLNAME_LIGHT	0x0001
#define GLNAME_OBJECT	0x0002

#define GLNAME_MODEL_DEPENDENT 0xFF00		// all these events are forwarded...

#define GLNAME_MD_TYPE1 (GLNAME_MODEL_DEPENDENT | 0x01)
#define GLNAME_MD_TYPE2 (GLNAME_MODEL_DEPENDENT | 0x02)

/*################################################################################################*/

#define RENDER_NOTHING		0x00
#define RENDER_WIREFRAME	0x01
#define RENDER_BALL_AND_STICK	0x02
#define RENDER_VAN_DER_WAALS	0x03
#define RENDER_ACCESSIBLE	0x04
#define RENDER_CYLINDERS	0x05

#define LABEL_NOTHING		0x00
#define LABEL_INDEX		0x01
#define LABEL_F_CHARGE		0x02
#define LABEL_P_CHARGE		0x03
#define LABEL_ELEMENT		0x04
#define LABEL_BUILDER_ID	0x05
#define LABEL_ATOMTYPE		0x06
#define LABEL_BONDTYPE		0x10
#define LABEL_RESIDUE		0x20
#define LABEL_SEC_STRUCT	0x21

/*################################################################################################*/

#endif	// APPDEFINE_H

// eof
