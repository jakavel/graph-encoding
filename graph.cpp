#include "graph.h"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <set>

Graph::Graph(std::vector<std::vector<int>> neighbors) {
    m_neighbors = std::move(neighbors);
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

Graph simple_decode(const std::string& encoded) {
    // Decodes a string of the form "n:n_11,n_12,...;n_21,n_22,...;..."
    // where n = number of vertices, n_ij = j-th neighbor of vertex i
    std::vector<std::vector<int>> neighbors;
    int n;
    sscanf(encoded.c_str(), "%d:", &n);
    neighbors.resize(n + 1); // 1-based indexing
    int spos = encoded.find(":") + 1;
    for (int i = 1; i <= n; i++) {
        int next_semicolon = encoded.find(";", spos);
        std::string neighbors_str = encoded.substr(spos, next_semicolon - spos);
        std::replace(neighbors_str.begin(), neighbors_str.end(), ',', ' ');
        std::istringstream iss(neighbors_str);
        int neighbor;
        while (iss >> neighbor) {
            neighbors[i].push_back(neighbor);
        }
        spos = next_semicolon + 1;
    }
    return Graph(neighbors);
}

bool Graph::operator==(const Graph& other) const {
    // checks if two graphs are identical
    if (n() != other.n()) return false;
    for (int i = 1; i <= n(); i++) {
        if (neighbors()[i].size() != other.neighbors()[i].size()) return false;
        std::set<int> set1(neighbors()[i].begin(), neighbors()[i].end());
        for (int n2 : other.neighbors()[i]) {
            if (set1.count(n2) == 0) {
                return false; // found a neighbor in neighbors2 that is not in neighbors1
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

std::vector<std::vector<int>> get_cyclic_decomposition(const std::vector<int>& automorphism) {
    int n = automorphism.size();
    std::vector<bool> visited(n + 1, false);
    std::vector<std::vector<int>> decomposition;

    for (int i = 1; i <= n; i++) {
        if (!visited[i]) {
            std::vector<int> cycle;
            int current = i;
            do {
                visited[current] = true;
                cycle.push_back(current);
                current = automorphism[current - 1]; // automorphism is 0-based, but we use 1-based indexing
            } while (current != i);
            decomposition.push_back(cycle);
        }
    }
    return decomposition;
}

std::string Graph::encode(const std::vector<int>& automorphism, bool sparse) const {
    std::vector<std::vector<int>> cyclic_decomposition = get_cyclic_decomposition(automorphism);
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

int process_comma_semicolon(const std::string& input, size_t position, std::set<int>* output) {
    // Process a string of type "11,17,3,43;" and add the integers to the output set
    // returns the position after the semicolon
    if (input[position] == ';')
        return position + 1; // No numbers to process
    while (1) {
        int value;
        sscanf(input.c_str() + position, "%d", &value);
        output->insert(value);
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

int mod_index_1(int x, int m) {
    // Returns the modulus in the range [1, m] for a given x
    if (x % m == 0) {
        return m;
    } else if (x > 0) {
        return x % m;
    } else {
        return ((x % m) + m) % m;
    }
}

Graph decode(const std::string& encoded) {
    int n, k; // n = number of vertices, k = number of orbits
    sscanf(encoded.c_str(), "%d:%d:", &n, &k);
    int m = n / k; // m = size of orbit
    assert(k * m == n);
    bool sparse = (encoded.find(":s:") != std::string::npos);
    size_t spos;
    if (sparse) {
        spos = encoded.find(":s:") + 3;
    } else {
        spos = encoded.find(":d:") + 3;
    }

    std::vector<std::vector<std::set<int>>> deltas_matrix(n + 1);
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

    // The indexing is first orbit, second orbit, ...
    std::vector<std::vector<int>> neighbors(n + 1);
    // source orbit index, target orbit index
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
                        neighbors[(source_o_i - 1) * m + i].push_back((target_o_i - 1) * m + mod_index_1(i - x, m));
                    }
                }
            }
        }
    }

    return neighbors;
}

std::vector<int> inverse(const std::vector<int>& morphism) {
    // returns the inverse of the morphism
    int n = morphism.size();
    std::vector<int> inv(n);
    for (int i = 0; i < n; i++) {
        inv[morphism[i] - 1] = i + 1; // morphism is 1-based, so we adjust accordingly
    }
    return inv;
}

void Graph::apply_morphism(const std::vector<int>& morphism) {
    for (int x : morphism) {
        assert(x >= 1 && x <= n());
    }
    std::vector<int> inv_morphism = inverse(morphism);
    std::set<int> visited;
    for (int i = 1; i <= n(); i++) {
        // (1 2)(2 3)(3 4) = (1 4 3 2) = (1 2 3 4)^(-1)
        while (visited.count(i) == 0 && inv_morphism[i-1] != i) {
            std::swap(m_neighbors[i], m_neighbors[inv_morphism[i-1]]);
            visited.insert(i);
            i = inv_morphism[i-1]; // move to the next vertex in the cycle
            if (visited.count(inv_morphism[i-1]) > 0) {
                visited.insert(i);
                i = inv_morphism[i-1]; // move back to the start of the cycle
                break;
            }
        }
    }
    for (int i = 1; i <= n(); i++) {
        for (size_t j = 0; j < m_neighbors[i].size(); j++) {
            // apply the morphism to the neighbors
            m_neighbors[i][j] = morphism[m_neighbors[i][j]-1];
        }
    }
}