#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

// Log types for different message categories
typedef enum {
    LOG_INFO,    // Regular information messages
    LOG_SUCCESS, // Success messages
    LOG_WARNING, // Warning messages
    LOG_ERROR,   // Error messages
    LOG_DEBUG    // Debug information
} LogType;

// Color codes for terminal output
#define COLOR_RESET   "\x1B[0m"
#define COLOR_GREEN   "\x1B[32m"
#define COLOR_YELLOW  "\x1B[33m"
#define COLOR_RED     "\x1B[31m"
#define COLOR_BLUE    "\x1B[34m"
#define COLOR_CYAN    "\x1B[36m"
#define COLOR_WHITE   "\x1B[37m"
#define COLOR_MAGENTA "\x1B[35m"

extern const char* LOG_PREFIX[];
extern const char* LOG_COLOR[];

// Convenience functions for different log types - using camelCase to match existing calls
void logInfo(const char* format, ...);
void logSuccess(const char* format, ...);
void logWarning(const char* format, ...);
void logError(const char* format, ...);
void logDebug(const char* format, ...);

// For backward compatibility, also provide underscore versions
#define log_info    logInfo
#define log_success logSuccess
#define log_warning logWarning
#define log_error   logError
#define log_debug   logDebug

// Title display function for game startup banner
void display_title(const char* game_title);

#endif // LOG_H