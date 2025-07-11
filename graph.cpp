#include "graph.h"
#include "permutation.h"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <set>

Graph::Graph(std::vector<std::vector<int>> neighbors)
    : m_neighbors{std::move(neighbors)} {
}

int Graph::m() const {
    int edge_count = 0;
    for (int i = 1; i <= n(); i++) {
        edge_count += m_neighbors[i].size();
    }
    return edge_count / 2; // each edge is counted twice
}

std::string Graph::simple_encode() const {
    std::string out = std::to_string(n()) + ":";
    for (int i = 1; i <= n(); i++) {
        for (int x : m_neighbors[i]) {
            out += std::to_string(x) + ",";
        }
        out += ";";
    }
    return out;
}

/**
 * Process a substring of integers separated by commas and ending with a semicolon.
 * Example input: "11,17,3,43;"
 * @param input The input string containing the substring with integers.
 * @param position The starting position in the input substring.
 * @param output A set pointer to store the processed integers.
 * @return The position in the input string the semicolon at the end of the 
 *         processed substring.
 */
int process_comma_semicolon(const std::string& input, size_t position, std::vector<int>* output);

Graph simple_decode(const std::string& encoded) {
    std::vector<std::vector<int>> neighbors;
    int n; // number of vertices
    sscanf(encoded.c_str(), "%d:", &n);
    neighbors.resize(n + 1); // padded to use 1-based indexing
    int spos = encoded.find(":") + 1; // position in string
    for (int i = 1; i <= n; i++) {
        spos = process_comma_semicolon(encoded, spos, &neighbors[i]);
    }
    return Graph(neighbors);
}

bool Graph::operator==(const Graph& other) const {
    if (n() != other.n()) return false;
    for (int i = 1; i <= n(); i++) {
        if (neighbors()[i].size() != other.neighbors()[i].size()) return false;
        // Only works for simple graphs.
        std::set<int> set1(neighbors()[i].begin(), neighbors()[i].end());
        for (int n2 : other.neighbors()[i]) {
            if (set1.count(n2) == 0) {
                return false;
            }
        }
    }
    return true;
}

std::string Graph::encode_dense_adjacency(const std::vector<std::vector<int>>& cyclic_decomposition) const {
    std::string out = "";
    int k = cyclic_decomposition.size();
    for (int j = 0; j < k; j++) {
        for (int i = 0; i <= j; i++) {
            // Take the first node of the i-th cycle / orbit and compute the 
            // edges to the j-th cycle.
            int source = cyclic_decomposition[i][0];
            std::set<int> source_neighbors(neighbors()[source].begin(), neighbors()[source].end());
            int target_i = 0;
            bool first = true;
            for (int target : cyclic_decomposition[j]) {
                if (source_neighbors.count(target) > 0) {
                    if (!first) out += ",";
                    first = false;
                    out += std::to_string(target_i);
                }
                target_i++;
            }
            out += ";";
        }
    }
    return out;
}

std::string Graph::encode_sparse_adjacency(const std::vector<std::vector<int>>& cyclic_decomposition) const {
    std::string out = "";
    int k = cyclic_decomposition.size();
    for (int j = 1; j <= k; j++) {
        for (int i = 1; i <= j; i++) {
            // Same as encode_dense_adjacency, but we skip cases where the source node
            // does not have any neighbors in the target cycle.
            int source = cyclic_decomposition[i-1][0];
            std::vector<int> deltas;
            int target_i = 0;
            for (int target : cyclic_decomposition[j-1]) {
                if (std::find(neighbors()[source].begin(), neighbors()[source].end(), target) != neighbors()[source].end()) {
                    deltas.push_back(target_i);
                }
                target_i++;
            }
            if (!deltas.empty()) {
                out += std::to_string(i) + "," + std::to_string(j) + ".";
                for (size_t l = 0; l < deltas.size(); l++) {
                    if (l > 0) out += ",";
                    out += std::to_string(deltas[l]);
                }
                out += ";";
            }
        }
    }
    return out;
}

std::string Graph::encode(const Permutation& automorphism, bool sparse) const {
    std::vector<std::vector<int>> cyclic_decomposition = automorphism.cyclic_decomposition();
    std::string out = std::to_string(n()) + ":" + std::to_string(cyclic_decomposition.size()) + ":";
    if (sparse)
        out += "s:";
    else
        out += "d:";
    if (sparse) {
        out += encode_sparse_adjacency(cyclic_decomposition);
    }
    else {
        out += encode_dense_adjacency(cyclic_decomposition);
    }
    return out;
}

