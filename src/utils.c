/*
utils.c - Helper functions
*/

#include "../include/clientList.h"
#include <stdbool.h>
#include <stdio.h>

char *getUserNameFromFD(struct clientNode *head, int fd) {
    struct clientNode *current = head;
    while (current != NULL) {
        if (current->fd == fd) {
            return current->username;
        }
        current = current->next;
    }
    return NULL; // Not found
}

// Loops through buffer ensuring legal UTF-8
bool isUTF8(const char *s, size_t len) {
    size_t i = 0;
    while (i < len) {
        unsigned char c = (unsigned char)s[i];
        if (c <= 0x7F) {
            // 1-byte character (ASCII)
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            // 2-byte UTF-8 character; next byte must start with 10
            if (i + 1 >= len || (s[i + 1] & 0xC0) != 0x80 || c < 0xC2) {
                return false;
            }
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3-byte UTF-8 character; next bytes must start with 10
            if (i + 2 >= len || (s[i + 1] & 0xC0) != 0x80 || (s[i + 2] & 0xC0) != 0x80) {
                return false;
            }

            // Check for overlong encoding
            if (c == 0xE0 && (unsigned char)s[i + 1] < 0xA0) {
                return false;
            }
            if (c == 0xED && (unsigned char)s[i + 1] >= 0xA0) {
                return false;
            }
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4-byte UTF-8 character; next bytes must start with 10
            if (i + 3 >= len || (s[i + 1] & 0xC0) != 0x80 || (s[i + 2] & 0xC0) != 0x80 ||
                (s[i + 3] & 0xC0) != 0x80) {
                return false;
            }

            // Check for overlong encoding
            if (c == 0xF0 && (unsigned char)s[i + 1] < 0x90) {
                return false;
            }
            if (c == 0xF4 && (unsigned char)s[i + 1] > 0x8F) {
                return false;
            }
            if (c > 0xF4) {
                return false;
            }
            i += 4;
        } else {
            return false; // Invalid byte
        }
    }
    return true;
}