#ifndef __DFA_H__
#define __DFA_H__

#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <cassert>
#include <type_traits>
#include "../src/lex/nfa.h"
#include "../src/syntax/syntax_nfa.h"
#include "hash.h"
#include "log.h"
#include "container_op.h"

namespace jhin
{
namespace comm
{


/* DFA definition */
/* a DFANode can be generated only if the sNodeData is compatible with that in 'static mHash' */
template <class PNFA>
struct DFANode
{
    /* hash map, (hash value of sNodeData) -> (DFA set) */
    static std::map<unsigned int, std::set<DFANode<PNFA>*>> mHash;

    /* record max node id to increase id */
    static unsigned int maxId;

    /* dfa node id */
    unsigned int id;

    /* node in PNFA(normal and terminal) */
    /* std::set<unsigned int> sNodeData; */
    /* {1, 3, 23, 57}  should be same with {1, 23, 57, 3}, so sNodeData should be ordered for equality comparation */
    /* if pNFANode is ordered, pNFANode->id is in particular order as well */
    std::set<PNFA> sNodeData;

    /* the follow-set of every NFA in a DFA node
     * used only in LALR algorithm */
    std::map<syntax::pSyntaxNFAData, std::unordered_set<unsigned>> followSet;

    /* enabled only for DEBUG!!! to display where the unsigned id(aka. token etc) from
     * pNFA -> map<terminal, pNFA>
     * */
    std::map<syntax::pSyntaxNFAData, std::unordered_map<unsigned, syntax::pSyntaxNFAData>> followMap;

    /* production only */
    std::string toStringRaw()
    {
        std::string s = "";
        for (PNFA p: sNodeData) {
            s += p->toString();
            s += "\n";
        }
        return s;
    }

    /* switch followSet to string */
    std::string toString()
    {
        std::string s = "DFA_" + std::to_string(this->id) + ":\n";
        std::string prefix = "";
        // for (syntax::pSyntaxNFAData p: sNodeData) { s += p->toString(); }
        for (const auto& item: followMap) {
            s += item.first->toString();

            /* format start */
            unsigned sSize = item.first->toString().length();
            unsigned sMaxSize = 128;
            if (sSize < sMaxSize) {
                for (int i = 0; i < sMaxSize - sSize; i++) s += "-";
            }
            /* format end */

            s += "\t<";
            for (auto it = item.second.begin(); it != item.second.end(); it++) {
                if (it == item.second.begin()) prefix = "";
                else prefix = " ";

                unsigned id = it->first;
                if (id == SYNTAX_EPSILON_IDX) {
                    // s += prefix + "{" + SYNTAX_EPSILON_STR + " -> nullptr}";
                } else if (id < SYNTAX_EPSILON_IDX) {
                    s += prefix + "{" + syntax::id_to_non_terminal[id] + " -> NFAID_" + std::to_string(it->second->id) + "}";
                } else if (id == SYNTAX_TOKEN_END) {
                    s += prefix + "{$ -> nullptr}";
                } else {
                    /* turn "PLUS" to "+" */
                    if (syntax::token_string_to_symbol.find(lex::tokenId2String[id]) == syntax::token_string_to_symbol.end()) {
                        s += prefix + "{" + lex::tokenId2String[id] + " -> NFAID_" + std::to_string(it->second->id) + "}";
                    } else {
                        s += prefix + "{" + syntax::token_string_to_symbol[lex::tokenId2String[id]] + " -> NFAID_" + ((it->second == nullptr) ? "nullptr" : std::to_string(it->second->id)) + "}";
                    }
                }
            }
            s += ">\n";
        }
        s += "\n\n";

        return s;
    }

    /* nfa terminal node id  */
    /* non-terminal node if terminalId == UINT_MAX */
    unsigned int terminalId;

    /* hash value of node data, used to compare sNodeData */
    unsigned int hash;

    /* edges */
    /* std::map<char, DFANode*> mEdges;
     * to get compatible with SyntaxNFAData
     */
    std::map<unsigned, DFANode<PNFA>*> mEdges;

    DFANode(unsigned int hash, std::set<lex::pNFANode> sNFA)
    {
        /* id starts from 1 */
        maxId += 1;
        id = maxId;
        this->hash = hash;
        sNodeData = sNFA;
        setTerminalId();
        mHash[hash].insert(this);
        assert(id < UINT_MAX);
    }

    DFANode(unsigned int hash, std::set<syntax::pSyntaxNFAData> sNFA)
    {
        /* id starts from 1 */
        maxId += 1;
        id = maxId;
        this->hash = hash;
        sNodeData = sNFA;
        mHash[hash].insert(this);
        assert(id < UINT_MAX);
    }

