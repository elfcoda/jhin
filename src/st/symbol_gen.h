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
#include "../../comm/algorithm.h"
#include "../../comm/container_op.h"

namespace jhin
{
namespace st
{

/* vector as stack
 * to check if every return expression is compatible with current function's return type,
 * so a stack is used to track current function stack.
 * stack is replaced by vector in this case.
 *
 * @pTypeTree: function type.
 * */

// deprecated
// std::vector<comm::pTypeTree> vFnRetStack = {};

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
    /*
     * valur in symbol table:
     * literal: 7    ->  | Int  | None | 7   | nullptr |
     * literal: m1   ->  | Int  | m1   | 7   | nullptr |
     *
     * literal: Int  ->  | Type | None | Int | nullptr |
     * literal: m2   ->  | Type | m2   | Int | nullptr |
     *
     * literal: Cat  ->  | Type | None | Cat | nullptr |
     * literal: m3   ->  | Type | m3   | Cat | nullptr |
     *
     * literal: m4   ->  | CLS  | m4   | uk  | ptr     | Cat    // instance of Class
     *
     * literal: m5   ->  | SUM  | m5   | uk  | ptr     |
     *
     * literal: m6   ->  | PDT  | m6   | uk  | ptr     |
     *
     * literal: Type ->  | Type | None | Type| nullptr |
     *
     * declaration in symbol table:
     * CLASS        ->   | CLS  | Cat  | uk  | ptr     | // only for search
     * FUNCTION     ->   | FN   | func | uk  | ptr     |
     * */
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
            comm::Log::singleton(DEBUG) >> "text: " >> text >> ", " >> "astSymId: " >> astSymId >> ", " >> "childrenNumber: " >> childrenNumber >> comm::newline;
            assert(text != "" && AST_DEFAULT_TEXT != "");

