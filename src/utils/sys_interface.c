/**
 * @brief The implementation
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sys_interface.h"
#include "logging.h"

RVALUE set_display_env(void) {
   char *displayEnv;
   displayEnv = getenv("DISPLAY");
   if (displayEnv == NULL) {
      // Construct the cmd string
      char cmd[128];
      snprintf(cmd, sizeof(cmd), "grep nameserver /etc/resolv.conf | sed 's/nameserver //'");

      // Open an interface with the OS to execute cmd, or die
      FILE *pF;
      pF = popen(cmd, "r");
      if (pF == NULL) {
         logging_llprintf(LOGLEVEL_ERROR, "trying to open cmd interface");
         return RVALUE_ERROR;
      }

      // Execute the cmd via write to cmd interface, or die
      char retBuf[256];
      if (fgets(retBuf, 256, pF) == NULL) {
         logging_llprintf(LOGLEVEL_ERROR, "something went wrong reading cmd response");
         return RVALUE_ERROR;
      }
      // strip the newline and add the ":0.0" display number
      retBuf[strcspn(retBuf, "\n")] = 0;
      char displayId[5] = {':', '0', '.', '0', 0};
      char nameserverStr[sizeof(retBuf) + sizeof(displayId)];
      snprintf(nameserverStr, sizeof(nameserverStr), "%s%s", retBuf, displayId);

      // Close the cmd interface, or die
      if (pclose(pF) != 0) {
         logging_llprintf(LOGLEVEL_ERROR, "closing cmd interface");
         return RVALUE_ERROR;
      }
      logging_llprintf(LOGLEVEL_INFO, "setenv: DISPLAY=%s", nameserverStr);

      bool envSetSuccess = setenv("DISPLAY", nameserverStr, false);

      return (envSetSuccess == 0) ? RVALUE_SUCCESS : RVALUE_ERROR;
   } else {
      return RVALUE_SUCCESS;
   }
}

void get_timestamp(char *buf, size_t bufsz) {
   time_t current_time;
   struct tm *timeinfo;

   time(&current_time);
   timeinfo = localtime(&current_time);
   snprintf(buf, bufsz, DEFAULT_TIMESTAMP_LOG_FORMAT,
            timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}
