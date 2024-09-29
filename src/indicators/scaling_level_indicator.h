/**
 * Created by french on 9/28/24.
 * @brief
 */
#ifndef SCALING_LEVEL_INDICATOR_H__
#define SCALING_LEVEL_INDICATOR_H__
#include "glib.h"
G_BEGIN_DECLS
#include <gtk/gtk.h>
#include <glib-object.h>

#define SCALING_LEVEL_TYPE_INDICATOR ( scaling_level_indicator_get_type () )

G_DECLARE_FINAL_TYPE(ScalingLevelIndicator, scaling_level_indicator, SCALING_LEVEL, INDICATOR, GtkWidget)

GtkWidget *scaling_level_indicator_new(gboolean vertical);

gdouble scaling_level_indicator_get_value(ScalingLevelIndicator *self);
void scaling_level_indicator_set_value(ScalingLevelIndicator *self, gdouble new_value);

G_END_DECLS
#endif  // SCALING_LEVEL_INDICATOR_H__
