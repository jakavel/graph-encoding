#pragma once

#include <vector>
#include <string>

class Graph {
private:
    std::vector<std::vector<int>> m_neighbors;

public:
    const std::vector<std::vector<int>>& neighbors() const {
        return m_neighbors;
    }
    int n() const {
        return m_neighbors.size() - 1; // index 0 is not a node
    }

    Graph(std::vector<std::vector<int>> neighbors);
    bool operator==(const Graph& other) const;
    bool operator!=(const Graph& other) const {
        return !(*this == other);
    }

    std::string simple_encode() const;
    std::string encode(const std::vector<int>& automorphism, bool sparse) const;
    void apply_morphism(const std::vector<int>& morphism);

private:
    std::string encode_dense_adjacency(const std::vector<std::vector<int>>& cyclic_decomposition) const;
    std::string encode_sparse_adjacency(const std::vector<std::vector<int>>& cyclic_decomposition) const;

};

Graph simple_decode(const std::string& str);
Graph decode(const std::string& str);

std::vector<std::vector<int>> get_cyclic_decomposition(const std::vector<int>& automorphism);