            if (comm::isASTSymbolLeaf(astSymId)) {
                /* leaf node, no declarations and no blocks */
                return handleLeaf(pRoot);
            } else if (comm::isASTSymbolNonLeaf(astSymId)) {
                /* non-leaf node */
                if (text == AST_DEFAULT_TEXT) {
                    pTypeTree pTT = nullptr;
                    for (int idx = 0; idx <  pRoot->size(); idx++) {
                        ast::pASTNode child = pRoot->getChild(idx);
                        pTT = genSymbolTable(child)->getTypeTreePtr();
                        if (getSymbolType(pTT) == E_ID_TYPE_FN_TYPE) {
                            if (comm::symbolId2String(pRoot->getSymbolId()) == "ExpN") {
                                /* or we can use ASTNode mark field to recognize the existance of Args. */
                                /* this is a fuction with parmeter(s) */
                                assert(childrenNumber == 2);
                                pTypeTree pArgsTree = genFnTypes(pRoot->getChild(1));
                                return std::make_shared<symbolGenRtn>(ts::checkFn(pArgsTree, pTT), cgen::genFnCall(pTT->getSymbolName(), pArgsTree));
                            } else {
                                // 如果无参函数返回函数类型又炸了，所以这里不作处理
                                // 况且无参函数在处理叶子节点的时候，都已经处理完了，这里本就不该处理，应该直接跳过
                                /* this is a fuction without parmeter(s) */
                                // pTypeTree pArgsTree = makeFnTree();
                                // appendTrivial(pArgsTree, SYMBOL_TYPE_UNIT);
                                // pTypeTree fnRet = checkFn(pArgsTree, pTT);
                                // if (idx == pRoot->size() - 1) return fnRet;
                            }

                        }
                    }
                    /* if this symbol is not function, AST_DEFAULT_TEXT Node is used to connect different expressions/cmds/declarations.
                     * and we want to return last line of code as default return value of this function, we just return the last return
                     * type of this non-terminal tree node, pTT, in this case.
                     * */
                    return std::make_shared<symbolGenRtn>(pTT);
                } else if (text == "class") {   /* block and declaration */
                    handleClass(pRoot, false);
                    return std::make_shared<symbolGenRtn>(nullptr);
                } else if (text == "class_inherits") {
                    handleClass(pRoot, true);
                    return std::make_shared<symbolGenRtn>(nullptr);
                } else if (text == "def") {
                    return handleFn(pRoot);
                } else if (text == "while") {   /* block, command */
                    symbolTable::add_symbol_mark(SYMBOL_MARK_WHILE);
                    std::shared_ptr<symbolGenRtn> rtn1 = genSymbolTable(pRoot->getChild(0));
                    std::shared_ptr<symbolGenRtn> rtn2 = genSymbolTable(pRoot->getChild(1));
                    symbolTable::pop_symbol_block();
                    return std::make_shared<symbolGenRtn>(nullptr, cgen::genWhile(pRoot, rtn1, rtn2));
                } else if (text == "if") {
                    symbolTable::add_symbol_mark(SYMBOL_MARK_IF);
                    std::shared_ptr<symbolGenRtn> rtn1 = genSymbolTable(pRoot->getChild(0));
                    std::shared_ptr<symbolGenRtn> rtn2 = genSymbolTable(pRoot->getChild(1));
                    symbolTable::pop_symbol_block();
                    return std::make_shared<symbolGenRtn>(nullptr, cgen::genIf(pRoot, rtn1, rtn2));
                } else if (text == "if_else") {
                    symbolTable::add_symbol_mark(SYMBOL_MARK_IF);
                    std::shared_ptr<symbolGenRtn> rtn1 = genSymbolTable(pRoot->getChild(0));
                    std::shared_ptr<symbolGenRtn> rtn2 = genSymbolTable(pRoot->getChild(1));
                    symbolTable::pop_symbol_block();

                    symbolTable::add_symbol_mark(SYMBOL_MARK_ELSE);
                    genSymbolTable(pRoot->getChild(0));
                    std::shared_ptr<symbolGenRtn> rtn3 = genSymbolTable(pRoot->getChild(2));
                    symbolTable::pop_symbol_block();

                    return std::make_shared<symbolGenRtn>(nullptr, cgen::genIfElse(pRoot, rtn1, rtn2, rtn3));
                } else if (text.length() >= 4 && text.substr(0, 4) == "case") {
                    // symbolTable::add_symbol_mark(SYMBOL_MARK_CASE_OF);
                    assert(!"case to be implemented");
                } else if (text.length() >= 6 && text.substr(0, 6) == "lambda") {
                    // symbolTable::add_symbol_mark(SYMBOL_MARK_LAMBDA);
                    assert(!"lambda to be implemented");
                } else if (text.length() >= 3 && text.substr(0, 3) == "let") {
                    // symbolTable::add_symbol_mark(SYMBOL_MARK_LET_IN);
                    assert(!"let...in... to be implemented");
                } else if (text == ":") {     /* declaration */
                    /* id: type: default_value */
                    /* 统一把算法改成从符号表取相关符号，而不是把每个声明都搞成函数参数 */
                    if (childrenNumber == 1) {
                        /* function return type */
                        return genSymbolTable(pRoot->getChild(0));
                    } else if (childrenNumber == 2) {
                        handleDecl(pRoot);
                        return std::make_shared<symbolGenRtn>(nullptr);
                    } else if (childrenNumber == 3) {
                        handleDecl(pRoot);
                        /* recurse */
                        genSymbolTable(pRoot->getChild(2));
                        return std::make_shared<symbolGenRtn>(nullptr);
                    } else {
                        assert(false);
                    }
                } else if (text == ":_<-") {
                    if (childrenNumber == 3) {
                        handleDeclAssign(pRoot);
                        return std::make_shared<symbolGenRtn>(nullptr);
                    } else if (childrenNumber == 4) {
                        handleDeclAssign(pRoot);
                        genSymbolTable(pRoot->getChild(3));
                        return std::make_shared<symbolGenRtn>(nullptr);
                    } else {
                        assert(false);
                    }
                } else if (text == "->") {  // TODO: function type
                } else if (isDoubleArgs(text)) {
                    assert(childrenNumber == 2);
                    std::shared_ptr<symbolGenRtn> rtn1 = genSymbolTable(pRoot->getChild(0));
                    std::shared_ptr<symbolGenRtn> rtn2 = genSymbolTable(pRoot->getChild(1));
                    return handleDoubleOps(rtn1, rtn2, text);
                }
                else if (isSingleArg(text)) {
                    assert(childrenNumber == 1);
                    std::shared_ptr<symbolGenRtn> rtn1 = genSymbolTable(pRoot->getChild(0));
                    return handleSingleOp(rtn1, text);
                }
                else {
                    /* undefined op */
                    comm::Log::singleton(ERROR) >> "text is: " >> text >> comm::newline;
                    assert(!"undefined op");
                }
            } else {
                assert(!"symbol should be leaf or non-leaf");
            }

