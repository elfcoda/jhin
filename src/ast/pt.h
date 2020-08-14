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
#include "../../comm/container_op.h"

namespace jhin
{
namespace ast
{

using stackTuple = std::tuple<unsigned, comm::pSyntaxDFA, pASTNode>;

class ParseTree
{

    public:
        pASTNode parse(const std::vector<std::pair<unsigned, std::string>>& lexResult, comm::pSyntaxDFA pDFAStart)
        {
            bool b = init();
            assert(b == true);

            pASTNode pRoot = genParseTree(lexResult, pDFAStart);
            // ASTNode::showTree(pRoot);

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
                                            mSymbolId2PT[item.first]));
                    idx ++;
                } else {
                    /* reduce */
                    pASTNode pNonterminal = nullptr;
                    pChildrenList children = new std::vector<pASTNode>();
                    if (!pNFA->isEPSILON()) {
                        /* production E -> EPSILON. should not pop stack */
                        std::vector<stackTuple> vTuple = comm::stackPopGetN(st, pNFA->production.size());
                        /* construct non-terminal sub-tree */
                        for (const auto& item: vTuple) {
                            children->push_back(std::get<2>(item));
                        }
                        pNonterminal = new ASTNode(ASTNodeData(pNFA->nonTerminal), children);
                    } else {
                        /* construct E -> EPSILON production */
                        children->push_back(mSymbolId2PT[SYNTAX_EPSILON_IDX]);
                        pNonterminal = new ASTNode(ASTNodeData(pNFA->nonTerminal), children);
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

            assert(pRoot != nullptr);

            return pRoot;
        }

    private:
        /* create token nodes when initing */
        std::unordered_map<unsigned, pASTNode> mSymbolId2PT;

        bool init()
        {
            return initTokens();
        }

        bool initTokens()
        {
            /* init mSymbolId2PT */
            pASTNode pPT = nullptr;
            for (unsigned token: lex::tokenSet) {
                pPT = new ASTNode(ASTNodeData(token), nullptr);
                mSymbolId2PT[token] = pPT;
            }

            /* shift: $ -> nullptr */
            mSymbolId2PT[SYNTAX_TOKEN_END] = nullptr;
            /* reduce: EPSILON -> ASTNode */
            mSymbolId2PT[SYNTAX_EPSILON_IDX] = new ASTNode(ASTNodeData(SYNTAX_EPSILON_IDX), nullptr);

            return true;
        }
};


};  /* namsspace ast */
};  /* namespace jhin */


