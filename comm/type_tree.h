/* data structure of types. used to generate symbol table, type check and code gen. */

#pragma once

#include <unordered_set>
#include "llvm/IR/Type.h"
#include "container_op.h"
#include "tree.h"
#include "jhin_assert.h"

namespace jhin
{
namespace comm
{

/* types:
 * basic type:      Int
 * function type:   Int -> Int -> Bool
 * sum type:        (Int, Bool)
 * product type:    (Int, Bool)
 * */

#define SYMBOL_TYPE_START_NONE      0
#define SYMBOL_TYPE_START_TRIVIAL   1
#define SYMBOL_TYPE_START_TYPE      512
#define SYMBOL_TYPE_START_COMPLEX   1024

enum ESymbolType
{
    /* None */
    SYMBOL_TYPE_NO_TYPE = SYMBOL_TYPE_START_NONE,

    /* trivial type */
    SYMBOL_TYPE_INT = SYMBOL_TYPE_START_TRIVIAL,
    SYMBOL_TYPE_FLOAT,
    SYMBOL_TYPE_DOUBLE, // TODO
    SYMBOL_TYPE_LONG,   // TODO
    SYMBOL_TYPE_OBJECT,
    SYMBOL_TYPE_BOOL,
    SYMBOL_TYPE_STRING,
    SYMBOL_TYPE_UNIT,

