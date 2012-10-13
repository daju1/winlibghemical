// Win32_PROJECT_VIEW.CPP

// Copyright (C) 2005 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/
#include "stdafx.h"

#include "win32_project_view.h"

#include "win32_graphics_view.h"

//#include <gtk/gtk.h>

#include <strstream>
#include <algorithm>
using namespace std;

/*################################################################################################*/

// the views-menu...
// ^^^^^^^^^^^^^^^^^
#if 0
GtkActionEntry win_project_view::views_entries[] =
{
	{ "views_SetCurrent", NULL, "_Set to Current Object", "<control>S", "Set this object to Current Object", (GCallback) win_project_view::views_SetCurrent },
	{ "views_Delete", NULL, "_Delete Object/View", "<control>D", "Delete this object or view", (GCallback) win_project_view::views_Delete },
};

const char * win_project_view::views_ui_description =
"<ui>"
"  <popup name='gpvViewsMenu'>"
"    <menuitem action='views_SetCurrent'/>"
"    <separator/>"
"    <menuitem action='views_Delete'/>"
"  </popup>"
"</ui>";

// the objects-menu...
// ^^^^^^^^^^^^^^^^^^^

GtkActionEntry win_project_view::objects_entries[] =
{
	{ "objects_SetCurrent", NULL, "_Set to Current Object", "<control>S", "Set this object to Current Object", (GCallback) win_project_view::objects_SetCurrent },
	{ "objects_Delete", NULL, "_Delete Object", "<control>D", "Delete this object", (GCallback) win_project_view::objects_Delete },
};

const char * win_project_view::objects_ui_description =
"<ui>"
"  <popup name='gpvObjectsMenu'>"
"    <menuitem action='objects_SetCurrent'/>"
"    <separator/>"
"    <menuitem action='objects_Delete'/>"
"  </popup>"
"</ui>";

// the chains-menu...
// ^^^^^^^^^^^^^^^^^^

GtkActionEntry win_project_view::chains_entries[] =
{
	{ "chains_UpdateView", NULL, "_Update View", "<control>U", "Rebuild the chains info for this view", (GCallback) win_project_view::chains_UpdateView },
	{ "chains_SelectItem", NULL, "_Select Item", "<control>S", "Select/unselect this chain/residue", (GCallback) win_project_view::chains_SelectItem },
};

const char * win_project_view::chains_ui_description =
"<ui>"
"  <popup name='gpvChainsMenu'>"
"    <menuitem action='chains_UpdateView'/>"
"    <separator/>"
"    <menuitem action='chains_SelectItem'/>"
"  </popup>"
"</ui>";

// the atoms-menu...
// ^^^^^^^^^^^^^^^^^

GtkActionEntry win_project_view::atoms_entries[] =
{
	{ "atoms_SelectAtom", NULL, "_Select Atom", "<control>S", "Select/unselect this atom", (GCallback) win_project_view::atoms_SelectAtom },
};

const char * win_project_view::atoms_ui_description =
"<ui>"
"  <popup name='gpvAtomsMenu'>"
"    <menuitem action='atoms_SelectAtom'/>"
"  </popup>"
"</ui>";

// the bonds-menu...
// ^^^^^^^^^^^^^^^^^

GtkActionEntry win_project_view::bonds_entries[] =
{
	{ "bonds_SelectBond", NULL, "_Select Bond", "<control>S", "Select/unselect this bond", (GCallback) win_project_view::bonds_SelectBond },
};

const char * win_project_view::bonds_ui_description =
"<ui>"
"  <popup name='gpvBondsMenu'>"
"    <menuitem action='bonds_SelectBond'/>"
"  </popup>"
"</ui>";
#endif
// other stuff...
// ^^^^^^^^^^^^^^

win_project_view * win_project_view::instance = NULL;

