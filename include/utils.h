#ifndef UTILS_H
#define UTILS_H

#include "clientList.h"
#include <stdbool.h>
#include <stdio.h>

char *getUserNameFromFD(struct clientNode *head, int fd);
bool isUTF8(const char *s, size_t len);

#endif