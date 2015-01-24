// Incomplete boolean satisfiability solver
// Data set chosen from ' http://www.cs.ubc.ca/~hoos/SATLIB/benchm.html '
// Data format is DIMACS
// @uthor : Stavan Karia

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <string>
#include <vector>
#include <list>
#include <hash_map>
#include <iostream> 
#include <sstream>
#include "common.h"
#include <cuda.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <cuda_runtime_api.h>
#include <device_launch_parameters.h>

// number of tries to check for random assignment
#define MAX_TRIES 256


// function to assign random values to the literals left in the new formula
// 'formulaGPU' integer array which consists of each literal in the clause
// 'assignment' hash_map to store assignment of literals after simplification 
// 'literalCount' number of literals
// 'formulaSize' size of formula array
// 'clauseCount' number of clauses
// 'seed' seed for Random Number generator
// 'successGPU' decision variable
__global__ void assignRandomValuesGPU(int * formulaGPU,int * assignment,int literalCount,int formulaSize, int *clause, int clauseCount,int seed, int *successGPU){
	int Id=threadIdx.x+blockIdx.x*blockDim.x;
	int lit,trueVal=0,lim=100000;
	// each threads local new assignment array to try for random assignment
	int *assignLocal; 
	assignLocal=new int[literalCount];
	for(int i=1;i<=literalCount;i++){
		assignLocal[i-1]=assignment[i-1];
		if(assignment[i-1]==2){
			// random number generator generator
			long a = 100001;
			a = (a * 125 * (Id+1) * i) % 27962039;
			a = ((a % lim) + 1 + i);
			// give polarity to literal
			// 1 is true
			// 0 is false
			// 2 is unassigned
			if(a<50000){
				assignLocal[i-1]=1;
			}
			else{
				assignLocal[i-1]=0;
			}
		}
	}

	// check for satisfiability of each clause
	// clause[k] is a start and end pointer to the clause in formulaGPU array
	for(int k=0;k<formulaSize-1;k++){
		for(int j=clause[k];j<clause[k+1];j++){
			lit=formulaGPU[j];
			if(lit > 0 && assignLocal[j] == 1){
				trueVal++;
				break;
			}
			else if (lit < 0 && assignLocal[j] == 0){
				trueVal++;
				break;
		
			}
		}
	}

	// if all clauses are satisfiabil, print the solution assignment
	// all possible assignments shall be printed here, by each thread
	if(trueVal==clauseCount){
		printf("A possible solution is : \n");
		printf("Literal\tValue\n");
		for(int l=1;l<=literalCount;l++){
			printf("%d\t%d\n",l,assignLocal[l-1]);
		}
		*successGPU=1;
	}
	else{
		*successGPU=0;
	}

}

// function to perform the boolean satisfiability on the GPU 
void satSolverGPU(){
	int literalCount = 75;													// number of literals 
	bool success=false;														// variable to check for satisfiability of formula
	vector<string> formula;													// vector of strings to store clauses	
	int * assignment;														// int array to store assignment of values to literals 
	assignment = new int[literalCount];
	// read data from file
	formula=readFile();
	// set all literals assignment value as unassigned i.e. 2
	// 1 for true
	// 0 for false
	initAssignmentGPU(assignment,literalCount);	
	cout<<"-------------------------------------------------------------------------------"<<endl;
	cout<<"GPU OUTPUT"<<endl;
	cout<<"-------------------------------------------------------------------------------"<<endl;
	// solve the formula
	success=solver(formula,assignment,literalCount);	
	// display the results on the terminal
	displaySolutionGPU(assignment,literalCount,success);
}

// function to perform unit propagation with
// 'formula' : vector of strings which consists of clauses
// 'assignment' is the assignment given to literals namely, true i.e. 1  or false i.e. 0 or unassigned i.e. 2
vector<string> unitPropagationGPU(vector<string> formula,int * assignment){
	// delimiter used to show logical OR 
	string delimiter=" ";
	// position of the delimiter found
	size_t position=0;
	// literal which is to be deleted as it is a unit clause
	int literalChosen,i=0;
	// variable to traverse accross the formula
	do{
		if(formula[i].length()!=0){
			position=formula[i].find(delimiter);	
			// if there is no delimiter it returns 'npos'
			// if it returns npos, that means it is a unit clause
			// assign true if polarity of the literal is positive else assign false
			if(position==string::npos){
				stringstream convert(formula[i]);
				convert>>literalChosen;
				// assign boolean value true or false depending on the polarity of the variable in the unit clause
				if(literalChosen>0){
					assignment[literalChosen-1]=1;
				}
				else if (literalChosen<0){
					assignment[(-1*literalChosen)-1]=0;
				}
				// delete clauses with the literal with same polarity
				// and remove same literal with opposite polarity from all clauses 
				formula=deleteClausesGPU(formula,literalChosen);
				// reset search from first element	
				i=0;
			}
			else{
					i++;
					if(i==formula.size()){
						return formula;
					}
				}
		}
		else{
			i=formula.size();
			break;
		}
		if(formula.size()==0){
			return formula;
		}
	}while(i<formula.size());	
	return formula;
}

