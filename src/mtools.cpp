// MTOOLS.CPP

// Copyright (C) 1998 Tommi Hassinen, Jarno Huuskonen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "mtools.h"	// config.h is here -> we get ENABLE-macros here...

#include "project.h"
#include "views.h"

#include "appdefine.h"

/*################################################################################################*/

transformer mouse_tool::tool_transformer = transformer();
i32s mouse_tool::latest_x = NOT_DEFINED; i32s mouse_tool::latest_y = NOT_DEFINED;
fGL mouse_tool::ang_sensitivity = 180.0; fGL mouse_tool::dist_sensitivity = 2.0;

mouse_tool::mouse_tool(void)
{
}

mouse_tool::~mouse_tool(void)
{
}

/*################################################################################################*/

void draw_tool::ButtonEvent(ogl_view * oglv, i32s x, i32s y)
{
	latest_x = x; latest_y = y;
	
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL) gv->cam->RenderWindow(gv, camera::Draw, x, y);
}

void draw_tool::MotionEvent(ogl_view *, i32s, i32s)
{
}

/*################################################################################################*/

void erase_tool::ButtonEvent(ogl_view * oglv, i32s x, i32s y)
{
	latest_x = x; latest_y = y;
	
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL) gv->cam->RenderWindow(gv, camera::Erase, x, y);
}

void erase_tool::MotionEvent(ogl_view *, i32s, i32s)
{
}

/*################################################################################################*/

void select_tool::ButtonEvent(ogl_view * oglv, i32s x, i32s y)
{
	if (ogl_view::state != mouse_tool::Down) return;
	
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL) gv->cam->RenderWindow(gv, camera::Select, x, y);
	
	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv != NULL) pv->UserEvent(x, y);
}

void select_tool::MotionEvent(ogl_view * oglv, i32s x, i32s y)
{
	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv != NULL) pv->UserEvent(x, y);
}

/*################################################################################################*/

void zoom_tool::ButtonEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		if (gv->quick_update && gv && gv->GetProject() && gv->cam)
		{
			if (ogl_view::state == mouse_tool::Down)
			{
				render = gv->render;
				accumulate = gv->accumulate;
				
				gv->render = RENDER_WIREFRAME;
				gv->accumulate = false;
			}
			else
			{
				gv->render = render;
				gv->accumulate = accumulate;
				
				gv->GetProject()->UpdateGraphicsViews(gv->cam);
			}
		}
	}
	
	latest_x = x;
	latest_y = y;
}

void zoom_tool::MotionEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		fGL dist[3] = { 0.0, 0.0, 0.0 };
		if (gv && gv->GetProject() && gv->cam)
		{
			dist[2] = dist_sensitivity * gv->range[1] * (fGL) (latest_y - y) / (fGL) gv->size[1];
			gv->cam->TranslateObject(dist, gv->cam->GetLocData());
			gv->cam->focus += dist[2];
			
			if (gv->quick_update) gv->GetProject()->UpdateGraphicsView(gv);
			else gv->GetProject()->UpdateGraphicsViews(gv->cam);
		}
	}
	
	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv != NULL) pv->ZoomEvent(latest_y - y);
	
	latest_x = x;
	latest_y = y;
}

/*################################################################################################*/

void clipping_tool::ButtonEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		if (gv->quick_update && gv && gv->GetProject() && gv->cam)
		{
			if (ogl_view::state == mouse_tool::Down)
			{
				render = gv->render;
				accumulate = gv->accumulate;
				
				gv->render = RENDER_WIREFRAME;
				gv->accumulate = false;
			}
			else
			{
				gv->render = render;
				gv->accumulate = accumulate;
				
				gv->GetProject()->UpdateGraphicsViews(gv->cam);
			}
		}
	}
	
	latest_x = x;
	latest_y = y;
}

