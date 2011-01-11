/*
 * RFKillApplet
 * An applet for Gnome that lets the user to inhibit the emission of radiation
 * of RF devices with a simple click on the applet’s icon.
 * The device manipulation is done using the special device /dev/rfkill
 *
 * Copyright (C) 2011 Carlos Alberto Lopez Perez <clopez@igalia.com>
 *
 * Applet code based on Gnome Power Inhibit Applet
 * http://projects.gnome.org/gnome-power-manager/
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <panel-applet.h>
#include <gtk/gtk.h>
#include <glib-object.h>

#include "rfkillapplet.h"

#include "rfkilldevice.c"


/**
 * rfk_applet_click_cb:
 * @applet: Inhibit applet instance
 *
 * pops and unpops
 **/
static gboolean
rfk_applet_click_cb (RFKillApplet *applet, GdkEventButton *event)
{
    /* react only to left mouse button */
    if (event->button != 1) {
        return FALSE;
    }

	/* Try to flip-flop our status */
	rfkill_change_status(applet);
	/* Don't trust anybody. Simply check our status */
	rfkill_get_status(applet);
	/* Update applet status */
	rfk_applet_get_icon (applet);
	rfk_applet_check_size (applet);
	rfk_applet_update_tooltip (applet);
	rfk_applet_draw_cb (applet);

	return TRUE;
}




/**
 * rfk_applet_update_tooltip:
 * @applet: Inhibit applet instance
 *
 * sets tooltip's content (percentage or disabled)
 **/
static void
rfk_applet_update_tooltip (RFKillApplet *applet)
{
    gtk_widget_set_tooltip_text (GTK_WIDGET(applet), applet->tooltip );
}



/**
 * rfk_applet_destroy_cb:
 * @object: Class instance to destroy
 **/
static void
rfk_applet_destroy_cb (GtkObject *object)
{
    RFKillApplet *applet = RFK_INHIBIT_APPLET(object);

    /* Free the memory allocated */

    if (applet->tooltip != NULL) {
        g_object_unref (applet->tooltip);
    }
    if (applet->icon != NULL) {
        g_object_unref (applet->icon);
    }

}

/**
 * rfk_applet_get_icon:
 * @applet: Inhibit applet instance
 *
 * retrieve an icon from stock with a size adapted to panel
 **/
static void
rfk_applet_get_icon (RFKillApplet *applet)
{
    const gchar *icon;

    /* free */
    if (applet->icon != NULL) {
        g_object_unref (applet->icon);
        applet->icon = NULL;
    }

    if (applet->size <= 2) {
        return;
    }

    /* get icon */
    if ( applet->status == RADIATION_EMITTING ) {
        icon = RFK_APPLET_ICON_EMITTING;

    } else if ( applet->status == RADIATION_KILLED ) {
        icon = RFK_APPLET_ICON_KILLED;

    } else icon = RFK_APPLET_ICON_UNKNOW;

    applet->icon = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                         icon,
                         applet->size - 2,
                         0,
                         NULL);

    /* update size cache */
    applet->icon_height = gdk_pixbuf_get_height (applet->icon);
    applet->icon_width = gdk_pixbuf_get_width (applet->icon);
}

/**
 * rfk_applet_check_size:
 * @applet: Inhibit applet instance
 *
 * check if panel size has changed and applet adapt size
 **/
static void
rfk_applet_check_size (RFKillApplet *applet)
{
    GtkAllocation allocation;

    /* we don't use the size function here, but the yet allocated size because the
       size value is false (kind of rounded) */
    gtk_widget_get_allocation (GTK_WIDGET (applet), &allocation);
    if (PANEL_APPLET_VERTICAL(panel_applet_get_orient (PANEL_APPLET (applet)))) {
        if (applet->size != allocation.width) {
            applet->size = allocation.width;
            rfk_applet_get_icon (applet);
            gtk_widget_set_size_request (GTK_WIDGET(applet), applet->size, applet->icon_height + 2);
        }
        /* Adjusting incase the icon size has changed */
        if (allocation.height < applet->icon_height + 2) {
            gtk_widget_set_size_request (GTK_WIDGET(applet), applet->size, applet->icon_height + 2);
        }
    } else {
        if (applet->size != allocation.height) {
            applet->size = allocation.height;
            rfk_applet_get_icon (applet);
            gtk_widget_set_size_request (GTK_WIDGET(applet), applet->icon_width + 2, applet->size);
        }
        /* Adjusting incase the icon size has changed */
        if (allocation.width < applet->icon_width + 2) {
            gtk_widget_set_size_request (GTK_WIDGET(applet), applet->icon_width + 2, applet->size);
        }
    }
}


/**
 * rfk_applet_change_background_cb:
 *
 * Enqueues an expose event (don't know why it's not the default behaviour)
 **/
static void
rfk_applet_change_background_cb (RFKillApplet *applet,
                 PanelAppletBackgroundType arg1,
                 GdkColor *arg2, GdkPixmap *arg3, gpointer data)
{
    gtk_widget_queue_draw (GTK_WIDGET (applet));
}

/**
 * rfk_applet_draw_cb:
 * @applet: Inhibit applet instance
 *
 * draws applet content (background + icon)
 **/