            return std::make_shared<symbolGenRtn>(nullptr);
        }


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
            assert(pArgs != nullptr);
            pTypeTree pTT = new TypeTree(SYMBOL_TYPE_FN, "", "", nullptr);
            if (pArgs->getText() == AST_DEFAULT_TEXT) {
                unsigned childrenNumber = pArgs->size();
                assert(childrenNumber == 2);
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

        pTypeTree getFnLastType(pTypeTree pTT)
        {
            assert(getSymbolType(pTT) == E_ID_TYPE_FN_TYPE);
            unsigned childrenNum = pTT->size();
            assert(childrenNum >= 2);
            return pTT->getChild(childrenNum - 1);
        }

    private:
        std::shared_ptr<symbolGenRtn> handleLeaf(ast::pASTNode pRoot)
        {
            unsigned astSymId = pRoot->getAstSymbolId();
            std::string text = pRoot->getText();
            switch (astSymId)
            {
                /* may be symbol or function name */
                case ast::AST_LEAF_RE_ID:
                    {
                        std::shared_ptr<symbolItem> symbol = symbolTable::find_symbol(text);
                        if (symbol == nullptr) {
                            comm::Log::singleton(ERROR) >> "symbol '" >> text >> "' not found in this context!" >> comm::newline;
                            assert(!"symbol not found!");
                        } else {
                            EIDType EType = getSymbolType(symbol->type);
                            bool isFn = pRoot->isFunction();
                            comm::Log::singleton(DEBUG) >> comm::getPtrString(pRoot) >> comm::newline;
                            /* check function's use type and declaration type be same */
                            if (isFn && EType != E_ID_TYPE_FN_TYPE) assert(!"symbol not function type.");
                            else if (!isFn && EType == E_ID_TYPE_FN_TYPE) assert(!"function should be called.");

                            if (EType == E_ID_TYPE_FN_TYPE && !pRoot->fnHasArgs()) {
                                /* 无参直接计算返回值好不好嘛 */
                                pTypeTree pArgsTree = makeFnTree();
                                appendTrivial(pArgsTree, SYMBOL_TYPE_UNIT);
                                pTypeTree fnRet = ts::checkFn(pArgsTree, symbol->type);
                                return std::make_shared<symbolGenRtn>(fnRet, cgen::genFnCall(text, nullptr));
                            }

                            if (isFn) {
                                /* 这里也有可能返回函数类型哦><，在有函数参数的情况下 */
                                return std::make_shared<symbolGenRtn>(symbol->type, "");
                            } else {
                                /* 不是函数，只是普通符号 */
                                return std::make_shared<symbolGenRtn>(symbol->type, cgen::genBySymbol(symbol->type));
                            }
                        }
                    }
                /* class name */
                case ast::AST_LEAF_RE_VALUE:
                    {
                        std::shared_ptr<symbolItem> symbol = symbolTable::find_symbol(text);
                        if (symbol == nullptr) {
                            comm::Log::singleton(ERROR) >> "symbol '" >> text >> "' not found in this context!" >> comm::newline;
                            assert(!"symbol not found!");
                        } else {
                            return std::make_shared<symbolGenRtn>(makeTrivial(SYMBOL_TYPE_TYPE, text), "");
                        }
                    }
                case ast::AST_LEAF_RE_INT:
                    {
                        return std::make_shared<symbolGenRtn>(makeTrivial(SYMBOL_TYPE_INT, text), cgen::genREInt(text));
                    }
                case ast::AST_LEAF_RE_DECIMAL:
                    {
                        return std::make_shared<symbolGenRtn>(makeTrivial(SYMBOL_TYPE_FLOAT, text), cgen::genDecimal(text));    // TODO: decimal as float or double
                    }
                case ast::AST_LEAF_RE_STRING:
                    {
                        return std::make_shared<symbolGenRtn>(makeTrivial(SYMBOL_TYPE_STRING, text), cgen::genREString(text));
                    }
                case ast::AST_LEAF_TRUE:
                    {
                        return std::make_shared<symbolGenRtn>(makeTrivial(SYMBOL_TYPE_BOOL, text), cgen::genBool(text));
                    }
                case ast::AST_LEAF_FALSE:
                    {
                        return std::make_shared<symbolGenRtn>(makeTrivial(SYMBOL_TYPE_BOOL, text), cgen::genBool(text));
                    }

                case ast::AST_LEAF_OBJECT:
                case ast::AST_LEAF_BOOL:
                case ast::AST_LEAF_INT:
                case ast::AST_LEAF_FLOAT:
                case ast::AST_LEAF_DOUBLE:
                case ast::AST_LEAF_LONG:
                case ast::AST_LEAF_STRING:
                case ast::AST_LEAF_UNIT:         return std::make_shared<symbolGenRtn>(makeTrivial(SYMBOL_TYPE_TYPE, text), "");
                case ast::AST_LEAF_THIS:
                    {
                        // TODO
                        assert(!"have not implemented yet!");
                    }
                case ast::AST_LEAF_TYPE:
                    {
                        return std::make_shared<symbolGenRtn>(makeTrivial(SYMBOL_TYPE_TYPE, text), "");
                    }
                default:
                    {
                        assert(!"Type Error!");
                    }
            }

            return std::make_shared<symbolGenRtn>(nullptr, "");
        }


        std::shared_ptr<symbolGenRtn> handleDoubleOps(std::shared_ptr<symbolGenRtn> rnt1, std::shared_ptr<symbolGenRtn> rnt2, const std::string& text)
        {
            assert(rnt1 != nullptr && rnt2 != nullptr);
            pTypeTree pTT1 = rnt1->getTypeTreePtr(), pTT2 = rnt2->getTypeTreePtr();
            std::string asmCode1 = rnt1->getAsmCode(), asmCode2 = rnt2->getAsmCode();
            assert(pTT1 != nullptr);
            assert(pTT2 != nullptr);
            pTypeTree pTT = nullptr;
            std::string asmCode = "";
            if (text == "+") {
                pTT = ts::checkPlus(pTT1, pTT2);
                asmCode = cgen::genPlus(asmCode1, asmCode2);
            } else if (text == "-") {
                pTT = ts::checkMinus(pTT1, pTT2);
                asmCode = cgen::genMinus(asmCode1, asmCode2);
            } else if (text == "*") {
                pTT = ts::checkStar(pTT1, pTT2);
                asmCode = cgen::genStar(asmCode1, asmCode2);
            } else if (text == "/") {
                pTT = ts::checkSlash(pTT1, pTT2);
                asmCode = cgen::genSlash(asmCode1, asmCode2);
            } else if (text == "==") {
                pTT = ts::checkEquality(pTT1, pTT2);
                asmCode = cgen::genCompare(asmCode1, asmCode2);
            } else if (text == ">") {
                pTT = ts::checkOrder(pTT1, pTT2);
                asmCode = cgen::genCompare(asmCode1, asmCode2);
            } else if (text == ">=") {
                pTT = ts::checkOrder(pTT1, pTT2);
                asmCode = cgen::genCompare(asmCode1, asmCode2);
            } else if (text == "<") {
                pTT = ts::checkOrder(pTT1, pTT2);
                asmCode = cgen::genCompare(asmCode1, asmCode2);
            } else if (text == "<=") {
                pTT = ts::checkOrder(pTT1, pTT2);
                asmCode = cgen::genCompare(asmCode1, asmCode2);
            } else if (text == "!=") {
                pTT = ts::checkEquality(pTT1, pTT2);
                asmCode = cgen::genCompare(asmCode1, asmCode2);
            } else if (text == "<-") {
                pTT = ts::checkAssign(pTT1, pTT2);
                /* find the stack address of symbol */
                asmCode = cgen::genAssign(pTT1->getSymbolName(), asmCode2);
            } else {
                assert(!"unknown symbol");
            }

            return std::make_shared<symbolGenRtn>(pTT, asmCode);
        }

        std::shared_ptr<symbolGenRtn> handleSingleOp(std::shared_ptr<symbolGenRtn> rtn1, const std::string& text)
        {
            assert(rtn1 != nullptr);
            pTypeTree pTT1 = rtn1->getTypeTreePtr();
            assert(pTT1 != nullptr);
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
                if (stFnDeclNumber.empty()) assert(!"symbol \"return\" should be appear in functions.");
                // pTypeTree topTypeTree = stackTop(stFnDeclNumber)->getTypeTreePtr();
                pTypeTree topTypeTree = stackTop(stFnDeclNumber).first;
                pTypeTree pLastType = getFnLastType(topTypeTree);
                assert(isTypeEqual(pTT, pLastType));
                comm::Log::singleton(DEBUG) >> __LINE__ >> ", return check: " >> getPtrString(pRet) >> ", Fn Type: " >> getPtrString(pLastType) >> comm::newline;
            } else {
                assert(!"unknown symbol");
            }

            return std::make_shared<symbolGenRtn>(pTT, asmCode);
        }

