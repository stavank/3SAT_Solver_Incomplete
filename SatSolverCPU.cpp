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
#include <cuda_runtime_api.h>

using namespace std;
using namespace stdext;

// number of tries to check for random assignment
#define MAX_TRIES 256

// file pointer for file to be read
FILE *fr;
// function to perform the boolean satisfiability on the CPU 
void satSolverCPU(){
	int *goAhead;															// decision variables to continue cycle of unit propgation
	goAhead=new int;														// and pure literal assignment
	int move=1;
	*goAhead=move;
	bool success=true;														// variable to check for satisfiability of formula
	int literalCount;														// number of literals in the formula - taken as input
	int seed;
	vector<string> formula;													// vector of strings to store clauses	
	hash_map<int, bool, hash_compare<int,less<int>>> * assignment;			// hash_map to store assignment of values to literals 
	hash_map<int, bool, hash_compare<int,less<int>>> randomAssignment;		// hash_map to store random assignments of values to literals 
	assignment = new hash_map<int,bool>();
	int numOfRandomTries=0;													// number of random assignment trials we do to reach solution
	// read data from file
	formula=readFile();
	cout<<"-------------------------------------------------------------------------------"<<endl;
	cout<<endl<<"Each number indicates a literal where a positive literal has positive polarity "<<endl<<"and negative literal has negative polarity"<<endl;
	cout<<"For eg. 27 means a positve literal whereas"<<endl;
	cout<<"-27 means a literal with negative polarity"<<endl;
	cout<<"Each line indicates a clause where ' ' means a logical OR"<<endl;
	cout<<"For eg. 23 -2 9 6 is a valid clause"<<endl<<endl;
	cout<<"-------------------------------------------------------------------------------"<<endl;
	// simplify the boolean formula to the simplest possible form
	// assign a few values to a few literals which were unit clauses
	do{
		// perform unit propagation
		formula=unitPropagation(formula, assignment);
		
		if(formula.size()==0){
			*goAhead=1;
		}
		// perform pure literal assignment
		formula=pureLiteralAssignment(formula,goAhead);		

		// peform unit propagation if any more clauses added in pureLiteralAssignment 
	}while(*goAhead);

	// function to assign random values to the literals left in the new formula
	do{
		if(formula.size()==0){
			break;
		}
		seed=numOfRandomTries*numOfRandomTries;
		randomAssignment.clear();
		randomAssignment=assignRandomValues(formula,randomAssignment,seed);		
		success=checkSatisfiability(formula,randomAssignment);
		numOfRandomTries++;
	}while(!((success || MAX_TRIES==numOfRandomTries)==1));
	
	// display the results on the terminal
	displaySolution(assignment,randomAssignment,success);

}





// function to read formula from file
vector <string> readFile(){
	int number_of_lines = 0;
	char ch;
	fr = fopen ("C:\\RIT-MS\\HighPerformanceArchitecture\\Project\\Data\\75lits\\UF75.325.100\\uf75-01.cnf", "rt");  
	//	fr = fopen ("C:\\RIT-MS\\HighPerformanceArchitecture\\Project\\Data\\DataTriv.txt", "rt");  

	while (EOF != (ch=getc(fr))){
        if ('\n' == ch){
            ++number_of_lines;
		}
	}
	number_of_lines+=1;
	vector<string> formula(number_of_lines);
	fclose(fr); 
	fr = fopen ("C:\\RIT-MS\\HighPerformanceArchitecture\\Project\\Data\\75lits\\UF75.325.100\\uf75-01.cnf", "rt");  
//		fr = fopen ("C:\\RIT-MS\\HighPerformanceArchitecture\\Project\\Data\\DataTriv.txt", "rt");  
	// "rt" means open the file for reading text 
	char line[80];
	int i=0;
	string delimiter=" 0";
	while(fgets(line, 80, fr) != NULL){
		// get a line, up to 80 chars from fr.  done if NULL 
		string clause(line);
		size_t pos = 0;
		pos = clause.find(delimiter); 
		clause = clause.substr(0, pos);
		formula[i].append(clause);
		i++;
	}
	fclose(fr); 
	return formula;
}

// --Deprecated function
// function to randomly generate a boolean formula with 
// 'literCount': number of boolean variables
// ' ' indicates Logical OR

