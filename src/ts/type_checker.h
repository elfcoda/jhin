#pragma once

#include <string>
#include <unordered_set>
#include "../../comm/type_tree.h"
#include "../../comm/log.h"
#include "../../comm/jhin_assert.h"

namespace jhin
{
namespace ts
{
    using namespace jhin::comm;

    /* type checker would not calculate the result */
    pTypeTree checkDecl(pTypeTree pTree)
    {
        pTypeTree pTT = nullptr;
        // type check
        if (getSymbolType(pTree) == E_ID_TYPE_TRIVIAL_TYPE) {
            JHIN_ASSERT_BOOL(trivialTypes.find(pTree->getValue()) != trivialTypes.end());
            pTT = new TypeTree(trivialTypes.at(pTree->getValue()), "", "", nullptr);
        } else if (getSymbolType(pTree) == E_ID_TYPE_EXPAND_TYPE) {
            std::shared_ptr<st::symbolItem> si = st::symbolTable::find_symbol(pTree->getValue());
            JHIN_ASSERT_BOOL(si != nullptr);
            pTT = new TypeTree(SYMBOL_TYPE_CLASS,
                               "",
                               "",
                               si->type->getChildrenList(),
                               pTree->getValue());
        } else if (getSymbolType(pTree) == E_ID_TYPE_TYPE_LITERAL) {
            pTT = new TypeTree(SYMBOL_TYPE_TYPE, "", "", nullptr);
        } else {
            JHIN_ASSERT_STR("symbol should be a type");
        }

        return pTT;
    }

    /* @pArgsTree: fn inputs */
    pTypeTree checkFn(pTypeTree pArgsTree, pTypeTree fnType)
    {
        int argSize = fnType->size();
        JHIN_ASSERT_BOOL(argSize >= 2);
        pTypeTree pLastType = fnType->getChild(argSize - 1);
        /* return value type */
        appendTree(pArgsTree, pLastType);
        int inputSize = pArgsTree->size();
        /* TODO: default argument, curry */
        JHIN_ASSERT_BOOL(inputSize == argSize);

        bool fnTypeEqual = isTypeEqual(pArgsTree, fnType);
        JHIN_ASSERT_BOOL(fnTypeEqual);

        return pLastType;
    }

    /* plus: + */
    pTypeTree checkPlus(pTypeTree t1, pTypeTree t2)
    {
        JHIN_ASSERT_BOOL(getSymbolType(t1) == E_ID_TYPE_TRIVIAL_VALUE &&
                         getSymbolType(t2) == E_ID_TYPE_TRIVIAL_VALUE);
        JHIN_ASSERT_BOOL((isCaclType(t1->getType()) || t1->getType() == SYMBOL_TYPE_STRING) &&
                         (isCaclType(t2->getType()) || t2->getType() == SYMBOL_TYPE_STRING));
        JHIN_ASSERT_BOOL(t1->getType() == t2->getType());
        pTypeTree pTT = new TypeTree(t1->getType(), "", "", nullptr);

        return pTT;
    }

    /* minus: - */
    pTypeTree checkMinus(pTypeTree t1, pTypeTree t2)
    {
        JHIN_ASSERT_BOOL(getSymbolType(t1) == E_ID_TYPE_TRIVIAL_VALUE &&
                         getSymbolType(t2) == E_ID_TYPE_TRIVIAL_VALUE);
        JHIN_ASSERT_BOOL(isCaclType(t1->getType()) && isCaclType(t2->getType()));
        JHIN_ASSERT_BOOL(t1->getType() == t2->getType());
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
        JHIN_ASSERT_BOOL(getSymbolType(t1) == getSymbolType(t2));
        if (getSymbolType(t1) == E_ID_TYPE_TRIVIAL_VALUE) {
            JHIN_ASSERT_BOOL(t1->getType() == t2->getType());
        } else if (getSymbolType(t1) == E_ID_TYPE_EXPAND_VALUE) {
            JHIN_ASSERT_BOOL(t1->getType() == SYMBOL_TYPE_CLASS && t1->getExpandType() == t2->getExpandType());
        } else {
            JHIN_ASSERT_BOOL(false);
        }
        pTypeTree pTT = new TypeTree(SYMBOL_TYPE_BOOL, "", "", nullptr);

        return pTT;
    }

    void checkOrderHelper(pTypeTree t1, pTypeTree t2)
    {
        JHIN_ASSERT_BOOL(getSymbolType(t1) == getSymbolType(t2));
        if (getSymbolType(t1) == E_ID_TYPE_TRIVIAL_VALUE) {
            JHIN_ASSERT_BOOL(t1->getType() == t2->getType());
        } else {
            JHIN_ASSERT_BOOL(false);
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

    pTypeTree checkAssign(pTypeTree t1, pTypeTree t2)
    {
        EIDType e1 = getSymbolType(t1), e2 = getSymbolType(t2);
        JHIN_ASSERT_BOOL(t1->getSymbolName() != "");
        JHIN_ASSERT_BOOL(e1 == e2);
        if (e1 == E_ID_TYPE_TRIVIAL_VALUE) {
            JHIN_ASSERT_BOOL(t1->getType() == t2->getType());
        } else if (e1 == E_ID_TYPE_EXPAND_VALUE) {
            JHIN_ASSERT_BOOL(t1->getExpandType() == t2->getExpandType());
        } else {
            JHIN_ASSERT_STR("type error.");
        }
        return nullptr;
    }

    /* not: ! */
    pTypeTree checkNot(pTypeTree t)
    {
        JHIN_ASSERT_BOOL(t->getType() == SYMBOL_TYPE_BOOL);
        pTypeTree pTT = new TypeTree(SYMBOL_TYPE_BOOL, "", "", nullptr);
        return pTT;
    }

    /* isVoid for expand type */
    pTypeTree checkIsVoid(pTypeTree t)
    {
        JHIN_ASSERT_BOOL(getSymbolType(t) == E_ID_TYPE_EXPAND_VALUE);
        pTypeTree pTT = new TypeTree(SYMBOL_TYPE_BOOL, "", "", nullptr);
        return pTT;
    }

    /* cat: Cat <- new Cat */
    pTypeTree checkNew(pTypeTree t)
    {
        JHIN_ASSERT_BOOL(getSymbolType(t) == E_ID_TYPE_EXPAND_TYPE);
        auto ptr = st::symbolTable::find_symbol(t->getValue())->type->children;
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
            JHIN_ASSERT_BOOL(false);
        }

        return pTT;
    }

};  /* namespace ts */
};  /* namespace jhin */

