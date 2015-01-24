// Program to compare executions of incomplete boolean satisfiability solver on CPU and GPU
// The approach is as follows :
// (1) Unit Propagation
// (2) Pure literal assignment - Repeat from step (1) if pure literal found else go to step (3)
// (3) (i)  Random Assignment/Random Walk across banchs of a tree of assignment of values to left over literals in remaining cluases
//			(Note : try this step for a limited number of times, if still solution not found, terminate execution)
// This approach does not guarantee a solution as it is incomplete
// @uthor : Stavan Karia
// Project for High Performance Achitecture

#include <cstdlib> 
#include <ctime>
#include <cmath>
#include <vector>
#include <string>
#include <iostream> 
#include <hash_map>
#include "common.h"
#include <cuda.h>
#include <cuda_runtime_api.h>

using namespace std;
using namespace stdext;

// main function
int main(){
	// varibales to clock the execution time
	double timeCPU,timeGPU;
	clock_t startClock,stopClock;
	
	// start timing
	startClock=clock();
	// function to perform the boolean satisfiability on the CPU 
	satSolverCPU();
	// stop timing
	stopClock=clock();
	timeCPU=(double)(stopClock-startClock)*1000/(double)CLOCKS_PER_SEC;
	printf("Execution time on CPU = %f\n",timeCPU);
	
	
	// start timing	
	startClock=clock();
	// function to perform the boolean satisfiability on the GPU 
	satSolverGPU();
	// stop timing
	stopClock=clock();
	timeGPU=(double)(stopClock-startClock)*1000/(double)CLOCKS_PER_SEC;
	printf("Execution time on GPU=%f\n",timeGPU);

	printf("\nSpeedup observed is %f",timeCPU/timeGPU);

	return 1;
}

