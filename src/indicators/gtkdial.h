/**
 * This file is an re-implementation of the GtkDial from the following tutorial:
 * https://www.geany.org/manual/gtk/gtk-tutorial/x2312.html
 *
*/
#ifndef GTKDIAL_H__
#define GTKDIAL_H__
#include "glib.h"
G_BEGIN_DECLS
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#define GTK_DIAL(obj)               G_TYPE_CHECK_INSTANCE_CAST(obj, gtk_dial_get_type(), GtkDial)
#define GTK_DIAL_CLASS(klass)       G_TYPE_CHECK_CLASS_CAST(klass, gtk_dial_get_type(), GtkDialClass)
#define GTK_IS_DIAL(obj)            G_TYPE_CHECK_INSTANCE_TYPE(obj, gtk_dial_get_type())

typedef struct _GtkDial          GtkDial;
typedef struct _GtkDialClass      GtkDialClass;

struct _GtkDial
{
   GtkWidget   widget;

   /* update policy (GTK_UPDATE_[CONTINUOUS|DELAYED|DISCONTINUOUS]) */
   guint       policy;

   /* Button currently pressed or 0 in not pressed */
   guint8      button;

   /* Dial 'physical' dimensions */
   gint        radius;
   gint        pointer_width;

   /* Update timer ID */
   guint32     ud_timer;

   /* Current angular position of dial */
   gdouble     curr_angle;
   gdouble     last_angle;

   /* "Old values from adjustment stored so we know when something changes" - tutorial */
   gdouble     old_value;
   gdouble     old_lower;
   gdouble     old_upper;

   /* The adjustment object that stores the data for this dial */
   GtkAdjustment *adjustment;
};

struct _GtkDialClass
{
   GtkWidgetClass parent_class;

   void (*dial_changed) (GtkDial *dial, gdouble value);
};

GtkWidget*        gtk_dial_new               (GtkAdjustment *adjustment);
GType             gtk_dial_get_type          (void);
GtkAdjustment*    gtk_dial_get_adjustment    (GtkDial *dial);
void              gtk_dial_set_update_policy (GtkDial *dial,
                                              guint8 policy);  // GtkUpdateType?

void              gtk_dial_set_old_value     (GtkDial *dial, gdouble curr_value);
gdouble           gtk_dial_get_old_value     (GtkDial *dial);

void              gtk_dial_set_adjustment    (GtkDial *dial,
                                              GtkAdjustment *adjustment);

void              gtk_dial_destroy           (GObject *object);

G_END_DECLS
#endif  // GTKDIAL_H__