    SYMBOL_TYPE_TYPE = SYMBOL_TYPE_START_TYPE,       /* type variable */

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

const std::unordered_map<ESymbolType, std::string> ESymbolType2String = {
    {SYMBOL_TYPE_NO_TYPE, "SYMBOL_TYPE_NO_TYPE"},

    {SYMBOL_TYPE_INT, "SYMBOL_TYPE_INT"},
    {SYMBOL_TYPE_FLOAT, "SYMBOL_TYPE_FLOAT"},
    {SYMBOL_TYPE_DOUBLE, "SYMBOL_TYPE_DOUBLE"},
    {SYMBOL_TYPE_LONG, "SYMBOL_TYPE_LONG"},
    {SYMBOL_TYPE_OBJECT, "SYMBOL_TYPE_OBJECT"},
    {SYMBOL_TYPE_BOOL, "SYMBOL_TYPE_BOOL"},
    {SYMBOL_TYPE_STRING, "SYMBOL_TYPE_STRING"},
    {SYMBOL_TYPE_UNIT, "SYMBOL_TYPE_UNIT"},

    {SYMBOL_TYPE_TYPE, "SYMBOL_TYPE_TYPE"},

    {SYMBOL_TYPE_FN, "SYMBOL_TYPE_FN"},
    {SYMBOL_TYPE_CLASS, "SYMBOL_TYPE_CLASS"},
    {SYMBOL_TYPE_SUM, "SYMBOL_TYPE_SUM"},
    {SYMBOL_TYPE_PRODUCT, "SYMBOL_TYPE_PRODUCT"},
};

std::string getESymbolType2String(ESymbolType e)
{
    JHIN_ASSERT_BOOL(ESymbolType2String.find(e) != ESymbolType2String.end());
    return ESymbolType2String.at(e);
}


/* eg: Int -> Int -> Float */
using FnType = std::vector<ESymbolType>;
using JType = std::vector<ESymbolType>;

const std::unordered_set<ESymbolType> calcType = {
    SYMBOL_TYPE_INT,
    SYMBOL_TYPE_FLOAT,
    SYMBOL_TYPE_DOUBLE,
    SYMBOL_TYPE_LONG
};

bool isCaclType(ESymbolType est)
{
    return calcType.find(est) != calcType.end();
}

bool isNoneType(ESymbolType type)
{
    return type == SYMBOL_TYPE_START_NONE;
}

bool isTrivialType(ESymbolType type)
{
    return type >= SYMBOL_TYPE_START_TRIVIAL && type < SYMBOL_TYPE_START_TYPE;
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
class TypeTree;
using pTypeTree = TypeTree*;
using pConstTypeTree = const TypeTree*;

class TypeTree: public comm::tree<TypeTree>
{
    public:
        /* children: std::vector<pTypeTree>* */
        TypeTree(ESymbolType est, Type* type, const std::string& symbolName, const std::string& value, const std::string& et, pChildrenList<TypeTree> pChildren):
                 comm::tree<TypeTree>(pChildren), est(est), type(type), symbolName(symbolName), value(value), expandType(et)
        {
        }

        void free()
        {
            if (children != nullptr) {
                delete children;
                children = nullptr;
            }
        }

        ~TypeTree() { free(); }

        bool isFP()
        {
            return est == SYMBOL_TYPE_FLOAT || est == SYMBOL_TYPE_DOUBLE;
        }

        ESymbolType getEST() const { return est; }
        Type* getType() const { return type; }
        std::string getSymbolName() const { return symbolName; }
        std::string getValue() const { return value; }
        std::string getExpandType() const { return expandType; }

        void setEST(ESymbolType e) { est = e; }
        void setType(Type* tp) { type = tp; }
        void setSymbolName(const std::string& sm) { symbolName = sm; }
        void setValue(const std::string& val) { value = val; }
        void setExpandType(const std::string& et) { expandType = et; }

        std::string toString() const
        {
            std::string s = "";
            s += "{est: " + getESymbolType2String(est) + ", symbolName: " \
                 + symbolName + ", value: " + value + ", expandType: " + expandType;
            if (!hasChildren()) {
                s += ", children: empty}\n";
            } else {
                s += ", children: \n";
                for (pTypeTree child: *children) {
                    s += child->toString();
                }
                s += "}\n";
            }

            return s;
        }
    private:
        ESymbolType est;
        Type* type;

        /* type don't have tsymbolName field */
        std::string symbolName;
        std::string value;

        /* class name or function name, null if it's basic types */
        std::string expandType;
};


pTypeTree makeFnTree()
{
    pTypeTree pTT = new TypeTree(SYMBOL_TYPE_FN, nullptr, "", "", "", nullptr);
    return pTT;
}

pTypeTree makeTrivial(ESymbolType est = SYMBOL_TYPE_UNIT, std::string value = "")
{
    if (value == "") value = getDefaultValueByType(est);
    pTypeTree pTT = new TypeTree(est, nullptr, "", value, "", nullptr);
    return pTT;
}

bool setTrivial2Children(pTypeTree pTT, ESymbolType est = SYMBOL_TYPE_UNIT)
{
    JHIN_ASSERT_BOOL(pTT != nullptr);
    if (pTT->children == nullptr) {
        pTT->children = new std::vector<pTypeTree>();
    }
    pTT->children->resize(1);
    (*(pTT->children))[0] = makeTrivial(est);
    return true;
}

bool appendTrivial2Children(pTypeTree pTT, ESymbolType est = SYMBOL_TYPE_UNIT)
{
    JHIN_ASSERT_BOOL(pTT != nullptr);
    if (pTT->children == nullptr) {
        pTT->children = new std::vector<pTypeTree>();
    }
    pTT->children->push_back(makeTrivial(est));
    return true;
}

bool appendTree2Children(pTypeTree pTT, const pConstTypeTree child)
{
    JHIN_ASSERT_BOOL(pTT != nullptr && child != nullptr);
    if (pTT->children == nullptr) {
        pTT->children = new std::vector<pTypeTree>();
    }
    pTT->children->push_back(const_cast<pTypeTree>(child));
    return true;
}

void mergeChildren(pTypeTree p1, const pConstTypeTree& p2)
{
    JHIN_ASSERT_BOOL(p1 != nullptr && p2 != nullptr);
    if (p1->children == nullptr) {
        p1->children = new std::vector<pTypeTree>();
    }
    if (!p2->hasChildren()) {
        return;
    }
    comm::mergeVec2Vec_P(p1->children, p2->children);
}

bool popChild(pTypeTree pTT)
{
    JHIN_ASSERT_BOOL(pTT != nullptr && pTT->hasChildren());
    pTT->children->pop_back();
    return true;
}

/* what kind of value */
const std::unordered_map<std::string, ESymbolType> trivialTypes = {
    {"Object", SYMBOL_TYPE_OBJECT}, {"Bool", SYMBOL_TYPE_BOOL}, {"Int", SYMBOL_TYPE_INT},
    {"Float", SYMBOL_TYPE_FLOAT}, {"Double", SYMBOL_TYPE_DOUBLE},
    {"Long", SYMBOL_TYPE_LONG}, {"String", SYMBOL_TYPE_STRING}, {"Unit", SYMBOL_TYPE_UNIT}
};

ESymbolType getTrivialTypeByStr(const std::string& str)
{
    JHIN_ASSERT_BOOL(trivialTypes.find(str) != trivialTypes.end());
    return trivialTypes.at(str);
}

/**/
enum EIDType
{
    /* value of trivlal type */
    E_ID_TYPE_ERROR = 0,

    E_ID_TYPE_TRIVIAL_VALUE,
    E_ID_TYPE_EXPAND_VALUE,
    E_ID_TYPE_TRIVIAL_TYPE,
    E_ID_TYPE_EXPAND_TYPE,
    E_ID_TYPE_FN_TYPE,
    E_ID_TYPE_TYPE_LITERAL,
};
EIDType getSymbolType(const pTypeTree& pTT)
{
    if (pTT == nullptr) return E_ID_TYPE_ERROR;
    ESymbolType tp = pTT->getType();
    if (isTrivialType(tp)) return E_ID_TYPE_TRIVIAL_VALUE;
    else if (tp == SYMBOL_TYPE_CLASS && pTT->getExpandType() != "") return E_ID_TYPE_EXPAND_VALUE;
    else if (tp == SYMBOL_TYPE_TYPE && trivialTypes.find(pTT->getValue()) != trivialTypes.end()) return E_ID_TYPE_TRIVIAL_TYPE;
    else if (tp == SYMBOL_TYPE_TYPE && trivialTypes.find(pTT->getValue()) == trivialTypes.end()) return E_ID_TYPE_EXPAND_TYPE;
    else if (tp == SYMBOL_TYPE_FN) return E_ID_TYPE_FN_TYPE;
    else if (tp == SYMBOL_TYPE_TYPE && pTT->getValue() == "Type") return E_ID_TYPE_TYPE_LITERAL;

    JHIN_ASSERT_BOOL(false);
    return E_ID_TYPE_ERROR;
}

bool isTypeEqual(pTypeTree t1, pTypeTree t2)
{
    EIDType e1 = getSymbolType(t1), e2 = getSymbolType(t2);
    if (t1->getType() != t2->getType()) return false;
    if (e1 == E_ID_TYPE_TRIVIAL_VALUE) {
        return true;
    } else if (e1 == E_ID_TYPE_EXPAND_VALUE) {
        if ((t1->getExpandType() != "") && (t1->getExpandType() == t2->getExpandType())) return true;
        else JHIN_ASSERT_STR("expand type error.");
    } else if (e1 == E_ID_TYPE_TRIVIAL_TYPE) {
        /* for literal "Type" is stored as | Type | None | Type| nullptr | in symbol table.
         * so we should make sure that t2's value field is not Type if we want to make
         * type checker happy.
         * we should just make their type field is all SYMBOL_TYPE_TYPE
         * */
        if (e2 == E_ID_TYPE_TYPE_LITERAL) JHIN_ASSERT_STR("type error, can not be Type literal.");
        else return true;
    } else if (e1 == E_ID_TYPE_EXPAND_TYPE) {
        /* same as above */
        if (e2 == E_ID_TYPE_TYPE_LITERAL) JHIN_ASSERT_STR("type error, can not be Type literal.");
        else return true;
    } else if (e1 == E_ID_TYPE_FN_TYPE) {
        /* fn type */
        if (!t1->hasChildren() || !t2->hasChildren()) JHIN_ASSERT_STR("fn type should hsas children.");
        pChildrenList<TypeTree> c1 = t1->children, c2 = t2->children;
        /* both have children list */
        unsigned n1 = c1->size(), n2 = c2->size();
        if (n1 != n2) JHIN_ASSERT_STR("fn types error.");
        for (unsigned idx = 0; idx < n1; idx++) {
            if (!isTypeEqual((*c1)[idx], (*c2)[idx])) JHIN_ASSERT_STR("fn type error.");
        }
        return true;
    } else {
        /* not type, assert */
        JHIN_ASSERT_STR("type error.");
    }

    return false;
}

unsigned fnArgsNumber(pTypeTree pTT)
{
    JHIN_ASSERT_BOOL(pTT != nullptr);
    JHIN_ASSERT_BOOL(getSymbolType(pTT) == E_ID_TYPE_FN_TYPE);
    unsigned n = pTT->size();
    JHIN_ASSERT_BOOL(n >= 2);
    if (pTT->getChild(0)->getType() == SYMBOL_TYPE_UNIT) {
        return 0;
    }
    return n - 1;
}


}   /* namespace comm */
}   /* namespace jhin */

