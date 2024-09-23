/**
 * @brief An interface layer to manage the underlying application
 */
#ifndef APP_INTERFACE_H__
#define APP_INTERFACE_H__
#include <stdbool.h>
#include "app_globals.h"

typedef enum {
   APP_STATE_UNINITIALIZED =  -1,
   APP_STATE_SUCCESS = 0,
   APP_STATE_ERROR
} APP_INTERFACE_STATE;

typedef struct {
   APP_INTERFACE_STATE app_state;
   bool debugging_enabled;
   bool log_with_timestamps;
   LOGLEVEL runtime_log_level;
} app_model_t;

void app_init(int argc, char **argv);

void app_finalize(void);

const char *get_state_str(APP_INTERFACE_STATE state);

void set_app_state(APP_INTERFACE_STATE state);
APP_INTERFACE_STATE get_app_state(void);

void set_app_debug_flag(bool enable);

void set_app_log_w_timestamp_flag(bool enable);
bool get_app_log_w_timestamp_flag(void);

void set_app_log_level(LOGLEVEL logLevel);
LOGLEVEL get_app_log_level(void);

#endif  /* APP_INTERFACE_H__ */
