/*
log.c - Chat logging function and file management
*/

#include "../include/log.h"
#include <stdarg.h>
#include <time.h>

static FILE *chatLogFile = NULL;
void openChatLog(const char *filename) {
    chatLogFile = fopen(filename, "a");
    if (chatLogFile == NULL) {
        perror("Error opening chat log file");
    }
}

void closeChatLog() {
    if (chatLogFile) {
        fclose(chatLogFile);
    }
}

void logChatMessage(const char *fmt, ...) {
    if (!chatLogFile) {
        return;
    }
    time_t now = time(NULL);
    struct tm *utc_tm = gmtime(&now);
    char timebuf[32];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S UTC", utc_tm);
    fprintf(chatLogFile, "[%s] ", timebuf);
    va_list args;
    va_start(args, fmt);
    vfprintf(chatLogFile, fmt, args);
    va_end(args);
    fprintf(chatLogFile, "\n");
    fflush(chatLogFile);
}