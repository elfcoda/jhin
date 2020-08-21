#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "type_tree.h"
#include "../lex/lex_meta.h"

namespace jhin
{
namespace ts
{

/* affect symbols' lifetime in scope */
enum ESymbolMark
{
    SYMBOL_MARK_SYMBOL = 1,
    SYMBOL_MARK_FN,
    SYMBOL_MARK_CLASS,
    SYMBOL_MARK_IF,
    SYMBOL_MARK_ELSE,
    SYMBOL_MARK_WHILE,
    SYMBOL_MARK_CASE_OF,
    SYMBOL_MARK_LAMBDA,
    SYMBOL_MARK_LET_IN,
};

const std::string TS_SYMBOL_ERROR = "UNKNOWN ERROR";
struct symbolItem
{
    /* text */
    // std::string     symbolName; // deprecated
    pTypeTree       type;
    lex::pMetaInfo  symbolInfo;

    /* mark */
    ESymbolMark     mark;

    symbolItem(ESymbolMark sm, pTypeTree tp) {
        mark = sm;
        type = tp;
    }

    bool isSymbolMark() { return mark == SYMBOL_MARK_SYMBOL; }
    bool isFnMark() { return mark == SYMBOL_MARK_FN; }
    bool isClassMark() { return mark == SYMBOL_MARK_CLASS; }
    bool isIfMark() {return mark == SYMBOL_MARK_IF; }
    bool isElseMark() {return mark == SYMBOL_MARK_ELSE; }
    bool isWhileMark() {return mark == SYMBOL_MARK_WHILE; }
    bool isCaseOfmark() {return mark == SYMBOL_MARK_CASE_OF; }

    std::string getSymbolName() {
        if (type == nullptr) {
            return TS_SYMBOL_ERROR;
        }
        return type->getSymbolName();
    }
};
using psymbolItem = symbolItem*;

struct symbolTable
{
    static std::vector<std::shared_ptr<symbolItem>> table;

    static bool add_symbol_mark(ESymbolMark sm);
    static bool add_symbol(ESymbolMark sm, const std::string& name, pTypeTree tp);
    static bool pop_symbol();
    static std::shared_ptr<symbolItem> find_symbol(const std::string& symbolName);
    static std::shared_ptr<symbolItem> find_symbol_in_scope(const std::string& symbolName);
    static std::vector<std::shared_ptr<symbolItem>> get_symbols_in_scope();
};
using pSymbolTable = symbolTable*;
std::vector<std::shared_ptr<symbolItem>> symbolTable::table;

bool symbolTable::add_symbol_mark(ESymbolMark sm)
{
    table.push_back(std::make_shared<symbolItem>(sm, nullptr));
    return true;
}

bool symbolTable::add_symbol(ESymbolMark sm, pTypeTree tp)
{
    table.push_back(std::make_shared<symbolItem>(sm, tp));
    return true;
}

/* deprecated */
bool symbolTable::pop_symbol()
{
    assert(!table.empty());
    table.pop_back();
    return true;
}

/* @popMark: false when clear current block symbols */
bool symbolTable::pop_symbol_block(bool popMark = true)
{
    assert(!table.empty());
    /* pop symbol mark */
    for (unsigned idx = table.size() - 1; idx >= 0 && table[idx]->isSymbolMark(); idx--) {
        table.pop_back();
    }

    /* pop non-symbol mark */
    if (popMark && !table.empty()) table.pop_back();

    return true;
}

std::shared_ptr<symbolItem> symbolTable::find_symbol(const std::string& symbolName)
{
    for (unsigned idx = table.size() - 1; idx >= 0; idx--) {
        if (table[idx]->getSymbolName() == symbolName) {
            return table[idx];
        }
    }
    return nullptr;
}

std::shared_ptr<symbolItem> symbolTable::find_symbol_in_scope(const std::string& symbolName)
{
    for (unsigned idx = table.size() - 1; idx >= 0 && table[idx]->isSymbolMark() ; idx--) {
        if (table[idx]->getSymbolName() == symbolName) {
            return table[idx];
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<symbolItem>> symbolTable::get_symbols_in_scope()
{
    std::vector<std::shared_ptr<symbolItem>> ans;
    for (unsigned idx = table.size() - 1; idx >= 0 && table[idx]->isSymbolMark() ; idx--) {
        ans.push_back(table[idx]);
    }
    std::reverse(ans.begin(), ans.end());
    return ans;
}

void unnionSymbolItems2Class(std::vector<std::shared_ptr<symbolItem>>& vTable)
{

}


};  /* namespace ts */
};  /* namespace jhin */

