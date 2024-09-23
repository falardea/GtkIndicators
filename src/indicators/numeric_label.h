/*
 * Created by french on 7/5/24.
*/
#ifndef NUMERIC_LABEL_H__
#define NUMERIC_LABEL_H__
#include "glib.h"
G_BEGIN_DECLS
#include <gtk/gtk.h>
#include <glib-object.h>

#define NUMERIC_TYPE_LABEL ( numeric_label_get_type () )

G_DECLARE_FINAL_TYPE(NumericLabel, numeric_label, NUMERIC, LABEL, GtkBin)

GtkWidget *numeric_label_new(gdouble init_value, guint precision);

gdouble numeric_label_get_value(NumericLabel *self);
void numeric_label_set_value(NumericLabel *self, gdouble new_value);

guint numeric_label_get_format_precision(NumericLabel *self);
void numeric_label_set_format_precision(NumericLabel *self, guint precision);

G_END_DECLS
#endif  // NUMERIC_LABEL_H__
