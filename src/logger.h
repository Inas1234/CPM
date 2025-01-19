#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_HEADER
} LogLevel;

void logger_init(int enable_colors);

void log_message(LogLevel level, const char *format, ...);

void logger_cleanup(void);

#endif 
