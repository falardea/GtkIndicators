/**
 * Implementation
 */
#include <math.h>
#include <cairo/cairo.h>
#include <glib-object.h>
#include "gtkdial.h"

// #include "views/root_child_msgout.h"

#define SCROLL_DELAY_LENGTH      300
// static gint DIAL_DEFAULT_SIZE = 100;

/* Forward declarations */
static void       gtk_dial_class_init              (GtkDialClass *klass,
                                                    gpointer class_data);
static void       gtk_dial_init                    (GtkDial *dial,
                                                    gpointer g_class);
static void       gtk_dial_realize                 (GtkWidget *widget);
// static void       gtk_dial_size_request            (GtkWidget* widget,
//                                                     GtkOrientation orientation,
//                                                     gint* minimum_size,
//                                                     gint* natural_size);
static void       gtk_dial_size_allocate           (GtkWidget *widget,
                                                    GtkAllocation *allocation);
static gboolean   gtk_dial_draw                    (GtkWidget *widget,
                                                    cairo_t *cr);
static gboolean   gtk_dial_button_press            (GtkWidget *widget,
                                                    GdkEventButton *event);
static gboolean   gtk_dial_button_release          (GtkWidget *widget,
                                                    GdkEventButton *event);
static gboolean   gtk_dial_motion_notify           (GtkWidget *widget,
                                                    GdkEventMotion *event);
static gboolean   gtk_dial_timer                   (GtkDial *dial);

static void       gtk_dial_update_mouse            (GtkDial *dial, gint x, gint y);
static void       gtk_dial_update                  (GtkDial *dial);
static void       gtk_dial_adjustment_changed      (GtkAdjustment *adjustment,
                                                    gpointer data);
static void       gtk_dial_adjustment_value_changed(GtkAdjustment *adjustment,
                                                    gpointer data);

/* Local data */
static GtkWidgetClass *parent_class = NULL;

enum {
   DIAL_CHANGED_SIGNAL,
   NUM_DIAL_SIGNALS
};

enum {
   PROP_0, // reserved for GObject
   GTK_DIAL_PROP_OLD_VALUE,
   N_GTK_DIAL_PROPERTIES
};

static guint gtk_dial_signals[NUM_DIAL_SIGNALS] = { 0 };
static GParamSpec *gtk_dial_properties[N_GTK_DIAL_PROPERTIES] = {NULL, };

GType gtk_dial_get_type()
{
   static GType dial_type = 0;

   if (!dial_type)
   {
      static const GTypeInfo dial_info =
            {
                  sizeof (GtkDialClass),
                  NULL,
                  NULL,
                  (GClassInitFunc) gtk_dial_class_init,
                  NULL,
                  NULL,
                  sizeof(GtkDial),
                  0,
                  (GInstanceInitFunc) gtk_dial_init,
                  NULL
            };
      dial_type = g_type_register_static(GTK_TYPE_WIDGET, "GtkDial", &dial_info, 0);
   }
   return dial_type;
}

static void gtk_dial_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
   GtkDial *dial = GTK_DIAL(object);

   switch(prop_id){
      case GTK_DIAL_PROP_OLD_VALUE:
         gtk_dial_set_old_value( dial, g_value_get_double ( value ) );
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
   }

}

static void gtk_dial_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
   GtkDial *dial = GTK_DIAL(object);

   switch (prop_id){
      case GTK_DIAL_PROP_OLD_VALUE:
         g_value_set_double(value, gtk_dial_get_old_value(dial));
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
   }
}

