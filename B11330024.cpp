#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
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
	vector<int> minterm;	//store the minterm index
	bool combined = false;	//check whether this term has been combined
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
			newTerm.minterm.push_back(index);	
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
			if(diff > 1){return -1;}
		}
	}
	return (diff == 1 ) ? pos : -1;
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
	map<string, Term> uniqueTerms;  // pattern -> Term
	
	for(int i = 0 ; i < groups.size() - 1; i++){
		if(groups.find(i) == groups.end() || groups.find(i+1) == groups.end()){
            continue;
        }
		for(auto subGroup : groups[i]){
			for(auto nextSubGroup : groups[i + 1]){
				if(canCombine(subGroup.pattern, nextSubGroup.pattern) != -1){
					string newPattern = combine(subGroup.pattern, nextSubGroup.pattern);
					if(newPattern != ""){
						// make new term
						Term newTerm;
						newTerm.pattern = newPattern;
						newTerm.combined = false;

						// combine two terms' minterms
						newTerm.minterm = subGroup.minterm;  // 複製第一個
						for(int m : nextSubGroup.minterm) {
							newTerm.minterm.push_back(m);
						}

						// sort and remove duplicates
						sort(newTerm.minterm.begin(), newTerm.minterm.end());
						newTerm.minterm.erase(
							unique(newTerm.minterm.begin(), newTerm.minterm.end()),
							newTerm.minterm.end()
						);

						if(uniqueTerms.find(newTerm.pattern) == uniqueTerms.end()){
                        // 第一次出現，加入
                        uniqueTerms[newTerm.pattern] = newTerm;
						}
					}
				}
			}
		}
	}
  	for(auto& pair : uniqueTerms){
        Term& term = pair.second;
        int ones = countOnes(term.pattern);
        newGroups[ones].push_back(term);
    }

	return newGroups;
}

vector<Term> quineMcCluskey(map<string, int>& truthTable) {
    // Step 1: 初始分組
    map<int, vector<Term>> groups = groupByOne(truthTable);
    
    vector<Term> allTerms;  // 儲存所有產生的 terms
    
    // 收集初始的 terms
    for(auto& group : groups) {
        for(auto& term : group.second) {
            allTerms.push_back(term);
        }
    }
    
    // Step 2: 持續合併
    bool hasNewCombination = true;
    
    while(hasNewCombination) {
        hasNewCombination = false;
        map<int, vector<Term>> newGroups;
        
		for(auto& [ones1, terms1] : groups) {
			int ones2 = ones1 + 1;
			if(groups.find(ones2) == groups.end()) continue;  // 沒有下一組
			for(auto& term1 : terms1) {
				for(auto& term2 : groups[ones2]) {
					int pos = canCombine(term1.pattern, term2.pattern);
					if(pos != -1) {
						// TODO: 建立 newTerm，合併 minterms，標記 used
						Term newTerm;
						newTerm.pattern = combine(term1.pattern, term2.pattern);
						// TODO: 加入 newGroups 和 allTerms
						newGroups[countOnes(newTerm.pattern)].push_back(newTerm);
						allTerms.push_back(newTerm);
						hasNewCombination = true;
					}
				}
			}
		}
        
        groups = newGroups;  // 準備下一輪
    }
    
    // Step 3: 找出 Prime Implicants
    vector<Term> primeImplicants;
    for(auto& term : allTerms) {
        if(!term.combined) {
            primeImplicants.push_back(term);
        }
    }
    
    return primeImplicants;
}

//do the method

vector<Term> findPrimeImplicants(map<string, int>& truthTable){
    // 1. 初始分組
    map<int, vector<Term>> groups = groupByOne(truthTable);
    
    // 2. 持續合併
    while(true){
        map<int, vector<Term>> newGroups = combineGroups(groups);
        
        // 如果沒有新的合併，結束
        if(newGroups.empty()){
            break;
        }
        
        groups = newGroups;  // 用新的 groups 繼續下一輪
    }
    
    // 3. 收集最後一輪的所有 terms 作為 Prime Implicants
    vector<Term> primeImplicants;
    for(auto& group : groups){
        for(auto& term : group.second){
            primeImplicants.push_back(term);
        }
    }
    
    return primeImplicants;
}

