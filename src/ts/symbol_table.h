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
/* 简单树形结构不会造成循环引用的话，可以使用shared_ptr */
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

    std::string getExpandType() {
        if (type == nullptr) {
            return TS_SYMBOL_ERROR;
        }
        return type->getExpandType();
    }

    ~symbolItem()
    {
        // Todo: delete type
    }
};
using psymbolItem = symbolItem*;

/* store both symbols and expand types, both are expired out of scope */
struct symbolTable
{
    static std::vector<std::shared_ptr<symbolItem>> table;

    static bool add_symbol_mark(ESymbolMark sm);
    static bool add_symbol(ESymbolMark sm, pTypeTree tp);
    static bool pop_symbol();
    static bool pop_symbol_block(bool popMark);
    static std::shared_ptr<symbolItem> find_symbol(const std::string& symbolName);
    static std::shared_ptr<symbolItem> find_symbol_in_scope(const std::string& symbolName);
    static std::vector<std::shared_ptr<symbolItem>> get_symbols_in_scope();
    static std::shared_ptr<symbolItem> get_last_symbol();
    static void unionSymbolItems2Tree(pTypeTree pTT, const std::vector<std::shared_ptr<symbolItem>>& vTable);
    static void unionSingleItem2Tree(pTypeTree pTT, const std::shared_ptr<symbolItem>& item);
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
    assert(find_symbol_in_scope(tp->getSymbolName()) == nullptr);
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

std::shared_ptr<symbolItem> symbolTable::get_last_symbol()
{
    assert(!table.empty());
    unsigned idx = table.size() - 1;
    assert(table[idx]->isSymbolMark());
    return table[idx];
}

void symbolTable::unionSymbolItems2Tree(pTypeTree pTT, const std::vector<std::shared_ptr<symbolItem>>& vTable)
{
    assert(pTT != nullptr);
    if (pTT->children == nullptr) {
        pTT->children = new std::vector<pTypeTree>();
    }

    for (const auto& item: vTable) {
        pTT->children->push_back(item->type);
    }
}

/* const_cast */
void symbolTable::unionSingleItem2Tree(pTypeTree pTT, const std::shared_ptr<symbolItem>& item)
{
    assert(pTT != nullptr && item != nullptr);
    appendTree(pTT, item->type);
}

};  /* namespace ts */
};  /* namespace jhin */

