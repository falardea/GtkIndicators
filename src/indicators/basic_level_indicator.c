/**
 * Created by french on 9/22/24.
 */
#include <math.h>
#include <cairo/cairo.h>
#include <glib-object.h>
#include "basic_level_indicator.h"

#include "utils/logging.h"

struct _BasicLevelIndicator
{
   GtkWidget            parent;
   gdouble              value;         // the percent charge
   gdouble              old_value;     // to determine change?
   gboolean             vertical_orientation;
};

// It appears that anything that isn't defined as a property is effectively private instance data?
enum {
   PROP_0,
   BASIC_LEVEL_INDICATOR_VALUE_PROP,
   N_BASIC_LEVEL_INDICATOR_PROPERTIES
};

G_DEFINE_TYPE(BasicLevelIndicator, basic_level_indicator, GTK_TYPE_WIDGET)

static void       basic_level_indicator_realize                 (GtkWidget *widget);
static void       basic_level_indicator_size_allocate           (GtkWidget *widget,
                                                             GtkAllocation *allocation);
static gboolean   basic_level_indicator_draw                    (GtkWidget *widget,
                                                             cairo_t *cr);
static gboolean   basic_level_indicator_button_press            (GtkWidget *widget,
                                                             GdkEventButton *event);
static void basic_level_indicator_set_property( GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
   BasicLevelIndicator *self = (BasicLevelIndicator *) object;
   switch (prop_id)
   {
      case BASIC_LEVEL_INDICATOR_VALUE_PROP:
         basic_level_indicator_set_value(self, g_value_get_double( value ));
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void basic_level_indicator_get_property( GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
   BasicLevelIndicator *self = (BasicLevelIndicator *) object;

   switch (prop_id)
   {
      case BASIC_LEVEL_INDICATOR_VALUE_PROP:
         g_value_set_double(value, basic_level_indicator_get_value(self));
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static GParamSpec *basic_level_indicator_properties[N_BASIC_LEVEL_INDICATOR_PROPERTIES] = {NULL, };

static void basic_level_indicator_class_init(BasicLevelIndicatorClass *klass)
{
   logging_llprintf(LOGLEVEL_DEBUG, "%s", __func__);
   GObjectClass *object_class = G_OBJECT_CLASS(klass);

   GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

   widget_class->button_press_event = basic_level_indicator_button_press;
   widget_class->realize = basic_level_indicator_realize;
   widget_class->draw = basic_level_indicator_draw;
   widget_class->size_allocate = basic_level_indicator_size_allocate;

   object_class->set_property = basic_level_indicator_set_property;
   object_class->get_property = basic_level_indicator_get_property;
   basic_level_indicator_properties[BASIC_LEVEL_INDICATOR_VALUE_PROP] = g_param_spec_double("value", NULL, NULL,
                                                                                    -G_MINDOUBLE, G_MAXDOUBLE,
                                                                                    0, G_PARAM_READWRITE);
   g_object_class_install_properties(object_class, N_BASIC_LEVEL_INDICATOR_PROPERTIES, basic_level_indicator_properties);
}

//// Instance ////////////
static void basic_level_indicator_init(BasicLevelIndicator *bli)
{
   bli->value = 0.0;
   bli->old_value = 0.0;
}

GtkWidget *basic_level_indicator_new(gboolean vertical)
{
   BasicLevelIndicator *bli = g_object_new(basic_level_indicator_get_type(), NULL);
   bli->vertical_orientation = vertical;

   return GTK_WIDGET(bli);
}

gdouble basic_level_indicator_get_value(BasicLevelIndicator *bli)
{
   g_return_val_if_fail(BASIC_LEVEL_IS_INDICATOR(bli), 0.0);
   return bli->value;
}

void basic_level_indicator_set_value(BasicLevelIndicator *bli, gdouble new_value)
{
   g_return_if_fail(BASIC_LEVEL_IS_INDICATOR(bli));
   bli->old_value = bli->value;
   bli->value = new_value;

   gtk_widget_queue_draw(GTK_WIDGET(bli));
//   g_object_notify_by_pspec(G_OBJECT(bli), basic_level_indicator_properties[BASIC_LEVEL_INDICATOR_VALUE_PROP]);
}

static void basic_level_indicator_realize(GtkWidget *widget)
{
   GdkWindowAttr  win_attr;
   GtkAllocation  alloc;
   gint           attr_mask;

   g_return_if_fail(widget != NULL);
   g_return_if_fail(BASIC_LEVEL_IS_INDICATOR(widget));

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

static void basic_level_indicator_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
//   BasicLevelIndicator *bli;
   g_return_if_fail(widget != NULL);
   g_return_if_fail(BASIC_LEVEL_IS_INDICATOR(widget));
   g_return_if_fail(allocation != NULL);

   gtk_widget_set_allocation(widget, allocation);

   if(gtk_widget_get_realized(widget))
   {
      gdk_window_move_resize(gtk_widget_get_window(widget),
                             allocation->x, allocation->y,
                             allocation->width, allocation->height);
   }
}

static gboolean basic_level_indicator_draw(GtkWidget *widget, cairo_t *cr)
{
   BasicLevelIndicator *bli = BASIC_LEVEL_INDICATOR(widget);
   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(BASIC_LEVEL_IS_INDICATOR(widget), FALSE);
   g_return_val_if_fail(cr != NULL, FALSE);

   float width = (float)gtk_widget_get_allocated_width(widget);
   float height = (float)gtk_widget_get_allocated_height(widget);
   GtkStyleContext  *context = gtk_widget_get_style_context(widget);
   gtk_style_context_add_class(context, "inner-indicator-class");
   gtk_render_background(context, cr, 0, 0, width, height);
   gtk_render_frame(context, cr, 0, 0, width, height);

   float line_width = 1;
   cairo_set_source_rgba(cr, 0.0, 255, 0.0, 1.0);
   cairo_set_line_width(cr, line_width);

   float bm = 0.0f; // base margin
   float whr = bli->vertical_orientation ? (3.0f/4.0f) : (4.0f/3.0f); // width-to-height-ratio
   float padx, pady;

   float w_marg = width - 2*bm;
   float h_marg = height - 2*bm;

   // Trying to keep an aspect ratio to the indicator
   if (((bli->vertical_orientation) && ((w_marg/h_marg) >= whr)) ||
      (!(bli->vertical_orientation) && ((h_marg/w_marg) <= 1.0f/whr)))
   {
      padx = (w_marg - h_marg*whr) / 2.0f;
      pady = 0;
   }
   else
   {
      padx = 0;
      pady = (h_marg - w_marg/whr) / 2.0f;
   }

   float body_left = 0 + bm + padx;
   float body_top = 0 + bm + pady;
   float body_right = width - bm - padx;
   float body_bottom = height - bm - pady;

   cairo_move_to(cr, body_left,body_top);
   cairo_line_to(cr, body_right,body_top);
   cairo_line_to(cr, body_right,body_bottom);
   cairo_line_to(cr, body_left,body_bottom);
   // Subtracting 1/2 the line width on the closing height to make sure it closes properly
   cairo_line_to(cr, body_left,body_top - line_width/2);
   cairo_stroke(cr);

   // The "level"
   float fm = line_width;  // filler-margin
   float fl; // fill-level

   float fill_left = body_left + fm;
   float fill_top = body_top + fm;
   float fill_right = body_right - fm;
   float fill_bottom = body_bottom - fm;
   if (bli->vertical_orientation)
   {
      fl = ((body_bottom-body_top) - 2.0f*fm) * (float)(bli->value/100.0f);
      cairo_move_to(cr, fill_left, fill_bottom - fl);
      cairo_line_to(cr, fill_right, fill_bottom - fl);
      cairo_line_to(cr, fill_right, fill_bottom);
      cairo_line_to(cr, fill_left, fill_bottom);
      cairo_line_to(cr, fill_left, fill_bottom - fl);
      cairo_fill(cr);
   }
   else
   {
      fl = ((body_right-body_left) - 2.0f*fm) * (float)(bli->value/100.0f);
      cairo_move_to(cr, fill_left, fill_top);
      cairo_line_to(cr, fill_left + fl, fill_top);
      cairo_line_to(cr, fill_left + fl, fill_bottom);
      cairo_line_to(cr, fill_left, fill_bottom);
      cairo_line_to(cr, fill_left, fill_top);
      cairo_fill(cr);
   }
   return FALSE;
}

static gboolean basic_level_indicator_button_press(GtkWidget *widget, GdkEventButton *event)
{
   BasicLevelIndicator *bli;
   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(BASIC_LEVEL_IS_INDICATOR(widget), FALSE);
   g_return_val_if_fail(event != NULL, FALSE);
   bli = BASIC_LEVEL_INDICATOR(widget);

   logging_llprintf(LOGLEVEL_DEBUG, "%s", __func__ );
   g_signal_emit_by_name(G_OBJECT(bli), "button-press-event");
   return TRUE;
}

