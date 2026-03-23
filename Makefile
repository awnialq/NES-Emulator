CXX := g++
WARNINGS := -Wall -Wextra -Wpedantic -Wshadow
CXXFLAGS := -std=c++20 -O2 $(WARNINGS) -Isrc
ASMFLAGS := $(filter-out -O%,$(CXXFLAGS)) -O0
RM := rm -f
NULLDEV := /dev/null
LDFLAGS :=
PROFILE_CXXFLAGS := $(CXXFLAGS) -pg
PROFILE_LDFLAGS := $(LDFLAGS) -pg

SRC_DIR := src
BIN_DIR := bin

TARGET := $(BIN_DIR)/nes-emulator
PROFILE_TARGET := $(BIN_DIR)/nes-emulator-prof
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)
PROFILE_OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.prof.o)
ASM_FILES := $(SOURCES:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.s)

CPU_TARGET := $(BIN_DIR)/cpu-test
PROFILE_CPU_TARGET := $(BIN_DIR)/cpu-test-prof
# CPU sources need to be specific if they are a subset, or we can use filter
CPU_SOURCES_NAMES := cpu6502.cpp Bus.cpp cartridge.cpp main.cpp
CPU_SOURCES := $(addprefix $(SRC_DIR)/, $(CPU_SOURCES_NAMES))
CPU_OBJECTS := $(CPU_SOURCES:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)
PROFILE_CPU_OBJECTS := $(CPU_SOURCES:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.prof.o)

all: $(TARGET)

all-prof: $(PROFILE_TARGET)

assm: $(ASM_FILES)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

cpu: $(CPU_TARGET)

cpu-prof: $(PROFILE_CPU_TARGET)

$(CPU_TARGET): $(CPU_OBJECTS) | $(BIN_DIR)
	$(CXX) $(CPU_OBJECTS) -o $@ $(LDFLAGS)

$(PROFILE_TARGET): $(PROFILE_OBJECTS) | $(BIN_DIR)
	$(CXX) $(PROFILE_OBJECTS) -o $@ $(PROFILE_LDFLAGS)

$(PROFILE_CPU_TARGET): $(PROFILE_CPU_OBJECTS) | $(BIN_DIR)
	$(CXX) $(PROFILE_CPU_OBJECTS) -o $@ $(PROFILE_LDFLAGS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/%.prof.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(PROFILE_CXXFLAGS) -c $< -o $@

$(BIN_DIR)/%.s: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(ASMFLAGS) -S $< -o $@

clean:
	-$(RM) -r $(BIN_DIR) > $(NULLDEV) 2>&1

.PHONY: all all-prof assm cpu cpu-prof clean