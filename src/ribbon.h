// RIBBON.H : ribbon models for peptides/proteins.

// Copyright (C) 1998 Tommi Hassinen, Jarno Huuskonen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef RIBBON_H
#define RIBBON_H

#include "appconfig.h"

class ribbon;

/*################################################################################################*/

class spline;	 	// spline.h

#include "project.h"
#include "objects.h"

/*################################################################################################*/

class ribbon : public smart_object
{
	public:
	
	project * prj;
	color_mode * cmode;
	
	i32s extra_points;
	i32s chn; i32s length;
	
	fGL * cp1;		// control points (three components)
	fGL * cp2;		// control points (three components)
	
	spline * ref1;
	fGL * head_refs1[2];
	fGL * head_points1;	// (three components)
	fGL * tail_refs1[2];
	fGL * tail_points1;	// (three components)
	
	spline * ref2;
	fGL * head_refs2[2];
	fGL * head_points2;	// (three components)
	fGL * tail_refs2[2];
	fGL * tail_points2;	// (three components)
	
	fGL * data1;		// places of the control points in spline
	fGL * data2a;		// coordinates calculated using the spline (three components)
	fGL * data2b;		// coordinates calculated using the spline (three components)
	fGL * data3;		// colors of the points (three components)
	
	iGLu list_id;
	
	static const i32s resol;
	
	static const fGL width;
	static const fGL helix;
	
	char * my_object_name;
	
	public:
	
	ribbon(project *, color_mode *, i32s, i32s);
	~ribbon(void);
	
	void UpdateExtraPoints(fGL **, fGL *, fGL **, fGL *);
	
	const char * GetObjectName(void) { return my_object_name; }	// virtual

	void CameraEvent(const camera &) { }	// virtual
	
	bool BeginTransformation(void) { return false; }	// virtual
	bool EndTransformation(void) { return false; }		// virtual
	
	void Render(void);		// virtual
};

/*################################################################################################*/

#endif	// RIBBON_H

// eof