static void gtk_dial_class_init(GtkDialClass *klass,__attribute__((unused)) gpointer class_data)
{
   GObjectClass         *object_class = (GObjectClass *) klass;
   GtkWidgetClass       *widget_class = (GtkWidgetClass *) klass;

   parent_class = g_type_class_ref(gtk_widget_get_type());

   object_class->dispose = gtk_dial_destroy;  // finalize?
   object_class->get_property = gtk_dial_get_property;
   object_class->set_property = gtk_dial_set_property;

   widget_class->realize = gtk_dial_realize;
   widget_class->draw = gtk_dial_draw;
   widget_class->size_allocate = gtk_dial_size_allocate;
   widget_class->button_press_event = gtk_dial_button_press;
   widget_class->button_release_event = gtk_dial_button_release;
   widget_class->motion_notify_event = gtk_dial_motion_notify;

   gtk_dial_properties[GTK_DIAL_PROP_OLD_VALUE] = g_param_spec_double("old_value", "old value", "Last dial value",
                                                                              -G_MINDOUBLE, G_MAXDOUBLE, 0, G_PARAM_READWRITE );
   g_object_class_install_properties( object_class, N_GTK_DIAL_PROPERTIES, gtk_dial_properties);
   gtk_dial_signals[DIAL_CHANGED_SIGNAL] = g_signal_new("dial-changed",
                                                        G_TYPE_FROM_CLASS(klass),
                                                        G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                                        G_STRUCT_OFFSET (GtkDialClass , dial_changed),
                                                        NULL,
                                                        NULL,
                                                        g_cclosure_marshal_VOID__DOUBLE,
                                                        G_TYPE_NONE, 1, G_TYPE_DOUBLE);
}

static void gtk_dial_init(GtkDial *dial,__attribute__((unused))  gpointer g_class)
{
   dial->button = 0;
   dial->policy = GTK_UPDATE_ALWAYS;
   dial->ud_timer = 0;
   dial->radius = 0;
   dial->curr_angle = 0;
   dial->last_angle = 0;
   dial->pointer_width = 0;
   dial->old_value = 0;
   dial->old_lower = 0;
   dial->old_upper = 0;
   dial->adjustment = NULL;
}

GtkWidget* gtk_dial_new(GtkAdjustment *adjustment)
{
   GtkDial *dial;

   dial = g_object_new(gtk_dial_get_type(), NULL);

   if(!adjustment)
   {
      adjustment = (GtkAdjustment*) gtk_adjustment_new(0, 0, 100, 0.01, 0.1, 0);
   }

   gtk_dial_set_adjustment(dial, adjustment);

   return GTK_WIDGET(dial);
}

void gtk_dial_destroy(GObject *object)
{
   GtkDial *dial;

   g_return_if_fail(object != NULL);
   g_return_if_fail(GTK_IS_DIAL(object));

   dial = GTK_DIAL(object);

   if(dial->adjustment)
   {
      g_object_unref(G_OBJECT(dial->adjustment));
      dial->adjustment = NULL;
   }

   if(G_OBJECT_CLASS(parent_class)->dispose)
   {
      (* G_OBJECT_CLASS (parent_class)->dispose) (object);
   }
}

GtkAdjustment* gtk_dial_get_adjustment(GtkDial *dial)
{
   g_return_val_if_fail(dial != NULL, NULL);
   g_return_val_if_fail(GTK_IS_DIAL(dial), NULL);

   return dial->adjustment;
}

void gtk_dial_update_policy(GtkDial *dial, guint8 policy)
{
   g_return_if_fail(dial != NULL);
   g_return_if_fail(GTK_IS_DIAL(dial));

   dial->policy = policy;
}

void gtk_dial_set_adjustment(GtkDial *dial, GtkAdjustment *adjustment)
{
   g_return_if_fail(dial != NULL);
   g_return_if_fail(GTK_IS_DIAL(dial));

   if(dial->adjustment)
   {
      g_signal_handlers_disconnect_by_func(G_OBJECT(dial->adjustment), gtk_dial_adjustment_changed, (gpointer) dial);
      g_signal_handlers_disconnect_by_func(G_OBJECT(dial->adjustment), gtk_dial_adjustment_value_changed, (gpointer) dial);
      g_object_unref(G_OBJECT(dial->adjustment));
   }

   dial->adjustment = adjustment;
   g_object_ref(G_OBJECT(dial->adjustment));
   g_signal_connect(G_OBJECT(dial->adjustment), "changed", G_CALLBACK(gtk_dial_adjustment_changed),
                    (gpointer) dial);
   g_signal_connect(G_OBJECT(dial->adjustment), "value-changed", G_CALLBACK(gtk_dial_adjustment_value_changed),
                    (gpointer) dial);

   dial->old_value = gtk_adjustment_get_value(adjustment);
   dial->old_lower = gtk_adjustment_get_lower(adjustment);
   dial->old_upper = gtk_adjustment_get_upper(adjustment);

   gtk_dial_update(dial);
}

