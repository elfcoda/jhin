#ifndef __HASH_H__
#define __HASH_H__

#include <set>
#include "../src/lex/nfa.h"

namespace jhin
{
    namespace comm
    {
        unsigned int genHash(const std::set<jhin::lex::pNFANode>& vNodeData)
        {
            unsigned ans = 1;
            for (jhin::lex::pNFANode p: vNodeData) ans *= p->id;

            return ans;
        }

    };  /* namespace tool*/
};  /* namespace jhin */





#endif

