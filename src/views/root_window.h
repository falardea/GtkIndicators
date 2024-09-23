/**
 * @brief The root window implementation
 */
#ifndef ROOT_WINDOW_H__
#define ROOT_WINDOW_H__
#include <gtk/gtk.h>
#include "../app_globals.h"

void on_min_close_clicked(__attribute__((unused)) GtkWidget *srcWidget,
                          __attribute__((unused)) gpointer uData);

gboolean on_min_main_wnd_delete_event(__attribute__((unused)) GtkWidget *srcWidget,
                                      __attribute__((unused)) GdkEvent *event,
                                      __attribute__((unused)) gpointer uData);

void print_log_level_msgout(LOGLEVEL loglevel, const char *_format, ...);

#endif  /* ROOT_WINDOW_H__ */
