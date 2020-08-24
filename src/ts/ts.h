#pragma once

#include <string>
#include "type_tree.h"
#include "type_checker.h"
#include "symbol_table.h"
#include "../ast/ast_node.h"
#include "../../comm/log.h"
#include "../../comm/container_op.h"

namespace jhin
{
namespace ts
{

const std::unordered_set<std::string> doubleArgs = {"+", "-", "*" "/", "==", ">", ">=", "<", "<=", "!="};
const std::unordered_set<std::string> singleArg = {"!", "isVoid", "new", "return"};
bool isDoubleArgs(const std::string& text)
{
    return doubleArgs.find(text) != doubleArgs.end();
}

bool isSingleArg(const std::string& text)
{
    return singleArg.find(text) != singleArg.end();
}

class TypeSystem
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
        TypeSystem() {}

        /* gen symbol table, type checker */
        pTypeTree genSymbolTable(ast::pASTNode pRoot)
        {
            std::string text = pRoot->getText();
            unsigned astSymId = pRoot->getAstSymbolId();
            unsigned childrenNumber = pRoot->size();
            assert(text != "" && AST_DEFAULT_TEXT != "");

            if (comm::isASTSymbolLeaf(astSymId)) {
                /* leaf node, no declarations and no blocks */
                return handleLeaf(astSymId, text);
            } else if (comm::isASTSymbolNonLeaf(astSymId)) {
                /* non-leaf node */
                if (text == AST_DEFAULT_TEXT) {
                    for (ast::pASTNode child: *(pRoot->children)) {
                        genSymbolTable(child);
                    }
                } else if (text == "class") {   /* block and declaration */
                    handleClass(pRoot, false);
                    return nullptr;
                } else if (text == "class_inherits") {
                    handleClass(pRoot, true);
                    return nullptr;
                } else if (text == "def") {
                    handleFn(pRoot);
                    return nullptr;
                } else if (text == "while") {   /* block, command */
                    handleCMD(pRoot, SYMBOL_MARK_WHILE);
                    return nullptr;
                } else if (text == "if") {
                    handleCMD(pRoot, SYMBOL_MARK_IF);
                    return nullptr;
                } else if (text == "if_else") {
                    handleCMD(pRoot, SYMBOL_MARK_IF);

                    symbolTable::add_symbol_mark(SYMBOL_MARK_ELSE);
                    genSymbolTable(pRoot->getChild(0));
                    genSymbolTable(pRoot->getChild(2));
                    symbolTable::pop_symbol_block();

                    return nullptr;
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
                        return nullptr;
                    } else if (childrenNumber == 3) {
                        handleDecl(pRoot);
                        /* recurse */
                        genSymbolTable(pRoot->getChild(2));
                        return nullptr;
                    } else {
                        assert(false);
                    }
                } else if (text == ":_<-") {
                    if (childrenNumber == 3) {
                        handleDeclAssign(pRoot);
                        return nullptr;
                    } else if (childrenNumber == 4) {
                        handleDeclAssign(pRoot);
                        genSymbolTable(pRoot->getChild(3));
                        return nullptr;
                    } else {
                        assert(false);
                    }
                } else if (text == "->") {  // TODO: function type
                } else if (isDoubleArgs(text)) {
                    assert(childrenNumber == 2);
                    pTypeTree pTT1 = genSymbolTable(pRoot->getChild(0));
                    pTypeTree pTT2 = genSymbolTable(pRoot->getChild(1));
                    return handleDoubleOps(pTT1, pTT2, text);
                }
                else if (isSingleArg(text)) {
                    assert(childrenNumber == 1);
                    pTypeTree pTT = genSymbolTable(pRoot->getChild(0));
                    return handleSingleOp(pTT, text);
                }
                else {
                    /* undefined op */
                    comm::Log::singleton(ERROR) >> "text is: " >> text >> comm::newline;
                    assert(!"undefined op");
                }
            } else {
                assert(!"symbol should be leaf or non-leaf");
            }

