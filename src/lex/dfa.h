#ifndef __DFA_H__
#define __DFA_H__

#include <vector>
#include <map>
#include <queue>
#include "nfa.h"
#include "../../comm/hash.h"

namespace jhin
{
namespace lex
{


/* DFA definition */
/* a DFANode can be generated only if the sNodeData is compatible with that in 'static mHash' */
struct DFANode
{
    /* hash map, (hash value of sNodeData) -> (DFA set) */
    static std::map<unsigned int, std::set<DFANode*>> mHash;

    /* record max node id to increase id */
    static unsigned int maxId;

    /* dfa node id */
    unsigned int id;

    /* node in NFA(normal and terminal) */
    /* std::set<unsigned int> sNodeData; */
    /* {1, 3, 23, 57}  should be same with {1, 23, 57, 3}, so sNodeData should be ordered for equality comparation */
    /* if pNFANode is ordered, pNFANode->id is in particular order as well */
    std::set<pNFANode> sNodeData;

    /* nfa terminal node id  */
    /* non-terminal node if terminalId == UINT_MAX */
    unsigned int terminalId;

    /* md5sum, used to compare sNodeData */
    unsigned int hash;

    /* edges */
    std::map<char, DFANode*> mEdges;

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

    void setTerminalId()
    {
        terminalId = UINT_MAX;
        for (pNFANode p: sNodeData) {
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
unsigned int DFANode::maxId = 0;
using pDFANode = DFANode*;
std::map<unsigned int, std::set<pDFANode>> DFANode::mHash = {};


template <class T>
bool isSetEqual(const std::set<T>& s1, const std::set<T>& s2)
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

bool DFAConflict(pDFANode p1, pDFANode p2)
{
    if (p1->hash != p2->hash) return false;
    if (isSetEqual(p1->sNodeData, p2->sNodeData))
        return true;
    return false;
}

/* return nullptr if s is compatible with mHash, unless return pNFANode */
std::pair<pDFANode, unsigned int> findSameNFASet(const std::set<pNFANode>& s)
{
    unsigned int hash = jhin::comm::genHash(s);
    if (DFANode::mHash.find(hash) == DFANode::mHash.end()) return std::make_pair(nullptr, hash);
    for (pDFANode p: DFANode::mHash[hash]) {
        if (isSetEqual(p->sNodeData, s)) {
            return std::make_pair(p, hash);
        }
    }

    return std::make_pair(nullptr, hash);
}

std::set<pNFANode> genEPClosure(std::queue<pNFANode>& qu)
{
    std::set<pNFANode> se;
    while (!qu.empty()) {
        pNFANode n = qu.front();
        qu.pop();
        const std::vector<pNFANode>& v = n->mNodes[EPSILON];
        for (pNFANode p: v) {
            if (se.find(p) == se.end()) {
                qu.push(p);
            }
        }
        se.insert(n);
    }

    return se;
}


/* handle DFA nodes, input a init node */
void propagateDFA(pDFANode init)
{
    std::queue<pDFANode> quDFAs;
    quDFAs.push(init);

    while (!quDFAs.empty()) {
        pDFANode pDFA = quDFAs.front();
        quDFAs.pop();

        const std::set<pNFANode>& s = pDFA->sNodeData;
        std::map<char, std::set<pNFANode>> m;
        for (pNFANode p: s) {
            for (const auto& it: p->mNodes) {
                if (it.first != EPSILON) {
                    for (pNFANode pNode: it.second) {
                        m[it.first].insert(pNode);
                    }
                }
            }
        }

        std::map<char, std::queue<pNFANode>> mqu;
        for (const auto& it: m) {
            for (pNFANode pNode: it.second) {
                mqu[it.first].push(pNode);
            }
        }


        /* check compatibility everytime gen a new DFA node */
        for (auto& it: mqu) {
            std::set<pNFANode> sNFA = genEPClosure(it.second);
            std::pair<pDFANode, unsigned int> pa = findSameNFASet(sNFA);
            if (pa.first == nullptr) {
                /* gen new DFA node, connect and add to worklist */
                pDFANode pNew = new DFANode(pa.second, sNFA);
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
bool releaseDFANodes(pDFANode init)
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

