/* 
 * Program to implement a Patrial Max SAT solver
 * branch and bound method
 */

#include <iostream>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
using namespace std;

const int inf = 0x3f3f3f3f; // value of infinite 

/*
 * enum for different types of return flags defined
 */
enum Cat {
    satisfied,   // when a satisfying assignment has been found
    unsatisfied, // when no satisfying assignment has been found after
			     // exhaustively searching
    normal		 // when no satisfying assignment has been found till now, and DPLL()
				 // has exited normally
};

/*
 * class to represent a boolean formula
 */
class Formula {
public:
    // a vector that stores the value assigned to each variable, where
    // -1 - unassigned
    // 0 - true
    // 1 - false
    vector<int> literals;
	
	// vector to store the number of occurrences of each literal
    vector<int> literal_frequency; 

    // vector to store the difference in number of occurrences with
    // positive and negative polarity of each literal
    vector<int> literal_polarity;

    // vector to store the clauses
    // for each clauses, if the variable n is of positive polarity, then 2n is
    // stored if the variable n is of negative polarity, then 2n+1 is stored here,
    // n is assumed to be zero indexed
    // clauses[0] - hard clauses
    // clauses[1] - soft clauses
    vector<vector<int> > clauses[2];

    // last version regarded the cost of each clause as 1
    // In this version, we use soft_clause_cost to store the cost of each soft clause
    vector<int> soft_clause_cost;

    // int to sore the number of optimal soft clauses cost in every branch
    // if formula is satisified
    int opt_cost;
    
    // int to store the num of sum of removed soft clauses cost
    int remove_cost;

    Formula() {}

    // copy constructor for copying a formula - each member is copied over
    Formula(const Formula &f) {
        literals = f.literals;
        clauses[0] = f.clauses[0];
        clauses[1] = f.clauses[1];
        literal_frequency = f.literal_frequency;
        literal_polarity = f.literal_polarity;
        soft_clause_cost = f.soft_clause_cost;
        opt_cost = f.opt_cost;
        remove_cost = f.remove_cost;
    }

    // set the vectors to their appropriate sizes and initial values
    void initialize(int, int, int);
    
    // set the literate over the clauses
    void input(int, int, int &);
};


void Formula:: initialize(int literal_count, int hard_clause_count, 
        int soft_clause_count){
    
    literals.clear();					// vector for literal
    literals.resize(literal_count, -1); 
    
    clauses[0].clear();				    // vector for hard clauses
    clauses[0].resize(hard_clause_count);
        
    clauses[1].clear();				    // vector for soft clauses
    clauses[1].resize(soft_clause_count);
     
    literal_frequency.clear();
    literal_frequency.resize(literal_count, 0);
     
    literal_polarity.clear();
    literal_polarity.resize(literal_count, 0);
    
    soft_clause_cost.clear();
    soft_clause_cost.resize(soft_clause_count, 0);
    
    opt_cost = remove_cost = 0; 
}

void Formula::input(int hard_clause_count, int soft_clause_count, int &sum_soft_cost){
    int literal; // store the incoming literal value
    // a array that store clause count  
    // 0 - hard clause 1 - soft clause
    int count[2] = {hard_clause_count, soft_clause_count}; 
    sum_soft_cost = 0;
    
    for(int i = 0; i < soft_clause_count; i++){
        cin >> soft_clause_cost[i];   // cost of each soft clause
        sum_soft_cost += soft_clause_cost[i];
    }
    
    for (int p = 0; p < 2; p++) { // point for hard or soft clause
        for (int i = 0; i < count[p]; i++) {
            while (true) { // while the ith clause gets more literals
                cin >> literal;
                if (literal > 0) { // if the variable has positive polarity
                    // store it in the form 2n
                    clauses[p][i].push_back(2 * (literal - 1));
                    // increment frequency and polarity of the literal
                    literal_frequency[literal - 1]++;
                    literal_polarity[literal - 1]++;
                } else if (literal < 0) { // if the variable has negative polarity
                    // store it in the form 2n+1
                    clauses[p][i].push_back(2 * ((-1) * literal - 1) + 1);
                    // increment frequency and decrement polarity of the literal
                    literal_frequency[-1 - literal]++;
                    literal_polarity[-1 - literal]--;
                } else {
                     break; // read 0, so move to next clause
                }
            }
        }
    }
}


/*
 * class to represent the structure and functions of the SAT solver
 */
class PMSATSolver{
private: 
	int literal_count;				// the number of variables in the formula
	Formula formula;			    // the initial hard and soft formula
	int hard_clause_count;			// the number of hard clauses in the formula
	int soft_clause_count;			// the number of soft clauses in the formula
    int sum_soft_cost;              // the number of sum of soft clause in the formula
	
