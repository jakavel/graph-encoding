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
#include "include/nauty/gtools.h"
#include "include/clipp.h"

std::ifstream input_file;
std::ifstream automorphisms_file;
std::ofstream output_file;

void encode_file(const std::string& input_fname, const std::string& automorphisms_fname, const std::string& output_fname, bool progr) {
    int codetype;
    bool fswitch = false; // do not assume fixed length lines
    long startline = 1; // first line (1-based)
    char* input_fname_cstr = const_cast<char*>(input_fname.c_str());
    FILE *infile = opengraphfile(input_fname_cstr, &codetype, fswitch, startline);
    if (infile == NULL) {
        std::cerr << "Error opening input file: " << input_fname << std::endl;
        return;
    }
    automorphisms_file.open(automorphisms_fname);
    if (!automorphisms_file.is_open()) {
        std::cerr << "Error opening automorphisms file: " << automorphisms_fname << std::endl;
        fclose(infile);
        return;
    }
    output_file.open(output_fname);
    if (!output_file.is_open()) {
        std::cerr << "Error opening output file: " << output_fname << std::endl;
        fclose(infile);
        automorphisms_file.close();
        return;
    }
    output_file << ">>ssg<<"; // Custom prefix to indicate our encoding format
    std::string automorphism_line;
    if (codetype & GRAPH6) {
        graph *g = NULL; // readg will allocate memory for g
        int n, m_wordsize;
        while ((g = readg(infile, NULL, 0, &m_wordsize, &n)) != NULL) {
            Graph graphObj = graph_to_Graph(g, m_wordsize, n);
            if (!std::getline(automorphisms_file, automorphism_line)) {
                std::cerr << "Error: Not enough lines in automorphisms file for the number of graphs in input file." << std::endl;
                break;
            }
            Permutation automorphism = parse_automorphism(automorphism_line);
            // non-sparse encoding not implemented
            output_file << graphObj.encode(automorphism, true) << std::endl;
            FREES(g);
        }
    }
    else if (codetype & SPARSE6) {
        sparsegraph *sg = NULL;
        while ((sg = read_sg(infile, sg)) != NULL) {
            Graph graphObj = sparsegraph_to_Graph(*sg);
            if (!std::getline(automorphisms_file, automorphism_line)) {
                std::cerr << "Error: Not enough lines in automorphisms file for the number of graphs in input file." << std::endl;
                break;
            }
            Permutation automorphism = parse_automorphism(automorphism_line);
            if (graphObj.is_automorphism(automorphism) == false) {
                std::cerr << "Error: Automorphism does not preserve the graph structure." << std::endl;
                break;
            }
            output_file << graphObj.encode(automorphism, true) << std::endl;
        }
    }

    fclose(infile);
    output_file.close();
    automorphisms_file.close();
}

void decode_file(const std::string& input_fname, const std::string& output_fname, bool sparse, bool progr) {
    input_file.open(input_fname);
    if (!input_file.is_open()) {
        std::cerr << "Error opening input file: " << input_fname << std::endl;
        return;
    }
    int input_graphs_count = 0;
    std::string line;
    if (progr) {
        // Count the number of lines in the input file for progress tracking
        while (std::getline(input_file, line)) {
            ++input_graphs_count;
        }
        input_file.clear();
        input_file.seekg(0, std::ios::beg);
    }
    FILE *out_graphs_file;
    out_graphs_file = fopen(output_fname.c_str(), "w");
    int progress = 0;
    if (sparse) {
        fprintf(out_graphs_file, ">>sparse6<<");
        while (std::getline(input_file, line)) {
            printf("\rDecoding graphs %d/%d\r", progress++, input_graphs_count);
            Graph graphObj = decode(line);
            sparsegraph s6_graph = graphObj.to_sparsegraph();
            writes6_sg(out_graphs_file, &s6_graph);
            free(s6_graph.v);
            free(s6_graph.d);
            free(s6_graph.e);
        }
        fclose(out_graphs_file);
        printf("Decoding graphs %d/%d\n", progress, input_graphs_count);
    }
    else {
        fprintf(out_graphs_file, ">>graph6<<");
        DYNALLSTAT(graph,g,g_sz);
        while (std::getline(input_file, line)) {
            printf("\rDecoding graphs %d/%d\r", progress++, input_graphs_count);
            Graph graphObj = decode(line);
            int n = graphObj.n();
            int m_wordsize = SETWORDSNEEDED(n);
            DYNALLOC2(graph,g,g_sz,m_wordsize,n,"malloc");
            graphObj.to_densegraph(g, m_wordsize);
            writeg6(out_graphs_file, g, m_wordsize, n);
        }
        DYNFREE(g,g_sz);
        fclose(out_graphs_file);
        printf("Decoding graphs %d/%d\n", progress, input_graphs_count);
    }
}

