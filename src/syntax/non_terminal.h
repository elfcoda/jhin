/* this file is generated by syntax.py, do not modify it */


#ifndef __NON_TERMINAL_H__
#define __NON_TERMINAL_H__

#include <unordered_map>
#include <vector>
#include <string>


namespace jhin {
namespace syntax {

	#define NON_TERMINAL_IDX_MIN    1
	#define NON_TERMINAL_IDX_MAX    55
	#define SYNTAX_EPSILON_STR      "EPSILON"
	#define SYNTAX_EPSILON_IDX      56

	std::unordered_map<std::string, unsigned int> non_terminal_to_id = {
			{"Prog'", 1}, {"Prog", 2}, {"Prog_u", 3}, {"Cmd", 4}, {"CmdN", 5}, {"CmdC", 6}, {"CmdU", 7}, {"Formals", 8}, {"Formalt", 9}, 
			{"Formal", 10}, {"FormalU", 11}, {"Newlq", 12}, {"Newls", 13}, {"Newlp", 14}, {"Decls", 15}, {"Declt", 16}, {"Decl", 17}, {"DeclN", 18}, {"Class", 19}, 
			{"Proc", 20}, {"FnArg", 21}, {"FnRetTp", 22}, {"Proc_ar", 23}, {"Type", 24}, {"Exp", 25}, {"Exp0", 26}, {"Exp1", 27}, {"Exp2", 28}, {"Exp3", 29}, 
			{"NotExp", 30}, {"VoidExp", 31}, {"NewObj", 32}, {"ExpN", 33}, {"ReInt", 34}, {"ReDeci", 35}, {"ReStr", 36}, {"ReId", 37}, {"NewExp", 38}, {"ReValue", 39}, 
			{"FnCall", 40}, {"FnArgs", 41}, {"Exp_ar", 42}, {"GEN_TERMINAL_1", 43}, {"GEN_TERMINAL_2", 44}, {"GEN_TERMINAL_3", 45}, {"GEN_TERMINAL_4", 46}, {"GEN_TERMINAL_5", 47}, {"GEN_TERMINAL_6", 48}, {"GEN_TERMINAL_7", 49}, 
			{"GEN_TERMINAL_8", 50}, {"GEN_TERMINAL_9", 51}, {"GEN_TERMINAL_10", 52}, {"GEN_TERMINAL_11", 53}, {"GEN_TERMINAL_12", 54}, {"GEN_TERMINAL_13", 55}
	};

	std::unordered_map<unsigned int, std::string> id_to_non_terminal = {
			{1, "Prog'"}, {2, "Prog"}, {3, "Prog_u"}, {4, "Cmd"}, {5, "CmdN"}, {6, "CmdC"}, {7, "CmdU"}, {8, "Formals"}, {9, "Formalt"}, 
			{10, "Formal"}, {11, "FormalU"}, {12, "Newlq"}, {13, "Newls"}, {14, "Newlp"}, {15, "Decls"}, {16, "Declt"}, {17, "Decl"}, {18, "DeclN"}, {19, "Class"}, 
			{20, "Proc"}, {21, "FnArg"}, {22, "FnRetTp"}, {23, "Proc_ar"}, {24, "Type"}, {25, "Exp"}, {26, "Exp0"}, {27, "Exp1"}, {28, "Exp2"}, {29, "Exp3"}, 
			{30, "NotExp"}, {31, "VoidExp"}, {32, "NewObj"}, {33, "ExpN"}, {34, "ReInt"}, {35, "ReDeci"}, {36, "ReStr"}, {37, "ReId"}, {38, "NewExp"}, {39, "ReValue"}, 
			{40, "FnCall"}, {41, "FnArgs"}, {42, "Exp_ar"}, {43, "GEN_TERMINAL_1"}, {44, "GEN_TERMINAL_2"}, {45, "GEN_TERMINAL_3"}, {46, "GEN_TERMINAL_4"}, {47, "GEN_TERMINAL_5"}, {48, "GEN_TERMINAL_6"}, {49, "GEN_TERMINAL_7"}, 
			{50, "GEN_TERMINAL_8"}, {51, "GEN_TERMINAL_9"}, {52, "GEN_TERMINAL_10"}, {53, "GEN_TERMINAL_11"}, {54, "GEN_TERMINAL_12"}, {55, "GEN_TERMINAL_13"}
	};


