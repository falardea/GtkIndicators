/**
 * @brief The implementation
 */
#include "root_window.h"
#include "interfaces/app_interface.h"
#include "utils/sys_interface.h"
#include "utils/logging.h"

static const char *MSG_OUT_CURSOR_NAME = "msgOutCursor";
static GtkTextMark *msgOutCursor;
static char timestamp[20];  // not sure why it felt better to allocate the memory once

void on_main_wnd_close_clicked(__attribute__((unused)) GtkWidget *srcWidget,
                                __attribute__((unused)) gpointer uData) {
   gtk_main_quit();
}

gboolean on_main_wnd_delete_event(__attribute__((unused)) GtkWidget *srcWidget,
                                    __attribute__((unused)) GdkEvent *event,
                                    __attribute__((unused)) gpointer uData) {
   gtk_main_quit();
   return FALSE;
}

void on_do_something_button_clicked(__attribute__((unused)) GtkButton *button, __attribute__((unused)) gpointer *user_data)
{
   app_widget_ref_struct *wdgts = (app_widget_ref_struct *) user_data;
   if (gtk_entry_buffer_get_length(gtk_entry_get_buffer(GTK_ENTRY(wdgts->w_say_something_entry))) > 0){
      print_log_level_msgout(LOGLEVEL_INFO, "%s", gtk_entry_get_text(GTK_ENTRY(wdgts->w_say_something_entry)));
   } else {
      print_log_level_msgout(LOGLEVEL_INFO, "nothing to say?");
   }
}

void set_msgout_buffer(const char *msgout)
{
   if (get_app_state() == APP_STATE_SUCCESS)
   {  /* Let's not use the MsgOut widget before it's been built */
      GtkTextBuffer  *tvBuff  =  gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_app_widget_refs->w_msg_out_textview));
      GtkTextIter    endIter;
      gtk_text_buffer_get_end_iter(tvBuff, &endIter);

      if (!gtk_text_buffer_get_mark(tvBuff, MSG_OUT_CURSOR_NAME))
      {
         msgOutCursor = gtk_text_mark_new(MSG_OUT_CURSOR_NAME, FALSE);
         gtk_text_buffer_add_mark(tvBuff, msgOutCursor, &endIter);
      }

      gtk_text_buffer_insert(tvBuff, &endIter, msgout, -1);

      // This keeps the latest msgout in view, BUT if you were scrolling up and a new msgout was posted, it will
      // autoscroll to the insertion.  It would be better to only auto-scroll the msgout if the user is not looking
      // at previous messages
      gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(g_app_widget_refs->w_msg_out_textview), msgOutCursor, 0.0, TRUE, 0.0, 0.0);
   }
}

void print_log_level_msgout(LOGLEVEL loglevel, const char *_format, ...)
{
   if (loglevel >= get_app_log_level()) {
      bool use_ts = get_app_log_w_timestamp_flag();

      // We're not going to "flag" out the timestamp in the memory sizing here, because... who cares if it's too big?
      char ll_msg_out[LOGGING_MAX_MSG_LENGTH + sizeof(timestamp) + (2*sizeof(':')) + sizeof(get_log_level_str(loglevel))];
      char line_out[LOGGING_MAX_MSG_LENGTH];

      va_list arg;
      va_start(arg, _format);
      vsnprintf(line_out, sizeof (line_out), _format, arg);
      va_end(arg);

      if (use_ts)
         get_timestamp(timestamp, sizeof(timestamp));

      snprintf(ll_msg_out, sizeof (ll_msg_out), "%s%s%s:%s\n",
               use_ts ? timestamp:"", use_ts?":":"", get_log_level_str(loglevel), line_out);

      logging_llprintf(loglevel, "%s", line_out);
      set_msgout_buffer(ll_msg_out);
   }
}