vector<string> generateFormula( int literalCount ){
	int vbl ; // randomly chossen boolean variable
	int sign ; // randomly polarity of boolean variable
	int sizeOfClause; // randomly chosen size of each clause
	srand(time(NULL));
	int numOfClauses = rand() % (400);
	vector<string> formula(numOfClauses);
	cout<<numOfClauses<<endl;
	for(int i=0 ; i < numOfClauses ; i++){
		sizeOfClause = rand() % 15;
		sizeOfClause+=1;		
		for(int j = 0 ; j < sizeOfClause ; j++){
			vbl	= rand() % literalCount;
			// if 'vbl' is '0' then assign it 1
			// this is to avoid '0' as a literal in the formula 
			if(vbl==0){
				vbl=1;	
			}
			sign= rand() % 2;
			// give polarity to the variable
			if(sign==0){
				vbl*=-1;
			}
			// convert the string to an integer
			ostringstream oss;
			oss<<vbl;
			// append the integer to the clause
			formula[i].append(oss.str());
			// append the logical OR sign (as per my convention) 
			if(j<sizeOfClause-1)
				formula[i].append(" ");
	
		}
	}
	return formula;
}

// function to perform unit propagation with
// 'formula' : vector of strings which consists of clauses
// 'assignment' is the assignment given to literals namely, true or false
vector<string> unitPropagation(vector<string> formula,hash_map<int, bool, hash_compare<int,less<int>>>* assignment){
	// pair of key an value to be inserted
	// key is the literal and value is its assignment
	typedef pair<int, bool> assignPair;
	// delimiter used to show logical OR (as per my convention)
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
				(*assignment).insert(assignPair(literalChosen,true));
				// delete clauses with the literal with same polarity
				// and remove same literal with opposite polarity from all clauses 
				formula=deleteClauses(formula,literalChosen);
				// reset search from first element	
				i=0;
			}
			else{
					i++;
			}
		}
		else{
			i=formula.size();
			break;
		}
		if(formula.size()==0){
			return formula;
		}
	}while(!(i==formula.size() && formula.size()!=0));	
	return formula;
}

