/**
 * @brief An attempt at a log-level enabled logger
 */
#ifndef LOGGING_H__
#define LOGGING_H__
#include <stdarg.h>
#include "app_globals.h"

// #define LOG_INFO(format, ...) logging_llprintf(LOGLEVEL_INFO, "%s: %s\n", __func__, format __VA_OPT__(,) __VA_ARGS__)
// #define LOG_DEBUG(format, ...) logging_llprintf(LOGLEVEL_DEBUG, "%s: %s\n", __func__, format __VA_OPT__(,) __VA_ARGS__)
// #define LOG_ERROR(format, ...) logging_llprintf(LOGLEVEL_ERROR, "%s: %s\n", __func__, format __VA_OPT__(,) __VA_ARGS__)

const char *get_log_level_str(LOGLEVEL level);

int logging_llprintf(LOGLEVEL level, const char *_format, ...);

#endif  /* LOGGING_H__ */
