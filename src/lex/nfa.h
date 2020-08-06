#ifndef __NFA_H__
#define __NFA_H__

#include <vector>
#include <map>
#include <cassert>
#include "keywords.h"

namespace jhin
{
namespace lex
{

/* NFA definition */
/* different NFANode address must corresponse to different node id */
struct NFANode
{
    /* current max normal node id */
    /* maxId maintain the old value even if normal node id is updated */
    static unsigned int maxId;

    /* 0~65536: normal node, 65538~MAXINT: terminal node */
    unsigned int id;

    /*
     * edges, and Epsilon is representd as '#'
     * but '#' is not a chacracter of this language
     * for the reason, this lex analyser will complain when feeded with '#'
     * */
    // to get compatible with syntax nfa epsilon when dealing with DFA parameter
    std::map<char, std::vector<NFANode*>> mNodes;
    // std::map<unsigned, std::vector<NFANode*>> mNodes;

    /* normal node */
    NFANode()
    {
        /* id starts from 1 */
        maxId += 1;
        id = maxId;
        assert(id < TERMINATOR);
    }

    /* initial node */
    NFANode(EInitialId initialId)
    {
        id = static_cast<unsigned int>(initialId);
    }

    /* terminal node */
    NFANode(EKeyWords terminalId)
    {
        id = static_cast<unsigned int>(terminalId);
    }

    /* error node */
    NFANode(EError terminalId)
    {
        id = static_cast<unsigned int>(terminalId);
    }

    /* switch normal node to terminal node */
    void setId(unsigned int terminalId)
    {
        id = terminalId;
    }

