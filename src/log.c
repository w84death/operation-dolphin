#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "../include/log.h"

// Define the global constants declared in the header
const char* LOG_PREFIX[] = {
    "[INFO]    ",
    "[SUCCESS] ",
    "[WARNING] ",
    "[ERROR]   ",
    "[DEBUG]   "
};

const char* LOG_COLOR[] = {
    COLOR_CYAN,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_RED,
    COLOR_MAGENTA
};

// Implementation of logging functions

void logInfo(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf("%s%s%s ", LOG_COLOR[LOG_INFO], LOG_PREFIX[LOG_INFO], COLOR_RESET);
    vprintf(format, args);
    if (format[0] != '\0' && format[strlen(format)-1] != '\n')
        printf("\n");
    va_end(args);
}

void logSuccess(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf("%s%s%s ", LOG_COLOR[LOG_SUCCESS], LOG_PREFIX[LOG_SUCCESS], COLOR_RESET);
    vprintf(format, args);
    if (format[0] != '\0' && format[strlen(format)-1] != '\n')
        printf("\n");
    va_end(args);
}

void logWarning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf("%s%s%s ", LOG_COLOR[LOG_WARNING], LOG_PREFIX[LOG_WARNING], COLOR_RESET);
    vprintf(format, args);
    if (format[0] != '\0' && format[strlen(format)-1] != '\n')
        printf("\n");
    va_end(args);
}

void logError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf("%s%s%s ", LOG_COLOR[LOG_ERROR], LOG_PREFIX[LOG_ERROR], COLOR_RESET);
    vprintf(format, args);
    if (format[0] != '\0' && format[strlen(format)-1] != '\n')
        printf("\n");
    va_end(args);
}

void logDebug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf("%s%s%s ", LOG_COLOR[LOG_DEBUG], LOG_PREFIX[LOG_DEBUG], COLOR_RESET);
    vprintf(format, args);
    if (format[0] != '\0' && format[strlen(format)-1] != '\n')
        printf("\n");
    va_end(args);
}

void display_title(const char* game_title) {
    printf("\n\n");
    printf("%s╔════════════════════════════════════════════════════════════╗%s\n", COLOR_GREEN, COLOR_RESET);
    printf("%s║                                                            ║%s\n", COLOR_GREEN, COLOR_RESET);
    printf("%s║                  %s%-40s%s  ║%s\n", COLOR_GREEN, COLOR_CYAN, game_title, COLOR_GREEN, COLOR_RESET);
    printf("%s║                     %s© 2025 P1X GAMES%s                    ║%s\n", COLOR_GREEN, COLOR_YELLOW, COLOR_GREEN, COLOR_RESET);
    printf("%s║                                                            ║%s\n", COLOR_GREEN, COLOR_RESET);
    printf("%s╚════════════════════════════════════════════════════════════╝%s\n", COLOR_GREEN, COLOR_RESET);
    printf("\n");
}