win_project_view::win_project_view(win_project * p1) : win_view(p1), project_view()
{
	if (instance == NULL) instance = this;
	else { cout << "win_project_view::instance is not NULL!" << endl; exit(EXIT_FAILURE); }
#if 0
	win_label_set_text(GTK_LABEL(label_widget), "project view");
	
	view_widget = win_notebook_new();
	win_widget_set_size_request(view_widget, 640, 400);	// minimum size...
	
	GtkCellRenderer * renderer;
	GtkTreeViewColumn * column;
	
	GtkActionGroup * action_group = NULL;
	GError * error = NULL;
	
	// "views"-page
	
	views_store = win_tree_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	views_widget = win_tree_view_new_with_model(win_TREE_MODEL(views_store));
	win_tree_view_set_rules_hint(win_TREE_VIEW(views_widget), TRUE);	// optional : draw the stripes to background.
	
	renderer = win_cell_renderer_text_new();	// ??? (string)
	column = win_tree_view_column_new_with_attributes("Object", renderer, "text", 0, NULL);
	win_tree_view_append_column(win_TREE_VIEW(views_widget), column);
	
	renderer = win_cell_renderer_text_new();	// ??? (string)
	column = win_tree_view_column_new_with_attributes("Sub-Objects", renderer, "text", 1, NULL);
	win_tree_view_append_column(win_TREE_VIEW(views_widget), column);
	
	views_label = win_label_new("Views");
	views_sw = win_scrolled_window_new (NULL, NULL);
	win_scrolled_window_set_shadow_type(win_SCROLLED_WINDOW(views_sw), win_SHADOW_ETCHED_IN);	// optional : ???
	win_scrolled_window_set_policy(win_SCROLLED_WINDOW(views_sw), win_POLICY_AUTOMATIC, win_POLICY_AUTOMATIC);
	win_container_add(win_CONTAINER(views_sw), views_widget);
	win_notebook_append_page(win_NOTEBOOK(view_widget), views_sw, views_label);
action_group = win_action_group_new("gpvViewsActions");
win_action_group_add_actions(action_group, views_entries, G_N_ELEMENTS(views_entries), win_WIDGET(view_widget));
win_ui_manager_insert_action_group(win_app::GetUIManager(), action_group, 0);
error = NULL;
if (!win_ui_manager_add_ui_from_string(win_app::GetUIManager(), views_ui_description, -1, & error))
{
	g_message("Building gpv Views menu failed : %s", error->message);
	g_error_free(error); exit(EXIT_FAILURE);
}
views_menu = win_ui_manager_get_widget(win_app::GetUIManager(), "/gpvViewsMenu");
g_signal_connect_swapped(win_OBJECT(views_widget), "button_press_event", G_CALLBACK(ViewsPopupHandler), win_WIDGET(views_widget));
	win_widget_show(views_widget);
	win_widget_show(views_label);
	win_widget_show(views_sw);
	
	// "objects"-page
	
	objects_store = win_list_store_new (1, G_TYPE_STRING);
	objects_widget = win_tree_view_new_with_model(win_TREE_MODEL(objects_store));
	win_tree_view_set_rules_hint(win_TREE_VIEW(objects_widget), TRUE);	// optional : draw the stripes to background.
	
	renderer = win_cell_renderer_text_new();	// object (string)
	column = win_tree_view_column_new_with_attributes("Object", renderer, "text", 0, NULL);
	win_tree_view_append_column(win_TREE_VIEW(objects_widget), column);
	
	objects_label = win_label_new("Objects");
	objects_sw = win_scrolled_window_new (NULL, NULL);
	win_scrolled_window_set_shadow_type(win_SCROLLED_WINDOW(objects_sw), win_SHADOW_ETCHED_IN);	// optional : ???
	win_scrolled_window_set_policy(win_SCROLLED_WINDOW(objects_sw), win_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(objects_sw), objects_widget);
	gtk_notebook_append_page(GTK_NOTEBOOK(view_widget), objects_sw, objects_label);
action_group = gtk_action_group_new("gpvObjectsActions");
gtk_action_group_add_actions(action_group, objects_entries, G_N_ELEMENTS(objects_entries), GTK_WIDGET(view_widget));
gtk_ui_manager_insert_action_group(gtk_app::GetUIManager(), action_group, 0);
error = NULL;
if (!gtk_ui_manager_add_ui_from_string(gtk_app::GetUIManager(), objects_ui_description, -1, & error))
{
	g_message("Building gpv Objects menu failed : %s", error->message);
	g_error_free(error); exit(EXIT_FAILURE);
}
objects_menu = gtk_ui_manager_get_widget(gtk_app::GetUIManager(), "/gpvObjectsMenu");
g_signal_connect_swapped(GTK_OBJECT(objects_widget), "button_press_event", G_CALLBACK(ObjectsPopupHandler), GTK_WIDGET(objects_widget));
	gtk_widget_show(objects_widget);
	gtk_widget_show(objects_label);
	gtk_widget_show(objects_sw);
	
	// "chains"-page
	
	chains_store = gtk_tree_store_new (5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	chains_widget = gtk_tree_view_new_with_model(GTK_TREE_MODEL(chains_store));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(chains_widget), TRUE);	// optional : draw the stripes to background.
	
	renderer = gtk_cell_renderer_text_new();	// chain_info (string)
	column = gtk_tree_view_column_new_with_attributes("Chain Description", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(chains_widget), column);
	
	renderer = gtk_cell_renderer_text_new();	// res_num (string)
	column = gtk_tree_view_column_new_with_attributes("Residue Number", renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(chains_widget), column);
	
	renderer = gtk_cell_renderer_text_new();	// res_id (string)
	column = gtk_tree_view_column_new_with_attributes("Residue ID", renderer, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(chains_widget), column);
	
	renderer = gtk_cell_renderer_text_new();	// res_state1 (string)
	column = gtk_tree_view_column_new_with_attributes("Sec-Str State", renderer, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(chains_widget), column);
	
	renderer = gtk_cell_renderer_text_new();	// res_state2 (string)
	column = gtk_tree_view_column_new_with_attributes("Protonation State", renderer, "text", 4, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(chains_widget), column);
	
	chains_label = gtk_label_new("Chains");
	chains_sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(chains_sw), GTK_SHADOW_ETCHED_IN);	// optional : ???
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(chains_sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(chains_sw), chains_widget);
	gtk_notebook_append_page(GTK_NOTEBOOK(view_widget), chains_sw, chains_label);
