#include "permutation.h"
#include <cassert>
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
    std::vector<std::vector<int>> decomposition;

    for (int i = 1; i <= n(); i++) {
        if (!visited[i]) {
            std::vector<int> cycle;
            int current = i;
            do {
                visited[current] = true;
                cycle.push_back(current);
                current = apply(current);
            } while (current != i);
            decomposition.push_back(cycle);
        }
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