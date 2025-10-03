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
	string pattern;
	set<int> minterm;
	bool combined = false;
	int mintermNum = 0;
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

void changeTruthTable(map<string, int>& truthTable, PlaData data) {
	for (auto element : data.productLine) {
		if (element.first.find('-') != string::npos) {
			vector<string> extraProduct = splitProduct(element.first);
			for (auto it : extraProduct) {
				truthTable[it] = element.second;
			}
		}
		else {
			truthTable[element.first] = element.second;
		}
	}
}

//do the groups distribute
int countOnes(const string& term){
	int count = 0;
	for(auto it : term){
		if(it == '1') count++;
	}
	return count;	
}

map<int, vector<Term>> groupByOne(const map<string, int> &truthTable){
	map<int, vector<Term>> groups;
	int index = 0;
	for(auto term : truthTable){
		if(term.second == 1 || term.second == -1){
			int ones = countOnes(term.first);
			Term newTerm;
			newTerm.pattern = term.first;
			newTerm.minterm.insert(index);
			newTerm.combined = false;
			groups[ones].push_back(newTerm);
		}
		index++;
	}
	return groups;
}

int canCombine(const string& term1, const string& term2){
	if(term1.length() != term2.length()) return -1;
	int diff = 0;
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

string combine(const string&term1, const string& term2){
	string newTerm;
	newTerm = term1;
	int pos = canCombine(term1, term2);
	if(pos != -1) newTerm[pos] = '-';
	else newTerm = "";
	return newTerm;
}

map<int, vector<Term>> combineGroups(map<int, vector<Term>>& groups){
	map<int, vector<Term>> newGroups;
	map<string, Term> patternToTerm; //to store the term by its pattern
	//combine the groups	
	for(int i = 0 ; i < groups.size() - 1; i++){
		//check whether the term exist
		if(groups.find(i) == groups.end() || groups.find(i+1) == groups.end()){
            continue;
        }
		for(auto &subGroup : groups[i]){
			for(auto &nextSubGroup : groups[i + 1]){
				int pos = canCombine(subGroup.pattern, nextSubGroup.pattern);
				if(pos != -1){
					string newPattern = combine(subGroup.pattern, nextSubGroup.pattern);
					if(newPattern != "" && (patternToTerm.find(newPattern) == patternToTerm.end())){
						Term newTerm;
						newTerm.pattern = newPattern;
						newTerm.minterm = subGroup.minterm;
						newTerm.minterm.insert(nextSubGroup.minterm.begin(), nextSubGroup.minterm.end());
						patternToTerm[newPattern] = newTerm;
					}
				}
			}
		}
	}

	for(auto it : patternToTerm){
		int ones = countOnes(it.first);
		newGroups[ones].push_back(it.second);
	}
	return newGroups;
}

//from the groups get all the terms that can't combine anymore
vector<Term> getAllTerms(const map<int, vector<Term>>& groups, set<int>& minternVar){
	vector<Term> allTerms;
	for(auto it : groups){
		for(auto subIt : it.second){
			allTerms.push_back(subIt);
			for(auto m : subIt.minterm){
				minternVar.insert(m);
			}
		}
	}
	return allTerms;
}

void printOutPI(vector<Term> primeImplicants){
	 cout << "\n=== Prime Implicants ===" << endl;
    for(int i = 0; i < primeImplicants.size(); i++){
        cout << "PI" << i << ": " << primeImplicants[i].pattern << " covers {";
        for(int m : primeImplicants[i].minterm) cout << m << " ";
        cout << "}" << endl;
    }
}
vector<int> buildPIChart(map<string, int>& truthTable, PlaData& data, vector<Term> primeImplicants){
	// Step 1: collect minterms
	cout << "\n=== Prime Implicant Chart ===" << endl;

	vector<int> onSetMinterms;
	for(int i = 0; i < truthTable.size(); i++){
		string binary = intoBinary(i, data.inputNum);
		if(truthTable[binary] == 1){  // find the minterm with output 1, the don't care not record first
		//if(truthTable[binary] == 1 || truthTable[binary] == -1){	
			onSetMinterms.push_back(i);
		}
	}

	cout << "On-set minterms (output=1): {";
	for(int m : onSetMinterms) cout << m << " ";
	cout << "}\n" << endl;

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

	// Step 3: print the table 
	cout << "Prime Implicant Chart:" << endl;
	cout << "       ";
	for(int m : onSetMinterms){
		if(m < 10) cout << " m" << m << " ";
		else cout << " m" << m;
	}
	cout << endl;

	for(int i = 0; i < primeImplicants.size(); i++){
		cout << "PI" << i << "   ";
		for(int m : onSetMinterms){
			bool covered = false;
			// check whether pi i cover m
			for(int pi : mintermToPIs[m]){
				if(pi == i){
					covered = true;
					break;
				}
			}
			cout << (covered ? "  X " : "    ");
		}
		cout << "  " << primeImplicants[i].pattern << endl;
	}

	// Step 4: find EPI 
	cout << "\n Finding Essential Prime Implicants " << endl;

	vector<int> essentialPIs;	//EPI number
	set<int> coveredByEPIs;  //minterms covered by EPI

	for(int m : onSetMinterms){
		if(mintermToPIs[m].size() == 1){
			// if the minterm is covered by only one PI -> Essential!
			int epi = mintermToPIs[m][0];
			
			// check if already added
			if(find(essentialPIs.begin(), essentialPIs.end(), epi) == essentialPIs.end()){
				essentialPIs.push_back(epi);
				cout << "m" << m << " only covered by PI" << epi 
					<< " (" << primeImplicants[epi].pattern << ") -> Essential!" << endl;
				
				// check this EPI cover which minterm
				for(int covered : primeImplicants[epi].minterm){
					if(truthTable[intoBinary(covered, data.inputNum)] == 1){
						coveredByEPIs.insert(covered);
					}
				}
			}
		}
	}

	cout << "\nEssential PIs: {";
	for(int epi : essentialPIs) cout << "PI" << epi << " ";
	cout << "}" << endl;

	cout << "Covered by EPIs: {";
	for(int m : coveredByEPIs) cout << m << " ";
	cout << "}" << endl;

	//find the uncovered 
	// Step 5: find uncovered minterms 
	vector<int> uncoveredMinterms;
	for(int m : onSetMinterms){
		if(coveredByEPIs.find(m) == coveredByEPIs.end()){
			uncoveredMinterms.push_back(m);
		}
	}

	cout << "\nStill need to cover: {";
	for(int m : uncoveredMinterms) cout << m << " ";
	cout << "}" << endl;

	if(uncoveredMinterms.empty()){
		cout << "\nAll minterms covered by Essential PIs!" << endl;
		cout << "No need for Petrick's Method." << endl;

		vector<int> selectedPIs = essentialPIs;
		
	}

	// Step 6: Petrick's Method  
	cout << "\nPetrick's Method" << endl;

	if (uncoveredMinterms.empty())
	{
		// all be covered by EPI
		vector<int> selectedPIs = essentialPIs;

		cout << "\nFinal Solution" << endl;
		cout << "Selected PIs: {";
		for (int pi : selectedPIs)
			cout << "PI" << pi << " ";
		cout << "}" << endl;
	}
	else
	{
		// Petrick's Method
		// find the candidate PIs that can cover the uncovered minterms
		cout << "Uncovered minterms and their covering PIs:" << endl;
		for (int m : uncoveredMinterms)
		{
			cout << "  m" << m << " can be covered by: {";
			for (int pi : mintermToPIs[m])
			{
				cout << "PI" << pi << " ";
			}
			cout << "}" << endl;
		}

		vector<int> candidatePIs;
		for (int i = 0; i < primeImplicants.size(); i++)
		{
			// skip EPIs
			if (find(essentialPIs.begin(), essentialPIs.end(), i) != essentialPIs.end())
			{
				continue;
			}

			// check whether pi i cover any uncovered minterm
			bool useful = false;
			for (int m : uncoveredMinterms)
			{
				// check whether pi i is in mintermToPIs[m]
				if (find(mintermToPIs[m].begin(), mintermToPIs[m].end(), i) != mintermToPIs[m].end())
				{
					useful = true;
					break;
				}
			}

			if (useful)
			{
				candidatePIs.push_back(i);
			}
		}

		cout << "\nCandidate PIs (non-Essential): {";
		for (int pi : candidatePIs)
			cout << "PI" << pi << " ";
		cout << "}" << endl;

		// 2. Enumerate all combinations (using bitmask)
		cout << "\nEnumerating all possible combinations..." << endl;

		vector<vector<int>> validSolutions;
		int minSize = candidatePIs.size() + 1; 

		// use bitmask to enumerate all combinations (from 1 to 2^n - 1)
		for (int mask = 1; mask < (1 << candidatePIs.size()); mask++)
		{
			vector<int> combination;

			// build the combination from the bitmask
			for (int i = 0; i < candidatePIs.size(); i++)
			{
				if (mask & (1 << i))
				{
					combination.push_back(candidatePIs[i]);
				}
			}

			// check that this combination covers all uncovered minterms
			bool coversAll = true;
			for (int m : uncoveredMinterms)
			{
				bool covered = false;
				for (int pi : combination)
				{
					// check whether pi is in mintermToPIs[m]
					if (primeImplicants[pi].minterm.find(m) != primeImplicants[pi].minterm.end())
					{
						covered = true;
						break;
					}
				}
				if (!covered)
				{
					coversAll = false;
					break;
				}
			}

			if (coversAll)
			{
				if (combination.size() < minSize)
				{
					// if found smaller solution, clear previous ones
					validSolutions.clear();
					validSolutions.push_back(combination);
					minSize = combination.size();
				}
				else if (combination.size() == minSize)
				{
					validSolutions.push_back(combination);
				}
			}
		}

		cout << "Found " << validSolutions.size() << " solution(s) with "
			 << minSize << " PI(s)" << endl;

		// 3. choose the best solution (minimize literals)
		int bestSolution = 0;
		int minLiterals = 999999;

		for (int i = 0; i < validSolutions.size(); i++)
		{
			int literals = 0;
			cout << "\nSolution " << i + 1 << ": {";
			for (int pi : validSolutions[i])
			{
				cout << "PI" << pi << " ";
				// cout the literals count
				for (char c : primeImplicants[pi].pattern)
				{
					if (c != '-')
						literals++;
				}
			}
			cout << "} - " << literals << " literals" << endl;

			if (literals < minLiterals)
			{
				minLiterals = literals;
				bestSolution = i;
			}
		}

		cout << "\nBest solution is #" << (bestSolution + 1) << endl;

		// 4. combine the best solution with EPIs
		vector<int> selectedPIs = essentialPIs;
		for (int pi : validSolutions[bestSolution])
		{
			selectedPIs.push_back(pi);
		}

		sort(selectedPIs.begin(), selectedPIs.end());

		cout << "\n=== Final Solution ===" << endl;
		cout << "Selected PIs: {";
		for (int pi : selectedPIs)
		{
			cout << "PI" << pi << "(" << primeImplicants[pi].pattern << ") ";
		}
		cout << "}" << endl;

		// cout total
		int totalTerms = selectedPIs.size();
		int totalLiterals = 0;
		for (int pi : selectedPIs)
		{
			for (char c : primeImplicants[pi].pattern)
			{
				if (c != '-')
					totalLiterals++;
			}
		}

		cout << "Total number of terms: " << totalTerms << endl;
		cout << "Total number of literals: " << totalLiterals << endl;

		return selectedPIs;
	}
	return {};
}

void writePLA(string &outputFileName, PlaData& data, vector<Term> primeImplicants, vector<int> selectedPIs){
	// ========== print out the output file ==========
	cout << "\nWriting output to " << outputFileName << "..." << endl;

	ofstream outFile(outputFileName);
	if (!outFile.is_open())
	{
		cout << "Error: Cannot create output file!" << endl;
	}

	// write the PLA format
	outFile << ".i " << data.inputNum << endl;
	outFile << ".o 1" << endl;
	outFile << ".ilb";
	for (const string &var : data.varNames)
	{
		outFile << " " << var;
	}
	outFile << endl;
	outFile << ".ob f" << endl;
	outFile << ".p " << selectedPIs.size() << endl;

	// write the selected prime implicants
	for (int pi : selectedPIs)
	{
		outFile << primeImplicants[pi].pattern << " 1" << endl;
	}

	outFile << ".e" << endl;
	outFile.close();

	cout << "Output file created successfully!" << endl;
}

int main(int argc, char* argv[]){
    // check the input command
	if(argc != 3){
        cout << "Usage: " << argv[0] << " <input.pla> <output.pla>" << endl;
        return 1;
    }
	
    string inputFileName = argv[1];
    string outputFileName = argv[2];

    // read PLA file
    PlaData data = readPlaFile(inputFileName);
    if(data.varNames.empty()){
        cout << "Failed to read input file!" << endl;
        return 1;
    }
    
    // build truth table
    map<string, int> truthTable = buildTruthTable(data.inputNum, data);
    changeTruthTable(truthTable, data);
    
    // do the groups by the one count
    map<int, vector<Term>> groups = groupByOne(truthTable);
	int round = 1;
	while(true){
    	map<int, vector<Term>> newGroups = combineGroups(groups);
    	if(newGroups.empty()) break;
    	groups = newGroups;
		round++;
		if(round > 10) break;	//make sure i will stop
	}

	//collect the prime implicants
	vector<Term> makePrimeImplicant;
	for(auto& g : groups){
		for(auto& term : g.second){
			makePrimeImplicant.push_back(term);
		}
	}
	printOutPI(makePrimeImplicant);

	buildPIChart(truthTable, data, makePrimeImplicant);

	vector<int> selectedPis = buildPIChart(truthTable, data, makePrimeImplicant);

	writePLA(outputFileName, data, makePrimeImplicant, selectedPis);

	return 0;
}