void clipping_tool::MotionEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		if (gv && gv->GetProject() && gv->cam)
		{
			gv->cam->clipping += (fGL) (latest_y - y) / (fGL) gv->size[1];
			if (gv->cam->clipping < 0.01) gv->cam->clipping = 0.01;
			if (gv->cam->clipping > 0.99) gv->cam->clipping = 0.99;
			
			cout << "clipping = " << gv->cam->clipping << " = ";
			cout << (gv->cam->clipping * (2.0 * gv->cam->focus)) << " nm." << endl;
			
			if (gv->quick_update) gv->GetProject()->UpdateGraphicsView(gv);
			else gv->GetProject()->UpdateGraphicsViews(gv->cam);
		}
	}
	
	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv != NULL) pv->ZoomEvent(latest_y - y);
	
	latest_x = x;
	latest_y = y;
}

/*################################################################################################*/

void translate_xy_tool::ButtonEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		if (ogl_view::state == mouse_tool::Down)
		{
			if (gv->quick_update)		// if quick, change the rendering params...
			{
				render = gv->render;
				accumulate = gv->accumulate;
				
				gv->render = RENDER_WIREFRAME;
				gv->accumulate = false;
			}
			
			if (!ogl_view::shift_down)
			{
				ref_to_object = gv->cam; local_object = true;
			}
			else
			{
				ref_to_object = gv->GetProject()->selected_object;
				
				i32s index = gv->GetProject()->IsLight(ref_to_object);
				local_object = (index != NOT_DEFINED && gv->GetProject()->light_vector[index]->owner == gv->cam);
				
				if (!ref_to_object && ogl_view::shift_down)
				{
					tool_transformer.Init(gv->GetProject());
					ref_to_object = (& tool_transformer);
				}
				
				if (ref_to_object) ref_to_object->BeginTransformation();
			}
			
			// if CTRL is down, use object's own direction, not that of camera's !!!!!!!!!
			// if CTRL is down, use object's own direction, not that of camera's !!!!!!!!!
			// if CTRL is down, use object's own direction, not that of camera's !!!!!!!!!
			
			if (ogl_view::ctrl_down) tdata = ref_to_object->GetLocData();
			else tdata = gv->cam->GetLocData();
		}
		else
		{
			if (ref_to_object) ref_to_object->EndTransformation();
			
			if (gv->quick_update)		// if quick, put back original params and update...
			{
				gv->render = render;
				gv->accumulate = accumulate;
				
				if (local_object) gv->GetProject()->UpdateGraphicsViews(gv->cam);
				else gv->GetProject()->UpdateAllGraphicsViews();
			}
		}
	}
	
	latest_x = x;
	latest_y = y;
}

void translate_xy_tool::MotionEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		fGL dist[3] = { 0.0, 0.0, 0.0 };
		if (gv && gv->GetProject() && ref_to_object && tdata)
		{
			dist[0] = dist_sensitivity * gv->range[0] * (fGL) (latest_x - x) / (fGL) gv->size[0];
			dist[1] = dist_sensitivity * gv->range[1] * (fGL) (latest_y - y) / (fGL) gv->size[1];
			ref_to_object->TranslateObject(dist, tdata);
			
			if (gv->quick_update) gv->GetProject()->UpdateGraphicsView(gv);
			else
			{
				if (local_object) gv->GetProject()->UpdateGraphicsViews(gv->cam);
				else gv->GetProject()->UpdateAllGraphicsViews();
			}
		}
	}
	
	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv != NULL) pv->TransEvent(latest_y - y);
	
	latest_x = x;
	latest_y = y;
}

/*################################################################################################*/

