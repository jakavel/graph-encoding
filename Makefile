encoder.exe: encoder.o graph.o
	g++ -Wall encoder.o graph.o -o encoder.exe

encoder.o: encoder.cpp graph.h
	g++ -Wall -c encoder.cpp

graph.o: graph.cpp graph.h
	g++ -Wall -c graph.cpp