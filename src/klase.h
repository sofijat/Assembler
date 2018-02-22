#pragma once
#include <vector>
#include <string>
#include <stack>
#include <cstdint>
#include <cstring>
#include <fstream>

using namespace std;


typedef struct Symbol { //moze biti SEG(sekcija) ili SYM(simbol)
	bool isSEG;
	uint32_t num; //redni broj
	string name; //naziv simbola
	int32_t sec_num; // SYM: 0=externa, -1=absolute; a ko SEG je isto kao num
	uint32_t addr; //starts with 0x
	char flag; //G-global, L-local, kod SYM
	uint32_t sec_size; //velicina sekcije, nema kod SYM, samo kod SEG
	string flags; //W-writeable, R-readable
	Symbol(string n, uint32_t address) : isSEG(false), num(0), name(n), sec_num(0), addr(address), flag('L'), sec_size(0), flags("") {} //za SEG
	Symbol(string n, uint32_t address, int32_t sec) : isSEG(false), num(0), name(n), sec_num(-1), addr(address), flag('C'), sec_size(0), flags("") {} //za SYM
};

class Instruction {
public:
	string name;
	int numOfArgs;
	uint32_t opcode;

	Instruction(string namee, uint32_t oper, int num) {
		name = namee; numOfArgs = num; opcode = oper;
	}
};

class State {
public:

	string cur_section;
	int cur_sec_num;
	bool error;
	int index;

	State() {
		cur_sec_num = 0;
		cur_section = "";
		error = false;
		index = 0;
	}
};

static State state;
static int location_counter = 0;

static Instruction* instructions[25] = {
	new Instruction("int", 0x00, 1),
	new Instruction("jmp", 0x02, 1),
	new Instruction("call", 0x03, 1),
	new Instruction("ret", 0x01, 0),
	new Instruction("jz", 0x04, 2),
	new Instruction("jnz", 0x05, 2),
	new Instruction("jgz", 0x06, 2),
	new Instruction("jgez", 0x07, 2),
	new Instruction("jlz", 0x08, 2),
	new Instruction("jlez", 0x09, 2),
	new Instruction("load", 0x10, 2),
	new Instruction("store", 0x11, 2),
	new Instruction("push", 0x20, 1),
	new Instruction("pop", 0x21, 1),
	new Instruction("add", 0x30, 3),
	new Instruction("sub", 0x31, 3),
	new Instruction("mul", 0x32, 3),
	new Instruction("div", 0x33, 3),
	new Instruction("mod", 0x34, 3),
	new Instruction("and", 0x35, 3),
	new Instruction("or", 0x36, 3),
	new Instruction("xor", 0x37, 3),
	new Instruction("not", 0x38, 2),
	new Instruction("asl", 0x39, 3),
	new Instruction("asr", 0x3A, 3),
};

int to_hex(int broj);
void endDirective(vector<Symbol*>& symtab);
void to_lower(string& str);
int add_to_table(std::vector<Symbol*>& symtable, string name, uint32_t adr, vector<string>& greske);
Symbol* get_symbol(std::vector<Symbol*>& symtable, const string name);
bool exists_by_name(string name, std::vector<Symbol*>& symtab);
bool is_operand(string str);
bool is_directive(const string str);
int translate_register(string str);
bool is_register(string str);
int32_t convert_to_num(string str);
int add_label(string name, vector<Symbol*>& symtab, vector<string>& greske);
void delete_char(string &s, char c);
void sectionDirective(string section, vector<Symbol*>& symtab, vector<string>& greske);
void update_location_counter(vector<string> str);
bool is_label(string str);
void defDirective(vector<string> buf, vector<Symbol*>& symtab, vector<string>& greske);
bool is_valid_label(const string str);
int addressing_mode(string str);
vector<string> split_keep_delimiter(string &s, string delimiter);
vector<string> to_postfix(vector<string> str);
bool is_operator(string c);
string calculate_expression(string expression, vector<Symbol*>& symtab, vector<string>& greske);
int get_value(string str, vector<Symbol*>& symtab);
void dataDirective(vector<string> buf, vector<Symbol*>& symtab, vector<string>& greske);
bool HasHigherPrecedence(string op1, string op2);
int IsRightAssociative(string op);
int priority(string c);
void passOne(vector<Symbol*>& symtab, vector<vector<string>> asem, vector<string>& greske);
int getInstruction(string name);
void globalDirective(vector<string> buf, vector<Symbol*>& symtab, vector<string>& greske);
void delete_spaces(vector<vector<string>>& vect);
char* appendCharToCharArray(char* array, char a);
int return_value(string str, vector<Symbol*>& symtab);
void split(const string &s, char* delim, vector<string> &v);
bool is_num(string str);
void write_error(string str);
