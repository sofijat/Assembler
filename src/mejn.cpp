#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stack>
#include <sstream>
#include <fstream>
#include <iostream>
#include "klase.h"
#include "drugi.h"


using namespace std;





vector<vector<string>> loadAssemblyFromFile(char* ulaz, vector<string>& greske) {
	
	ifstream infile(ulaz);
	string line;
	if (!infile) {
		greske.push_back("Nije moguce otvoriti ulazni fajl");
	}
	
	bool first = true;
	
	vector<vector<string>> vect;
	while (getline(infile, line)) {
		unsigned int found = line.find(';');
		if (found != string::npos) {
			line = line.substr(0, found);
		}
		vector<string> tokens;
		split(line, "\t\n\v\r,", tokens); //mozda i neki \v\r
	
		if (tokens.size() == 0) continue;
		vect.push_back(tokens);
	}
	return vect;

}




int main(int argc, char* argv[])
{
	
	//FILE* input = fopen(argv[1], "r");
	vector<string> greske;
	ofstream out("izlaz.txt");
	vector<vector<string>> vect = loadAssemblyFromFile(argv[1], greske);
	delete_spaces(vect);

	vector<Symbol*> symtab;
	passOne(symtab, vect, greske);


	vector<RelTable*> reltables;
	passTwo(symtab, vect, reltables, greske);

	out << "#TabelaSimbola\n";
	for (int i = 0; i < symtab.size(); i++) {

		if (symtab[i]->isSEG == true) {
			out << "SEG " << symtab[i]->num << " " << symtab[i]->name << " " << symtab[i]->sec_num << " " << "0x" << convert_to_hex(symtab[i]->addr) << " 0x" << convert_to_hex(symtab[i]->sec_size) << " " << symtab[i]->flags << "\n";
		}
		else {
			out << "SYM " << symtab[i]->num << " " << symtab[i]->name << " " << symtab[i]->sec_num << " " << "0x" << convert_to_hex(symtab[i]->addr) << " " << symtab[i]->flag << "\n";
		}

	}

	for (int i = 0; i < reltables.size(); i++) {
		int br_sek = reltables[i]->br_sekcije;
		string naziv = "";
		for (int j = 0; j < symtab.size(); j++) {
			if (symtab[j]->isSEG && symtab[j]->sec_num == br_sek) {
				naziv = symtab[j]->name;
				break;
			}
		}
		out << "#rel" << naziv << "\n";
		for (int j = 0; j < reltables[i]->table.size(); j++) {
			out << "0x" << convert_to_hex(reltables[i]->table[j]->addr) << " " << reltables[i]->table[j]->type << " " << reltables[i]->table[j]->sym_num << "\n";
		}

		out << naziv << "\n";
		int brojac = 0;
		for (int j = 0; j < reltables[i]->section_content.size(); j++) {
			brojac++;
			out << reltables[i]->section_content[j] << " ";
			if (brojac == 16) {
				brojac = 0;
				out << "\n";
			}
		}
		out << "\n";
	}
	out << "#end\n";

	if (greske.size() > 0) {
		ofstream errout("errors.txt");
		for (int n = 0; n < greske.size(); n++) {
			errout << greske[n] << "\n";
		}
	}

}