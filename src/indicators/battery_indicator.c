/**
 * Implementation
*/
#include <math.h>
#include <cairo/cairo.h>
#include <glib-object.h>
#include "battery_indicator.h"

#include "utils/logging.h"

struct _BatteryIndicator
{
   GtkWidget            parent;
   gdouble              value;         // the percent charge
   gdouble              old_value;     // to determine change?
};

// It appears that anything that isn't defined as a property is effectively private instance data?
enum {
   PROP_0,
   BATTERY_INDICATOR_VALUE_PROP,
   N_BATTERY_INDICATOR_PROPERTIES
};

G_DEFINE_TYPE(BatteryIndicator, battery_indicator, GTK_TYPE_WIDGET)

static void       battery_indicator_realize                 (GtkWidget *widget);
static void       battery_indicator_size_allocate           (GtkWidget *widget,
                                                             GtkAllocation *allocation);
static gboolean   battery_indicator_draw                    (GtkWidget *widget,
                                                             cairo_t *cr);
static gboolean   battery_indicator_button_press            (GtkWidget *widget,
                                                             GdkEventButton *event);
static void battery_indicator_set_property( GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
   BatteryIndicator *self = (BatteryIndicator *) object;
   switch (prop_id)
   {
      case BATTERY_INDICATOR_VALUE_PROP:
         battery_indicator_set_value(self, g_value_get_double( value ));
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void battery_indicator_get_property( GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
   BatteryIndicator *self = (BatteryIndicator *) object;

   switch (prop_id)
   {
      case BATTERY_INDICATOR_VALUE_PROP:
         g_value_set_double(value, battery_indicator_get_value(self));
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static GParamSpec *battery_indicator_properties[N_BATTERY_INDICATOR_PROPERTIES] = {NULL, };

static void battery_indicator_class_init(BatteryIndicatorClass *klass)
{
   logging_llprintf(LOGLEVEL_DEBUG, "%s", __func__);
   GObjectClass *object_class = G_OBJECT_CLASS(klass);

   GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

   widget_class->button_press_event = battery_indicator_button_press;
   widget_class->realize = battery_indicator_realize;
   widget_class->draw = battery_indicator_draw;
   widget_class->size_allocate = battery_indicator_size_allocate;

   object_class->set_property = battery_indicator_set_property;
   object_class->get_property = battery_indicator_get_property;
   battery_indicator_properties[BATTERY_INDICATOR_VALUE_PROP] = g_param_spec_double("value", NULL, NULL,
                                                                                    -G_MINDOUBLE, G_MAXDOUBLE,
                                                                                    0, G_PARAM_READWRITE);
   g_object_class_install_properties(object_class, N_BATTERY_INDICATOR_PROPERTIES, battery_indicator_properties);
}

//// Instance ////////////
static void battery_indicator_init(BatteryIndicator *batt)
{
   batt->value = 0.0;
   batt->old_value = 0.0;
}

GtkWidget *battery_indicator_new(void)
{
   return g_object_new(battery_indicator_get_type(), NULL);
}

gdouble battery_indicator_get_value(BatteryIndicator *batt)
{
   g_return_val_if_fail(BATTERY_IS_INDICATOR(batt), 0.0);
   return batt->value;
}

void battery_indicator_set_value(BatteryIndicator *batt, gdouble new_value)
{
   g_return_if_fail(BATTERY_IS_INDICATOR(batt));
   batt->old_value = batt->value;
   batt->value = new_value;

   gtk_widget_queue_draw(GTK_WIDGET(batt));
//   g_object_notify_by_pspec(G_OBJECT(batt), battery_indicator_properties[BATTERY_INDICATOR_VALUE_PROP]);
}

static void battery_indicator_realize(GtkWidget *widget)
{
   GdkWindowAttr  win_attr;
   GtkAllocation  alloc;
   gint           attr_mask;

   g_return_if_fail(widget != NULL);
   g_return_if_fail(BATTERY_IS_INDICATOR(widget));

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

static void battery_indicator_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
//   BatteryIndicator *batt;
   g_return_if_fail(widget != NULL);
   g_return_if_fail(BATTERY_IS_INDICATOR(widget));
   g_return_if_fail(allocation != NULL);

   gtk_widget_set_allocation(widget, allocation);

   if(gtk_widget_get_realized(widget))
   {
      gdk_window_move_resize(gtk_widget_get_window(widget),
                             allocation->x, allocation->y,
                             allocation->width, allocation->height);
   }
}

static gboolean battery_indicator_draw(GtkWidget *widget, cairo_t *cr)
{
   BatteryIndicator *batt = BATTERY_INDICATOR(widget);
   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(BATTERY_IS_INDICATOR(widget), FALSE);
   g_return_val_if_fail(cr != NULL, FALSE);

   int width = gtk_widget_get_allocated_width(widget);
   int height = gtk_widget_get_allocated_height(widget);
   GtkStyleContext  *context = gtk_widget_get_style_context(widget);
   gtk_style_context_add_class(gtk_widget_get_style_context(widget), "battery-indicator-class");
   gtk_render_background(context, cr, 0, 0, width, height);
   gtk_render_frame(context, cr, 0, 0, width, height);

   // The outline
   int marg = 2;
   int batt_top_height = 10;
   int batt_top_width = 15;
   int pad = 10;
   int line_width = 4;
   cairo_set_source_rgba(cr, 0.0, 255, 0.0, 1.0);
   cairo_set_line_width(cr, line_width);

   // Note to self, these are absolute coords of widget area, not relative movement of a pen.
   // you could use cairo_rel_line_to if we want to use relative from the move_to
   cairo_move_to(cr, 0 + marg,                        0 + marg);
   cairo_line_to(cr, width - marg,                    0 + marg);
   cairo_line_to(cr, width - marg,                    height - marg - batt_top_height);
   cairo_line_to(cr, width - marg - batt_top_width,   height - marg - batt_top_height);
   cairo_line_to(cr, width - marg - batt_top_width,   height - marg);
   cairo_line_to(cr, marg + batt_top_width,           height - marg);
   cairo_line_to(cr, marg + batt_top_width,           height - marg - batt_top_height);
   cairo_line_to(cr, 0 + marg,                        height - marg - batt_top_height);
   cairo_line_to(cr, 0 + marg,                        0 + marg);
   cairo_line_to(cr, 0 + marg + line_width,           0 + marg); // to eliminate a weird missing pixel in the last corner
   cairo_stroke(cr);

   // The "level"
   gdouble eff_w = height - 2*pad - batt_top_height;
   gdouble eff_v = eff_w * (batt->value/100);
   cairo_move_to(cr, pad,           pad);
   cairo_line_to(cr, pad,           eff_v + pad);
   cairo_line_to(cr, width - pad,   eff_v + pad);
   cairo_line_to(cr, width - pad,   pad);
   cairo_line_to(cr, pad,           pad);
   cairo_line_to(cr, pad,           pad);
   cairo_fill(cr);

   return FALSE;
}

static gboolean battery_indicator_button_press(GtkWidget *widget, GdkEventButton *event)
{
   BatteryIndicator *batt;
   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(BATTERY_IS_INDICATOR(widget), FALSE);
   g_return_val_if_fail(event != NULL, FALSE);
   batt = BATTERY_INDICATOR(widget);

   g_signal_emit_by_name(G_OBJECT(batt), "button-press-event");
   return TRUE;
}