// Function documentation is above the forward declaration.
int process_comma_semicolon(const std::string& input, size_t position, std::vector<int>* output) {
    if (input[position] == ';')
        return position + 1; // No numbers to process
    while (1) {
        int value;
        sscanf(input.c_str() + position, "%d", &value);
        output->push_back(value);
        size_t next_comma = input.find(",", position);
        size_t next_semicolon = input.find(";", position);
        if (next_comma == std::string::npos) position = next_semicolon + 1;
        else position = std::min(next_comma, next_semicolon) + 1;
        if (position == next_semicolon + 1) {
            break; // We reached the end of the current set
        }
    }
    return position;
}

/**
 * Computes the modulus of x, guaranteeing that the result is in the range [1, m].
 * @param x The integer to compute the modulus for.
 * @param m The modulus value.
 * @return The modulus of x in the range [1, m].
 */
int mod_index_1(int x, int m) {
    if (x % m == 0) {
        return m;
    } else {
        // Works for both positive and negative x, since -7 % 5 = -2.
        return ((x % m) + m) % m;
    }
}

Graph decode(const std::string& encoded) {
    int n, k; // n = number of vertices, k = number of orbits
    sscanf(encoded.c_str(), "%d:%d:", &n, &k);
    int m = n / k; // m = size of each orbit
    assert(k * m == n);
    bool sparse = (encoded.find(":s:") != std::string::npos);
    size_t spos;
    if (sparse) {
        spos = encoded.find(":s:") + 3;
    } else {
        spos = encoded.find(":d:") + 3;
    }

    std::vector<std::vector<std::vector<int>>> deltas_matrix(n + 1);
    for (int i = 1; i <= n; i++) {
        deltas_matrix[i].resize(n + 1);
    }
    if (sparse)
    {
        while (spos < encoded.size()) {
            int i, j;
            sscanf(encoded.c_str() + spos, "%d,%d.", &i, &j);
            spos = encoded.find(".", spos) + 1;
            spos = process_comma_semicolon(encoded, spos, &deltas_matrix[i][j]);
        }
    }
    else {
        for (int j = 1; j <= k; j++) {
            for (int i = 1; i <= j; i++) {
                spos = process_comma_semicolon(encoded, spos, &deltas_matrix[i][j]);
            }
        }
    }

    // The indexing is based on the cyclic decomposition:
    // first orbit in order, second orbit in order, ...
    std::vector<std::vector<int>> neighbors(n + 1);
    // Generate neighbors list based on deltas_matrix
    // source_o_i, target_o_i are the indices of the orbits in the cyclic decomposition
    for (int source_o_i = 1; source_o_i <= k; source_o_i++) {
        for (int target_o_i = 1; target_o_i <= k; target_o_i++) {
            // deltas_matrix only has the upper triangle, but we want our neighbors list to be 'symmetric'
            if (source_o_i <= target_o_i) {
                for (int x : deltas_matrix[source_o_i][target_o_i]) {
                    for (int i = 1; i <= m; i++) { // i = vertex index in the orbit
                        neighbors[(source_o_i - 1) * m + i].push_back((target_o_i - 1) * m + mod_index_1(i + x, m));
                    }
                }
            }
            else {
                for (int x : deltas_matrix[target_o_i][source_o_i]) {
                    for (int i = 1; i <= m; i++) { // i = vertex index in the orbit
                        // If the delta from source to target is x, then the delta from target to source is -x!
                        neighbors[(source_o_i - 1) * m + i].push_back((target_o_i - 1) * m + mod_index_1(i - x, m));
                    }
                }
            }
        }
    }

    return neighbors;
}

void Graph::apply_morphism(const Permutation& morphism) {
    Permutation inv_morphism = morphism.inverse();
    std::set<int> visited;
    // Permute the outer vector of the neighbors list according to the morphism.
    for (int i = 1; i <= n(); i++) {
        // (1 2)(2 3)(3 4) = (1 4 3 2) = (1 2 3 4)^(-1)
        while (visited.count(i) == 0 && inv_morphism.apply(i) != i) {
            std::swap(m_neighbors[i], m_neighbors[inv_morphism.apply(i)]);
            visited.insert(i);
            i = inv_morphism.apply(i); // move to the next vertex in the cycle
            if (visited.count(inv_morphism.apply(i)) > 0) {
                visited.insert(i);
                i = inv_morphism.apply(i); // move back to the start of the cycle
                break;
            }
        }
    }
    // Apply the morphism to the neighbors of each node.
    for (int i = 1; i <= n(); i++) {
        morphism.apply(&(m_neighbors[i]));
    }
}