import os

split_str = "----------"
split_str1 = "::="
fin = "../src/syntax/syntax.txt"
# fout = "syntax.out"
# if not os.path.isfile(fout):
#     fd = open(fout, mode="w", encoding="utf-8")
#     fd.close()

origin_production = {}
gen_production = {}
all_production_replaced = {}

gen_note_count = 0
gen_note_pre = "GEN_TERMINAL_"
EPSILON = "EPSILON"
NOTE_QUES = "NOTE_QUES"
NOTE_STAR = "NOTE_STAR"
NOTE_PLUS = "NOTE_PLUS"
gened_star = {}
gened_ques = {}
gened_plus = {}
def gen_note_str():
    global gen_note_count
    gen_note_count += 1
    return gen_note_pre + str(gen_note_count)

def gen_star(str_production):
    global gen_production
    global gened_star
    if str_production in gened_star:
        return gened_star[str_production]
    else:
        new_symbol = gen_note_str()
        gen_production[new_symbol] = [str_production + " " + new_symbol, EPSILON]
        gened_star[str_production] = new_symbol
        return new_symbol

def gen_plus(str_production):
    global gen_production
    global gened_plus
    if str_production in gened_plus:
        return gened_plus[str_production]
    else:
        new_symbol = gen_note_str()
        gen_production[new_symbol] = [str_production + " " + new_symbol, str_production]
        gened_plus[str_production] = new_symbol
        return new_symbol

def gen_ques(str_production):
    global gen_production
    global gened_ques
    if str_production in gened_ques:
        return gened_ques[str_production]
    else:
        new_symbol = gen_note_str()
        gen_production[new_symbol] = [str_production, EPSILON]
        gened_ques[str_production] = new_symbol
        return new_symbol

def substitute_production(production, NOTE_KIND):
    while production.find(NOTE_KIND) != -1:
        idx1 = production.find(NOTE_KIND)
        idx2 = production.find("(", idx1)
        idx3 = production.find(")", idx1)
        if NOTE_KIND == NOTE_QUES:
            gen_func = gen_ques
        elif NOTE_KIND == NOTE_STAR:
            gen_func = gen_star
        elif NOTE_KIND == NOTE_PLUS:
            gen_func = gen_plus
        new_symbol = gen_func(production[idx2+1: idx3])
        s_replace = production[idx1: idx3+1]
        production = production.replace(s_replace, new_symbol, 1)

    return production

# handle NOTE_ macro
def expand_production(production):
    production = substitute_production(production, NOTE_QUES)
    production = substitute_production(production, NOTE_STAR)
    production = substitute_production(production, NOTE_PLUS)

    return production

list1 = ["!=", "<-", ">=", "=>", "<=", "==", "->",
        "(", ")", "{", "}", "[", "]", "*", "@", ":", ",", \
        "if", "while", ".", "+", "-", "/", "!", \
        "<", ">", "for", "return", "case", "of", "else", \
        "otherwise", "lambda", "isVoid", "'back_n'", "inherits", "class", "new", "let", "in", "def", "callcc"
        ]

list2 = ["NEQ", "ASSIGN", "GE", "INFER", "LE", "EQ", "ARROW",
        "LPAREN", "RPAREN", "LCURLY", "RCURLY", "LBRACK", "RBRACK", "STAR", "AT", "COLON", "COMMA", \
        "IF", "WHILE", "DOT", "PLUS", "MINUS", "SLASH", "NOT", \
        "LT", "GT", "FOR", "RETURN", "CASE", "OF", "ELSE", \
        "OTHERWISE", "LAMBDA", "ISVOID", "NEWLINE", "INHERITS", "CLASS", "NEW", "LET", "IN", "DEF", "CALLCC"
        ]

# replace "*" by STAR, and so on
def replace_production(production):
    global list1
    global list2

    for (l1, l2) in zip(list1, list2):
        production = production.replace(l1, l2)

    return production

def gen_token_string_to_symbol():
    global list1
    global list2

    s = ""
    cnt = 0
    for (l1, l2) in zip(list1, list2):
        s += "{\"" + l2 + "\", \"" + l1 + "\"}, "
        cnt += 1
        if (cnt % 10 == 0):
            s += "\n\t\t\t"

    return "\n\t\t\t" + s[: -2]



