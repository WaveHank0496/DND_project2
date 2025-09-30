#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

//by proeject1
struct PlaData {
	int inputNum = 0;	//count how many variable
	int productNum = 0;	//used in for loop to check how many product lines have to read
	vector<string> varNames;	//store the different variable names
	map<string, int> productLine;	//the result and the product line command
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
		result = stoi(temp);
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

//do the groups distribute
int countOnes(const string& term){
	int count = 0;
	for(auto it : term){
		if(it == '1') count++;
	}
	return count;	
}

map<int, vector<string>> groupByOne(const map<string, int> &truthTable){
	map<int, vector<string>> groups;
	for(auto term : truthTable){
		if(term.second == 1){
			int ones = countOnes(term.first);
			groups[ones].push_back(term.first);
		}
	}
	return groups;
}

bool canCombine(const string& term1, const string& term2){
	if(term1.length() != term2.length()) return false;
	int same = 0;
	for(int i = 0; i < term1.length(); i++){
		if(term1[i] == term2[i]){
			same++;
		}
		else if(term1[i] == '-' || term2[i] == '-'){
			return false;
		}
	}

	if(same == term1.length() - 1){
		return true;
	}
	else{
		return false;
	}
}

string combine(const string&term1, const string& term2){
	string newTerm;
	for(int i = 0; i < term1.length(); i++){
		if(term1[i] != term2[i]){
			newTerm  = newTerm + '-';
		}
		else{
			newTerm = newTerm + term1[i];
		}
	}
	return newTerm;
}

//do the method

int main(int argc, char* argv[]){
    string t1 = "0101";
    string t2 = "0111";

    cout << countOnes(t1) << endl;  // 應該輸出：2
    cout << canCombine(t1, t2) << endl;  // 應該輸出：1 (true)
    cout << combine(t1, t2) << endl;  // 應該輸出：01-1
}

