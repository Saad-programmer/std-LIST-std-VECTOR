CXX ?= g++
all: problem fixed
problem: problem.cpp common.hpp
	$(CXX) -O2 -std=c++17 -Wall -Wextra problem.cpp -o problem
fixed: fixed.cpp common.hpp
	$(CXX) -O2 -std=c++20 -Wall -Wextra fixed.cpp -o fixed
run: all
	./problem && ./fixed