action_group = gtk_action_group_new("gpvChainsActions");
gtk_action_group_add_actions(action_group, chains_entries, G_N_ELEMENTS(chains_entries), GTK_WIDGET(view_widget));
gtk_ui_manager_insert_action_group(gtk_app::GetUIManager(), action_group, 0);
error = NULL;
if (!gtk_ui_manager_add_ui_from_string(gtk_app::GetUIManager(), chains_ui_description, -1, & error))
{
	g_message("Building gpv Chains menu failed : %s", error->message);
	g_error_free(error); exit(EXIT_FAILURE);
}
chains_menu = gtk_ui_manager_get_widget(gtk_app::GetUIManager(), "/gpvChainsMenu");
g_signal_connect_swapped(GTK_OBJECT(chains_widget), "button_press_event", G_CALLBACK(ChainsPopupHandler), GTK_WIDGET(chains_widget));
	gtk_widget_show(chains_widget);
	gtk_widget_show(chains_label);
	gtk_widget_show(chains_sw);
	
	// "atoms"-page
	
	atoms_store = gtk_list_store_new (2, G_TYPE_UINT, G_TYPE_STRING);
	atoms_widget = gtk_tree_view_new_with_model(GTK_TREE_MODEL(atoms_store));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(atoms_widget), TRUE);	// optional : draw the stripes to background.
	
	renderer = gtk_cell_renderer_text_new();	// index1
	column = gtk_tree_view_column_new_with_attributes("Atom Index", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(atoms_widget), column);
	
	renderer = gtk_cell_renderer_text_new();	// element
	column = gtk_tree_view_column_new_with_attributes("Element", renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(atoms_widget), column);
	
	atoms_label = gtk_label_new("Atoms");
	atoms_sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(atoms_sw), GTK_SHADOW_ETCHED_IN);	// optional : ???
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(atoms_sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(atoms_sw), atoms_widget);
	gtk_notebook_append_page(GTK_NOTEBOOK(view_widget), atoms_sw, atoms_label);
action_group = gtk_action_group_new("gpvAtomsActions");
gtk_action_group_add_actions(action_group, atoms_entries, G_N_ELEMENTS(atoms_entries), GTK_WIDGET(view_widget));
gtk_ui_manager_insert_action_group(gtk_app::GetUIManager(), action_group, 0);
error = NULL;
if (!gtk_ui_manager_add_ui_from_string(gtk_app::GetUIManager(), atoms_ui_description, -1, & error))
{
	g_message("Building gpv Atoms menu failed : %s", error->message);
	g_error_free(error); exit(EXIT_FAILURE);
}
atoms_menu = gtk_ui_manager_get_widget(gtk_app::GetUIManager(), "/gpvAtomsMenu");
g_signal_connect_swapped(GTK_OBJECT(atoms_widget), "button_press_event", G_CALLBACK(AtomsPopupHandler), GTK_WIDGET(atoms_widget));
	gtk_widget_show(atoms_widget);
	gtk_widget_show(atoms_label);
	gtk_widget_show(atoms_sw);
	
	// "bonds"-page
	
	bonds_store = gtk_list_store_new (3, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_STRING);
	bonds_widget = gtk_tree_view_new_with_model(GTK_TREE_MODEL(bonds_store));
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(bonds_widget), TRUE);	// optional : draw the stripes to background.
	
	renderer = gtk_cell_renderer_text_new();	// index1
	column = gtk_tree_view_column_new_with_attributes("Atom Index #1", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(bonds_widget), column);
	
	renderer = gtk_cell_renderer_text_new();	// index2
	column = gtk_tree_view_column_new_with_attributes("Atom Index #2", renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(bonds_widget), column);
	
	renderer = gtk_cell_renderer_text_new();	// bondtype
	column = gtk_tree_view_column_new_with_attributes("BondType", renderer, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(bonds_widget), column);
	
	bonds_label = gtk_label_new("Bonds");
	bonds_sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(bonds_sw), GTK_SHADOW_ETCHED_IN);	// optional : ???
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(bonds_sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(bonds_sw), bonds_widget);
	gtk_notebook_append_page(GTK_NOTEBOOK(view_widget), bonds_sw, bonds_label);
action_group = gtk_action_group_new("gpvBondsActions");
gtk_action_group_add_actions(action_group, bonds_entries, G_N_ELEMENTS(bonds_entries), GTK_WIDGET(view_widget));
gtk_ui_manager_insert_action_group(gtk_app::GetUIManager(), action_group, 0);
error = NULL;
if (!gtk_ui_manager_add_ui_from_string(gtk_app::GetUIManager(), bonds_ui_description, -1, & error))
{
	g_message("Building gpv Bonds menu failed : %s", error->message);
	g_error_free(error); exit(EXIT_FAILURE);
}
bonds_menu = gtk_ui_manager_get_widget(gtk_app::GetUIManager(), "/gpvBondsMenu");
g_signal_connect_swapped(GTK_OBJECT(bonds_widget), "button_press_event", G_CALLBACK(BondsPopupHandler), GTK_WIDGET(bonds_widget));
	gtk_widget_show(bonds_widget);
	gtk_widget_show(bonds_label);
	gtk_widget_show(bonds_sw);
#endif
	
	// initialize...
	// initialize...
	// initialize...
	
	for (i32u n1 = 0;n1 < prj->light_vector.size();n1++)
	{
		LightAdded(prj->light_vector[n1]);
	}
	
	for (/*i32u*/ n1 = 0;n1 < prj->object_vector.size();n1++)
	{
		ObjectAdded(prj->object_vector[n1]);
	}
	
	// ready...
	// ready...
	// ready...
	
//	gtk_widget_show(GTK_WIDGET(view_widget));
}

win_project_view::~win_project_view(void)
{
	instance = NULL;
}

win_project_view * win_project_view::GetInstance(void)
{
	return instance;
}

