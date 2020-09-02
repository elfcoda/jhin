#pragma once

namespace jhin
{
namespace cgen
{

#include <string>
#include "../ast/ast_node.h"
#include "../../comm/log.h"
#include "../../comm/type_tree.h"
#include "../../comm/symbol_table.h"

class CGen
{
    public:
        CGen()
        {
            symbolTable::initSymbolTable();
        }

        std::string codeGen(ast::pASTNode pNode)
        {
            std::string text = pNode->getText();
            unsigned astSymId = pNode->getAstSymbolId();
            unsigned childrenNumber = pNode->size();
            comm::Log::singleton(DEBUG) >> "text: " >> text >> ", " >> "astSymId: " >> astSymId >> ", " >> "childrenNumber: " >> childrenNumber >> comm::newline;
            assert(text != "" && AST_DEFAULT_TEXT != "");

            if (comm::isASTSymbolLeaf(astSymId)) {
                /* leaf node, no declarations and no blocks */
                return handleLeaf(pNode);
            } else if (comm::isASTSymbolNonLeaf(astSymId)) {
                /* non-leaf node */
                if (text == AST_DEFAULT_TEXT) {
                    std::string asmCode = "";
                    for (int idx = 0; idx <  pNode->size(); idx++) {
                        ast::pASTNode child = pNode->getChild(idx);
                        asmCode = codeGen(child);
                        if (getSymbolType(asmCode) == E_ID_TYPE_FN_TYPE) {
                            if (comm::symbolId2String(pNode->getSymbolId()) == "ExpN") {
                                /* or we can use ASTNode mark field to recognize the existance of Args. */
                                /* this is a fuction with parmeter(s) */
                                assert(childrenNumber == 2);
                                std::string argsStr = genFnTypes(pNode->getChild(1));
                                return genFn(pArgsTree, asmCode);
                            }
                        }
                    }
                    /* if this symbol is not function, AST_DEFAULT_TEXT Node is used to connect different expressions/cmds/declarations.
                     * and we want to return last line of code as default return value of this function, we just return the last return
                     * type of this non-terminal tree node, asmCode, in this case.
                     * */
                    return asmCode;
                } else if (text == "class") {   /* block and declaration */
                    handleClass(pNode, false);
                    return "";
                } else if (text == "class_inherits") {
                    handleClass(pNode, true);
                    return "";
                } else if (text == "def") {
                    handleFn(pNode);
                    return "";
                } else if (text == "while") {   /* block, command */
                    handleCMD(pNode, SYMBOL_MARK_WHILE);
                    return "";
                } else if (text == "if") {
                    handleCMD(pNode, SYMBOL_MARK_IF);
                    return "";
                } else if (text == "if_else") {
                    handleCMD(pNode, SYMBOL_MARK_IF);

                    symbolTable::add_symbol_mark(SYMBOL_MARK_ELSE);
                    codeGen(pNode->getChild(0));
                    codeGen(pNode->getChild(2));
                    symbolTable::pop_symbol_block();

                    return "";
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
                        return codeGen(pNode->getChild(0));
                    } else if (childrenNumber == 2) {
                        handleDecl(pNode);
                        return "";
                    } else if (childrenNumber == 3) {
                        handleDecl(pNode);
                        /* recurse */
                        codeGen(pNode->getChild(2));
                        return "";
                    } else {
                        assert(false);
                    }
                } else if (text == ":_<-") {
                    if (childrenNumber == 3) {
                        handleDeclAssign(pNode);
                        return "";
                    } else if (childrenNumber == 4) {
                        handleDeclAssign(pNode);
                        codeGen(pNode->getChild(3));
                        return "";
                    } else {
                        assert(false);
                    }
                } else if (text == "->") {  // TODO: function type
                } else if (isDoubleArgs(text)) {
                    assert(childrenNumber == 2);
                    std::string asmCode1 = codeGen(pNode->getChild(0));
                    std::string asmCode2 = codeGen(pNode->getChild(1));
                    return handleDoubleOps(asmCode1, asmCode2, text);
                } else if (isSingleArg(text)) {
                    assert(childrenNumber == 1);
                    std::string asmCode = codeGen(pNode->getChild(0));
                    return handleSingleOp(asmCode, text);
                }
                else {
                    /* undefined op */
                    comm::Log::singleton(ERROR) >> "text is: " >> text >> comm::newline;
                    assert(!"undefined op");
                }
            } else {
                assert(!"symbol should be leaf or non-leaf");
            }

