CXX = g++
CXXFLAGS = -g -Wall -O3
OBJS = main.o src/Upgma.o src/SCluster.o src/UVCluster.o src/Graph.o src/Surprise.o
TARGET = jerarca

all: $(TARGET)

jerarca: src/$(OBJS)
	$(CXX) $(CXXFLAGS) src/$(OBJS) -o jerarca
main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) -c src/main.cpp
Upgma.o: src/Upgma.cpp src/Upgma.h
	$(CXX) $(CXXFLAGS) -c src/Upgma.cpp
UVCluster.o: UVCluster.cpp UVCluster.h
	$(CXX) $(CXXFLAGS) -c UVCluster.cpp
SCluster.o: src/SCluster.cpp src/SCluster.h
	$(CXX) $(CXXFLAGS) -c src/SCluster.cpp
Surprise.o: src/Surprise.cpp src/Surprise.h
	$(CXX) $(CXXFLAGS) -c src/Surprise.cpp
Graph.o: src/Graph.cpp src/Graph.h 
	$(CXX) $(CXXFLAGS) -c src/Graph.cpp
clean:
	rm -fr src/$(OBJS) $(TARGET) src/*~
trash:
	rm -fr *meg *att *nwk
tar:
	tar cvzfp jerarca.tar.gz src/*h src/*cpp Makefile gpl.txt