void win_project_view::Update(bool directly)
{
	cout << "win_project_view::Update() called with param " << directly << endl;
// how to handle update requests!??!?!
// how to handle update requests!??!?!
// how to handle update requests!??!?!
}
void win_project_view::UpdateAllWindowTitles(void)
{
	list<gpv_views_record *>::iterator it = views_data.begin();
	while (it != views_data.end())
	{
		if ((* it)->obj2 != NULL)
		{
			char object_name[256]; ostrstream ons(object_name, sizeof(object_name));
			ons << "graphics_view #" << (* it)->obj2->my_gv_number << ends;
			
//			gtk_tree_store_set(views_store, & (* it)->iter, 0, object_name, 1, FALSE, -1);
		}
		
		it++;
	}
}

void win_project_view::CameraAdded(camera * p1)
{
//printf("win_project_view::CameraAdded(camera * p1)\n\n");

	gpv_views_record * v_rec = new gpv_views_record;
	v_rec->obj1 = p1; v_rec->obj2 = NULL; v_rec->owner = NULL;
//	gtk_tree_store_append(views_store, & v_rec->iter, NULL);
	
	views_data.push_back(v_rec);
	
	const char * object_name = p1->GetObjectName();
	
//	gtk_tree_store_set(views_store, & v_rec->iter, 0, object_name, 1, FALSE, -1);
}

void win_project_view::CameraRemoved(camera * p1)
{
	list<gpv_views_record *>::iterator it = views_data.begin();
	while (it != views_data.end()) { if ((* it)->obj1 == (dummy_object *) p1) break; else it++; }
	if (it == views_data.end()) { cout << "ERROR : win_project_view::CameraRemoved() failed." << endl; exit(EXIT_FAILURE); }
	
//	gtk_tree_store_remove(views_store, & (* it)->iter);
	
	delete (* it);
	views_data.erase(it);
}

void win_project_view::LightAdded(light * p1)
{
	bool is_local_light = (p1->owner != NULL);
	if (is_local_light)
	{
		list<gpv_views_record *>::iterator it = views_data.begin();
		while (it != views_data.end()) { if ((* it)->obj1 == (dummy_object *) p1->owner) break; else it++; }
		if (it == views_data.end()) { cout << "ERROR : win_project_view::LightAdded() failed." << endl; exit(EXIT_FAILURE); }
		
		gpv_views_record * v_rec = new gpv_views_record;
		v_rec->obj1 = p1; v_rec->obj2 = NULL; v_rec->owner = p1->owner;
//		gtk_tree_store_append(views_store, & v_rec->iter, & (* it)->iter);
		
		views_data.push_back(v_rec);
		
		const char * object_name = p1->GetObjectName();
		
//		gtk_tree_store_set(views_store, & v_rec->iter, 0, object_name, 1, FALSE, -1);
	}
	else
	{
		gpv_views_record * v_rec = new gpv_views_record;
		v_rec->obj1 = p1; v_rec->obj2 = NULL; v_rec->owner = NULL;
//		gtk_tree_store_append(views_store, & v_rec->iter, NULL);
		
		views_data.push_back(v_rec);
		
		const char * object_name = p1->GetObjectName();
		
//		gtk_tree_store_set(views_store, & v_rec->iter, 0, object_name, 1, FALSE, -1);
	}
}

void win_project_view::LightRemoved(light * p1)
{
	list<gpv_views_record *>::iterator it = views_data.begin();
	while (it != views_data.end()) { if ((* it)->obj1 == (dummy_object *) p1) break; else it++; }
	if (it == views_data.end()) { cout << "ERROR : win_project_view::LightRemoved() failed." << endl; exit(EXIT_FAILURE); }
	
//	gtk_tree_store_remove(views_store, & (* it)->iter);
	
	delete (* it);
	views_data.erase(it);
}

void win_project_view::GraphicsViewAdded(graphics_view * gv)
{
//printf("win_project_view::GraphicsViewAdded(graphics_view * gv)\n\n");

	list<gpv_views_record *>::iterator it = views_data.begin();
	while (it != views_data.end()) { if ((* it)->obj1 == (dummy_object *) gv->cam) break; else it++; }
	if (it == views_data.end()) { cout << "ERROR : win_project_view::GraphicsViewAdded() failed." << endl; exit(EXIT_FAILURE); }
	
	gpv_views_record * v_rec = new gpv_views_record;
	v_rec->obj1 = NULL; v_rec->obj2 = gv; v_rec->owner = gv->cam;
//	gtk_tree_store_append(views_store, & v_rec->iter, & (* it)->iter);
	
	views_data.push_back(v_rec);
	
	char object_name[256]; ostrstream ons(object_name, sizeof(object_name));
	ons << "graphics_view #" << gv->my_gv_number << ends;
	
//	gtk_tree_store_set(views_store, & v_rec->iter, 0, object_name, 1, FALSE, -1);
}

void win_project_view::GraphicsViewRemoved(graphics_view * gv)
{
	list<gpv_views_record *>::iterator it = views_data.begin();
	while (it != views_data.end()) { if ((* it)->obj2 == gv) break; else it++; }
	if (it == views_data.end()) { cout << "ERROR : win_project_view::GraphicsViewRemoved() failed." << endl; exit(EXIT_FAILURE); }
	
//	gtk_tree_store_remove(views_store, & (* it)->iter);
	
	delete (* it);
	views_data.erase(it);
}

void win_project_view::PlottingViewAdded(plotting_view *)
{
	cout << "called win_project_view::PlottingViewAdded()" << endl;
}

