#pragma once


#include <string>
#include "../ast/ast_node.h"
#include "../ast/ast_define.h"
#include "../../comm/log.h"

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

        bool genSymbolTable(pASTNode pRoot)
        {
            std::string text = pRoot->getText();
            assert(text != "" && AST_DEFAULT_TEXT != "");
            if (text == AST_DEFAULT_TEXT) {
                for (pASTNode child: *(pRoot->children)) {
                    genSymbolTable(child);
                }
            } else if (text == "if") {
                // pRoot.getChild();
            } else if () {
            } else if () {
            } else if () {
            } else if () {
            } else if () {
            } else if () {
            } else if () {
            } else if () {
            } else if () {
            } else if () {
            } else if () {
            } else {
                /* undefined op */
                comm::Log::singleton(ERROR) >> "text is: " >> text >> comm::newline;
                assert(false);
            }
        }
};

};  /* namespace ts */
};  /* namespace jhin */
