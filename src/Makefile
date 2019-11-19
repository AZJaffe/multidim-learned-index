CXX      := -g++
CXXFLAGS := -std=c++11 -pedantic-errors -Wall -Wextra -Werror
BUILD_DIR := ./build
TEST_DIR := $(BUILD_DIR)/test

TESTS := $(wildcard *_test.cpp)

.PHONY: tests
tests: build $(TESTS:%.cpp=$(TEST_DIR)/%.out)

tests: build
$(TEST_DIR)/linear_cdf_model_test.out: linear_cdf_model_test.cpp linear_cdf_model.h
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY: build
build:
	@mkdir -p $(TEST_DIR)

.PHONY: clean
clean:
	@rm -r $(BUILD_DIR)