void win_project_view::PlottingViewRemoved(plotting_view *)
{
	cout << "called win_project_view::PlottingViewRemoved()" << endl;
}
/*
gint win_project_view::ViewsPopupHandler(GtkWidget * widget, GdkEvent * event)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		GdkEventButton * event_button = (GdkEventButton *) event;
		if (event_button->button == 3)
		{
			win_project_view * gpv = GetInstance(); GtkMenu * menu = GTK_MENU(gpv->views_menu);
			gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event_button->button, event_button->time);
			return TRUE;
		}
	}
	
	return FALSE;
}

void win_project_view::views_SetCurrent(GtkWidget * widget, gpointer data)
{
	win_project_view * gpv = GetInstance();
	
// assume that tsel is in mode GTK_SELECTION_SINGLE (as it seems to be); this is the simplest case...
// here the equivalence test of GtkTreeIter is a bit uncertain... need to test any other records???
	
	GtkTreeSelection * tsel = gtk_tree_view_get_selection(GTK_TREE_VIEW(gpv->views_widget));
	GtkTreeModel * tm = gtk_tree_view_get_model(GTK_TREE_VIEW(gpv->views_widget));
	GtkTreeIter iter;
	
	bool has_item = gtk_tree_selection_get_selected(tsel, & tm, & iter);
	if (has_item)
	{
		list<gpv_views_record *>::iterator it = gpv->views_data.begin();
		while (it != gpv->views_data.end()) { if ((* it)->iter.user_data == iter.user_data) break; else it++; }
		if (it == gpv->views_data.end()) { cout << "ERROR : win_project_view::ViewsEventHandler1() failed." << endl; exit(EXIT_FAILURE); }
		
		bool is_light = ((* it)->obj1 != NULL && dynamic_cast<light *>((* it)->obj1) != NULL);
		if (is_light)
		{
			gpv->prj->selected_object = (* it)->obj1;
			
			char message[256]; ostrstream mstr(message, sizeof(message));
			mstr << "Object " << gpv->prj->selected_object->GetObjectName() << " is set to current object." << endl << ends;
			gpv->prj->PrintToLog(message);
		}
		else gpv->prj->Message("Sorry, this is only for light objects!");
	}
}

void win_project_view::views_Delete(GtkWidget *, gpointer data)
{
	win_project_view * gpv = GetInstance();
	
// assume that tsel is in mode GTK_SELECTION_SINGLE (as it seems to be); this is the simplest case...
// here the equivalence test of GtkTreeIter is a bit uncertain... need to test any other records???
	
	GtkTreeSelection * tsel = gtk_tree_view_get_selection(GTK_TREE_VIEW(gpv->views_widget));
	GtkTreeModel * tm = gtk_tree_view_get_model(GTK_TREE_VIEW(gpv->views_widget));
	GtkTreeIter iter;
	
	bool has_item = gtk_tree_selection_get_selected(tsel, & tm, & iter);
	if (has_item)
	{
		list<gpv_views_record *>::iterator it = gpv->views_data.begin();
		while (it != gpv->views_data.end()) { if ((* it)->iter.user_data == iter.user_data) break; else it++; }
		if (it == gpv->views_data.end()) { cout << "ERROR : win_project_view::ViewsEventHandler2() failed." << endl; exit(EXIT_FAILURE); }
		
		bool is_light = ((* it)->obj1 != NULL && dynamic_cast<light *>((* it)->obj1) != NULL);
		bool is_gv = ((* it)->obj2 != NULL);
		
		if (is_light)
		{
			char message[256]; ostrstream mstr(message, sizeof(message));
			mstr << "Object " << gpv->prj->selected_object->GetObjectName() << " is deleted." << endl << ends;
			gpv->prj->PrintToLog(message);
			
			if (gpv->prj->selected_object == (* it)->obj1) gpv->prj->selected_object = NULL;
			gpv->prj->RemoveLight((* it)->obj1);
		}
		else if (is_gv)
		{
			// see gtk_graphics_view::popup_ViewsDeleteView() for more about this...
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			
			if (!gpv->prj->IsThisLastGraphicsView((* it)->obj2))
			{
				gtk_graphics_view * ggv = dynamic_cast<gtk_graphics_view *>((* it)->obj2);
				
				char message[256]; ostrstream mstr(message, sizeof(message));
				mstr << "View #" << ggv->my_gv_number << " is deleted." << endl << ends;
				gpv->prj->PrintToLog(message);
				
				if (ggv->detached != NULL)
				{
					gtk_widget_destroy(GTK_WIDGET(ggv->detached));
				}
				else
				{
					gint page = gtk_notebook_page_num(GTK_NOTEBOOK(gpv->prj->notebook_widget), ggv->view_widget);
					gtk_notebook_remove_page(GTK_NOTEBOOK(gpv->prj->notebook_widget), page);
				}
				
				gpv->prj->RemoveGraphicsView((* it)->obj2, false);
			}
		}
		else gpv->prj->Message("Sorry, this is only for graphics_view/light objects!");
	}
}
*/
void win_project_view::ObjectAdded(smart_object * p1)
{
	gpv_objects_record * o_rec = new gpv_objects_record;
//	o_rec->obj = p1; gtk_list_store_append(objects_store, & o_rec->iter);
	
	objects_data.push_back(o_rec);
	
	const char * object_name = p1->GetObjectName();
	
//	gtk_list_store_set(objects_store, & o_rec->iter, 0, object_name, -1);
}

