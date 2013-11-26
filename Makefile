#!/bin/bash

CXX = g++
LINK = $(CXX)
CXXFLAGS = -Wall -O3
SRC_DIR = src
SRC_JER = src/Jerarca
EXEC = src/Surprise/computeSurprise Jerarca CPM RB Infomap RN RNSC src/scripts/VI

OBJSURPRISE = $(SRC_DIR)/Surprise/computeSurprise.o $(SRC_DIR)/scripts/VI.o


all: $(EXEC)

Jerarca :
	cd src/Jerarca; make -j

CPM:
	cd src/CPM; make -j

RB:
	cd src/RB; make -j

Infomap:
	cd src/Infomap/src; make -j

RN:
	cd src/RN; perl compileRN

RNSC:
	cd src/RNSC; make


%.o: %.cpp %.h
	$(CXX) -o $@ -c $< $(CXXFLAGS)

%.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

%.o: %.cc %.h
	$(CXX) -o $@ -c $< $(CXXFLAGS)

clean:
	rm -f $(OBJSURPRISE) $(EXEC)
	cd src/Jerarca; make clean
	cd src/CPM/; make clean
	cd src/RB/; make clean
	cd src/Infomap/src; make distclean
	cd src/RN; rm -f rnMRA
	cd src/RNSC; make clean
