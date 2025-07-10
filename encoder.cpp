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
    ofstream outfile("compression_test.csv");
    if (!outfile.is_open()) {
        cerr << "Error opening compression_test.csv." << endl;
        return 1;
    }
    outfile << "graph_i,num_vertices,num_edges,num_orbits,simple_encoding_length,dense_encoding_length,sparse_encoding_length" << endl;
    for (int graph_i = 0; graph_i <= 2000; graph_i++) {
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

        for (const vector<int>& automorphism : automorphisms) {
            outfile << graph_i << "," << graph.n() << "," << graph.m() << "," << get_cyclic_decomposition(automorphism).size() << ","
                    << graph.simple_encode().length() << ","
                    << graph.encode(automorphism, false).length() << ","
                    << graph.encode(automorphism, true).length() << endl;
        }
        if (graph_i % 100 == 0 && graph_i > 0) {
            cout << "Processed graph " << graph_i  << endl;
        }
    }

    outfile.close();
    infile.close();

    return 0;
}