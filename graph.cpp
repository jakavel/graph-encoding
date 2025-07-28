#include "graph.h"
#include "permutation.h"
#include "binary_to_string.h"
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <set>
#include <numeric>

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
 * @param terminator The character that marks the end of the substring (e.g., ';' or ':').
 * @param output A set pointer to store the processed integers.
 * @return The position in the input string the semicolon at the end of the 
 *         processed substring.
 */
int process_csv(const std::string& input, size_t position, char terminator, std::vector<int>* output);

Graph simple_decode(const std::string& encoded) {
    std::vector<std::vector<int>> neighbors;
    int n; // number of vertices
    sscanf(encoded.c_str(), "%d:", &n);
    neighbors.resize(n + 1); // padded to use 1-based indexing
    int spos = encoded.find(":") + 1; // position in string
    for (int i = 1; i <= n; i++) {
        spos = process_csv(encoded, spos, ';', &neighbors[i]);
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
            int m = std::gcd(cyclic_decomposition[i].size(), cyclic_decomposition[j].size());
            int target_i = 0;
            std::set<int> deltas;
            for (int target : cyclic_decomposition[j]) {
                if (source_neighbors.count(target) > 0) {
                    deltas.insert(target_i % m);
                }
                target_i++;
            }
            bool first = true;
            for (int delta : deltas) {
                if (!first) out += ",";
                first = false;
                out += std::to_string(delta);
            }
            out += ";";
        }
    }
    return out;
}

std::string Graph::encode_sparse_adjacency(const std::vector<std::vector<int>>& cyclic_decomposition) const {
    std::string out = "";
    int k = cyclic_decomposition.size();
    std::vector<bool> edges_bits;
    int v = 1; // current position as a vertex in the quotient graph (that is cycle in the cyclic decomposition)
    int b_k = log_2_ceil(k);
    for (int i = 1; i <= k; i++) {
        for (int j = 1; j <= i; j++) {
            int source = cyclic_decomposition[i-1][0];
            int m = std::gcd(cyclic_decomposition[i-1].size(), cyclic_decomposition[j-1].size());
            std::set<int> deltas;
            int target_i = 0;
            for (int target : cyclic_decomposition[j-1]) {
                if (std::find(neighbors()[source].begin(), neighbors()[source].end(), target) != neighbors()[source].end()) {
                    deltas.insert(target_i % m);
                }
                target_i++;
            }
            if (!deltas.empty()) {
                if (v != i) {
                    // move the current position to the source cycle
                    edges_bits.push_back(0);
                    for (int l = b_k - 1; l >= 0; l--) {
                        edges_bits.push_back(i & (1 << l));
                    }
                    v = i;
                }
                // Now that v is correct, add the edge to the target cycle.
                edges_bits.push_back(0);
                for (int l = b_k - 1; l >= 0; l--) {
                    edges_bits.push_back(j & (1 << l));
                }
                int b_ij = log_2_ceil(m);
                for (int delta : deltas) {
                    edges_bits.push_back(1);
                    for (int l = b_ij - 1; l >= 0; l--) {
                        edges_bits.push_back(delta & (1 << l));
                    }
                }
            }
        }
    }
    if (edges_bits.size() % 6 != 0) {
        int padding = 6 - (edges_bits.size() % 6);
        edges_bits.resize(edges_bits.size() + padding, false);
    }
    out += bits_to_string(edges_bits);
    return out;
}

