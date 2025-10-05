#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <set>
#include <cmath>
#include <algorithm>

using namespace std;

//by proeject1
struct PlaData {
	int inputNum = 0;	//count how many variable
	int productNum = 0;	//used in for loop to check how many product lines have to read
	vector<string> varNames;	//store the different variable names
	map<string, int> productLine;	//the result and the product line command
};

struct Term {
	string pattern;	//the minterm pattern
	set<int> minterm;	//collect the pattern that have been simply, and contain which minterms
	bool combined = false;
};

PlaData readPlaFile(string fileName) {
	PlaData data;
	ifstream file(fileName);
	if (!file.is_open()) {
		cout << "Can not open the file:" << fileName << endl;
		return data;
	}
	string line;
	string temp;
	bool end = false;

	//read .i
	getline(file, line);
	stringstream ss(line);
	ss >> temp;
	ss >> temp;
	data.inputNum = stoi(temp);

	//read .o (we only use one output)
	getline(file, line);

	//read .ilb
	getline(file, line);
	stringstream ss1(line);
	ss1 >> temp;	//skip .lib;
	for (int i = 0; i < data.inputNum; i++) {
		ss1 >> temp;
		data.varNames.push_back(temp);
	}

	//read .ob f
	getline(file, line);

	//read .p 
	getline(file, line);
	stringstream ss2(line);
	ss2 >> temp;	//skip .p;	
	ss2 >> temp;
	data.productNum = stoi(temp);

	//read the products
	for (int i = 0; i < data.productNum; i++) {
		getline(file, line);
		stringstream ss3(line);
		string product;
		int result;

		ss3 >> product;
		ss3 >> temp;
		if(temp == "-") result = -1;
		else result = stoi(temp);
		data.productLine[product] = result;
	}

	//read the .e, end the read
	getline(file, line);
	stringstream ss4(line);
	ss4 >> temp;
	if (temp == ".e") end = true;

	file.close();
	return data;
}

//make the number into binary format and return the string
string intoBinary(int num, int inputNum) {	//inputNum can represent how many bits
	string result = "";
	if (num < 2) {
		result = result + to_string(num);
	}
	else {
		while (num) {
			if (num % 2 == 1) result = "1" + result;
			else result = "0" + result;
			num /= 2;
		}
	}
	while (result.length() < inputNum) {
		result = "0" + result;
	}
	return result;
}

//make the binary string into int, cause I need to know which minterm is covered
int binaryToInt(const string &binary)
{
	int result = 0;
	int power = 1;
	for (int i = binary.length() - 1; i >= 0; i--)
	{
		if (binary[i] == '1')
		{
			result += power;
		}
		power *= 2;
	}
	return result;
}

//build the truth table with all the input 
map<string, int> buildTruthTable(int inputNum, PlaData& data) {
	map<string, int> truthTable;
	int elementAmount = pow(2, inputNum);
	string temp;
	for (int i = 0; i < elementAmount; i++) {
		temp = intoBinary(i, inputNum);
		truthTable[temp] = 0;	//declare the element 0 first, the value would change after the data check
	}
	return truthTable;
}

vector<string> splitProduct(string product) {
	vector<string> result;
	bool hasHivent = false;
	for (int i = 0; i < product.length(); i++) {
		if (product[i] == '-') {
			hasHivent = true;
			string temp1 = product;
			string temp2 = product;
			temp1[i] = '0';
			temp2[i] = '1';
			//do the recusion if the product has more than one '-'
			vector<string> result1 = splitProduct(temp1);
			for (auto it : result1) {
				result.push_back(it);
			}
			vector<string> result2 = splitProduct(temp2);
			for (auto it : result2) {
				result.push_back(it);
			}
			return result;
		}

	}
	if(!hasHivent) result.push_back(product);
	return result;
}

//change the truth table according to the product line
void changeTruthTable(map<string, int> &truthTable, PlaData data)
{
	// build the truth table according to the product line
	for (auto element : data.productLine){
		if (element.first.find('-') != string::npos){	// if the product has don't care, expand it first
			vector<string> extraProduct = splitProduct(element.first);
			for (auto it : extraProduct){
				if (truthTable[it] == 0){ // only change when the value is 0
					truthTable[it] = element.second;	//might be 1 or -1
				}
			}
		}
		else{
			if (truthTable[element.first] == 0){	// only change when the value is 0
				truthTable[element.first] = element.second;
			}
		}
	}

	// check the product line again, make don't care replace by 1
	for (auto element : data.productLine){
		if (element.second == 1){
			if (element.first.find('-') != string::npos){
				vector<string> extraProduct = splitProduct(element.first);
				for (auto it : extraProduct){
					truthTable[it] = 1;
				}
			}
			else{
				truthTable[element.first] = 1;
			}
		}
	}
}

