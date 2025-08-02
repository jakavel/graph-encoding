#pragma once

#include "permutation.h"
#include <vector>
#include <string>
#include <gtools.h>

class Graph {
public:
    /**
     *  Returns the neighbors list of the graph. The indexing is 1-based
     *  and the list is padded so that neighbors()[1] are the neighbors of node 1.
     */
    const std::vector<std::vector<int>>& neighbors() const {
        return m_neighbors;
    }
    /** @return The number of nodes in the graph. */
    int n() const {
        return m_neighbors.size() - 1; // index 0 is not a node
    }
    /** @return The number of edges in the graph. */
    int m() const;

    /** 
     * Constructor from a neighbors list. 
     * @param neighbors Neighbors list. Must be 1-based indexing and padded
     *                  so that neighbors()[1] are the neighbors of node 1.
     */
    Graph(std::vector<std::vector<int>> neighbors);


    /**
     * Checks if two graphs are identical (completely identical, not just
     * isomorphic).
     * @param other The other graph to compare against.
     * @return True if the graphs are identical, false otherwise.
     */
    bool operator==(const Graph& other) const;
    /**
     * Checks if two graphs are not identical (completely identical, not just
     * isomorphic).
     * @param other The other graph to compare against.
     * @return True if the graphs are not identical, false otherwise.
     */
    bool operator!=(const Graph& other) const {
        return !(*this == other);
    }

    /**
     * Encodes the graph as a string.
     * @return A string representation of the graph in the form
     *         "n:n_11,n_12,...;n_21,n_22,...;..."
     *         where n = number of vertices, n_ij = j-th neighbor of vertex i.
     */
    std::string simple_encode() const;
    /**
     * Encodes the graph as a string using the given automorphism.
     * @param automorphism The automorphism to use for encoding.
     * @param sparse If true, uses sparse encoding, otherwise uses dense encoding.
     * @return A string representation of the graph in the form
     *         "n:k:d/s:...".
     */
    std::string encode(const Permutation& automorphism, bool sparse) const;
    /**
     * Applies the given morphism to the graph, modifying it in place.
     * @param morphism A vector of integers representing the morphism to apply.
     *                 The i-th element of the vector is the new index for the i-th node.
     */
    void apply_morphism(const Permutation& morphism);
    /**
     * Converts the graph to the sparsegraph type from gtools / nauty.
     * @return A sparsegraph representation of the graph.
     */
    sparsegraph to_sparsegraph() const;

private:
    std::vector<std::vector<int>> m_neighbors;
    std::string encode_dense_adjacency(const std::vector<std::vector<int>>& cyclic_decomposition) const;
    std::string encode_sparse_adjacency(const std::vector<std::vector<int>>& cyclic_decomposition) const;

};

/**
 * Decodes a string of the form "n:n_11,n_12,...;n_21,n_22,...;..." into a Graph object.
 * @param str The string to decode.
 * @return A Graph object representing the decoded graph.
 */
Graph simple_decode(const std::string& str);
/**
 * Decodes a automorphism based encoding string of the form "n:k:d/s:..." 
 * into a Graph object.
 * @param str The string to decode.
 * @return A Graph object representing the decoded graph.
 */
Graph decode(const std::string& str);
/**
 * Computes the cyclic decomposition of a permutation.
 * @param permutation A vector of integers representing the permutation.
 * @return A vector of vectors, where each inner vector represents a cycle in the decomposition.
 */
std::vector<std::vector<int>> get_cyclic_decomposition(const std::vector<int>& permutation);