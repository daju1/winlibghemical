// Win32_PROJECT_VIEW.H : write a short description here...

// Copyright (C) 2005 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#ifndef WIN_PROJECT_VIEW_H
#define WIN_PROJECT_VIEW_H

#include "appconfig.h"

struct gpv_views_record;
struct gpv_objects_record;
struct gpv_chains_record;
struct gpv_atoms_record;
struct gpv_bonds_record;

class gtk_project_view;

#include "win32_project.h"
#include "win32_views.h"

/*################################################################################################*/

struct gpv_views_record
{
	dummy_object * obj1;		///< for camera/light objects.
	graphics_view * obj2;		///< for graphics_view objects.
	
	camera * owner;
	
//	GtkTreeIter iter;
};

struct gpv_objects_record
{
	smart_object * obj;
//	GtkTreeIter iter;
};

struct gpv_chains_record
{
	i32s c_r_ind;
//	GtkTreeIter iter;
};

struct gpv_atoms_record
{
	atom * ref;
//	GtkTreeIter iter;
};

struct gpv_bonds_record
{
	bond * ref;
//	GtkTreeIter iter;
};

/**	A GTK project view implementation. Under construction, 
*/

class win_project_view : public win_view, public project_view
{
	protected:
/*	
	static GtkActionEntry views_entries[];
	static const char * views_ui_description;
	
	static GtkActionEntry objects_entries[];
	static const char * objects_ui_description;
	
	static GtkActionEntry chains_entries[];
	static const char * chains_ui_description;
	
	static GtkActionEntry atoms_entries[];
	static const char * atoms_ui_description;
	
	static GtkActionEntry bonds_entries[];
	static const char * bonds_ui_description;
*/
	static win_project_view * instance;		///< in an SDI GUI we only need a single view...
	
	list<gpv_views_record *> views_data;
/*	GtkTreeStore * views_store;
	GtkWidget * views_widget;
	GtkWidget * views_label;
	GtkWidget * views_menu;
	GtkWidget * views_sw;*/
	
	list<gpv_objects_record *> objects_data;
/*	GtkListStore * objects_store;
	GtkWidget * objects_widget;
	GtkWidget * objects_label;
	GtkWidget * objects_menu;
	GtkWidget * objects_sw;*/
	
	list<gpv_chains_record *> chains_data;
/*	GtkTreeStore * chains_store;
	GtkWidget * chains_widget;
	GtkWidget * chains_label;
	GtkWidget * chains_menu;
	GtkWidget * chains_sw;*/
	
	list<gpv_atoms_record *> atoms_data;
/*	GtkListStore * atoms_store;
	GtkWidget * atoms_widget;
	GtkWidget * atoms_label;
GtkWidget * atoms_menu;
	GtkWidget * atoms_sw;*/
	
	list<gpv_bonds_record *> bonds_data;
/*	GtkListStore * bonds_store;
	GtkWidget * bonds_widget;
	GtkWidget * bonds_label;
GtkWidget * bonds_menu;
	GtkWidget * bonds_sw;*/
	
	private:
	
	win_project_view(win_project *);
	
	friend class win_class_factory;
		
	public:
	
	~win_project_view(void);
	
	static win_project_view * GetInstance(void);
	
	void Update(bool);			// virtual
	void UpdateAllWindowTitles(void);	// virtual
	
	void CameraAdded(camera *);			// virtual
	void CameraRemoved(camera *);			// virtual
	void LightAdded(light *);			// virtual
	void LightRemoved(light *);			// virtual
	void GraphicsViewAdded(graphics_view *);	// virtual
	void GraphicsViewRemoved(graphics_view *);	// virtual
	void PlottingViewAdded(plotting_view *);	// virtual
	void PlottingViewRemoved(plotting_view *);	// virtual
//	static gint ViewsPopupHandler(GtkWidget *, GdkEvent *);
//	static void views_SetCurrent(GtkWidget *, gpointer);
//	static void views_Delete(GtkWidget *, gpointer);
	
	void ObjectAdded(smart_object *);		// virtual
	void ObjectRemoved(smart_object *);		// virtual
//	static gint ObjectsPopupHandler(GtkWidget *, GdkEvent *);
//	static void objects_SetCurrent(GtkWidget *, gpointer);
//	static void objects_Delete(GtkWidget *, gpointer);
	
	void BuildChainsView(void);			// virtual
	void ClearChainsView(void);			// virtual
//	static gint ChainsPopupHandler(GtkWidget *, GdkEvent *);
//	static void chains_UpdateView(GtkWidget *, gpointer);
//	static void chains_SelectItem(GtkWidget *, gpointer);
	
	void AtomAdded(atom *);				// virtual
	void AtomUpdateItem(atom *);			// virtual
	void AtomRemoved(atom *);			// virtual
//	static gint AtomsPopupHandler(GtkWidget *, GdkEvent *);
//	static void atoms_SelectAtom(GtkWidget *, gpointer);
	
	void BondAdded(bond *);				// virtual
	void BondUpdateItem(bond *);			// virtual
	void BondRemoved(bond *);			// virtual
//	static gint BondsPopupHandler(GtkWidget *, GdkEvent *);
//	static void bonds_SelectBond(GtkWidget *, gpointer);
};

/*################################################################################################*/

#endif	// GTK_PROJECT_VIEW_H

// eof