//from here is the Quine-McCluskey Algorithm part
//cout 1 to make groups
int countOnes(const string& term){
	int count = 0;
	for(auto it : term){
		if(it == '1') count++;
	}
	return count;	
}

//make groups by the number of 1s in the binary string
map<int, vector<Term>> groupByOne(const map<string, int> &truthTable)
{
	map<int, vector<Term>> groups;	// store the groups by number of 1s
	// key is the number of 1s, value is the list of terms
	for (auto term : truthTable){
		if (term.second == 1 || term.second == -1) {	// 1 and don't care
			int ones = countOnes(term.first);
			Term newTerm;
			newTerm.pattern = term.first;

			int mintermIndex = binaryToInt(term.first);
			newTerm.minterm.insert(mintermIndex);
			newTerm.combined = false;

			groups[ones].push_back(newTerm);
		}
	}
	return groups;
}

//check if two terms can be combined, return the position of the different bit
int canCombine(const string& term1, const string& term2){
	if(term1.length() != term2.length()) return -1;
	int diff = 0;	// check only one bit different
	int pos = -1;
	for(int i = 0; i < term1.length(); i++){
		if (term1[i] == '-' && term2[i] == '-'){
			continue;
		}
		else if(term1[i] == '-' || term2[i] == '-'){
			return -1;
		}
		else if(term1[i] != term2[i]){
			diff++;
			pos = i;
		}
		
		if(diff > 1){return -1;}
	}
	return pos;
}

//combine two terms into a new term by the hyphem method
string combine(const string&term1, const string& term2){
	string newTerm;
	newTerm = term1;
	int pos = canCombine(term1, term2);
	if(pos != -1) newTerm[pos] = '-';
	else newTerm = "";
	return newTerm;
}

//combine the neighbor groups
map<int, vector<Term>> combineGroups(map<int, vector<Term>> &groups){
	map<int, vector<Term>> newGroups;	// store the new groups after combine
	map<string, Term> patternToTerm;	// store the new pattern make sure no double term

	// collect all the group keys for map, to make sure every index is existing
	vector<int> groupKeys;
	for (auto &gro : groups){
		groupKeys.push_back(gro.first);
	}

	// combine every two term in neighbor groups
	for (int i = 0; i < groupKeys.size() - 1; i++){
		int currentGroup = groupKeys[i];
		int nextGroup = groupKeys[i + 1];

		if (nextGroup - currentGroup != 1) continue;	// check if they are neighbor groups

		for (auto &subGroup : groups[currentGroup]){
			for (auto &nextSubGroup : groups[nextGroup]){
				int pos = canCombine(subGroup.pattern, nextSubGroup.pattern);	// check if can combine
				if (pos != -1){	//the -1 means cannot combine
					string newPattern = combine(subGroup.pattern, nextSubGroup.pattern);
					if (newPattern != "" && (patternToTerm.find(newPattern) == patternToTerm.end())){
						// create a new term
						Term newTerm;
						newTerm.pattern = newPattern;
						newTerm.minterm = subGroup.minterm;
						newTerm.minterm.insert(nextSubGroup.minterm.begin(), nextSubGroup.minterm.end());
						patternToTerm[newPattern] = newTerm;

						// mark the old terms as combined
						for (auto &term : groups[currentGroup]){
							if (term.pattern == subGroup.pattern){
								term.combined = true;
							}
						}
						// mark the old terms as combined
						for (auto &term : groups[nextGroup]){
							if (term.pattern == nextSubGroup.pattern){
								term.combined = true;
							}
						}
					}
				}
			}
		}
	}

	for (auto it : patternToTerm){
		int ones = countOnes(it.first);
		newGroups[ones].push_back(it.second);
	}
	return newGroups;
}

void printOutPI(vector<Term> primeImplicants){
	 cout << "\nPrime Implicants " << endl;
    for(int i = 0; i < primeImplicants.size(); i++){
        cout << "PI" << i << ": " << primeImplicants[i].pattern << " covers {";
        for(int m : primeImplicants[i].minterm) cout << m << " ";
        cout << "}" << endl;
    }
}