void win_project_view::ObjectRemoved(smart_object * p1)
{
	list<gpv_objects_record *>::iterator it = objects_data.begin();
	while (it != objects_data.end()) { if ((* it)->obj == p1) break; else it++; }
	if (it == objects_data.end()) { cout << "ERROR : win_project_view::ObjectRemoved() failed." << endl; exit(EXIT_FAILURE); }
	
//	gtk_list_store_remove(objects_store, & (* it)->iter);
	
	delete (* it);
	objects_data.erase(it);
}
/*
gint win_project_view::ObjectsPopupHandler(GtkWidget * widget, GdkEvent * event)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		GdkEventButton * event_button = (GdkEventButton *) event;
		if (event_button->button == 3)
		{
			win_project_view * gpv = GetInstance(); GtkMenu * menu = GTK_MENU(gpv->objects_menu);
			gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event_button->button, event_button->time);
			return TRUE;
		}
	}
	
	return FALSE;
}

void win_project_view::objects_SetCurrent(GtkWidget *, gpointer data)
{
	win_project_view * gpv = GetInstance();
	
// assume that tsel is in mode GTK_SELECTION_SINGLE (as it seems to be); this is the simplest case...
// here the equivalence test of GtkTreeIter is a bit uncertain... need to test any other records???
	
	GtkTreeSelection * tsel = gtk_tree_view_get_selection(GTK_TREE_VIEW(gpv->objects_widget));
	GtkTreeModel * tm = gtk_tree_view_get_model(GTK_TREE_VIEW(gpv->objects_widget));
	GtkTreeIter iter;
	
	bool has_item = gtk_tree_selection_get_selected(tsel, & tm, & iter);
	if (has_item)
	{
		list<gpv_objects_record *>::iterator it = gpv->objects_data.begin();
		while (it != gpv->objects_data.end()) { if ((* it)->iter.user_data == iter.user_data) break; else it++; }
		if (it == gpv->objects_data.end()) { cout << "ERROR : win_project_view::ObjectsEventHandler1() failed." << endl; exit(EXIT_FAILURE); }
		
		gpv->prj->selected_object = (* it)->obj;
		char message[256]; ostrstream mstr(message, sizeof(message));
		mstr << "Object " << gpv->prj->selected_object->GetObjectName() << " is set to current object." << endl << ends;
		gpv->prj->PrintToLog(message);
	}
}

void win_project_view::objects_Delete(GtkWidget *, gpointer data)
{
	win_project_view * gpv = GetInstance();
	
// assume that tsel is in mode GTK_SELECTION_SINGLE (as it seems to be); this is the simplest case...
// here the equivalence test of GtkTreeIter is a bit uncertain... need to test any other records???
	
	GtkTreeSelection * tsel = gtk_tree_view_get_selection(GTK_TREE_VIEW(gpv->objects_widget));
	GtkTreeModel * tm = gtk_tree_view_get_model(GTK_TREE_VIEW(gpv->objects_widget));
	GtkTreeIter iter;
	
	bool has_item = gtk_tree_selection_get_selected(tsel, & tm, & iter);
	if (has_item)
	{
		list<gpv_objects_record *>::iterator it = gpv->objects_data.begin();
		while (it != gpv->objects_data.end()) { if ((* it)->iter.user_data == iter.user_data) break; else it++; }
		if (it == gpv->objects_data.end()) { cout << "ERROR : win_project_view::ObjectsEventHandler2() failed." << endl; exit(EXIT_FAILURE); }
		
		char message[256]; ostrstream mstr(message, sizeof(message));
		mstr << "Object " << (* it)->obj->GetObjectName() << " is deleted." << endl << ends;
		gpv->prj->PrintToLog(message);
		
		if (gpv->prj->selected_object == (* it)->obj) gpv->prj->selected_object = NULL;
		gpv->prj->RemoveObject((* it)->obj);
		
		gpv->prj->UpdateAllGraphicsViews();
	}
}
*/
void win_project_view::BuildChainsView(void)
{
	if (chains_data.size() != 0)
	{
		ClearChainsView();
	}
	
	if (prj->ref_civ != NULL)
	{
		vector<chn_info> & ci_vector = (* prj->ref_civ);
		
		for (i32u n1 = 0;n1 < ci_vector.size();n1++)
		{
			gpv_chains_record * c_rec1 = new gpv_chains_record;
//			c_rec1->c_r_ind = (n1 << 16) + 0xFFFF; gtk_tree_store_append(chains_store, & c_rec1->iter, NULL);
			
			chains_data.push_back(c_rec1);
			
			char chain_info[256];
			ostrstream cis(chain_info, sizeof(chain_info));
		//	if (ci_vector[n1].description...	// this not implemented at the moment...
			cis << "chain #" << n1 << " (";
			switch (ci_vector[n1].GetType())
			{
				case chn_info::amino_acid:	cis << "AA"; break;
				case chn_info::nucleic_acid:	cis << "NA"; break;
				default:			cis << "??";
			}
			cis << " chain)." << ends;
			
//			gtk_tree_store_set(chains_store, & c_rec1->iter, 0, chain_info, 1, FALSE, 2, FALSE, 3, FALSE, 4, FALSE, -1);
			
			const char * seq_buff = ci_vector[n1].GetSequence();
			for (i32s n2 = 0;n2 < ci_vector[n1].GetLength();n2++)
			{
				gpv_chains_record * c_rec2 = new gpv_chains_record;
				c_rec2->c_r_ind = (n1 << 16) + n2; 
				
				//gtk_tree_store_append(chains_store, & c_rec2->iter, & c_rec1->iter);
				
				chains_data.push_back(c_rec2);
				
				char res_num[16]; ostrstream rns(res_num, sizeof(res_num)); rns << n2 << ends;
				char res_id[8] = "?"; res_id[0] = seq_buff[n2];
				
//				gtk_tree_store_set(chains_store, & c_rec2->iter, 0, FALSE, 1, res_num, 2, res_id, 3, "???", 4, "???", -1);
			}
		}
	}
}

