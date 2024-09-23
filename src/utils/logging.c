/**
 * @brief The implementation
 */
#include <stdio.h>
#include "interfaces/app_interface.h"
#include "sys_interface.h"
#include "logging.h"

const char* DEBUG_STR = "DEBUG";
const char* INFO_STR = "INFO";
const char* ERROR_STR = "ERROR";
const int   LOGGING_MAX_MSG_LENGTH = 1024;
static char timestamp[20];  // not sure why it felt better to allocate the memory once

const char *get_log_level_str(LOGLEVEL level)
{
   if (level == LOGLEVEL_ERROR)
      return ERROR_STR;
   else if (level == LOGLEVEL_INFO)
      return INFO_STR;
   else
      return DEBUG_STR;
}

int logging_llprintf(LOGLEVEL level, const char *_format, ...) {
   int done = 0;

   if (level >= get_app_log_level()) {
      bool use_ts = get_app_log_w_timestamp_flag();
      // This started as effectively a copy of stdio.printf
      char line_out[LOGGING_MAX_MSG_LENGTH];

      if (use_ts)
         get_timestamp(timestamp, sizeof(timestamp));

      va_list arg;
      va_start(arg, _format);
      done = vsnprintf(line_out, sizeof (line_out), _format, arg);
      va_end(arg);

      /* This seems a bit sloppy, we slipped the use of use_ts in as an afterthought  */
      printf("%s%s%s:%s\n", (use_ts) ? timestamp : "", (use_ts) ? ":":"", get_log_level_str(level), line_out);

      return done;
   }
   return done;
}
