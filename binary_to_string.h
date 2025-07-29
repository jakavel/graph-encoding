#pragma once

#include <string>
#include <vector>

/**
 * Converts an integer in the range 0-68719476735 to a string representation,
 * using McKay's encoding:
 * https://users.cecs.anu.edu.au/~bdm/data/formats.txt
 * @param n The integer to convert.
 * @return A string representing the integer.
 */
std::string string_N(int n);

/**
 * Calculates the number of bits needed to represent an integer n in binary.
 * @param n A positive integer.
 * @return The number of bits needed to represent n.
 */
int log_2_ceil(int n);

/**
 * Converts an array of integers to a vector of bits via concatenation.
 * Each integer is represented in k bits.
 * @param k The number of bits for each integer.
 * @param array The array of integers to convert.
 * @return A vector of bits representing the integers in the array.
 */
std::vector<bool> array_to_bits(int k, const std::vector<int>& array);

/**
 * Converts a vector of bits to a string representation.
 * Each group of 6 bits is converted to a character by x -> char(x + 63).
 * @param bits The vector of bits to convert, length must be a multiple of 6.
 * @return A string representation of the bits.
 */
std::string bits_to_string(const std::vector<bool>& bits);