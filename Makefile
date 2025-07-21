CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 $(shell sdl2-config --cflags)

CPU_SOURCES = cpu6502.cpp Bus.cpp main.cpp
CPU_OBJECTS = $(CPU_SOURCES:.cpp=.o)
CPU_TARGET = cpu6502-test

all: $(CPU_TARGET)

$(CPU_TARGET): $(CPU_OBJECTS)
	$(CXX) $(CPU_OBJECTS) -o $@ $(shell sdl2-config --libs)

%.o: %.cpp cpu6502.h Bus.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(CPU_OBJECTS) $(CPU_TARGET)

.PHONY: all clean