// function to delete clauses with 
// 'literalChosen' as one of its literals
vector<string> deleteClauses(vector<string> formula,int literalChosen){
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
vector<string> pureLiteralAssignment(vector<string> formula, int * goAhead){
	// hash map of all literals found in the clauses in the formula 
	hash_map<int, bool> literals;	
	// assignment pair for hash map
	typedef pair<int, bool> assignPair;
	// literal chosen for inspection of singular polarity
	// count of new clauses added to the formula
	int newClause=0;
	int literalChosen=0;
	string delimiter=" ";
	// iterate over all clauses to make a map of literals with literal value and a boolean 'true'
	for(int i=0;i<formula.size();i++){
		size_t pos = 0;
		string token;
		string temp=formula[i];
		pos = temp.find(delimiter);
		while (!(pos == string::npos)) {
			token = temp.substr(0, pos);
			stringstream convert(token);
			convert>>literalChosen;		
			literals.insert(assignPair(literalChosen,true));
			temp.erase(0, pos + delimiter.length());
			pos = temp.find(delimiter);
			if(pos==string::npos && temp.length()!=0){
				pos=temp.length();	
			}
		}
	}
	// iterator to iterate through the map
	typedef hash_map<int, bool>::iterator it_type;
	
	// iterate over the map and check for existance of literals with opposite polarity in the map
	// if there is one, then do not add a clause
	// if there is none, then add a unit clause for that literal
	for(it_type iterator = literals.begin(); iterator != literals.end(); iterator++) {
		literalChosen=iterator->first;
		literalChosen*=-1;
		hash_map<int,bool>::iterator i;
		i = literals.find(literalChosen);
		if( i==literals.end()) {
			ostringstream oss;
			oss<<literalChosen*-1;
			formula.push_back(oss.str());
			newClause++;
		}
 	}
	// if a cluase was added, set global variable goAhead to true
	// this variable decides if further cycle of unit propagation and 
	// pure literal assignment is to be continued or no
	if (newClause>0){
		*goAhead=newClause;	
	}
	else{
		int noGo=0;
		*goAhead=noGo;
	}
	return formula;
}

// function to assign random values to the literals left in the new formula
// 'formula' vector of strings which consists of clauses
// 'randomAssignment' hash_map to store assignment of literals after simplification 
hash_map<int, bool, hash_compare<int,less<int>>> assignRandomValues(vector<string> formula,hash_map<int, bool, hash_compare<int,less<int>>>  randomAssignment, int seed){
	typedef hash_map<int, bool>::iterator it_type2;
	int literalChosen=0;
	string delimiter=" ";
	// pair of key an value to be inserted
	// key is the literal and value is its assignment
	typedef pair<int, bool> assignPair;
	for(int i=0;i<formula.size();i++){
		size_t pos = 0;
		string token;
		string temp=formula[i];
		int sign=0;
		pos = temp.find(delimiter); 
		srand(time(NULL)*seed);
		while(!(pos== string::npos)) {
			token = temp.substr(0, pos);
			stringstream convert(token);
			convert>>literalChosen;
			if(randomAssignment.find(-1*literalChosen)==randomAssignment.end()){
				// give polarity to the variable
				int check=rand()%100;
				if(check<50){
					randomAssignment.insert(assignPair(literalChosen,true));
				}
				else if(check>50){
					randomAssignment.insert(assignPair(literalChosen,false));
				}
			}
			temp.erase(0, pos + delimiter.length());
			pos = temp.find(delimiter);
			if(pos==string::npos && temp.length()!=0){
				pos=temp.length();	
			}
		}
	}
	return randomAssignment;
}

// function to assign random values to the literals left in the new formula
// 'formula' vector of strings which consists of clauses
// 'randomAssignment' hash_map to store assignment of literals after simplification on CPU
bool checkSatisfiability(vector<string> formula,hash_map<int, bool, hash_compare<int,less<int>>> randomAssignment){
	// hash map to keep record of truth value of clause as per assignment
	hash_map<int, bool, hash_compare<int,less<int>>> clauseEvaluation;
	typedef pair<int, bool> assignPair;
	string delimiter=" ";
	int foundOpp;
	int literalChosen=0;
	hash_map<int,bool>::iterator iter;		
	for(int i=0;i<formula.size();i++){
		size_t pos = 0;
		string token;
		string temp=formula[i];		
		pos = temp.find(delimiter);
		while (!(pos== string::npos)) {
			foundOpp=0;
			token = temp.substr(0, pos);
			stringstream convert(token);
			convert>>literalChosen;	
			if(randomAssignment.find(literalChosen)!=randomAssignment.end()){
				iter=(randomAssignment).find(literalChosen);
				// if literal is true, clause is satisfied
				if(iter->second==1){
					clauseEvaluation.insert(assignPair(i,true));				
					break;
				}
				
			}
			else{
				literalChosen*=-1;
				iter=(randomAssignment).find(literalChosen);
				if(literalChosen>0 && iter->second==0){				
				// if literal is flase, clause is satisfied
					clauseEvaluation.insert(assignPair(i,true));				
					break;
				}

				if(literalChosen<0 && iter->second==0){				
				// if literal is false, clause is satisfied
					clauseEvaluation.insert(assignPair(i,true));				
					break;
				}							
			}
			temp.erase(0, pos + delimiter.length());
			pos = temp.find(delimiter);
			if(pos==string::npos && temp.length()!=0){
				pos=temp.length();
			}
		}
	}
	if(clauseEvaluation.size()==formula.size()){
		return true;
	}
	else{
		return false;
	}
}
 
// display the assignment values if a solution exists or 
// display message to say that solution does not exist on CPU
void displaySolution(hash_map<int, bool, hash_compare<int,less<int>>> *assignment,hash_map<int, bool, hash_compare<int,less<int>>>  randomAssignment,bool success){
	typedef hash_map<int, bool>::iterator it_type;
	typedef hash_map<int, bool>::iterator it_type2;
	if(success==1){
		cout<<"Congratulations !"<<endl;
		cout<<endl<<"Formula Satisfied !"<<endl;
		cout<<"( All the lierals not assigned any value below are to be assumed as false )"<<endl;  
		cout<<"Assignments are : "<<endl;
		cout<<"-----------------------------------"<<endl;
		cout<<"Literal \t"<<"Value"<<endl;
		for(it_type iterator = (*assignment).begin(); iterator != (*assignment).end(); iterator++) {
			cout<<iterator->first<<"\t\t"<<iterator->second<<endl;
		}
		for(it_type2 iterator2 = (randomAssignment).begin(); iterator2 != (randomAssignment).end(); iterator2++) {
			cout<<iterator2->first<<"\t\t"<<iterator2->second<<endl;
		}
	}
	else{
		cout<<"Sorry :("<<endl;
		cout<<"There is no solution i.e. possible assignment for this formula !"<<endl;
	}

}

