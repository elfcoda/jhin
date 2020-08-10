#ifndef __PT_H__
#define __PT_H__

#include <vector>
#include <string>
#include <tuple>
#include <unordered_map>
#include <queue>
#include "../../comm/log.h"
#include "../../comm/dfa.h"
#include "../../comm/comm.h"
#include "../../comm/container_op.h"

namespace jhin
{
namespace ast
{

struct PTNodeData;
using pPTNodeData = PTNodeData*;

struct PTNode;
using pPTNode = PTNode*;

using pChildrenList = std::vector<pPTNode>*;

struct PTNodeData
{
    /* non-terminal id or token id */
    unsigned symbolId;

    std::string toString()
    {
        std::string s = comm::symbolId2String(symbolId);
        return s;
    }

    PTNodeData(unsigned symbolId)
    {
        this->symbolId = symbolId;
    }

    PTNodeData(PTNodeData&& ptnode)
    {
        this->symbolId = ptnode.symbolId;
    }
};

struct PTNode
{
    pPTNodeData data;
    pChildrenList children;

    PTNode(PTNodeData&& data, pChildrenList children)
    {
        this->data = new PTNodeData(std::move(data));
        this->children = children;
    }

    /* free memory */
    bool free()
    {
        return true;
    }
};

using stackTuple = std::tuple<unsigned, comm::pSyntaxDFA, pPTNode>;

class ParseTree
{

    public:
        bool parse(const std::vector<std::pair<unsigned, std::string>>& lexResult, comm::pSyntaxDFA pDFAStart)
        {
            bool b = init();
            assert(b == true);

            pPTNode pRoot = genParseTree(lexResult, pDFAStart);
            showTree(pRoot);

            return true;
        }

        /* return root node of parse tree */
        pPTNode genParseTree(const std::vector<std::pair<unsigned, std::string>>& lexResult, comm::pSyntaxDFA pDFAStart)
        {
            pPTNode pRoot = nullptr;
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
                    pPTNode pNonterminal = nullptr;
                    pChildrenList children = new std::vector<pPTNode>();
                    if (!pNFA->isEPSILON()) {
                        /* production E -> EPSILON. should not pop stack */
                        std::vector<stackTuple> vTuple = comm::stackPopGetN(st, pNFA->production.size());
                        /* construct non-terminal sub-tree */
                        for (const auto& item: vTuple) {
                            children->push_back(std::get<2>(item));
                        }
                        pNonterminal = new PTNode(PTNodeData(pNFA->nonTerminal), children);
                    } else {
                        /* construct E -> EPSILON production */
                        children->push_back(mSymbolId2PT[SYNTAX_EPSILON_IDX]);
                        pNonterminal = new PTNode(PTNodeData(pNFA->nonTerminal), children);
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
             * Prog', special, pPTNode;
             * $, nullptr, nullptr;
             * */

            assert(pRoot != nullptr);

            return pRoot;
        }

        void showTree(pPTNode pRoot)
        {
            std::string graphFile = "/Users/luwenjie/git/jhin/jhin/src/ast/graph.py";
            std::tuple<std::string, unsigned> tree = parseTree2String(pRoot, 0);
            std::string sPy = "tree = \n";
            comm::Log::singleton(INFO, false, graphFile) >> sPy >> std::get<0>(tree) >> comm::newline;
        }

        std::tuple<std::string, unsigned> parseTree2String(pPTNode pRoot, int indent)
        {
            if (pRoot == nullptr) return std::make_tuple("[]", 0);

            std::string sBlank(indent * 4, ' ');
            /* (symbolString, wide, symboltype) */
            SymbolType stype = comm::symbolId2Type(pRoot->data->symbolId);
            std::string s1 = "[(\"" + pRoot->data->toString() + "\", ";
            std::string s2 = ", " + std::to_string(stype) + ")";
            unsigned curWide = 1;
            if (pRoot->children != nullptr) {
                for (pPTNode p: *(pRoot->children)) {
                    /* pPTNodes we push_back before can not be nullptr */
                    assert(p != nullptr);
                    auto tu = parseTree2String(p, indent + 1);
                    curWide += std::get<1>(tu);
                    s2 += ", \n" + sBlank + std::get<0>(tu);
                }
            }
            curWide = (curWide == 1 ? 1 : curWide - 1);
            s2 += "]";

            return std::make_tuple(s1 + std::to_string(curWide) + s2, curWide);
        }

    private:
        /* create token nodes when initing */
        std::unordered_map<unsigned, pPTNode> mSymbolId2PT;

        bool init()
        {
            return initTokens();
        }

        bool initTokens()
        {
            /* init mSymbolId2PT */
            pPTNode pPT = nullptr;
            for (unsigned token: lex::tokenSet) {
                pPT = new PTNode(PTNodeData(token), nullptr);
                mSymbolId2PT[token] = pPT;
            }

            /* shift: $ -> nullptr */
            mSymbolId2PT[SYNTAX_TOKEN_END] = nullptr;
            /* reduce: EPSILON -> PTNode */
            mSymbolId2PT[SYNTAX_EPSILON_IDX] = new PTNode(PTNodeData(SYNTAX_EPSILON_IDX), nullptr);

            return true;
        }
};


};  /* namsspace ast */
};  /* namespace jhin */

#endif