// function to solve the formula
bool solver(vector<string> formula,int * assignment,int literalCount){
	// decision variables to continue cycle of unit propgation
	int *goAhead=new int;												
	*goAhead=1;
	int seed;
	bool success= false;	
	do{
		// perform unit propagation
		formula=unitPropagationGPU(formula, assignment);
		if(formula.size()==0){
			*goAhead=1;
			return true;
		}
		// perform pure literal assignment
		formula=pureLiteralAssignmentGPU(formula,goAhead,literalCount,assignment);

		// peform unit propagation if any more clauses added in pureLiteralAssignment 
	}while(*goAhead);

	// generate a random seed
	seed=rand();

	//number of elements in the formula
	int formulaElems=0; 

	// delimiter in the clauses
	string delimiter=" ";

	// find number of elements
	for(int i=0;i<formula.size();i++){
		size_t pos = 0;
		string token;
		string temp=formula[i];
		pos = temp.find(delimiter);
		while (!(pos == string::npos)) {
			token = temp.substr(0, pos);
			formulaElems++;
			temp.erase(0, pos + delimiter.length());
			pos = temp.find(delimiter);
			if(pos==string::npos && temp.length()!=0){
				pos=temp.length();	
			}
		}
	}

	// create array of formula elements as integers
	// create pointers to clause start and end locatins in formula array
	int * formulaArray,*clause;
	formulaArray = new int[formulaElems];
	clause = new int[formula.size()]; 
	int literalChosen,counter=0;
	for(int i=0;i<formula.size();i++){
		size_t pos = 0;
		string token;
		string temp=formula[i];
		pos = temp.find(delimiter);
		clause[i]=counter;
		while (!(pos == string::npos)) {
			token = temp.substr(0, pos);
			stringstream convert(token);
			convert>>literalChosen;		
			formulaArray[counter]=literalChosen;
			counter++;
			temp.erase(0, pos + delimiter.length());
			pos = temp.find(delimiter);
			if(pos==string::npos && temp.length()!=0){
				pos=temp.length();	
			}
		}
	}
	
	// create variables for GPU
	int *formulaGPU;
	int *clauseGPU;
	int *assignmentGPU;
	int *successGPU;
	// allocate memory on GPU for above mentioned variables
	cudaMalloc((void**)&formulaGPU, formulaElems*sizeof(int));
	cudaError_t	status = cudaGetLastError();
		if (status != cudaSuccess) 
		{
			std::cout << "Kernel failed 1: " << cudaGetErrorString(status) << std::endl;
			return false;
		}

	cudaMalloc((void**)&assignmentGPU, literalCount*sizeof(int));
	status = cudaGetLastError();
		if (status != cudaSuccess) 
		{
			std::cout << "Kernel failed 2: " << cudaGetErrorString(status) << std::endl;
			return false;
		}

	cudaMalloc((void**)&clauseGPU, formula.size()*sizeof(int));
		status = cudaGetLastError();
		if (status != cudaSuccess) 
		{
			std::cout << "Kernel failed 3: " << cudaGetErrorString(status) << std::endl;
			return false;
		}
	cudaMalloc((void**)&successGPU, sizeof(int));
	status = cudaGetLastError();
		if (status != cudaSuccess) 
		{
			std::cout << "Kernel failed 4: " << cudaGetErrorString(status) << std::endl;
			return false;
		}
	
	// copy data from CPU variables to GPU variables
	cudaMemcpy(formulaGPU, formulaArray,formulaElems*sizeof(int),cudaMemcpyHostToDevice);
	status = cudaGetLastError();
		if (status != cudaSuccess) 
		{
			std::cout << "Kernel failed 5: " << cudaGetErrorString(status) << std::endl;
			return false;
		}

	cudaMemcpy(assignmentGPU,assignment,literalCount*sizeof(int),cudaMemcpyHostToDevice);
	status = cudaGetLastError();
		if (status != cudaSuccess) 
		{
			std::cout << "Kernel failed 6: " << cudaGetErrorString(status) << std::endl;
			return false;
		}


	cudaMemcpy(clauseGPU,clause,formula.size()*sizeof(int),cudaMemcpyHostToDevice);
	status = cudaGetLastError();
		if (status != cudaSuccess) 
		{
			std::cout << "Kernel failed 7: " << cudaGetErrorString(status) << std::endl;
			return false;
		}

	// give block and grid dimensions for GPU kernel
	// 1D GRID 1D block
	dim3 dimBlock(256,1);
	status = cudaGetLastError();
		if (status != cudaSuccess) 
		{
			std::cout << "Kernel failed 8: " << cudaGetErrorString(status) << std::endl;
			return false;
		}

	int gridX=1;//MAX_TRIES/1024;
	dim3 dimGrid(gridX,1);
		status = cudaGetLastError();
		if (status != cudaSuccess) 
		{
			std::cout << "Kernel failed 9: " << cudaGetErrorString(status) << std::endl;
			return false;
		}
	// kernel to assign random values on each thread and check for that threads satisfiability
	assignRandomValuesGPU<<<dimGrid,dimBlock>>>(formulaGPU,assignmentGPU,literalCount,formulaElems,clauseGPU,formula.size(),seed,successGPU);
		status = cudaGetLastError();
		if (status != cudaSuccess) 
		{
			std::cout << "Kernel failed 10: " << cudaGetErrorString(status) << std::endl;
			return false;
		}

	if(success==true){
		return true;
	}
	return success;
}