static gboolean
rfk_applet_draw_cb (RFKillApplet *applet)
{
    gint w, h, bg_type;
    GdkColor color;
    GdkGC *gc;
    GdkPixmap *background;
    GtkAllocation allocation;

    if (gtk_widget_get_window (GTK_WIDGET(applet)) == NULL) {
        return FALSE;
    }

    /* Clear the window so we can draw on it later */
    gdk_window_clear(gtk_widget_get_window (GTK_WIDGET (applet)));

    /* retrieve applet size */
    rfk_applet_get_icon (applet);
    rfk_applet_check_size (applet);
    if (applet->size <= 2) {
        return FALSE;
    }

    /* if no icon, then don't try to display */
    if (applet->icon == NULL) {
        return FALSE;
    }

    gtk_widget_get_allocation (GTK_WIDGET (applet), &allocation);
    w = allocation.width;
    h = allocation.height;

    gc = gdk_gc_new (gtk_widget_get_window (GTK_WIDGET(applet)));

    /* draw pixmap background */
    bg_type = panel_applet_get_background (PANEL_APPLET (applet), &color, &background);
    if (bg_type == PANEL_PIXMAP_BACKGROUND) {
        /* fill with given background pixmap */
        gdk_draw_drawable (gtk_widget_get_window (GTK_WIDGET(applet)), gc, background, 0, 0, 0, 0, w, h);
    }

    /* draw color background */
    if (bg_type == PANEL_COLOR_BACKGROUND) {
        gdk_gc_set_rgb_fg_color (gc,&color);
        gdk_gc_set_fill (gc,GDK_SOLID);
        gdk_draw_rectangle (gtk_widget_get_window (GTK_WIDGET(applet)), gc, TRUE, 0, 0, w, h);
    }

    /* draw icon at center */
    gdk_draw_pixbuf (gtk_widget_get_window (GTK_WIDGET(applet)), gc, applet->icon,
             0, 0, (w - applet->icon_width)/2, (h - applet->icon_height)/2,
             applet->icon_width, applet->icon_height,
             GDK_RGB_DITHER_NONE, 0, 0);

    return TRUE;
}


/**
 * rfk_inhibit_applet_init:
 * @applet: Inhibit applet instance
 **/
static void
rfk_inhibit_applet_init (RFKillApplet *applet)
{


    /* initialize fields */
    applet->size = 0;

    /* check /dev/rfkill and set the status and tooltip */
    rfkill_get_status(applet);

    /* Add application specific icons to search path */
    gtk_icon_theme_append_search_path (gtk_icon_theme_get_default (),
                                           ICON_DATA G_DIR_SEPARATOR_S "icons");

    /* prepare */
    panel_applet_set_flags (PANEL_APPLET (applet), PANEL_APPLET_EXPAND_MINOR);

    /* show */
    gtk_widget_show_all (GTK_WIDGET(applet));

    /* set appropriate size and load icon accordingly */
    rfk_applet_draw_cb (applet);

    /* connect */
    g_signal_connect (G_OBJECT(applet), "button-press-event",
              G_CALLBACK(rfk_applet_click_cb), NULL);

    g_signal_connect (G_OBJECT(applet), "expose-event",
              G_CALLBACK(rfk_applet_draw_cb), NULL);

    /* We use g_signal_connect_after because letting the panel draw
     * the background is the only way to have the correct
     * background when a theme defines a background picture. */
    g_signal_connect_after (G_OBJECT(applet), "expose-event",
                G_CALLBACK(rfk_applet_draw_cb), NULL);

    g_signal_connect (G_OBJECT(applet), "change-background",
              G_CALLBACK(rfk_applet_change_background_cb), NULL);

    g_signal_connect (G_OBJECT(applet), "change-orient",
              G_CALLBACK(rfk_applet_draw_cb), NULL);

    g_signal_connect (G_OBJECT(applet), "destroy",
              G_CALLBACK(rfk_applet_destroy_cb), NULL);
}

/**
 * rfk_applet_bonobo_cb:
 * @_applet: RFKillApplet instance created by the bonobo factory
 * @iid: Bonobo id
 *
 * the function called by bonobo factory after creation
 **/
static gboolean
rfk_applet_bonobo_cb (PanelApplet *_applet, const gchar *iid, gpointer data)
{
    RFKillApplet *applet = RFK_INHIBIT_APPLET(_applet);

    /*
    static BonoboUIVerb verbs [] = {
        BONOBO_UI_VERB ("About", rfk_applet_dialog_about_cb),
        BONOBO_UI_VERB ("Help", rfk_applet_help_cb),
        BONOBO_UI_VERB_END
    };
    */
    if (strcmp (iid, RFK_INHIBIT_APPLET_OAFID) != 0) {
        return FALSE;
    }

    /*
    panel_applet_setup_menu_from_file (PANEL_APPLET (applet),
                       DATADIR,
                       "GNOME_InhibitApplet.xml",
                       NULL, verbs, applet);
    */
    rfk_applet_draw_cb (applet);
    return TRUE;
}



/**
 * this generates a main with a bonobo factory
 **/
PANEL_APPLET_BONOBO_FACTORY
     (/* the factory iid */
     RFK_INHIBIT_APPLET_FACTORY_OAFID,
     /* generates brighness applets instead of regular gnome applets  */
     RFK_TYPE_INHIBIT_APPLET,
     /* the applet name and version */
     "RFKillApplet", VERSION,
     /* our callback (with no user data) */
     rfk_applet_bonobo_cb, NULL);
