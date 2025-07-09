#include "Graph.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <set>
using namespace std;

ifstream infile("CubicATsemiregAut.txt");

void print_neighbors(const vector<vector<int>>& neighbors) {
    // prints the neighbors of the graph
    for (int i = 1; i < (int)neighbors.size(); i++) {
        cout << i << ": ";
        vector<int> sorted_neighbors = neighbors[i];
        sort(sorted_neighbors.begin(), sorted_neighbors.end());
        for (int j = 0; j < (int)sorted_neighbors.size(); j++) {
            cout << sorted_neighbors[j] << " ";
        }
        cout << endl;
    }
}

int main() {
    string line;
    if (!infile.is_open()) {
        cerr << "Error opening CubicATsemiregAut.txt." << endl;
        return 1;
    }
    for (int graph_i = 0; 1; graph_i++) {
        getline(infile, line);
        if (line.empty()) {
            cout << "End of file reached, graph_i = " << graph_i << endl;
            break; // end of file
        }
        int n, n_index, n_of_automorphisms;
        sscanf(line.c_str(), "%d,%d,%d", &n, &n_index, &n_of_automorphisms);

        getline(infile, line);
        int n2; sscanf(line.c_str(), "%d", &n2);
        assert(n == n2);
        Graph graph = simple_decode(line);

        vector<vector<int>> automorphisms(n_of_automorphisms);
        for (int i = 0; i < n_of_automorphisms; i++) {
            getline(infile, line);
            replace(line.begin(), line.end(), ',', ' ');
            istringstream iss(line);
            for (int j = 0; j < n; j++) {
                int x;
                iss >> x;
                automorphisms[i].push_back(x);
            }
        }
        vector<int> automorphism = automorphisms[0];

        Graph dense_check_graph = decode(graph.encode(automorphism, false));
        Graph sparse_check_graph = decode(graph.encode(automorphism, true));
        vector<vector<int>> cyclic_decomposition = get_cyclic_decomposition(automorphism);
        vector<int> isomorphism;
        for (vector<int> cycle : cyclic_decomposition) {
            isomorphism.insert(isomorphism.end(), cycle.begin(), cycle.end());
        }

        dense_check_graph.apply_morphism(isomorphism);
        sparse_check_graph.apply_morphism(isomorphism);

        if (graph != dense_check_graph)
            cout << "Dense is NOT identical, graph_i = " << graph_i << endl;
        if (graph != sparse_check_graph)
            cout << "Sparse is NOT identical, graph_i = " << graph_i << endl;
        if (graph_i % 100 == 0 && graph_i > 0 && graph == dense_check_graph && graph == sparse_check_graph)
            cout << "All identical up to graph_i = " << graph_i << endl;
    }

    infile.close();

    return 0;
}