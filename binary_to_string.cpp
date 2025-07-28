#include "binary_to_string.h"
#include <string>
#include <cassert>
#include <vector>

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

int log_2_ceil(int n) {
    assert(n > 0);
    int log = 0;
    while ((1 << log) <= n) {
        log++;
    }
    return log;
}

std::vector<bool> array_to_bits(int k, const std::vector<int>& array) {
    assert(k > 0);
    std::vector<bool> bits;
    bits.reserve(k * array.size());
    for (int x : array) {
        assert(x >= 0 && (x >> k) == 0); // x must fit in k bits
        for (int i = k - 1; i >= 0; i--) {
            bits.push_back(x & (1 << i));
        }
    }
    return bits;
}

std::string bits_to_string(const std::vector<bool>& bits) {
    assert(bits.size() % 6 == 0);
    std::string out;
    out.reserve(bits.size() / 6);
    for (size_t i = 0; i < bits.size(); i += 6) {
        int n = 0;
        for (int j = 0; j < 6; j++) {
            n |= (bits[i + j] << (5 - j));
        }
        out += char(n + 63);
    }
    return out;
}