	std::unordered_map<std::string, std::vector<std::vector<std::string>>> all_production = {
			{"Prog'", {{"Newls", "Prog"}}}, 
			{"Prog", {{"GEN_TERMINAL_1"}}}, 
			{"Prog_u", {{"Class", "Newlp"}, {"Proc", "Newlp"}}}, 
			{"Cmd", {{"CmdN"}, {"CmdC"}, {"CmdU"}}}, 
			{"CmdN", {{"RE_ID", "ASSIGN", "Exp"}}}, 
			{"CmdC", {{"WHILE", "LPAREN", "Exp", "RPAREN", "Newls", "LCURLY", "Formals", "RCURLY"}, {"IF", "LPAREN", "Exp", "RPAREN", "Newls", "LCURLY", "Formals", "RCURLY"}, {"IF", "LPAREN", "Exp", "RPAREN", "Newls", "LCURLY", "Formals", "RCURLY", "ELSE", "LCURLY", "Formals", "RCURLY"}}}, 
			{"CmdU", {{"WHILE", "LPAREN", "Exp", "RPAREN", "Newls", "FormalU", "NEWLINE"}, {"IF", "LPAREN", "Exp", "RPAREN", "Newls", "FormalU", "Newlq", "ELSE", "Newls", "FormalU", "NEWLINE"}, {"IF", "LPAREN", "Exp", "RPAREN", "Newls", "FormalU", "NEWLINE"}}}, 
			{"Formals", {{"Newls", "Formalt"}}}, 
			{"Formalt", {{"Formal", "GEN_TERMINAL_2"}, {"EPSILON"}}}, 
			{"Formal", {{"Cmd"}, {"Exp"}, {"DeclN"}}}, 
			{"FormalU", {{"CmdN"}, {"Exp"}, {"DeclN"}}}, 
			{"Newlq", {{"GEN_TERMINAL_3"}}}, 
			{"Newls", {{"GEN_TERMINAL_4"}}}, 
			{"Newlp", {{"GEN_TERMINAL_5"}}}, 
			{"Decls", {{"Newls", "Declt"}}}, 
			{"Declt", {{"Decl", "GEN_TERMINAL_6"}, {"EPSILON"}}}, 
			{"Decl", {{"DeclN"}, {"Class"}}}, 
			{"DeclN", {{"RE_ID", "COLON", "Type"}, {"RE_ID", "COLON", "Type", "ASSIGN", "Exp"}, {"Proc"}}}, 
			{"Class", {{"CLASS", "RE_VALUE", "Newls", "LCURLY", "Decls", "RCURLY"}, {"CLASS", "RE_VALUE", "INHERITS", "RE_VALUE", "Newls", "LCURLY", "Decls", "RCURLY"}}}, 
			{"Proc", {{"DEF", "RE_ID", "LPAREN", "FnArg", "RPAREN", "FnRetTp", "Newls", "LCURLY", "Formals", "RCURLY"}}}, 
			{"FnArg", {{"GEN_TERMINAL_7"}}}, 
			{"FnRetTp", {{"GEN_TERMINAL_8"}}}, 
			{"Proc_ar", {{"RE_ID", "COLON", "Type", "GEN_TERMINAL_9"}}}, 
			{"Type", {{"Exp"}, {"TYPE"}}}, 
			{"Exp", {{"RETURN", "Exp"}, {"CASE", "Exp", "OF", "Newls", "LCURLY", "GEN_TERMINAL_10", "OTHERWISE", "INFER", "Exp", "Newls", "RCURLY"}, {"THIS"}, {"LET", "DeclN", "GEN_TERMINAL_11", "IN", "LCURLY", "Exp", "RCURLY"}, {"RE_ID", "DOT", "RE_ID", "LPAREN", "GEN_TERMINAL_12", "RPAREN"}, {"RE_ID", "AT", "RE_VALUE", "DOT", "RE_ID", "LPAREN", "GEN_TERMINAL_12", "RPAREN"}, {"LPAREN", "Exp", "RPAREN", "DOT", "RE_ID", "LPAREN", "GEN_TERMINAL_12", "RPAREN"}, {"LPAREN", "Exp", "RPAREN", "AT", "RE_VALUE", "DOT", "RE_ID", "LPAREN", "GEN_TERMINAL_12", "RPAREN"}, {"LAMBDA", "ARROW", "LCURLY", "Exp", "RCURLY"}, {"LAMBDA", "DeclN", "GEN_TERMINAL_11", "ARROW", "LCURLY", "Exp", "RCURLY"}, {"Exp0"}}}, 
			{"Exp0", {{"Exp0", "EQ", "Exp1"}, {"Exp0", "LT", "Exp1"}, {"Exp0", "LE", "Exp1"}, {"Exp0", "GT", "Exp1"}, {"Exp0", "GE", "Exp1"}, {"Exp1"}}}, 
			{"Exp1", {{"Exp1", "PLUS", "Exp2"}, {"Exp1", "MINUS", "Exp2"}, {"Exp2"}}}, 
			{"Exp2", {{"Exp2", "STAR", "Exp3"}, {"Exp2", "SLASH", "Exp3"}, {"Exp3"}}}, 
			{"Exp3", {{"NotExp"}, {"VoidExp"}, {"ExpN"}, {"NewObj"}}}, 
			{"NotExp", {{"NOT", "ExpN"}}}, 
			{"VoidExp", {{"ISVOID", "ExpN"}}}, 
			{"NewObj", {{"NEW", "RE_VALUE"}}}, 
			{"ExpN", {{"ReInt"}, {"ReDeci"}, {"ReStr"}, {"ReId"}, {"TRUE"}, {"FALSE"}, {"NewExp"}, {"ReValue"}, {"OBJECT"}, {"BOOL"}, {"INT"}, {"FLOAT"}, {"DOUBLE"}, {"LONG"}, {"STRING"}, {"UNIT"}, {"FnCall"}}}, 
			{"ReInt", {{"RE_INT"}}}, 
			{"ReDeci", {{"RE_DECIMAL"}}}, 
			{"ReStr", {{"RE_STRING"}}}, 
			{"ReId", {{"RE_ID"}}}, 
			{"NewExp", {{"LPAREN", "Exp", "RPAREN"}}}, 
			{"ReValue", {{"RE_VALUE"}}}, 
			{"FnCall", {{"RE_ID", "LPAREN", "FnArgs", "RPAREN"}}}, 
			{"FnArgs", {{"GEN_TERMINAL_12"}}}, 
			{"Exp_ar", {{"Exp", "GEN_TERMINAL_13"}}}, 
			{"GEN_TERMINAL_1", {{"Prog_u", "GEN_TERMINAL_1"}, {"EPSILON"}}}, 
			{"GEN_TERMINAL_2", {{"Newlp", "Formalt"}, {"EPSILON"}}}, 
			{"GEN_TERMINAL_3", {{"NEWLINE"}, {"EPSILON"}}}, 
			{"GEN_TERMINAL_4", {{"NEWLINE", "GEN_TERMINAL_4"}, {"EPSILON"}}}, 
			{"GEN_TERMINAL_5", {{"NEWLINE", "GEN_TERMINAL_5"}, {"NEWLINE"}}}, 
			{"GEN_TERMINAL_6", {{"Newlp", "Declt"}, {"EPSILON"}}}, 
			{"GEN_TERMINAL_7", {{"Proc_ar"}, {"EPSILON"}}}, 
			{"GEN_TERMINAL_8", {{"COLON", "Type"}, {"EPSILON"}}}, 
			{"GEN_TERMINAL_9", {{"COMMA", "RE_ID", "COLON", "Type", "GEN_TERMINAL_9"}, {"EPSILON"}}}, 
			{"GEN_TERMINAL_10", {{"RE_ID", "COLON", "Type", "INFER", "Exp", "GEN_TERMINAL_10"}, {"EPSILON"}}}, 
			{"GEN_TERMINAL_11", {{"COMMA", "DeclN", "GEN_TERMINAL_11"}, {"EPSILON"}}}, 
			{"GEN_TERMINAL_12", {{"Exp_ar"}, {"EPSILON"}}}, 
			{"GEN_TERMINAL_13", {{"COMMA", "Exp", "GEN_TERMINAL_13"}, {"EPSILON"}}}
	};

