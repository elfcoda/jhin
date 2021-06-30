#pragma once

#include <string>

namespace jhin
{
namespace lex
{

struct metaInfo
{
    unsigned    row;
    unsigned    col;
    std::string filename;
};
using pMetaInfo = metaInfo*;


};  /* namespace lex */
};  /* namespace jhin */

