/*
 * Developed by Claudio André <claudioandre.br at gmail.com> in 2016
 *
 * More information at https://github.com/claudioandre
 *
 * Copyright (c) 2015-2016 Claudio André <claudioandre.br at gmail.com>
 * This program comes with ABSOLUTELY NO WARRANTY; express or implied.
 *
 * This is free software, and you are welcome to redistribute it
 * under certain conditions; as expressed here
 * http://www.gnu.org/licenses/gpl-2.0.html
 */

/* *****
 *  $ gcc -Wall -L /usr/lib -I /usr/include/webkitgtk-1.0/ -I /usr/include/libsoup-2.4 -g leve.c
 *       -o leve `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0 webkit-1.0`
 *
 *   sudo apt-get install libwebkit-dev
 * *****/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <webkit/webkit.h>

static void destroy_window(GtkWidget *widget, GtkWidget *window);
static GdkPixbuf *create_pixbuf(const gchar *filename);
static void message_box(const char *filename);
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event,
                             gpointer user_data);
static char *get_uri(char *orig);
static char *str_append_fast(char *dest, char *string);
static int copy_file(const char *orig, const char *dest);
static int file_exists(const char *filename);
static char *find_icon(char *arg_v);

GtkWidget *window;
char *cur_filename = "";


int main(int argc, char *argv[])
{
	// The web page to be loaded into the browser
	char *uri, *name;
	GtkWidget *main_window;
	GtkWidget *scroll_window;
	WebKitWebView *webView;
	GdkPixbuf *icon;

	name = find_icon(argv[0]);

	// Initialize GTK+
	gtk_init(&argc, &argv);

	// Create the main window
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	icon = create_pixbuf(name);

	if (icon)
		gtk_window_set_icon(GTK_WINDOW(main_window), icon);
	gtk_window_set_title(GTK_WINDOW(main_window),
	                     "Recommended Configurations");
	gtk_window_set_default_size(GTK_WINDOW(main_window), 1050, 600);
	gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
	gtk_window_maximize(GTK_WINDOW(main_window));

	// Create a scrollable area
	scroll_window = gtk_scrolled_window_new(NULL, NULL);

	gtk_container_add(GTK_CONTAINER(main_window), scroll_window);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window),
	                               GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	// Create the browser instance
	webView = WEBKIT_WEB_VIEW(webkit_web_view_new());

	gtk_container_add(GTK_CONTAINER(scroll_window), GTK_WIDGET(webView));

	// Set up callbacks
	g_signal_connect(main_window, "destroy", G_CALLBACK(destroy_window), NULL);
	g_signal_connect(main_window, "key_press_event", G_CALLBACK(on_key_press),
	                 NULL);

	// Environment is ok
	if (argc == 1)
		cur_filename = "www.webkitgtk.org";
	else
		cur_filename = argv[1];
	
        uri = get_uri(cur_filename);
	window = main_window;

	// Load the web page into the browser instance
	webkit_web_view_load_uri(webView, uri);
	gtk_widget_grab_focus(GTK_WIDGET(webView));
	gtk_widget_show_all(main_window);

	if (icon)
		g_object_unref(icon);

	// Run the main GTK+ event loop
	gtk_main();

	// Release resources
	free(uri);
	free(name);

	return 0;
}


static void destroy_window(GtkWidget *widget, GtkWidget *window)
{
	gtk_main_quit();
}


static GdkPixbuf *create_pixbuf(const gchar *filename)
{

	GdkPixbuf *pixbuf;

	pixbuf = gdk_pixbuf_new_from_file(filename, NULL);

	return pixbuf;
}


static void message_box(const char *filename)
{
	GtkWidget *dialog;
	GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

	dialog =
	    gtk_message_dialog_new(GTK_WINDOW(window), flags, GTK_MESSAGE_ERROR,
	                           GTK_BUTTONS_CLOSE, "Error saving '%s'", filename);

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}


static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event,
                             gpointer user_data)
{
	if (event->keyval == GDK_s && event->state & GDK_CONTROL_MASK) {
		GtkWidget *dialog;
		GtkFileChooser *chooser;
		GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
		gint res;

		dialog = gtk_file_chooser_dialog_new("Save File",
		                                     GTK_WINDOW(window), action, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		                                     GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
		chooser = GTK_FILE_CHOOSER(dialog);

		gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
		gtk_file_chooser_set_filename(chooser, cur_filename);

		res = gtk_dialog_run(GTK_DIALOG(dialog));

		if (res == GTK_RESPONSE_ACCEPT) {
			char *filename;

			filename = gtk_file_chooser_get_filename(chooser);

			if (!copy_file(cur_filename, filename))
				message_box(filename);
			g_free(filename);
		}

		gtk_widget_destroy(dialog);
	}

	return FALSE;
}


/*
 * Convert a file path to a file URI:
 * - return a newly allocated string that is the original transformed.
 */
static char *get_uri(char *orig)
{
	char *new = malloc(strlen(orig) * 3 + 8);
	char *s = orig;
	char *d = new;

	if (file_exists(orig)) {
		d = str_append_fast(d, "file://");

		while (*s) {
			if (*s == ' ') {
				d = str_append_fast(d, "%20");
				s++;
			} else
				*d++ = *s++;
		}

	} else {
		if (!strstr(orig, "http://"))
			d = str_append_fast(d, "http://");

		while (*s)
			*d++ = *s++;
	}
	*d = 0;

	return new;
}


/*
 * Append one string into another:
 * - return the new dest append position.
 */
static char *str_append_fast(char *dest, char *string)
{
	char *s = string;
	char *d = dest;

	while (*s)
		*d++ = *s++;
	*d = 0;

	return d;
}


static int copy_file(const char *orig, const char *dest)
{
	FILE *source = fopen(orig, "r");
	FILE *target = fopen(dest, "w");
	int ch;

	if (source == NULL || target == NULL)
		return 0;

	while ((ch = fgetc(source)) != EOF)
		fputc(ch, target);

	fclose(source);
	fclose(target);

	return 1;
}


/*
 * Check if a file exists:
 * - return true if exists, otherwise false.
 */
static int file_exists(const char *filename)
{
	FILE *file;

	if ((file = fopen(filename, "r"))) {
		fclose(file);
		return 1;
	}
	return 0;
}


/*
 * Try to find the application icon $PATH:
 * - return the icon location.
 */
static char *find_icon(char *arg_v)
{
	char *name, *p, *s, *tmp;
	char *tmp_value;

	name = malloc(strlen(arg_v) + 5);
	tmp = str_append_fast(name, arg_v);
	tmp = str_append_fast(tmp, ".png");

	if (!file_exists(name) && (tmp_value = getenv("PATH"))) {
		s = tmp_value;

		do {
			p = strchr(s, ':');

			if (p)
				*p = 0;

			// Set a file name
			free(name);
			name = malloc(strlen(s) + 10);
			tmp = str_append_fast(name, s);
			tmp = str_append_fast(tmp, "/leve.png");

			if (file_exists(name))
				break;
			s = p + 1;
		} while (p);
	}
	return name;
}

