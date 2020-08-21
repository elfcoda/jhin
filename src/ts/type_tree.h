#pragma once

#include "../../comm/tree.h"

namespace jhin
{
namespace ts
{

/* types:
 * basic type:      Int
 * function type:   Int -> Int -> Bool
 * sum type:        (Int, Bool)
 * product type:    (Int, Bool)
 * */

#define SYMBOL_TYPE_START_NONE      0
#define SYMBOL_TYPE_START_TRIVIAL   1
#define SYMBOL_TYPE_START_COMPLEX   1024

enum ESymbolType
{
    /* None */
    SYMBOL_TYPE_NO_TYPE = SYMBOL_TYPE_START_NONE,

    /* trivial type */
    SYMBOL_TYPE_INT = SYMBOL_TYPE_START_TRIVIAL,
    SYMBOL_TYPE_FLOAT,
    SYMBOL_TYPE_DOUBLE,
    SYMBOL_TYPE_LONG,
    SYMBOL_TYPE_OBJECT,
    SYMBOL_TYPE_BOOL,
    SYMBOL_TYPE_STRING,
    SYMBOL_TYPE_UNIT,
    SYMBOL_TYPE_TYPE,       /* type variable */

    /* complex recursive type */
    /* function type */
    SYMBOL_TYPE_FN = SYMBOL_TYPE_START_COMPLEX,         /* vector */
    /* class */
    SYMBOL_TYPE_CLASS,    /* vector */
    /* Sum */
    SYMBOL_TYPE_SUM,        /* vector */
    /* Product */
    SYMBOL_TYPE_PRODUCT,    /* vector */
};
/* eg: Int -> Int -> Float */
using FnType = std::vector<ESymbolType>;
using Type = std::vector<ESymbolType>;

bool isNoneType(ESymbolType type)
{
    return type == SYMBOL_TYPE_START_NONE;
}

bool isTrivialType(ESymbolType type)
{
    return type >= SYMBOL_TYPE_START_TRIVIAL && type < SYMBOL_TYPE_START_COMPLEX;
}

bool isComplexType(ESymbolType type)
{
    return type >= SYMBOL_TYPE_START_COMPLEX;
}

const std::string type2DefaultValue_ERR = "error";
const std::unordered_map<ESymbolType, std::string> type2DefaultValue = {
    /* None, can not presense in program */
    {SYMBOL_TYPE_NO_TYPE, ""},

    /* trivial types */
    {SYMBOL_TYPE_INT, "0"},
    {SYMBOL_TYPE_FLOAT, "0.0"},
    {SYMBOL_TYPE_DOUBLE, "0.0"},
    {SYMBOL_TYPE_LONG, "0"},
    {SYMBOL_TYPE_OBJECT, ""},
    {SYMBOL_TYPE_BOOL, "False"},
    {SYMBOL_TYPE_STRING, "\"\""},
    {SYMBOL_TYPE_UNIT, "unit"},
    {SYMBOL_TYPE_TYPE, ""},

    /* complex type */
    {SYMBOL_TYPE_FN, ""},
    {SYMBOL_TYPE_CLASS, ""},
    {SYMBOL_TYPE_SUM, ""},
    {SYMBOL_TYPE_PRODUCT, ""}
};

std::string getDefaultValueByType(ESymbolType type)
{
    if (type2DefaultValue.find(type) == type2DefaultValue.end()) return type2DefaultValue_ERR;
    return type2DefaultValue.at(type);
}

/* basic types include Int/Long/.../Type, which have no children, aka.leaf nodes
 * while other types(sum type/function type etc) have children field, aka. non-leaf nodes
 * */
class TypeTree: public comm::tree<TypeTree>
using pTypeTree = TypeTree*;

class TypeTree: public comm::tree<TypeTree>
{
    public:
        /* children: std::vector<pTypeTree>* */
        TypeTree(ESymbolType tp, const std::string& symbolName, const std::string& value, pChildrenList<TypeTree> pChildren):
                 comm::tree<TypeTree>(pChildren), type(tp), symbolName(symbolName), value(value)
        {
        }

        ESymbolType getSymbolType() { return type; }

        void free()
        {
            if (children != nullptr) {
                delete children;
                children = nullptr;
            }
        }

        ~TypeTree() { free(); }

        tree(tree&& t)
        {
            this->children = t.children;
            t.children = nullptr;
        }

        TypeTree& operator +(const TypeTree& tt)
        {
            mergeVec2Vec(*children, *(tt.children));
        }

        ESymbolType getType() const { return type; }
        std::string getSymbolName() const { return symbolName; }
        std::string getValue() const { return value; }

        void setType(ESymbolType st) { type = st; }
        void setSymbolName(const std::string& sm) { symbolName = sm; }
        void setValue(const std::string& val) { value = val; }
    private:
        ESymbolType type;

        std::string symbolName;
        std::string value;
        // chlidren
};

void mergeTypeTree(pTypeTree p1, pTypeTree p2)
{
    assert(p1 != nullptr && p2 != nullptr);
    if (p1->children == nullptr) {
        p1->children = new std::vector<pTypeTree>();
    }
    if (!p2->hasChildren()) {
        return;
    }
    comm::mergeVec2Vec(*(p1->children), *(p2->children));
}

pTypeTree makeFnTree()
{
    pTypeTree pTT = new TypeTree(SYMBOL_TYPE_FN, "", "", nullptr);
    return pTT;
}

pTypeTree makeTrivial(ESymbolType type = SYMBOL_TYPE_UNIT)
{
    pTypeTree pTT = new TypeTree(type, "", getDefaultValueByType(type), nullptr);
    return pTT;
}

bool setTrivial(pTypeTree pTT, ESymbolType type = SYMBOL_TYPE_UNIT)
{
    assert(pTT != nullptr);
    if (pTT->children == nullptr) {
        pTT->children = new std::vector<pTypeTree>{makeTrivial(type)};
        return true;
    }
    pTT->children->resize(1);
    *(pTT->children)[0] = makeTrivial(type);
    return true;
}

bool appendTrivial(pTypeTree pTT, ESymbolType type = SYMBOL_TYPE_UNIT)
{
    assert(pTT != nullptr);
    if (pTT->children == nullptr) {
        pTT->children = new std::vector<pTypeTree>{makeTrivial(type)};
        return true;
    }
    pTT->children->push_back(makeTrivial(type));
    return true;
}

bool popChild(pTypeTree pTT)
{
    assert(pTT != nullptr && pTT->hasChildren());
    pTT->children->pop_back();
    return true
}


};  /* namespace ts */
};  /* namespace jhin */

