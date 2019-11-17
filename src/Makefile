CXX      := -g++
CXXFLAGS := -std=c++11 -pedantic-errors -Wall -Wextra -Werror
BUILD_DIR := ./build
TEST_DIR := $(BUILD_DIR)/test

TESTS := $(wildcard *_test.cpp)

.PHONY: tests
tests: build $(TESTS:%.cpp=$(TEST_DIR)/%.out)

$(TEST_DIR)/%_test.out: %_test.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY: build
build:
	@mkdir -p $(TEST_DIR)