static void gtk_dial_realize(GtkWidget *widget)
{
   GdkWindowAttr  attributes;
   GtkAllocation  allocations;
   gint           attributes_mask;

   g_return_if_fail(widget != NULL);
   g_return_if_fail(GTK_IS_DIAL(widget));

   gtk_widget_set_realized(widget, TRUE);
   // GtkDial *dial = GTK_DIAL(widget);

   gtk_widget_get_allocation(widget, &allocations);

   attributes.x            = allocations.x;
   attributes.y            = allocations.y;
   attributes.width        = allocations.width;
   attributes.height       = allocations.height;
   attributes.wclass       = GDK_INPUT_OUTPUT;
   attributes.window_type  = GDK_WINDOW_CHILD;
   attributes.event_mask   = gtk_widget_get_events(widget) |
         GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK |
         GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
         GDK_POINTER_MOTION_HINT_MASK;
   // GTK2 has an attributes.colormap, but may have been packed them into visual in GTK3
   attributes.visual       = gtk_widget_get_visual(widget);

   attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;
   gtk_widget_set_window(widget,gdk_window_new(gtk_widget_get_parent_window(widget), &attributes, attributes_mask));

   gdk_window_set_user_data (gtk_widget_get_window(widget), widget);
}

static void gtk_dial_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
   GtkDial *dial;

   g_return_if_fail(widget != NULL);
   g_return_if_fail(GTK_IS_DIAL(widget));
   g_return_if_fail(allocation != NULL);

   gtk_widget_set_allocation(widget, allocation);
   dial = GTK_DIAL(widget);

   if (gtk_widget_get_realized(widget))
   {
      gdk_window_move_resize(gtk_widget_get_window(widget), allocation->x, allocation->y,
                             allocation->width, allocation->height);
   }
   dial->radius = MIN (allocation->width, allocation->height) * 0.45;
   dial->pointer_width = dial->radius / 5;
}

static void gtk_dial_draw_dial_ticks(GtkWidget *widget, cairo_t *cr)
{
   GtkDial        *dial;
   gint           xcalc, ycalc;
   gint           i, inc;
   gdouble        s, c;
   gdouble        theta, last, increment;
   gint           tick_length;
   gdouble        upper, lower;

   g_return_if_fail(widget != NULL);
   g_return_if_fail(GTK_IS_DIAL(widget));
   g_return_if_fail(cr != NULL);

   dial = GTK_DIAL(widget);

   upper = gtk_adjustment_get_upper(dial->adjustment);
   lower = gtk_adjustment_get_lower(dial->adjustment);

   xcalc = gtk_widget_get_allocated_width(widget) / 2;
   ycalc = gtk_widget_get_allocated_height(widget) / 2;

   if ((upper - lower) == 0)
   {
      return;
   }

   increment = (100 * M_PI) / (dial->radius * dial->radius);
   inc = (upper - lower);

   while (inc < 100) inc *= 10;
   while (inc >= 1000) inc /= 10;
   last = -1;

   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
   cairo_set_line_width(cr, 2.0);
   gint x1, x2, y1, y2;
   for (i = 0; i <= inc; i++)
   {
      theta = ((gdouble) i*M_PI / (18*inc/24.) - M_PI/6.);

      if ((theta - last) < (increment))
         continue;
      last = theta;

      s = sin(theta);
      c = cos(theta);
      tick_length = (i % (inc/10) == 0) ? dial->pointer_width : dial->pointer_width/2;

      x1 = xcalc + c*(dial->radius - tick_length);
      y1 = ycalc - s*(dial->radius - tick_length);
      x2 = xcalc + c*dial->radius;
      y2 = ycalc - s*dial->radius;
      cairo_move_to(cr, x1, y1);
      cairo_line_to(cr, x2, y2);
      cairo_stroke(cr);
   }
}

