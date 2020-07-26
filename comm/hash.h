#ifndef __HASH_H__
#define __HASH_H__

#include <set>
#include "../src/lex/nfa.h"

namespace jhin
{
    namespace comm
    {
        /* for DFA */
        template <class NFA>
        unsigned genHash(const std::set<NFA>& vNodeData)
        {
            unsigned ans = 1;
            for (NFA p: vNodeData) ans *= p->id;

            return ans;
        }

        /* for NFA */
        unsigned genHash(unsigned nonTerminal, const std::vector<unsigned>& production, unsigned position)
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

