// PROGRESS_DIALOG.CPP

// Copyright (C) 2005 Tommi Hassinen.

// This program is free software; you can redistribute it and/or modify it
// under the terms of the license (GNU GPL) which comes with this package.

/*################################################################################################*/

#include "progress_dialog.h"

#include <gtk/gtk.h>

#include <iostream>
#include <strstream>
using namespace std;

/*################################################################################################*/

progress_dialog::progress_dialog(const char * jobname, bool show_pbar, int graphs_n, int graphs_sz)
: glade_dialog("glade/progress_dialog.glade")
{
	dialog = glade_xml_get_widget(xml, "progress_dialog");
	if (dialog == NULL) { cout << "progress_dialog : glade_xml_get_widget() failed!!!" << endl; return; }
	
	cancel = false;
	
	g_n = graphs_n;
	g_sz = graphs_sz;
	g_fill = 0;
	
	g_data = NULL;
	if (g_n > 0 && g_sz > 0)
	{
		g_data = new double[g_n * g_sz];
	}
	
	// initialize the widgets...
	
	entry_job = glade_xml_get_widget(xml, "entry_job");
	drawingarea_job = glade_xml_get_widget(xml, "drawingarea_job");
	progressbar_job = glade_xml_get_widget(xml, "progressbar_job");
	
	gtk_entry_set_text(GTK_ENTRY(entry_job), jobname);
	
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressbar_job), "Progress");
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar_job), 0.0);
	
	// connect the handlers...
	
	glade_xml_signal_connect_data(xml, "on_dialog_destroy", (GtkSignalFunc) handler_Destroy, (gpointer) this);
	glade_xml_signal_connect_data(xml, "on_button_cancel_clicked", (GtkSignalFunc) handler_ButtonCancel, (gpointer) this);
	
	if (graphs_n > 0)
	{
		gtk_widget_set_size_request(drawingarea_job, 100, 100);
		g_signal_connect(G_OBJECT(drawingarea_job), "expose_event", G_CALLBACK(handler_ExposeEvent), this);
		
		gtk_widget_show(drawingarea_job);
	}
	else
	{
		gtk_widget_hide(drawingarea_job);
	}
	
	if (show_pbar)
	{
		gtk_widget_show(progressbar_job);
	}
	else
	{
		gtk_widget_hide(progressbar_job);
	}
	
	gtk_widget_show(dialog);	// MODELESS
}

progress_dialog::~progress_dialog(void)
{
	if (g_data != NULL)
	{
		delete[] g_data;
		g_data = NULL;
	}
}

void progress_dialog::handler_Destroy(GtkWidget *, gpointer data)
{
	progress_dialog * ref = (progress_dialog *) data;
	//cout << "handler_Destroy() : ref = " << ref << endl;
}

void progress_dialog::handler_ButtonCancel(GtkWidget *, gpointer data)
{
	progress_dialog * ref = (progress_dialog *) data;
	//cout << "handler_ButtonCancel() : ref = " << ref << endl;
	
// just send a message that we want cancel the current operation.
// the dialog is closed elsewhere (automatically if not cancelled).
	
	ref->cancel = true;
}

gboolean progress_dialog::handler_ExposeEvent(GtkWidget * widget, GdkEventExpose * event, gpointer data)
{
	progress_dialog * ref = (progress_dialog *) data;
	//cout << "handler_ExposeEvent() : ref = " << ref << endl;
	
	// update the graphs...
	// ^^^^^^^^^^^^^^^^^^^^
	// what about thread safety issues here?
	// it appears that the gdk_threads mechanism is enough...
	
	ref->da_w = widget->allocation.width;
	ref->da_h = widget->allocation.height;
	
// there is no need to blank the drawingarea???
// there is no need to blank the drawingarea???
// there is no need to blank the drawingarea???
	
	const double dx = ref->da_w / ref->g_sz;
	
	int sz = ref->g_fill;
	if (sz > ref->g_sz) sz = ref->g_sz;
	
	for (int i = 0;i < ref->g_n;i++)
	{
		// find the min and max values.
		
		double min = ref->g_data[i * ref->g_sz + 0];
		double max = min;
		
		for (int f = 1;f < sz;f++)
		{
			double v = ref->g_data[i * ref->g_sz + f];
			
			if (v < min) min = v;
			if (v > max) max = v;
		}
		
		double deltay = max - min;
		if (deltay < 1.0)
		{
			deltay = 1.0;
			
			double midy = (min + max) * 0.5;
			min = midy - deltay * 0.5;
			max = midy + deltay * 0.5;
		}
		
		// draw the graph
		
		double xv = 0.0;
		
		int start = (ref->g_fill % ref->g_sz);
		if (ref->g_fill < ref->g_sz) start = 0;
		
		for (int f = 0;f < sz - 1;f++)
		{
			int index1 = ((start + f + 0) % ref->g_sz);
			int index2 = ((start + f + 1) % ref->g_sz);
			
			double v1 = ref->g_data[i * ref->g_sz + index1];
			double v2 = ref->g_data[i * ref->g_sz + index2];
			
			double yv1 = ref->da_h * (1.0 - (v1 - min) / deltay);
			double yv2 = ref->da_h * (1.0 - (v2 - min) / deltay);
			
			gdk_draw_line(widget->window, widget->style->fg_gc[GTK_WIDGET_STATE(widget)], (int) xv, (int) yv1, (int) (xv + dx), (int) yv2);
			
			xv += dx;
		}
	}
	
	return TRUE;
}

/*################################################################################################*/

// eof
