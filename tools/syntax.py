import os

split_str = "----------"
split_str1 = "::="
fin = "../src/syntax/syntax.txt"
fout = "syntax.out"
if not os.path.isfile(fout):
    fd = open(fout, mode="w", encoding="utf-8")
    fd.close()

origin_production = {}
gen_production = {}

gen_note_count = 0
gen_note_pre = "GEN_TERMINAL_"
EPSILON = "EPSILON"
NOTE_QUES = "NOTE_QUES"
NOTE_STAR = "NOTE_STAR"
NOTE_PLUS = "NOTE_PLUS"
def gen_note_str():
    global gen_note_count
    gen_note_count += 1
    return gen_note_pre + str(gen_note_count)

def gen_star(str_production):
    new_symbol = gen_note_str()
    gen_production[new_symbol] = [str_production + " " + new_symbol, EPSILON]
    return new_symbol

def gen_plus(str_production):
    new_symbol = gen_note_str()
    gen_production[new_symbol] = [str_production + " " + new_symbol, str_production]
    return new_symbol

def gen_ques(str_production):
    new_symbol = gen_note_str()
    gen_production[new_symbol] = [str_production, EPSILON]
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



with open(fin) as file_in, open(fout) as file_out:
    pre_process = ""
    for line in file_in.readlines():
        pre_process += line.split("#")[0]

    # print(pre_process)
    non_terminals = pre_process.split(split_str)
    non_terminals_len = len(non_terminals)
    # test
    non_terminals_len = 2
    # test
    for i in range(1, non_terminals_len):  # idx of valid non_terminals
        non_terminal_v = non_terminals[i].strip().split(split_str1)
        non_terminal = non_terminal_v[0].strip()
        productions_v = non_terminal_v[1].strip().split("|")
        productions = []
        for p in productions_v:
            productions += expand_production(p.strip())
        origin_production[non_terminal] = productions
        # now we have non_terminal and productions in origin_production
        # and other productions in gen_production
        print("=================")
        print(origin_production)
        print("=================")
        print(gen_production)
        print("=================")
        print("=================")




