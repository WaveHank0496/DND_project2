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

map<int, vector<Term>> groupByOne(const map<string, int> &truthTable)
{
	map<int, vector<Term>> groups;

	for (auto term : truthTable)
	{
		if (term.second == 1 || term.second == -1)
		{
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

map<int, vector<Term>> combineGroups(map<int, vector<Term>> &groups)
{
	map<int, vector<Term>> newGroups;
	map<string, Term> patternToTerm;

	// ⭐ 修正：遍歷所有實際存在的 group pairs
	vector<int> groupKeys;
	for (auto &g : groups)
	{
		groupKeys.push_back(g.first);
	}

	// 對每對相鄰的 group 進行合併
	for (int i = 0; i < groupKeys.size() - 1; i++)
	{
		int currentGroup = groupKeys[i];
		int nextGroup = groupKeys[i + 1];

		// 只合併相鄰的組（差1個1）
		if (nextGroup - currentGroup != 1)
			continue;

		for (auto &subGroup : groups[currentGroup])
		{
			for (auto &nextSubGroup : groups[nextGroup])
			{
				int pos = canCombine(subGroup.pattern, nextSubGroup.pattern);
				if (pos != -1)
				{
					string newPattern = combine(subGroup.pattern, nextSubGroup.pattern);
					if (newPattern != "" && (patternToTerm.find(newPattern) == patternToTerm.end()))
					{
						Term newTerm;
						newTerm.pattern = newPattern;
						newTerm.minterm = subGroup.minterm;
						newTerm.minterm.insert(nextSubGroup.minterm.begin(), nextSubGroup.minterm.end());
						patternToTerm[newPattern] = newTerm;

						// 標記為已合併
						for (auto &term : groups[currentGroup])
						{
							if (term.pattern == subGroup.pattern)
							{
								term.combined = true;
							}
						}
						for (auto &term : groups[nextGroup])
						{
							if (term.pattern == nextSubGroup.pattern)
							{
								term.combined = true;
							}
						}
					}
				}
			}
		}
	}

	for (auto it : patternToTerm)
	{
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

	// Step 3: print the table 

	// Step 4: find EPI 

	vector<int> essentialPIs;	//EPI number
	set<int> coveredByEPIs;  //minterms covered by EPI

	for(int m : onSetMinterms){
		if(mintermToPIs[m].size() == 1){
			// if the minterm is covered by only one PI -> Essential!
			int epi = mintermToPIs[m][0];
			
			// check if already added
			if(find(essentialPIs.begin(), essentialPIs.end(), epi) == essentialPIs.end()){
				essentialPIs.push_back(epi);
				// check this EPI cover which minterm
				for(int covered : primeImplicants[epi].minterm){
					if(truthTable[intoBinary(covered, data.inputNum)] == 1){
						coveredByEPIs.insert(covered);
					}
				}
			}
		}
	}

	//find the uncovered 
	// Step 5: find uncovered minterms 
	vector<int> uncoveredMinterms;
	for(int m : onSetMinterms){
		if(coveredByEPIs.find(m) == coveredByEPIs.end()){
			uncoveredMinterms.push_back(m);
		}
	}

	if (uncoveredMinterms.empty())
	{
		// all be covered by EPI
		vector<int> selectedPIs = essentialPIs;

		// 計算 literals
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

		return selectedPIs; 
	}

	// Step 6: Petrick's Method  
	{
		// Petrick's Method
		// find the candidate PIs that can cover the uncovered minterms
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

		// 2. Enumerate all combinations (using bitmask)
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
		
		if(validSolutions.empty()){
			cout << "No valid solution found to cover all minterms!" << endl;
			vector<int> selectedPIs = essentialPIs;
			for (int pi : candidatePIs)
			{
				selectedPIs.push_back(pi);
			}
			return selectedPIs;
		}

		// 3. choose the best solution (minimize literals)
		int bestSolution = 0;
		int minLiterals = 999999;

		for (int i = 0; i < validSolutions.size(); i++)
		{
			int literals = 0;

			if (literals < minLiterals)
			{
				minLiterals = literals;
				bestSolution = i;
			}
		}

		// 4. combine the best solution with EPIs
		vector<int> selectedPIs = essentialPIs;
		for (int pi : validSolutions[bestSolution])
		{
			selectedPIs.push_back(pi);
		}

		sort(selectedPIs.begin(), selectedPIs.end());

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

int main(int argc, char *argv[])
{
	// check the input command
	if (argc != 3)
	{
		cout << "Usage: " << argv[0] << " <input.pla> <output.pla>" << endl;
		return 1;
	}

	string inputFileName = argv[1];
	string outputFileName = argv[2];

	// read PLA file
	PlaData data = readPlaFile(inputFileName);
	if (data.varNames.empty())
	{
		cout << "Failed to read input file!" << endl;
		return 1;
	}

	// build truth table
	map<string, int> truthTable = buildTruthTable(data.inputNum, data);
	changeTruthTable(truthTable, data);

	// ========== Quine-McCluskey Algorithm ==========
	cout << "\n=== Starting Quine-McCluskey Algorithm ===" << endl;

	// 初始分組
	map<int, vector<Term>> groups = groupByOne(truthTable);

	cout << "\n=== Round 0 (Original Minterms) ===" << endl;
	for (auto &g : groups)
	{
		cout << "Group " << g.first << ": ";
		for (auto &t : g.second)
		{
			cout << t.pattern << " ";
		}
		cout << endl;
	}

	// 保存所有輪次
	vector<map<int, vector<Term>>> allRounds;
	allRounds.push_back(groups);

	// 進行合併
	int round = 1;
	while (true)
	{
		cout << "\n--- Round " << round << " ---" << endl;
		map<int, vector<Term>> newGroups = combineGroups(groups);

		if (newGroups.empty())
		{
			cout << "No more combinations possible." << endl;
			break;
		}

		cout << "New terms: ";
		for (auto &g : newGroups)
		{
			for (auto &t : g.second)
			{
				cout << t.pattern << " ";
			}
		}
		cout << endl;

		allRounds.push_back(newGroups);
		groups = newGroups;
		round++;

		if (round > 10)
		{
			cout << "Warning: Stopped at round 10" << endl;
			break;
		}
	}

	cout << "\nTotal rounds: " << allRounds.size() << endl;

	// ========== 收集 Prime Implicants ==========
	cout << "\n=== Collecting Prime Implicants ===" << endl;

	vector<Term> allPrimeImplicants;
	set<string> seenPIPatterns;

	for (int r = 0; r < allRounds.size(); r++)
	{
		cout << "\nChecking Round " << r << ":" << endl;
		for (auto &g : allRounds[r])
		{
			for (auto &term : g.second)
			{
				// 檢查這個 term 的 minterms 是否被包含在更高層的某個 term 中
				bool isSubsetOfHigherTerm = false;

				for (int rr = r + 1; rr < allRounds.size(); rr++)
				{
					for (auto &gg : allRounds[rr])
					{
						for (auto &higherTerm : gg.second)
						{
							// 檢查 term.minterm 是否是 higherTerm.minterm 的子集
							bool allIncluded = true;
							for (int m : term.minterm)
							{
								if (higherTerm.minterm.find(m) == higherTerm.minterm.end())
								{
									allIncluded = false;
									break;
								}
							}

							// 如果所有 minterms 都被包含，且不完全相同（表示被合併了）
							if (allIncluded && term.minterm.size() < higherTerm.minterm.size())
							{
								isSubsetOfHigherTerm = true;
								break;
							}
						}
						if (isSubsetOfHigherTerm)
							break;
					}
					if (isSubsetOfHigherTerm)
						break;
				}

				if (!isSubsetOfHigherTerm && seenPIPatterns.find(term.pattern) == seenPIPatterns.end())
				{
					allPrimeImplicants.push_back(term);
					seenPIPatterns.insert(term.pattern);
				}
				else if (isSubsetOfHigherTerm)
				{
					cout << "  " << term.pattern << " was combined (not a PI)" << endl;
				}
			}
		}
	}
	printOutPI(allPrimeImplicants);

	// ========== PI Chart & Petrick's Method ==========
	vector<int> selectedPIs = buildPIChart(truthTable, data, allPrimeImplicants);

	// ========== Write Output ==========
	writePLA(outputFileName, data, allPrimeImplicants, selectedPIs);

	return 0;
}