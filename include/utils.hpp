#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>

inline bool isUTF8(const std::string &s) {
    size_t i = 0, len = s.size();
    while (i < len) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c <= 0x7F) {
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            if (i + 1 >= len || (static_cast<unsigned char>(s[i + 1]) & 0xC0) != 0x80 || c < 0xC2) {
                return false;
            }
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            if (i + 2 >= len || (static_cast<unsigned char>(s[i + 1]) & 0xC0) != 0x80 ||
                (static_cast<unsigned char>(s[i + 2]) & 0xC0) != 0x80) {
                return false;
            }
            if (c == 0xE0 && static_cast<unsigned char>(s[i + 1]) < 0xA0) {
                return false;
            }
            if (c == 0xED && static_cast<unsigned char>(s[i + 1]) >= 0xA0) {
                return false;
            }
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            if (i + 3 >= len || (static_cast<unsigned char>(s[i + 1]) & 0xC0) != 0x80 ||
                (static_cast<unsigned char>(s[i + 2]) & 0xC0) != 0x80 ||
                (static_cast<unsigned char>(s[i + 3]) & 0xC0) != 0x80) {
                return false;
            }
            if (c == 0xF0 && static_cast<unsigned char>(s[i + 1]) < 0x90) {
                return false;
            }
            if (c == 0xF4 && static_cast<unsigned char>(s[i + 1]) > 0x8F) {
                return false;
            }
            if (c > 0xF4) {
                return false;
            }
            i += 4;
        } else {
            return false;
        }
    }
    return true;
}

#endif