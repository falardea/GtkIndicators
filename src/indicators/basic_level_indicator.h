/**
 * Created by french on 9/22/24.
*/
#ifndef BASIC_LEVEL_INDICATOR_H__
#define BASIC_LEVEL_INDICATOR_H__
#include "glib.h"
G_BEGIN_DECLS
#include <gtk/gtk.h>
#include <glib-object.h>

#define BASIC_LEVEL_TYPE_INDICATOR ( basic_level_indicator_get_type () )

G_DECLARE_FINAL_TYPE(BasicLevelIndicator, basic_level_indicator, BASIC_LEVEL, INDICATOR, GtkWidget)

GtkWidget *basic_level_indicator_new(gboolean vertical);

gdouble basic_level_indicator_get_value(BasicLevelIndicator *self);
void basic_level_indicator_set_value(BasicLevelIndicator *self, gdouble new_value);

G_END_DECLS
#endif  // BASIC_LEVEL_INDICATOR_H__
