#include "klase.h"

int add_to_table(std::vector<Symbol*>& symtable, string name, uint32_t adr, vector<string>& greske) {
	string n;
	n = name;
	if (n == "") {
		greske.push_back("Ne mozete dodati simbol bez imena");
		return -1;
	} 
	Symbol* sym = new Symbol(n, adr);
	sym->name = name;
	sym->num = ++state.index;

	symtable.push_back(sym);
	return 0; //sve ok
}



bool is_valid_label(const string str) {
	if (str == "" || str == " ") return false;
	bool ret = false;;
	if (str[0] >= 'A' && str[0] <= 'Z' || str[0] >= 'a' && str[0] <= 'z' || str[0] == '_') ret = true;
	return ret;
}


Symbol* get_symbol(std::vector<Symbol*>& symtable, const string name) {
	for (int i = 0; i < symtable.size(); i++) {
		if (name == symtable[i]->name) {
			return symtable[i];
		}
	}
	return NULL;
}

bool exists_by_name(string name, std::vector<Symbol*>& symtab) {
	int ret = false;
	for (int i = 0; i < symtab.size(); i++) {
		if (symtab[i]->name == name) {
			ret = true;
			break;
		}
	}
	return ret;
}





bool is_operand(string str) {
	if (str[0] >= 'a' && str[0] <= 'z') {
		return true;
	}
	else if (str[0] >= 'A' && str[0] <= 'Z') return true;
	else if (str[0] == '_' || str[0]=='#' || str[0]=='$' || str[0]=='[' || str[0]==']') return true;
	else if (str[0] - '0' >= 0 && str[0] - '0' <= 9) return true;
	else return false;
}
bool is_directive(const string str) {
	if (str == "") return false;
	bool ret = false;
	if (str == ".global" ||
		str == ".text" ||
		str == ".data" ||
		str == ".bss" ||
		str == ".rodata" ||
		str == "org" ||
		str == ".end" ||
		str == "db" ||
		str == "dw" ||
		str == "dd" ||
		str == "def"
		) ret = true;
	return ret;
}

int translate_register(string str) {
	int ret = -1;
	
	if (str.size() == 2 && (str[0] == 'r' || str[0] == 'R') && str[1]-'0' >= 0 && str[1]-'0' <= 9)
		ret = str[1] - '0';
	else if (str.size() == 3 && (str[0] == 'r' || str[0] == 'R') && str[1]-'0' == 1 && str[2] -'0'>= 0 && str[2]-'0' <= 5) {
		int a = str[1] - '0';
		a *= 10;
		int b = str[2] - '0';
		ret = a + b;
	}
	else if (str == "pc" || str == "PC") ret = 17;
	else if (str == "sp" || str == "SP") ret = 16;
	return ret;
}

bool is_register(string str) {
	bool ret = false;
	if (translate_register(str) != -1) ret = true;
	return ret;
}
int32_t convert_to_num(string str) {
	int32_t res = 0;
	const char* stri = str.c_str();
	if (str[0] == '0') {
		if (str[1] == 'b') res = strtol(stri, NULL, 2);
		else if (str[1] == 'x' || str[1] == 'X') res = (int32_t)strtol(stri, NULL, 16);
	}
	else {
		res = strtol(stri, NULL, 10);
	}
	return res;
}

int add_label(string name, vector<Symbol*>& symtab, vector<string>& greske) {
	if (is_label(name)) {
		delete_char(name, ':');
	}
	
	if (is_valid_label(name)) {
		Symbol* sym = get_symbol(symtab, name);
		int section = state.cur_sec_num;
		if (sym != NULL) {
			if (sym->sec_num == 0) {
				sym->sec_num = section;
				sym->addr = location_counter;
			}
			else {
				greske.push_back("Labela " + name + " vec postoji");
			}
		}
		else {
			if (add_to_table(symtab, name, location_counter, greske) == 0) {
				Symbol* ss = get_symbol(symtab, name);
				ss->addr = location_counter;
				ss->sec_num = section;
			}

		}
		return 0;
	}
	else {
		greske.push_back("Labela " + name + " nije validna");
		return 1;
	}
}

