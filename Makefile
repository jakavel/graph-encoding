NAUTY_LIB := ./nauty.a
C_FLAGS := -O3 -Wall

encoder.exe: encoder.o graph.o binary_to_string.o permutation.o
	g++ $(C_FLAGS) encoder.o graph.o binary_to_string.o permutation.o $(NAUTY_LIB) -o encoder.exe

permutation.o: permutation.cpp permutation.h
	g++ $(C_FLAGS) -c permutation.cpp

encoder.o: encoder.cpp graph.h permutation.h
	g++ $(C_FLAGS) -c encoder.cpp

graph.o: graph.cpp graph.h permutation.h
	g++ $(C_FLAGS) -c graph.cpp

binary_to_string.o: binary_to_string.cpp binary_to_string.h
	g++ $(C_FLAGS) -c binary_to_string.cpp