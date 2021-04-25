CC = gcc
CXX = g++-10

CFLAGS = -g -Wall -std=c++20 -pthread -fsanitize=address

all: test

test: test.o
	$(CXX) $(CFLAGS) -o test test.o

test.o: test.cpp STM/TLEncounterModeTx.hpp STM/Orec.hpp STM/Transaction.hpp RedBlackTree/RedBlackTree.hpp RedBlackTree/AbstractTree.hpp
	$(CXX) $(CFLAGS) -c test.cpp