std::vector<Graph> read_graphs(const std::string& input_fname) {
    FILE *infile = fopen(input_fname.c_str(), "r");
    char line[8];
    if (infile == NULL || fgets(line, sizeof(line), infile) == NULL) {
        std::cerr << "Error opening input file: " << input_fname << std::endl;
        return {};
    }
    fclose(infile);
    std::vector<Graph> output_graphs;
    // input file is in ssg format
    if (strcmp(line, ">>ssg<<") == 0 || (line[0] == ':' && line[1] == ':')) {
        input_file.open(input_fname);
        std::string line;
        while (std::getline(input_file, line)) {
            output_graphs.push_back(decode(line));
        }
        input_file.close();
        return output_graphs;
    }
    // input file is in graph6/sparse6 format
    int codetype;
    bool fswitch = false; // do not assume fixed length lines
    long startline = 1; // first line (1-based)
    char* input_fname_cstr = const_cast<char*>(input_fname.c_str());
    infile = opengraphfile(input_fname_cstr, &codetype, fswitch, startline);
    if (infile == NULL) {
        std::cerr << "Error opening input file: " << input_fname << std::endl;
        return {};
    }
    if (codetype & GRAPH6) {
        graph *g = NULL; // readg will allocate memory for g
        int n, m_wordsize;
        while ((g = readg(infile, NULL, 0, &m_wordsize, &n)) != NULL) {
            Graph graphObj = graph_to_Graph(g, m_wordsize, n);
            output_graphs.push_back(graphObj);
            FREES(g);
        }
    }
    else if (codetype & SPARSE6) {
        sparsegraph *sg = NULL;
        while ((sg = read_sg(infile, sg)) != NULL) {
            Graph graphObj = sparsegraph_to_Graph(*sg);
            output_graphs.push_back(graphObj);
        }
    }
    fclose(infile);
    return output_graphs;
}

void test_files(const std::string& input_fname, const std::string& input2_fname) {
    std::vector<Graph> graphs1 = read_graphs(input_fname);
    std::vector<Graph> graphs2 = read_graphs(input2_fname);
    if (graphs1.empty() || graphs2.empty()) {
        return;
    }
    if (graphs1.size() != graphs2.size()) {
        std::cerr << "Error: The two input files contain different numbers of graphs." << std::endl;
        return;
    }
    int first_non_isomorphic_index = -1;
    int isomorphic_count = 0;
    int total_graphs = graphs1.size();
    for (size_t i = 0; i < graphs1.size(); i++) {
        if (graphs1[i].is_isomorphic(graphs2[i]) == false) {
            std::cerr << "Error: Graph " << i + 1 << " in the two files are not isomorphic." << std::endl;
            if (first_non_isomorphic_index == -1) {
                first_non_isomorphic_index = i + 1; // Store the index of the first non-isomorphic graph (1-based)
            }
        } else {
            isomorphic_count++;
        }
    }
    std::cout << isomorphic_count << "/" << total_graphs << " graphs are isomorphic." << std::endl;
    if (first_non_isomorphic_index != -1) {
        std::cout << "The first non-isomorphic pair is at line " << first_non_isomorphic_index << "." << std::endl;
    }
}

int main(int argc, char *argv[]) {
    enum class mode {encode, decode, test, help};
    mode selected = mode::help;
    std::string input_fname;
    std::string input2_fname;
    std::string automorphisms_fname;
    std::string output_fname;
    bool progr = false, sparse = true;

    auto input_file = clipp::required("-i", "--input") & clipp::value("input_file", input_fname);
    auto input2_file = clipp::required("-I", "--input2") & clipp::value("input2_file", input2_fname);
    auto output_file = clipp::required("-o", "--output") & clipp::value("output_file", output_fname);

    auto encodeMode = (
        clipp::command("encode").set(selected,mode::encode),
        input_file % "input file containing graphs in graph6/sparse6/ssg format",
        clipp::required("-a", "--automorphisms") & clipp::value("automorphisms_file", automorphisms_fname),
        output_file % "output file to write encoded/decoded graphs to",
        clipp::option("--progress", "-p").set(progr));

    auto decodeMode = (
        clipp::command("decode").set(selected,mode::decode),
        input_file,
        output_file,
        ( clipp::option("-s", "-sparse"  ).set(sparse,true) |
        clipp::option("-d", "-dense" ).set(sparse,false) ) % "Output format is sparse6 / graph6",
        clipp::option("-p", "--progress").set(progr) % "show progress" );

    auto testMode = (
        clipp::command("test").set(selected,mode::test),
        input_file,
        input2_file);

    auto cli = (
        (encodeMode | decodeMode | testMode | clipp::command("help").set(selected,mode::help) ),
        clipp::option("-v", "--version").call([]{std::cout << "version 0.1\n\n";}).doc("show version")  );

    if(clipp::parse(argc, argv, cli)) {
        switch(selected) {
            case mode::encode: encode_file(input_fname, automorphisms_fname, output_fname, progr); break;
            case mode::decode: decode_file(input_fname, output_fname, sparse, progr); break;
            case mode::test: test_files(input_fname, input2_fname); break;
            case mode::help: std::cout << clipp::make_man_page(cli, "encoder"); break;
        }
    } else {
        std::cout << clipp::usage_lines(cli, "encoder") << '\n';
    }
    
    return 0;
}
