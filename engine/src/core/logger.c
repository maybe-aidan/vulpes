#include "logger.h"
#include "asserts.h"
#include "../platform/platform.h"

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line){
    log_output(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file %s, line: %d\n", expression, message, file, line);
}

b8 initialize_logging(){
    // TODO: create log file
    return TRUE;
}
void shutdown_logging(){
    // TODO: cleanup, write queued entries
}

void log_output(log_level level, const char* message, ...){
    const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    b8 is_error = level < LOG_LEVEL_WARN;

    // Imposes 32k character limit on a single log entry to avoid dynamic memory allocation
    // DO NOT ACTUALLY USE 32K CHARACTERS!!!
    const i32 msg_length = 32000;
    char out_message[msg_length];
    memset(out_message, 0, sizeof(out_message));

    // Format original message
    // Avoid strange compiler errors from MS overriding GCC/Clang va_list by using __builtin_va_list instead
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(out_message, msg_length, message, arg_ptr);
    va_end(arg_ptr);

    char out_message2[msg_length];
    sprintf(out_message2, "%s%s\n", level_strings[level], out_message);

    // Platform-specific output
    if(is_error){
        platform_console_write_error(out_message2, level);
    }else{
        platform_console_write(out_message2, level);
    }
}