#pragma once

#include <string>
#include <vector>

std::string string_N(int n);

int log_2_ceil(int n);

std::vector<bool> array_to_bits(int k, const std::vector<int>& array);

std::string bits_to_string(const std::vector<bool>& bits);