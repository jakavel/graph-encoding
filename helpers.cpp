#include "helpers.h"
#include <vector>
#include <string>
#include <tuple>

int mod_index_1(int x, int m) {
    if (x % m == 0) {
        return m;
    } else {
        // Works for both positive and negative x, since -7 % 5 = -2.
        return ((x % m) + m) % m;
    }
}

int process_csv(const std::string& input, size_t position, char terminator, std::vector<int>* output) {
    if (input[position] == terminator)
        return position + 1; // No numbers to process
    while (1) {
        int value;
        sscanf(input.c_str() + position, "%d", &value);
        output->push_back(value);
        size_t next_comma = input.find(",", position);
        size_t next_terminator = input.find(terminator, position);
        if (next_comma == std::string::npos) {
            if (next_terminator == std::string::npos) {
                break; // No more numbers to process, but string is not ended with a terminator.
                // This is fine, since the end of a string can be considered as a terminator.
            }
            position = next_terminator + 1;
        }
        else position = std::min(next_comma, next_terminator) + 1;
        if (position == next_terminator + 1) {
            break; // We reached the end of the current set
        }
    }
    return position;
}

int read_k_bits(const std::string& s, int k, std::tuple<int, int>& pos) {
    int a = std::get<0>(pos);
    int b = std::get<1>(pos);
    int bits = 0;
    int bits_read = 0;
    while (bits_read < k) {
        if (a >= (int) s.size()) {
            return -1; // Not enough bits left
        }
        char c = s[a];
        int c_value = c - 63; // Convert character to its value
        int bits_to_read = k - bits_read;
        if (bits_to_read >= 6 - b) {
            // Read the remaining bits from the current character
            bits_to_read = 6 - b;
            int bits_mask = (1 << bits_to_read) - 1;
            bits = (bits << bits_to_read) | (c_value & bits_mask);
            bits_read += 6 - b;
            a++; // Move to the next character
            b = 0; // Reset bit position
        } else {
            // Read only part of the current character
            int bits_mask = (1 << bits_to_read) - 1;
            bits = (bits << bits_to_read) | ((c_value & (bits_mask << (6 - b - bits_to_read))) >> (6 - b - bits_to_read));
            b += bits_to_read; // Update bit position
            bits_read = k; // All bits read
        }
    }
    pos = std::make_tuple(a, b);
    return bits;
}