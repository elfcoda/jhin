#ifndef __HASH_H__
#define __HASH_H__

#include <set>
#include "../src/lex/nfa.h"

namespace jhin
{
    namespace comm
    {
        unsigned genHash(const std::set<jhin::lex::pNFANode>& vNodeData)
        {
            unsigned ans = 1;
            for (jhin::lex::pNFANode p: vNodeData) ans *= p->id;

            return ans;
        }

        unsigned genHash(unsigned nonTerminal, const std::vector<unsigned>>& production, unsigned position)
        {
            unsigned ans = 1;
            ans *= nonTerminal;
            for (unsigned id: production) ans *= id;
            ans += position;

            return ans;
        }

    };  /* namespace tool*/
};  /* namespace jhin */





#endif

