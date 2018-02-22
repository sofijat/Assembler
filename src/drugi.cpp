#include "drugi.h"

RelTable* get_reltab_for_section(int sec_num, vector<RelTable*>& reltables) {
	for (int i = 0; i < reltables.size(); i++) {
		if (reltables[i]->br_sekcije == sec_num) return reltables[i];
	}
	return NULL;
}

void add_rel_table(int num, vector<RelTable*>& reltables) {
	RelTable* rt = new RelTable(num);
	if (rt != NULL) reltables.push_back(rt);
}

void add_rel_symbol(RelTable& tab, RelSymbol* rs) {
	if (rs != NULL) {
		tab.table.push_back(rs);
	}
	else return;
}

void add_content_to_table(RelTable& tab, string cont) {
	if (!cont.size() == 0) {
		tab.section_content.push_back(cont);
	}
}

void free_rel_tables(vector<RelTable*>& rt) {
	if (rt.size()==0) return;
	for (int i = 0; i < rt.size(); i++) {
		for (int j = 0; j < rt[i]->table.size(); j++) {
			RelSymbol* rs = rt[i]->table[j];
			delete rs;
		}	
	}
}

int getAritmOp(int opindex, vector<string> args, vector<Symbol*>& symtab, vector<RelTable*>& reltables, vector<string>& greske) {
	Instruction* instr = instructions[opindex];
	int numArg = instr->numOfArgs;
	uint32_t instr1;
	uint32_t instr2;
	int ret = 0;
	vector<uint32_t> regs;
	uint32_t operation;
	int addr_mode;
	int type = 0;
	string to_rellocate;

	if (numArg +1!= args.size()) {
		greske.push_back("Broj argumenata u liniji nije odgovarajuci");
		ret = -1;
		return ret;
	}

	if ((opindex >= 12 && opindex <= 24) || opindex == 0 || opindex == 3) {//onda se radi ili push/pop ili neka aritmeticka + not, int, ret

		for (int i = 0; i < numArg; i++) {
			regs.push_back(translate_register(args[i + 1]));
		}
		for (int i = 0; i < numArg; i++)
			if (regs[i] == -1) {
				greske.push_back("Registar ne postoji");
				ret = -1;
				return ret;
			}
		if (regs.size() >= 1 && regs[0] != -1) regs[0]= regs[0] << 16;
		if (regs.size() >= 2 && regs[1] != -1) regs[1]=regs[1] << 11;
		if (regs.size() >= 3 && regs[2] != -1) regs[2]=regs[2] << 6;
		operation = (instr->opcode << 24) | (regs.size()>=1?regs[0]:0) | (regs.size()>=2 ? regs[1] : 0) | (regs.size()>=3 ? regs[2] : 0);
		add_hex_instruction(operation, get_reltab_for_section(state.cur_sec_num, reltables));
		location_counter += 4;
		return ret;
	}
	else if (opindex == 1 || opindex == 2 || opindex >= 4 && opindex <= 9) { //jmp i call + uslovni skokovi
																			 //Podrzava: 1, 2 ,6 ,7 

		int reg1 = -1;
		if (opindex >= 4 && opindex <= 9) {
			reg1 = translate_register(args[1]);
			addr_mode = addressing_mode(args[2]);
		}
		else addr_mode = addressing_mode(args[1]);
		location_counter += 4;
		// 1
		if (addr_mode == 1) {
			string ab;
			if (opindex >= 4 && opindex <= 9) {
				ab = calculate_expression(args[2], symtab, greske);
			}
			else ab = calculate_expression(args[1], symtab, greske);

			if (is_num(ab)) instr2 = stoi(ab);
			else instr2 = 0;
			

			if (state.error == true) {
				state.error = false;
				return -1;
			}
			location_counter += 4;
			to_rellocate = relocationNeeded(args, symtab, greske);
			if (to_rellocate != "") {
				Symbol* sym = get_symbol(symtab, to_rellocate);
				RelSymbol* rs=NULL;
				RelTable* rt = get_reltab_for_section(state.cur_sec_num, reltables);
				Symbol* section = get_symbol(symtab, state.cur_section);
				if (sym->flag == 'L') rs = new RelSymbol(location_counter+section->addr-4, 'R', sym->sec_num);
				if (sym->flag == 'G') rs = new RelSymbol(location_counter + section->addr - 4, 'R', sym->num);
				if (rs!=NULL && rt!=NULL) add_rel_symbol(*rt, rs);
			}

			instr1 = (instr->opcode << 24) | (7 << 21) | (translate_register("pc") << 16);
			add_hex_instruction(instr1, get_reltab_for_section(state.cur_sec_num, reltables));
			add_hex_instruction(instr2, get_reltab_for_section(state.cur_sec_num, reltables));
		}
		//2
		else if (addr_mode == 2) {
			if (opindex >= 4 && opindex <= 9) {
				delete_char(args[2], '[');
				delete_char(args[2], ']');
			}
			else {
				delete_char(args[1], '[');
				delete_char(args[1], ']');
			}
			int regi = -1;
			if (opindex >= 4 && opindex <= 9) {
				regi = translate_register(args[2]);
			}
			else regi = translate_register(args[1]);
			if (regi == -1) {
				greske.push_back("Registar ne postoji");
			}
			instr1 = (instr->opcode << 24) | (addr_mode << 21) | (reg1 << 16) | (regi<<11);
			add_hex_instruction(instr1, get_reltab_for_section(state.cur_sec_num, reltables));
		}
		//6 , 7
		else if (addr_mode == 6 || addr_mode == 7) {
			
			int regi = -1;
			string izlaz;
			if (opindex==1 || opindex==2) izlaz=calculate_expression(args[1], symtab, greske);
			else izlaz=calculate_expression(args[2], symtab, greske);
			
			if (addr_mode == 7) {
				vector<string> parsir;
				if (izlaz.find('+') != string::npos) {
					parsir = split_keep_delimiter(izlaz, "+");

				}
				else if (izlaz.find('-') != string::npos) {
					parsir = split_keep_delimiter(izlaz, "-");
				}
				regi = translate_register(parsir[0]);
				if (parsir.size() == 3 && parsir[1] == "+") instr2 = convert_to_num(parsir[2]);
				else if (parsir.size() == 3 && parsir[1] == "-") instr2 = convert_to_num(parsir[1] + parsir[2]);
				else instr2 = 0;
			}
			else instr2 = convert_to_num(izlaz);


			if (state.error == true) {
				state.error = false;
				return -1;
			}
			location_counter += 4;
			to_rellocate = relocationNeeded(args, symtab, greske);
			if (to_rellocate != "") {
				Symbol* sym = get_symbol(symtab, to_rellocate);
				RelSymbol* rs = NULL;
				RelTable* rt = get_reltab_for_section(state.cur_sec_num, reltables);
				Symbol* section = get_symbol(symtab, state.cur_section);
				if (sym->flag == 'L') rs = new RelSymbol(location_counter+section->addr-4, 'A', sym->sec_num);
				if (sym->flag == 'G') rs = new RelSymbol(location_counter+section->addr-4, 'A', sym->num);
				if (rs != NULL) add_rel_symbol(*rt, rs);
			}

			
			instr1 = (instr->opcode << 24) | (addr_mode << 21) | (((reg1 != -1) ? reg1 : 0) << 16) | (((regi != -1) ? regi : 0)<<11);
			add_hex_instruction(instr1, get_reltab_for_section(state.cur_sec_num, reltables));
			add_hex_instruction(instr2, get_reltab_for_section(state.cur_sec_num, reltables));
		}
		else return -1;
	}
	else if (opindex == 10 || opindex == 11) {//load/store
		type = 0;
		int reg1 = -1;
		if (args[0].find("ub") != string::npos || args[0].find("UB") != string::npos) type = 3;
		else if (args[0].find("sb") != string::npos || args[0].find("SB") != string::npos) type = 7;
		else if (args[0].find("uw") != string::npos || args[0].find("UW") != string::npos) type = 1;
		else if (args[0].find("sw") != string::npos || args[0].find("SW") != string::npos) type = 5;

		if (translate_register(args[1]) == -1) {
			greske.push_back("Registar ne postoji");
			return -1;
		}
		reg1 = translate_register(args[1]);
		location_counter += 4;
		addr_mode = addressing_mode(args[2]);
		if (addr_mode == 0) {
			int r = translate_register(args[2]);
			if (r == -1) {
				greske.push_back("Registar ne postoji");
				return -1;
			}
			instr1 = (instr->opcode << 24) | (addr_mode << 21) | (r << 16) | (reg1 << 11) | (type << 3);
			add_hex_instruction(instr1, get_reltab_for_section(state.cur_sec_num, reltables));
		}
		else if (addr_mode == 2) {
			delete_char(args[2], '[');
			delete_char(args[2], ']');
			int regi = translate_register(args[2]);
			if (regi == -1) {
				greske.push_back("Registar ne postoji");
				return -1;
			}

			instr1 = (instr->opcode << 24) | (addr_mode << 21) | (regi << 16) | (reg1 << 11) | (type << 3);
			add_hex_instruction(instr1, get_reltab_for_section(state.cur_sec_num, reltables));
		}
		else if (addr_mode == 1) {
			string ab = calculate_expression(args[2], symtab, greske);
			if (is_num(ab)) instr2 = stoi(ab);
			else instr2 = 0;

			if (state.error == true) {
				state.error = false;
				return -1;
			}
			location_counter += 4;
			to_rellocate = relocationNeeded(args, symtab, greske);
			if (to_rellocate != "") {
				Symbol* sym = get_symbol(symtab, to_rellocate);
				RelSymbol* rs = NULL;
				RelTable* rt = get_reltab_for_section(state.cur_sec_num, reltables);
				Symbol* section = get_symbol(symtab, state.cur_section);
				if (sym->flag == 'L') rs = new RelSymbol(location_counter+section->addr-4, 'R', sym->sec_num);
				if (sym->flag == 'G') rs = new RelSymbol(location_counter+section->addr-4, 'R', sym->num);
				if (rs != NULL) add_rel_symbol(*rt, rs);
			}

			
			instr1 = (instr->opcode << 24) | (7 << 21) | (translate_register("pc") << 16) | (reg1 << 11) | (type << 3);
			add_hex_instruction(instr1, get_reltab_for_section(state.cur_sec_num, reltables));
			add_hex_instruction(instr2, get_reltab_for_section(state.cur_sec_num, reltables));
		}
		else if (addr_mode == 6 || addr_mode == 7) {
			/*delete_char(args[2], '[');
			delete_char(args[2], ']');
			
			instr2 = stoi(calculate_expression(args[2], symtab));

			if (state.error == true) {
				state.error = false;
				return -1;
			}
			location_counter += 4;
			
			to_rellocate = relocationNeeded(args, symtab);
			if (to_rellocate != "") {
				Symbol* sym = get_symbol(symtab, to_rellocate);
				RelSymbol* rs = NULL;
				RelTable* rt = get_reltab_for_section(sym->sec_num, reltables);
				if (sym->flag == 'L') rs = new RelSymbol(location_counter, 'A', sym->sec_num);
				if (sym->flag == 'G') rs = new RelSymbol(location_counter, 'A', sym->num);
				if (rs != NULL) add_rel_symbol(*rt, rs);
			}
		
			instr1 = (instr->opcode << 24) | (addr_mode << 21) | (((reg1 != -1) ? reg1 : 0) << 16) | (reg1 << 11) | (type << 3);
			add_hex_instruction(instr1, get_reltab_for_section(state.cur_sec_num, reltables));
			add_hex_instruction(instr2, get_reltab_for_section(state.cur_sec_num, reltables));*/

			int regi = -1;
			string izlaz;
			izlaz = calculate_expression(args[2], symtab, greske);

			if (addr_mode == 7) {
				vector<string> parsir;
				if (izlaz.find('+') != string::npos) {
					parsir = split_keep_delimiter(izlaz, "+");

				}
				else if (izlaz.find('-') != string::npos) {
					parsir = split_keep_delimiter(izlaz, "-");
				}
				regi = translate_register(parsir[0]);
				if (regi == -1) {
					greske.push_back("Registar ne postoji");
					return -1;
				}
				if (parsir.size() == 3 && parsir[1] == "+") instr2 = convert_to_num(parsir[2]);
				else if (parsir.size() == 3 && parsir[1] == "-") instr2 = convert_to_num(parsir[1] + parsir[2]);
				else instr2 = 0;
			}
			else instr2 = convert_to_num(izlaz);


			if (state.error == true) {
				state.error = false;
				return -1;
			}
			location_counter += 4;
			to_rellocate = relocationNeeded(args, symtab, greske);
			if (to_rellocate != "") {
				Symbol* sym = get_symbol(symtab, to_rellocate);
				RelSymbol* rs = NULL;
				RelTable* rt = get_reltab_for_section(state.cur_sec_num, reltables);
				Symbol* section = get_symbol(symtab, state.cur_section);
				if (sym->flag == 'L') rs = new RelSymbol(location_counter+section->addr-4, 'A', sym->sec_num);
				if (sym->flag == 'G') rs = new RelSymbol(location_counter+section->addr-4, 'A', sym->num);
				if (rs != NULL) add_rel_symbol(*rt, rs);
			}


			if (addr_mode==6) instr1 = (instr->opcode << 24) | (addr_mode << 21) | (((reg1 != -1) ? reg1 : 0) << 11) | (type<<3);
			if (addr_mode == 7) instr1 = (instr->opcode << 24) | (addr_mode << 21) | (((regi != -1) ? regi : 0) << 16) | (((reg1 != -1) ? reg1 : 0) << 11) | (type << 3);
			add_hex_instruction(instr1, get_reltab_for_section(state.cur_sec_num, reltables));
			add_hex_instruction(instr2, get_reltab_for_section(state.cur_sec_num, reltables));
		}
		else if (addr_mode == 4) {
			if (opindex == 10) {
				string ab = calculate_expression(args[2], symtab, greske);
				if (is_num(ab)) instr2 = stoi(ab);
				else instr2 = 0;

				if (state.error == true) {
					state.error = false;
					return -1;
				}
				location_counter += 4;
				to_rellocate = relocationNeeded(args, symtab, greske);
				if (to_rellocate != "") {
					Symbol* sym = get_symbol(symtab, to_rellocate);
					RelSymbol* rs = NULL;
					RelTable* rt = get_reltab_for_section(state.cur_sec_num, reltables);
					Symbol* section = get_symbol(symtab, state.cur_section);
					if (sym->flag == 'L') rs = new RelSymbol(location_counter+section->addr-4, 'A', sym->sec_num);
					if (sym->flag == 'G') rs = new RelSymbol(location_counter+section->addr-4, 'A', sym->num);
					if (rs != NULL) add_rel_symbol(*rt, rs);
				}

				instr1 = (instr->opcode << 24) | (addr_mode << 21) | (reg1 << 11) | (type << 3);
				add_hex_instruction(instr1, get_reltab_for_section(state.cur_sec_num, reltables));
				add_hex_instruction(instr2, get_reltab_for_section(state.cur_sec_num, reltables));
			}
			else if (opindex == 11) {
				greske.push_back("U STORE instrukciji ne sme biti neposredno adresiranje");
			}
		}
	}
}

