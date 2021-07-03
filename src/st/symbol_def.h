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
    std::string asmCode;

    symbolGenRtn(pTypeTree pTT, const std::string& asmCode): pTT(pTT), asmCode(asmCode) {}
    pTypeTree getTypeTreePtr() { return pTT; }
    std::string getAsmCode() { return asmCode; }
};

}   /* namespace st */
}   /* namespace jhin */


