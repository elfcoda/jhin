#pragma once

#include <string>
#include "type_tree.h"
#include "symbol_table.h"
#include "../ast/ast_node.h"
#include "../../comm/log.h"
#include "../../comm/container_op.h"

namespace jhin
{
namespace ts
{

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
     * literal: m4   ->  | CLS  | m4   | uk  | ptr     | Cat
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

        pTypeTree checkDecl(pTypeTree pTree)
        {
            pTypeTree pTT = nullptr;
            // type check
            if (getSymbolType(pTree) == E_ID_TYPE_TRIVIAL_TYPE) {
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

        pTypeTree genSymbolTable(ast::pASTNode pRoot)
        {
            std::string text = pRoot->getText();
            unsigned astSymId = pRoot->getAstSymbolId();
            assert(text != "" && AST_DEFAULT_TEXT != "");

            if (comm::isASTSymbolLeaf(astSymId)) {
                /* leaf node, no declarations and no blocks */
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
            } else if (comm::isASTSymbolNonLeaf(astSymId)) {
                /* non-leaf node */
                if (text == AST_DEFAULT_TEXT) {
                    for (ast::pASTNode child: *(pRoot->children)) {
                        genSymbolTable(child);
                    }
                }
                /* block and declaration */
                else if (text == "class") {
                    pTypeTree pTT = new TypeTree(SYMBOL_TYPE_CLASS,
                                                 pRoot->getChild(0)->getText(),
                                                 "",    // Todo
                                                 new std::vector<pTypeTree>());
                    symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, pTT);

                    /* 1. add mark */
                    symbolTable::add_symbol_mark(SYMBOL_MARK_CLASS);
                    /* 2. parse sub-tree */
                    genSymbolTable(pRoot->getChild(1));
                    /* update pTT */
                    std::vector<std::shared_ptr<symbolItem>> vTable = symbolTable::get_symbols_in_scope();
                    symbolTable::unionSymbolItems2Tree(pTT, vTable);
                    /* 3. pop symbols */
                    symbolTable::pop_symbol_block();
                } else if (text == "class_inherits") {
                    pTypeTree pTT = new TypeTree(SYMBOL_TYPE_CLASS,
                                                 pRoot->getChild(0)->getText(),
                                                 "",    // Todo
                                                 new std::vector<pTypeTree>());
                    symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, pTT);

                    /* 1. add mark */
                    symbolTable::add_symbol_mark(SYMBOL_MARK_CLASS);
                    /* 2. parse sub-tree */
                    genSymbolTable(pRoot->getChild(2));
                    /* update pTT */
                    std::vector<std::shared_ptr<symbolItem>> vTable = symbolTable::get_symbols_in_scope();
                    symbolTable::unionSymbolItems2Tree(pTT, vTable);
                    /* 3. pop symbols */
                    symbolTable::pop_symbol_block();
                } else if (text == "def") {
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
                /* block
                 * command
                 * */
                else if (text == "while") {
                    symbolTable::add_symbol_mark(SYMBOL_MARK_WHILE);
                    genSymbolTable(pRoot->getChild(0));
                    genSymbolTable(pRoot->getChild(1));
                    symbolTable::pop_symbol_block();

                    return nullptr;
                } else if (text == "if") {
                    symbolTable::add_symbol_mark(SYMBOL_MARK_IF);
                    genSymbolTable(pRoot->getChild(0));
                    genSymbolTable(pRoot->getChild(1));
                    symbolTable::pop_symbol_block();

                    return nullptr;
                } else if (text == "if_else") {
                    symbolTable::add_symbol_mark(SYMBOL_MARK_IF);
                    genSymbolTable(pRoot->getChild(0));
                    genSymbolTable(pRoot->getChild(1));
                    symbolTable::pop_symbol_block();

                    symbolTable::add_symbol_mark(SYMBOL_MARK_ELSE);
                    genSymbolTable(pRoot->getChild(0));
                    genSymbolTable(pRoot->getChild(2));
                    symbolTable::pop_symbol_block();

                    return nullptr;
                } else if (text.length() >= 4 && text.substr(0, 4) == "case") {
                    // symbolTable::add_symbol_mark(SYMBOL_MARK_CASE_OF);
                } else if (text.length() >= 6 && text.substr(0, 6) == "lambda") {
                    // symbolTable::add_symbol_mark(SYMBOL_MARK_LAMBDA);
                } else if (text.length() >= 3 && text.substr(0, 3) == "let") {
                    // symbolTable::add_symbol_mark(SYMBOL_MARK_LET_IN);
                }
                /* declaration */
                else if (text == ":") {
                    /* id: type: default_value */
                    // 统一把算法改成从符号表取相关符号，而不是把每个声明都搞成函数参数
                    unsigned childrenNumber = pRoot->size();
                    if (childrenNumber == 1) {
                        /* function return type */
                        return genSymbolTable(pRoot->getChild(0));
                    } else if (childrenNumber == 2) {
                        /**/
                        pTypeTree pLeaf = genSymbolTable(pRoot->getChild(1));
                        pTypeTree pTT = checkDecl(pLeaf);
                        pTT->setSymbolName(pRoot->getChild(0)->getText());
                        symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, pTT);
                        return nullptr;
                    } else if (childrenNumber == 3) {
                        /* recurse */
                        pTypeTree pNonLeaf = genSymbolTable(pRoot->getChild(1));
                        pTypeTree pTT = checkDecl(pNonLeaf);
                        pTT->setSymbolName(pRoot->getChild(0)->getText());
                        symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, pTT);
                        genSymbolTable(pRoot->getChild(2));
                        return nullptr;
                    } else {
                        assert(false);
                    }
                } else if (text == ":_<-") {
                    unsigned childrenNumber = pRoot->size();
                    if (childrenNumber == 3) {
                        pTypeTree pLeaf = genSymbolTable(pRoot->getChild(1));
                        pTypeTree pTT = checkDecl(pLeaf);
                        pTT->setSymbolName(pRoot->getChild(0)->getText());
                        /* initial value */
                        pTypeTree pValue = genSymbolTable(pRoot->getChild(2));
                        pTT->setValue(pValue->getValue());
                        /* add to symbol table */
                        symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, pTT);
                        return nullptr;
                    } else if (childrenNumber == 4) {
                        pTypeTree pNonLeaf = genSymbolTable(pRoot->getChild(1));
                        pTypeTree pTT = checkDecl(pNonLeaf);
                        pTT->setSymbolName(pRoot->getChild(0)->getText());
                        /* initial value */
                        pTypeTree pValue = genSymbolTable(pRoot->getChild(2));
                        pTT->setValue(pValue->getValue());
                        /* add to symbol table */
                        symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, pTT);
                        /* recurse */
                        genSymbolTable(pRoot->getChild(3));
                        return nullptr;
                    } else {
                        assert(false);
                    }
                } else if (text == "->") {  // function type
                } else if (text == "+") {
                } else if (text == "-") {
                } else if (text == "*") {
                } else if (text == "/") {
                } else if (text == "==") {
                } else if (text == ">") {
                } else if (text == ">=") {
                } else if (text == "<") {
                } else if (text == "<=") {
                } else if (text == "!=") {
                } else if (text == "!") {
                } else if (text == "isVoid") {
                } else if (text == "new") {
                } else if (text == "return") {
                } else if (text == "") {
                } else {
                    /* undefined op */
                    comm::Log::singleton(ERROR) >> "text is: " >> text >> comm::newline;
                    assert(false);
                }
            } else {
                assert(false);
            }

            return nullptr;
        }

    private:
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
