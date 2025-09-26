#include "../include/utils.hpp"
#include <gtest/gtest.h>
#include <string>

using namespace std;

class UtilsTest : public ::testing::Test {};

TEST_F(UtilsTest, SingleByteUTF8) {
    string singleByte = "A";
    EXPECT_TRUE(isUTF8(singleByte));
}

TEST_F(UtilsTest, MultiByteUTF8) {
    string multiByte = "€";
    EXPECT_TRUE(isUTF8(multiByte));
}

TEST_F(UtilsTest, ValidUTF8) {
    // Valid UTF-8
    string valid1 = "Hello";            // Latin
    string valid2 = "Привет";           // Cyrillic
    string valid3 = "こんにちは";       // Japanese
    string valid4 = "\xF0\x9F\x98\x81"; // 😀 emoji

    EXPECT_TRUE(isUTF8(valid1));
    EXPECT_TRUE(isUTF8(valid2));
    EXPECT_TRUE(isUTF8(valid3));
    EXPECT_TRUE(isUTF8(valid4));
}

TEST_F(UtilsTest, InvalidUTF8) {
    // Invalid UTF-8
    string invalid1 = "He"
                      "\x80"
                      "llo"; // Lone continuation byte
    string invalid2 = "\xC0"
                      "A";            // Overlong encoding
    string invalid3 = "\xE0\x80\x80"; // Overlong 3-byte
    string invalid4 = string(1, (char)0xF5) + string(1, (char)0x80) + string(1, (char)0x80) +
                      string(1, (char)0x80); // Codepoint > U+10FFFF

    EXPECT_FALSE(isUTF8(invalid1));
    EXPECT_FALSE(isUTF8(invalid2));
    EXPECT_FALSE(isUTF8(invalid3));
    EXPECT_FALSE(isUTF8(invalid4));
}

TEST_F(UtilsTest, EmptyString) { EXPECT_TRUE(isUTF8("")); }

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}