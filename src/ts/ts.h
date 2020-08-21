#pragma once

#include <string>
#include "type_tree.h"
#include "../ast/ast_node.h"
#include "../ast/ast_define.h"
#include "../../comm/log.h"
#include "../../comm/container_op.h"

namespace jhin
{
namespace ts
{

class TypeSystem
{
    public:
        TypeSystem() {}

        bool check(pASTNode pRoot)
        {
            return false;
        }

        TypeTree genSymbolTable(pASTNode pRoot)
        {
            constexpr static std::vector<ESymbolType> EtyVtr = {};

            std::string text = pRoot->getText();
            unsigned astSymId = pRoot->getAstSymbolId();
            assert(text != "" && AST_DEFAULT_TEXT != "");

            if (comm::isASTSymbolLeaf(astSymId)) {
                /* leaf node, no declarations and no blocks */
                // TODO
            } else if (comm::isASTSymbolNonLeaf(astSymId)){
                /* non-leaf node */
                if (text == AST_DEFAULT_TEXT) {
                    for (pASTNode child: *(pRoot->children)) {
                        genSymbolTable(child);
                    }
                }
                /* block and declaration */
                else if (text == "class") {
                    pRoot->getChild(0)->getText()
                    symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, SYMBOL_TYPE_CLASS);

                    symbolTable::add_symbol_mark(SYMBOL_MARK_CLASS);
                    genSymbolTable(pRoot->getChild(1));
                    symbolTable::pop_symbol_block();
                } else if (text == "class_inherits") {
                    pRoot->getChild(0)->getText()
                    symbolTable::add_symbol(SYMBOL_MARK_SYMBOL, SYMBOL_TYPE_CLASS);

                    symbolTable::add_symbol_mark(SYMBOL_MARK_CLASS);
                    genSymbolTable(pRoot->getChild(2));
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
                            assert(pTp != nullptr);
                            mergeTypeTree(fnType, pTp);
                            assert(fnType->hasChildren());
                            appendTrivial(fnType, SYMBOL_TYPE_UNIT);
                        } else if (notationStr == "FnRetTp") {
                            assert(pTp != nullptr);
                            if (!fnType->hasChildren()) {
                                appendTrivial(fnType, SYMBOL_TYPE_UNIT);
                            } else {
                                popChild(fnType);
                            }
                            mergeTypeTree(fnType, pTp);
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
                /* block */
                else if (text == "while") {
                    symbolTable::add_symbol_mark(SYMBOL_MARK_WHILE);
                    genSymbolTable(pRoot->getChild(0));
                    genSymbolTable(pRoot->getChild(1));
                    symbolTable::pop_symbol_block();
                } else if (text == "if") {
                    symbolTable::add_symbol_mark(SYMBOL_MARK_IF);
                    genSymbolTable(pRoot->getChild(0));
                    genSymbolTable(pRoot->getChild(1));
                    symbolTable::pop_symbol_block();
                } else if (text == "if_else") {
                    symbolTable::add_symbol_mark(SYMBOL_MARK_IF);
                    genSymbolTable(pRoot->getChild(0));
                    genSymbolTable(pRoot->getChild(1));
                    symbolTable::pop_symbol_block();

                    symbolTable::add_symbol_mark(SYMBOL_MARK_ELSE);
                    genSymbolTable(pRoot->getChild(0));
                    genSymbolTable(pRoot->getChild(2));
                    symbolTable::pop_symbol_block();
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
                    // TODO normal declaration also recognized to function type, should distinguish them latter by more parameters
                    unsigned childrenNumber = pRoot->size();
                    if (childrenNumber == 1) {
                        /* function return type */
                        return genSymbolTable(pRoot->getChild(0));
                    } else if (childrenNumber == 2) {
                        pTypeTree pLeaf = genSymbolTable(pRoot->getChild(1));
                        pLeaf->symbolName = pRoot->getChild(0)->getText();
                        return new TypeTree(SYMBOL_TYPE_FN, "", "", &comm::element2Vec(pLeaf))
                    } else if (childrenNumber == 3) {
                        /* recurse */
                        pTypeTree pNonLeaf = genSymbolTable(pRoot->getChild(1));
                        pNonLeaf->symbolName = pRoot->getChild(0)->getText();
                        pTypeTree pRet = genSymbolTable(pRoot->getChild(2));
                        pRet->append(pNonLeaf);
                        return pRet;
                    } else {
                        assert(false);
                    }
                } else if (text == ":_<-") {
                    unsigned childrenNumber = pRoot->size();
                    if (childrenNumber == 3) {
                        pTypeTree pLeaf = genSymbolTable(pRoot->getChild(1));
                        /* initial value */
                        pTypeTree pValue = genSymbolTable(pRoot->getChild(2));
                        // TODO: Typecheck && default value check(trivial value)
                        pLeaf->symbolName = pRoot->getChild(0)->getText();
                        pLeaf->value = pValue->value;
                        return new TypeTree(SYMBOL_TYPE_FN, "", "", &comm::element2Vec(pLeaf))
                    } else if (childrenNumber == 4) {
                        /* recurse */
                        pTypeTree pNonLeaf = genSymbolTable(pRoot->getChild(1));
                        /* initial value */
                        pTypeTree pValue = genSymbolTable(pRoot->getChild(2));
                        pNonLeaf->symbolName = pRoot->getChild(0)->getText();
                        pNonLeaf->value = pValue->value;
                        pTypeTree pRet = genSymbolTable(pRoot->getChild(3));
                        pRet->append(pNonLeaf);
                        return pRet;
                    } else {
                        assert(false);
                    }
                } else if (text == "->") {
                    // TODO
                } else {
                    /* undefined op */
                    comm::Log::singleton(ERROR) >> "text is: " >> text >> comm::newline;
                    assert(false);
                }
            } else {
                assert(false);
            }
        }

    private:
        std::string getNotationStr(pASTNode parent, unsigned idx)
        {
            unsigned notation = parent->getChild(idx)->getNotation();
            assert(syntax::id_to_non_terminal.find(notation) != syntax::id_to_non_terminal.end());
            std::string notationStr = syntax::id_to_non_terminal.at(notation);

            return notationStr;
        }
};

};  /* namespace ts */
};  /* namespace jhin */
