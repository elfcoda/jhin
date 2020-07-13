#ifndef __DFA_H__
#define __DFA_H__

#include <vector>
#include <map>
#include <queue>
#include "nfa.h"
#include "../tools/hash.h"

namespace jhin
{
namespace lex
{


/* DFA definition */
/* a DFANode can be generated only if the sNodeData is compatible with that in 'static mHash' */
struct DFANode
{
    /* hash map */
    static std::map<unsigned int, std::set<DFANode*>> mHash;

    /* record max node id*/
    static unsigned int maxId;

    /* node id */
    unsigned int id;

    /* node in NFA(normal and terminal) */
    /* std::set<unsigned int> sNodeData; */
    /* {1, 3, 23, 57}  should be same with {1, 23, 57, 3}, so sNodeData should be ordered for equality comparation */
    /* if pNFANode is ordered, pNFANode->id is in particular order as well */
    std::set<pNFANode> sNodeData;

    /* md5sum, used to compare sNodeData */
    unsigned int hash;

    /* edges */
    std::map<char, DFANode*> mEdges;

    DFANode(unsigned int hash)
    {
        /* id starts from 1 */
        maxId += 1;
        id = maxId;
        this->hash = hash;
        assert();
        assert(id < UINT_MAX);
    }
};
unsigned int DFANode::maxId = 0;
std::map<unsigned int, std::set<pNFANode>> DFANode::mHash = {};
using pDFANode = DFANode*;


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
pDFANode findSameNFASet(const std::set<pNFANode>& s)
{
    unsigned int hash = jhin::tool::genHash(s);
    if (DFANode::mHash.find(hash) == DFANode::mHash.end()) return nullptr;
    for (pDFANode p: DFANode::mHash[hash]) {
        if (isSetEqual(p->sNodeData, s)) {
            return p;
        }
    }

    return nullptr;
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

/* handle a DFA data */
std::map<char, std::queue<pNFANode>> getAvailableChar(pDFANode pDFA)
{
    const std::set<pNFANode>& s = pDFA->sNodeData;
    std::map<char, std::set<pNFANode>> m;
    std::map<char, std::queue<pNFANode>> mqu;
    for (pNFANode p: s) {
        for (const auto& it: p->mNodes) {
            if (it.first != EPSILON) {
                for (pNFANode pNode: it.second) {
                    m[it.first].insert(pNode);
                }
            }
        }
    }

    for (const auto& it: m) {
        for (pNFANode pNode: it.second) {
            mqu[it.first].push(pNode);
        }
    }

    return mqu;
}

/* check compatibility everytime gen a new DFA node */



std::set<pNFANode> genStepClosure(pNFANode node, char c)
{
    return std::set<pNFANode>{};
}

};  /* namespace lex */
};  /* namespace jhin */



#endif

