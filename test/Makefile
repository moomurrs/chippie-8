# OBJS specifies which files to compile as part of the project
OBJS = *.cpp

# CXX specifies the compiler that we want to use
CXX = clang++

# COMPILER_FLAGS specifies the additional compilation options that we are using
# -w supress all warnings
COMPILER_FLAGS = -std=c++17

# INCLUDE_PATHS
INCLUDE_PATHS = -I"../include/"

# LINKER_PATHS
LINKER_PATHS = -L"/usr/local/lib"

# LINKER_FLAGS
LINKER_FLAGS = -lraylib

# OBJ_NAME specifies the name of our executable
OBJ_NAME = test


run: build # build and run tests
	./test

build: $(OBJS) # build only
	$(CXX) $(OBJS) $(COMPILER_FLAGS) $(INCLUDE_PATHS) $(LINKER_PATHS) $(LINKER_FLAGS) -o $(OBJ_NAME)

clean:
	rm -f test
