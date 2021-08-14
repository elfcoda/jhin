/* available for type system and code generation phase */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdlib>
#include <ctime>
#include "llvm/IR/Type.h"
#include "../ast/ast_node_semantic.h"
#include "../../comm/jhin_assert.h"
#include "../lex/lex_meta.h"
#include "../../comm/type_tree.h"

namespace jhin
{
namespace st
{

struct userDefinedType
{
    private:
        // type name: Class Cat
        std::string name;

    public:
        userDefinedType(std::string name)
        {
            this->name = name;
        }

        ~userDefinedType() {}

        std::string getName()
        {
            return name;
        }
};

struct TypeTable
{
    static std::vector<std::shared_ptr<userDefinedType>> table;

    static void addType(std::shared_ptr<userDefinedType> tp);
    static void addType(std::string name);
    /// pop type when necessary
    static void popType();
    static std::shared_ptr<userDefinedType> findType(std::string typeName);
};
std::vector<std::shared_ptr<userDefinedType>> TypeTable::table;

void TypeTable::addType(std::shared_ptr<userDefinedType> tp)
{
    table.push_back(tp);
}

void TypeTable::addType(std::string name)
{
    table.push_back(std::make_shared<userDefinedType>(name));
}

void TypeTable::popType()
{
    // TODO
}

std::shared_ptr<userDefinedType> TypeTable::findType(std::string typeName)
{
    for (int i = table.size() - 1; i >= 0; i--)
    {
        if (table[i]->getName() == typeName)
        {
            return table[i];
        }
    }
    JHIN_ASSERT_STR("Type Not Found!");
    return nullptr;
}


enum SymbolTag
{
    ST_DEFAULT_SYMBOL = 0,
    ST_FUNCTION_SCOPE,
    ST_CLASS_SCOPE,
    ST_THEN_SCOPE,
    ST_ELSE_SCOPE,
    ST_WHILE_SCOPE,
    ST_CASE_OF_SCOPE,
};

std::map<SymbolTag, std::string> SymbolTag2String =
{
    {ST_DEFAULT_SYMBOL, "ST_DEFAULT_SYMBOL"},
    {ST_FUNCTION_SCOPE, "ST_FUNCTION_SCOPE"},
    {ST_CLASS_SCOPE, "ST_CLASS_SCOPE"},
    {ST_THEN_SCOPE, "ST_THEN_SCOPE"},
    {ST_ELSE_SCOPE, "ST_ELSE_SCOPE"},
    {ST_WHILE_SCOPE, "ST_WHILE_SCOPE"},
    {ST_CASE_OF_SCOPE, "ST_CASE_OF_SCOPE"}
};

std::string getStrForSymbolTag(SymbolTag tag)
{
    JHIN_ASSERT_BOOL(SymbolTag2String.find(tag) != SymbolTag2String.end());
    return SymbolTag2String.at(tag);
}

const std::string TS_SYMBOL_ERROR = "UNKNOWN ERROR";
struct symbolItem
{
    lex::pMetaInfo      symbolInfo;

    // symbol name: x
    std::string         name;

    // symbol type: Int
    pTypeTree           pTT;

    // symbol value: 6
    llvm::AllocaInst*   value;

    // symbol tag: DefaultSymbol / FunctionScope / ClassScope / IfScope / ElseScope / WhileScope / LambdaScope
    SymbolTag           tag;

    // random track value
    unsigned            ran;

    symbolItem(std::string name, pTypeTree pTT, llvm::AllocaInst *value, SymbolTag tag)
    {
        this->name = name;
        this->pTT = pTT;
        this->value = value;
        this->tag = tag;
        this->ran = rand();
    }

    bool isSymbol() { return ST_DEFAULT_SYMBOL == tag; }
    bool isFnTag() { return ST_FUNCTION_SCOPE == tag; }
    bool isClassTag() { return ST_CLASS_SCOPE == tag; }
    bool isThenTag() { return ST_THEN_SCOPE == tag; }
    bool isElseTag() { return ST_ELSE_SCOPE == tag; }
    bool isWhileTag() { return ST_WHILE_SCOPE == tag; }
    bool isCaseOfTag() { return ST_CASE_OF_SCOPE == tag; }

    std::string getSymbolName()
    {
        return name;
    }

    pTypeTree getpTT()
    {
        return pTT;
    }

    llvm::Type* getType()
    {
        return pTT->getType();
    }

    llvm::AllocaInst *getValue()
    {
        return value;
    }

    SymbolTag getTag()
    {
        return tag;
    }

    unsigned getRan()
    {
        return ran;
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

    static void print();
    static void initSymbolTable();
    static bool add_symbol(std::string name, pTypeTree pTT, llvm::AllocaInst * value, SymbolTag tag);
    static bool add_symbol_tag(SymbolTag tag);
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

void symbolTable::print()
{
    comm::Log::singleton(INFO) >> "##########PRINT SYMBOL TABL START##########" >> comm::newline;
    for (const auto& item: table)
    {
        comm::Log::singleton(INFO) >> item->getSymbolName() >> ", " >> getStrForSymbolTag(item->getTag()) >> ", " >> item->getRan() >> comm::newline;
    }
    comm::Log::singleton(INFO) >> "##########PRINT SYMBOL TABL END##########" >> comm::newline;
}

void symbolTable::initSymbolTable()
{
    table.clear();
}

bool symbolTable::add_symbol(std::string name, pTypeTree pTT, llvm::AllocaInst * value, SymbolTag tag)
{
    JHIN_ASSERT_BOOL(find_symbol_in_scope(name) == nullptr);
    table.push_back(std::make_shared<symbolItem>(name, pTT, value, tag));
    return true;
}

bool symbolTable::add_symbol_tag(SymbolTag tag)
{
    table.push_back(std::make_shared<symbolItem>("", nullptr, nullptr, tag));
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

    for (int idx = table.size() - 1; idx >= 0 && table[idx]->isSymbol(); idx--) {
        table.pop_back();
        cnt += 1;
    }

    // pop back symbol tag
    table.pop_back();


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
    for (int idx = table.size() - 1; idx >= 0 && table[idx]->isSymbol() ; idx--) {
        if (table[idx]->getSymbolName() == symbolName) {
            return table[idx];
        }
    }
    return nullptr;
}

std::shared_ptr<symbolItem> symbolTable::find_symbol_in_fn(const std::string& symbolName)
{
    for (int idx = table.size() - 1; idx >= 0 && !table[idx]->isFnTag() ; idx--) {
        if (table[idx]->getSymbolName() == symbolName) {
            return table[idx];
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<symbolItem>> symbolTable::get_symbols_in_scope()
{
    std::vector<std::shared_ptr<symbolItem>> ans;
    for (int idx = table.size() - 1; idx >= 0 && table[idx]->isSymbol() ; idx--) {
        ans.push_back(table[idx]);
    }
    std::reverse(ans.begin(), ans.end());
    return ans;
}

std::shared_ptr<symbolItem> symbolTable::get_last_symbol()
{
    JHIN_ASSERT_BOOL(!table.empty());
    int idx = table.size() - 1;
    JHIN_ASSERT_BOOL(table[idx]->isSymbol());
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
    for (idx = symbolTable::table.size() - 1; idx >= 0 && !symbolTable::table[idx]->isFnTag() ; idx--) {}
    if (idx < 0) return nullptr;
    JHIN_ASSERT_BOOL(idx >= 1);
    return symbolTable::table[idx - 1];
}

}   /* namespace st */
}   /* namespace jhin */