    /* implement toString() */
    std::string toString()
    {
        return std::to_string(id);
    }
};
unsigned int NFANode::maxId = 0;
using pNFANode = NFANode*;
using pNFAPair = std::pair<pNFANode, pNFANode>;


pNFAPair genSingle(char c)
{
    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    pStart->mNodes[c] = std::vector<pNFANode>{pEnd};

    return std::make_pair(pStart, pEnd);
}

pNFAPair genEpsilon()
{
    return genSingle(EPSILON);
}

/* 0-9 a-z A-Z */
pNFAPair genOrByRange(char start, char end)
{
    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    for (char c = start; c <= end; c++) {
        pNFANode p1 = new NFANode();
        pNFANode p2 = new NFANode();
        p1->mNodes[c] = std::vector<pNFANode>{p2};
        p2->mNodes[EPSILON] = std::vector<pNFANode>{pEnd};
        pStart->mNodes[EPSILON].push_back(p1);
    }

    return std::make_pair(pStart, pEnd);
}

#define GENTYPE_DIGITS  1
#define GENTYPE_LOWERC  2
#define GENTYPE_UPPERC  3
pNFAPair genOrByRange(int gentype)
{
    if (gentype == GENTYPE_DIGITS) return genOrByRange('0', '9');
    if (gentype == GENTYPE_LOWERC) return genOrByRange('a', 'z');
    if (gentype == GENTYPE_UPPERC) return genOrByRange('A', 'Z');

    assert(false);
    return std::make_pair(nullptr, nullptr);
}

pNFAPair genOrByString(const std::string& s)
{
    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    for (char c: s) {
        pNFANode p1 = new NFANode();
        pNFANode p2 = new NFANode();
        p1->mNodes[c] = std::vector<pNFANode>{p2};
        p2->mNodes[EPSILON] = std::vector<pNFANode>{pEnd};
        pStart->mNodes[EPSILON].push_back(p1);
    }

    return std::make_pair(pStart, pEnd);
}

pNFAPair genAndByString(const std::string& s)
{
    assert(s != "");
    pNFANode pStart = new NFANode();
    pNFANode p1 = pStart, p2 = nullptr;
    for (char c: s) {
        p2 = new NFANode();
        p1->mNodes[c] = std::vector<pNFANode>{p2};
        p1 = p2;
    }

    return std::make_pair(pStart, p2);
}

pNFAPair genStar(const pNFAPair& M)
{
    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    pStart->mNodes[EPSILON] = std::vector<pNFANode>{pEnd};
    pEnd->mNodes[EPSILON] = std::vector<pNFANode>{M.first};
    M.second->mNodes[EPSILON] = std::vector<pNFANode>{pEnd};

    return std::make_pair(pStart, pEnd);
}

pNFAPair connectAndNodes(const pNFAPair& M1, const pNFAPair& M2)
{
    assert(M1 != M2);
    M1.second->mNodes[EPSILON].push_back(M2.first);

    return std::make_pair(M1.first, M2.second);
}

pNFAPair connectAndNodesV(const std::vector<pNFAPair>&& M)
{
    int n = M.size();
    assert(n >= 2);

    for (int i = 1; i < n; i++) {
        M[i-1].second->mNodes[EPSILON].push_back(M[i].first);
    }

    return std::make_pair(M[0].first, M[n-1].second);
}

pNFAPair connectOrNodes(const pNFAPair& M1, const pNFAPair& M2)
{
    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    pStart->mNodes[EPSILON] = std::vector<pNFANode>{M1.first, M2.first};
    M1.second->mNodes[EPSILON].push_back(pEnd);
    M2.second->mNodes[EPSILON].push_back(pEnd);

    return std::make_pair(pStart, pEnd);
}

/* [a-z]? OR c?, aka. M | EPSILON */
pNFAPair genExist(const pNFAPair& M)
{
    pNFAPair ME = genEpsilon();
    return connectOrNodes(M, ME);
}

pNFAPair connectOrNodesV(const std::vector<pNFAPair>&& M)
{
    assert(M.size() >= 2);

    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    for (const pNFAPair& m: M) {
        pStart->mNodes[EPSILON].push_back(m.first);
        m.second->mNodes[EPSILON].push_back(pEnd);
    }

    return std::make_pair(pStart, pEnd);
}

/*
 * INT     [0-9]+
 * DECIMAL [0-9]+.[0-9]*(e(+|-)?[0-9]+)?   -- may be float or double
 * ID      [a-z](a-zA-Z0-9|_)*
 * VALUE   [A-Z](a-zA-Z|_)*  -- value constructor
 * STRING  "$<any_but_douquo>*"
 * */
pNFAPair genReINT()
{
    pNFAPair M1 = genOrByRange(GENTYPE_DIGITS);
    pNFAPair M2 = genOrByRange(GENTYPE_DIGITS);
    pNFAPair M2Star = genStar(M2);

    return connectAndNodes(M1, M2Star);
}

pNFAPair genReDECIMAL()
{
    pNFAPair M1 = genReINT();
    pNFAPair M2 = genSingle('.');
    pNFAPair M3 = genStar(genOrByRange(GENTYPE_DIGITS));

    pNFAPair t0 = genSingle('e');
    pNFAPair t1 = genOrByString("+-");
    pNFAPair t2 = genExist(t1);
    pNFAPair t3 = genReINT();
    pNFAPair t4 = connectAndNodesV({t0, t2, t3});
    pNFAPair M4 = genExist(t4);

    pNFAPair M = connectAndNodesV({M1, M2, M3, M4});

    return M;
}

pNFAPair genReID()
{
    pNFAPair M1 = genOrByRange(GENTYPE_LOWERC);

    pNFAPair t0 = genOrByRange(GENTYPE_LOWERC);
    pNFAPair t1 = genOrByRange(GENTYPE_UPPERC);
    pNFAPair t2 = genOrByRange(GENTYPE_DIGITS);
    pNFAPair t3 = genSingle('_');
    pNFAPair t4 = connectOrNodesV({t0, t1, t2, t3});

    pNFAPair M2 = genStar(t4);

    return connectAndNodes(M1, M2);
}

pNFAPair genReVALUE()
{
    pNFAPair M1 = genOrByRange(GENTYPE_UPPERC);

    pNFAPair t0 = genOrByRange(GENTYPE_LOWERC);
    pNFAPair t1 = genOrByRange(GENTYPE_UPPERC);
    pNFAPair t2 = genSingle('_');
    pNFAPair t3 = connectOrNodesV({t0, t1, t2});

    pNFAPair M2 = genStar(t3);

    return connectAndNodes(M1, M2);
}

pNFAPair genReSTRING()
{
    pNFANode p1 = new NFANode();
    pNFANode p2 = new NFANode();
    pNFANode p3 = new NFANode();
    p1->mNodes['\"'].push_back(p2);
    p2->mNodes['\"'].push_back(p3);
    p2->mNodes[CHAR_NOT_DOUQUO].push_back(p2);

    return std::make_pair(p1, p3);
}

void connectAndSetRe(pNFANode init, pNFAPair M, ERESymbol terminalId)
{
    init->mNodes[EPSILON].push_back(M.first);
    M.second->setId(static_cast<unsigned int>(terminalId));
}

/*
 * need to release nfa node pointers
 * recursive reference
 * OR IT WILL TAKE TOO MUCH MEMORY
 * */
bool releaseNFANodes(pNFANode init)
{
    // TODO
    // 后面再慢慢优化吧，代码要写不完惹QAQ
    // weak_ptr?
    return true;
}


};  /* namespace lex */
};  /* namespace jhin */


#endif

