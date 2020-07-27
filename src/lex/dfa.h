#ifndef __DFA_H__
#define __DFA_H__

#include <vector>
#include <map>
#include <queue>
#include "nfa.h"
#include "../syntax/syntax_nfa.h"
#include "../../comm/hash.h"

namespace jhin
{
namespace lex
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

    /* nfa terminal node id  */
    /* non-terminal node if terminalId == UINT_MAX */
    unsigned int terminalId;

    /* md5sum, used to compare sNodeData */
    unsigned int hash;

    /* edges */
    /* std::map<char, DFANode*> mEdges;
     * to get compatible with SyntaxNFAData
     */
    std::map<unsigned, DFANode<PNFA>*> mEdges;

    DFANode(unsigned int hash, std::set<pNFANode> sNFA)
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

    void setTerminalId()
    {
        terminalId = UINT_MAX;
        for (PNFA p: sNodeData) {
            /* ERROR first, not in charset error */
            if (p->id == static_cast<unsigned int>(ERR_ERROR)) {
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
unsigned int DFANode<pNFANode>::maxId = 0;
template <>
std::map<unsigned int, std::set<pDFANode<pNFANode>>> DFANode<pNFANode>::mHash = {};
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
        std::map<char, std::set<PNFA>> m;
        for (PNFA p: s) {
            for (const auto& it: p->mNodes) {
                if (it.first != EP) {
                    for (PNFA pNode: it.second) {
                        m[it.first].insert(pNode);
                    }
                }
            }
        }

        std::map<char, std::queue<PNFA>> mqu;
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
                /* add new node to the worklist */
                quDFAs.push(pNew);
            } else {
                /* if the node is already exist, do not add to the worklist */
                pDFA->mEdges[it.first] = pa.first;
            }
        }
    }
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

