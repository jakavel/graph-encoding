#pragma once

#include <vector>
#include <string>

class Permutation {
public:
    /**
     * Constructs a permutation from a vector of integers.
     * The vector must contain integers from 1 to n, where n is the size of the vector.
     * @param perm A vector representing the permutation.
     */
    Permutation(std::vector<int> perm);
    /**
     * Returns the size of the permutation.
     * @return The number of elements in the permutation.
     */
    int n() const;
    /**
     * Applies the permutation to an integer.
     * @param x The integer to apply the permutation to (1-based index).
     * @return The permuted integer.
     */
    int apply(int x) const;
    /**
     * Applies the permutation to a vector of integers.
     * @param vec A pointer to a vector of integers to apply the permutation to.
     */
    void apply(std::vector<int>* vec) const;
    /**
     * Computes the inverse of the permutation.
     * @return A new Permutation object representing the inverse permutation.
     */
    Permutation inverse() const;
    /**
     * Computes the cyclic decomposition of the permutation.
     * @return A vector of vectors, where each inner vector represents a cycle in the permutation.
     */
    std::vector<std::vector<int>> cyclic_decomposition() const;
    /**
     * Encodes the permutation as a human-readable string.
     * @return A string representation of the cyclic decomposition.
     */
    std::string cyclic_decomposition_string() const;

private:
    std::vector<int> m_perm;
};