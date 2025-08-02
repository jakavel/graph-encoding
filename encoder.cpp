#include "graph.h"
#include "permutation.h"
#include "binary_to_string.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <set>
#include <gtools.h>
using namespace std;

ifstream infile("CubicATAut.txt");
FILE *s6_graphs_file;

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

int main(int argc, char *argv[]) {
    string line;
    if (!infile.is_open()) {
        cerr << "Error opening CubicATAut.txt." << endl;
        return 1;
    }
    s6_graphs_file = fopen("s6_graphs.txt", "w");
    if (!s6_graphs_file) {
        cerr << "Error opening s6_graphs.txt." << endl;
        return 1;
    }
    ofstream outfile("enc_graphs.txt");
    if (!outfile.is_open()) {
        cerr << "Error opening enc_graphs.txt." << endl;
        return 1;
    }
    int max_graph = 1000;
    if (argc > 1) {
        max_graph = atoi(argv[1]);
        if (max_graph <= 0) {
            cerr << "Invalid maximum graph number specified." << endl;
            return 1;
        }
    }
    for (int graph_i = 0; graph_i <= max_graph; graph_i++) {
        if (!getline(infile, line)) {
            cout << "End of file reached, read " << graph_i << " graphs" << endl;
            break; // end of file
        }
        int n, n_index, n_of_automorphisms;
        sscanf(line.c_str(), "%d,%d,%d", &n, &n_index, &n_of_automorphisms);

        getline(infile, line);
        int n2; sscanf(line.c_str(), "%d", &n2);
        assert(n == n2);
        Graph graph = simple_decode(line);

        vector<Permutation> automorphisms;
        automorphisms.reserve(n_of_automorphisms);
        for (int i = 0; i < n_of_automorphisms; i++) {
            getline(infile, line);
            replace(line.begin(), line.end(), ',', ' ');
            istringstream iss(line);
            vector<int> perm;
            for (int j = 0; j < n; j++) {
                int x;
                iss >> x;
                perm.push_back(x);
            }
            automorphisms.push_back(Permutation(perm));
        }

        for (const Permutation& automorphism : automorphisms) {
            /* vector<int> isomorphism;
            isomorphism.reserve(n);
            for (vector<int>& cycle : automorphism.cyclic_decomposition()) {
                isomorphism.insert(isomorphism.end(), cycle.begin(), cycle.end());
            }
            Permutation isomorphism_perm(isomorphism);
            Graph new_graph = decode(graph.encode(automorphism, true));
            new_graph.apply_morphism(isomorphism_perm);
            if (graph != new_graph) {
                cout << "Graph mismatch with sparse encoding!" << endl;
                cout << "Graph neighbors:" << endl;
                cout << "n=" << graph.n() << ", automorphism=" << automorphism.cyclic_decomposition_string() << endl;
                cout << "Original graph neighbors:" << endl;
                print_neighbors(graph.neighbors());
                cout << "New graph neighbors:" << endl;
                print_neighbors(new_graph.neighbors());
                return 1;
            }
            assert(graph == new_graph); */
            outfile << graph.encode(automorphism, true) << endl;
            sparsegraph s6_graph = graph.to_sparsegraph();
            writes6_sg(s6_graphs_file, &s6_graph);
            free(s6_graph.v);
            free(s6_graph.d);
            free(s6_graph.e);

        }
        if (graph_i % 100 == 0 && graph_i > 0) {
            cout << "Processed graph " << graph_i  << endl;
        }
    }

    outfile.close();
    infile.close();
    fclose(s6_graphs_file);
    cout << "All graphs processed successfully." << endl;
    return 0;
}