static gboolean gtk_dial_draw(GtkWidget *widget, cairo_t *cr)
{
   GtkDial        *dial;
   GdkPoint       points[6];
   gdouble        s, c;
   gint           xc, yc;

   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(GTK_IS_DIAL(widget), FALSE);
   g_return_val_if_fail(cr != NULL, FALSE);

   dial = GTK_DIAL(widget);

   int width = gtk_widget_get_allocated_width(widget);
   int height = gtk_widget_get_allocated_height(widget);

   GtkStyleContext  *context = gtk_widget_get_style_context(widget);
   gtk_style_context_add_class(context, "dial-style");
   gtk_render_background(context, cr, 0, 0, width, height);
   gtk_render_frame(context, cr, 0, 0, width, height);

   xc = gtk_widget_get_allocated_width(widget) / 2;
   yc = gtk_widget_get_allocated_height(widget) / 2;

   ///// Draw the Dial //////////////////
   gtk_dial_draw_dial_ticks(widget, cr);

   ///// Draw the Needle ///////////////////
   s = sin(dial->curr_angle);
   c = cos(dial->curr_angle);
   points[0].x = xc + s * dial->pointer_width/2;
   points[0].y = yc + c * dial->pointer_width/2;
   points[1].x = xc + c * dial->radius;
   points[1].y = yc - s * dial->radius;
   points[2].x = xc - s * dial->pointer_width/2;
   points[2].y = yc - c * dial->pointer_width/2;
   points[3].x = xc - c * dial->radius/10;
   points[3].y = yc + s * dial->radius/10;
   points[4].x = points[0].x;
   points[4].y = points[0].y;

   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
   cairo_set_line_width(cr, 4.0);
   cairo_move_to(cr, points[0].x, points[0].y);
   cairo_line_to(cr, points[1].x, points[1].y);
   cairo_line_to(cr, points[2].x, points[2].y);
   cairo_line_to(cr, points[3].x, points[3].y);
   cairo_line_to(cr, points[4].x, points[4].y);
   cairo_line_to(cr, points[0].x, points[0].y);
   cairo_fill (cr);

   dial->last_angle = dial->curr_angle;
   return FALSE;
}

static gboolean gtk_dial_button_press(GtkWidget *widget, GdkEventButton *event)
{
   GtkDial        *dial;
   gdouble        dx, dy;
   gdouble        s, c;
   gdouble        d_parallel;
   gdouble        d_perpendicular;

   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(GTK_IS_DIAL(widget), FALSE);
   g_return_val_if_fail(event != NULL, FALSE);

   dial = GTK_DIAL(widget);

   /* ORIGINAL NOTE: Determine if button press was within pointer region -
    * we do this by computing the parallel and perpendicular distance of
    * the point where the mouse was pressed from the line passing through
    * the pointer */

   dx = event->x - gtk_widget_get_allocated_width(widget) / 2.;
   dy = gtk_widget_get_allocated_height(widget) / 2. - event->y;

   s = sin(dial->curr_angle);
   c = cos(dial->curr_angle);

   d_parallel = s*dy + c*dx;
   d_perpendicular = fabs(s*dx - c*dy);

   if (!dial->button && (d_perpendicular < dial->pointer_width/2.) && (d_parallel > -dial->pointer_width))
   {
      gtk_grab_add(widget);
      dial->button = event->button;
      gtk_dial_update_mouse(dial, ceil(event->x), ceil(event->y));
   }

   return FALSE;
}

