/**
 * Created by french on 9/28/24.
 * @brief
 */
#ifndef TANK_INDICATOR_H__
#define TANK_INDICATOR_H__
#include "glib.h"
G_BEGIN_DECLS
#include <gtk/gtk.h>
#include <glib-object.h>
#include "../views/indicator_layout.h"


#define TANK_TYPE_INDICATOR ( tank_indicator_get_type () )

G_DECLARE_FINAL_TYPE(TankIndicator, tank_indicator, TANK, INDICATOR, GtkWidget)

GtkWidget *tank_indicator_new(IndicatorLayout *layout, gboolean vertical);

gdouble tank_indicator_get_value(TankIndicator *self);
void tank_indicator_set_value(TankIndicator *self, gdouble new_value);


G_END_DECLS
#endif  // TANK_INDICATOR_H__
