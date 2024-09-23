/*
 * Created by french on 7/5/24.
*/
#include <math.h>
#include "numeric_label.h"
#include "utils/logging.h"

struct _NumericLabel
{
   GtkBin      parent;
   GtkLabel    *label_ref;

   gdouble     value;
   guint       format_precision;
};

enum {
   PROP_O,
   NUMERIC_LABEL_VALUE_PROP,
   NUMERIC_LABEL_PROP_FORMAT_PRECISION,
   N_NUMERIC_LABEL_PROPERTIES
};

G_DEFINE_TYPE(NumericLabel, numeric_label, GTK_TYPE_BIN )

static void numeric_label_finalize( GObject *object );

static void numeric_label_set_property( GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
   NumericLabel *self = NUMERIC_LABEL(object);
   switch(prop_id){
      case NUMERIC_LABEL_VALUE_PROP:
         numeric_label_set_value(self, g_value_get_double( value ) );
         break;
      case NUMERIC_LABEL_PROP_FORMAT_PRECISION:
         numeric_label_set_format_precision(self, g_value_get_uint( value ) );
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
   }
}

static void numeric_label_get_property( GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
   NumericLabel *self = NUMERIC_LABEL(object);
   switch(prop_id)
   {
      case NUMERIC_LABEL_VALUE_PROP:
         g_value_set_double( value, numeric_label_get_value(self));
         break;
      case NUMERIC_LABEL_PROP_FORMAT_PRECISION:
         g_value_set_uint( value, numeric_label_get_format_precision(self));
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
   }
}

static GParamSpec *numeric_properties[N_NUMERIC_LABEL_PROPERTIES] = {NULL, };

static void numeric_label_class_init(NumericLabelClass *klass)
{
   logging_llprintf(LOGLEVEL_DEBUG, "%s", __func__);

   GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
   GtkWidgetClass *widget_class = (GtkWidgetClass *) klass;

   gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS (klass), "/resource_path/resources/numeric_label.glade");
   gtk_widget_class_bind_template_child(widget_class, NumericLabel , label_ref);

   gobject_class->finalize = numeric_label_finalize;
   gobject_class->get_property = numeric_label_get_property;
   gobject_class->set_property = numeric_label_set_property;
   numeric_properties[NUMERIC_LABEL_VALUE_PROP] = g_param_spec_double("value", NULL, NULL, -G_MINDOUBLE, G_MAXDOUBLE, 0.0, G_PARAM_READWRITE );
   numeric_properties[NUMERIC_LABEL_PROP_FORMAT_PRECISION] = g_param_spec_uint("format_precision", NULL, NULL, 0, G_MAXINT,0, G_PARAM_READWRITE);
   g_object_class_install_properties(gobject_class, N_NUMERIC_LABEL_PROPERTIES, numeric_properties);
}

//// Instance ////////////
static void numeric_label_init(NumericLabel *self)
{
   logging_llprintf(LOGLEVEL_DEBUG, "%s", __func__);
   gtk_widget_init_template(GTK_WIDGET(self));
   numeric_label_set_value(self, 0.0);
   numeric_label_set_format_precision(self, 0);
}

GtkWidget *numeric_label_new(gdouble init_value, guint precision)
{
   NumericLabel *label;
   label = g_object_new(numeric_label_get_type(), NULL);

   numeric_label_set_format_precision(label, precision);

   // Always set the initial value AFTER setting the format
   if (!isnan(init_value))
   {
      numeric_label_set_value(label, init_value);
   }

   return GTK_WIDGET(label);
}

static void numeric_label_finalize( GObject *object )
{
   g_return_if_fail(object != NULL);
   g_return_if_fail(NUMERIC_IS_LABEL(object));

   logging_llprintf(LOGLEVEL_DEBUG, "%s", __func__);
   G_OBJECT_CLASS(numeric_label_parent_class)->finalize (object);
}

gdouble numeric_label_get_value(NumericLabel *self)
{
   g_return_val_if_fail(NUMERIC_IS_LABEL(self), NAN);
   return self->value;
}
void numeric_label_set_value(NumericLabel *self, gdouble new_value)
{
   g_return_if_fail(NUMERIC_IS_LABEL(self));
   self->value = new_value;

   char value_str[28];
   snprintf(value_str, sizeof (value_str), "%.*f", self->format_precision, self->value);
   gtk_label_set_label(GTK_LABEL(self->label_ref), value_str);

   // In case there's something listening/bound?
   g_object_notify_by_pspec(G_OBJECT(self), numeric_properties[NUMERIC_LABEL_VALUE_PROP]);
}

guint numeric_label_get_format_precision(NumericLabel *self)
{
   g_return_val_if_fail(NUMERIC_IS_LABEL(self), 0);
   return self->format_precision;
}
void numeric_label_set_format_precision(NumericLabel *self, guint precision)
{
   g_return_if_fail(self != NULL);
   g_return_if_fail(NUMERIC_IS_LABEL(self));

   self->format_precision = precision;
}
