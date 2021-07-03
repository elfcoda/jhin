#pragma once

#include <unordered_map>
#include <string>
#include "../../comm/log.h"

namespace jhin
{
namespace ast
{

#define AST_NON_LEAF_START  1024

/* SYMBOL */

// enum EASTNonLeaf
// {
//     AST_NON_LEAF_ASSIGN = AST_NON_LEAF_START,
//     AST_NON_LEAF_WHILE,
//     AST_NON_LEAF_IF,
//     AST_NON_LEAF_IF_ELSE,
//     AST_NON_LEAF_CLASS,
//     AST_NON_LEAF_CLASS_INHERITS,
//     AST_NON_LEAF_FUNCTION,
//     AST_NON_LEAF_IS_VOID,
//     AST_NON_LEAF_NOT,
//     AST_NON_LEAF_NEW,
//
//     AST_NON_LEAF_PLUS,
//     AST_NON_LEAF_MINUS,
//     AST_NON_LEAF_STAR,
//     AST_NON_LEAF_SLASH,
//
//     AST_NON_LEAF_EQ,
//     AST_NON_LEAF_LT,
//     AST_NON_LEAF_LE,
//     AST_NON_LEAF_GT,
//     AST_NON_LEAF_GE,
//
//     AST_NON_LEAF_LAMBDA,
//     AST_NON_LEAF_FN_CALL,
//     AST_NON_LEAF_OBJECT_FN_CALL,
//     AST_NON_LEAF_OBJECT_FN_CALL_DISPATCH,
//     AST_NON_LEAF_EXP_OBJECT_FN_CALL,
//     AST_NON_LEAF_EXP_OBJECT_FN_CALL_DISPATCH,
//
//     AST_NON_LEAF_LET_IN,
//     AST_NON_LEAF_RETURN,
//     AST_NON_LEAF_CASE_OF_OTHERWISE,
//
//     AST_NON_LEAF_DCL,
// };


/* readable text -> AstSymbolId */
std::unordered_map<std::string, unsigned> mASTNonLeafStr2Id = {};
std::unordered_map<unsigned, std::string> mASTNonLeafId2Str = {};

unsigned getNonLeafId(const std::string& text)
{
    static unsigned u_NonLeafId = AST_NON_LEAF_START;
    if (mASTNonLeafStr2Id.find(text) != mASTNonLeafStr2Id.end()) {
        return mASTNonLeafStr2Id.at(text);
    }

    mASTNonLeafStr2Id[text] = u_NonLeafId++;
    mASTNonLeafId2Str[u_NonLeafId - 1] = text;
    return u_NonLeafId - 1;
}

std::string getNonLeafStr(unsigned id)
{
    if (mASTNonLeafId2Str.find(id) == mASTNonLeafId2Str.end()) {
        comm::Log::singleton(ERROR) >> "error id: " >> id >> comm::newline;
        return "";
    }

    return mASTNonLeafId2Str.at(id);
}


}   /* namsspace ast */
}   /* namespace jhin */