void translate_z_tool::ButtonEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		if (ogl_view::state == mouse_tool::Down)
		{
			if (gv->quick_update)		// if quick, change the rendering params...
			{
				render = gv->render;
				accumulate = gv->accumulate;
				
				gv->render = RENDER_WIREFRAME;
				gv->accumulate = false;
			}
			
			if (!ogl_view::shift_down)
			{
				ref_to_object = gv->cam; local_object = true;
			}
			else
			{
				ref_to_object = gv->GetProject()->selected_object;
				
				i32s index = gv->GetProject()->IsLight(ref_to_object);
				local_object = (index != NOT_DEFINED && gv->GetProject()->light_vector[index]->owner == gv->cam);
				
				if (!ref_to_object && ogl_view::shift_down)
				{
					tool_transformer.Init(gv->GetProject());
					ref_to_object = (& tool_transformer);
				}
				
				if (ref_to_object) ref_to_object->BeginTransformation();
			}
			
			// if CTRL is down, use object's own direction, not that of camera's !!!!!!!!!
			// if CTRL is down, use object's own direction, not that of camera's !!!!!!!!!
			// if CTRL is down, use object's own direction, not that of camera's !!!!!!!!!
			
			if (ogl_view::ctrl_down) tdata = ref_to_object->GetLocData();
			else tdata = gv->cam->GetLocData();
		}
		else
		{
			if (ref_to_object) ref_to_object->EndTransformation();
			
			if (gv->quick_update)		// if quick, put back original params and update...
			{
				gv->render = render;
				gv->accumulate = accumulate;
				
				if (local_object) gv->GetProject()->UpdateGraphicsViews(gv->cam);
				else gv->GetProject()->UpdateAllGraphicsViews();
			}
		}
	}
	
	latest_x = x;
	latest_y = y;
}

void translate_z_tool::MotionEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		fGL dist[3] = { 0.0, 0.0, 0.0 };
		if (gv && gv->GetProject() && ref_to_object && tdata)
		{
			dist[2] = dist_sensitivity * gv->range[1] * (fGL) (latest_y - y) / (fGL) gv->size[1];
			ref_to_object->TranslateObject(dist, tdata);
			
			if (gv->quick_update) gv->GetProject()->UpdateGraphicsView(gv);
			else
			{
				if (local_object) gv->GetProject()->UpdateGraphicsViews(gv->cam);
				else gv->GetProject()->UpdateAllGraphicsViews();
			}
		}
	}
	
	latest_x = x;
	latest_y = y;
}

/*################################################################################################*/

void orbit_xy_tool::ButtonEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		if (ogl_view::state == mouse_tool::Down)
		{
			if (gv->quick_update)		// if quick, change the rendering params...
			{
				render = gv->render;
				accumulate = gv->accumulate;
				
				gv->render = RENDER_WIREFRAME;
				gv->accumulate = false;
			}
			
			if (!ogl_view::shift_down)
			{
				ref_to_object = gv->cam; local_object = true;
			}
			else
			{
				ref_to_object = gv->GetProject()->selected_object;
				
				i32s index = gv->GetProject()->IsLight(ref_to_object);
				local_object = (index != NOT_DEFINED && gv->GetProject()->light_vector[index]->owner == gv->cam);
			
				if (!ref_to_object && ogl_view::shift_down)
				{
					tool_transformer.Init(gv->GetProject());
					ref_to_object = (& tool_transformer);
				}
				
				if (ref_to_object) ref_to_object->BeginTransformation();
			}
			
			rdata = gv->cam;
		}
		else
		{
			if (ref_to_object) ref_to_object->EndTransformation();
			
			if (gv->quick_update)		// if quick, put back original params and update...
			{
				gv->render = render;
				gv->accumulate = accumulate;
				
				if (local_object) gv->GetProject()->UpdateGraphicsViews(gv->cam);
				else gv->GetProject()->UpdateAllGraphicsViews();
			}
		}
	}

	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv != NULL) pv->UserEvent(x, y);
	
	latest_x = x;
	latest_y = y;
}

void orbit_xy_tool::MotionEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		fGL ang[3] = { 0.0, 0.0, 0.0 };
		if (gv && gv->GetProject() && ref_to_object)
		{
			ang[0] = ang_sensitivity * (fGL) (latest_y - y) / (fGL) gv->size[1];
			ang[1] = ang_sensitivity * (fGL) (x - latest_x) / (fGL) gv->size[0];
			ref_to_object->OrbitObject(ang, * rdata);
			
			if (gv->quick_update) gv->GetProject()->UpdateGraphicsView(gv);
			else
			{
				if (local_object) gv->GetProject()->UpdateGraphicsViews(gv->cam);
				else gv->GetProject()->UpdateAllGraphicsViews();
			}
		}
	}

	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv != NULL) pv->UserEvent(x, y);
	
	latest_x = x;
	latest_y = y;
}

