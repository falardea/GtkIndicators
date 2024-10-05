/**
 * Created by french on 9/28/24.
 * @brief
 */
#include <math.h>
#include <cairo/cairo.h>
#include <glib-object.h>
#include "tank_indicator.h"

#include "utils/logging.h"

struct _TankIndicator
{
   GtkWidget            parent;
   gdouble              value;         // the percent charge
   gdouble              old_value;     // to determine change?
   gboolean             vertical_orientation;
};

// It appears that anything that isn't defined as a property is effectively private instance data?
enum {
   PROP_0,
   TANK_INDICATOR_VALUE_PROP,
   N_TANK_INDICATOR_PROPERTIES
};

G_DEFINE_TYPE(TankIndicator, tank_indicator, GTK_TYPE_WIDGET)

static void       tank_indicator_realize                 (GtkWidget *widget);
static void       tank_indicator_size_allocate           (GtkWidget *widget,
                                                                   GtkAllocation *allocation);
static gboolean   tank_indicator_draw                    (GtkWidget *widget,
                                                                   cairo_t *cr);
static gboolean   tank_indicator_button_press            (GtkWidget *widget,
                                                                   GdkEventButton *event);
static void tank_indicator_set_property( GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
   TankIndicator *self = (TankIndicator *) object;
   switch (prop_id)
   {
      case TANK_INDICATOR_VALUE_PROP:
         tank_indicator_set_value(self, g_value_get_double( value ));
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void tank_indicator_get_property( GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
   TankIndicator *self = (TankIndicator *) object;

   switch (prop_id)
   {
      case TANK_INDICATOR_VALUE_PROP:
         g_value_set_double(value, tank_indicator_get_value(self));
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static GParamSpec *tank_indicator_properties[N_TANK_INDICATOR_PROPERTIES] = {NULL, };

static void tank_indicator_class_init(TankIndicatorClass *klass)
{
   logging_llprintf(LOGLEVEL_DEBUG, "%s", __func__);
   GObjectClass *object_class = G_OBJECT_CLASS(klass);

   GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

   widget_class->button_press_event = tank_indicator_button_press;
   widget_class->realize = tank_indicator_realize;
   widget_class->draw = tank_indicator_draw;
   widget_class->size_allocate = tank_indicator_size_allocate;

   object_class->set_property = tank_indicator_set_property;
   object_class->get_property = tank_indicator_get_property;
   tank_indicator_properties[TANK_INDICATOR_VALUE_PROP] = g_param_spec_double("value", NULL, NULL,
                                                                                                -G_MINDOUBLE, G_MAXDOUBLE,
                                                                                                0, G_PARAM_READWRITE);
   g_object_class_install_properties(object_class, N_TANK_INDICATOR_PROPERTIES, tank_indicator_properties);
}

//// Instance ////////////
static void tank_indicator_init(TankIndicator *tnk)
{
   tnk->value = 0.0;
   tnk->old_value = 0.0;
}

GtkWidget *tank_indicator_new(IndicatorLayout *layout, gboolean vertical)
{
   TankIndicator *tnk = g_object_new(tank_indicator_get_type(), NULL);
   tnk->vertical_orientation = vertical;

   gtk_widget_set_hexpand(GTK_WIDGET(tnk), layout->h_expand);
   gtk_widget_set_vexpand(GTK_WIDGET(tnk), layout->v_expand);
   gtk_widget_set_margin_start(GTK_WIDGET(tnk), layout->start);
   gtk_widget_set_margin_top(GTK_WIDGET(tnk), layout->top);
   gtk_widget_set_margin_end(GTK_WIDGET(tnk), layout->end);
   gtk_widget_set_margin_bottom(GTK_WIDGET(tnk), layout->bottom);

   return GTK_WIDGET(tnk);
}

gdouble tank_indicator_get_value(TankIndicator *tnk)
{
   g_return_val_if_fail(TANK_IS_INDICATOR(tnk), 0.0);
   return tnk->value;
}

void tank_indicator_set_value(TankIndicator *tnk, gdouble new_value)
{
   g_return_if_fail(TANK_IS_INDICATOR(tnk));
   tnk->old_value = tnk->value;
   tnk->value = new_value;

   gtk_widget_queue_draw(GTK_WIDGET(tnk));
//   g_object_notify_by_pspec(G_OBJECT(tnk), tank_indicator_properties[TANK_INDICATOR_VALUE_PROP]);
}

static void tank_indicator_realize(GtkWidget *widget)
{
   GdkWindowAttr  win_attr;
   GtkAllocation  alloc;
   gint           attr_mask;

   g_return_if_fail(widget != NULL);
   g_return_if_fail(TANK_IS_INDICATOR(widget));

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

static void tank_indicator_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
//   TankIndicator *tnk;
   g_return_if_fail(widget != NULL);
   g_return_if_fail(TANK_IS_INDICATOR(widget));
   g_return_if_fail(allocation != NULL);

   gtk_widget_set_allocation(widget, allocation);

   if(gtk_widget_get_realized(widget))
   {
      gdk_window_move_resize(gtk_widget_get_window(widget),
                             allocation->x, allocation->y,
                             allocation->width, allocation->height);
   }
}

static  float hypotenuse(float dx, float dy)
{
   return sqrtf(powf(dx,2) + powf(dy,2));
}

/**
 * @brief draw a "tank" in the o
 * @param w
 * @param h
 * @param vw_p
 * @param vh_p
 */
static void draw_tank_body(cairo_t *cr,
                           float canvas_width, float canvas_height,
                           float top, float bottom,
                           float left, float right,
                           float valve_w_pcnt, float valve_h_pcnt,
                           float lw,
                           gboolean vert)
{
   float valve_len, valve_span, radius;
   if (vert)
   {
      valve_len = valve_h_pcnt * (bottom - top);
      valve_span = valve_w_pcnt * (right - left);
      radius = (right-left)/2.0f;

      cairo_move_to(cr, left,top + radius + valve_len);
      cairo_arc(cr, (canvas_width/2), (top + radius + valve_len), radius, M_PI, 0);
      cairo_line_to(cr, right,bottom);
      cairo_line_to(cr, left,bottom);
      cairo_line_to(cr, left,top + radius + valve_len);
      cairo_stroke(cr);
      // valve
      cairo_set_line_width(cr, lw*2);
      cairo_move_to(cr, canvas_width/2, top+valve_len);
      cairo_line_to(cr, canvas_width/2, top);
      cairo_stroke(cr);
      cairo_set_line_width(cr, lw);
      cairo_move_to(cr, (canvas_width/2)-(valve_span/2), top);
      cairo_line_to(cr, (canvas_width/2)+(valve_span/2), top);
      cairo_stroke(cr);
   }
   else
   {
      valve_len = valve_h_pcnt * (right - left);
      valve_span = valve_w_pcnt * (bottom - top);
      radius = (bottom-top)/2.0f;

      cairo_move_to(cr, right - radius - valve_len,top);
      cairo_arc(cr, right - radius - valve_len, canvas_height/2, radius, 3*M_PI/2, M_PI/2);
      cairo_line_to(cr, left,bottom);
      cairo_line_to(cr, left,top);
      cairo_line_to(cr, right - radius - valve_len,top);
      cairo_stroke(cr);
      // valve
      cairo_set_line_width(cr, lw*2);
      cairo_move_to(cr, right - valve_len, canvas_height/2);
      cairo_line_to(cr, right, canvas_height/2);
      cairo_stroke(cr);
      cairo_set_line_width(cr, lw);
      cairo_move_to(cr, right, (canvas_height/2)-(valve_span));
      cairo_line_to(cr, right, (canvas_height/2)+(valve_span));
      cairo_stroke(cr);
   }

}
static gboolean tank_indicator_draw(GtkWidget *widget, cairo_t *cr)
{
   TankIndicator *tnk = TANK_INDICATOR(widget);
   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(TANK_IS_INDICATOR(widget), FALSE);
   g_return_val_if_fail(cr != NULL, FALSE);

   float width = (float)gtk_widget_get_allocated_width(widget);
   float height = (float)gtk_widget_get_allocated_height(widget);
   GtkStyleContext  *context = gtk_widget_get_style_context(widget);
   gtk_style_context_add_class(context, "inner-indicator-class");
   gtk_render_background(context, cr, 0, 0, width, height);
   gtk_render_frame(context, cr, 0, 0, width, height);

   float bm = 0.0f; // base margin
   float whr = tnk->vertical_orientation ? (3.0f/5.0f) : (5.0f/3.0f); // width-to-height-ratio
   float padx, pady;

   float w_marg = width - 2*bm;
   float h_marg = height - 2*bm;

   // Trying to keep an aspect ratio to the indicator
   if (((tnk->vertical_orientation) && ((w_marg/h_marg) >= whr)) ||
       (!(tnk->vertical_orientation) && ((h_marg/w_marg) <= 1.0f/whr)))
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
   float line_width = 0.04f * hypotenuse((w_marg - 2*padx), (h_marg- 2*pady));

   float body_left = padx + 0.5f*(width - w_marg + line_width);
   float body_top = pady + 0.5f*(height - h_marg + line_width);
   float body_right = 0.5f*(width + w_marg - line_width) - padx;
   float body_bottom = 0.5f*(height + h_marg - line_width) - pady;

   // The filling
   // Caution: as fill margins and line widths approach the inner width/height, things get wonky...
   // There's a pseudo-clamp on the filler-margin, but that could be reworked
   float fm = 1 * line_width;  // filler-margin
   float fl, rad;
   float valve_len, valve_span;
   float valve_pcnt = 0.10f;
   float valve_span_pcnt = 0.3f;
   cairo_set_source_rgba(cr, 0, 0, 255.0, 1.0);
   cairo_set_line_width(cr, line_width);

   if (tnk->vertical_orientation){
      if (2*fm > body_right-body_left){
         fm = (body_right-body_left - 0.01f*(body_right-body_left))/2;
      }
   } else if (2*fm > body_bottom-body_top)
   {
      fm = (body_bottom-body_top - 0.01f*(body_bottom-body_top))/2;
   }

   draw_tank_body(cr, width, height,
                  body_top, body_bottom,
                  body_left, body_right,
                  valve_span_pcnt, valve_pcnt,
                  line_width,
                  tnk->vertical_orientation);
   if (tnk->vertical_orientation)
   {
      valve_len = valve_pcnt * (body_bottom - body_top);
      valve_span = valve_span_pcnt * (body_right - body_left);
      rad = (body_right-body_left)/2.0f;
      // fill
      float inner_height = (body_bottom - body_top - valve_len);
      float fill_height = inner_height - 2*fm;
      float fill_rad = rad - fm;
      fl = fill_height * (float)tnk->value/100.0f;
      float fill_block_top = body_bottom - fm - (fill_height - fill_rad);

      if (tnk->value > 0) {
         if (fl <= (fill_height - fill_rad)) {
            // fill up to curved portion normally
            cairo_move_to(cr, body_left + fm, body_bottom - fm - fl);
            cairo_line_to(cr, body_right - fm, body_bottom - fm - fl);
            cairo_line_to(cr, body_right - fm, body_bottom - fm);
            cairo_line_to(cr, body_left + fm, body_bottom - fm);
            cairo_line_to(cr, body_left + fm, body_bottom - fm - fl);
            cairo_fill(cr);
         } else if (tnk->value < 100.0f) {
            // fill all up to the curved portion
            // float fill_block_top = body_bottom - fm - (fill_height - fill_rad);
            float fill_remain = fl - (fill_height - fill_rad);
            float beta = asinf(fill_remain / fill_rad);
            float arc_top_x = fill_rad * cosf(beta);
            cairo_move_to(cr, body_left + fm, body_bottom - fm);
            cairo_line_to(cr, body_left + fm, fill_block_top);
            cairo_arc(cr, (width / 2), (body_top + rad + valve_len), fill_rad, M_PI, M_PI + beta);
            cairo_line_to(cr, (width / 2) + arc_top_x, fill_block_top - fill_remain);
            cairo_arc(cr, (width / 2), (body_top + rad + valve_len), fill_rad, (3 * M_PI / 2) + ((M_PI / 2) - beta),
                      2 * M_PI);
            // cairo_line_to(cr, body_right - fm, fill_block_top);
            cairo_line_to(cr, body_right - fm, body_bottom - fm);
            cairo_line_to(cr, body_left + fm, body_bottom - fm);
            cairo_line_to(cr, body_left + fm, fill_block_top);
            cairo_fill(cr);
         } else {
            cairo_move_to(cr, body_left + fm, body_bottom - fm);
            cairo_line_to(cr, body_left + fm, fill_block_top);
            cairo_arc(cr, (width / 2), (body_top + rad + valve_len), fill_rad, M_PI, 2*M_PI);
            // cairo_line_to(cr, body_right - fm, fill_block_top);
            cairo_line_to(cr, body_right - fm, body_bottom - fm);
            cairo_line_to(cr, body_left + fm, body_bottom - fm);
            cairo_line_to(cr, body_left + fm, fill_block_top);
            cairo_fill(cr);
         }
      }
   }
   else
   {
      valve_len = valve_pcnt * (body_right  - body_left);
      valve_span = valve_span_pcnt * (body_bottom - body_top);
      rad = (body_bottom-body_top)/2.0f;
      // fill
      float inner_height = (body_right - body_left - valve_len);
      float fill_height = inner_height - 2*fm;
      float fill_rad = rad - fm;
      fl = fill_height * (float)tnk->value/100.0f;
      float fill_block_right = body_left + fm + (fill_height - fill_rad);

      if (tnk->value > 0) {
         if (fl <= (fill_height - fill_rad)) {
            // fill up to curved portion normally
            cairo_move_to(cr, body_left + fm, body_top + fm);
            cairo_line_to(cr, body_left + fm + fl, body_top + fm);
            cairo_line_to(cr, body_left + fm + fl, body_bottom - fm);
            cairo_line_to(cr, body_left + fm, body_bottom - fm);
            cairo_line_to(cr, body_left + fm, body_top + fm);
            cairo_fill(cr);
         } else if (tnk->value < 100.0f) {
            // fill all up to the curved portion
            // float fill_block_top = body_bottom - fm - (fill_height - fill_rad);
            float fill_remain = fl - (fill_height - fill_rad);
            float beta = asinf(fill_remain / fill_rad);
            float arc_top_y = fill_rad * cosf(beta);
            cairo_move_to(cr, body_left + fm, body_top + fm);
            cairo_line_to(cr, fill_block_right, body_top + fm);
            cairo_arc(cr, fill_block_right, height/2, fill_rad, ((3*M_PI)/2), ((3*M_PI)/2) + beta);
            cairo_line_to(cr, body_left + fm + fl, (height/2)+arc_top_y);
            cairo_arc(cr, fill_block_right, height/2, fill_rad, (M_PI/2) - beta,M_PI/2);
            cairo_line_to(cr, body_left + fm, body_bottom - fm);
            cairo_line_to(cr, body_left + fm, body_top + fm);
            cairo_fill(cr);
         } else {
            cairo_move_to(cr, body_left + fm, body_top + fm);
            cairo_line_to(cr, fill_block_right, body_top + fm);
            cairo_arc(cr, fill_block_right, height/2, fill_rad, (3*M_PI/2), M_PI/2);
            cairo_line_to(cr, body_left + fm, body_bottom - fm);
            cairo_line_to(cr, body_left + fm, body_top + fm);
            cairo_fill(cr);
         }
      }
   }

   return FALSE;
}

static gboolean tank_indicator_button_press(GtkWidget *widget, GdkEventButton *event)
{
   TankIndicator *tnk;
   g_return_val_if_fail(widget != NULL, FALSE);
   g_return_val_if_fail(TANK_IS_INDICATOR(widget), FALSE);
   g_return_val_if_fail(event != NULL, FALSE);
   tnk = TANK_INDICATOR(widget);

   logging_llprintf(LOGLEVEL_DEBUG, "%s", __func__ );
   g_signal_emit_by_name(G_OBJECT(tnk), "button-press-event");
   return TRUE;
}