def show_production(production_dict):
    # format
    print("=================")
    for k, v in production_dict.items():
        for production in v:
            print(k + "\t ::= " + production)
    print("=================")

def gen_non_terminal_list():
    global origin_production
    global gen_production
    global all_production_replaced
    non_terminal_v = []
    # origin_production
    for k, v in origin_production.items():
        non_terminal_v.append(k)
        v_replaced = []
        for production in v:
            v_replaced.append(replace_production(production))
        all_production_replaced[k] = v_replaced

    # gen_production
    for k, v in gen_production.items():
        non_terminal_v.append(k)
        v_replaced = []
        for production in v:
            v_replaced.append(replace_production(production))
        all_production_replaced[k] = v_replaced
    return non_terminal_v

# generate non_terminal_file
non_terminal_file = "non_terminal.h"
def gen_cpp_code(non_terminal_to_id, id_to_non_terminal, non_ter_cnt_min, non_ter_cnt, all_production, token_string_to_symbol):
    s = "/* this file is generated by syntax.py, do not modify it */\n\n\n"
    s_define = "__" + non_terminal_file.upper().replace(".", "_") + "__"
    s += "#ifndef " + s_define + "\n"
    s += "#define " + s_define + "\n\n"
    s += "#include <unordered_map>\n"
    s += "#include <vector>\n"
    s += "#include <string>\n\n\n"
    s += "namespace jhin {\n"
    s += "namespace syntax {\n\n"
    s += "\t#define NON_TERMINAL_IDX_MIN    " + str(non_ter_cnt_min) + "\n"
    s += "\t#define NON_TERMINAL_IDX_MAX    " + str(non_ter_cnt) + "\n"
    s += "\t#define SYNTAX_EPSILON_STR      \"" + EPSILON + "\"\n"
    s += "\t#define SYNTAX_EPSILON_IDX      " + str(non_ter_cnt+1) + "\n\n"
    s += "\tstd::unordered_map<std::string, unsigned int> non_terminal_to_id = {" + non_terminal_to_id + "\n\t};\n\n"
    s += "\tstd::unordered_map<unsigned int, std::string> id_to_non_terminal = {" + id_to_non_terminal + "\n\t};\n\n\n"
    s += "\tstd::unordered_map<std::string, std::vector<std::vector<std::string>>> all_production = {" + all_production + "\n\t};\n\n"
    s += "\tstd::unordered_map<std::string, std::string> token_string_to_symbol = {" + token_string_to_symbol + "\n\t};\n\n"

    s += "}     /* namespace syntax */\n"
    s += "}     /* namespace jhin */\n\n\n"
    # end of file
    s += "#endif\n\n"
    with open("../src/syntax/" + non_terminal_file, "w") as non_ter_file:
        non_ter_file.write(s)


def gen_non_terminal_code(non_terminal_v):
    # non_terminal_to_id
    non_ter_str = ""
    id_to_non_terminal = ""
    non_ter_cnt = 0
    non_ter_cnt_min = non_ter_cnt + 1
    for item in non_terminal_v:
        non_ter_str += ", "
        id_to_non_terminal += ", "
        non_ter_cnt += 1
        if non_ter_cnt % 10 == 0:
            non_ter_str += "\n\t\t\t"
            id_to_non_terminal += "\n\t\t\t"
        non_ter_str += ("{\"" + item + "\", " + str(non_ter_cnt) + "}")
        id_to_non_terminal += ("{" + str(non_ter_cnt) + ", \"" + item + "\"}")
    non_ter_str = "\n\t\t\t" + non_ter_str[2:]
    id_to_non_terminal = "\n\t\t\t" + id_to_non_terminal[2:]

    # gen_all_production_cpp_map
    all_production = gen_all_production_cpp_map()

    # eg: "WHILE" -> "while", "PLUS" -> "+"
    token_string_to_symbol = gen_token_string_to_symbol()

    gen_cpp_code(non_ter_str, id_to_non_terminal, non_ter_cnt_min, non_ter_cnt, all_production, token_string_to_symbol)


