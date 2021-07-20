#pragma once

#include <string>
#include <vector>
#include <memory>
#include "symbol_def.h"
#include "symbol_table.h"
#include "../cgen/code_gen.h"
#include "../ts/type_checker.h"
#include "../ast/ast_node.h"
#include "../../comm/type_tree.h"
#include "../../comm/log.h"
#include "../../comm/jhin_assert.h"
#include "../../comm/algorithm.h"
#include "../../comm/container_op.h"

namespace jhin
{
namespace st
{


/* check which kind of op current string is. */
const std::unordered_set<std::string> doubleArgs = {"+", "-", "*", "/", "==", ">", ">=", "<", "<=", "!=", "<-"};
const std::unordered_set<std::string> singleArg = {"!", "isVoid", "new", "return"};
bool isDoubleArgs(const std::string& text)
{
    return doubleArgs.find(text) != doubleArgs.end();
}

bool isSingleArg(const std::string& text)
{
    return singleArg.find(text) != singleArg.end();
}

class SymbolGen
{

    public:
        SymbolGen()
        {
            symbolTable::initSymbolTable();
        }

        /* gen symbol table, type checker */
        std::shared_ptr<symbolGenRtn> genSymbolTable(ast::pASTNode pRoot)
        {
            std::string text = pRoot->getText();
            unsigned astSymId = pRoot->getAstSymbolId();
            unsigned childrenNumber = pRoot->size();

                return handleLeaf(pRoot);
                
                /* non-leaf node */
                if (text == AST_DEFAULT_TEXT) {
                    pTypeTree pTT = nullptr;
                    for (unsigned idx = 0; idx <  pRoot->size(); idx++) {
                        ast::pASTNode child = pRoot->getChild(idx);
                        pTT = genSymbolTable(child)->getTypeTreePtr();
                        if (getSymbolType(pTT) == E_ID_TYPE_FN_TYPE) {
                            if (comm::symbolId2String(pRoot->getSymbolId()) == "ExpN") {
                                /* or we can use ASTNode mark field to recognize the existance of Args. */
                                /* this is a fuction with parmeter(s) */
                                JHIN_ASSERT_BOOL(childrenNumber == 2);
                                pTypeTree pArgsTree = genFnTypes(pRoot->getChild(1));
                                return std::make_shared<symbolGenRtn>(ts::checkFn(pArgsTree, pTT), cgen::genFnCall(pTT->getSymbolName(), pArgsTree));

                    return std::make_shared<symbolGenRtn>(nullptr, cgen::genIfElse(pRoot, rtn1, rtn2, rtn3));
                } else if (isDoubleArgs(text)) {
                    JHIN_ASSERT_BOOL(childrenNumber == 2);
                    std::shared_ptr<symbolGenRtn> rtn1 = genSymbolTable(pRoot->getChild(0));
                    std::shared_ptr<symbolGenRtn> rtn2 = genSymbolTable(pRoot->getChild(1));
                    return handleDoubleOps(rtn1, rtn2, text);
                }
                else if (isSingleArg(text)) {
                    JHIN_ASSERT_BOOL(childrenNumber == 1);
                    std::shared_ptr<symbolGenRtn> rtn1 = genSymbolTable(pRoot->getChild(0));
                    return handleSingleOp(rtn1, text);


    private:
        pTypeTree genFnTypes(ast::pASTNode pArgs)
        {
            /* for this tree:
             *
             *      \
             *      AST
             *     /   \
             *   Arg1  AST
             *         /  \
             *       Arg2 Arg3
             *
             *  genFnTypes return pTypeTree whose children is [Arg1, Arg2, Arg3]
             **/
            JHIN_ASSERT_BOOL(pArgs != nullptr);
            pTypeTree pTT = new TypeTree(SYMBOL_TYPE_FN, "", "", nullptr);
            if (pArgs->getText() == AST_DEFAULT_TEXT) {
                unsigned childrenNumber = pArgs->size();
                JHIN_ASSERT_BOOL(childrenNumber == 2);
                pTypeTree singleArg = genSymbolTable(pArgs->getChild(0))->getTypeTreePtr();
                appendTree(pTT, singleArg);
                pTypeTree pSub = genFnTypes(pArgs->getChild(1));
                mergeTree(pTT, pSub);
            } else {
                /* single argument */
                pTypeTree singleArg = genSymbolTable(pArgs)->getTypeTreePtr();
                appendTree(pTT, singleArg);
            }

            return pTT;
        }

        std::shared_ptr<symbolGenRtn> handleSingleOp(std::shared_ptr<symbolGenRtn> rtn1, const std::string& text)
        {
            JHIN_ASSERT_BOOL(rtn1 != nullptr);
            pTypeTree pTT1 = rtn1->getTypeTreePtr();
            JHIN_ASSERT_BOOL(pTT1 != nullptr);
            pTypeTree pTT = nullptr;
            std::string asmCode = "";

            if (text == "!") {
                pTT = ts::checkNot(pTT1);
                // asmCode = cgen::genNot();
            } else if (text == "isVoid") {
                pTT = ts::checkIsVoid(pTT1);
            } else if (text == "new") {
                pTT = ts::checkNew(pTT1);
            } else if (text == "return") {
                pTT = ts::checkReturn(pTT1);
                /* check if return type is compatilbe with function's signature. */
                if (stFnDeclNumber.empty()) JHIN_ASSERT_STR("symbol \"return\" should be appear in functions.");
                // pTypeTree topTypeTree = stackTop(stFnDeclNumber)->getTypeTreePtr();
                pTypeTree topTypeTree = stackTop(stFnDeclNumber).first;
                pTypeTree pLastType = getFnLastType(topTypeTree);
                JHIN_ASSERT_BOOL(isTypeEqual(pTT, pLastType));
                comm::Log::singleton(DEBUG) >> __LINE__ >> ", return check: " >> getPtrString(topTypeTree) >> ", Fn Type: " >> getPtrString(pLastType) >> comm::newline;
            } else {
                JHIN_ASSERT_STR("unknown symbol");
            }

            return std::make_shared<symbolGenRtn>(pTT, asmCode);
        }

        void handleClass(ast::pASTNode pRoot, bool isInherits)
        {
            pTypeTree pTT = new TypeTree(SYMBOL_TYPE_CLASS,
                                         pRoot->getChild(0)->getText(),
                                         "",    // Todo
                                         new std::vector<pTypeTree>());
            symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, pTT, 0);

            /* 1. add mark */
            symbolTable::add_symbol_mark(SYMBOL_MARK_CLASS);
            /* 2. parse sub-tree */
            if (isInherits) genSymbolTable(pRoot->getChild(2));
            else genSymbolTable(pRoot->getChild(1));
            /* update pTT */
            std::vector<std::shared_ptr<symbolItem>> vTable = symbolTable::get_symbols_in_scope();
            symbolTable::unionSymbolItems2Tree(pTT, vTable);
            /* 3. pop symbols */
            symbolTable::pop_symbol_block();
        }

        void handleCMD(ast::pASTNode pRoot, ESymbolMark mark)
        {
            symbolTable::add_symbol_mark(mark);
            genSymbolTable(pRoot->getChild(0));
            genSymbolTable(pRoot->getChild(1));
            symbolTable::pop_symbol_block();
        }

        void handleDecl(ast::pASTNode pRoot)
        {
            unsigned stFnArgsSize = stFnDeclNumber.size();
            unsigned symbolIdx = 0;
            if (stFnArgsSize != 0) {
                symbolIdx = stFnDeclNumber[stFnArgsSize-1].second;
                stFnDeclNumber[stFnArgsSize-1].second += 1;
            }

            pTypeTree pNode = genSymbolTable(pRoot->getChild(1))->getTypeTreePtr();
            pTypeTree pTT = ts::checkDecl(pNode);
            pTT->setSymbolName(pRoot->getChild(0)->getText());
            symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, pTT, symbolIdx);
            std::string genStr = cgen::genDecl(pTT->getSymbolName(), pTT->getValue());
        }

        void handleDeclAssign(ast::pASTNode pRoot)
        {
            unsigned stFnArgsSize = stFnDeclNumber.size();
            unsigned symbolIdx = 0;
            if (stFnArgsSize != 0) {
                symbolIdx = stFnDeclNumber[stFnArgsSize-1].second;
                stFnDeclNumber[stFnArgsSize-1].second += 1;
            }

            pTypeTree pNode = genSymbolTable(pRoot->getChild(1))->getTypeTreePtr();
            pTypeTree pTT = ts::checkDecl(pNode);
            pTT->setSymbolName(pRoot->getChild(0)->getText());
            /* initial value */
            pTypeTree pValue = genSymbolTable(pRoot->getChild(2))->getTypeTreePtr();
            pTT->setValue(pValue->getValue());
            /* add to symbol table */
            symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, pTT, symbolIdx);
            std::string genStr = cgen::genDecl(pTT->getSymbolName(), pTT->getValue());
        
};

}   /* namespace st */
}   /* namespace jhin */

