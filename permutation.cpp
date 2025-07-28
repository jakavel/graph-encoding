#include "permutation.h"
#include <cassert>
#include <algorithm>
#include <string>

Permutation::Permutation(std::vector<int> perm) : m_perm(std::move(perm)) {
    for (int x : m_perm) {
        assert(x >= 1 && x <= n());
    }
}

int Permutation::n() const {
    return m_perm.size();
}

int Permutation::apply(int x) const {
    assert(x >= 1 && x <= n());
    return m_perm[x - 1];
}

void Permutation::apply(std::vector<int>* vec) const {
    for (size_t i = 0; i < vec->size(); i++) {
        assert(vec->at(i) >= 1 && vec->at(i) <= n());
        (*vec)[i] = apply(vec->at(i));
    }
}

Permutation Permutation::inverse() const {
    std::vector<int> inv_perm(n());
    for (int i = 1; i <= n(); i++) {
        inv_perm[apply(i) - 1] = i;
    }
    return Permutation(inv_perm);
}

std::vector<std::vector<int>> Permutation::cyclic_decomposition() const {
    std::vector<bool> visited(n() + 1, false);
    std::vector<std::tuple<int, int>> cycles; // (length, start)
    std::vector<std::vector<int>> decomposition;

    for (int i = 1; i <= n(); i++) {
        if (!visited[i]) {
            int current = i;
            int length = 0;
            do {
                visited[current] = true;
                length++;
                current = apply(current);
            } while (current != i);
            cycles.emplace_back(length, i);
        }
    }
    // Sort cycles by length (descending) and then by starting (minimum) point (ascending)
    auto compare = [](const std::tuple<int, int>& a, const std::tuple<int, int>& b) {
        return std::get<0>(a) > std::get<0>(b) || (std::get<0>(a) == std::get<0>(b) && std::get<1>(a) < std::get<1>(b));
    };
    std::sort(cycles.begin(), cycles.end(), compare);
    for (const auto& cycle : cycles) {
        int start = std::get<1>(cycle);
        std::vector<int> current_cycle;
        current_cycle.reserve(std::get<0>(cycle));
        int current = start;
        do {
            current_cycle.push_back(current);
            current = apply(current);
        } while (current != start);
        decomposition.push_back(current_cycle);
    }
    return decomposition;
}

std::string Permutation::cyclic_decomposition_string() const {
    std::string out;
    auto decomposition = cyclic_decomposition();
    for (const auto& cycle : decomposition) {
        out += "(";
        for (size_t i = 0; i < cycle.size(); i++) {
            if (i > 0) out += ",";
            out += std::to_string(cycle[i]);
        }
        out += ")";
    }
    return out;
}