void orgDirective(vector<string> args, vector<Symbol*>& symtab, vector<string>& greske) {
	if (args.size() != 2) {
		greske.push_back("Neodgovarajuci broj argumenata u liniji");
		return;
	}
	int rez;
	string ab = calculate_expression(args[2], symtab, greske);
	if (is_num(ab)) rez = stoi(ab);
	else rez = 0;
	
	if (state.error == true) {
		state.error = false;
		return;
	}
	//for (int i = 0; i < symtab.size(); i++)

	/*string to_rellocate = relocationNeeded(args, symtab);
	if (to_rellocate != "") {
		Symbol* sym = get_symbol(symtab, to_rellocate);
		RelSymbol* rs = NULL;
		RelTable* rt = get_reltab_for_section(sym->sec_num);
		if (sym->flag == 'L') rs = new RelSymbol(location_counter, 'A', sym->sec_num);
		if (sym->flag == 'G') rs = new RelSymbol(location_counter, 'A', sym->num);
		if (rs != NULL) add_rel_symbol(*rt, rs);
	}*/
}


void passTwo(vector<Symbol*>& symtab, vector<vector<string>>& asem, vector<RelTable*>& reltables, vector<string>& greske) {
	vector<string> novi;
	string prev = "";
	int orgVal = 0;

	for (int m = 0; m < asem.size(); m++)
		for (int n = 0; n < asem[m].size(); n++) {
			delete_char(asem[m][n], '\r');
		}

	for (int i = 0; i < asem.size(); i++) {
		if (!novi.empty()) novi.clear();
		if (is_label(asem[i][0])) {
			for (int j = 1; j < asem[i].size(); j++)
				novi.push_back(asem[i][j]);
		}
		else novi = asem[i];
		
		if (novi.size() >= 2) {
			if (novi[1] == "DEF" || novi[1] == "def") continue;
		}

		if (novi.empty()) {
			prev = ""; 
			continue;
		}
		if (novi[0] == ".end") break;

		else if (novi[0] == "ORG" || novi[0] == "org") {
			string ab = calculate_expression(novi[1], symtab, greske);
			if (is_num(ab)) orgVal = stoi(ab);
			else orgVal = 0;
			
			if (state.error==true) {
				state.error = false;
			}
		}
		else if (novi[0].find(".text") != string::npos || novi[0].find(".data") != string::npos ||
			novi[0].find(".rodata") != string::npos || novi[0].find(".bss") != string::npos) {


			for (int k = 0; k < symtab.size(); k++) {
				if (symtab[k]->name == novi[0]) {
					
					state.cur_section = novi[0];
					state.cur_sec_num = symtab[k]->sec_num;
					break;
				}
			}
			
			RelTable* rt = new RelTable(state.cur_sec_num);
			reltables.push_back(rt);

			location_counter = 0;
			state.cur_section = novi[0];
			if (prev == "ORG" || prev == "org") {
				Symbol* simb = get_symbol(symtab, novi[0]);
				if (simb != NULL) simb->flags += "O";

				for (int i = 0; i < symtab.size(); i++) {
					Symbol* s = symtab[i];
					if (s->sec_num == state.cur_sec_num) {
						s->addr += orgVal;
					}
				}
			}
		}
		else if (getInstruction(novi[0]) != -1) {
			int opindex = getInstruction(novi[0]);
			
			getAritmOp(opindex, novi, symtab, reltables, greske);
		}
		else if (novi[0] == "DB" || novi[0] == "DW" || novi[0] == "DD") {
			dataPassTwo(novi, symtab, reltables, greske);
		}


		prev = novi[0];
	}
}


