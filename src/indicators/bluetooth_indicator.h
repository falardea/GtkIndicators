/**
 * Created by french on 9/22/24.
 * @brief
 */
#ifndef BLUETOOTH_INDICATOR_H__
#define BLUETOOTH_INDICATOR_H__
#include "glib.h"
G_BEGIN_DECLS
#include <gtk/gtk.h>
#include <glib-object.h>

#define BLUETOOTH_TYPE_INDICATOR ( bluetooth_indicator_get_type () )

G_DECLARE_FINAL_TYPE(BluetoothIndicator, bluetooth_indicator, BLUETOOTH, INDICATOR, GtkWidget)

GtkWidget *bluetooth_indicator_new(void);

gdouble bluetooth_indicator_get_value(BluetoothIndicator *self);
void bluetooth_indicator_set_value(BluetoothIndicator *self, gdouble new_value);

G_END_DECLS
#endif  // BLUETOOTH_INDICATOR_H__