void delete_char(string &s, char c) {
	string novi = "";
	for (int i = 0; i < s.length(); i++) {
		if (s[i] != c) {
			novi += s[i];
		}
	}
	s = novi;
}
void sectionDirective(string section, vector<Symbol*>& symtab, vector<string>& greske) {
	Symbol* old=NULL;
	if (add_to_table(symtab, section, 0, greske) == 0) {

		//treba da se odvoji i drugi deo .text.nesto
		if (state.cur_section != section) {
			Symbol* sym = get_symbol(symtab, section);

			string old_section = state.cur_section;
			if (old_section != "") {
				old = get_symbol(symtab, old_section);
				if (old!=NULL) old->sec_size = location_counter;
			}

			state.cur_section = section;
			state.cur_sec_num=sym->num;
			sym->sec_num = sym->num;
			sym->isSEG = true;
			location_counter = 0;
			sym->addr = location_counter;

			if (section.find(".text") != string::npos) {
				sym->flags = "RX";
			}
			if (section.find(".data") != string::npos) {
				sym->flags = "RW";
			}
			if (section.find(".bss") != string::npos) {
				sym->flags = "RW";
			}
			if (section.find(".rodata") != string::npos) {
				sym->flags = "R";
			}
		}

		

	}
}

char *my_strdup(const char *str) {
	size_t len = strlen(str);
	char *x = (char *)malloc(len + 1); /* 1 for the null terminator */
	if (!x) return NULL; /* malloc could not allocate memory */
	memcpy(x, str, len + 1); /* copy the string into the new buffer */
	return x;
}

vector<string> split_keep_delimiter(string &s, string delimiter) {
	string token = "";
	string del = "";
	string ch = "";
	vector<string> novi;
	bool found = false;

	for (int i = 0; i < s.length(); i++) {

		for (int j = 0; j < delimiter.length(); j++) {
			if (s[i] == delimiter[j]) {
				found = true;
				del = delimiter[j];

				break;
			}
			else ch = s[i];
		}
		if (found == true) {
			found = false;
			if (token != "") {
				novi.push_back(token);
				token = "";
			}
			novi.push_back(del);
		}
		else token += ch;
	}
	if (token != "") novi.push_back(token);
	return novi;
}

int addressing_mode(string str) {
	int ret = -1;
	if (str[0] == '#') ret = 4;
	else if (str[0] == '[') {
		int len = str.length();
		if (len > 5) ret = 7;
		else ret = 2;
	}
	else if (str[0] == '$') ret = 1;
	else if (str.find('+') != string::npos || str.find('-') != string::npos || str.find('*') != string::npos || str.find('/') != string::npos)
		ret = 6;
	else if (translate_register(str) == -1) ret = 6;
	else ret = 0;
	return ret;
}

void update_location_counter(vector<string> str) {//prosledi se ceo red
	int off = 4;
	for (int i = 1; i < str.size(); i++) {
		int addr = addressing_mode(str[i]);

		if (addr == 6 || addr == 4 || addr == 7 || addr == 1) {
			off = 8;
			break;
		}
	}
	location_counter += off;
}
bool is_label(string str) {
	int len = str.length();
	bool ret = false;
	if (str[len - 1] == ':') ret = true;
	return ret;
}

bool is_number(string s) {
	int32_t br = atoi(s.c_str());
	if (br != 0) return true;
	else return false;
}

