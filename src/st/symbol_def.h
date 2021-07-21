#pragma once

#include <string>
#include "../../comm/type_tree.h"


namespace jhin
{
namespace st
{

struct symbolGenRtn
{
    pTypeTree pTT;

    symbolGenRtn(pTypeTree pTT): pTT(pTT) {}
    pTypeTree getTypeTreePtr() { return pTT; }
};

}   /* namespace st */
}   /* namespace jhin */