std::string Graph::encode(const Permutation& automorphism, bool sparse) const {
    std::vector<std::vector<int>> cyclic_decomposition = automorphism.cyclic_decomposition();
    std::string out = "::" + string_N(n());
    int k = cyclic_decomposition.size();
    std::vector<std::tuple<int, int>> cycle_sizes; // (number of cycles, size of those cycles)
    cycle_sizes.reserve(k);
    int counter = 0;
    int multi_cycles = 0; // Count for how many lengths there are multiple cycles of that length.
    int single_cycles = 0;
    for (int i = 0; i < k; i++) {
        counter++;
        if (i != k-1 && cyclic_decomposition[i].size() == cyclic_decomposition[i+1].size()) {
            continue;
        } else {
            cycle_sizes.emplace_back(counter, cyclic_decomposition[i].size());
            if (counter > 1) {
                multi_cycles++;
            } else {
                single_cycles++;
            }
            counter = 0;
        }
    }
    assert((int) cycle_sizes.size() == multi_cycles + single_cycles);
    int b_n = log_2_ceil(n());
    std::vector<int> cycle_sizes_encoded;
    cycle_sizes_encoded.reserve(2 + 2 * multi_cycles + single_cycles);
    int i;
    for (i = 0; i < multi_cycles; i++) {
        cycle_sizes_encoded.push_back(std::get<0>(cycle_sizes[i])); // numer of cycles of that size
        cycle_sizes_encoded.push_back(std::get<1>(cycle_sizes[i])); // size of those cycles
    }
    cycle_sizes_encoded.push_back(0);
    for (; i < (int) cycle_sizes.size(); i++) {
        cycle_sizes_encoded.push_back(std::get<1>(cycle_sizes[i])); // size of the single cycle
    }
    cycle_sizes_encoded.push_back(0);
    std::vector<bool> cycle_sizes_bits = array_to_bits(b_n, cycle_sizes_encoded);
    if (cycle_sizes_bits.size() % 6 != 0) {
        int padding = 6 - (cycle_sizes_bits.size() % 6);
        cycle_sizes_bits.resize(cycle_sizes_bits.size() + padding, false);
    }
    out += bits_to_string(cycle_sizes_bits);
    if (sparse) {
        out += encode_sparse_adjacency(cyclic_decomposition);
    }
    else {
        assert(false && "Dense encoding is not implemented yet.");
        out += encode_dense_adjacency(cyclic_decomposition);
    }
    return out;
}