string calculate_expression(string expression, vector<Symbol*>& symtab, vector<string>& greske) {
	//treba da mu se prosledi jedan izraz, kod mene je to string
	string in = "";
	string ex = expression;
	stack<string> s;
	string deli = "+-*/()";
	string lab = "";
	int32_t res = 0;

	vector<string> ulaz = split_keep_delimiter(ex, deli); //ulaz sadrzi i $, #, 
	delete_char(ulaz[0], '[');
	delete_char(ulaz[ulaz.size()-1], ']');
	vector<string> postfix = to_postfix(ulaz); //postfix ce isto imati $,#,[,]

	string op1;
	string op2;
	string op3;
	string op4;
	int i = 0;
	if (postfix.size()>0) {
		op1 = postfix[i];
		delete_char(op1, '$');
		delete_char(op1, '#');
		delete_char(op1, '[');
		s.push(op1);
		i++;
	}
	if (postfix.size() > 1) {
		op2 = postfix[i];
		delete_char(op2, ']');
		s.push(op2);
		i++;
	}
	else { //ako je postojao samo 1 operand koji je stavljen na stek
		if (!s.empty()) {
			op1 = s.top();
			s.pop();
			delete_char(op1, '$');
			delete_char(op1, '#');
			delete_char(op1, '[');
			delete_char(op1, ']');
			//if (is_register(op1)) return op1;
			if (exists_by_name(op1, symtab)) {
				Symbol* s = get_symbol(symtab, op1);
				return to_string(s->addr);
			}
			else {
				res = get_value(op1, symtab);
				return to_string(res);
			}
		}
	}
	while (i < postfix.size()) {
		op2 = postfix[i];
		delete_char(op2, ']');
		if (is_operator(op2)) {
			op4 = s.top();
			s.pop();
			op3 = s.top();
			s.pop();
			delete_char(op4, '#');
			delete_char(op4, '$');
			delete_char(op4, '[');
			delete_char(op4, ']');
			delete_char(op3, '#');
			delete_char(op3, '$');
			delete_char(op3, '[');
			delete_char(op3, ']');

			/*
			if (exists_by_name(op3, symtab)) {
			Symbol* sym3 = get_symbol(symtab, op3);
			op3 = sym3->addr + "";
			}
			if (exists_by_name(op4, symtab)) {
			Symbol* sym4 = get_symbol(symtab, op4);
			op4 = sym4->addr + "";
			}*/

			//ako postoje u tabeli simbola ili ako ne postoje, a nisu registri
			if ((exists_by_name(op3, symtab) && exists_by_name(op4, symtab)) || (!(exists_by_name(op3, symtab)) &&
				!(exists_by_name(op4, symtab)) && !(is_register(op3)) && !(is_register(op4))) ||
				exists_by_name(op3, symtab) && is_number(op4) ||
				exists_by_name(op4, symtab) && is_number(op3)) {//A DA JEDNO POSTOJI A DRUGO NE
				if (op2 == "+") { res = get_value(op3, symtab) + get_value(op4, symtab); }
				else if (op2 == "-") { res = get_value(op3, symtab) - get_value(op4, symtab); }
				else if (op2 == "*") { res = get_value(op3, symtab) * get_value(op4, symtab); }
				else if (op2 == "/") {
					if (get_value(op4, symtab) != 0) res = get_value(op3, symtab) / get_value(op4, symtab);
					else res = 0;
				}
				s.push(to_string(res));
			}
			else {
				if (exists_by_name(op3, symtab) || is_register(op3)) {
					lab += op3;
					if (op2 == "-" && !op4.empty()) {
						op4 = "-"+op4;
					}
					else res = get_value(op4, symtab);
					s.push(op4);
					i++;
					continue;
				}
				if (exists_by_name(op4, symtab) || is_register(op4)) {
					lab += op4;
					if (op2 == "-" && !op3.empty()) {
						op3 = "-" + op3;
					}
					else res = convert_to_num(op3);
					s.push(op3);
					i++;
					continue;
				}
			}
		}
		else {
			s.push(op2);
		}
		i++;
	}
	string res1 = "";
	res1 = s.top();
	res1 = to_string(get_value(res1, symtab));
	s.pop();

	if (!s.empty()) {
		greske.push_back("Izraz je nekorektan");
		state.error = true;
	}
	return lab + (is_num(res1) && !lab.empty() ? "+" : "") + res1;
}

bool is_num(string str) {
	if (str == "") return false;
	if (isdigit(str[0])) return true;
	else return false;
}

void defDirective(vector<string> buf, vector<Symbol*>& symtab, vector<string>& greske) {//naziv DEF 0xA0
	if (buf.size() != 3) {
		greske.push_back("Broj argumenata u liniji nije odgovarajuci");
		return;
	}
	Symbol* sym;
	string rez;
	if (is_valid_label(buf[0])) {
		int br;
		rez = calculate_expression(buf[2], symtab, greske);
		if (is_num(rez)) br = stoi(rez);
		else br = 0;
		if (state.error == true) {
			state.error = false;
			return;
		}
			else {
				add_to_table(symtab, buf[0], br, greske);
				Symbol *s = get_symbol(symtab, buf[0]);
				s->sec_num=-1;
			}
	}
	else {
		greske.push_back("Labela " + buf[0] + " nije validna");
		return;
	}
}

int get_value(string str, vector<Symbol*>& symtab) {
	int val = 0;
	if (exists_by_name(str, symtab)) {
		Symbol *sym = get_symbol(symtab, str);
		val = sym->addr;
	}
	else {
		val = convert_to_num(str);
	}
	return val;
}

