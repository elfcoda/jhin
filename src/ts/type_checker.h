#pragma once

#include <string>
#include <unordered_set>
#include "type_tree.h"
#include "symbol_table.h"

namespace jhin
{
namespace ts
{

    /* type checker would not calculate the result */
    pTypeTree checkDecl(pTypeTree pTree)
    {
        pTypeTree pTT = nullptr;
        // type check
        if (getSymbolType(pTree) == E_ID_TYPE_TRIVIAL_TYPE) {
            assert(trivialTypes.find(pTree->getValue()) != trivialTypes.end());
            pTT = new TypeTree(trivialTypes.at(pTree->getValue()), "", "", nullptr);
        } else if (getSymbolType(pTree) == E_ID_TYPE_EXPAND_TYPE) {
            std::shared_ptr<symbolItem> si = symbolTable::find_symbol(pTree->getValue());
            assert(si != nullptr);
            pTT = new TypeTree(SYMBOL_TYPE_CLASS,
                               "",
                               "",
                               si->type->getChildrenList(),
                               pTree->getValue());
        } else if (getSymbolType(pTree) == E_ID_TYPE_TYPE_LITERAL) {
            pTT = new TypeTree(SYMBOL_TYPE_TYPE, "", "", nullptr);
        } else {
            assert(!"symbol should be a type");
        }

        return pTT;
    }

    /* plus: + */
    pTypeTree checkPlus(pTypeTree t1, pTypeTree t2)
    {
        assert(getSymbolType(t1) == E_ID_TYPE_TRIVIAL_VALUE &&
               getSymbolType(t2) == E_ID_TYPE_TRIVIAL_VALUE);
        assert((isCaclType(t1->getType()) || t1->getType() == SYMBOL_TYPE_STRING) &&
               (isCaclType(t2->getType()) || t2->getType() == SYMBOL_TYPE_STRING));
        assert(t1->getType() == t2->getType());
        pTypeTree pTT = new TypeTree(t1->getType(), "", "", nullptr);

        return pTT;
    }

    /* minus: - */
    pTypeTree checkMinus(pTypeTree t1, pTypeTree t2)
    {
        assert(getSymbolType(t1) == E_ID_TYPE_TRIVIAL_VALUE &&
               getSymbolType(t2) == E_ID_TYPE_TRIVIAL_VALUE);
        assert(isCaclType(t1->getType()) && isCaclType(t2->getType()));
        assert(t1->getType() == t2->getType());
        pTypeTree pTT = new TypeTree(t1->getType(), "", "", nullptr);

        return pTT;
    }

    /* multiply: *
     **/
    pTypeTree checkStar(pTypeTree t1, pTypeTree t2)
    {
        return checkMinus(t1, t2);
    }

    /* divide: / */
    pTypeTree checkSlash(pTypeTree t1, pTypeTree t2)
    {
        return checkMinus(t1, t2);
    }


    /* equality: ==, != */
    pTypeTree checkEquality(pTypeTree t1, pTypeTree t2)
    {
        assert(getSymbolType(t1) == getSymbolType(t2));
        if (getSymbolType(t1) == E_ID_TYPE_TRIVIAL_VALUE) {
            assert(t1->getType() == t2->getType());
        } else if (getSymbolType(t1) == E_ID_TYPE_EXPAND_VALUE) {
            assert(t1->getType() == SYMBOL_TYPE_CLASS && t1->getExpandType() == t2->getExpandType());
        } else {
            assert(false);
        }
        pTypeTree pTT = new TypeTree(SYMBOL_TYPE_BOOL, "", "", nullptr);

        return pTT;
    }

    void checkOrderHelper(pTypeTree t1, pTypeTree t2)
    {
        assert(getSymbolType(t1) == getSymbolType(t2));
        if (getSymbolType(t1) == E_ID_TYPE_TRIVIAL_VALUE) {
            assert(t1->getType() == t2->getType());
        } else {
            assert(false);
        }
    }

    /* order:
     *  >, >=, <, <=
     **/
    pTypeTree checkOrder(pTypeTree t1, pTypeTree t2)
    {
        checkOrderHelper(t1, t2);
        pTypeTree pTT = new TypeTree(SYMBOL_TYPE_BOOL, "", "", nullptr);
        return pTT;
    }

    /* not: ! */
    pTypeTree checkNot(pTypeTree t)
    {
        assert(t->getType() == SYMBOL_TYPE_BOOL);
        pTypeTree pTT = new TypeTree(SYMBOL_TYPE_BOOL, "", "", nullptr);
        return pTT;
    }

    /* isVoid for expand type */
    pTypeTree checkIsVoid(pTypeTree t)
    {
        assert(getSymbolType(t) == E_ID_TYPE_EXPAND_VALUE);
        pTypeTree pTT = new TypeTree(SYMBOL_TYPE_BOOL, "", "", nullptr);
        return pTT;
    }

    /* cat: Cat <- new Cat */
    pTypeTree checkNew(pTypeTree t)
    {
        assert(getSymbolType(t) == E_ID_TYPE_EXPAND_TYPE);
        auto ptr = symbolTable::find_symbol(t->getValue())->type->children;
        pTypeTree pTT = new TypeTree(SYMBOL_TYPE_CLASS, "", "", ptr, t->getValue());
        return pTT;
    }

    /* return */
    pTypeTree checkReturn(pTypeTree t)
    {
        pTypeTree pTT = nullptr;
        if (getSymbolType(t) == E_ID_TYPE_TRIVIAL_VALUE) {
            pTT = new TypeTree(t->getType(), "", "", nullptr);
        } else if (getSymbolType(t) == E_ID_TYPE_EXPAND_VALUE) {
            pTT = new TypeTree(t->getType(), "", "", t->children, t->getExpandType());
        } else {
            assert(false);
        }

        return pTT;
    }

};  /* namespace ts */
};  /* namespace jhin */

