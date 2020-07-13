#ifndef __HASH_H__
#define __HASH_H__

#include <set>
#include "nfa.h"

unsigned genHash(const std::set<pNFANode>& vNodeData)
{
    unsigned ans = 1;
    for (pNFANode p: vNodeData) ans *= p->id;

    return ans;
}





#endif

