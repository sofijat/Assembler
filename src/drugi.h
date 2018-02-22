#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include "klase.h"

using namespace std;

typedef struct RelSymbol {
	uint32_t addr; //adresa koju treba prepraviti, pocinje sa 0x
	char type; //A-absolute, R-relative
	uint32_t sym_num; //redni broj
	RelSymbol(uint32_t address, char t, uint32_t sym) : addr(address), type(t), sym_num(sym) {}
};

typedef struct RelTable {
	std::vector<RelSymbol*> table;
	int br_sekcije; //redni broj sekcije za koju se pravi RElTAble
	std::vector<string> section_content; //pravi se posle drugog prolaza, ima velicinu sec_size
	RelTable(int num) {
		br_sekcije = num;
	}
};



void add_rel_symbol(RelTable& tab, RelSymbol* rs);
RelTable* get_reltab_for_section(int sec_num, vector<RelTable*>& reltables);
void add_rel_table(int num, vector<RelTable*>& reltables);
void add_content_to_table(RelTable& tab, string cont);
void free_rel_tables(vector<RelTable*>& rt);
int getAritmOp(int opindex, vector<string> args, vector<Symbol*>& symtab, vector<RelTable*>& reltables, vector<string>& greske);
void orgDirective(vector<string> args, vector<Symbol*>& symtab, vector<string>& greske);
void passTwo(vector<Symbol*>& symtab, vector<vector<string>>& asem, vector<RelTable*>& reltables, vector<string>& greske);
void dataPassTwo(vector<string> str, vector<Symbol*>& symtab, vector<RelTable*>& reltables, vector<string>& greske);
string relocationNeeded(vector<string> str, vector<Symbol*>& symtab, vector<string>& greske);
string to_ones_and_zeros(vector<string> str, vector<Symbol*>& symtab);
string symbol_to_relocate(string str, vector<Symbol*>& symtab, vector<string>& greske);
string convert_to_hex(int num);
void add_hex_instruction(uint32_t instr, RelTable* reltab);
void add_hex_instruction_first(uint32_t instr, RelTable* reltab);