    std::unordered_set<unsigned>  genShift()
    {
        std::unordered_set<unsigned> shiftSet;
        for (const auto& item: mEdges) {
            if (lex::tokenSet.find(item.first) != lex::tokenSet.end()) {
                shiftSet.insert(item.first);
            }
        }

        return shiftSet;
    }

    void isConflict()
    {
        Log::singleton() >> toString() >> newline >> "$$$$$$$$$$$$$$$$$$$$$" >> newline;
        std::unordered_map<unsigned, syntax::pSyntaxNFAData> mToken2NFA;

        /* shift token may correspond more than one NFA, we just set it to nullptr */
        comm::unionSet2Map<std::unordered_map, unsigned, syntax::pSyntaxNFAData, std::unordered_set>(mToken2NFA, genShift(), nullptr);

        for (const auto& item: followSet) {
            /* find unreducable NFA productions */
            if (item.first->production.size() != item.first->position) continue;

            for (unsigned id: item.second) {
                if (mToken2NFA.find(id) != mToken2NFA.end()) {
                    /* conflict */
                    if (mToken2NFA[id] == nullptr) {
                        Log::singleton() >> "in DFA_" >> this->id >> ": Shift-Reduce conflict, Reduce production is: " >> syntax::id_to_non_terminal[item.first->nonTerminal] >> " => " >> item.first->production >> newline;
                    } else {
                        Log::singleton() >> "in DFA_" >> this->id >> ": Reduce-Reduce conflict, Reduce production1 is: " >> syntax::id_to_non_terminal[mToken2NFA[id]->nonTerminal] >> " => " >> mToken2NFA[id]->production >> newline \
                                         >> "Reduce production2 is: " >> syntax::id_to_non_terminal[item.first->nonTerminal] >> " => " >> item.first->production >> newline;
                    }
                    // assert(false);
                } else {
                    mToken2NFA[id] = item.first;
                }
            }
        }
    }

