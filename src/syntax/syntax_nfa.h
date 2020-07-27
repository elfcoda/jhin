#ifndef __SYNTAX_NFA_H__
#define __SYNTAX_NFA_H__

#include <unordered_map>
#include <vector>
#include <map>
#include "../../comm/hash.h"


namespace jhin
{
namespace syntax
{

#define SYNTAX_TOKEN_END    1025

struct SyntaxNFAData;
using pSyntaxNFAData = SyntaxNFAData*;

/* new all possible nfa nodes when initing */
struct SyntaxNFAData
{
    /* hash -> SyntaxNFAData Nodes */
    static std::unordered_map<unsigned, std::vector<pSyntaxNFAData>> mHash;

    /* max nfa node id */
    static unsigned maxId;

    /* nfa node id */
    unsigned id;

    /* node hash of node data */
    unsigned hash;

    /* pointer */
    std::map<unsigned, std::vector<pSyntaxNFAData>> mNodes;

    /* node data */
    unsigned nonTerminal;
    std::vector<unsigned> production;
    unsigned position;

    SyntaxNFAData(unsigned nonTerminal, const std::vector<unsigned>& production, unsigned position)
    {
        this->nonTerminal = nonTerminal;
        this->production = production;
        this->position = position;
        this->hash = comm::genHash(nonTerminal, production, position);

        maxId += 1;
        id = maxId;
    }

    bool isDataSame(pSyntaxNFAData p)
    {
        if (this->hash != p->hash) return false;

        if (this->nonTerminal != p->nonTerminal) return false;
        if (this->position != p->position) return false;
        unsigned n1 = this->production.size(), n2 = p->production.size();
        if (n1 != n2) return false;
        for (unsigned i = 0; i < n1; i++) {
            if (this->production[i] != p->production[i]) return false;
        }

        return true;
    }
};
using pcSyntaxNFAData = const SyntaxNFAData*;
std::unordered_map<unsigned, std::vector<pSyntaxNFAData>> SyntaxNFAData::mHash = {};
unsigned SyntaxNFAData::maxId = 0;

enum SyntaxSymbolKind
{
    SYN_SYM_TOKEN = 1,
    SYN_SYM_NON_TERMINAL,
    SYN_SYM_EPSILON,
    SYN_SYM_UNKNOWN,
};




};  /* namespace syntax*/
};  /* namespace jhin */

#endif

