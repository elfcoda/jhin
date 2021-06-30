#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <unordered_map>
#include <queue>
#include "ast_node.h"
#include "../../comm/log.h"
#include "../../comm/dfa.h"
#include "../../comm/comm.h"
#include "../../comm/jhin_assert.h"
#include "../../comm/container_op.h"

namespace jhin
{
namespace ast
{

using stackTuple = std::tuple<unsigned, comm::pSyntaxDFA, pASTNode>;

class ParseTree
{

    public:
        // template <class RET, class... Ts>
        // using PTFunc = RET(Ts...);

        // template <class RET = void, class... Ts>
        // void travelParseTree(pASTNode pRoot, PTFunc<RET, Ts...> f, Ts... ts)

        void genFnLeafVector(std::vector<unsigned>& ans, pASTNode pNode)
        {
            if (!pNode->hasChildren()) ans.push_back(pNode->getSymbolId());
            for (unsigned i = 0; i < pNode->size(); i++) {
                genFnLeafVector(ans, pNode->getChild(i));
            }
        }

        bool isFnNoArgs(pASTNode pNode)
        {
            std::vector<unsigned> vLeaf;
            genFnLeafVector(vLeaf, pNode);
            JHIN_ASSERT_BOOL(!vLeaf.empty());
            if (vLeaf.size() == 1) {
                JHIN_ASSERT_BOOL(vLeaf[0] == SYNTAX_EPSILON_IDX);
                return true;
            }

            return false;
        }

        void travelHandleParseTree(pASTNode pRoot)
        {
            if (pRoot == nullptr) return;
            /* do something to pRoot */
            if (comm::symbolId2String(pRoot->getSymbolId()) == "FnCall") {
                /* add fn mark */
                JHIN_ASSERT_BOOL(pRoot->hasChildren());
                pASTNode child = pRoot->getChild(0);
                child->setMark(E_AST_MARK_FN);
                comm::Log::singleton(DEBUG) >> "set-> " >> child->getText() >> " " >> child->getMark() >> comm::newline;
                /* handle FnArgs */
                for (unsigned i = 0; i < pRoot->size(); i++) {
                    if (comm::symbolId2String(pRoot->getChild(i)->getSymbolId()) == "FnArgs") {
                        if (!isFnNoArgs(pRoot->getChild(i))) {
                            child->setMark(E_AST_MARK_FN_HAS_ARGS);
                            break;
                        }
                    }
                }
            }
            if (!pRoot->hasChildren()) return;
            for (unsigned i = 0; i < pRoot->size(); i++) {
                travelHandleParseTree(pRoot->getChild(i));
            }
        }

        void travelParseTree(pASTNode pRoot)
        {
            if (pRoot == nullptr) return;
            /* do something to pRoot */
            comm::Log::singleton(DEBUG) >> pRoot->getText() >> " " >> pRoot->getMark() >> comm::newline;
            if (!pRoot->hasChildren()) return;
            for (unsigned i = 0; i < pRoot->size(); i++) {
                travelParseTree(pRoot->getChild(i));
            }
        }

        pASTNode parse(const std::vector<std::pair<unsigned, std::string>>& lexResult, comm::pSyntaxDFA pDFAStart)
        {
            bool b = init();
            JHIN_ASSERT_BOOL(b == true);

            pASTNode pRoot = genParseTree(lexResult, pDFAStart);
            ASTNode::showTree(pRoot);
            travelHandleParseTree(pRoot);
            // travelParseTree(pRoot);

            return pRoot;
        }

        /* return root node of parse tree */
        pASTNode genParseTree(const std::vector<std::pair<unsigned, std::string>>& lexResult, comm::pSyntaxDFA pDFAStart)
        {
            pASTNode pRoot = nullptr;
            std::stack<stackTuple> st;
            st.push(std::make_tuple(SYNTAX_TOKEN_BEGIN, pDFAStart, nullptr));

            syntax::pSyntaxNFAData pNFA = nullptr;
            for (unsigned idx = 0; idx < lexResult.size(); ) {
                const std::pair<unsigned, std::string>& item = lexResult[idx];
                if ((pNFA = std::get<1>(st.top())->canReduce(item.first)) == nullptr) {
                    /* shift:
                     * Prog'  should shift $ to nullptr
                     * */
                    st.push(std::make_tuple(item.first,
                                            std::get<1>(st.top())->mEdges[item.first],
                                            new ASTNode(ASTNodeData(item.first, item.second), nullptr)));
                    idx ++;
                } else {
                    /* reduce */
                    pASTNode pNonterminal = nullptr;
                    pChildrenList children = new std::vector<pASTNode>();
                    if (!pNFA->isEPSILON()) {
                        /* production E -> EPSILON. would not pop stack */
                        std::vector<stackTuple> vTuple = comm::stackPopGetN(st, pNFA->production.size());
                        /* construct non-terminal sub-tree */
                        for (const auto& item: vTuple) {
                            children->push_back(std::get<2>(item));
                        }
                        pNonterminal = new ASTNode(ASTNodeData(pNFA->nonTerminal, AST_DEFAULT_TEXT), children);
                    } else {
                        /* construct E -> EPSILON production */
                        children->push_back(new ASTNode(ASTNodeData(SYNTAX_EPSILON_IDX, AST_DEFAULT_TEXT), nullptr));
                        pNonterminal = new ASTNode(ASTNodeData(pNFA->nonTerminal, AST_DEFAULT_TEXT), children);
                    }

                    if (pNFA->nonTerminal == NON_TERMINAL_IDX_MIN) {
                        /* Prog' -> Prog */
                        pRoot = pNonterminal;
                    }

                    st.push(std::make_tuple(pNFA->nonTerminal,
                                            std::get<1>(st.top())->mEdges[pNFA->nonTerminal],
                                            pNonterminal));
                }
            }
            /* $ has been shifted, stack:
             * #, pStart, nullptr;
             * Prog', special, pASTNode;
             * $, nullptr, nullptr;
             * */

            JHIN_ASSERT_BOOL(pRoot != nullptr);

            return pRoot;
        }

    private:
        bool init()
        {
            return initTokens();
        }

        bool initTokens()
        {
            return true;
        }
};


};  /* namsspace ast */
};  /* namespace jhin */


