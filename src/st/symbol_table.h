/* available for type system and code generation phase */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "../ast/ast_node_semantic.h"
#include "../../comm/jhin_assert.h"
#include "../lex/lex_meta.h"

namespace jhin
{
namespace st
{

enum SymbolTag
{
    ST_DEFAULT_SYMBOL = 0,
    ST_FUNCTION_SCOPE,
    ST_CLASS_SCOPE,
    ST_IF_SCOPE,
    ST_ELSE_SCOPE,
    ST_WHILE_SCOPE,
    ST_CASE_OF_SCOPE,
};

const std::string TS_SYMBOL_ERROR = "UNKNOWN ERROR";
struct symbolItem
{
    lex::pMetaInfo      symbolInfo;

    // symbol name: x
    std::string         name;

    // symbol type: Int
    std::string         type;

    // symbol value: 6
    std::string         value;

    // symbol tag: DefaultSymbol / FunctionScope / ClassScope / IfScope / ElseScope / WhileScope / LambdaScope
    SymbolTag           tag;

    symbolItem(std::string name, std::string type, std::string value, SymbolTag tag)
    {
        this->name = name;
        this->type = type;
        this->value = value;
        this->tag = tag;
    }

    bool isFnAST() { return ST_FUNCTION_SCOPE == tag; }
    bool isClassAST() { return ST_CLASS_SCOPE == tag; }
    bool isIfAST() { return ST_IF_SCOPE == tag; }
    bool isElseAST() { return ST_ELSE_SCOPE == tag; }
    bool isWhileAST() { return ST_WHILE_SCOPE == tag; }
    bool isCaseOfAST() { return ST_CASE_OF_SCOPE == tag; }

    std::string getSymbolName()
    {
        return name;
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
    static bool add_symbol(std::string name, std::string type, std::string value, SymbolTag tag);
    static bool pop_symbol();
    static unsigned pop_symbol_block();
    static std::shared_ptr<symbolItem> find_symbol(const std::string& symbolName);
    static std::shared_ptr<symbolItem> find_symbol_in_scope(const std::string& symbolName);
    static std::shared_ptr<symbolItem> find_symbol_in_fn(const std::string& symbolName);
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

bool symbolTable::add_symbol(std::string name, std::string type, std::string value, SymbolTag tag)
{
    JHIN_ASSERT_BOOL(find_symbol_in_scope(node->getSymbolName()) == nullptr);
    table.push_back(std::make_shared<symbolItem>(name, type, value, tag));
    return true;
}

/* deprecated */
bool symbolTable::pop_symbol()
{
    JHIN_ASSERT_BOOL(!table.empty());
    table.pop_back();
    return true;
}

unsigned symbolTable::pop_symbol_block()
{
    unsigned cnt = 0;
    JHIN_ASSERT_BOOL(!table.empty());

    for (int idx = table.size() - 1; idx >= 0 && table[idx]->isSymbolAST(); idx--) {
        table.pop_back();
        cnt += 1;
    }

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
    for (int idx = table.size() - 1; idx >= 0 && table[idx]->isSymbolAST() ; idx--) {
        if (table[idx]->getSymbolName() == symbolName) {
            return table[idx];
        }
    }
    return nullptr;
}

std::shared_ptr<symbolItem> symbolTable::find_symbol_in_fn(const std::string& symbolName)
{
    for (int idx = table.size() - 1; idx >= 0 && !table[idx]->isFnAST() ; idx--) {
        if (table[idx]->getSymbolName() == symbolName) {
            return table[idx];
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<symbolItem>> symbolTable::get_symbols_in_scope()
{
    std::vector<std::shared_ptr<symbolItem>> ans;
    for (int idx = table.size() - 1; idx >= 0 && table[idx]->isSymbolAST() ; idx--) {
        ans.push_back(table[idx]);
    }
    std::reverse(ans.begin(), ans.end());
    return ans;
}

std::shared_ptr<symbolItem> symbolTable::get_last_symbol()
{
    JHIN_ASSERT_BOOL(!table.empty());
    int idx = table.size() - 1;
    JHIN_ASSERT_BOOL(table[idx]->isSymbolAST());
    return table[idx];
}

void symbolTable::unionSymbolItems2Tree(pTypeTree pTT, const std::vector<std::shared_ptr<symbolItem>>& vTable)
{
    // TODO
}

/* const_cast */
void symbolTable::unionSingleItem2Tree(pTypeTree pTT, const std::shared_ptr<symbolItem>& item)
{
    // TODO
}

/**/
std::shared_ptr<symbolItem> getRecentFn()
{
    int idx = 0;
    for (idx = symbolTable::table.size() - 1; idx >= 0 && !symbolTable::table[idx]->isFnAST() ; idx--) {}
    if (idx < 0) return nullptr;
    JHIN_ASSERT_BOOL(idx >= 1);
    return symbolTable::table[idx - 1];
}

}   /* namespace st */
}   /* namespace jhin */

