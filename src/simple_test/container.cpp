// GHEMICAL.CPP

// Copyright (C) 2003 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "container.h"

/*################################################################################################*/

BonoboControlFrame  *ctrl_frame;
BonoboUIComponent   *ui_comp;

/* vbox */
GtkWidget           *box;
GtkWidget           *ctrl_widget;

char * image_file;

static Bonobo_Control
instantiate_control()
{
	Bonobo_Control control;
	Bonobo_PersistFile pfile;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	/* get control component */
//	control = bonobo_get_object ("OAFIID:gchem3d_control", "Bonobo/Control", &ev);
//	control = bonobo_get_object ("OAFIID:gchempaint_control", "Bonobo/Control", &ev);
	control = bonobo_get_object ("OAFIID:GNOME_Ghemical_Control", "Bonobo/Control", &ev);
	if (BONOBO_EX (&ev) || (control == CORBA_OBJECT_NIL))
        exit(1);
	
	/* get PersistFile interface */
//	pfile = Bonobo_Unknown_queryInterface (control, "IDL:Bonobo/PersistFile:1.0", &ev);
//	if (BONOBO_EX (&ev) || (pfile == CORBA_OBJECT_NIL))
//	exit(1);
	
	/* load the file */
//	Bonobo_PersistFile_load (pfile, image_file, &ev);
//	bonobo_object_release_unref (pfile, NULL);
	
	return control;
}

static void
verb_DoNothing (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
}

static BonoboUIVerb app_verbs[] = {
	BONOBO_UI_VERB ("FileNewWindow", verb_DoNothing),
	BONOBO_UI_VERB ("FileOpen",      verb_DoNothing),
	BONOBO_UI_VERB ("FileCloseWindow", verb_DoNothing),
	BONOBO_UI_VERB ("FileExit",      verb_DoNothing),
	BONOBO_UI_VERB ("Preferences",   verb_DoNothing),
	BONOBO_UI_VERB ("HelpAbout",     verb_DoNothing),
	BONOBO_UI_VERB ("Help",          verb_DoNothing),
	BONOBO_UI_VERB ("DnDNewWindow",  verb_DoNothing),
	BONOBO_UI_VERB ("DnDSameWindow", verb_DoNothing),
	BONOBO_UI_VERB ("DnDCancel",     verb_DoNothing),
	BONOBO_UI_VERB_END
};

static void 
add_control_to_ui (BonoboWindow *window, Bonobo_Control control)
{
	CORBA_Environment ev;
	Bonobo_PropertyControl prop_control;
	BonoboUIContainer *ui_container;
	char *curdir;
	
	g_return_if_fail (window != NULL);
	g_return_if_fail (BONOBO_IS_WINDOW (window));
	
	CORBA_exception_init (&ev);

	ui_container = bonobo_window_get_ui_container (BONOBO_WINDOW (window));
	ctrl_frame = bonobo_control_frame_new (BONOBO_OBJREF (ui_container));

	/* bind and view new control widget */
	bonobo_control_frame_bind_to_control (ctrl_frame, control, &ev);
	bonobo_control_frame_control_activate (ctrl_frame);
	if (control != CORBA_OBJECT_NIL && ctrl_widget == NULL) {
		ctrl_widget = bonobo_control_frame_get_widget (ctrl_frame);
		if (!ctrl_widget)
			g_assert_not_reached ();

        bonobo_window_set_contents (BONOBO_WINDOW(window), GTK_WIDGET(ctrl_widget));
		gtk_widget_show (ctrl_widget);
	}

	ui_comp = bonobo_ui_component_new ("eog");
	bonobo_ui_component_set_container (ui_comp, BONOBO_OBJREF (ui_container), NULL);

    curdir = (char *)getcwd(NULL, 0);
    printf("curdir = %s\n", curdir);
	bonobo_ui_util_set_ui (ui_comp, curdir, "container-ui.xml", "Container", NULL);
    free(curdir);
    bonobo_ui_component_add_verb_list_with_data (ui_comp, app_verbs, window);

	/* update sensitivity of the properties menu item */
	prop_control = Bonobo_Unknown_queryInterface (control, 
						      "IDL:Bonobo/PropertyControl:1.0", &ev);
	bonobo_ui_component_set_prop (ui_comp,
				      "/commands/Preferences",
				      "sensitive",
				      prop_control == CORBA_OBJECT_NIL ? "0" : "1",
				      &ev);
	
	bonobo_object_release_unref (prop_control, &ev);

	/* enable view menu */
	/* FIXME: We should check if the component adds anything to 
	 *        the menu, so that we don't view an empty menu.
	 */

	CORBA_exception_free (&ev);

	/* retrieve control properties and install listeners */
	//check_for_control_properties (window);
}

static void
window_destroyed (GtkWindow *window, char * data)
{
    Bonobo_Control control;
    bonobo_control_frame_control_deactivate(ctrl_frame);
    control = bonobo_control_frame_get_control(ctrl_frame);
    bonobo_object_release_unref( control, NULL );
    bonobo_main_quit();
}

int main(int argc, char * argv[] ) 
{
    BonoboWidget * bw;
    gchar filename[100];
    BonoboWindow      *window;
	BonoboUIEngine    *engine;
	BonoboUIContainer *container;
    Bonobo_Control    control;

    if( argc <= 1 ) { 
/*        fprintf(stderr, "%s: not enough args\n", argv[0] );
        fprintf(stderr, "Usage: %s <image file>\n", argv[0]);
        exit(1);	*/
    }
    else image_file = argv[1];

    bonobo_ui_init ("container", "1.0", &argc, argv);

	if(gnome_vfs_init () == FALSE)
		g_error (_("Could not initialize GnomeVFS!\n"));

	window = BONOBO_WINDOW ( bonobo_window_new ("Window", "ghemical (bonobo container)"));

    // instantiate a control
    control = instantiate_control();
    
    // put it into our window
    add_control_to_ui (window, control);

    g_signal_connect (window, "destroy",
          G_CALLBACK (window_destroyed),
          &window);


    gtk_widget_show_all( GTK_WIDGET( window ));

    bonobo_main();

    return 0;
}

/*################################################################################################*/

// eof