vector<int> buildPIChart(map<string, int>& truthTable, PlaData& data, vector<Term> primeImplicants){
	// Step 1: collect minterms
	vector<int> onSetMinterms;
	for(int i = 0; i < truthTable.size(); i++){
		string binary = intoBinary(i, data.inputNum);
		if(truthTable[binary] == 1){  // find the minterm with output 1, the don't care not record first
		//if(truthTable[binary] == 1 || truthTable[binary] == -1){	
			onSetMinterms.push_back(i);
		}
	}

	// Step 2: build the PI chart 
	// mintermToPIs[m] = where m is covered by which PIs
	map<int, vector<int>> mintermToPIs;

	for(int i = 0; i < primeImplicants.size(); i++){
		for(int m : primeImplicants[i].minterm){
			// only check the minterm in on-set
			if(truthTable[intoBinary(m, data.inputNum)] == 1){
				mintermToPIs[m].push_back(i);
			}
		}
	}

	// Step 3: find EPI 
	vector<int> essentialPIs;	//EPI number
	set<int> coveredByEPIs;  //minterms covered by EPI

	for(int m : onSetMinterms){
		if(mintermToPIs[m].size() == 1){
			// if the minterm is covered by only one PI -> Essential!
			int epi = mintermToPIs[m][0];
			
			// check if already added
			if(find(essentialPIs.begin(), essentialPIs.end(), epi) == essentialPIs.end()){
				essentialPIs.push_back(epi);
				// check this EPI cover which minterms
				for(int covered : primeImplicants[epi].minterm){
					if(truthTable[intoBinary(covered, data.inputNum)] == 1){
						coveredByEPIs.insert(covered);
					}
				}
			}
		}
	}

	//find the uncovered 
	// Step 4: find uncovered minterms 
	vector<int> uncoveredMinterms;
	for(int m : onSetMinterms){
		if(coveredByEPIs.find(m) == coveredByEPIs.end()){
			uncoveredMinterms.push_back(m);
		}
	}

	if (uncoveredMinterms.empty()){	// if all minterms are covered by EPI
		// all be covered by EPI
		vector<int> selectedPIs = essentialPIs;	

		// find the lowest literals
		int totalTerms = selectedPIs.size();	// total number of terms
		int totalLiterals = 0;	//
		for (int pi : selectedPIs){
			for (char c : primeImplicants[pi].pattern){
				if (c != '-')
					totalLiterals++;
			}
		}

		return selectedPIs; 
	}
	else{ // Step 5: maybe is the Petrick's Method
		// find the candidate PIs that can cover the uncovered minterms
		vector<int> candidatePIs;
		for (int i = 0; i < primeImplicants.size(); i++){
			// skip EPIs
			if (find(essentialPIs.begin(), essentialPIs.end(), i) != essentialPIs.end()) continue;

			// check whether pi i cover any uncovered minterm
			bool useful = false;
			for (int m : uncoveredMinterms){
				// check whether pi i is in mintermToPIs[m]
				if (find(mintermToPIs[m].begin(), mintermToPIs[m].end(), i) != mintermToPIs[m].end()){
					useful = true;
					break;
				}
			}

			if (useful){	// if this PI can cover any uncovered minterm, add to candidate list
				candidatePIs.push_back(i);
			}
		}

		// 2. Enumerate all combinations (using bitmask)
		vector<vector<int>> validSolutions;
		int minSize = candidatePIs.size() + 1; 

		// use bitmask to print out all combinations (from 1 to 2^n - 1)
		for (int mask = 1; mask < (1 << candidatePIs.size()); mask++){
			// use the bitmask to build the combination
			// such as 101 means include candidatePIs[0] and candidatePIs[2]
			// 011 means include candidatePIs[1] and candidatePIs[2]
			vector<int> combination;

			// build the combination from the bitmask, such as
			// there are {3,5,7}, then 001 means {7}, 010 means {5}, 011 means {5,7}
			for (int i = 0; i < candidatePIs.size(); i++){
				if (mask & (1 << i)){	
					// here use the method of the bit counting
					// this means if the i-th bit is 1, include candidatePIs[i]
					combination.push_back(candidatePIs[i]);
				}
			}

			// check that this combination covers all uncovered minterms
			bool coversAll = true;
			for (int m : uncoveredMinterms){
				bool covered = false;
				for (int pi : combination){
					// check whether pi is in mintermToPIs[m]
					if (primeImplicants[pi].minterm.find(m) != primeImplicants[pi].minterm.end()){
						covered = true;
						break;
					}
				}
				if (!covered){	// if any minterm is not covered
					coversAll = false;
					break;
				}
			}

			if (coversAll){	// if this combination covers all uncovered minterms, then it's a valid solution
				if (combination.size() < minSize){	// if found a smaller combination
					validSolutions.clear();	// clear previous solutions
					validSolutions.push_back(combination);	
					minSize = combination.size();
				}
				else if (combination.size() == minSize){
					validSolutions.push_back(combination);
				}
			}
		}
		
		if(validSolutions.empty()){
			cout << "No valid solution found to cover all minterms!" << endl;
			vector<int> selectedPIs = essentialPIs;
			for (int pi : candidatePIs){
				selectedPIs.push_back(pi);
			}
			return selectedPIs;
		}

		// 3. choose the best solution (minimize literals)
		int bestSolution = 0;
		int minLiterals = 999999;	// a large number

		for (int i = 0; i < validSolutions.size(); i++){
			int literals = 0;

			// count literals in this solution
			for (int pi : validSolutions[i]){
				for (char c : primeImplicants[pi].pattern){	// count the number of literals
					if (c != '-')
						literals++;
				}
			}

			if (literals < minLiterals){	// if found a better solution
				minLiterals = literals;	// update minLiterals
				bestSolution = i;	// update best solution index
			}
		}

		// 4. combine the best solution with EPIs
		vector<int> selectedPIs = essentialPIs;
		for (int pi : validSolutions[bestSolution]){
			selectedPIs.push_back(pi);
		}

		sort(selectedPIs.begin(), selectedPIs.end());

		int totalTerms = selectedPIs.size();
		int totalLiterals = 0;
		for (int pi : selectedPIs){	// count total literals
			for (char c : primeImplicants[pi].pattern){	// count the number of literals
				if (c != '-')
					totalLiterals++;
			}
		}
		return selectedPIs;
	}
}