	int unit_propagate(Formula &);	// performs unit propagation
	int DPLL(Formula);				// performs DPLL recursively
	int apply_transform(Formula &, int);// applies the value of the literal
	void display(Formula &, int, int);	// display the result
    int PMSAT(Formula, int);        // performs branch and bound methods recursively

public:
	PMSATSolver() {}
	void initialize();			    // initializes the values
	void solve();				    // calls the solver
};

/*
 * function that accepts the inputs from the user and initializes the attributes
 * in the solver
 */
void PMSATSolver::initialize() {
    char c;   // store first character
    string s; // dummy string

    while (true) {
        cin >> c;
        if (c == 'c') {         // if comment
            getline(cin, s);    // ignore
        } else {                // else, if would be a p
            cin >> s;           // this would be cnf
            break;
        }
    }
    cin >> literal_count;
    cin >> hard_clause_count;
	cin >> soft_clause_count;

	formula.initialize(literal_count, hard_clause_count, soft_clause_count);
	
	formula.input(hard_clause_count, soft_clause_count, sum_soft_cost);
}

/*
 * function to perform unit resolution in a given formula
 * arguments: f - the formula to perform unit resolution on
 * return value: int - the status of the solver after unit resolution, a member
 * of the Cat enum Cat::satisfied - both hard and soft formula has been satisfied
 *                 Cat::unsatisfied - the formula can no longer be satisfied
 *                 Cat::normal - normal exit
 */
int PMSATSolver::unit_propagate(Formula &f) {
    // stores whether the current iteration found a unit clause
    bool unit_clause_found = false; 
    if (f.clauses[0].size() == 0 && f.clauses[1].size() == 0) {	
        // if the formula contains no clauses
        // display(f, Cat::satisfied);
        return Cat::satisfied;		// it is vacuously satisfied
    }
    do {
        unit_clause_found = false;
        // iterate over only the hard clauses in f
        for (int i = 0; i < f.clauses[0].size(); i++) {
            // if the size of a clause is 1, it is a unit clause
            if (f.clauses[0][i].size() == 1) { 
                unit_clause_found = true;
                // 0 - if true, 1 - if false, set the literal
                f.literals[f.clauses[0][i][0] / 2] = f.clauses[0][i][0] % 2; 
                // once assigned, reset the frequency to mark it closed
                f.literal_frequency[f.clauses[0][i][0] / 2] = -1; 
                // apply this change through f
                int result = apply_transform(f, f.clauses[0][i][0] / 2); 
                // if this caused the formula to be either satisfied 
                // or unsatisfied, return the result flag
                if (result == Cat::satisfied || result == Cat::unsatisfied) {
                    return result;
                }
                break; // check for another unit clause from start
            } 
            // else if (f.clauses[p][i].size() == 0) { // ????
            //     // if a given clause is empty
            //     return Cat::unsatisfied; // unsatisfiable in this branch
            // }
            // continue do-whiile loop to check for another unit clause from start
            if(unit_clause_found) break;
        } 
    } while (unit_clause_found);

    return Cat::normal; // if reached here, the unit resolution ended normally
}

/*
 * applies a value of a literal to all clauses in a given formula
 * arguments: f - the formula to apply on
 *            literal_to_apply - the literal which has just been set
 * return value: int - the return status flag, a member of the Cat enum
 *               Cat::satisfied - both hard and soft formula has been satisfied
 *               Cat::unsatisfied - the formula can no longer be satisfied
 *               Cat::normal - normal exit
 */
