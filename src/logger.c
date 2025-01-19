#include "logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#define COLOR_RESET "\033[0m"
#define COLOR_INFO "\033[32m"    // Green
#define COLOR_WARNING "\033[33m" // Yellow
#define COLOR_ERROR "\033[31m"   // Red
#define COLOR_DEBUG "\033[36m"   // Cyan
#define COLOR_BLUE "\033[34m"    // Blue


static bool colors_enabled = true;

void logger_init(int enable_colors) {
    colors_enabled = enable_colors;
}

void log_message(LogLevel level, const char *format, ...) {
    const char *level_str;
    const char *color_code;

    // Determine log level and color
    switch (level) {
        case LOG_LEVEL_INFO:
            level_str = "INFO";
            color_code = COLOR_INFO;
            break;
        case LOG_LEVEL_WARNING:
            level_str = "WARNING";
            color_code = COLOR_WARNING;
            break;
        case LOG_LEVEL_ERROR:
            level_str = "ERROR";
            color_code = COLOR_ERROR;
            break;
        case LOG_LEVEL_DEBUG:
            level_str = "DEBUG";
            color_code = COLOR_DEBUG;
            break;
        case LOG_LEVEL_HEADER:
            level_str = "---";
            color_code = COLOR_BLUE;
            break;
        default:
            level_str = "UNKNOWN";
            color_code = COLOR_RESET;
            break;
    }

    // Print the log message
    if (colors_enabled ) {
        printf("%s[%s]: ", color_code, level_str);
    }
 
    else {
        printf("[%s]: ", level_str);
    }

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    if (colors_enabled) {
        printf(COLOR_RESET);
    }

    printf("\n");
}

void logger_cleanup(void) {
    // Currently, nothing to clean up
    // Placeholder for potential future resource cleanup
}
