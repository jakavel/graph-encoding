NAUTY_LIB := ./include/nauty/nauty.a
C_FLAGS := -O3 -Wall

encoder.exe: encoder.o graph.o binary_to_string.o permutation.o helpers.o
	g++ $(C_FLAGS) encoder.o graph.o binary_to_string.o permutation.o helpers.o $(NAUTY_LIB) -o symencode

permutation.o: permutation.cpp permutation.h helpers.h
	g++ $(C_FLAGS) -c permutation.cpp

encoder.o: encoder.cpp graph.h permutation.h
	g++ $(C_FLAGS) -c encoder.cpp

graph.o: graph.cpp graph.h permutation.h helpers.h
	g++ $(C_FLAGS) -c graph.cpp

binary_to_string.o: binary_to_string.cpp binary_to_string.h
	g++ $(C_FLAGS) -c binary_to_string.cpp

helpers.o: helpers.cpp helpers.h
	g++ $(C_FLAGS) -c helpers.cpp
