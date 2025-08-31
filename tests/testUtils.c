#include "../include/clientList.h"
#include "../include/utils.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int main() {
    printf("Starting utils tests...\n");

    // Valid UTF-8
    const char *valid1 = "Hello";            // Latin
    const char *valid2 = "Привет";           // Cyrillic
    const char *valid3 = "こんにちは";       // Japanese
    const char *valid4 = "\xF0\x9F\x98\x81"; // 😀 emoji

    // Invalid UTF-8
    const char invalid1[] = {'H', 'e', (char)0x80, 'l', 'o', 0};     // Lone continuation byte
    const char invalid2[] = {(char)0xC0, 'A', 0};                    // Overlong encoding
    const char invalid3[] = {(char)0xE0, (char)0x80, (char)0x80, 0}; // Overlong 3-byte
    const char invalid4[] = {(char)0xF5, 0x80, 0x80, 0x80, 0};       // Codepoint > U+10FFFF

    assert(isUTF8(valid1, strlen(valid1)) == true);
    assert(isUTF8(valid2, strlen(valid2)) == true);
    assert(isUTF8(valid3, strlen(valid3)) == true);
    assert(isUTF8(valid4, 4) == true);

    assert(!isUTF8(invalid1, sizeof(invalid1) - 1));
    assert(!isUTF8(invalid2, sizeof(invalid2) - 1));
    assert(!isUTF8(invalid3, sizeof(invalid3) - 1));
    assert(!isUTF8(invalid4, sizeof(invalid4) - 1));

    // Test getUserNameFromFD
    struct clientNode *head = NULL;
    head = addClient(head, 1);
    strncpy(head->username, "UserA", sizeof(head->username));
    head->username[sizeof(head->username) - 1] = '\0';

    char *username = getUserNameFromFD(head, 1);
    assert(username != NULL);
    assert(strcmp(username, "UserA") == 0);

    printf("Testing complete.\n");
    return 0;
}