/*################################################################################################*/

void orbit_z_tool::ButtonEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		if (ogl_view::state == mouse_tool::Down)
		{
			if (gv->quick_update)		// if quick, change the rendering params...
			{
				render = gv->render;
				accumulate = gv->accumulate;
				
				gv->render = RENDER_WIREFRAME;
				gv->accumulate = false;
			}
			
			if (!ogl_view::shift_down)
			{
				ref_to_object = gv->cam; local_object = true;
			}
			else
			{
				ref_to_object = gv->GetProject()->selected_object;
				
				i32s index = gv->GetProject()->IsLight(ref_to_object);
				local_object = (index != NOT_DEFINED && gv->GetProject()->light_vector[index]->owner == gv->cam);
				
				if (!ref_to_object && ogl_view::shift_down)
				{
					tool_transformer.Init(gv->GetProject());
					ref_to_object = (& tool_transformer);
				}
				
				if (ref_to_object) ref_to_object->BeginTransformation();
			}
			
			rdata = gv->cam;
		}
		else
		{
			if (ref_to_object) ref_to_object->EndTransformation();
			
			if (gv->quick_update)		// if quick, put back original params and update...
			{
				gv->render = render;
				gv->accumulate = accumulate;
				
				if (local_object) gv->GetProject()->UpdateGraphicsViews(gv->cam);
				else gv->GetProject()->UpdateAllGraphicsViews();
			}
		}
	}

	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv != NULL) pv->UserEvent(x, y);
	
	latest_x = x;
	latest_y = y;
}

void orbit_z_tool::MotionEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		fGL ang[3] = { 0.0, 0.0, 0.0 };
		if (gv && gv->GetProject() && ref_to_object)
		{
			ang[2] = ang_sensitivity * (fGL) (x - latest_x) / (fGL) gv->size[0];
			ref_to_object->OrbitObject(ang, * rdata);
			
			if (gv->quick_update) gv->GetProject()->UpdateGraphicsView(gv);
			else
			{
				if (local_object) gv->GetProject()->UpdateGraphicsViews(gv->cam);
				else gv->GetProject()->UpdateAllGraphicsViews();
			}
		}
	}

	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv != NULL) pv->UserEvent(x, y);
	
	latest_x = x;
	latest_y = y;
}

/*################################################################################################*/

void rotate_xy_tool::ButtonEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		if (ogl_view::state == mouse_tool::Down)
		{
			if (gv->quick_update)		// if quick, change the rendering params...
			{
				render = gv->render;
				accumulate = gv->accumulate;
				
				gv->render = RENDER_WIREFRAME;
				gv->accumulate = false;
			}
			
			if (!ogl_view::shift_down)
			{
				ref_to_object = gv->cam; local_object = true;
			}
			else
			{
				ref_to_object = gv->GetProject()->selected_object;
				
				i32s index = gv->GetProject()->IsLight(ref_to_object);
				local_object = (index != NOT_DEFINED && gv->GetProject()->light_vector[index]->owner == gv->cam);
				
				if (!ref_to_object && ogl_view::shift_down)
				{
					tool_transformer.Init(gv->GetProject());
					ref_to_object = (& tool_transformer);
				}
				
				if (ref_to_object) ref_to_object->BeginTransformation();
			}
			
			rdata = gv->cam;
		}
		else
		{
			if (ref_to_object) ref_to_object->EndTransformation();
			
			if (gv->quick_update)		// if quick, put back original params and update...
			{
				gv->render = render;
				gv->accumulate = accumulate;
				
				if (local_object) gv->GetProject()->UpdateGraphicsViews(gv->cam);
				else gv->GetProject()->UpdateAllGraphicsViews();
			}
		}
	}

	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv != NULL) pv->UserEvent(x, y);
	
	latest_x = x;
	latest_y = y;
}