void dataPassTwo(vector<string> str, vector<Symbol*>& symtab, vector<RelTable*>& reltables, vector<string>& greske) {//dd 1 dup 5
	int num_rep;
	if (str.size() == 4) {
		if (str[2] == "DUP") {
			int res;
			string ab = calculate_expression(str[1], symtab, greske);
			if (is_num(ab)) res = stoi(ab);
			else res = 0;
			
			if (state.error) {
				state.error = false;
				return;
			}
			num_rep = res;
			if (str[0] == "DB") {
				location_counter += num_rep;
			}
			else if (str[0] == "DW") {
				location_counter += 2 * num_rep;
			}
			else if (str[0] == "DD") {
				location_counter += 4 * num_rep;
			}
			if (str[3] != "?") {
				int32_t value = atoi(calculate_expression(str[3], symtab, greske).c_str());
				char num[9];
				if (state.error == true) {
					state.error = false;
					return;
				}
				sprintf(num, "%08x", value);
				uint8_t one;
				uint8_t two;
				uint8_t three;
				uint8_t four;
				if (str[0] == "DB") {
					one = strtol(num + 6, NULL, 16);
					for (int j = 0; j < num_rep; j++) {
						add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(one));
					}
				}
				if (str[0] == "DW") {
					one = strtol(num + 6, NULL, 16);
					num[6] = '\0';
					two = strtol(num + 4, NULL, 16);
					for (int j = 0; j < num_rep; j++) {
						add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(one));
						add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(two));
					}
				}
				if (str[0] == "DD") {
					one = strtol(num + 6, NULL, 16);
					num[6] = '\0';
					two = strtol(num + 4, NULL, 16);
					num[4] = '\0';
					three = strtol(num + 2, NULL, 16);
					num[2] = '\0';
					four = strtol(num + 0, NULL, 16);
					num[0] = '\0';
					for (int j = 0; j < num_rep; j++) {
						add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(one));
						add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(two));
						add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(three));
						add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(four));
					}
				}
			}
			else {
				/*if (str[0] == "DB") {
					location_counter += 1;
				}
				else if (str[0] == "DW") {
					location_counter += 2 ;
				}
				else if (str[0] == "DD") {
					location_counter += 4 ;
				}*/

				uint8_t one=0;
				uint8_t two=0;
				uint8_t three=0;
				uint8_t four=0;

				if (str[0] == "DB") {
					for (int j = 0; j < num_rep; j++) {
						add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(one));
					}
				}
				else if (str[0] == "DW") {
					for (int j = 0; j < num_rep; j++) {
						add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(one));
						add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(two));
					}
				}
				else if (str[0] == "DD") {
					for (int j = 0; j < num_rep; j++) {
						add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(one));
						add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(two));
						add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(three));
						add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(four));
					}
				}
			}
		}
	}
	else { //ako nema DUP
		if (str[0] == "DB") {
			location_counter += 1;
		}
		else if (str[0] == "DW") {
			location_counter += 2;
		}
		else if (str[0] == "DD") {
			location_counter += 4;
		}

		int32_t value = convert_to_num(calculate_expression(str[1], symtab, greske));
		char num[9];
		if (state.error == true) {
			state.error = false;
			return;
		}
		sprintf(num, "%08x", value);
		uint8_t one;
		uint8_t two;
		uint8_t three;
		uint8_t four;
		if (str[0] == "DB") {
			one = strtol(num + 6, NULL, 16);
			add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(one));
		}
		if (str[0] == "DW") {
			one = strtol(num + 6, NULL, 16);
			num[6] = '\0';
			two = strtol(num + 4, NULL, 16);
			add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(one));
			add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(two));
		}
		if (str[0] == "DD") {
			one = strtol(num + 6, NULL, 16);
			num[6] = '\0';
			two = strtol(num + 4, NULL, 16);
			num[4] = '\0';
			three = strtol(num + 2, NULL, 16);
			num[2] = '\0';
			four = strtol(num + 0, NULL, 16);
			num[0] = '\0';
			add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(one));
			add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(two));
			add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(three));
			add_content_to_table(*get_reltab_for_section(state.cur_sec_num, reltables), convert_to_hex(four));

		}
	}

	int ofset = 0;
	if (str[0] == "DB") {
		ofset = 1;
	}
	else if (str[0] == "DW") {
		ofset = 2;
	}
	else if (str[0] == "DD") {
		ofset = 4;
	}

	vector<string> rel_check;
	rel_check.push_back(str[0]);
	rel_check.push_back(str[1]);

	string to_rellocate = relocationNeeded(rel_check, symtab, greske);
	if (to_rellocate != "") {
		Symbol* sym = get_symbol(symtab, to_rellocate);
		RelSymbol* rs = NULL;
		RelTable* rt = get_reltab_for_section(state.cur_sec_num, reltables);
		Symbol* section = get_symbol(symtab, state.cur_section);
		if (sym->flag == 'L') rs = new RelSymbol(location_counter + section->addr - ofset, 'A', sym->sec_num);
		if (sym->flag == 'G') rs = new RelSymbol(location_counter + section->addr - ofset, 'A', sym->num);
		if (rs != NULL && rt != NULL) add_rel_symbol(*rt, rs);
	}
}

