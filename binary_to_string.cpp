#include "binary_to_string.h"
#include <string>
#include <cassert>

char charify(int n) {
    n = n % 64;
    return (char)(n + 63);
}

std::string string_N(int n) {
    assert(n >= 0);
    assert(n <= 68719476735);
    if (n <= 62) {
        std::string s = "1";
        s[0] = (char) (n + 63);
        return s;
    }
    if (n <= 258047) {
        std::string s = "1234";
        s[0] = (char) 126;
        s[1] = charify(n >> 12);
        s[2] = charify(n >> 6);
        s[3] = charify(n);
        return s;
    }
    else {
        std::string s = "12345678";
        s[0] = (char) 126;
        s[1] = (char) 126;
        s[2] = charify(n >> 30);
        s[3] = charify(n >> 24);
        s[4] = charify(n >> 18);
        s[5] = charify(n >> 12);
        s[6] = charify(n >> 6);
        s[7] = charify(n);
        return s;
    }
}