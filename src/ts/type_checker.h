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
        JHIN_ASSERT_STR("should be implemented first");
        return nullptr;
    }

    /* @pArgsTree: fn inputs */
    pTypeTree checkFn(pTypeTree pArgsTree, pTypeTree fnType)
    {
        JHIN_ASSERT_STR("should be implemented first");
        return nullptr;
    }

    /* plus: + */
    pTypeTree checkPlus(pTypeTree t1, pTypeTree t2)
    {
        auto e1 = t1->getEST(), e2 = t2->getEST();
        JHIN_ASSERT_BOOL(isCaclType(e1) && isCaclType(e2));
        JHIN_ASSERT_BOOL(e1 == e2);
        pTypeTree pTT = new TypeTree(e1, nullptr, "", "", "", nullptr);

        return pTT;
    }

    /* minus: - */
    pTypeTree checkMinus(pTypeTree t1, pTypeTree t2)
    {
        auto e1 = t1->getEST(), e2 = t2->getEST();
        JHIN_ASSERT_BOOL(isCaclType(e1) && isCaclType(e2));
        JHIN_ASSERT_BOOL(e1 == e2);
        pTypeTree pTT = new TypeTree(e1, nullptr, "", "", "", nullptr);

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
        JHIN_ASSERT_BOOL(isTypeEqual(t1, t2));
        pTypeTree pTT = new TypeTree(SYMBOL_TYPE_BOOL, nullptr, "", "", "", nullptr);

        return pTT;
    }

    void checkOrderHelper(pTypeTree t1, pTypeTree t2)
    {
        auto e1 = t1->getEST(), e2 = t2->getEST();
        JHIN_ASSERT_BOOL(isCaclType(e1) && isCaclType(e2));
        JHIN_ASSERT_BOOL(e1 == e2);
    }

    /* order:
     *  >, >=, <, <=
     **/
    pTypeTree checkOrder(pTypeTree t1, pTypeTree t2)
    {
        checkOrderHelper(t1, t2);
        pTypeTree pTT = new TypeTree(SYMBOL_TYPE_BOOL, nullptr, "", "", "", nullptr);
        return pTT;
    }

    pTypeTree checkAssign(pTypeTree t1, pTypeTree t2)
    {
        JHIN_ASSERT_STR("should be implemented first");
        return nullptr;
    }

    /* not: ! */
    pTypeTree checkNot(pTypeTree t)
    {
        JHIN_ASSERT_BOOL(t->getEST() == SYMBOL_TYPE_BOOL);
        pTypeTree pTT = new TypeTree(SYMBOL_TYPE_BOOL, nullptr, "", "", "", nullptr);
        return pTT;
    }

    /* isVoid for expand type */
    pTypeTree checkIsVoid(pTypeTree t)
    {
        JHIN_ASSERT_STR("should be implemented first");

        pTypeTree pTT = new TypeTree(SYMBOL_TYPE_BOOL, nullptr, "", "", "", nullptr);
        return pTT;
    }

    /* cat: Cat <- new Cat */
    pTypeTree checkNew(pTypeTree t)
    {
        JHIN_ASSERT_STR("should be implemented first");
        return nullptr;
    }

    /* return */
    pTypeTree checkReturn(pTypeTree t)
    {
        JHIN_ASSERT_STR("should be implemented first");
        return nullptr;
    }

}   /* namespace ts */
}   /* namespace jhin */