void win_project_view::ClearChainsView(void)
{
	list<gpv_chains_record *>::iterator it;
	
	it = chains_data.begin();		// first remove the residue records...
	while (it != chains_data.end())		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	{
		if (((* it)->c_r_ind & 0xFFFF) != 0xFFFF)
		{
//			gtk_tree_store_remove(chains_store, & (* it)->iter);
			
			delete (* it);
			chains_data.erase(it);
			
			it = chains_data.begin();
		}
		
		it++;
	}
	
	it = chains_data.begin();		// ...and then the chain records!
	while (it != chains_data.end())		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	{
//		gtk_tree_store_remove(chains_store, & (* it)->iter);
		
		delete (* it);
		chains_data.erase(it);
		
		it = chains_data.begin();
	}
}
/*
gint win_project_view::ChainsPopupHandler(GtkWidget * widget, GdkEvent * event)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		GdkEventButton * event_button = (GdkEventButton *) event;
		if (event_button->button == 3)
		{
			win_project_view * gpv = GetInstance(); GtkMenu * menu = GTK_MENU(gpv->chains_menu);
			gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event_button->button, event_button->time);
			return TRUE;
		}
	}
	
	return FALSE;
}

void win_project_view::chains_UpdateView(GtkWidget *, gpointer data)
{
	win_project_view * gpv = GetInstance();
	
	// update the data only if it's necessary ; there might be some extra info stored in the chains data!
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	
	if (gpv->prj->ref_civ == NULL)
	{
		gpv->prj->UpdateChains();
	}
}

void win_project_view::chains_SelectItem(GtkWidget *, gpointer data)
{
	win_project_view * gpv = GetInstance();
	
// assume that tsel is in mode GTK_SELECTION_SINGLE (as it seems to be); this is the simplest case...
// here the equivalence test of GtkTreeIter is a bit uncertain... need to test any other records???
	
	GtkTreeSelection * tsel = gtk_tree_view_get_selection(GTK_TREE_VIEW(gpv->chains_widget));
	GtkTreeModel * tm = gtk_tree_view_get_model(GTK_TREE_VIEW(gpv->chains_widget));
	GtkTreeIter iter;
	
	bool has_item = gtk_tree_selection_get_selected(tsel, & tm, & iter);
	if (has_item)
	{
		list<gpv_chains_record *>::iterator it = gpv->chains_data.begin();
		while (it != gpv->chains_data.end()) { if ((* it)->iter.user_data == iter.user_data) break; else it++; }
		if (it == gpv->chains_data.end()) { cout << "ERROR : win_project_view::ChainsEventHandler2() failed." << endl; exit(EXIT_FAILURE); }
		
		i32s c_ind = ((* it)->c_r_ind >> 16);
		i32s r_ind = ((* it)->c_r_ind & 0xFFFF);
		
		iter_al c_rng[2]; gpv->prj->GetRange(1, c_ind, c_rng);
		
		if (r_ind == 0xFFFF)	// select chain
		{
			for (iter_al iter = c_rng[0]; iter != c_rng[1];iter++)
			{
				(* iter).flags |= ATOMFLAG_SELECTED;
			}
		}
		else			// select residue
		{
			iter_al r_rng[2]; gpv->prj->GetRange(2, c_rng, r_ind, r_rng);
			
			for (iter_al iter = r_rng[0]; iter != r_rng[1];iter++)
			{
				(* iter).flags |= ATOMFLAG_SELECTED;
			}
		}
		
		gpv->prj->UpdateAllGraphicsViews();
	}
}
*/
void win_project_view::AtomAdded(atom * p1)
{
	gpv_atoms_record * a_rec = new gpv_atoms_record;
	a_rec->ref = p1; 
	
	//gtk_list_store_append(atoms_store, & a_rec->iter);
	
	atoms_data.push_back(a_rec);
	
	AtomUpdateItem(p1);
}

void win_project_view::AtomUpdateItem(atom * p1)
{
	list<gpv_atoms_record *>::iterator it = atoms_data.begin();
	while (it != atoms_data.end()) { if ((* it)->ref == p1) break; else it++; }
	if (it == atoms_data.end()) { cout << "ERROR : win_project_view::AtomUpdateItem() failed." << endl; exit(EXIT_FAILURE); }
	
	i32s atmi = p1->index;
	const char * ele = p1->el.GetSymbol();
	
//	gtk_list_store_set(atoms_store, & (* it)->iter, 0, atmi, 1, ele, -1);
}

