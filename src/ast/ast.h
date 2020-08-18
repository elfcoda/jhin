#pragma once

#include <set>
#include <unordered_set>
#include <stack>
#include <queue>
#include <string>
#include "pt.h"
#include "ast_node.h"
#include "../../comm/log.h"
#include "../lex/keywords.h"
#include "../syntax/non_terminal.h"
#include "../syntax/syntax_nfa.h"

namespace jhin
{
namespace ast
{

const std::unordered_set<std::string> cutSetStr = {"LPAREN", "RPAREN", "LCURLY", "RCURLY", "LBRACK", "RBRACK", "NEWLINE", "COMMA"};

class AST
{
    public:
        pASTNode parseTree2AST(pASTNode pRoot)
        {
            init();
            cut(pRoot, nullptr, 0);

            /* generate AST */
            liftTokens(pRoot);

            comm::Log::singleton(INFO) >> mASTNonLeafStr2Id >> comm::newline;
            ASTNode::showTree(pRoot);

            return pRoot;
        }

        bool init()
        {
            initCutSet();
            return true;
        }
    private:
        /* return: current sub-tree should be cut or not
         * @idx: pRoot is the idx children of parent. necessary when parent is not nullptr */
        bool cut(pASTNode pRoot, pASTNode parent, unsigned idx)
        {
            /**/
            assert(pRoot != nullptr);

            pASTNode p = pRoot, singleChild = nullptr;
            while (true) {
                if (!p->hasChildren()) {
                    if (inCutSet(p)) return true;
                    else return false;
                } else if ((singleChild = p->getSingle()) != nullptr) {
                    if (parent != nullptr) {
                        (*(parent->children))[idx] = singleChild;
                        singleChild->setNotation(p->getNotation());
                        p = singleChild;
                    } else if (!singleChild->hasChildren()) {
                        // p->setSymbolId(singleChild->getSymbolId());
                        // p->children = nullptr;
                        return false;
                    } else {
                        /* if single child has children, we move the children to p */
                        // p->children = singleChild->children;
                        return false;
                    }
                } else {
                    for (int i = 0; i < p->children->size();) {
                        pASTNode child = (*(p->children))[i];
                        bool isCut = cut(child, p, i);
                        if (isCut) {
                            p->erase(i);
                        } else {
                            i++;
                        }
                    }
                    if (p->children->size() > 1) return false;
                    if (p->children->size() == 0) continue;
                    if (p->children->size() == 1) {
                        singleChild = (*(p->children))[0];
                        if (parent != nullptr) {
                            (*(parent->children))[idx] = singleChild;
                            singleChild->setNotation(p->getNotation());
                            return false;
                        } else if (!singleChild->hasChildren()) {
                            // p->setSymbolId(singleChild->getSymbolId());
                            // p->children = nullptr;
                            return false;
                        } else {
                            // p->children = singleChild->children;
                            return false;
                        }
                    }
                }
            }
        }

        bool liftTokens(pASTNode pRoot)
        {
            std::string astLiftString = "";
            if (!pRoot->hasChildren()) {
                return true;
            }

            for (auto it = pRoot->children->begin(); it != pRoot->children->end(); ) {
                pASTNode pChild = *it;
                unsigned symbol = pChild->getSymbolId();
                if (comm::isTokenLeaf(symbol)) {
                    pChild->setAstSymbolId(getTokenLeafId(symbol));
                    it++;
                } else if (comm::isToken(symbol) && !comm::isTokenLeaf(symbol)) {
                    astLiftString += comm::symbolId2String(symbol, true) + "_";
                    it = pRoot->children->erase(it);
                } else if (comm::isNonTerminal(symbol)) {
                    liftTokens(pChild);
                    it++;
                } else {
                    assert(false);
                }
            }
            if (astLiftString == "") astLiftString = AST_DEFAULT_TEXT;
            else astLiftString.pop_back();

            pRoot->setText(astLiftString);
            pRoot->setAstSymbolId(getNonLeafId(astLiftString));
            return true;
        }


        bool initCutSet()
        {
            for (const std::string& s: cutSetStr) {
                assert(lex::string2TokenId.find(s) != lex::string2TokenId.end());
                cutSet.insert(lex::string2TokenId[s]);
            }

            for (unsigned nonterminal = NON_TERMINAL_IDX_MIN; nonterminal <= NON_TERMINAL_IDX_MAX; nonterminal++) {
                cutSet.insert(nonterminal);
            }

            cutSet.insert(SYNTAX_EPSILON_IDX);
            cutSet.insert(SYNTAX_TOKEN_END);
            cutSet.insert(SYNTAX_TOKEN_BEGIN);

            return true;
        }

        std::unordered_set<unsigned> cutSet;

        bool inCutSet(pASTNode p)
        {
            return cutSet.find(p->data->symbolId) != cutSet.end();
        }
};

};  /* namespace ast */
};  /* namespace jhin */