            return nullptr;
        }


    private:
        pTypeTree handleLeaf(unsigned astSymId, const std::string& text)
        {
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

            return nullptr;
        }


        pTypeTree handleDoubleOps(pTypeTree pTT1, pTypeTree pTT2, const std::string& text)
        {
            if (text == "+") {
                return checkPlus(pTT1, pTT2);
            } else if (text == "-") {
                return checkMinus(pTT1, pTT2);
            } else if (text == "*") {
                return checkStar(pTT1, pTT2);
            } else if (text == "/") {
                return checkSlash(pTT1, pTT2);
            } else if (text == "==") {
                return checkEquality(pTT1, pTT2);
            } else if (text == ">") {
                return checkOrder(pTT1, pTT2);
            } else if (text == ">=") {
                return checkOrder(pTT1, pTT2);
            } else if (text == "<") {
                return checkOrder(pTT1, pTT2);
            } else if (text == "<=") {
                return checkOrder(pTT1, pTT2);
            } else if (text == "!=") {
                return checkEquality(pTT1, pTT2);
            } else {
                assert(!"unknown symbol");
            }

            return nullptr;
        }

        pTypeTree handleSingleOp(pTypeTree pTT, const std::string& text)
        {
            if (text == "!") {
                return checkNot(pTT);
            } else if (text == "isVoid") {
                return checkIsVoid(pTT);
            } else if (text == "new") {
                return checkNew(pTT);
            } else if (text == "return") {
                return checkReturn(pTT);
            } else {
                assert(!"unknown symbol");
            }

            return nullptr;
        }

        void handleFn(ast::pASTNode pRoot)
        {
            pTypeTree fnType = makeFnTree();
            fnType->setSymbolName(pRoot->getChild(0)->getText());
            // TODO set value
            symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, fnType);


            /* generate the signature of function */
            symbolTable::add_symbol_mark(SYMBOL_MARK_FN);
            for (int idx = 1; idx < pRoot->size(); idx++) {
                pTypeTree pTp = genSymbolTable(pRoot->getChild(idx));
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
                    appendTree(fnType, static_cast<pConstTypeTree>(pTp));
                }
            }
            if (!fnType->hasChildren()) {
                /* neither args and ret types is defined */
                /* input */
                appendTrivial(fnType, SYMBOL_TYPE_UNIT);
                /* output */
                appendTrivial(fnType, SYMBOL_TYPE_UNIT);
            }

            symbolTable::pop_symbol_block();
        }

        void handleClass(ast::pASTNode pRoot, bool isInherits)
        {
            pTypeTree pTT = new TypeTree(SYMBOL_TYPE_CLASS,
                                         pRoot->getChild(0)->getText(),
                                         "",    // Todo
                                         new std::vector<pTypeTree>());
            symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, pTT);

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
            pTypeTree pNode = genSymbolTable(pRoot->getChild(1));
            pTypeTree pTT = checkDecl(pNode);
            pTT->setSymbolName(pRoot->getChild(0)->getText());
            symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, pTT);
        }

        void handleDeclAssign(ast::pASTNode pRoot)
        {
            pTypeTree pNode = genSymbolTable(pRoot->getChild(1));
            pTypeTree pTT = checkDecl(pNode);
            pTT->setSymbolName(pRoot->getChild(0)->getText());
            /* initial value */
            pTypeTree pValue = genSymbolTable(pRoot->getChild(2));
            pTT->setValue(pValue->getValue());
            /* add to symbol table */
            symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, pTT);
        }

        std::string getNotationStr(ast::pASTNode parent, unsigned idx)
        {
            unsigned notation = parent->getChild(idx)->getNotation();
            assert(syntax::id_to_non_terminal.find(notation) != syntax::id_to_non_terminal.end());
            std::string notationStr = syntax::id_to_non_terminal.at(notation);

            return notationStr;
        }
};

};  /* namespace ts */
};  /* namespace jhin */