// function to delete clauses with 
// 'literalChosen' as one of its literals
vector<string> deleteClausesGPU(vector<string> formula,int literalChosen){
	// position of literal in formula
	size_t position=0;
	size_t positionOpp=0;
	for(int i=0;i<formula.size();i++){
			// check if chosen literal is found
			ostringstream oss;
			oss<<literalChosen;
			position=formula[i].find(oss.str());
			// check if chosen literal's opposite polarity is found
			ostringstream ossOpp;
			ossOpp<<(literalChosen*-1);
			positionOpp=formula[i].find(ossOpp.str());
			// if the literal is present in the clause
			// then delete / erase the clause
			if(position!=string::npos){
				//cout<<"deleted clause "<<formula[i]<<endl;
				formula.erase(formula.begin()+i);
				i--;
			}
			// if literal with opposite polarity is found
			// remove the literal from the clause
			else if(positionOpp!=string::npos){
				// removes the first occurence of the literal with opposite polarity
				// if the literal is the first in the clause
				if(positionOpp==0){
					formula[i].erase(positionOpp,ossOpp.str().length()+1);
				}
				// if the literal is at the end of the clause
				else if (positionOpp==formula[i].length()-ossOpp.str().length()){
					formula[i].erase(positionOpp-1,ossOpp.str().length()+1);
				}
				// if the literal is somewhere in the middle section of the clause
				else{
					formula[i].erase(positionOpp,ossOpp.str().length()+1);
				}
				// removes all occurences after the first of the literal with opposite polarity
				while((positionOpp=formula[i].find(ossOpp.str()))!=string::npos){
					// if the literal is the first in the clause
					if(positionOpp==0){
						formula[i].erase(positionOpp,ossOpp.str().length()+1);
					}
					// if the literal is at the end of the clause
					else if (positionOpp==formula[i].length()-ossOpp.str().length()){
						formula[i].erase(positionOpp-1,ossOpp.str().length()+1);			
					}
					// if the literal is somwhere in the middle section of the clause
					else{
						formula[i].erase(positionOpp,ossOpp.str().length()+1);
					}
				}
		}
	}
	return formula;
}


// function to identify pure literals and remove them from clauses, then add a new unit clause for theses literals
// 'formula' vector of strings which consists of clauses
// 'goAhead' decision variable for looping
// 'literalCount' number of literals
// 'assignment' integer array to store assignment of literals after simplification
vector<string> pureLiteralAssignmentGPU(vector<string> formula, int * goAhead, int literalCount, int * assignment){
	*goAhead=0;
	int positive, neg, n;
	size_t pos,posOpp;
	string str1,str2;
	// for each literal and its polarity
	// check all clauses 
	for(int i=1;i<=literalCount;i++){
		if(assignment[i-1]==2){
			positive=0;
			neg=0;
			n=0;
			for(int j=0;j<formula.size();j++){
				ostringstream oss;
				oss<<i;
				ostringstream ossOpp;
				ossOpp<<i*(-1);
				pos=formula[j].find(oss.str());
				posOpp=formula[j].find(ossOpp.str());
				str1=oss.str();
				str2=ossOpp.str();
				// if positive polarity found
				if (pos!=string::npos){
					positive++; 
					n++;
		     	}
				// if negative polarity found
				if (posOpp!=string::npos){
					neg++; 
       				n++;
       			}
			}      
			// if literal found only in positive polarity
			 if (n!=0 && positive!=0 && positive == n){
				assignment[i-1] = 1; 
				// add literal to clause
				formula.push_back(str1);
				*goAhead=1;
			 }
   			// if literal found only in negative polarity
			else if (n!=0 && neg!=0 && neg == n){
				assignment[i-1] = 0; 
				// add literal to clause
				formula.push_back(str2);
				*goAhead=1;
			}
		}
	}
	return formula;
}

// display the solution assignment for the given formula
void displaySolutionGPU(int *assignment,int literalCount,bool success){
	if(success==1){
		cout<<"Congratulations !"<<endl;
		cout<<endl<<"Formula Satisfied !"<<endl;
		cout<<"( All the lierals not assigned any value below are to be assumed as false )"<<endl;  
		cout<<"Assignments are : "<<endl;
		cout<<"-----------------------------------"<<endl;
		cout<<"Literal \t"<<"Value"<<endl;
		for(int i=1;i<=literalCount;i++){
			printf("%d\t%d\n",i,assignment[i-1]);
		}
	}
	else{
		cout<<"Sorry :("<<endl;
		cout<<"There is no solution i.e. possible assignment for this formula !"<<endl;
	}

}


void initAssignmentGPU(int * assignment,int literalCount){
	for(int i=1;i<=literalCount;i++){
		assignment[i-1]=2;
	}
}
