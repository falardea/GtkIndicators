/**
 * Created by french on 9/22/24.
 * @brief
 */
#include <math.h>
#include <cairo/cairo.h>
#include <glib-object.h>
#include "bluetooth_indicator.h"

#include "utils/logging.h"

struct _BluetoothIndicator
{
   GtkWidget            parent;
   gdouble              value;         // the percent charge
   gdouble              old_value;     // to determine change?
};

// It appears that anything that isn't defined as a property is effectively private instance data?
enum {
   PROP_0,
   BLUETOOTH_INDICATOR_VALUE_PROP,
   N_BLUETOOTH_INDICATOR_PROPERTIES
};

G_DEFINE_TYPE(BluetoothIndicator, bluetooth_indicator, GTK_TYPE_WIDGET)

static void       bluetooth_indicator_realize                 (GtkWidget *widget);
static void       bluetooth_indicator_size_allocate           (GtkWidget *widget,
                                                             GtkAllocation *allocation);
static gboolean   bluetooth_indicator_draw                    (GtkWidget *widget,
                                                             cairo_t *cr);
static gboolean   bluetooth_indicator_button_press            (GtkWidget *widget,
                                                             GdkEventButton *event);
static void bluetooth_indicator_set_property( GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
   BluetoothIndicator *self = (BluetoothIndicator *) object;
   switch (prop_id)
   {
      case BLUETOOTH_INDICATOR_VALUE_PROP:
         bluetooth_indicator_set_value(self, g_value_get_double( value ));
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void bluetooth_indicator_get_property( GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
   BluetoothIndicator *self = (BluetoothIndicator *) object;

   switch (prop_id)
   {
      case BLUETOOTH_INDICATOR_VALUE_PROP:
         g_value_set_double(value, bluetooth_indicator_get_value(self));
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static GParamSpec *bluetooth_indicator_properties[N_BLUETOOTH_INDICATOR_PROPERTIES] = {NULL, };

static void bluetooth_indicator_class_init(BluetoothIndicatorClass *klass)
{
   logging_llprintf(LOGLEVEL_DEBUG, "%s", __func__);
   GObjectClass *object_class = G_OBJECT_CLASS(klass);

   GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

   widget_class->button_press_event = bluetooth_indicator_button_press;
   widget_class->realize = bluetooth_indicator_realize;
   widget_class->draw = bluetooth_indicator_draw;
   widget_class->size_allocate = bluetooth_indicator_size_allocate;

   object_class->set_property = bluetooth_indicator_set_property;
   object_class->get_property = bluetooth_indicator_get_property;
   bluetooth_indicator_properties[BLUETOOTH_INDICATOR_VALUE_PROP] = g_param_spec_double("value", NULL, NULL,
                                                                                    -G_MINDOUBLE, G_MAXDOUBLE,
                                                                                    0, G_PARAM_READWRITE);
   g_object_class_install_properties(object_class, N_BLUETOOTH_INDICATOR_PROPERTIES, bluetooth_indicator_properties);
}

//// Instance ////////////
static void bluetooth_indicator_init(BluetoothIndicator *bluet)
{
   bluet->value = 0.0;
   bluet->old_value = 0.0;
}

GtkWidget *bluetooth_indicator_new(void)
{
   return g_object_new(bluetooth_indicator_get_type(), NULL);
}

gdouble bluetooth_indicator_get_value(BluetoothIndicator *bluet)
{
   g_return_val_if_fail(BLUETOOTH_IS_INDICATOR(bluet), 0.0);
   return bluet->value;
}

void bluetooth_indicator_set_value(BluetoothIndicator *bluet, gdouble new_value)
{
   g_return_if_fail(BLUETOOTH_IS_INDICATOR(bluet));
   bluet->old_value = bluet->value;
   bluet->value = new_value;

   gtk_widget_queue_draw(GTK_WIDGET(bluet));
//   g_object_notify_by_pspec(G_OBJECT(bluet), bluetooth_indicator_properties[BLUETOOTH_INDICATOR_VALUE_PROP]);
}

static void bluetooth_indicator_realize(GtkWidget *widget)
{
   GdkWindowAttr  win_attr;
   GtkAllocation  alloc;
   gint           attr_mask;

   g_return_if_fail(widget != NULL);
   g_return_if_fail(BLUETOOTH_IS_INDICATOR(widget));

   gtk_widget_set_realized(widget, TRUE);
   gtk_widget_get_allocation(widget, &alloc);
   win_attr.x = alloc.x;
   win_attr.y = alloc.y;
   win_attr.width = alloc.width;
   win_attr.height = alloc.height;
   win_attr.wclass = GDK_INPUT_OUTPUT;
   win_attr.window_type = GDK_WINDOW_CHILD;
   win_attr.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK |
                         GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK;
   win_attr.visual = gtk_widget_get_visual(widget);
   attr_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;
   gtk_widget_set_window(widget, gdk_window_new(gtk_widget_get_parent_window(widget), &win_attr, attr_mask));
   gdk_window_set_user_data(gtk_widget_get_window(widget), widget);
}

static void bluetooth_indicator_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
//   BluetoothIndicator *bluet;
   g_return_if_fail(widget != NULL);
   g_return_if_fail(BLUETOOTH_IS_INDICATOR(widget));
   g_return_if_fail(allocation != NULL);

   gtk_widget_set_allocation(widget, allocation);

   if(gtk_widget_get_realized(widget))
   {
      gdk_window_move_resize(gtk_widget_get_window(widget),
                             allocation->x, allocation->y,
                             allocation->width, allocation->height);
   }
}

static gboolean bluetooth_indicator_draw(GtkWidget *widget, cairo_t *cr)
{
   BluetoothIndicator *bluet = BLUETOOTH_INDICATOR(widget);
   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(BLUETOOTH_IS_INDICATOR(widget), FALSE);
   g_return_val_if_fail(cr != NULL, FALSE);

   int width = gtk_widget_get_allocated_width(widget);
   int height = gtk_widget_get_allocated_height(widget);
   GtkStyleContext  *context = gtk_widget_get_style_context(widget);
   // gtk_render_background(context, cr, 0, 0, width, height);
   gtk_render_frame(context, cr, 0, 0, width, height);
   gtk_style_context_add_class(gtk_widget_get_style_context(widget), "bluetooth-indicator-class");

   int line_width = 1;
   cairo_set_source_rgba(cr, 0.0, 0.0, 255.0, 1.0);
   cairo_set_line_width(cr, line_width);
   float bm = 0.0f; // base margin
   float whr = 3.0f/5.0f; // width-to-height-ratio
   float padx, pady;

   float indicator_w = (float)width - 2*bm;
   float indicator_h = (float)height - 2*bm;

   // Trying to keep an aspect ratio to the indicator
   if ((indicator_w/indicator_h) >= whr)
   {
      padx = (indicator_w - indicator_h*whr) / 2.0f;
      pady = 0;
   }
   else
   {
      padx = 0;
      pady = (indicator_h - indicator_w/whr) / 2.0f;
   }

   float cl = (float)width/2.0f; // center line x
   float rad = cl - padx - bm;
   float Ax = padx + bm;
   float Ay = rad + pady + bm;
   float Bx = (float)width - padx - bm;
   float By = Ay;
   float Cx = Bx;
   float Cy = (float)height - rad - pady - bm;
   float Dx = Ax;
   float Dy = Cy;
   float ctx = cl;
   float cty = Ay;
   float cbx = ctx;
   float cby = Cy;

   // The "background"
   cairo_move_to(cr, ctx, cty);
   cairo_arc(cr, ctx, cty, rad, M_PI, 0);
   cairo_move_to(cr, cbx, cby);
   cairo_arc(cr, cbx, cby, rad, 0, M_PI);
   cairo_move_to(cr, Ax, Ay);
   cairo_line_to(cr, Bx, By);
   cairo_line_to(cr, Cx, Cy);
   cairo_line_to(cr, Dx, Dy);
   cairo_line_to(cr, Ax, Ay);
   cairo_fill(cr);

   float fm = ( (float)height - 2*pady - 2*bm) * 0.20f;
   float ih = (float)height - 2*pady - 2*bm - 2*fm; // inner height
   float hc = (float)height/2.0f; // height center

   cairo_set_source_rgba(cr, 255.0, 255, 255.0, 1.0);
   // Make the line width a factor of the fill margin
   cairo_set_line_width(cr, fm*0.2);
   cairo_move_to(cr, cl - (ih/4.0f), hc - (ih/4));
   cairo_line_to(cr, cl + (ih/4.0f), hc + (ih/4));
   cairo_line_to(cr, cl, hc + (ih/2.0f));
   cairo_line_to(cr, cl, hc - (ih/2.0f));
   cairo_line_to(cr, cl + (ih/4.0f), hc - (ih/4));
   cairo_line_to(cr, cl - (ih/4.0f), hc + (ih/4));
   cairo_stroke(cr);

   return FALSE;
}

static gboolean bluetooth_indicator_button_press(GtkWidget *widget, GdkEventButton *event)
{
   BluetoothIndicator *bluet;
   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(BLUETOOTH_IS_INDICATOR(widget), FALSE);
   g_return_val_if_fail(event != NULL, FALSE);
   bluet = BLUETOOTH_INDICATOR(widget);

   // g_signal_emit_by_name(G_OBJECT(bluet), "button-press-event");
   return TRUE;
}

