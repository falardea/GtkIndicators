/**
 * @brief A modules to centralize interfacing with the OS
 */
#ifndef SYS_INTERFACE_H__
#define SYS_INTERFACE_H__
#include <stdbool.h>
#include "app_globals.h"

RVALUE set_display_env(void);

void get_timestamp(char *buf, size_t bufsz);

#endif  /* SYS_INTERFACE_H__ */