string relocationNeeded(vector<string> str, vector<Symbol*>& symtab, vector<string>& greske) { //str je ceo jedan red
	string rez="";

	if (str.size() == 0) return "";
	if (str.size() > 1) {
		if (str.size()==2) {
			rez = symbol_to_relocate(str[1], symtab, greske);
		}
		else if (str.size() == 3) {
			rez = symbol_to_relocate(str[2], symtab, greske);
		}
		else if (str.size() == 4) {
			rez = symbol_to_relocate(str[3], symtab, greske);
		}
	}
	return rez;
}

string symbol_to_relocate(string str, vector<Symbol*>& symtab, vector<string>& greske) {
	string deli = "+-*/()";
	vector<string> split = split_keep_delimiter(str, deli); //pretvori novi u nule i jedinice pa calculate expression
	for (int i = 0; i < split.size(); i++) {
		delete_char(split[i], '[');
		delete_char(split[i], ']');
		delete_char(split[i], '#');
		delete_char(split[i], '$');
	}

	string variable="";
	string novi = to_ones_and_zeros(split, symtab);
	string rez = calculate_expression(novi, symtab, greske);
	string prev_oper = "";
	if (rez == "1") {
		for (int i = 0; i < split.size(); i++) {
			Symbol* sym = get_symbol(symtab, split[i]);
			if (sym != NULL) {
				if (sym->sec_num != -1 && (prev_oper == "" || prev_oper == "+"))
					variable = split[i];
			}
			else if (is_operator(split[i])) prev_oper = split[i];
		}
	}
	else if (rez == "-1") {
		for (int i = 0; i < split.size(); i++) {
			Symbol* sym = get_symbol(symtab, split[i]);
			if (sym != NULL) {
				if (sym->sec_num != -1 && prev_oper == "-" )
					variable= split[i];
			}
			else if (is_operator(split[i])) prev_oper = split[i];
		}
	}
	return variable;

}