	std::unordered_map<std::string, std::string> token_string_to_symbol = {
			{"NEQ", "!="}, {"ASSIGN", "<-"}, {"GE", ">="}, {"INFER", "=>"}, {"LE", "<="}, {"EQ", "=="}, {"ARROW", "->"}, {"LPAREN", "("}, {"RPAREN", ")"}, {"LCURLY", "{"}, 
			{"RCURLY", "}"}, {"LBRACK", "["}, {"RBRACK", "]"}, {"STAR", "*"}, {"AT", "@"}, {"COLON", ":"}, {"COMMA", ","}, {"IF", "if"}, {"WHILE", "while"}, {"DOT", "."}, 
			{"PLUS", "+"}, {"MINUS", "-"}, {"SLASH", "/"}, {"NOT", "!"}, {"LT", "<"}, {"GT", ">"}, {"FOR", "for"}, {"RETURN", "return"}, {"CASE", "case"}, {"OF", "of"}, 
			{"ELSE", "else"}, {"OTHERWISE", "otherwise"}, {"LAMBDA", "lambda"}, {"ISVOID", "isVoid"}, {"NEWLINE", "'back_n'"}, {"INHERITS", "inherits"}, {"CLASS", "class"}, {"NEW", "new"}, {"LET", "let"}, {"IN", "in"}, 
			{"DEF", "def"}, {"CALLCC", "callcc"}
	};

}     /* namespace syntax */
}     /* namespace jhin */


#endif

