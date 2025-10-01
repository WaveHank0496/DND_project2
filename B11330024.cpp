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
	int minterm;
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
			groups[ones].push_back({term.first, index});
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
		if(term1[i] != term2[i]){
			diff++;
			pos = i;
		}
		else if(term1[i] == '-' || term2[i] == '-'){
			return -1;
		}
		
		if(diff > 1){return -1;}
	}
	return pos;
}

string combine(const string&term1, const string& term2){
	string newTerm;
	newTerm = term1;
	if(canCombine(term1, term2)) newTerm[canCombine(term1, term2)] = '-';
	else newTerm = "";
	return newTerm;
}

map<string, vector<int>> combineGroups(map<int, vector<Term>>& groups){
	map<string, vector<int>> newGroups;
	for(int i = 0 ; i < groups.size() - 1; i++){
		for(auto subGroup : groups[i]){
			for(auto nextSubGroup : groups[i + 1]){
				if(canCombine(subGroup.pattern, nextSubGroup.pattern) != -1){
					string newPattern = combine(subGroup.pattern, nextSubGroup.pattern);
					if(newPattern != ""){
						newGroups[newPattern].push_back(subGroup.minterm);
						newGroups[newPattern].push_back(nextSubGroup.minterm);
					}
				}
			}
		}
	}
	return newGroups;
}

//do the method

int main(int argc, char* argv[]){
    string t1 = "0101";
    string t2 = "0111";

    cout << countOnes(t1) << endl;  // 應該輸出：2
    cout << canCombine(t1, t2) << endl;  // 應該輸出：1 (true)
    cout << combine(t1, t2) << endl;  // 應該輸出：01-1
}