// Function documentation is above the forward declaration.
int process_csv(const std::string& input, size_t position, char terminator, std::vector<int>* output) {
    if (input[position] == terminator)
        return position + 1; // No numbers to process
    while (1) {
        int value;
        sscanf(input.c_str() + position, "%d", &value);
        output->push_back(value);
        size_t next_comma = input.find(",", position);
        size_t next_terminator = input.find(terminator, position);
        if (next_comma == std::string::npos) position = next_terminator + 1;
        else position = std::min(next_comma, next_terminator) + 1;
        if (position == next_terminator + 1) {
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

/**
 * Reads k bits from the string s starting at the position specified by pos,
 * where each character c represents the 6 bits if (c-63) and pos={a, b},
 * where 0 <= a < length(s) and 0 <= b < 6.
 * Function also modifies pos to point to the next character after the k bits.
 * @param s The string to read bits from.
 * @param pos A tuple containing the current position in the string and the bit position within the character.
 * @return The bits read from the string as an integer, or -1 if there are not enough bits left.
*/
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

Graph decode(const std::string& encoded) {
    assert(encoded.find("::") == 0); // The encoded string must start with "::"
    int n; // n = number of vertices
    int s_pos = 2; // string (encoded) position
    if (encoded[s_pos] == 126 && encoded[s_pos + 1] == 126) {
        n = ((encoded[s_pos + 2] - 63) << 30) +
            ((encoded[s_pos + 3] - 63) << 24) +
            ((encoded[s_pos + 4] - 63) << 18) +
            ((encoded[s_pos + 5] - 63) << 12) +
            ((encoded[s_pos + 6] - 63) << 6) +
            (encoded[s_pos + 7] - 63);
        s_pos = s_pos + 8; // Move past the n value
    }
    else if (encoded[s_pos] == 126) {
        n = ((encoded[s_pos + 1] - 63) << 12) +
            ((encoded[s_pos + 2] - 63) << 6) +
            (encoded[s_pos + 3] - 63);
        s_pos += 4; // Move past the n value
    }
    else {
        n = int(encoded[s_pos] - 63);
        s_pos += 1; // Move past the n value
    }

    std::vector<int> cycle_sizes;
    std::tuple<int, int> pos(s_pos, 0); // position in the string and bit position within the character
    int b_n = log_2_ceil(n);
    int factor = -1;
    int cycle_size = -1;
    bool multi_cycles = true;
    while (1) {
        int x = read_k_bits(encoded, b_n, pos);
        assert(x != -1);
        if (x == 0 && multi_cycles == true) {
            multi_cycles = false; // No more multi-cycles, now single cycles
            continue;
        }
        else if (x == 0 && multi_cycles == false) {
            break; // No more cycles
        }
        if (multi_cycles) {
            if (factor == -1) {
                factor = x; // The number of cycles of that size
            } else {
                cycle_size = x; // The size of the cycles
                for (int i = 0; i < factor; i++) {
                    cycle_sizes.push_back(cycle_size);
                }
                // Reset for the next pair
                cycle_size = -1;
                factor = -1;
            }
        } else {
            cycle_sizes.push_back(x);
        }
    }
    if (std::get<1>(pos) > 0) {
        s_pos = std::get<0>(pos) + 1; // Move past the last character
    }
    else {
        s_pos = std::get<0>(pos); // No need to move, we are at the end of the string
    }
    int k = cycle_sizes.size();

    std::vector<std::vector<std::vector<int>>> deltas_matrix(k + 1);
    for (int i = 1; i <= k; i++) {
        deltas_matrix[i].resize(k + 1);
    }
    pos = std::make_tuple(s_pos, 0); // Reset position for reading deltas
    int b_k = log_2_ceil(k);
    int v = 1;
    int u = -1;
    int m, b_ij;
    while (1) {
        int b = read_k_bits(encoded, 1, pos);
        if (b == -1) break;
        if (b == 0) {
            int x = read_k_bits(encoded, b_k, pos);
            if (x == -1 || x == 0) break;
            if (x > v) {
                v = x;
                u = -1;
            } else {
                u = x;
                m = std::gcd(cycle_sizes[v - 1], cycle_sizes[u - 1]);
                b_ij = log_2_ceil(m);
            }
        } else {
            assert(u != -1);
            // Remember that always v >= u
            int delta = read_k_bits(encoded, b_ij, pos);
            assert(delta != -1);
            deltas_matrix[v][u].push_back(delta);
        }
    }

    // The indexing is based on the cyclic decomposition:
    // first orbit in order, second orbit in order, ...
    std::vector<std::vector<int>> neighbors(n + 1);
    // Generate neighbors list based on deltas_matrix
    // source_o_i, target_o_i are the indices of the orbits in the cyclic decomposition
    std::vector<int> index_starts; // cumulative sum of the sizes of the orbits
    index_starts.reserve(k + 1);
    index_starts.push_back(0);
    for (int i = 0; i < k; i++) {
        index_starts.push_back(index_starts.back() + cycle_sizes[i]);
    }
    for (int source_o_i = 1; source_o_i <= k; source_o_i++) {
        for (int target_o_i = 1; target_o_i <= k; target_o_i++) {
            // deltas_matrix only has the upper triangle, but we want our neighbors list to be 'symmetric'
            int factor; // If the delta from source to target is x, then the delta from target to source is -x!
            if (source_o_i >= target_o_i) factor = 1;
            else factor = -1;
            for (int x : deltas_matrix[std::max(target_o_i, source_o_i)][std::min(target_o_i, source_o_i)]) {
                for (int i = 1; i <= cycle_sizes[source_o_i - 1]; i++) { // i = vertex index in the source orbit
                    int s = 0;
                    do {
                        // The automorphism g^(cycle_sizes[source_o_i - 1]) fixes the source orbit,
                        // but if the size of the target orbit is different it acts non-trivially in it.
                        // Therefore each delta actually represents multiple edges specified by
                        // the subgroup cycle_sizes[source_o_i - 1] generates in Z_{cycle_sizes[target_o_i - 1]}.
                        neighbors[index_starts[source_o_i - 1] + i].push_back(
                            index_starts[target_o_i - 1] +
                            mod_index_1(i + factor*x + s, cycle_sizes[target_o_i - 1])
                        );
                        s = (s + cycle_sizes[source_o_i - 1]) % cycle_sizes[target_o_i - 1];
                    } while (s != 0);
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