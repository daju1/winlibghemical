// OBJECTS.H : base classes for different 3D-objects.

// Copyright (C) 1998 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef OBJECTS_H
#define OBJECTS_H

#include "appconfig.h"

struct obj_loc_data;

class dummy_object;
class smart_object;

class object_location;

class ol_static;
class ol_dynamic_l1;

/*################################################################################################*/

class camera;		// camera.h
class graphics_view;	// views.h

#include "../libghemical/src/v3d.h"
#include "../libghemical/src/libdefine.h"

#include <list>
#include <vector>
using namespace std;

/*################################################################################################*/

/**	This sets the current modelview-matrix equivalent to the location and orientation 
	of the object without making any persistent changes to the matrix stack state.
*/

void SetModelView(const obj_loc_data *);

/**	Object location data is stored here.

	Directions are similar to those in OpenGL:
	"zdir" -> positive direction is to the forward.
	"ydir" -> positive direction is to the upward.
	"xdir" -> positive direction is to the left.
	
	All vectors are supposed to be unit vectors !!!
	Only lights will use the fourth component !!!
*/

struct obj_loc_data
{
	fGL crd[4];
	v3d<fGL> zdir;
	v3d<fGL> ydir;
	
	i32s lock_count;
};

/*################################################################################################*/

/**	The "##dummy_object" is a base class for many internally used objects (like cameras, 
	lights, clippers and transformers), and contains interfaces for transformations, 
	simple modifications and rendering.
	
	Some objects (like ribbons) are fixed so not all objects necessarily have object_location...
*/

class dummy_object
{
	protected:
	
	object_location * ol;
	i32s my_id_number;
	
	public:
	
	dummy_object(bool);
	dummy_object(const object_location &);
	virtual ~dummy_object(void);
	
	void SetModelView(void) const;
	const obj_loc_data * GetLocData(void) const;
	
	protected:
	
	obj_loc_data * GetLocDataRW(void) const;
	
	public:
	
	virtual const char * GetObjectName(void) = 0;
	
	virtual void OrbitObject(const fGL *, const camera &);
	virtual void RotateObject(const fGL *, const camera &);
	
	virtual void TranslateObject(const fGL *, const obj_loc_data *);

	virtual bool BeginTransformation(void) = 0;
	virtual bool EndTransformation(void) = 0;
	
	virtual void Render(void) = 0;
};

/*################################################################################################*/

// UNDER CONSTRUCTION!!!!! UNDER CONSTRUCTION!!!!! UNDER CONSTRUCTION!!!!!
// UNDER CONSTRUCTION!!!!! UNDER CONSTRUCTION!!!!! UNDER CONSTRUCTION!!!!!
// UNDER CONSTRUCTION!!!!! UNDER CONSTRUCTION!!!!! UNDER CONSTRUCTION!!!!!

//struct smart_client
//{
//	graphics_view * client;
//	void * custom_data;
//};
//	vector<smart_client> client_vector;

/**	The "##smart_object" is used to implement complex visualization objects (like colored 
	3D-planes and surfaces, and ribbon models of peptides/proteins).
	
	A "##smart_object" can respond to connectivity/geometry changes and can have other 
	customized properties. Clippers can be used to clip smart_objects (AND ONLY THOSE?????). 
	BAD NEWS: the clipper objects does not exist yet.
	
	"##new connectivity" means that some atoms/bonds have been added/removed.
	"##new geometry" means that some atom coordinates have been modified.
	
	THIS IS STILL A BIT UNDER CONSTRUCTION!!!
*/

class smart_object : public dummy_object
{
	private:
	
	list<camera *> cam_list;
	
	public:
	
	bool transparent;	// why public???
	
	public:
	
	smart_object(void);
	smart_object(const object_location &);
	virtual ~smart_object(void);
	
	void ConnectCamera(camera &);
	
	virtual void CameraEvent(const camera &) = 0;
	
/*##############################################*/
/*##############################################*/

//	virtual bool AddClient(graphics_view *, void *) = 0;
//	virtual bool RemoveClient(graphics_view *) = 0;
	
//	virtual void NewConnectivity(void) = 0;
//	virtual void NewGeometry(void) = 0;
};

/*################################################################################################*/

/**	The "##object_location" is an abstract base class for all objects(???). It contains 
	information about the position and orientation of the object and is also able to write 
	that information into the OpenGL modelview matrix.
	
	Idea is to create derived classes that can have different (also dynamic) ways to 
	determine the location...
*/

class object_location
{
	protected:
	
	friend obj_loc_data * dummy_object::GetLocDataRW(void) const;
	
	obj_loc_data * data;
	
	public:
	
	object_location(void);
	object_location(const object_location &);
	virtual ~object_location(void);
	
	void SetModelView(void) const;
	const obj_loc_data * GetLocData(void) const;
	
	virtual void UpdateLocation(void) const = 0;
	virtual object_location * MakeCopy(void) const = 0;
};

/*################################################################################################*/

/// A simple static object location - seems to work, just use this...

class ol_static : public object_location
{
	protected:
	
// this is just an interface definition -> there is no relevant common data?!?!?
// this is just an interface definition -> there is no relevant common data?!?!?
// this is just an interface definition -> there is no relevant common data?!?!?

	public:
	
	ol_static(void);
	ol_static(const obj_loc_data *);
	~ol_static(void);
	
	void UpdateLocation(void) const;
	object_location * MakeCopy(void) const;
};

/**	A dynamic object location (UNDER CONSTRUCTION).
	
	The idea here is, that we could make objects that are automatically located using 
	some external data; for example we could have a camera or a light that is always 
	focused on a target, no matter how the target moves. We would have here pointers 
	to the target coordinates, and we would update the object location dynamically 
	using those...
*/

class ol_dynamic_l1 : public object_location
{
	private:
	
// bring the points and weights here?!?!?!?!?
// bring the points and weights here?!?!?!?!?
// bring the points and weights here?!?!?!?!?

	public:
	
	ol_dynamic_l1(void);
	~ol_dynamic_l1(void);
	
	void UpdateLocation(void) const;
	object_location * MakeCopy(void) const;
};

/*################################################################################################*/

#endif	// OBJECTS_H

// eof
