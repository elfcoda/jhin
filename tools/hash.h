#ifndef __HASH_H__
#define __HASH_H__

#include <set>
#include "nfa.h"

namespace jhin
{
    namespace tools
    {
        unsigned int genHash(const std::set<pNFANode>& vNodeData)
        {
            unsigned ans = 1;
            for (pNFANode p: vNodeData) ans *= p->id;

            return ans;
        }

    };  /* namespace tool*/
};  /* namespace jhin */





#endif

