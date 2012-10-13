// CAMERA.H : the camera class + some other viewing/utility classes.

// Copyright (C) 1998 Tommi Hassinen, Jarno Huuskonen, Mike Cruz.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef CAMERA_H
#define CAMERA_H

#include "appconfig.h"

class camera;

class light;
class spot_light;
class directional_light;

//class clipper; ???
//class single_clipper;
//class double_clipper;

class transformer;

/*################################################################################################*/

class graphics_view;	// views.h
class project;		// project.h

#include "objects.h"

/*################################################################################################*/

/**	The "##camera" class holds information about position and orientation of the views 
	in viewing windows. A single camera can pass this information to multiple windows. 
	There can also be "##local lights", lights connected to a camera. If some 
	translations/rotations are done to the camera, these lights will follow...
*/

class camera : public dummy_object
{
	private:
	
	list<smart_object *> obj_list;
	
	public:
	
	project * prj;
	
	i32s my_c_number;
	i32s graphics_view_counter;
	
	char * my_object_name;
	
	fGL focus;
	fGL clipping;
	
	bool use_local_lights;
	bool use_global_lights;
	
	bool ortho;
	bool stereo_mode;	// this is true if any stereo mode; relaxed-eye or color-based.
	bool stereo_relaxed;	// this is true if relaxed_eye stereo mode.
	float slider;
	float rang;
	
	public:
	
	enum rmode { Normal = 0, Draw = 1, Erase = 2, Select = 3, Measure = 4 };
	
	camera(const object_location &, fGL, project *);
	camera(const camera &);
	~camera(void);
	
	bool CopySettings(const camera *);
	
/**	This will make the view position/orientation ready for actual rendering 
	commands. Also some extra operations for drawing/selection have found their 
	way here... Some drawing/selection events are passed here to the models.
*/
	void RenderWindow(graphics_view *, rmode = Normal, int = 0, int = 0);
	
	/// This just draws the objects for a window...
	
	void RenderObjects(graphics_view *);
	
	void DoCameraEvents(void);
	
	const char * GetObjectName(void) { return my_object_name; }	// virtual
	
	void OrbitObject(const fGL *, const camera &);		// virtual
	void RotateObject(const fGL *, const camera &);		// virtual
	
	void TranslateObject(const fGL *, const obj_loc_data *);	// virtual
	
	bool BeginTransformation(void) { return false; }	// virtual
	bool EndTransformation(void) { return false; }		// virtual
	
	void Render(void) { }		// virtual
	
	friend class smart_object;
};

/*################################################################################################*/

struct light_components
{
	fGL * amb_comp;
	fGL * diff_comp;
	fGL * spec_comp;
};

/**	The "##light" class is a base class for OpenGL lights. All lights must be 
	dummy_objects, because they are connected to a camera rather than to a set of 
	windows. That's because cameras will drag their local lights along when they 
	are rotated/translated...
	
	Also there is no need for lights to respond to any connectivity/geometry changes.
*/

class light : public light_components, public dummy_object
{
	public:
	
	/**	This should be either NULL (for a global light) or some valid camera of 
		the model (for a local light). Only model::Add???Light()-functions are allowed 
		to make modifications to "##owner" !!!
	*/
	
	camera * owner;
	
	/**	This is the OpenGL light number GL_LIGHTi for this light. The actual values 
		are determined in the model when adding/removing local/global lights...
	*/
	
	iGLs number;
	
	char * my_object_name;
	
	static const fGL def_amb_comp[4];
	static const fGL def_diff_comp[4];
	static const fGL def_spec_comp[4];
	
	static const light_components def_components;
	
	public:
	
	light(const object_location &, const light_components &);
	virtual ~light(void);
	
	/**	This is used to set up those light properties that are independent on 
		location/orientation -> should be called for every window when it's created 
		and always when the light numbering is changed.
	*/
	
	virtual void SetupProperties(void) = 0;
	
	/**	This should be called for all active lights always when rendering them, 
		after the modelview-matrix is ready but before any actual rendering commands.
	*/
	
	virtual void SetupLocation(void) = 0;
	
	bool BeginTransformation(void) { return false; }	// virtual
	bool EndTransformation(void) { return false; }		// virtual
	
	private:
	
	void InitComponents(const light_components *);
};

/*################################################################################################*/

/**	The OpenGL spotlight (a light source at finite distance, like a flashlight or a candle). 
	It has the standard cutoff/exponent-properties, and will be rendered as a sphere/cone...
*/

class spot_light : public light
{
	public:
	
	fGL cutoff;
	fGL exponent;
	
	static fGL size;
	
	static fGL shade1[3];
	static fGL shade2[3];
	
	static fGL bulb_on[3];
	static fGL bulb_off[3];
	
	static const fGL def_cutoff;
	static const fGL def_exponent;
	
	public:
	
	spot_light(const object_location &, i32s, const light_components & = light::def_components, fGL = spot_light::def_cutoff, fGL = spot_light::def_exponent);
	virtual ~spot_light(void);
	
	const char * GetObjectName(void) { return my_object_name; }	// virtual
	
	void SetupProperties(void);		// virtual
	void SetupLocation(void);		// virtual
	
	void Render(void);		// virtual
};

/*################################################################################################*/

/**	The OpenGL directional light (a light source at infinite distance, like the sunlight). 
	No rendering for this one...
*/

class directional_light : public light
{
	protected:
	
	public:
	
	directional_light(const object_location &, i32s, const light_components & = light::def_components);
	virtual ~directional_light(void);
	
	const char * GetObjectName(void) { return my_object_name; }	// virtual
	
	void SetupProperties(void);		// virtual
	void SetupLocation(void);		// virtual
	
	void Render(void);		// virtual
};

/*################################################################################################*/

/**	The "##transformer" is a special class used only when rotating/translating some 
	selected part of the model. transformer::GetMatrix() is used to get the transformation 
	matrix for making those modifications permanent.
	
	It is useful to switch orbiting and rotating, so that objects will behave similarly to 
	lights and the whole scene...
*/

class transformer : public dummy_object
{
	public:
	
	project * target_prj;
	
	static bool transform_in_progress;
	
	friend class project;		// temporary !?!?!?!?
	
	public:
	
	transformer(void);
	~transformer(void);
	
	void Init(project *);
	void GetMatrix(fGL *) const;
	
	const char * GetObjectName(void) { return "[transformer]"; }	// virtual
	
	bool BeginTransformation(void);		// virtual
	bool EndTransformation(void);		// virtual
	
	void OrbitObject(const fGL *, const camera &);		// virtual
	void RotateObject(const fGL *, const camera &);		// virtual
	
	void Render(void);	// virtual
};

/*################################################################################################*/

#endif	// CAMERA_H

// eof
