# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Iinclude

# Google Test library
GTEST_LIBS = -lgtest -lgtest_main -pthread

# Directories
SRCDIR = src
OBJDIR = obj
TESTDIR = tests
BINDIR = bin

# Create necessary directories
$(shell mkdir -p $(OBJDIR) $(BINDIR))

# Target executable
TARGET = $(BINDIR)/chess

# Source files (excluding main.cpp for tests)
SRCS = $(wildcard $(SRCDIR)/*.cpp)
SRCS_NO_MAIN = $(filter-out $(SRCDIR)/main.cpp, $(SRCS))

# Object files
OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))
OBJS_NO_MAIN = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS_NO_MAIN))

# Test files (only compiling tests.cpp)
TEST_SRCS = $(TESTDIR)/tests.cpp
TEST_OBJS = $(OBJDIR)/tests.o
TEST_EXE = $(BINDIR)/tests

# Default build target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files into object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile the test file into an object file
$(OBJDIR)/tests.o: $(TESTDIR)/tests.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile and link test executable (excluding main.cpp)
$(TEST_EXE): $(TEST_OBJS) $(OBJS_NO_MAIN)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(GTEST_LIBS)

# Run tests
run_tests: $(TEST_EXE)
	./$(TEST_EXE)

# Clean up
clean:
	rm -f $(OBJDIR)/*.o $(TARGET) $(TEST_EXE)