void rotate_xy_tool::MotionEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		fGL ang[3] = { 0.0, 0.0, 0.0 };
		if (gv && gv->GetProject() && ref_to_object)
		{
			ang[0] = ang_sensitivity * (fGL) (latest_y - y) / (fGL) gv->size[1];
			ang[1] = ang_sensitivity * (fGL) (x - latest_x) / (fGL) gv->size[0];
			ref_to_object->RotateObject(ang, * rdata);
			
			if (gv->quick_update) gv->GetProject()->UpdateGraphicsView(gv);
			else
			{
				if (local_object) gv->GetProject()->UpdateGraphicsViews(gv->cam);
				else gv->GetProject()->UpdateAllGraphicsViews();
			}
		}
	}

	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv != NULL) pv->UserEvent(x, y);
	
	latest_x = x;
	latest_y = y;
}

/*################################################################################################*/

void rotate_z_tool::ButtonEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		if (ogl_view::state == mouse_tool::Down)
		{
			if (gv->quick_update)		// if quick, change the rendering params...
			{
				render = gv->render;
				accumulate = gv->accumulate;
				
				gv->render = RENDER_WIREFRAME;
				gv->accumulate = false;
			}
			
			if (!ogl_view::shift_down)
			{
				ref_to_object = gv->cam; local_object = true;
			}
			else
			{
				ref_to_object = gv->GetProject()->selected_object;
				
				i32s index = gv->GetProject()->IsLight(ref_to_object);
				local_object = (index != NOT_DEFINED && gv->GetProject()->light_vector[index]->owner == gv->cam);
				
				if (!ref_to_object && ogl_view::shift_down)
				{
					tool_transformer.Init(gv->GetProject());
					ref_to_object = (& tool_transformer);
				}
				
				if (ref_to_object) ref_to_object->BeginTransformation();
			}
			
			rdata = gv->cam;
		}
		else
		{
			if (ref_to_object) ref_to_object->EndTransformation();
			
			if (gv->quick_update)		// if quick, put back original params and update...
			{
				gv->render = render;
				gv->accumulate = accumulate;
				
				if (local_object) gv->GetProject()->UpdateGraphicsViews(gv->cam);
				else gv->GetProject()->UpdateAllGraphicsViews();
			}
		}
	}

	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv != NULL) pv->UserEvent(x, y);
	
	latest_x = x;
	latest_y = y;
}

void rotate_z_tool::MotionEvent(ogl_view * oglv, i32s x, i32s y)
{
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL)
	{
		fGL ang[3] = { 0.0, 0.0, 0.0 };
		if (gv && gv->GetProject() && ref_to_object)
		{
			ang[2] = ang_sensitivity * (fGL) (x - latest_x) / (fGL) gv->size[0];
			ref_to_object->RotateObject(ang, * rdata);
		
			if (gv->quick_update) gv->GetProject()->UpdateGraphicsView(gv);
			else
			{
				if (local_object) gv->GetProject()->UpdateGraphicsViews(gv->cam);
				else gv->GetProject()->UpdateAllGraphicsViews();
			}
		}
	}

	plotting_view * pv = dynamic_cast<plotting_view *>(oglv);
	if (pv != NULL) pv->UserEvent(x, y);
	
	latest_x = x;
	latest_y = y;
}

/*################################################################################################*/

// the operating principle here is the same as in select_tool above...

void measure_tool::ButtonEvent(ogl_view * oglv, i32s x, i32s y)
{
	if (ogl_view::state != mouse_tool::Down) return;
	
	graphics_view * gv = dynamic_cast<graphics_view *>(oglv);
	if (gv != NULL) gv->cam->RenderWindow(gv, camera::Measure, x, y);
}

void measure_tool::MotionEvent(ogl_view *, i32s, i32s)
{
}

/*################################################################################################*/

// eof
