ifeq ($(OS),Windows_NT)
    RM = del /F /Q
    NULLDEV = NUL
else
    RM = rm -f
    NULLDEV = /dev/null
endif

CXX       = g++
CXXFLAGS  = -std=c++20 -Wall -Wextra -O2 -ISDL3/include -I.
LDFLAGS   = -LSDL3/lib -lmingw32 -lSDL3

CPU_SOURCES = cpu6502.cpp Bus.cpp main.cpp cartridge.cpp
CPU_OBJECTS = $(CPU_SOURCES:.cpp=.o)
CPU_TARGET  = test

all: $(CPU_TARGET)

$(CPU_TARGET): $(CPU_OBJECTS)
	$(CXX) $(CPU_OBJECTS) -o $@ $(LDFLAGS)
	-$(RM) $(CPU_OBJECTS) > $(NULLDEV) 2>&1

%.o: %.cpp cpu6502.h Bus.h cartridge.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	-$(RM) $(CPU_OBJECTS) $(CPU_TARGET) > $(NULLDEV) 2>&1

.PHONY: all clean