/*
 * Copyright (C) 2001,2002 Red Hat, Inc.
 *
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ident "$Id$"
#include "../config.h"
#include <string.h>
#include <gtk/gtk.h>
#include <glib-object.h>
#include "vte.h"

static void
set_window_title(GtkWidget *widget, const char *title, gpointer win)
{
	GtkWindow *window;
	g_return_if_fail(VTE_TERMINAL(widget));
	g_return_if_fail(GTK_IS_WINDOW(win));
	g_return_if_fail(title != NULL);
	window = GTK_WINDOW(win);
	gtk_window_set_title(window, title);
}

static void
char_size_changed(GtkWidget *widget, guint width, guint height, gpointer win)
{
	VteTerminal *terminal;
	GtkWindow *window;
	GdkGeometry geometry;
	g_return_if_fail(GTK_IS_WINDOW(win));
	g_return_if_fail(VTE_IS_TERMINAL(widget));
	terminal = VTE_TERMINAL(widget);
	window = GTK_WINDOW(win);
	geometry.base_width = 0;
	geometry.base_height = 0;
	geometry.min_width = terminal->char_width;
	geometry.min_height = terminal->char_height;
	geometry.width_inc = terminal->char_width;
	geometry.height_inc = terminal->char_height;
	gtk_window_set_geometry_hints(window, widget, &geometry,
				      GDK_HINT_RESIZE_INC |
				      GDK_HINT_BASE_SIZE |
				      GDK_HINT_MIN_SIZE);
}

int
main(int argc, char **argv)
{
	GtkWidget *window, *hbox, *scrollbar, *widget;
	const char *message = "Launching interactive shell...\r\n";

	gtk_init(&argc, &argv);

	/* Create a window to hold the scrolling shell, and hook its
	 * delete event to the quit function.. */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "delete_event",
			 GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

	/* Create a box to hold everything. */
	hbox = gtk_hbox_new(0, FALSE);
	gtk_container_add(GTK_CONTAINER(window), hbox);

	/* Create the terminal widget and add it to the scrolling shell. */
	widget = vte_terminal_new();
	char_size_changed(widget,
			  (VTE_TERMINAL(widget)->char_width),
			  (VTE_TERMINAL(widget)->char_height),
			  window);
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);

	/* Connect to the "char_size_changed" signal to set geometry hints
	 * whenever the font used by the terminal is changed. */
	g_signal_connect_object(G_OBJECT(widget), "char_size_changed",
				G_CALLBACK(char_size_changed), window, 0);

	/* Connect to the "set_window_title" signal to set the main window's
	 * title. */
	g_signal_connect(G_OBJECT(widget), "set_window_title",
			 G_CALLBACK(set_window_title), window);

	/* Connect to the "eof" signal to quit when the session ends. */
	g_signal_connect(G_OBJECT(widget), "eof",
			 G_CALLBACK(gtk_main_quit), NULL);

	/* Create the scrollbar for the widget. */
	scrollbar = gtk_vscrollbar_new((VTE_TERMINAL(widget))->adjustment);
	gtk_box_pack_start(GTK_BOX(hbox), scrollbar, FALSE, FALSE, 0);

	/* Launch a shell. */
#ifdef VTE_DEBUG
	vte_terminal_feed(VTE_TERMINAL(widget), message, strlen(message));
#endif
	vte_terminal_fork_command(VTE_TERMINAL(widget), NULL, NULL);

	/* Go for it! */
	gtk_widget_show_all(window);
	gtk_main();

	return 0;
}