        std::shared_ptr<symbolGenRtn> handleFn(ast::pASTNode pRoot)
        {
            // TODO: check return type

            pTypeTree fnType = makeFnTree();
            std::string fnCode = "";
            fnType->setSymbolName(pRoot->getChild(0)->getText());


            // stackPush(vFnRetStack, fnType);
            unsigned stFnArgsSize = stFnDeclNumber.size();
            unsigned symbolIdx = 0;
            if (stFnArgsSize != 0) {
                symbolIdx = stFnDeclNumber[stFnArgsSize-1].second;
                stFnDeclNumber[stFnArgsSize-1].second += 1;
            }
            // TODO set value?
            symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, fnType, symbolIdx);


            stFnDeclNumber.push_back(std::make_pair(fnType, 0));
            pTypeTree fnBodyType = nullptr;
            /* generate the signature of function */
            symbolTable::add_symbol_mark(SYMBOL_MARK_FN);
            for (int idx = 1; idx < pRoot->size(); idx++) {
                auto pSymGenRtn = genSymbolTable(pRoot->getChild(idx));
                assert(pSymGenRtn != nullptr);
                pTypeTree pTp = pSymGenRtn->getTypeTreePtr();
                fnCode += pSymGenRtn->getAsmCode();
                comm::Log::singleton(DEBUG) >> pRoot->getChild(0)->getText() >> ": " >> comm::getPtrString(pTp) >> comm::newline;
                /* notation: symbol and non-terminal */
                std::string notationStr = pRoot->getNotationStr(idx);
                if (notationStr == "FnArg") {
                    assert(pTp == nullptr);
                    auto vTable = symbolTable::get_symbols_in_scope();
                    symbolTable::unionSymbolItems2Tree(fnType, vTable);
                    assert(fnType->hasChildren());
                    appendTrivial(fnType, SYMBOL_TYPE_UNIT);
                } else if (notationStr == "FnRetTp") {
                    assert(pTp != nullptr);
                    if (!fnType->hasChildren()) {
                        appendTrivial(fnType, SYMBOL_TYPE_UNIT);
                    } else {
                        /* pop redundant Unit symbol */
                        popChild(fnType);
                    }
                    /* fn: check function's return type:
                     * return type is a type, should be switch to value.
                     * for this define: "def function(i: Int): Int",
                     * the return type is pTypeTree(Type, "", "Int", nullptr, "")
                     * turn to pTypeTree(Int, "", "", nullptr, "").
                     * */
                    if (getSymbolType(pTp) == E_ID_TYPE_TRIVIAL_TYPE) {
                        pTp->setType(getTrivialTypeByStr(pTp->getValue()));
                        pTp->setValue("");
                    } else if (getSymbolType(pTp) == E_ID_TYPE_EXPAND_TYPE) {
                        pTp->setType(SYMBOL_TYPE_CLASS);
                        pTp->children = symbolTable::find_symbol(pTp->getValue())->type->children;
                        pTp->setExpandType(pTp->getValue());
                    } else if (getSymbolType(pTp) == E_ID_TYPE_TYPE_LITERAL) {
                        assert(!"can not return 'Type'.");
                    } else {
                        assert(!"return type error.");
                    }

                    appendTree(fnType, static_cast<pConstTypeTree>(pTp));
                } else {
                    /* function body */
                    fnBodyType = pTp;
                    if (fnBodyType == nullptr) {
                        fnBodyType = makeTrivial(SYMBOL_TYPE_UNIT);
                    }
                }
            }
            if (!fnType->hasChildren()) {
                /* neither args and ret types is defined */
                /* input */
                appendTrivial(fnType, SYMBOL_TYPE_UNIT);
                /* output */
                appendTrivial(fnType, SYMBOL_TYPE_UNIT);
            }

            /* check function return type of the last expression. */
            assert(isTypeEqual(getFnLastType(fnType), fnBodyType));

            // stackPop(vFnRetStack);
            unsigned declNum = stFnDeclNumber[stFnArgsSize-1].second;
            stFnDeclNumber.pop_back();
            std::string sDeclRemain = cgen::genDeclRemainSpace(declNum);
            symbolTable::pop_symbol_block();

            std::string sFnHead = "_" + fnType->getSymbolName() + ":\n";
            std::string sFnTail = cgen::genFnTail();

            return std::make_shared<symbolGenRtn>(nullptr, sFnHead + sDeclRemain + fnCode + sFnTail);
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
        }

    public:
        /* function name, number of declarations
         */
        static std::vector<std::pair<pTypeTree, unsigned>> stFnDeclNumber;
};
std::vector<std::pair<pTypeTree, unsigned>> SymbolGen::stFnDeclNumber;

};  /* namespace st */
};  /* namespace jhin */

