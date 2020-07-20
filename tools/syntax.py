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
def gen_note_str():
    gen_note_count += 1
    return gen_note_pre + str(gen_note_count)

def gen_star(str_production):
    new_symbol = gen_note_str()
    gen_production[new_symbol] = [str_production + " " + new_symbol, EPSILON]
    return new_symbol

def gen_plus(str_production):
    new_symbol = gen_note_str()
    gen_production[new_symbol] = [str_production + " " + new_symbol， str_production]
    return new_symbol

def gen_ques(str_production):
    new_symbol = gen_note_str()
    gen_production[new_symbol] = [str_production, EPSILON]
    return new_symbol

# handle NOTE_ macro
def expand_production(production):
    while production.find("NOTE_QUES") != -1:
        idx1 = production.find("NOTE_QUES")
        idx2 = production.find("(", idx1)
        idx3 = production.find(")", idx1)
        new_symbol = gen_ques(production[idx2+1: idx3])


with open(fin) as file_in, open(fout) as file_out:
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
            productions += p.strip()
        # now we have non_terminal and productions




