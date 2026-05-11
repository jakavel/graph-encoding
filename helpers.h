#pragma once
#include <vector>
#include <string>
#include <tuple>

/**
 * Process a substring of integers separated by commas and ending with a terminator.
 * If the string is missing a terminator at the end, it will be processed until the end of the string.
 * Example input: "11,17,3,43;"
 * @param input The input string containing the substring with integers.
 * @param position The starting position in the input substring.
 * @param terminator The character that marks the end of the substring (e.g., ';' or ':').
 * @param output A set pointer to store the processed integers.
 * @return The position in the input string the terminator at the end of the 
 *         processed substring.
 */
int process_csv(const std::string& input, size_t position, char terminator, std::vector<int>* output);

/**
 * Computes the modulus of x, guaranteeing that the result is in the range [1, m].
 * @param x The integer to compute the modulus for.
 * @param m The modulus value.
 * @return The modulus of x in the range [1, m].
 */
int mod_index_1(int x, int m);

/**
 * Reads k bits from the string s starting at the position specified by pos,
 * where each character c represents the 6 bits if (c-63) and pos={a, b},
 * where 0 <= a < length(s) and 0 <= b < 6.
 * Function also modifies pos to point to the next character after the k bits.
 * @param s The string to read bits from.
 * @param pos A tuple containing the current position in the string and the bit position within the character.
 * @return The bits read from the string as an integer, or -1 if there are not enough bits left.
*/
int read_k_bits(const std::string& s, int k, std::tuple<int, int>& pos);