/**
 * Created by french on 9/28/24.
 * @brief
 */
#ifndef PUCK_INDICATOR_H__
#define PUCK_INDICATOR_H__
#include "glib.h"
G_BEGIN_DECLS
#include <gtk/gtk.h>
#include <glib-object.h>
#include "../views/indicator_layout.h"

#define PUCK_TYPE_INDICATOR ( puck_indicator_get_type () )

G_DECLARE_FINAL_TYPE(PuckIndicator, puck_indicator, PUCK, INDICATOR, GtkWidget)

GtkWidget *puck_indicator_new(IndicatorLayout *layout, gboolean vertical);

gdouble puck_indicator_get_value(PuckIndicator *self);
void puck_indicator_set_value(PuckIndicator *self, gdouble new_value);

G_END_DECLS
#endif  // PUCK_INDICATOR_H__