static gboolean gtk_dial_button_release(GtkWidget *widget, GdkEventButton *event)
{
   GtkDial        *dial;

   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(GTK_IS_DIAL(widget), FALSE);
   g_return_val_if_fail(event != NULL, FALSE);

   dial = GTK_DIAL(widget);

   if (dial->button == event->button)
   {
      gtk_grab_remove(widget);

      dial->button = 0;

      if (dial->policy != GTK_UPDATE_ALWAYS)
      {
         g_source_remove(dial->ud_timer);
      }
      if (dial->policy == GTK_UPDATE_ALWAYS && dial->old_value != gtk_adjustment_get_value(dial->adjustment))
      {
         g_signal_emit_by_name(G_OBJECT(dial->adjustment), "value-changed");
         g_signal_emit(G_OBJECT(dial), gtk_dial_signals[DIAL_CHANGED_SIGNAL], 0, gtk_adjustment_get_value(dial->adjustment));
      }
   }

   return FALSE;
}

static gboolean gtk_dial_motion_notify(GtkWidget *widget, GdkEventMotion *event)
{
   GtkDial           *dial;
   GdkModifierType   mods;
   gint              x, y, mask;

   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(GTK_IS_DIAL(widget), FALSE);
   g_return_val_if_fail(event != NULL, FALSE);

   dial = GTK_DIAL(widget);

   if (dial->button != 0)
   {
      x = event->x;
      y = event->y;

      if (event->is_hint || event->window != gtk_widget_get_window(widget))
      {
         // gdk_window_get_device_position will throw errors, but this works
         // https://stackoverflow.com/questions/24844489/how-to-use-gdk-device-get-position
         GdkDisplay* display = gdk_display_get_default();
         GdkSeat* seat = gdk_display_get_default_seat(display);
         GdkDevice* pointer = gdk_seat_get_pointer(seat);
         gdk_window_get_device_position(gtk_widget_get_window(widget),
                                        pointer, &x, &y, &mods);
      }
      switch(dial->button)
      {
         case 1:
            mask = GDK_BUTTON1_MASK;
            break;
         case 2:
            mask = GDK_BUTTON2_MASK;
            break;
         case 3:
            mask = GDK_BUTTON3_MASK;
            break;
         default:
            mask = 0;
            break;
      }
      if(mods & mask)
      {
         gtk_dial_update_mouse(dial, x, y);
      }
   }
   return FALSE;
}

static gboolean gtk_dial_timer(GtkDial *dial)
{
   g_return_val_if_fail(dial != NULL, FALSE);
   g_return_val_if_fail(GTK_IS_DIAL(dial), FALSE);

   if (dial->policy != GTK_UPDATE_ALWAYS)
   {
      g_signal_emit_by_name(G_OBJECT(dial->adjustment), "value-changed");
      g_signal_emit(G_OBJECT(dial), gtk_dial_signals[DIAL_CHANGED_SIGNAL], 0, gtk_adjustment_get_value(dial->adjustment));
   }
   return FALSE;
}

static void gtk_dial_update_mouse(GtkDial *dial, gint x, gint y)
{
   gint     xc, yc;
   gdouble  old_value;

   g_return_if_fail(dial != NULL);
   g_return_if_fail(GTK_IS_DIAL(dial));

   // GtkAllocation allocation;
   xc = gtk_widget_get_allocated_width(GTK_WIDGET(dial)) / 2;
   yc = gtk_widget_get_allocated_height(GTK_WIDGET(dial)) / 2;

   old_value = gtk_adjustment_get_value(dial->adjustment);
   dial->curr_angle = atan2(yc - y, x - xc);

   if (dial->curr_angle < -M_PI/2.)
   {
      dial->curr_angle += 2*M_PI;
   }
   if (dial->curr_angle < -M_PI/6.)
   {
      dial->curr_angle = -M_PI/6.;
   }
   if (dial->curr_angle > 7.*M_PI/6.)
   {
      dial->curr_angle = 7.*M_PI/6.;
   }

   gdouble new_adjustment_value = gtk_adjustment_get_lower(dial->adjustment) +
         (7.*M_PI/6. - dial->curr_angle) * (gtk_adjustment_get_upper(dial->adjustment) - gtk_adjustment_get_lower(dial->adjustment)) / (4.*M_PI/3.);
   gtk_adjustment_set_value(dial->adjustment, new_adjustment_value);

   if (gtk_adjustment_get_value(dial->adjustment) != old_value)
   {
      if (dial->policy == GTK_UPDATE_ALWAYS)
      {
         g_signal_emit_by_name(G_OBJECT(dial->adjustment), "value-changed");
         g_signal_emit(G_OBJECT(dial), gtk_dial_signals[DIAL_CHANGED_SIGNAL], 0, new_adjustment_value);
      }
      else
      {
         gtk_widget_queue_draw(GTK_WIDGET(dial));
         if (dial->ud_timer)
            g_source_remove(dial->ud_timer);
         dial->ud_timer = g_timeout_add(SCROLL_DELAY_LENGTH, (GSourceFunc)gtk_dial_timer, (gpointer) dial);
      }
   }
}

