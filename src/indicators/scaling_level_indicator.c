/**
 * Created by french on 9/28/24.
 * @brief
 */
#include <math.h>
#include <cairo/cairo.h>
#include <glib-object.h>
#include "scaling_level_indicator.h"

#include "utils/logging.h"

struct _ScalingLevelIndicator
{
   GtkWidget            parent;
   gdouble              value;         // the percent charge
   gdouble              old_value;     // to determine change?
   gboolean             vertical_orientation;
};

// It appears that anything that isn't defined as a property is effectively private instance data?
enum {
   PROP_0,
   SCALING_LEVEL_INDICATOR_VALUE_PROP,
   N_SCALING_LEVEL_INDICATOR_PROPERTIES
};

G_DEFINE_TYPE(ScalingLevelIndicator, scaling_level_indicator, GTK_TYPE_WIDGET)

static void       scaling_level_indicator_realize                 (GtkWidget *widget);
static void       scaling_level_indicator_size_allocate           (GtkWidget *widget,
                                                                 GtkAllocation *allocation);
static gboolean   scaling_level_indicator_draw                    (GtkWidget *widget,
                                                                 cairo_t *cr);
static gboolean   scaling_level_indicator_button_press            (GtkWidget *widget,
                                                                 GdkEventButton *event);
static void scaling_level_indicator_set_property( GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
   ScalingLevelIndicator *self = (ScalingLevelIndicator *) object;
   switch (prop_id)
   {
      case SCALING_LEVEL_INDICATOR_VALUE_PROP:
         scaling_level_indicator_set_value(self, g_value_get_double( value ));
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void scaling_level_indicator_get_property( GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
   ScalingLevelIndicator *self = (ScalingLevelIndicator *) object;

   switch (prop_id)
   {
      case SCALING_LEVEL_INDICATOR_VALUE_PROP:
         g_value_set_double(value, scaling_level_indicator_get_value(self));
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static GParamSpec *scaling_level_indicator_properties[N_SCALING_LEVEL_INDICATOR_PROPERTIES] = {NULL, };

static void scaling_level_indicator_class_init(ScalingLevelIndicatorClass *klass)
{
   logging_llprintf(LOGLEVEL_DEBUG, "%s", __func__);
   GObjectClass *object_class = G_OBJECT_CLASS(klass);

   GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

   widget_class->button_press_event = scaling_level_indicator_button_press;
   widget_class->realize = scaling_level_indicator_realize;
   widget_class->draw = scaling_level_indicator_draw;
   widget_class->size_allocate = scaling_level_indicator_size_allocate;

   object_class->set_property = scaling_level_indicator_set_property;
   object_class->get_property = scaling_level_indicator_get_property;
   scaling_level_indicator_properties[SCALING_LEVEL_INDICATOR_VALUE_PROP] = g_param_spec_double("value", NULL, NULL,
                                                                                            -G_MINDOUBLE, G_MAXDOUBLE,
                                                                                            0, G_PARAM_READWRITE);
   g_object_class_install_properties(object_class, N_SCALING_LEVEL_INDICATOR_PROPERTIES, scaling_level_indicator_properties);
}

//// Instance ////////////
static void scaling_level_indicator_init(ScalingLevelIndicator *sli)
{
   sli->value = 0.0;
   sli->old_value = 0.0;
}

GtkWidget *scaling_level_indicator_new(IndicatorLayout *layout, gboolean vertical)
{
   ScalingLevelIndicator *sli = g_object_new(scaling_level_indicator_get_type(), NULL);
   sli->vertical_orientation = vertical;
   gtk_widget_set_hexpand(GTK_WIDGET(sli), layout->h_expand);
   gtk_widget_set_vexpand(GTK_WIDGET(sli), layout->v_expand);
   gtk_widget_set_margin_start(GTK_WIDGET(sli), layout->start);
   gtk_widget_set_margin_top(GTK_WIDGET(sli), layout->top);
   gtk_widget_set_margin_end(GTK_WIDGET(sli), layout->end);
   gtk_widget_set_margin_bottom(GTK_WIDGET(sli), layout->bottom);
   return GTK_WIDGET(sli);
}

gdouble scaling_level_indicator_get_value(ScalingLevelIndicator *sli)
{
   g_return_val_if_fail(SCALING_LEVEL_IS_INDICATOR(sli), 0.0);
   return sli->value;
}

void scaling_level_indicator_set_value(ScalingLevelIndicator *sli, gdouble new_value)
{
   g_return_if_fail(SCALING_LEVEL_IS_INDICATOR(sli));
   sli->old_value = sli->value;
   sli->value = new_value;

   gtk_widget_queue_draw(GTK_WIDGET(sli));
//   g_object_notify_by_pspec(G_OBJECT(sli), scaling_level_indicator_properties[SCALING_LEVEL_INDICATOR_VALUE_PROP]);
}

static void scaling_level_indicator_realize(GtkWidget *widget)
{
   GdkWindowAttr  win_attr;
   GtkAllocation  alloc;
   gint           attr_mask;

   g_return_if_fail(widget != NULL);
   g_return_if_fail(SCALING_LEVEL_IS_INDICATOR(widget));

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

static void scaling_level_indicator_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
//   ScalingLevelIndicator *sli;
   g_return_if_fail(widget != NULL);
   g_return_if_fail(SCALING_LEVEL_IS_INDICATOR(widget));
   g_return_if_fail(allocation != NULL);

   gtk_widget_set_allocation(widget, allocation);

   if(gtk_widget_get_realized(widget))
   {
      gdk_window_move_resize(gtk_widget_get_window(widget),
                             allocation->x, allocation->y,
                             allocation->width, allocation->height);
   }
}

static gboolean scaling_level_indicator_draw(GtkWidget *widget, cairo_t *cr)
{
   ScalingLevelIndicator *sli = SCALING_LEVEL_INDICATOR(widget);
   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(SCALING_LEVEL_IS_INDICATOR(widget), FALSE);
   g_return_val_if_fail(cr != NULL, FALSE);

   float width = (float)gtk_widget_get_allocated_width(widget);
   float height = (float)gtk_widget_get_allocated_height(widget);
   GtkStyleContext  *context = gtk_widget_get_style_context(widget);
   gtk_style_context_add_class(context, "inner-indicator-class");
   gtk_render_background(context, cr, 0, 0, width, height);
   gtk_render_frame(context, cr, 0, 0, width, height);

   float bm = 0.0f; // base margin
   float whr = sli->vertical_orientation ? (3.0f/4.0f) : (4.0f/3.0f); // width-to-height-ratio
   float padx, pady;

   float w_marg = width - 2*bm;
   float h_marg = height - 2*bm;

   // Trying to keep an aspect ratio to the indicator
   if (((sli->vertical_orientation) && ((w_marg/h_marg) >= whr)) ||
       (!(sli->vertical_orientation) && ((h_marg/w_marg) <= 1.0f/whr)))
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

   float body_left = padx + 0.5f*(width - w_marg + line_width);
   float body_top = pady + 0.5f*(height - h_marg + line_width);
   float body_right = 0.5f*(width + w_marg - line_width) - padx;
   float body_bottom = 0.5f*(height + h_marg - line_width) - pady;

   cairo_set_source_rgba(cr, 255.0, 255, 255.0, 1.0);
   cairo_set_line_width(cr, line_width);
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
   if (sli->vertical_orientation)
   {
      fl = ((body_bottom-body_top) - 2.0f*fm) * (float)(sli->value/100.0f);
      cairo_move_to(cr, fill_left, fill_bottom - fl);
      cairo_line_to(cr, fill_right, fill_bottom - fl);
      cairo_line_to(cr, fill_right, fill_bottom);
      cairo_line_to(cr, fill_left, fill_bottom);
      cairo_line_to(cr, fill_left, fill_bottom - fl);
      cairo_fill(cr);
   }
   else
   {
      fl = ((body_right-body_left) - 2.0f*fm) * (float)(sli->value/100.0f);
      cairo_move_to(cr, fill_left, fill_top);
      cairo_line_to(cr, fill_left + fl, fill_top);
      cairo_line_to(cr, fill_left + fl, fill_bottom);
      cairo_line_to(cr, fill_left, fill_bottom);
      cairo_line_to(cr, fill_left, fill_top);
      cairo_fill(cr);
   }
   return FALSE;
}

static gboolean scaling_level_indicator_button_press(GtkWidget *widget, GdkEventButton *event)
{
   ScalingLevelIndicator *sli;
   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(SCALING_LEVEL_IS_INDICATOR(widget), FALSE);
   g_return_val_if_fail(event != NULL, FALSE);
   sli = SCALING_LEVEL_INDICATOR(widget);

   logging_llprintf(LOGLEVEL_DEBUG, "%s", __func__ );
   g_signal_emit_by_name(G_OBJECT(sli), "button-press-event");
   return TRUE;
}

