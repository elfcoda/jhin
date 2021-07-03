#pragma once

#include <set>
#include "log.h"
#include "../src/lex/nfa.h"

namespace jhin
{
    namespace comm
    {
        /* for DFA */
        template <class PNFA>
        unsigned genHash(const std::set<PNFA>& vNodeData)
        {
            unsigned ans = 1;
            for (PNFA p: vNodeData) ans *= p->id;

            return ans;
        }

        /* for PNFA */
        unsigned genHash(unsigned nonTerminal, const std::vector<unsigned>& production, unsigned position)
        {
            unsigned ans = 1;
            ans *= nonTerminal;
            for (unsigned id: production) ans *= id;
            ans += position;

            return ans;
        }

    }   /* namespace tool*/
}   /* namespace jhin */