static void gtk_dial_update(GtkDial *dial)
{
   gdouble new_value;

   g_return_if_fail(dial != NULL);
   g_return_if_fail(GTK_IS_DIAL(dial));

   new_value = gtk_adjustment_get_value(dial->adjustment);

   if (new_value < gtk_adjustment_get_lower(dial->adjustment))
   {
      new_value = gtk_adjustment_get_lower(dial->adjustment);
   }

   if (new_value > gtk_adjustment_get_upper(dial->adjustment))
   {
      new_value = gtk_adjustment_get_upper(dial->adjustment);
   }

   if (new_value != gtk_adjustment_get_value(dial->adjustment))
   {
      gtk_adjustment_set_value(dial->adjustment, new_value);

      g_signal_emit_by_name(G_OBJECT(dial->adjustment), "value-changed");
      g_signal_emit(G_OBJECT(dial), gtk_dial_signals[DIAL_CHANGED_SIGNAL], 0, new_value);
   }

   dial->curr_angle = 7.*M_PI/6. - (new_value - gtk_adjustment_get_lower(dial->adjustment)) * 4.*M_PI/3. /
                                                      (gtk_adjustment_get_upper(dial->adjustment) - gtk_adjustment_get_lower(dial->adjustment));

   gtk_widget_queue_draw(GTK_WIDGET(dial));
}

static void gtk_dial_adjustment_changed(GtkAdjustment *adjustment, gpointer data)
{
   GtkDial        *dial;

   g_return_if_fail(adjustment != NULL);
   g_return_if_fail(data != NULL);

   dial = GTK_DIAL(data);

   if (dial->old_value != gtk_adjustment_get_value(adjustment) ||
   dial->old_lower != gtk_adjustment_get_lower(adjustment) ||
   dial->old_upper != gtk_adjustment_get_upper(adjustment))
   {
      gtk_dial_update(dial);

      dial->old_value = gtk_adjustment_get_value(adjustment);
      dial->old_lower = gtk_adjustment_get_lower(adjustment);
      dial->old_upper = gtk_adjustment_get_upper(adjustment);
   }
}

static void gtk_dial_adjustment_value_changed(GtkAdjustment *adjustment, gpointer data)
{
   GtkDial *dial;

   g_return_if_fail(adjustment != NULL);
   g_return_if_fail(data != NULL);

   dial = GTK_DIAL(data);

   if (dial->old_value != gtk_adjustment_get_value(adjustment))
   {
      gtk_dial_update(dial);
      gtk_dial_set_old_value(dial, gtk_adjustment_get_value(adjustment));
   }
}

void gtk_dial_set_old_value (GtkDial *dial, gdouble curr_value)
{
   g_return_if_fail(GTK_IS_DIAL(dial));
   dial->old_value = curr_value;
   g_object_notify_by_pspec(G_OBJECT(dial), gtk_dial_properties[GTK_DIAL_PROP_OLD_VALUE]);
}
gdouble gtk_dial_get_old_value (GtkDial *dial)
{
   g_return_val_if_fail(GTK_IS_DIAL(dial), NAN);
   return dial->old_value;
}