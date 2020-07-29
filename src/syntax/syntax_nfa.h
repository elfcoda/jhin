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

    bool canReduce()
    {
        return position == production.size();
    }

    std::string toString()
    {
        std::string s = "";
        s += id_to_non_terminal[nonTerminal];
        s += "\t-> ";
        std::string dot = ".";
        for (unsigned idx = 0; idx < production.size(); idx++) {
            if (idx == position) s += dot;
            id = production[idx];
            if (id == SYNTAX_EPSILON_IDX) s += "EPSILON ";
            else if (id < SYNTAX_EPSILON_IDX) s += id_to_non_terminal[id] + " ";
            else s += lex::tokenId2String[id] + " ";
        }

        if (position == production.size()) s += dot;

        return s;
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

enum SyntaxConflictType
{
    SYNTAX_CONFLICT_SHIFT_REDUCE = 1,
    SYNTAX_CONFLICT_REDUCE_REDUCE,
    SYNTAX_CONFLICT_NORMAL,
};



};  /* namespace syntax*/
};  /* namespace jhin */

#endif

