CXX      := -g++
CXXFLAGS := -g --std=c++17 -pedantic-errors -Wall -Wextra -Werror -O3
BUILD_DIR := ./build
TEST_DIR := $(BUILD_DIR)/test

TESTS := $(wildcard *_test.cpp)
SRCS := $(wildcard *.h)

# Run make TRACE=1 to enable debug print statements and other information
# Run make NDEBUG=1 to disable assertions (don't do this for tests)

ifeq ($(NDEBUG), 1)
	CXXFLAGS += -DNDEBUG
endif

ifeq ($(TRACE), 1)
	CXXFLAGS += -DTRACE
endif

ifeq ($(ASAN), 1)
	CXXFLAGS += -fsanitize=address -static-libasan
endif

.PHONY: build
build:
	@mkdir -p $(TEST_DIR)

.PHONY: clean
clean:
	@rm -r $(BUILD_DIR)

echo:
	@echo srcs: $(SRCS) tests: $(TESTS)

.PHONY: tests
tests: build $(TESTS:%.cpp=$(TEST_DIR)/%.out)

$(TEST_DIR)/%_test.out: %_test.cpp $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $<

$(TEST_DIR)/benchmark.out: benchmark.cpp $(SRCS) build
	$(CXX) $(CXXFLAGS) -o $@ $<
