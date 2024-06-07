CC=g++ -g3 -Wall -std=c++11

# List of source files for your sudoku solver
SOLVER_SOURCES=cell.cpp sudokuboard.cpp techniques.cpp utility.cpp main.cpp

# Generate the names of the disk solver's object files
SOLVER_OBJS=${SOLVER_SOURCES:.cpp=.o}

all: solver

# Compile the disk solver and tag this compilation
solver: ${SOLVER_OBJS} ${LIBTHREAD}
	${CC} -o $@ $^ -ldl -pthread

# Generic rules for compiling a source file to an object file
%.o: %.cpp
	${CC} -c $<
%.o: %.cc
	${CC} -c $<

clean:
	rm -f ${SOLVER_OBJS} solver