void win_project_view::AtomRemoved(atom * p1)
{
	list<gpv_atoms_record *>::iterator it = atoms_data.begin();
	while (it != atoms_data.end()) { if ((* it)->ref == p1) break; else it++; }
	if (it == atoms_data.end()) { cout << "ERROR : win_project_view::AtomRemoved() failed." << endl; exit(EXIT_FAILURE); }
	
//	gtk_list_store_remove(atoms_store, & (* it)->iter);
	
	delete (* it);
	atoms_data.erase(it);
}
/*
gint win_project_view::AtomsPopupHandler(GtkWidget * widget, GdkEvent * event)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		GdkEventButton * event_button = (GdkEventButton *) event;
		if (event_button->button == 3)
		{
			win_project_view * gpv = GetInstance(); GtkMenu * menu = GTK_MENU(gpv->atoms_menu);
			gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event_button->button, event_button->time);
			return TRUE;
		}
	}
	
	return FALSE;
}

void win_project_view::atoms_SelectAtom(GtkWidget *, gpointer data)
{
	win_project_view * gpv = GetInstance();
	
// assume that tsel is in mode GTK_SELECTION_SINGLE (as it seems to be); this is the simplest case...
// here the equivalence test of GtkTreeIter is a bit uncertain... need to test any other records???
	
	GtkTreeSelection * tsel = gtk_tree_view_get_selection(GTK_TREE_VIEW(gpv->atoms_widget));
	GtkTreeModel * tm = gtk_tree_view_get_model(GTK_TREE_VIEW(gpv->atoms_widget));
	GtkTreeIter iter;
	
	bool has_item = gtk_tree_selection_get_selected(tsel, & tm, & iter);
	if (has_item)
	{
		list<gpv_atoms_record *>::iterator it = gpv->atoms_data.begin();
		while (it != gpv->atoms_data.end()) { if ((* it)->iter.user_data == iter.user_data) break; else it++; }
		if (it == gpv->atoms_data.end()) { cout << "ERROR : win_project_view::AtomsEventHandler1() failed." << endl; exit(EXIT_FAILURE); }
		
		(* it)->ref->flags ^= ATOMFLAG_SELECTED;
		gpv->prj->UpdateAllGraphicsViews();
	}
}
*/
void win_project_view::BondAdded(bond * p1)
{
	gpv_bonds_record * b_rec = new gpv_bonds_record;
	b_rec->ref = p1; 
	
	//gtk_list_store_append(bonds_store, & b_rec->iter);
	
	bonds_data.push_back(b_rec);
	
	BondUpdateItem(p1);
}

void win_project_view::BondUpdateItem(bond * p1)
{
	list<gpv_bonds_record *>::iterator it = bonds_data.begin();
	while (it != bonds_data.end()) { if ((* it)->ref == p1) break; else it++; }
	if (it == bonds_data.end()) { cout << "ERROR : win_project_view::BondUpdateItem() failed." << endl; exit(EXIT_FAILURE); }
	
	const char * bt_strings[4] = 
	{
		"conjugated",
		"single",
		"double",
		"triple"
	};
	
	i32s atmi1 = p1->atmr[0]->index;
	i32s atmi2 = p1->atmr[1]->index;
	const char * btype = bt_strings[p1->bt.GetValue()];
	
//	gtk_list_store_set(bonds_store, & (* it)->iter, 0, atmi1, 1, atmi2, 2, btype, -1);
}

void win_project_view::BondRemoved(bond * p1)
{
	list<gpv_bonds_record *>::iterator it = bonds_data.begin();
	while (it != bonds_data.end()) { if ((* it)->ref == p1) break; else it++; }
	if (it == bonds_data.end()) { cout << "ERROR : win_project_view::BondRemoved() failed." << endl; exit(EXIT_FAILURE); }
	
//	gtk_list_store_remove(bonds_store, & (* it)->iter);
	
	delete (* it);
	bonds_data.erase(it);
}
/*
gint win_project_view::BondsPopupHandler(GtkWidget * widget, GdkEvent * event)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		GdkEventButton * event_button = (GdkEventButton *) event;
		if (event_button->button == 3)
		{
			win_project_view * gpv = GetInstance(); GtkMenu * menu = GTK_MENU(gpv->bonds_menu);
			gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event_button->button, event_button->time);
			return TRUE;
		}
	}
	
	return FALSE;
}

void win_project_view::bonds_SelectBond(GtkWidget *, gpointer data)
{
	win_project_view * gpv = GetInstance();
	
// assume that tsel is in mode GTK_SELECTION_SINGLE (as it seems to be); this is the simplest case...
// here the equivalence test of GtkTreeIter is a bit uncertain... need to test any other records???
	
	GtkTreeSelection * tsel = gtk_tree_view_get_selection(GTK_TREE_VIEW(gpv->bonds_widget));
	GtkTreeModel * tm = gtk_tree_view_get_model(GTK_TREE_VIEW(gpv->bonds_widget));
	GtkTreeIter iter;
	
	bool has_item = gtk_tree_selection_get_selected(tsel, & tm, & iter);
	if (has_item)
	{
		list<gpv_bonds_record *>::iterator it = gpv->bonds_data.begin();
		while (it != gpv->bonds_data.end()) { if ((* it)->iter.user_data == iter.user_data) break; else it++; }
		if (it == gpv->bonds_data.end()) { cout << "ERROR : win_project_view::BondsEventHandler1() failed." << endl; exit(EXIT_FAILURE); }
		
		bool both_selected = (((* it)->ref->atmr[0]->flags & ATOMFLAG_SELECTED) && ((* it)->ref->atmr[1]->flags & ATOMFLAG_SELECTED));
		if (!both_selected)	// select...
		{
			(* it)->ref->atmr[0]->flags |= ATOMFLAG_SELECTED;
			(* it)->ref->atmr[1]->flags |= ATOMFLAG_SELECTED;
		}
		else			// un-select...
		{
			(* it)->ref->atmr[0]->flags &= (~ATOMFLAG_SELECTED);
			(* it)->ref->atmr[1]->flags &= (~ATOMFLAG_SELECTED);
		}
		
		gpv->prj->UpdateAllGraphicsViews();
	}
}

  */
/*################################################################################################*/

// eof
