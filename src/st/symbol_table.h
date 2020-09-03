/* available for type system and code generation phase */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "../../comm/type_tree.h"
#include "../lex/lex_meta.h"

namespace jhin
{
namespace st
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

    /* the declarations' index in the stack */
    unsigned idx;

    /* mark */
    ESymbolMark     mark;

    symbolItem(ESymbolMark sm, pTypeTree tp, unsigned idx) {
        this->mark = sm;
        this->type = tp;
        this->idx = idx;
    }

    bool isSymbolMark() { return mark == SYMBOL_MARK_SYMBOL; }
    bool isFnMark() { return mark == SYMBOL_MARK_FN; }
    bool isClassMark() { return mark == SYMBOL_MARK_CLASS; }
    bool isIfMark() { return mark == SYMBOL_MARK_IF; }
    bool isElseMark() { return mark == SYMBOL_MARK_ELSE; }
    bool isWhileMark() { return mark == SYMBOL_MARK_WHILE; }
    bool isCaseOfmark() { return mark == SYMBOL_MARK_CASE_OF; }

    std::string getSymbolName() {
        if (type == nullptr) {
            return TS_SYMBOL_ERROR;
        }
        return type->getSymbolName();
    }

    void setIdx(unsigned idx)
    {
        this->idx = idx;
    }

    unsigned getIdx()
    {
        return idx;
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

    static void initSymbolTable();
    static bool add_symbol_mark(ESymbolMark sm);
    static bool add_symbol(ESymbolMark sm, pTypeTree tp, unsigned idx);
    static bool pop_symbol();
    static unsigned pop_symbol_block(bool popMark);
    static std::shared_ptr<symbolItem> find_symbol(const std::string& symbolName);
    static std::shared_ptr<symbolItem> find_symbol_in_scope(const std::string& symbolName);
    static std::vector<std::shared_ptr<symbolItem>> get_symbols_in_scope();
    static std::shared_ptr<symbolItem> get_last_symbol();
    static void unionSymbolItems2Tree(pTypeTree pTT, const std::vector<std::shared_ptr<symbolItem>>& vTable);
    static void unionSingleItem2Tree(pTypeTree pTT, const std::shared_ptr<symbolItem>& item);
};
using pSymbolTable = symbolTable*;
std::vector<std::shared_ptr<symbolItem>> symbolTable::table;

void symbolTable::initSymbolTable()
{
    table.clear();
}

bool symbolTable::add_symbol_mark(ESymbolMark sm)
{
    table.push_back(std::make_shared<symbolItem>(sm, nullptr, 0));
    return true;
}

bool symbolTable::add_symbol(ESymbolMark sm, pTypeTree tp, unsigned idx)
{
    assert(find_symbol_in_scope(tp->getSymbolName()) == nullptr);
    table.push_back(std::make_shared<symbolItem>(sm, tp, idx));
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
unsigned symbolTable::pop_symbol_block(bool popMark = true)
{
    unsigned cnt = 0;
    assert(!table.empty());
    /* pop symbol mark */
    for (int idx = table.size() - 1; idx >= 0 && table[idx]->isSymbolMark(); idx--) {
        table.pop_back();
        cnt += 1;
    }

    /* pop non-symbol mark */
    if (popMark && !table.empty()) table.pop_back();

    return cnt;
}

std::shared_ptr<symbolItem> symbolTable::find_symbol(const std::string& symbolName)
{
    for (int idx = table.size() - 1; idx >= 0; idx--) {
        if (table[idx]->getSymbolName() == symbolName) {
            return table[idx];
        }
    }
    return nullptr;
}

std::shared_ptr<symbolItem> symbolTable::find_symbol_in_scope(const std::string& symbolName)
{
    for (int idx = table.size() - 1; idx >= 0 && table[idx]->isSymbolMark() ; idx--) {
        if (table[idx]->getSymbolName() == symbolName) {
            return table[idx];
        }
    }
    return nullptr;
}

std::shared_ptr<symbolItem> symbolTable::find_symbol_in_fn(const std::string& symbolName)
{
    for (int idx = table.size() - 1; idx >= 0 && !table[idx]->isFnMark() ; idx--) {
        if (table[idx]->getSymbolName() == symbolName) {
            return table[idx];
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<symbolItem>> symbolTable::get_symbols_in_scope()
{
    std::vector<std::shared_ptr<symbolItem>> ans;
    for (int idx = table.size() - 1; idx >= 0 && table[idx]->isSymbolMark() ; idx--) {
        ans.push_back(table[idx]);
    }
    std::reverse(ans.begin(), ans.end());
    return ans;
}

std::shared_ptr<symbolItem> symbolTable::get_last_symbol()
{
    assert(!table.empty());
    int idx = table.size() - 1;
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

/**/
pTypeTree getRecentFn()
{
    int idx = 0;
    for (idx = table.size() - 1; idx >= 0 && !table[idx]->isFnMark() ; idx--) {}
    if (idx < 0) return nullptr;
    assert(idx >= 1);
    return table[idx - 1]->type;
}

};  /* namespace st */
};  /* namespace jhin */

