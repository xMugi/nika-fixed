# Makefile for a C++ program linked with X11 and Xtst libraries

# Compiler
CXX = g++

# Source files
SRCS = Nika.cpp

# Libraries to link
LIBS = -lX11 -lXtst

# Default target
all: rebuild; 

# Clean up object files and the executable
clean:
	rm -f noka

# Compilation and linking
noka: $(SRCS)
	$(CXX) $^ -o $@ $(LIBS)


# clean & build
rebuild:
	@$(MAKE) clean
	@$(MAKE) noka
	echo "BUILD SUCCESSFUL - RUN: sudo ./noka"

