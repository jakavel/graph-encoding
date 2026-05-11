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
    std::string automorphism_line;
    if (codetype & GRAPH6) {
        graph *g = NULL; // readg will allocate memory for g
        int n, m_wordsize;
        while ((g = readg(infile, g, 0, &m_wordsize, &n)) != NULL) {
            Graph graphObj = graph_to_Graph(*g, m_wordsize, n);
            if (!std::getline(automorphisms_file, automorphism_line)) {
                std::cerr << "Error: Not enough lines in automorphisms file for the number of graphs in input file." << std::endl;
                FREES(g);
                fclose(infile);
                automorphisms_file.close();
                output_file.close();
                return;
            }
            Permutation automorphism = parse_automorphism(automorphism_line);
            // non-sparse encoding not implemented
            output_file << graphObj.encode(automorphism, true) << std::endl;
        }
        FREES(g);
    }
    else if (codetype & SPARSE6) {
        sparsegraph *sg = NULL;
        while ((sg = read_sg(infile, sg)) != NULL) {
            Graph graphObj = sparsegraph_to_Graph(*sg);
            if (!std::getline(automorphisms_file, automorphism_line)) {
                std::cerr << "Error: Not enough lines in automorphisms file for the number of graphs in input file." << std::endl;
                free(sg->v);
                free(sg->d);
                free(sg->e);
                fclose(infile);
                automorphisms_file.close();
                output_file.close();
                return;
            }
            Permutation automorphism = parse_automorphism(automorphism_line);
            output_file << graphObj.encode(automorphism, true) << std::endl;
        }
        free(sg->v);
        free(sg->d);
        free(sg->e);
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

int main(int argc, char *argv[]) {
    enum class mode {encode, decode, help};
    mode selected = mode::help;
    std::string input_fname;
    std::string automorphisms_fname;
    std::string output_fname;
    bool progr = false, sparse = true;

    auto input_file = clipp::required("-i", "--input") & clipp::value("input_file", input_fname);
    auto output_file = clipp::required("-o", "--output") & clipp::value("output_file", output_fname);

    auto encodeMode = (
        clipp::command("encode").set(selected,mode::encode),
        input_file,
        clipp::required("-a", "--automorphisms") & clipp::value("automorphisms_file", automorphisms_fname),
        output_file,
        clipp::option("--progress", "-p").set(progr) % "show progress" );

    auto decodeMode = (
        clipp::command("decode").set(selected,mode::decode),
        input_file,
        output_file,
        ( clipp::option("-s", "-sparse"  ).set(sparse,true) |
        clipp::option("-d", "-dense" ).set(sparse,false) ) % "Output format is sparse6 / graph6",
        clipp::option("-p", "--progress").set(progr) % "show progress" );

    auto cli = (
        (encodeMode | decodeMode | clipp::command("help").set(selected,mode::help) ),
        clipp::option("-v", "--version").call([]{std::cout << "version 0.1\n\n";}).doc("show version")  );

    if(clipp::parse(argc, argv, cli)) {
        switch(selected) {
            case mode::encode: encode_file(input_fname, automorphisms_fname, output_fname, progr); break;
            case mode::decode: decode_file(input_fname, output_fname, sparse, progr); break;
            case mode::help: std::cout << clipp::make_man_page(cli, "encoder"); break;
        }
    } else {
        std::cout << clipp::usage_lines(cli, "encoder") << '\n';
    }
    
    return 0;
}