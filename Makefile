CXX := g++
WARNINGS := -Wall -Wextra -Wpedantic -Wshadow
CXXFLAGS := -std=c++20 -O2 $(WARNINGS)
ASMFLAGS := $(filter-out -O%,$(CXXFLAGS)) -O0
RM := rm -f
NULLDEV := /dev/null
LDFLAGS :=

TARGET := nes-emulator
SOURCES := main.cpp Bus.cpp cpu6502.cpp cartridge.cpp Mapper.cpp Mapper000.cpp ppu2C02.cpp
OBJECTS := $(SOURCES:.cpp=.o)
ASM_FILES := $(SOURCES:.cpp=.s)

CPU_TARGET := cpu-test
CPU_SOURCES := cpu6502.cpp Bus.cpp cartridge.cpp main.cpp
CPU_OBJECTS := $(CPU_SOURCES:.cpp=.o)

all: $(TARGET)

assm: $(ASM_FILES)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

cpu: $(CPU_TARGET)

$(CPU_TARGET): $(CPU_OBJECTS)
	$(CXX) $(CPU_OBJECTS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.s: %.cpp
	$(CXX) $(ASMFLAGS) -S $< -o $@

clean:
	-$(RM) $(OBJECTS) $(CPU_OBJECTS) $(ASM_FILES) $(TARGET) $(CPU_TARGET) > $(NULLDEV) 2>&1

.PHONY: all assm cpu clean