void dataDirective(vector<string> buf, vector<Symbol*>& symtab, vector<string>& greske) {//ako ima DUP, onda se mnozi sa num_rept

	int num_rept=0;
	string rez;
	if (buf.size() == 2) num_rept = 1;
	if (buf.size() > 2) {
		if (buf[0] == "DD" || buf[0] == "DW" || buf[0] == "DB") {
			 if (buf[2]=="DUP") {
				rez = calculate_expression(buf[1], symtab, greske);
				if (state.error == true) {
					state.error = false;
					return;
				}
				if (is_num(rez)) num_rept = convert_to_num(rez);
				else num_rept = 0;
			}

		}
	}

	if (buf[0] == "DB") location_counter += 1 * num_rept;
	if (buf[0] == "DW") location_counter += 2 * num_rept;
	if (buf[0] == "DD") location_counter += 4 * num_rept;

}

bool is_operator(string c) {
	bool ret = false;
	if (c == "/" || c == "*" || c == "+" || c == "-" || c == "$" ) ret = true;
	return ret;
}

vector<string> to_postfix(vector<string> str) {
	stack<string> s;
	vector<string> out;

	int i = 0;
	int j = 0;
	string next;

	for (int i = 0; i< str.size(); i++) {

		if (is_operator(str[i]))
		{
			while (!s.empty() && s.top() != "(" && HasHigherPrecedence(s.top(), str[i]))
			{
				out.push_back(s.top());
				s.pop();
			}
			s.push(str[i]);
		}

		else if (is_operand(str[i]))
		{
			out.push_back(str[i]);
		}

		else if (str[i] == "(")
		{
			s.push(str[i]);
		}

		else if (str[i] == ")")
		{
			while (!s.empty() && s.top() != "(") {
				out.push_back(s.top());
				s.pop();
			}
			s.pop();
		}
	}

	while (!s.empty()) {
		out.push_back(s.top());
		s.pop();
	}

	return out;
}

int IsRightAssociative(string op)
{
	if (op == "$") return true;
	return false;
}

void passOne(vector<Symbol*>& symtab, vector<vector<string>> asem, vector<string>& greske) {
	vector<string> novi;

	for (int i = 0; i < asem.size(); i++) {
		if (!novi.empty()) novi.clear();
		if (is_label(asem[i][0])) {
			if (is_valid_label(asem[i][0])) {
				//ako je labela, obradi je i ubaci u symtable
				add_label(asem[i][0], symtab, greske);

				//izbaci labelu iz vector<string>
				for (int j = 1; j < asem[i].size(); j++)
					novi.push_back(asem[i][j]);
			}
			else {
				greske.push_back("Labela nije ispravna");
				continue;
			}
		}
		else novi = asem[i];

		if (novi.size() == 0) continue;
		

		if (novi[0] == ".global") globalDirective(novi, symtab, greske);
		else if (novi[0].find(".text") != string::npos || novi[0].find(".data") != string::npos ||
			novi[0].find(".rodata") != string::npos || novi[0].find(".bss") != string::npos)
			sectionDirective(novi[0], symtab, greske);
		else if (novi[0] == "ORG") {
			continue;
			//orgDirective(novi, symtab); NE RADI NISTA JER JE 1. PROLAZ
		}
		else if (getInstruction(novi[0]) != -1) {//ako je instrukcija, valjda samo treba da uveca LocatinCnt
			update_location_counter(novi);
		}
		else if (novi[0] == "DB" || novi[0] == "DW" || novi[0] == "DD") dataDirective(novi, symtab, greske);
		else if (novi.size() >= 2) {
			if (novi[1] == "DEF" || novi[1] == "def") defDirective(novi, symtab, greske);
		}
		else if (novi[0] == ".end") {
			endDirective(symtab);
			break;
		}
	}


}

void to_lower(string& str) {
	for (int i = 0; i < str.size(); i++) {
		char r=tolower(str[i]);
		str[i] = r;
	}
}


void endDirective(vector<Symbol*>& symtab) {
	if (state.cur_section != ".end") {
		string old_section = state.cur_section;
		if (old_section != "") {
			Symbol* sym = get_symbol(symtab, old_section);
			sym->sec_size = location_counter;
		}
		location_counter = 0;
	}
}

