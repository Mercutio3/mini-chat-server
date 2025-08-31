#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define LOG_INFO(fmt, ...) fprintf(stdout, "[INFO] " fmt "\n", ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)

#ifdef DEBUG
#define LOG_DEBUG(fmt, ...) fprintf(stdout, "[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...) // Does nothing if DEBUG isn't set
#endif

void openChatLog(const char *filename);
void closeChatLog();
void logChatMessage(const char *fmt, ...);

#endif