string to_ones_and_zeros(vector<string> str, vector<Symbol*>& symtab) {
	string novi;
	string oz = "";

	for (int i = 0; i < str.size(); i++) {
		if (!str[i].empty() && str[i][str[i].size() - 1] == '\r')
			str[i].erase(str[i].size() - 1);

		Symbol* sym = get_symbol(symtab, str[i]);
		if (sym != NULL && sym->sec_num != -1) {
			oz = "1";
			novi+=oz;
		}
		else if (sym != NULL && sym->sec_num == -1) {
			oz = "0";
			novi += oz;
		}
		else if (is_register(str[i])) {
			if (i != str.size() - 1) {
				i++;
				continue;
			}
		}
		else if (str[i] == "+" || str[i] == "-") {
			oz = str[i];
			novi+=oz;
		}
		else if (convert_to_num(str[i]) != 0 || convert_to_num(str[i])==0) {
			oz = "0";
			novi+=oz;
		}
	}
	return novi;
}

string convert_to_hex(int num) {
	char str[100];
	sprintf(str, "%x", num);
	string stri(str);
	if (stri.size() == 1) stri = "0" + stri;
	return stri;
}

void add_hex_instruction_first(uint32_t instr, RelTable* reltab) {
	uint32_t x = instr;

	char str[9];
	sprintf(str, "%08x", instr);
	uint8_t first = strtol(str + 6, NULL, 16);
	str[6] = '\0';
	uint8_t second = strtol(str + 4, NULL, 16);
	str[4] = '\0';
	uint8_t third = strtol(str + 2, NULL, 16);
	str[2] = '\0';
	uint8_t fourth = strtol(str, NULL, 16);;

	add_content_to_table(*reltab, convert_to_hex(fourth));
	add_content_to_table(*reltab, convert_to_hex(third));
	add_content_to_table(*reltab, convert_to_hex(second));
	add_content_to_table(*reltab, convert_to_hex(first));
}

void add_hex_instruction(uint32_t instr, RelTable* reltab) {
	uint32_t x = instr;
	/*uint8_t first = (x >> 24) & 0xFF;
	uint8_t second = (x >> 16) & 0xFF;
	uint8_t third = (x >> 8) & 0xFF;
	uint8_t fourth = (x >> 0) & 0xFF;*/
	char str[9];
	sprintf(str, "%08x", instr);
	uint8_t first = strtol(str + 6, NULL, 16);
	str[6] = '\0';
	uint8_t second = strtol(str + 4, NULL, 16);
	str[4] = '\0';
	uint8_t third = strtol(str + 2, NULL, 16);
	str[2] = '\0';
	uint8_t fourth = strtol(str, NULL, 16);;

	add_content_to_table(*reltab, convert_to_hex(first));
	add_content_to_table(*reltab, convert_to_hex(second));
	add_content_to_table(*reltab, convert_to_hex(third));
	add_content_to_table(*reltab, convert_to_hex(fourth));

}