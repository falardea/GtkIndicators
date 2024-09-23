/**
 * @brief This is a first cut at a battery indicator
*/
#ifndef BATTERY_INDICATOR_H__
#define BATTERY_INDICATOR_H__
#include "glib.h"
G_BEGIN_DECLS
#include <gtk/gtk.h>
#include <glib-object.h>

#define BATTERY_TYPE_INDICATOR ( battery_indicator_get_type () )

G_DECLARE_FINAL_TYPE(BatteryIndicator, battery_indicator, BATTERY, INDICATOR, GtkWidget)

GtkWidget *battery_indicator_new(void);

gdouble battery_indicator_get_value(BatteryIndicator *self);
void battery_indicator_set_value(BatteryIndicator *self, gdouble new_value);

G_END_DECLS
#endif  // BATTERY_INDICATOR_H__
