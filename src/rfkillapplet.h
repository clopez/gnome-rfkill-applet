#define RFK_APPLET_ICON              "rfkill-applet"
#define RFK_APPLET_ICON_EMITTING     "rfkill-applet-a"
#define RFK_APPLET_ICON_KILLED       "rfkill-applet-b"
#define RFK_APPLET_ICON_UNKNOW       "rfkill-applet-c"
#define RFK_INHIBIT_APPLET_OAFID     "OAFIID:RFKillApplet"
#define RFK_INHIBIT_APPLET_FACTORY_OAFID "OAFIID:RFKillApplet_Factory"

#define RFK_INHIBIT_APPLET_NAME         _("RFKill Applet")
#define RFK_INHIBIT_APPLET_DESC         _("Allows user to inhibit the emission of radiation from RF devices.")
#define RFK_HOMEPAGE_URL                "http://gitorious.org/gnome-rfkill-applet"
#define RFK_APPLET_VERSION              "0.1"

/* Replace this paths for if you need it */
#define ICON_DATADIR                    "/usr/share/icons/hicolor"
#define XML_DATADIR                     "/usr/share/gnome-2.0/ui/"

#define RFK_TYPE_INHIBIT_APPLET         (rfk_inhibit_applet_get_type ())
#define RFK_INHIBIT_APPLET(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), RFK_TYPE_INHIBIT_APPLET, RFKillApplet))
#define RFK_INHIBIT_APPLET_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), RFK_TYPE_INHIBIT_APPLET, RFKillAppletClass))
#define RFK_IS_INHIBIT_APPLET(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), RFK_TYPE_INHIBIT_APPLET))
#define RFK_IS_INHIBIT_APPLET_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), RFK_TYPE_INHIBIT_APPLET))
#define RFK_INHIBIT_APPLET_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), RFK_TYPE_INHIBIT_APPLET, RFKillAppletClass))

/* possible values for RFKillApplet->status */
#define RADIATION_KILLED    0
#define RADIATION_EMITTING  1
#define RADIATION_UNKNOW    2

/* The number of seconds we wait until update our status */
#define RFKILL_CHECK_INTERVAL 1

/* The structure for the rfkill applet */
typedef struct{
    PanelApplet parent;
    /* the icon and a cache for size*/
    GdkPixbuf *icon;
    gint icon_width, icon_height;
    /* applet state ( radiation killed / radiation emitting / unknow ) */
    guint status;
    /* the message/error for the tooltip */
    gchar* tooltip;
    /* a cache for panel size */
    gint size;
} RFKillApplet;


typedef struct{
    PanelAppletClass    parent_class;
} RFKillAppletClass;



GType                rfk_inhibit_applet_get_type  (void);



static void      rfk_inhibit_applet_class_init (RFKillAppletClass *klass);
static void      rfk_inhibit_applet_init       (RFKillApplet *applet);

G_DEFINE_TYPE (RFKillApplet, rfk_inhibit_applet, PANEL_TYPE_APPLET)

static void rfk_applet_get_icon         (RFKillApplet *applet);
static void rfk_applet_check_size       (RFKillApplet *applet);
static gboolean rfk_applet_draw_cb      (RFKillApplet *applet);
static void rfk_applet_update_tooltip   (RFKillApplet *applet);
static gboolean rfk_applet_click_cb     (RFKillApplet *applet, GdkEventButton *event);
static void rfk_applet_dialog_about_cb  (BonoboUIComponent *uic, gpointer data, const gchar *verbname);
static gboolean rfk_applet_bonobo_cb    (PanelApplet *_applet, const gchar *iid, gpointer data);
static void rfk_applet_destroy_cb       (GtkObject *object);



#define PANEL_APPLET_VERTICAL(p)                    \
     (((p) == PANEL_APPLET_ORIENT_LEFT) || ((p) == PANEL_APPLET_ORIENT_RIGHT))




/**
 * rfk_inhibit_applet_class_init:
 * @klass: Class instance
 **/
static void
rfk_inhibit_applet_class_init (RFKillAppletClass *class)
{
    /* nothing to do here */
}

