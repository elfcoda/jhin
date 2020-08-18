#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../lex/lex_meta.h"

namespace jhin
{
namespace ts
{

#define SYMBOL_TYPE_START_ID    1
#define SYMBOL_TYPE_START_VALUE 1024

enum ESymbolType
{
    /* RE_ID */
    SYMBOL_TYPE_INT = SYMBOL_TYPE_START_ID,
    SYMBOL_TYPE_FLOAT,
    SYMBOL_TYPE_DOUBLE,
    SYMBOL_TYPE_LONG,
    SYMBOL_TYPE_OBJECT,
    SYMBOL_TYPE_BOOL,
    SYMBOL_TYPE_STRING,
    SYMBOL_TYPE_UNIT,
    SYMBOL_TYPE_TYPE,
    SYMBOL_TYPE_FN,

    /* RE_VALUE */
    SYMBOL_TYPE_CLASS = SYMBOL_TYPE_START_VALUE,
};

/* affect symbols' lifetime */
enum ESymbolMark
{
    SYMBOL_MARK_SYMBOL = 1,
    SYMBOL_MARK_FN,
    SYMBOL_MARK_CLASS,
    SYMBOL_MARK_IF,
    SYMBOL_MARK_ELSE,
    SYMBOL_MARK_WHILE,
    SYMBOL_MARK_CASE_OF,
};

struct symbolItem
{
    /* text */
    std::string     symbolName;
    ESymbolType     type;
    lex::pMetaInfo  symbolInfo;
    /* mark */
    ESymbolMark     mark;

    symbolItem(ESymbolMark sm, const std::string& name, ESymbolType tp) {
        mark = sm;
        symbolName = name;
        type = tp;
    }

    bool isSymbolMark() { return mark == SYMBOL_MARK_SYMBOL; }
    bool isFnMark() { return mark == SYMBOL_MARK_FN; }
    bool isClassMark() { return mark == SYMBOL_MARK_CLASS; }
    bool isIfMark() {return mark == SYMBOL_MARK_IF; }
    bool isElseMark() {return mark == SYMBOL_MARK_ELSE; }
    bool isWhileMark() {return mark == SYMBOL_MARK_WHILE; }
    bool isCaseOfmark() {return mark == SYMBOL_MARK_CASE_OF; }

    std::string getSymbolName() { return symbolName; }
};
using pSymbolItem = symbolItem*;

struct symbolTable
{
    static std::vector<std::shared_ptr<SymbolItem>> table;

    bool add_symbol(ESymbolMark sm, const std::string& name, ESymbolType tp)
    {
        table.push_back(std::make_shared<symbolItem>(sm, name, tp));
        return true;
    }

    bool pop_symbol(unsigned n)
    {
        while (n--) {
            assert(!table.empty());
            table.pop_back();
        }
        return true;
    }

    std::shared_ptr<SymbolItem> find_symbol(const std::string& symbolName)
    {
        for (unsigned idx = table.size() - 1; idx >= 0; idx--) {
            if (table[idx]->getSymbolName() == symbolName) {
                return table[idx];
            }
        }
        return nullptr;
    }

    std::shared_ptr<SymbolItem> find_symbol_in_scope(const std::string& symbolName)
    {
        for (unsigned idx = table.size() - 1; idx >= 0 && table[idx]->isSymbolMark() ; idx--) {
            if (table[idx]->getSymbolName() == symbolName) {
                return table[idx];
            }
        }
        return nullptr;
    }


};
using pSymbolTable = symbolTable*;
std::vector<std::shared_ptr<SymbolItem>> symbolTable::table;


};  /* namespace ts */
};  /* namespace jhin */