    void setTerminalId()
    {
        terminalId = UINT_MAX;
        for (PNFA p: sNodeData) {
            /* ERROR first, not in charset error */
            if (p->id == static_cast<unsigned int>(lex::ERR_ERROR)) {
                terminalId = p->id;
                return;
            }
            if (p->id > TERMINATOR && p->id < terminalId) {
                terminalId = p->id;
            }
        }
    }
};
/* PNFA: pNFA para */
template <class PNFA>
using pDFANode = DFANode<PNFA>*;
/* Lex */
template <>
unsigned int DFANode<lex::pNFANode>::maxId = 0;
template <>
std::map<unsigned int, std::set<pDFANode<lex::pNFANode>>> DFANode<lex::pNFANode>::mHash = {};
/* Syntax */
template <>
unsigned int DFANode<syntax::pSyntaxNFAData>::maxId = 0;
template <>
std::map<unsigned int, std::set<pDFANode<syntax::pSyntaxNFAData>>> DFANode<syntax::pSyntaxNFAData>::mHash = {};


template <class PNFA>
bool isSetEqual(const std::set<PNFA>& s1, const std::set<PNFA>& s2)
{
    auto it1 = s1.begin();
    auto it2 = s2.begin();
    unsigned n1 = s1.size(), n2 = s2.size();
    if (n1 != n2) return false;
    for (; it1 != s1.end() && it2 != s2.end(); it1++, it2++) {
        if (*it1 != *it2) return false;
    }

    return true;
}

template <class PNFA>
bool DFAConflict(pDFANode<PNFA> p1, pDFANode<PNFA> p2)
{
    if (p1->hash != p2->hash) return false;
    if (isSetEqual(p1->sNodeData, p2->sNodeData))
        return true;
    return false;
}

/* return nullptr if s is compatible with mHash, unless return pNFANode */
template <class PNFA>
std::pair<pDFANode<PNFA>, unsigned int> findSameNFASet(const std::set<PNFA>& s)
{
    unsigned int hash = jhin::comm::genHash<PNFA>(s);
    if (DFANode<PNFA>::mHash.find(hash) == DFANode<PNFA>::mHash.end()) return std::make_pair(nullptr, hash);
    for (pDFANode<PNFA> p: DFANode<PNFA>::mHash[hash]) {
        if (isSetEqual(p->sNodeData, s)) {
            return std::make_pair(p, hash);
        }
    }

    return std::make_pair(nullptr, hash);
}

template <class PNFA>
std::set<PNFA> genEPClosure(std::queue<PNFA>& qu, unsigned EP)
{
    std::set<PNFA> se;
    while (!qu.empty()) {
        PNFA n = qu.front();
        qu.pop();
        const std::vector<PNFA>& v = n->mNodes[EP];
        for (PNFA p: v) {
            if (se.find(p) == se.end()) {
                qu.push(p);
            }
        }
        se.insert(n);
    }

    return se;
}


/* handle DFA nodes, input a init node */
template <class PNFA>
void propagateDFA(pDFANode<PNFA> init, unsigned EP)
{
    std::queue<pDFANode<PNFA>> quDFAs;
    quDFAs.push(init);

    while (!quDFAs.empty()) {
        pDFANode<PNFA> pDFA = quDFAs.front();
        quDFAs.pop();

        const std::set<PNFA>& s = pDFA->sNodeData;
        /* m.first should be compatible with SyntaxNFAData */
        std::map<unsigned, std::set<PNFA>> m;
        for (PNFA p: s) {
            for (const auto& it: p->mNodes) {
                if (it.first != EP) {
                    for (PNFA pNode: it.second) {
                        m[it.first].insert(pNode);
                    }
                }
            }
        }

        /* mqu.first should be compatible with SyntaxNFAData */
        std::map<unsigned, std::queue<PNFA>> mqu;
        for (const auto& it: m) {
            for (PNFA pNode: it.second) {
                mqu[it.first].push(pNode);
            }
        }


        /* check compatibility everytime gen a new DFA node */
        for (auto& it: mqu) {
            std::set<PNFA> sNFA = genEPClosure<PNFA>(it.second, EP);
            std::pair<pDFANode<PNFA>, unsigned int> pa = findSameNFASet<PNFA>(sNFA);
            if (pa.first == nullptr) {
                /* gen new DFA node, connect and add to worklist */
                pDFANode<PNFA> pNew = new DFANode<PNFA>(pa.second, sNFA);
                pDFA->mEdges[it.first] = pNew;
                Log::singleton() >> "DFA_" >> pDFA->id >> ": \n" >> pDFA->toStringRaw() >> "\nconnect " >> it.first >> " to DFA_" >> pNew->id >> ": \n" >> pNew->toStringRaw() >> newline >> newline;
                /* add new node to the worklist */
                quDFAs.push(pNew);
            } else {
                Log::singleton() >> "DFA_" >> pDFA->id >> ": \n" >> pDFA->toStringRaw() >> "\nconnect " >> it.first >> " to DFA_" >> pa.first->id >> newline >> newline >> newline;
                /* if the node is already exist, do not add to the worklist */
                pDFA->mEdges[it.first] = pa.first;
            }
        }
    }
}

template <class RET, class PNFA, class... Ts>
using DFAFunc = RET(pDFANode<PNFA>, Ts...);

/* REPEAT: whether an old DFA node can be rehandled */
template <class RET = void, class PNFA = syntax::pSyntaxNFAData, bool REPEAT = false, class... Ts>
pDFANode<PNFA> travelDFA(pDFANode<PNFA> pStart,
                         DFAFunc<RET, PNFA, Ts...> f,
                         Ts... ts)
{
    std::queue<pDFANode<PNFA>> worklist;
    worklist.push(pStart);
    std::unordered_set<pDFANode<PNFA>> se = {pStart};

    if constexpr (REPEAT) {
        /* this branch should be processed in compile-time to make type-checker happy. */
        static_assert(std::is_same<RET, bool>::value, "RET should be type 'bool' when 'REPEAT' is true");

        while (!worklist.empty()) {
            pDFANode<PNFA> p = worklist.front(); worklist.pop();
            /* f 必须是一个单调递增有上界函数，或单调递减有下界函数，由调用方来保证 */
            bool bIsChanged = f(p, ts...);
            if (bIsChanged) {
                for (const auto& item: p->mEdges) {
                    worklist.push(item.second);
                }
            }
        }
    } else {
        while (!worklist.empty()) {
            pDFANode<PNFA> p = worklist.front(); worklist.pop();
            f(p, ts...);
            for (const auto& item: p->mEdges) {
                pDFANode<PNFA> pDFA = item.second;
                if (se.find(pDFA) == se.end()) {
                    worklist.push(pDFA);
                    se.insert(pDFA);
                }
            }
        }
    }

    return pStart;
}

/*
 * need to release the dfa node pointers
 * recursive reference
 * OR IT WILL TAKE TOO MUCH MEMORY
 * */
template <class PNFA>
bool releaseDFANodes(pDFANode<PNFA> init)
{
    // TODO
    // 后面再慢慢优化吧，代码要写不完惹QAQ
    // weak_ptr?
    return true;
}

/* minimal DFA */
void minimalDFA()
{
    // TODO
}


};  /* namespace lex */
};  /* namespace jhin */



#endif

