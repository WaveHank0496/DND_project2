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
		if(term.second == 1){
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

//make a sheet that store all the minterm and the prime implicant that can cover it
map<int, vector<bool>> makePrimeImplicantChart(const vector<Term>& primeImplicants, const set<int>& minternVar){
	map<int, vector<bool>> chart; 
	int setSize = minternVar.size();
	int piIndex = 0;
	for(auto it : primeImplicants){
		vector<bool> row(setSize);
		for(auto minterm : it.minterm){
			row[minterm] = true;
		}
		chart[piIndex] = row;
		piIndex++;
	}
	return chart;
}

//find the epi
set<int> findEPI(map<int, vector<bool>> chart, const set<int>& mintermVar ){
	set<int> EPI;
	int setSize = mintermVar.size();
	for(int i = 0; i < setSize; i++){
		int count = 0;
		for(int j = 0; j < chart.size(); j++){
			if(chart[j][i] == true){count++;}
			if(count > 1) break;
			else{EPI.insert(i);}
		}
	}
	return EPI;
}

//find the minterm that didn't in epi
set<int> findEPIOther(set<int>& EPI, set<int>& mintermVar){
	set<int> stillUncovered;
	for(auto it : mintermVar){
		if(find(EPI.begin(),EPI.end(),it) == EPI.end()){
			stillUncovered.insert(it);
		}
	}
	return stillUncovered;
}


//test (the test is by ai)
void testWithRealData() {
    cout << "\n=== Testing with Real PLA Data ===" << endl;
    
    // 讀取真實的 PLA 檔案
    PlaData data = readPlaFile("input.pla");
    if(data.varNames.empty()) {
        cout << "Cannot read input.pla, skipping this test" << endl;
        return;
    }
    
    map<string, int> truthTable = buildTruthTable(data.inputNum, data);
    changeTruthTable(truthTable, data);
    
    // 初始分組
    map<int, vector<Term>> groups = groupByOne(truthTable);
    
    cout << "Round 0 (initial): " << endl;
    int totalTerms = 0;
    for(auto& g : groups) {
        cout << "  Group " << g.first << ": " << g.second.size() << " terms" << endl;
        totalTerms += g.second.size();
    }
    cout << "  Total: " << totalTerms << " terms" << endl;
    
    // 第一輪合併
    auto round1 = combineGroups(groups);
    cout << "\nRound 1: " << endl;
    totalTerms = 0;
    for(auto& g : round1) {
        cout << "  Group " << g.first << ": " << g.second.size() << " terms" << endl;
        totalTerms += g.second.size();
    }
    cout << "  Total: " << totalTerms << " terms" << endl;
    
    // 第二輪合併
    auto round2 = combineGroups(round1);
    cout << "\nRound 2: " << endl;
    if(round2.empty()) {
        cout << "  (no more combinations)" << endl;
    } else {
        totalTerms = 0;
        for(auto& g : round2) {
            cout << "  Group " << g.first << ": " << g.second.size() << " terms" << endl;
            totalTerms += g.second.size();
        }
        cout << "  Total: " << totalTerms << " terms" << endl;
    }
}
void testCombineGroups() {
    cout << "\n=== Testing combineGroups ===" << endl;
    
    // 建立測試資料
    map<int, vector<Term>> testGroups;
    
    // Group 0: 只有一個 term "0000" (m0)
    Term t0;
    t0.pattern = "0000";
    t0.minterm.insert(0);
    testGroups[0].push_back(t0);
    
    // Group 1: 兩個 terms
    Term t1;
    t1.pattern = "0010";
    t1.minterm.insert(2);
    testGroups[1].push_back(t1);
    
    Term t2;
    t2.pattern = "1000";
    t2.minterm.insert(8);
    testGroups[1].push_back(t2);
    
    // 印出輸入
    cout << "Input groups:" << endl;
    for(auto& g : testGroups) {
        cout << "Group " << g.first << ":" << endl;
        for(auto& term : g.second) {
            cout << "  " << term.pattern << " -> {";
            for(int m : term.minterm) cout << m << " ";
            cout << "}" << endl;
        }
    }
    
    // 執行合併
    map<int, vector<Term>> result = combineGroups(testGroups);
    
    // 印出結果
    cout << "\nOutput groups:" << endl;
    if(result.empty()) {
        cout << "  (empty - no combinations possible)" << endl;
    } else {
        for(auto& g : result) {
            cout << "Group " << g.first << ":" << endl;
            for(auto& term : g.second) {
                cout << "  " << term.pattern << " -> {";
                for(int m : term.minterm) cout << m << " ";
                cout << "}" << endl;
            }
        }
    }
    
    // 預期結果
    cout << "\nExpected:" << endl;
    cout << "Group 0: 00-0 -> {0 2}" << endl;
    cout << "Group 0: -000 -> {0 8}" << endl;
}
void testDuplicateRemoval() {
    cout << "\n=== Testing Duplicate Removal ===" << endl;
    
    map<int, vector<Term>> testGroups;
    
    // 建立會產生重複的情況
    // Group 0 (0個1): 00-0, -000
    // Group 1 (1個1): 10-0, -010
    // 00-0 + 10-0 -> -0-0
    // -000 + -010 -> -0-0 (重複！)
    
    Term t1;
    t1.pattern = "00-0";  // 1個1
    t1.minterm.insert(0);
    t1.minterm.insert(2);
    testGroups[1].push_back(t1);
    
    Term t2;
    t2.pattern = "-000";  // 0個1
    t2.minterm.insert(0);
    t2.minterm.insert(8);
    testGroups[0].push_back(t2);  // ← 改成 Group 0
    
    Term t3;
    t3.pattern = "10-0";  // 1個1
    t3.minterm.insert(8);
    t3.minterm.insert(10);
    testGroups[1].push_back(t3);  // ← 改成 Group 1
    
    Term t4;
    t4.pattern = "-010";  // 1個1
    t4.minterm.insert(2);
    t4.minterm.insert(10);
    testGroups[1].push_back(t4);  // ← 改成 Group 1
    
    cout << "Input:" << endl;
    for(auto& g : testGroups) {
        cout << "Group " << g.first << ":" << endl;
        for(auto& term : g.second) {
            cout << "  " << term.pattern << " (" << countOnes(term.pattern) << " ones) -> {";
            for(int m : term.minterm) cout << m << " ";
            cout << "}" << endl;
        }
    }
    
    auto result = combineGroups(testGroups);
    
    cout << "\nOutput (should have only ONE -0-0):" << endl;
    int count = 0;
    for(auto& g : result) {
        cout << "Group " << g.first << ":" << endl;
        for(auto& term : g.second) {
            cout << "  " << term.pattern << " -> {";
            for(int m : term.minterm) cout << m << " ";
            cout << "}" << endl;
            if(term.pattern == "-0-0") count++;
        }
    }
    
    cout << "\nResult: ";
    if(count == 1) {
        cout << "PASS - Only one -0-0 found" << endl;  // ← 移除特殊符號
    } else {
        cout << "FAIL - Found " << count << " instances of -0-0 (expected 1)" << endl;
    }
}


int main(int argc, char* argv[]){
	if(argc == 1) {
        cout << "Running tests..." << endl;
        testCombineGroups();
        testDuplicateRemoval();
        testWithRealData();
        return 0;
    }

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
	map<int, vector<Term>> newGroups = combineGroups(groups);


	return 0;
}
