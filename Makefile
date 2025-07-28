encoder.exe: encoder.o graph.o binary_to_string.o permutation.o
	g++ -Wall encoder.o graph.o binary_to_string.o permutation.o -o encoder.exe

permutation.o: permutation.cpp permutation.h
	g++ -Wall -c permutation.cpp

encoder.o: encoder.cpp graph.h permutation.h
	g++ -Wall -c encoder.cpp

graph.o: graph.cpp graph.h permutation.h
	g++ -Wall -c graph.cpp

binary_to_string.o: binary_to_string.cpp binary_to_string.h
	g++ -Wall -c binary_to_string.cpp