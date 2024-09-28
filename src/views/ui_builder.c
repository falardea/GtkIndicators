/**
 * @brief The implementation
 */
#include <gtk/gtk.h>
#include "app_globals.h"
#include "ui_builder.h"
#include "utils/logging.h"
#include "indicators/gtkdial.h"
#include "indicators/battery_indicator.h"
#include "indicators/numeric_label.h"
#include "indicators/basic_level_indicator.h"
#include "indicators/bluetooth_indicator.h"

const double DIAL_INIT_VALUE = 0.0;

gboolean on_component_clicked(__attribute__((unused)) GtkWidget *wdgt, __attribute__((unused)) app_widget_ref_struct appWidgetsT)
{
   // Weird bug... if the log_level is not set, and this is a debug print, and we don't have a return value...
   // we get a seg fault with an infinite loop.
   logging_llprintf(LOGLEVEL_DEBUG, "%s", __func__);
   return TRUE; // stop propagating the event
}

app_widget_ref_struct *app_builder(void) {
   GtkBuilder        *builder;

   app_widget_ref_struct *appWidgetsT = g_slice_new(app_widget_ref_struct);

   builder = gtk_builder_new();

   if (gtk_builder_add_from_resource(builder, "/resource_path/resources/gtk_indicators.glade", NULL) == 0) {
      logging_llprintf(LOGLEVEL_ERROR, "failed to load glade resource");
      return NULL;
   }

   appWidgetsT->w_msg_out_textview = GTK_WIDGET(gtk_builder_get_object(builder, "msg_out_textview"));
   appWidgetsT->w_say_something_entry = GTK_WIDGET(gtk_builder_get_object(builder, "say_something_entry"));

   appWidgetsT->w_dial_box = GTK_WIDGET(gtk_builder_get_object(builder, "dial_box"));
   appWidgetsT->w_dial_label_box = GTK_WIDGET(gtk_builder_get_object(builder, "dial_label_box"));

   GtkAdjustment *adjustment;
   adjustment  = GTK_ADJUSTMENT (gtk_adjustment_new (DIAL_INIT_VALUE, 0, 100, 0.01, 0.1, 0));
   appWidgetsT->w_dial = gtk_dial_new (adjustment);
   appWidgetsT->w_dial_label = numeric_label_new(DIAL_INIT_VALUE, 1);
   gtk_box_pack_start(GTK_BOX(appWidgetsT->w_dial_box), appWidgetsT->w_dial, TRUE, TRUE, 10);
   gtk_box_pack_end(GTK_BOX(appWidgetsT->w_dial_label_box), appWidgetsT->w_dial_label, FALSE, TRUE, 0);

   appWidgetsT->w_indicator_box_00 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_00"));
   appWidgetsT->w_indicator_00 = battery_indicator_new();
   gtk_box_pack_start(GTK_BOX(appWidgetsT->w_indicator_box_00), appWidgetsT->w_indicator_00, TRUE, TRUE, 0);

   appWidgetsT->w_indicator_box_01 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_01"));
   appWidgetsT->w_indicator_01 = basic_level_indicator_new(TRUE);
   gtk_box_pack_start(GTK_BOX(appWidgetsT->w_indicator_box_01), appWidgetsT->w_indicator_01, TRUE, TRUE, 0);

   appWidgetsT->w_indicator_box_02 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_02"));
   appWidgetsT->w_indicator_02 = bluetooth_indicator_new();
   gtk_box_pack_start(GTK_BOX(appWidgetsT->w_indicator_box_02), appWidgetsT->w_indicator_02, TRUE, TRUE, 0);

   appWidgetsT->w_indicator_box_10 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_10"));
   appWidgetsT->w_indicator_10 = basic_level_indicator_new(FALSE);
   gtk_box_pack_start(GTK_BOX(appWidgetsT->w_indicator_box_10), appWidgetsT->w_indicator_10, TRUE, TRUE, 1);

   appWidgetsT->w_indicator_box_11 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_11"));
   appWidgetsT->w_indicator_box_12 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_12"));
   appWidgetsT->w_indicator_box_20 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_20"));
   appWidgetsT->w_indicator_box_21 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_21"));
   appWidgetsT->w_indicator_box_22 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_22"));

   g_object_bind_property(appWidgetsT->w_dial, "old_value", appWidgetsT->w_dial_label, "value", G_BINDING_DEFAULT);
   g_object_bind_property(appWidgetsT->w_dial_label, "value", appWidgetsT->w_indicator_00, "value", G_BINDING_DEFAULT);
   g_object_bind_property(appWidgetsT->w_dial_label, "value", appWidgetsT->w_indicator_01, "value", G_BINDING_DEFAULT);
   g_object_bind_property(appWidgetsT->w_dial_label, "value", appWidgetsT->w_indicator_10, "value", G_BINDING_DEFAULT);

   gtk_builder_connect_signals(builder, appWidgetsT);

   g_signal_connect (G_OBJECT(appWidgetsT->w_indicator_00), "button-press-event",
                     G_CALLBACK(on_component_clicked), appWidgetsT);
   g_signal_connect (G_OBJECT(appWidgetsT->w_indicator_01), "button-press-event",
                     G_CALLBACK(on_component_clicked), appWidgetsT);
   g_signal_connect (G_OBJECT(appWidgetsT->w_indicator_10), "button-press-event",
                     G_CALLBACK(on_component_clicked), appWidgetsT);

   // It appears the composite/custom widgets with templates are loaded and show, where these
   // seem to need explicit exposure.
   gtk_widget_show(appWidgetsT->w_dial);
   gtk_widget_show(appWidgetsT->w_indicator_00);
   gtk_widget_show(appWidgetsT->w_indicator_01);
   gtk_widget_show(appWidgetsT->w_indicator_02);
   gtk_widget_show(appWidgetsT->w_indicator_10);

   g_object_unref(builder);
   return appWidgetsT;
}