int getInstruction(string name) {
	int indexOfInstruction = -1;
	to_lower(name);
	for (int i = 0; i < 25; i++) {
		if (name.find(instructions[i]->name) != string::npos) {
			indexOfInstruction = i;
			break;
		}
	}
	return indexOfInstruction;

}


int priority(string c) {
	int ret = -1;
	if (c == "+" || c == "-") ret = 1;
	else if (c == "*" || c == "/") ret = 2;
	return ret;
}


bool HasHigherPrecedence(string op1, string op2)
{

	int op1Weight = priority(op1);
	int op2Weight = priority(op2);


	if (op1Weight == op2Weight)
	{
		if (IsRightAssociative(op1)) return false;
		else return true;
	}
	return op1Weight > op2Weight ? true : false;
}


void globalDirective(vector<string> buf, vector<Symbol*>& symtab, vector<string>& greske) {//.global a b c
																   //prosledjuje se sve
	for (int i = 1; i < buf.size(); i++) {
		if (is_valid_label(buf[i])) {
			Symbol* sym = get_symbol(symtab, buf[i]);
			if (sym != NULL) sym->flag = 'G';
			else if (add_to_table(symtab, buf[i], location_counter, greske) == 0) {
				Symbol* sym = get_symbol(symtab, buf[i]);
				if (sym!=NULL)sym->flag = 'G';
			}
		}
		else {
			greske.push_back("Labela " + buf[i] + " nije validna");
		}
	}
}


void delete_spaces(vector<vector<string>>& vect) {
	for (int i = 0; i < vect.size(); i++)
		for (int j = 0; j < vect[i].size(); j++)
			delete_char(vect[i][j], ' ');
}


char* appendCharToCharArray(char* array, char a)
{
	size_t len = strlen(array);

	char* ret = new char[len + 2];

	strcpy(ret, array);
	ret[len] = a;
	ret[len + 1] = '\0';

	return ret;
}


int return_value(string str, vector<Symbol*>& symtab) {
	int val = 0;
	if (exists_by_name(str, symtab)) {
		Symbol* sym = get_symbol(symtab, str);
		val = sym->addr;
	}
	return val;
}


void split(const string &s, char* delim, vector<string> &v) {
	char * dup = my_strdup(s.c_str());
	bool first = true;
	char * token;
	char* delimfirst = delim;

	if (first) {
		char* jbt = appendCharToCharArray(delimfirst, ' ');
		token = strtok(dup, jbt);
		first = false;
	}
	if (token != NULL) v.push_back(string(token));
	else return;

	string tok(token);
	bool labela = false;
	if (is_label(tok)) labela = true;

	if (token == "INT" || token == "JMP" || token == "CALL" || labela) {
		char* jbt1 = appendCharToCharArray(delimfirst, ' ');
		token = strtok(NULL, jbt1);
		first = true;
		if (token != NULL) v.push_back(string(token));
		else return;
	}

	if ((strstr(token,"DD") || strstr(token, "DW") || strstr(token, "DB")) && !strstr(token, "ADD")) {
		if (s.find("DUP") != string::npos) {
			char * dupli = my_strdup(s.c_str());//DD b - a DUP s
			int pos1 = s.find(token);
			dupli += 3+pos1;
			
			int pos = 0;
			string novi;
			string d(dupli);
			
			if ((pos = d.find("DUP")) != string::npos) {
				novi = d.substr(0, pos);
				if (!novi.empty()) v.push_back(novi);
				d.erase(0, pos + 3);
				v.push_back("DUP");
				if (!d.empty()) v.push_back(d);			
			}
		}
		else {
			char * dupli =my_strdup(s.c_str());
			dupli += 3; 
			if (dupli) v.push_back(string(dupli));
		}
		return;
	}


	while (token != NULL) {
		char* jbt1 = appendCharToCharArray(delimfirst, ' ');
		token = strtok(NULL, delim);//def 5
		if (token != NULL) {
			if (strstr(token, "def") != NULL || strstr(token, "DEF") != NULL) {
				char* t = strtok(token, " ");
				if (t != NULL) v.push_back(string(t));
				token = t;
			}
			else if (token != NULL) v.push_back(token);
		}
		token = strtok(NULL, delim);
		if (token != NULL) v.push_back(string(token));
	}
	
}