void outputPlaFile(const string& fileName, const PlaData& data, 
                   const vector<int>& selectedPIIndices, 
                   const vector<Term>& primeImplicants) {
    ofstream file(fileName);
    
    if(!file.is_open()){
        cout << "Error: Cannot create output file " << fileName << endl;
        return;
    }
    
    // 寫入檔頭
    file << ".i " << data.inputNum << endl;
    file << ".o 1" << endl;
    file << ".ilb";
    for(const string& var : data.varNames){
        file << " " << var;
    }
    file << endl;
    file << ".ob f" << endl;
    file << ".p " << selectedPIIndices.size() << endl;
    
    // 寫入每個選中的 PI
    for(int pi : selectedPIIndices){
        file << primeImplicants[pi].pattern << " 1" << endl;
    }
    
    file << ".e" << endl;
    file.close();
    
    cout << "\nOutput written to " << fileName << endl;
}

int main(int argc, char* argv[]){
    // 檢查命令列參數
    if(argc != 3){
        cout << "Usage: " << argv[0] << " <input.pla> <output.pla>" << endl;
        return 1;
    }
    
    string inputFileName = argv[1];
    string outputFileName = argv[2];

    // 讀取 PLA 檔案
    PlaData data = readPlaFile(inputFileName);
    if(data.varNames.empty()){
        cout << "Failed to read input file!" << endl;
        return 1;
    }
	
	map<string, int> truthTable = buildTruthTable(data.inputNum, data);
    changeTruthTable(truthTable, data);
    
    // 初始分組
    map<int, vector<Term>> groups = groupByOne(truthTable);
    
    cout << "=== original groups ===" << endl;
    for(auto& g : groups){
        cout << "Group " << g.first << ":" << endl;
        for(auto& term : g.second){
            cout << "  " << term.pattern << " -> {";
            for(int m : term.minterm) cout << m << " ";
            cout << "}" << endl;
        }
    }
    
    // 持續合併
    int round = 1;
    while(true){
        map<int, vector<Term>> newGroups = combineGroups(groups);
        
        if(newGroups.empty()){
            cout << "\nnumber " << round << " round can't merge anymore." << endl;
            break;
        }

        cout << "\n=== number " << round << " round after merge ===" << endl;
        for(auto& g : newGroups){
            cout << "Group " << g.first << " (there are " << g.second.size() << " terms):" << endl;
            for(auto& term : g.second){
                cout << "  " << term.pattern << " -> {";
                for(int m : term.minterm) cout << m << " ";
                cout << "}" << endl;
            }
        }
        
        groups = newGroups;  // 準備下一輪
        round++;
        
        // 安全機制：避免無限循環
        if(round > 10){
            cout << "Over 10 rounds, forced to end!" << endl;
            break;
        }
    }
    
	// 在 main 的最後加上
	cout << "\n=== Prime Implicant Chart ===" << endl;

	// 收集所有需要覆蓋的 on-set minterms
	vector<int> onSetMinterms;
	for(int i = 0; i < truthTable.size(); i++){
		string binary = intoBinary(i, data.inputNum);
		if(truthTable[binary] == 1){  // 只要 output=1 的
			onSetMinterms.push_back(i);
		}
	}

	cout << "On-set minterms: {";
	for(int m : onSetMinterms) cout << m << " ";
	cout << "}\n\n";

	// 儲存最後一輪的 Prime Implicants
	vector<Term> primeImplicants;
	for(auto& g : groups){
		for(auto& term : g.second){
			primeImplicants.push_back(term);
		}
	}

	// 建立反向索引：每個 minterm 被哪些 PI 覆蓋
	map<int, vector<int>> mintermToPIs;
	for(int i = 0; i < primeImplicants.size(); i++){
		for(int m : primeImplicants[i].minterm){
			// 只記錄 on-set 的 minterm
			if(truthTable[intoBinary(m, data.inputNum)] == 1){
				mintermToPIs[m].push_back(i);
			}
		}
	}

	// 印出表格
	cout << "       ";
	for(int m : onSetMinterms) cout << " m" << m;
	cout << endl;

	for(int i = 0; i < primeImplicants.size(); i++){
		cout << "PI" << i << "    ";
		for(int m : onSetMinterms){
			bool covered = false;
			for(int pi : mintermToPIs[m]){
				if(pi == i){
					covered = true;
					break;
				}
			}
			cout << (covered ? "  X" : "   ");
		}
		cout << "    " << primeImplicants[i].pattern << endl;
	}
		// 找出 Essential Prime Implicants
	vector<int> essentialPIs;
	vector<int> coveredMinterms;  // 被 EPI 覆蓋的 minterms

	cout << "\n=== Finding Essential Prime Implicants ===" << endl;
	for(int m : onSetMinterms){
		if(mintermToPIs[m].size() == 1){
			// 這個 minterm 只被一個 PI 覆蓋，那個 PI 就是 Essential
			int epi = mintermToPIs[m][0];
			
			// 檢查是否已經加入過
			bool alreadyAdded = false;
			for(int existingEPI : essentialPIs){
				if(existingEPI == epi){
					alreadyAdded = true;
					break;
				}
			}
			
			if(!alreadyAdded){
				essentialPIs.push_back(epi);
				cout << "m" << m << " only covered by PI" << epi 
					<< " (" << primeImplicants[epi].pattern << ") -> Essential!" << endl;
				
				// 記錄這個 EPI 覆蓋的所有 minterms
				for(int covered : primeImplicants[epi].minterm){
					if(truthTable[intoBinary(covered, data.inputNum)] == 1){
						coveredMinterms.push_back(covered);
					}
				}
			}
		}
	}

	// 去重
	sort(coveredMinterms.begin(), coveredMinterms.end());
	coveredMinterms.erase(
		unique(coveredMinterms.begin(), coveredMinterms.end()),
		coveredMinterms.end()
	);

	cout << "\nEssential PIs: {";
	for(int epi : essentialPIs) cout << "PI" << epi << " ";
	cout << "}" << endl;

	cout << "Covered by EPIs: {";
	for(int m : coveredMinterms) cout << m << " ";
	cout << "}" << endl;
		// 計算還沒被覆蓋的 minterms
	cout << "\nCovered by EPIs: {";
	for(int m : coveredMinterms) cout << m << " ";
	cout << "}" << endl;

	vector<int> uncoveredMinterms;
	for(int m : onSetMinterms){
		bool covered = false;
		for(int c : coveredMinterms){
			if(m == c){
				covered = true;
				break;
			}
		}
		if(!covered){
			uncoveredMinterms.push_back(m);
		}
	}

	cout << "Still need to cover: {";
	for(int m : uncoveredMinterms) cout << m << " ";
	cout << "}" << endl;

	// 找出哪些 PI 可以覆蓋這些 minterm
	cout << "\n=== Petrick's Method ===" << endl;
	for(int m : uncoveredMinterms){
		cout << "m" << m << " can be covered by: {";
		for(int pi : mintermToPIs[m]){
			cout << "PI" << pi << " ";
		}
		cout << "}" << endl;
	}

	// 簡單選擇：貪心法（選覆蓋最多 uncovered minterm 的 PI）
	/*
	vector<int> selectedPIs = essentialPIs;  // 從 EPI 開始
	vector<int> stillUncovered = uncoveredMinterms;

	while(!stillUncovered.empty()){
		int bestPI = -1;
		int maxCover = 0;
		
		// 找出能覆蓋最多 uncovered minterm 的 PI
		for(int i = 0; i < primeImplicants.size(); i++){
			// 跳過已選的
			bool alreadySelected = false;
			for(int selected : selectedPIs){
				if(i == selected){
					alreadySelected = true;
					break;
				}
			}
			if(alreadySelected) continue;
			
			// 計算能覆蓋多少個
			int coverCount = 0;
			for(int m : stillUncovered){
				for(int covered : primeImplicants[i].minterm){
					if(m == covered && truthTable[intoBinary(m, data.inputNum)] == 1){
						coverCount++;
						break;
					}
				}
			}
			
			if(coverCount > maxCover){
				maxCover = coverCount;
				bestPI = i;
			}
		}
		
		if(bestPI == -1) break;  // 找不到了
		
		selectedPIs.push_back(bestPI);
		cout << "Select PI" << bestPI << " (" << primeImplicants[bestPI].pattern 
			<< "), covers " << maxCover << " uncovered minterms" << endl;
		
		// 更新 stillUncovered
		vector<int> newUncovered;
		for(int m : stillUncovered){
			bool covered = false;
			for(int covered_m : primeImplicants[bestPI].minterm){
				if(m == covered_m){
					covered = true;
					break;
				}
			}
			if(!covered){
				newUncovered.push_back(m);
			}
		}
		stillUncovered = newUncovered;
	}
	*/

	// 輸出最終結果
	cout << "\n=== Final Solution ===" << endl;
	cout << "Selected PIs: {";
	for(int pi : selectedPIs) cout << "PI" << pi << " ";
	cout << "}" << endl;

	int totalTerms = selectedPIs.size();
	int totalLiterals = 0;
	for(int pi : selectedPIs){
		totalLiterals += primeImplicants[pi].pattern.length() - 
						count(primeImplicants[pi].pattern.begin(), 
							primeImplicants[pi].pattern.end(), '-');
	}

	cout << "Total number of terms: " << totalTerms << endl;
	cout << "Total number of literals: " << totalLiterals << endl;
    
	outputPlaFile(outputFileName, data, selectedPIs, primeImplicants);
    cout << "Total number of terms: " << totalTerms << endl;
    cout << "Total number of literals: " << totalLiterals << endl;
    
    return 0;
}

