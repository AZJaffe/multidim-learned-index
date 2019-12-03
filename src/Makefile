CXX      := -g++
CXXFLAGS := --std=c++17 -pedantic-errors -Wall -Wextra -Werror -O0
BUILD_DIR := ./build
TEST_DIR := $(BUILD_DIR)/test

TESTS := $(wildcard *_test.cpp)
SRCS := $(filter-out $(TESTS),$(wildcard *.h))

ifeq ($(DEBUG), 1)
	CXXFLAGS += -g -DDEBUG
else
	CXXFLAGS += -DNDEBUG
endif

.PHONY: tests
tests: build $(TESTS:%.cpp=$(TEST_DIR)/%.out)

$(TEST_DIR)/%_test.out: %_test.cpp $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $<

.PHONY: build
build:
	@mkdir -p $(TEST_DIR)

.PHONY: clean
clean:
	@rm -r $(BUILD_DIR)

echo:
	@echo srcs: $(SRCS) tests: $(TESTS)