void writePLA(string &outputFileName, PlaData& data, vector<Term> primeImplicants, vector<int> selectedPIs){
	//  print out the output file 
	cout << "\nWriting output to " << outputFileName << "..." << endl;

	ofstream outFile(outputFileName);
	if (!outFile.is_open()){
		cout << "Error: Cannot create output file!" << endl;
	}

	// write the PLA format
	outFile << ".i " << data.inputNum << endl;
	outFile << ".o 1" << endl;
	outFile << ".ilb";
	for (const string &var : data.varNames){
		outFile << " " << var;
	}
	outFile << endl;
	outFile << ".ob f" << endl;
	outFile << ".p " << selectedPIs.size() << endl;

	// write the selected prime implicants
	for (int pi : selectedPIs){
		outFile << primeImplicants[pi].pattern << " 1" << endl;
	}

	outFile << ".e" << endl;
	outFile.close();

	cout << "Output file created successfully!" << endl;
}

int main(int argc, char *argv[])
{
	// check the input command
	if (argc != 3){
		cout << "Usage: " << argv[0] << " <input.pla> <output.pla>" << endl;
		return 1;
	}

	string inputFileName = argv[1];
	string outputFileName = argv[2];

	// read PLA file
	PlaData data = readPlaFile(inputFileName);
	if (data.varNames.empty()){
		cout << "Failed to read input file!" << endl;
		return 1;
	}

	// build truth table
	map<string, int> truthTable = buildTruthTable(data.inputNum, data);
	changeTruthTable(truthTable, data);

	// Quine-McCluskey Algorithm 
	// original groups
	map<int, vector<Term>> groups = groupByOne(truthTable);

	vector<map<int, vector<Term>>> allRounds;
	allRounds.push_back(groups);

	// do the combine process
	int round = 1;	// count how many rounds
	while (true){
		map<int, vector<Term>> newGroups = combineGroups(groups);

		if (newGroups.empty()){	// if no new groups, stop
			break;
		}

		allRounds.push_back(newGroups);
		groups = newGroups;
		round++;

		if (round > 10){	// i set a limit to avoid infinite loop
			cout << "Warning: Stopped at round 10, it means this combination is too complex." << endl;
			break;
		}
	}

	// collect Prime Implicants from all rounds
	vector<Term> allPrimeImplicants;
	set<string> seenPIPatterns;

	for (int round = 0; round < allRounds.size(); round++){
		for (auto &group : allRounds[round]){
			for (auto &term : group.second){
				// check if this term's minterms are subset of any higher level term
				// if yes, then this term is not a prime implicant
				// if no, then this term is a prime implicant
				bool gotCombined = false;

				for (int nextLevel = round + 1; nextLevel < allRounds.size(); nextLevel++){
					for (auto &higherGroup : allRounds[nextLevel]){
						for (auto &higherTerm : higherGroup.second){
							// check if all minterms in term are in higherTerm.minterm
							bool allIncluded = true;
							for (int m : term.minterm){
								if (higherTerm.minterm.find(m) == higherTerm.minterm.end()){
									allIncluded = false;
									break;
								}
							}

							// If all minterms are included, and not exactly the same (indicating it has been combined)
							if (allIncluded && term.minterm.size() < higherTerm.minterm.size())	{
								gotCombined = true;
								break;
							}
						}
						if (gotCombined) break;	// if found, break the loop
					}
					if (gotCombined) break;
				}

				if (!gotCombined && seenPIPatterns.find(term.pattern) == seenPIPatterns.end()){
					allPrimeImplicants.push_back(term);
					seenPIPatterns.insert(term.pattern);
				}
			}
		}
	}
	//printOutPI(allPrimeImplicants);

	// PI Chart & Petrick's Method 
	vector<int> selectedPIs = buildPIChart(truthTable, data, allPrimeImplicants);

	// Write Output  FINALLY! 
	writePLA(outputFileName, data, allPrimeImplicants, selectedPIs);

	return 0;
}