def gen_all_production_cpp_map():
    global all_production_replaced
    s = ""
    for k, v in all_production_replaced.items():
        outer_vector = ""
        for vs in v:
            vss = vs.split()
            inner_vector = ""
            for vss_symbol in vss:
                inner_vector += ", \"" + vss_symbol.strip() + "\""
            inner_vector = ", {" + inner_vector[2: ] + "}"  # , {"STAR", "NOT"}
            outer_vector += inner_vector
        outer_vector = "{" + outer_vector[2: ] + "}"        # {{"STAR", "NOT"}, {"STAR", "NOT"}}
        s += ", \n\t\t\t{\"" + k + "\", " + outer_vector + "}"      # , {"k", {{"STAR", "NOT"}, {"STAR", "NOT"}}}, {"k", {{"STAR", "NOT"}, {"STAR", "NOT"}}}
    return s[2: ]


def genMap(body, startId, fst_type, snd_type, map_name):
    # generate unordered_map
    ast_map_body = "const std::unordered_map<" + fst_type + ", " + snd_type + "> " + map_name + " = {\n\t"
    astStartLeafId = startId
    for idx in range(len(body)):
        item = body[idx]
        if fst_type == "std::string":
            ast_map_body += "{\"" + item + "\", " + str(astStartLeafId) + "}, "
        elif snd_type == "std::string":
            ast_map_body += "{" + str(astStartLeafId) + ", \"" + item + "\"}, "
        else:
            pass
        astStartLeafId += 1
        if astStartLeafId % 10 == 0:
            ast_map_body += "\n\t"
    if astStartLeafId % 10 == 0:
        ast_map_body = ast_map_body[:-4]
    else:
        ast_map_body = ast_map_body[:-2]
    ast_map_body += "\n};\n\n"

    return ast_map_body



ast_leaf_file_name = "ast_leaf.h"
def handleLEAF(body):
    LEAFPrefix = "AST_LEAF_"
    LEAF_START_MACRO = "AST_LEAF_START"
    LEAF_START_MACRO_VALUE = 1

    src = "/* this file is generated by syntax.py, do not modify it */\n\n"
    src += "#pragma once\n\n"
    src += "#include <string>\n"
    src += "#include <unordered_map>\n\n\n"
    src += "namespace jhin\n{\n"
    src += "namespace ast\n{\n\n"
    src += "#define " + LEAF_START_MACRO + "\t" + str(LEAF_START_MACRO_VALUE) + "\n\n"

    # generate enum
    src += "enum EASTLeaf\n"
    src += "{\n"
    for idx in range(len(body)):
        item = body[idx]
        if idx == 0:
            src += "\t" + LEAFPrefix + item + " = " + LEAF_START_MACRO + ",\n"
        else:
            src += "\t" + LEAFPrefix + item + ",\n"
    src += "};\n\n"

    # generate unordered_map
    src += genMap(body, LEAF_START_MACRO_VALUE, "unsigned", "std::string", "ast_leafid_to_string")
    src += genMap(body, LEAF_START_MACRO_VALUE, "std::string", "unsigned", "ast_string_to_leafid")

    src += "}     /* namespace ast */\n"
    src += "}     /* namespace jhin */\n\n\n"

    with open("../src/ast/" + ast_leaf_file_name, "w") as ast_leaf_file:
        ast_leaf_file.write(src)


with open(fin, "r") as file_in:         # , open(fout, "w") as file_out:
    pre_process = ""
    for line in file_in.readlines():
        pre_process += line.split("#")[0]

    # print(pre_process)
    non_terminals = pre_process.split(split_str)
    non_terminals_len = len(non_terminals)
    for i in range(1, non_terminals_len):  # idx of valid non_terminals
        non_terminal_v = non_terminals[i].strip().split(split_str1)
        non_terminal = non_terminal_v[0].strip()
        productions_v = non_terminal_v[1].strip().split("|")
        productions = []
        for p in productions_v:
            productions.append(expand_production(p.strip()))
        origin_production[non_terminal] = productions
    # now we have non_terminal and productions in origin_production
    # and other productions in gen_production

    # show_production(origin_production)
    # show_production(gen_production)
    non_terminal_v = gen_non_terminal_list()
    show_production(all_production_replaced)
    gen_non_terminal_code(non_terminal_v)

    # process non_terminals[0]: some declarations
    declarations = non_terminals[0].strip().split("%%")
    for declaration in declarations:
        decl = declaration.strip().split("@", 1)
        head = decl[0].strip()
        body_o = decl[1].strip().split(",")
        body = []
        for body_item in body_o:
            body.append(body_item.strip())
        if head == "LEAF":
            handleLEAF(body)