            return "";
        }

    private:
        void handleDeclAssign(ast::pASTNode pRoot)
        {
            unsigned stFnArgsSize = stFnDeclNumber.size();
            if (stFnArgsSize != 0) { stFnDeclNumber[stFnArgsSize-1].second += 1; }
            std::string pNode = codeGen(pRoot->getChild(1));
            std::string asmCode = checkDecl(pNode);
            asmCode->setSymbolName(pRoot->getChild(0)->getText());
            /* initial value */
            std::string pValue = codeGen(pRoot->getChild(2));
            asmCode->setValue(pValue->getValue());
            /* add to symbol table */
            symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, asmCode);
        }

        void handleDecl(ast::pASTNode pRoot)
        {
            unsigned stFnArgsSize = stFnDeclNumber.size();
            if (stFnArgsSize != 0) { stFnDeclNumber[stFnArgsSize-1].second += 1; }
            std::string pNode = codeGen(pRoot->getChild(1));
            std::string asmCode = checkDecl(pNode);
            asmCode->setSymbolName(pRoot->getChild(0)->getText());
            symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, asmCode);
        }

        void handleCMD(ast::pASTNode pRoot, ESymbolMark mark)
        {
            symbolTable::add_symbol_mark(mark);
            codeGen(pRoot->getChild(0));
            codeGen(pRoot->getChild(1));
            symbolTable::pop_symbol_block();
        }

        void handleFn(ast::pASTNode pRoot)
        {
            // TODO: check return type

            pTypeTree fnType = makeFnTree();
            fnType->setSymbolName(pRoot->getChild(0)->getText());
            // TODO set value?
            symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, fnType);


            unsigned stFnArgsSize = stFnDeclNumber.size();
            if (stFnArgsSize != 0) { stFnDeclNumber[stFnArgsSize-1].second += 1; }
            stFnDeclNumber.push_back(std::make_pair(fnType->getSymbolName(), 0));
            std::string fnBodyType = nullptr;
            /* generate the signature of function */
            symbolTable::add_symbol_mark(SYMBOL_MARK_FN);
            for (int idx = 1; idx < pRoot->size(); idx++) {
                std::string pTp = codeGen(pRoot->getChild(idx));
                comm::Log::singleton(DEBUG) >> pRoot->getChild(0)->getText() >> ": " >> comm::getPtrString(pTp) >> comm::newline;
                /* notation: symbol and non-terminal */
                std::string notationStr = getNotationStr(pRoot, idx);
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
                     * the return type is std::string(Type, "", "Int", nullptr, "")
                     * turn to std::string(Int, "", "", nullptr, "").
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

            unsigned declNum = stFnDeclNumber[stFnArgsSize-1].second;
            stFnDeclNumber.pop_back();
            symbolTable::pop_symbol_block();
        }

        void handleClass(ast::pASTNode pRoot, bool isInherits)
        {
            pTypeTree pTT = new TypeTree(SYMBOL_TYPE_CLASS,
                                         pRoot->getChild(0)->getText(),
                                         "",    // Todo
                                         new std::vector<std::string>());
            symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, pTT);

            /* 1. add mark */
            symbolTable::add_symbol_mark(SYMBOL_MARK_CLASS);
            /* 2. parse sub-tree */
            if (isInherits) codeGen(pRoot->getChild(2));
            else codeGen(pRoot->getChild(1));
            /* update asmCode */
            std::vector<std::shared_ptr<symbolItem>> vTable = symbolTable::get_symbols_in_scope();
            symbolTable::unionSymbolItems2Tree(pTT, vTable);
            /* 3. pop symbols */
            symbolTable::pop_symbol_block();
        }

        std::string genFnTypes(ast::pASTNode pArgs)
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
             *  genFnTypes return std::string whose children is [Arg1, Arg2, Arg3]
             **/
            assert(pArgs != nullptr);
            std::string asmCode = new TypeTree(SYMBOL_TYPE_FN, "", "", nullptr);
            if (pArgs->getText() == AST_DEFAULT_TEXT) {
                unsigned childrenNumber = pArgs->size();
                assert(childrenNumber == 2);
                std::string singleArg = codeGen(pArgs->getChild(0));
                appendTree(asmCode, singleArg);
                std::string pSub = genFnTypes(pArgs->getChild(1));
                mergeTree(asmCode, pSub);
            } else {
                /* single argument */
                std::string singleArg = codeGen(pArgs);
                appendTree(asmCode, singleArg);
            }

            return asmCode;
        }

        std::string handleLeaf(ast::pASTNode pRoot)
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
                            /* gen function's use type and declaration type be same */
                            if (isFn && EType != E_ID_TYPE_FN_TYPE) assert(!"symbol not function type.");
                            else if (!isFn && EType == E_ID_TYPE_FN_TYPE) assert(!"function should be called.");

                            if (EType == E_ID_TYPE_FN_TYPE && !pRoot->fnHasArgs()) {
                                /* 无参直接计算返回值好不好嘛 */
                                std::string pArgsTree = makeFnTree();
                                appendTrivial(pArgsTree, SYMBOL_TYPE_UNIT);
                                std::string fnRet = genFn(pArgsTree, symbol->type);
                                return fnRet;
                            }

                            /* 这里也有可能返回函数类型哦><，在有函数参数的情况下 */
                            return symbol->type;
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
                            return makeTrivial(SYMBOL_TYPE_TYPE, text);
                        }
                    }
                case ast::AST_LEAF_RE_INT:       return makeTrivial(SYMBOL_TYPE_INT, text);
                case ast::AST_LEAF_RE_DECIMAL:   return makeTrivial(SYMBOL_TYPE_FLOAT, text);    // TODO: decimal as float or double
                case ast::AST_LEAF_RE_STRING:    return makeTrivial(SYMBOL_TYPE_STRING, text);
                case ast::AST_LEAF_TRUE:         return makeTrivial(SYMBOL_TYPE_BOOL, text);
                case ast::AST_LEAF_FALSE:        return makeTrivial(SYMBOL_TYPE_BOOL, text);

                case ast::AST_LEAF_OBJECT:
                case ast::AST_LEAF_BOOL:
                case ast::AST_LEAF_INT:
                case ast::AST_LEAF_FLOAT:
                case ast::AST_LEAF_DOUBLE:
                case ast::AST_LEAF_LONG:
                case ast::AST_LEAF_STRING:
                case ast::AST_LEAF_UNIT:         return makeTrivial(SYMBOL_TYPE_TYPE, text);
                case ast::AST_LEAF_THIS:
                    {
                        // TODO
                        assert(!"have not implemented yet!");
                    }
                case ast::AST_LEAF_TYPE:
                    {
                        return makeTrivial(SYMBOL_TYPE_TYPE, text);
                    }
                default:
                    {
                        assert(!"Type Error!");
                    }
            }

            return "";
        }


        std::string handleDoubleOps(std::string asmCode1, std::string asmCode2, const std::string& text)
        {
            assert(asmCode1 != "");
            assert(asmCode2 != "");
            if (text == "+") {
                return genPlus(asmCode1, asmCode2);
            } else if (text == "-") {
                return genMinus(asmCode1, asmCode2);
            } else if (text == "*") {
                return genStar(asmCode1, asmCode2);
            } else if (text == "/") {
                return genSlash(asmCode1, asmCode2);
            } else if (text == "==") {
                return genCompare(asmCode1, asmCode2);
            } else if (text == ">") {
                return genCompare(asmCode1, asmCode2);
            } else if (text == ">=") {
                return genCompare(asmCode1, asmCode2);
            } else if (text == "<") {
                return genCompare(asmCode1, asmCode2);
            } else if (text == "<=") {
                return genCompare(asmCode1, asmCode2);
            } else if (text == "!=") {
                return genCompare(asmCode1, asmCode2);
            } else if (text == "<-") {
                return genAssign(asmCode1, asmCode2);
            } else {
                assert(!"unknown symbol");
            }

            return "";
        }

        std::string handleSingleOp(std::string asmCode, const std::string& text)
        {
            assert(asmCode != "");
            if (text == "!") {
                return genNot(asmCode);
            } else if (text == "isVoid") {
                return genIsVoid(asmCode);
            } else if (text == "new") {
                return genNew(asmCode);
            } else if (text == "return") {
                std::string pRet = genReturn(asmCode);
                return pRet;
            } else {
                assert(!"unknown symbol");
            }

            return "";
        }

    public:
        /* function name, number of declarations */
        static std::vector<std::pair<std::string, unsigned>> stFnDeclNumber;
};
std::vector<unsigned> CGen::stFnDeclNumber;

};  /* namespace cgen */
};  /* namespace jhin */

