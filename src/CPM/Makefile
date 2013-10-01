#!/bin/bash

CC=g++
CFLAGS=  -Wall
LDFLAGS= -Wall
SRC_DIR=src
EXEC=createbin bin/community bin/slicer
OBJ1= $(SRC_DIR)/Base/community.o $(SRC_DIR)/Base/graph.o $(SRC_DIR)/Base/greedy_louvain.o $(SRC_DIR)/Base/info.o $(SRC_DIR)/MTRand/MersenneTwister.o
OBJ2= $(SRC_DIR)/Slicer/slicer.o $(SRC_DIR)/Base/graph.o

all: $(EXEC)

createbin :
	mkdir -p bin
bin/community : $(OBJ1) $(SRC_DIR)/Community/main_community.o
	$(CC) -o $@ $^ $(LDFLAGS)

bin/slicer : $(OBJ2) $(SRC_DIR)/Slicer/main_slicer.o
	$(CC) -o $@ $^ $(LDFLAGS)

##########################################
# Generic rules
##########################################

%.o: %.cpp %.h
	$(CC) -o $@ -c $< $(CFLAGS)

%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

%.o: %.cc %.h
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -f $(EXEC) src/*/*.o
