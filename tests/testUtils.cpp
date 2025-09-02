#include "../include/utils.hpp"
#include <cassert>
#include <iostream>
#include <string>

using namespace std;

int main() {
    cout << "Starting utils tests...\n";

    // Valid UTF-8
    string valid1 = "Hello";            // Latin
    string valid2 = "Привет";           // Cyrillic
    string valid3 = "こんにちは";       // Japanese
    string valid4 = "\xF0\x9F\x98\x81"; // 😀 emoji

    // Invalid UTF-8
    string invalid1 = "He"
                      "\x80"
                      "llo"; // Lone continuation byte
    string invalid2 = "\xC0"
                      "A";            // Overlong encoding
    string invalid3 = "\xE0\x80\x80"; // Overlong 3-byte
    string invalid4 = string(1, (char)0xF5) + string(1, (char)0x80) + string(1, (char)0x80) +
                      string(1, (char)0x80); // Codepoint > U+10FFFF

    assert(isUTF8(valid1));
    assert(isUTF8(valid2));
    assert(isUTF8(valid3));
    assert(isUTF8(valid4));

    assert(!isUTF8(invalid1));
    assert(!isUTF8(invalid2));
    assert(!isUTF8(invalid3));
    assert(!isUTF8(invalid4));

    cout << "Testing complete." << endl;
    return 0;
}