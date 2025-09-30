#include <iostream>
#include <map>
#include <vector>
#include <string>

using namespace std;

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

