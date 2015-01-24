#ifndef __COMMON_H__
#define __COMMON_H__

using namespace std;
// function to perform the boolean satisfiability on the CPU 
void satSolverCPU();

// function to perform the boolean satisfiability on the GPU 
void satSolverGPU();

// function to read formula from file
vector <string> readFile();

// --Deprecated function
// function to randomly generate a boolean formula with 
// 'literalCount': number of boolean variables
vector<string> generateFormula( int literalCount);

// function to perform unit propagation on CPU
// 'formula' vector of strings which consists of clauses
// 'assignment' is the assignment given to literals namely, true or false
vector<string> unitPropagation(vector<string> formula,hash_map<int, bool, hash_compare<int,less<int>>> * assignment);

// function to delete clauses with 
// 'literalChosen' as one of its literals
// 'formula' vector of strings which consists of clauses on CPU
vector<string> deleteClauses(vector<string> formula,int literalChosen);

// function to identify pure literals and remove them from clauses, then add a new unit clause for theses literals
// 'formula' vector of strings which consists of clauses
// 'goAhead' decision vaiable for looping
vector<string> pureLiteralAssignment(vector<string> formula, int * goAhead);

// function to assign random values to the literals left in the new formula
// 'formula' vector of strings which consists of clauses
// 'randomAssignment' hash_map to store assignment of literals after simplification on CPU 
hash_map<int, bool, hash_compare<int,less<int>>> assignRandomValues(vector<string> formula, hash_map<int, bool, hash_compare<int,less<int>>> randomAssignment,int seed);

// function to assign random values to the literals left in the new formula
// 'formula' vector of strings which consists of clauses
// 'randomAssignment' hash_map to store assignment of literals after simplification on CPU
bool checkSatisfiability(vector<string> formula,hash_map<int, bool, hash_compare<int,less<int>>>  randomAssignment);

// display the assignment values if a solution exists or 
// display message to say that solution
// does not exist on CPU
void displaySolution(hash_map<int, bool, hash_compare<int,less<int>>> * assignment,hash_map<int, bool, hash_compare<int,less<int>>>  randomAssignment,bool success);

// initially set all literals value as unassigned on GPU
void initAssignmentGPU(int * assignment,int literalCount);

// fuunction to solve the formula on GPU
bool solver(vector<string> formula,int * assignment, int literalCount);

// display the assignment values if a solution exists or 
// display message to say that solution does not exist on GPU
void displaySolutionGPU(int * assignment,int literalCount,bool success);

// function to assign random values to the literals left in the new formula
// 'formula' vector of strings which consists of clauses
// 'assignment' integer array to store assignment of literals after simplification 
bool checkSatisfiabilityGPU(vector<string> formula,int * assignment);

// function to delete clauses with 
// 'literalChosen' as one of its literals
// 'formula' vector of strings which consists of clauses on GPU
vector<string> deleteClausesGPU(vector<string> formula,int literalChosen);

// function to identify pure literals and remove them from clauses, then add a new unit clause for theses literals
// 'formula' vector of strings which consists of clauses
// 'goAhead' decision variable for looping
// 'literalCount' number of literals
// 'assignment' integer array to store assignment of literals after simplification
vector<string> pureLiteralAssignmentGPU(vector<string> formula, int * goAhead, int literalCount, int * assignment);


#endif
