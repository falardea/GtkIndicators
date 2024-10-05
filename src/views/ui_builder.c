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
#include "indicators/puck_indicator.h"
#include "indicators/scaling_level_indicator.h"
#include "indicators/tank_indicator.h"
#include "views/indicator_layout.h"

const double DIAL_INIT_VALUE = 0.0;

gboolean on_component_clicked(__attribute__((unused)) GtkWidget *wdgt, __attribute__((unused)) app_widget_ref_struct appWidgetsT)
{
   // Weird bug... if the log_level is not set, and this is a debug print, and we don't have a return value...
   // we get a seg fault with an infinite loop.
   logging_llprintf(LOGLEVEL_INFO, "%s", __func__);
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
   IndicatorLayout *batt_layout = g_new(IndicatorLayout, 1);
   batt_layout = indicator_layout_set(batt_layout, TRUE, TRUE, 3, 3, 3, 3);
   appWidgetsT->w_indicator_00 = battery_indicator_new(batt_layout, TRUE);
   gtk_box_pack_start(GTK_BOX(appWidgetsT->w_indicator_box_00), appWidgetsT->w_indicator_00, TRUE, TRUE, 0);
   indicator_layout_free(batt_layout);

   appWidgetsT->w_indicator_box_01 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_01"));
   IndicatorLayout *basio_layout = g_new(IndicatorLayout, 1);
   basio_layout = indicator_layout_set(basio_layout, TRUE, TRUE, 3, 3, 3, 3);
   appWidgetsT->w_indicator_01 = basic_level_indicator_new(basio_layout, TRUE);
   gtk_box_pack_start(GTK_BOX(appWidgetsT->w_indicator_box_01), appWidgetsT->w_indicator_01, TRUE, TRUE, 0);
   indicator_layout_free(basio_layout);

   appWidgetsT->w_indicator_box_02 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_02"));
   appWidgetsT->w_indicator_02 = bluetooth_indicator_new();
   gtk_box_pack_start(GTK_BOX(appWidgetsT->w_indicator_box_02), appWidgetsT->w_indicator_02, TRUE, TRUE, 0);

   appWidgetsT->w_indicator_box_10 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_10"));
   IndicatorLayout *horiz_scaling_layout = g_new(IndicatorLayout, 1);
   horiz_scaling_layout = indicator_layout_set(horiz_scaling_layout, TRUE, TRUE, 3, 3, 3, 3);
   appWidgetsT->w_indicator_10 = scaling_level_indicator_new(horiz_scaling_layout, FALSE);
   gtk_box_pack_start(GTK_BOX(appWidgetsT->w_indicator_box_10), appWidgetsT->w_indicator_10, TRUE, TRUE, 0);
   indicator_layout_free(horiz_scaling_layout);

   appWidgetsT->w_indicator_box_11 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_11"));
   IndicatorLayout *vert_scaling_layout = g_new(IndicatorLayout, 1);
   vert_scaling_layout = indicator_layout_set(vert_scaling_layout, TRUE, TRUE, 3, 3, 3, 3);
   appWidgetsT->w_indicator_11 = scaling_level_indicator_new(vert_scaling_layout, TRUE);
   gtk_box_pack_start(GTK_BOX(appWidgetsT->w_indicator_box_11), appWidgetsT->w_indicator_11, TRUE, TRUE, 0);
   indicator_layout_free(vert_scaling_layout);

   appWidgetsT->w_indicator_box_12 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_12"));
   IndicatorLayout *basio2_layout = g_new(IndicatorLayout, 1);
   basio_layout = indicator_layout_set(basio2_layout, TRUE, TRUE, 10, 10, 3, 3);
   appWidgetsT->w_indicator_12 = basic_level_indicator_new(basio2_layout, FALSE);
   gtk_grid_attach(GTK_GRID(appWidgetsT->w_indicator_box_12), appWidgetsT->w_indicator_12, 0, 0, 1, 1);
   indicator_layout_free(basio2_layout);

   appWidgetsT->w_indicator_box_20 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_20"));
   IndicatorLayout *puck_layout = g_new(IndicatorLayout, 1);
   puck_layout = indicator_layout_set(puck_layout, TRUE, TRUE, 0, 0, 0, 0);
   appWidgetsT->w_indicator_20 = puck_indicator_new(puck_layout,FALSE);
   gtk_box_pack_start(GTK_BOX(appWidgetsT->w_indicator_box_20), appWidgetsT->w_indicator_20, TRUE, TRUE, 0);
   indicator_layout_free(puck_layout);

   appWidgetsT->w_indicator_box_21 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_21"));
   IndicatorLayout *tank_layout = g_new(IndicatorLayout, 1);
   tank_layout = indicator_layout_set(tank_layout, TRUE, TRUE, 0, 0, 0, 0);
   appWidgetsT->w_indicator_21 = tank_indicator_new(tank_layout, FALSE);
   gtk_box_pack_start(GTK_BOX(appWidgetsT->w_indicator_box_21), appWidgetsT->w_indicator_21, TRUE, TRUE, 0);
   indicator_layout_free(tank_layout);

   appWidgetsT->w_indicator_box_22 = GTK_WIDGET(gtk_builder_get_object(builder, "indicator_box_22"));

   g_object_bind_property(appWidgetsT->w_dial, "old_value", appWidgetsT->w_dial_label, "value", G_BINDING_DEFAULT);
   g_object_bind_property(appWidgetsT->w_dial_label, "value", appWidgetsT->w_indicator_00, "value", G_BINDING_DEFAULT);
   g_object_bind_property(appWidgetsT->w_dial_label, "value", appWidgetsT->w_indicator_01, "value", G_BINDING_DEFAULT);
   g_object_bind_property(appWidgetsT->w_dial_label, "value", appWidgetsT->w_indicator_10, "value", G_BINDING_DEFAULT);
   g_object_bind_property(appWidgetsT->w_dial_label, "value", appWidgetsT->w_indicator_11, "value", G_BINDING_DEFAULT);
   g_object_bind_property(appWidgetsT->w_dial_label, "value", appWidgetsT->w_indicator_12, "value", G_BINDING_DEFAULT);
   g_object_bind_property(appWidgetsT->w_dial_label, "value", appWidgetsT->w_indicator_20, "value", G_BINDING_DEFAULT);
   g_object_bind_property(appWidgetsT->w_dial_label, "value", appWidgetsT->w_indicator_21, "value", G_BINDING_DEFAULT);


   gtk_builder_connect_signals(builder, appWidgetsT);

   g_signal_connect (G_OBJECT(appWidgetsT->w_indicator_00), "button-press-event",
                     G_CALLBACK(on_component_clicked), appWidgetsT);
   g_signal_connect (G_OBJECT(appWidgetsT->w_indicator_01), "button-press-event",
                     G_CALLBACK(on_component_clicked), appWidgetsT);
   g_signal_connect (G_OBJECT(appWidgetsT->w_indicator_10), "button-press-event",
                     G_CALLBACK(on_component_clicked), appWidgetsT);
   g_signal_connect (G_OBJECT(appWidgetsT->w_indicator_11), "button-press-event",
                     G_CALLBACK(on_component_clicked), appWidgetsT);
   g_signal_connect (G_OBJECT(appWidgetsT->w_indicator_12), "button-press-event",
                     G_CALLBACK(on_component_clicked), appWidgetsT);
   g_signal_connect (G_OBJECT(appWidgetsT->w_indicator_20), "button-press-event",
                     G_CALLBACK(on_component_clicked), appWidgetsT);
   g_signal_connect (G_OBJECT(appWidgetsT->w_indicator_21), "button-press-event",
                     G_CALLBACK(on_component_clicked), appWidgetsT);

   // It appears the composite/custom widgets with templates are loaded and show, where these
   // seem to need explicit exposure.
   gtk_widget_show(appWidgetsT->w_dial);
   gtk_widget_show(appWidgetsT->w_indicator_00);
   gtk_widget_show(appWidgetsT->w_indicator_01);
   gtk_widget_show(appWidgetsT->w_indicator_02);
   gtk_widget_show(appWidgetsT->w_indicator_10);
   gtk_widget_show(appWidgetsT->w_indicator_11);
   gtk_widget_show(appWidgetsT->w_indicator_12);
   gtk_widget_show(appWidgetsT->w_indicator_20);
   gtk_widget_show(appWidgetsT->w_indicator_21);

   g_object_unref(builder);
   return appWidgetsT;
}
