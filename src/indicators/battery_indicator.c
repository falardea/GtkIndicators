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
   gboolean             vertical_orientation;
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

GtkWidget *battery_indicator_new(IndicatorLayout *layout, gboolean vertical)
{
   BatteryIndicator *batt = g_object_new(battery_indicator_get_type(), NULL);
   batt->vertical_orientation = vertical;
   gtk_widget_set_hexpand(GTK_WIDGET(batt), layout->h_expand);
   gtk_widget_set_vexpand(GTK_WIDGET(batt), layout->v_expand);
   gtk_widget_set_margin_start(GTK_WIDGET(batt), layout->start);
   gtk_widget_set_margin_top(GTK_WIDGET(batt), layout->top);
   gtk_widget_set_margin_end(GTK_WIDGET(batt), layout->end);
   gtk_widget_set_margin_bottom(GTK_WIDGET(batt), layout->bottom);
   return GTK_WIDGET(batt);
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

   float width = (float)gtk_widget_get_allocated_width(widget);
   float height = (float)gtk_widget_get_allocated_height(widget);
   GtkStyleContext  *context = gtk_widget_get_style_context(widget);
   gtk_style_context_add_class(context, "inner-indicator-class");
   gtk_render_background(context, cr, 0, 0, width, height);
   gtk_render_frame(context, cr, 0, 0, width, height);

   float bm = 0.0f; // base margin
   float whr = batt->vertical_orientation ? (3.0f/4.0f) : (4.0f/3.0f); // width-to-height-ratio
   float padx, pady;

   float w_marg = width - 2*bm;
   float h_marg = height - 2*bm;

   // Trying to keep an aspect ratio to the indicator
   if (((batt->vertical_orientation) && ((w_marg/h_marg) >= whr)) ||
       (!(batt->vertical_orientation) && ((h_marg/w_marg) <= 1.0f/whr)))
   {
      padx = (w_marg - h_marg*whr) / 2.0f;
      pady = 0;
   }
   else
   {
      padx = 0;
      pady = (h_marg - w_marg/whr) / 2.0f;
   }

   // Make the line width proportional to the hypotenuse of the indicator content area
   float line_width = 0.04f * sqrtf(powf((w_marg - 2*padx),2) + powf((h_marg- 2*pady),2));

   cairo_set_source_rgba(cr, 0, 255, 0, 1.0);
   cairo_set_line_width(cr, line_width);

   float body_left = padx + 0.5f*(width - w_marg + line_width);
   float body_bottom = 0.5f*(height + h_marg - line_width) - pady;
   float top_protrusion = 3*line_width;
   float body_top, body_right, top_span;
   if (batt->vertical_orientation)
   {
      body_top = pady + 0.5f*(height - h_marg + line_width) + top_protrusion;
      body_right = 0.5f*(width + w_marg - line_width) - padx;
      // "Top"
      top_span = 0.5f * (body_right - body_left);
      cairo_move_to(cr, (width/2) - (top_span/2),body_top);
      cairo_line_to(cr, (width/2) - (top_span/2),(body_top - top_protrusion));
      cairo_line_to(cr, (width/2) + (top_span/2),(body_top - top_protrusion));
      cairo_line_to(cr, (width/2) + (top_span/2),body_top);
      cairo_line_to(cr, (width/2) - (top_span/2),body_top);
      cairo_fill(cr);
   }
   else
   {
      body_top = pady + 0.5f*(height - h_marg + line_width);
      body_right = 0.5f*(width + w_marg - line_width) - padx - top_protrusion;
      // "Top"
      top_span = 0.5f * (body_bottom - body_top);
      cairo_move_to(cr, body_right,(height/2) - (top_span/2));
      cairo_line_to(cr, (body_right + top_protrusion),(height/2) - (top_span/2));
      cairo_line_to(cr, (body_right + top_protrusion),(height/2) + (top_span/2));
      cairo_line_to(cr, body_right,(height/2) + (top_span/2));
      cairo_line_to(cr, body_right,(height/2) - (top_span/2));
      cairo_fill(cr);
   }

   cairo_move_to(cr, body_left,body_top);
   cairo_line_to(cr, body_right,body_top);
   cairo_line_to(cr, body_right,body_bottom);
   cairo_line_to(cr, body_left,body_bottom);
   // Subtracting 1/2 the line width on the closing height to make sure it "closes" properly
   // otherwise you end up with a 1/2 line-width square in the start/final corner
   cairo_line_to(cr, body_left,body_top - line_width/2);
   cairo_stroke(cr);

   // The filling
   float fm = line_width;  // filler-margin
   float fl; // fill-level

   float fill_left = body_left + fm;
   float fill_top = body_top + fm;
   float fill_right = body_right - fm;
   float fill_bottom = body_bottom - fm;
   if (batt->vertical_orientation)
   {
      fl = ((body_bottom-body_top) - 2.0f*fm) * (float)(batt->value/100.0f);
      cairo_move_to(cr, fill_left, fill_bottom - fl);
      cairo_line_to(cr, fill_right, fill_bottom - fl);
      cairo_line_to(cr, fill_right, fill_bottom);
      cairo_line_to(cr, fill_left, fill_bottom);
      cairo_line_to(cr, fill_left, fill_bottom - fl);
      cairo_fill(cr);
   }
   else
   {
      fl = ((body_right-body_left) - 2.0f*fm) * (float)(batt->value/100.0f);
      cairo_move_to(cr, fill_left, fill_top);
      cairo_line_to(cr, fill_left + fl, fill_top);
      cairo_line_to(cr, fill_left + fl, fill_bottom);
      cairo_line_to(cr, fill_left, fill_bottom);
      cairo_line_to(cr, fill_left, fill_top);
      cairo_fill(cr);
   }
   return FALSE;
}

static gboolean battery_indicator_button_press(GtkWidget *widget, GdkEventButton *event)
{
   BatteryIndicator *batt;
   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(BATTERY_IS_INDICATOR(widget), FALSE);
   g_return_val_if_fail(event != NULL, FALSE);
   batt = BATTERY_INDICATOR(widget);

   logging_llprintf(LOGLEVEL_DEBUG, "%s", __func__ );
   g_signal_emit_by_name(G_OBJECT(batt), "button-press-event");
   return TRUE;
}

