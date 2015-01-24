# 3-SATSolver
This is a 3-SAT solver written in C ++ and CUDA C++ using DPLL and random assignment approach.

To run this code, you will need 
(1) Visual Studio 2010 or better
(2) CUDA 6 or better
(3) NVIDIA GPU (CUDA enabled) .
(4) Data files in DIMACS format for testing the formula.

Instructions :
(1) Place all these files in a solution in the project you create in Visual Studio.
(2) Put the path of the file in the function where file is read.
(3) Remove the first lines of the DIMACS format file till the point the data starts.
(4) Remove the last ending lines in the DIMACS format data file.
(3) Build and run the main.cpp file

Approach : Incomplete DPLL algorithm and Random Assignment.
Motivation : To evaluate Unit Propagation and pure literal assignment (parts of DPLL) and then perform random assignment to   literals to reduce search across the tree and observe speedup.
Pros : Takes a lot lesser time than the sequential version
Cons : Incomplete solver in both the cases i.e. sequential and parallel.


