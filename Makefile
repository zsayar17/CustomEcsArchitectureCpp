CXX = g++
CXXFLAGS = -std=c++11 -Wall -I src -pthread -O2
TARGET = ecs_dots
BUILD_DIR = build

SRC = $(wildcard src/*.cpp)
CORE_SRC = $(filter-out src/main.cpp src/benchmark.cpp, $(SRC))
CORE_OBJ = $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(CORE_SRC))

MAIN_OBJ = $(BUILD_DIR)/main.o
BENCH_OBJ = $(BUILD_DIR)/benchmark.o

all: $(BUILD_DIR) $(TARGET) benchmark

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(CORE_OBJ) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

benchmark: $(CORE_OBJ) $(BENCH_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) benchmark

re:
	$(MAKE) clean
	$(MAKE) all
