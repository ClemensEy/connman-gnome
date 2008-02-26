/*
 *
 *  Connection Manager
 *
 *  Copyright (C) 2008  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <dbus/dbus-glib.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "client.h"
#include "status.h"

static void close_callback(GtkWidget *dialog, gpointer user_data)
{
	gtk_widget_destroy(dialog);
}

static void about_url_hook(GtkAboutDialog *dialog,
					const gchar *url, gpointer data)
{
}

static void about_email_hook(GtkAboutDialog *dialog,
					const gchar *email, gpointer data)
{
}

static void about_callback(GtkWidget *item, gpointer user_data)
{
	const gchar *authors[] = {
		"Marcel Holtmann <marcel@holtmann.org>",
		NULL
	};
	GtkWidget *dialog;

	dialog = gtk_about_dialog_new();

	gtk_about_dialog_set_name(GTK_ABOUT_DIALOG(dialog),
						_("Connection Manager"));
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), VERSION);
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog),
				"Copyright \xc2\xa9 2008 Intel Corporation");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog),
			_("A connection manager for the GNOME desktop"));
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(dialog),
							"stock_internet");

	gtk_about_dialog_set_url_hook(about_url_hook, NULL, NULL);
	gtk_about_dialog_set_email_hook(about_email_hook, NULL, NULL);

	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);
	gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(dialog),
						_("translator-credits"));

	g_signal_connect(dialog, "close", G_CALLBACK(close_callback), NULL);
	g_signal_connect(dialog, "response", G_CALLBACK(close_callback), NULL);

	gtk_widget_show_all(dialog);
}

static void settings_callback(GObject *widget, gpointer user_data)
{
	const char *command = "connman-properties";

	if (g_spawn_command_line_async(command, NULL) == FALSE)
		g_printerr("Couldn't execute command: %s\n", command);
}

static GtkWidget *create_popupmenu(void)
{
	GtkWidget *menu;
	GtkWidget *item;

	menu = gtk_menu_new();

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, NULL);
	g_signal_connect(item, "activate", G_CALLBACK(settings_callback), NULL);
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
	g_signal_connect(item, "activate", G_CALLBACK(about_callback), NULL);
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	return menu;
}

static GtkWidget *append_menuitem(GtkMenu *menu, const char *ssid,
					gboolean security, double strength)
{
	GtkWidget *item;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *image;
	GtkWidget *progress;

	item = gtk_check_menu_item_new();
	gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(item), TRUE);

	hbox = gtk_hbox_new(FALSE, 6);
	gtk_container_add(GTK_CONTAINER(item), hbox);
	gtk_widget_show(hbox);

	label = gtk_label_new(NULL);
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
	gtk_label_set_text(GTK_LABEL(label), ssid);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
	gtk_widget_show(label);

	image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_AUTHENTICATION,
							GTK_ICON_SIZE_MENU);
	gtk_misc_set_alignment(GTK_MISC(image), 1.0, 0.5);
	if (security == TRUE) {
		gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);
		gtk_widget_show(image);
	}

	progress = gtk_progress_bar_new();
	gtk_widget_set_size_request(progress, 100, -1);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), strength);
	gtk_box_pack_end(GTK_BOX(hbox), progress, FALSE, TRUE, 0);
	gtk_widget_show(progress);

	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	return item;
}

static void enumerate_networks(GtkMenu *menu,
				GtkTreeModel *model, GtkTreeIter *parent)
{
	GtkTreeIter iter;
	gboolean cont;

	cont = gtk_tree_model_iter_children(model, &iter, parent);

	while (cont == TRUE) {
		GtkWidget *item;
		gboolean security;
		guint signal;
		gchar *str;

		gtk_tree_model_get(model, &iter,
				CLIENT_COLUMN_SIGNAL, &signal,
				CLIENT_COLUMN_NETWORK_ESSID, &str,
				CLIENT_COLUMN_NETWORK_SECURITY, &security, -1);

		item = append_menuitem(menu, str, security,
						(double) signal / 100);

		g_free(str);

		cont = gtk_tree_model_iter_next(model, &iter);
	}
}

static gboolean menu_callback(GtkMenu *menu)
{
	GtkTreeModel *model;
	GtkTreeIter parent;
	GtkWidget *item;
	gboolean cont;

	client_propose_scanning();

	model = client_get_active_model();

	cont = gtk_tree_model_get_iter_first(model, &parent);

	while (cont == TRUE) {
		guint type;
		gchar *str;

		gtk_tree_model_get(model, &parent,
					CLIENT_COLUMN_TYPE, &type,
					CLIENT_COLUMN_PRODUCT, &str, -1);

		switch (type) {
		case CLIENT_TYPE_80211:
			enumerate_networks(menu, model, &parent);
			break;
		default:
			break;
		}

		g_free(str);

		cont = gtk_tree_model_iter_next(model, &parent);
	}

	g_object_unref(model);

	item = gtk_separator_menu_item_new();
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	item = gtk_menu_item_new_with_label(_("Join Other Network..."));
	gtk_widget_set_sensitive(item, FALSE);
	//g_signal_connect(item, "activate",
	//			G_CALLBACK(join_callback), NULL);
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	return TRUE;
}

static void state_callback(guint state, gint signal)
{
	switch (state) {
	case CLIENT_STATE_OFF:
	case CLIENT_STATE_ENABLED:
	case CLIENT_STATE_SHUTDOWN:
		status_offline();
		break;
	case CLIENT_STATE_SCANNING:
	case CLIENT_STATE_CONNECT:
		status_prepare();
		break;
	case CLIENT_STATE_CONNECTED:
	case CLIENT_STATE_CARRIER:
	case CLIENT_STATE_CONFIGURE:
		status_config();
		break;
	case CLIENT_STATE_READY:
		status_ready(signal);
		break;
	default:
		status_hide();
		break;
	}
}

static void sig_term(int sig)
{
	gtk_main_quit();
}

int main(int argc, char *argv[])
{
	struct sigaction sa;

	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	gtk_init(&argc, &argv);

	gtk_window_set_default_icon_name("stock_internet");

	status_init(menu_callback, create_popupmenu());

	client_init(NULL);

	client_set_state_callback(state_callback);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sig_term;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	gtk_main();

	client_cleanup();

	status_cleanup();

	return 0;
}