int PMSATSolver::apply_transform(Formula &f, int literal_to_apply) {
    // the value to apply, 0 - if true, 1 - if false
    int value_to_apply = f.literals[literal_to_apply]; 
    for (int p = 0; p < 2; p++) {
        // iterate over the hard clauses in f
        for (int i = 0; i < f.clauses[p].size(); i++) {
            // iterate over the variables in the clause
            for (int j = 0; j < f.clauses[p][i].size(); j++) {
                // if this is true, then the literal appears with the same polarity 
                // as it is being applied that is, if assigned true, it appears 
                // positive if assigned false, it appears negative, in this clause 
                // hence, the clause has now become true
                if ((2 * literal_to_apply + value_to_apply) == f.clauses[p][i][j]) {
                    // remove the clause from the list
                    f.clauses[p].erase(f.clauses[p].begin() + i); 
                    if (p == 1){  // soft clause
                        // add clause cost to opt_cost
                        f.opt_cost += f.soft_clause_cost[i];
                        // remove the clause cost from the list
                        f.soft_clause_cost.erase(f.soft_clause_cost.begin() + i);
                    }
                    i--;                // reset iterator
                    // if all hard and soft clauses have been removed
                    if (f.clauses[0].size() == 0 && f.clauses[1].size() == 0 ) { 
                        return Cat::satisfied;  // the formula is satisfied
                    }
                    break; // move to the next clause
                } else if (f.clauses[p][i][j] / 2 == literal_to_apply) {
                    // the literal appears with opposite polarity 
                    // remove the literal from the clause, as it is false in it
                    f.clauses[p][i].erase(f.clauses[p][i].begin() + j); 
                    j--;    // reset the iterator
                    if (f.clauses[p][i].size() == 0) {
                        if(p == 0){
                            // if the hard clause is empty
                            // formula is unsatisfiable currently
                            return Cat::unsatisfied;
                         }  else {
                            // remove the clause from list
                            f.clauses[p].erase(f.clauses[p].begin() + i);
                            // add clause cost to remove cost
                            f.remove_cost += f.soft_clause_cost[i];
                            // remove the clause cost from the list
                            f.soft_clause_cost.erase(f.soft_clause_cost.begin() + i);
                            i--;
                        }
                    }
                    break; // move to the next clause
                }
            }
        }
    }
    // if reached here, the function is exiting normally
    return Cat::normal;
}

void PMSATSolver::display(Formula &f, int result, int ans) {
    cout << endl << "******** display ***********" << endl;
    if (result == Cat::satisfied) { // if the formula is satisfiable
        cout << "SAT" << endl;
        for (int i = 0; i < f.literals.size(); i++) {
            if (i != 0) {
                cout << " ";
            }
            if (f.literals[i] != -1) {
                cout << pow(-1, f.literals[i]) * (i + 1);
            } else {  
                // for literals which can take either value, 
                // arbitrarily assign them to be true
                cout << (i + 1);
            }
        }
        cout << " 0" << endl; 
        cout << ans << endl;
    } else { // if the formula is unsatisfiable
        cout << "UNSAT";
    }
    cout << "****************************" << endl << endl;
}

/* 
 * function to perform the branch and bound method on a given formula
 * argument: formula - the formula to perform branch and bound method on
 *           lower_bound - value of optimal complete solution initialized to inf
 * return value: int - value of optimal complete solution
 *               inf - no satisfiable solution
 */
int PMSATSolver::PMSAT(Formula f, int lower_bound){
    // purning process
	// lower_bound is the optimalcomplete solution initialized to -inf
	// upper_bound is number of empty clause in f at most
    int upper_bound = sum_soft_cost - f.remove_cost;
    if(upper_bound <= lower_bound) return lower_bound;
   
    int result = unit_propagate(f); // perform unit propagation on the formula 
    // to store empty soft clause number
    
    if(result == Cat::satisfied) {  // if satisfied, show result and return
        int ans = f.opt_cost;
        display(f, result, ans);
        return ans;         // answer is lower bound 
    } else if(result == Cat::unsatisfied) { // if hard clauses not satisfied
        return -inf;                // return -inf
    }
    
    // find the variable with maximum frequency in f, which will be the next to be
    // assigned a value already assigned variables have this field reset to -1 in
    // order to ignore them
    int i = distance(f.literal_frequency.begin(), 
            max_element(f.literal_frequency.begin(), f.literal_frequency.end()));
    // need to apply twice, once true, the other false
    for (int j = 0; j < 2; j++) {
        Formula new_f = f; // copy the formula before recursing
        new_f.literals[i] = j;  // assign positive first
        new_f.literal_frequency[i] = -1; 
        // reset the frequency to -1 to ignore in the future
        int transform_result = apply_transform(new_f, i); 
        int ret = new_f.opt_cost;
        // apply the change to all the clauses
        if (transform_result == Cat::satisfied) { 
            // if formula satisfied both hard and soft clause
            // meas all literal has been selected
            display(new_f, transform_result, ret);
            lower_bound = max(lower_bound, ret);
        } else if (transform_result == Cat::unsatisfied) { 
            // if formula not satisfied in this branch, return inf 
            lower_bound = max(lower_bound, -inf); // just for completement
        } 
        else {
            // after apply, there is not either satisfied or unsatisfied
            // recursively call PMSAT on the new formula
            // to update upper_bound
            lower_bound = max(lower_bound, PMSAT(new_f, lower_bound));
        }
    }
    return lower_bound;
}

void PMSATSolver::solve(){
    int result = PMSAT(formula, -inf);
    if(result == -inf) cout << "UNSAT" << endl;
    else cout << "PMS result: " << result << endl;
}

int main() {
    PMSATSolver solver;     // create the solver
    solver.initialize();    // initialize
    solver.solve();         // solve
    return 0;
}

