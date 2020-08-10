#pragma once

#include <string>
#include "log.h"
#include "../src/syntax/non_terminal.h"
#include "../src/lex/keywords.h"

namespace jhin
{
namespace comm
{

enum SymbolType
{
    SymbolNone = 0,
    SymbolNonterminal,
    SymbolToken,
    SymbolEPSILON,
    SymbolTokenBegin,
    SymbolTokenEnd,
};

SymbolType symbolId2Type(unsigned symbolId)
{
    if (syntax::id_to_non_terminal.find(symbolId) != syntax::id_to_non_terminal.end())
        return SymbolNonterminal;
    else if (lex::tokenId2String.find(symbolId) != lex::tokenId2String.end())
        return SymbolToken;
    else if (symbolId == SYNTAX_EPSILON_IDX)
        return SymbolEPSILON;
    else if (symbolId == SYNTAX_TOKEN_BEGIN)
        return SymbolTokenBegin;
    else if (symbolId == SYNTAX_TOKEN_END)
        return SymbolTokenEnd;

    Log::singleton(WARN) >> "symbolId not found!" >> newline;
    return SymbolNone;
}

std::string symbolId2String(unsigned symbolId)
{
    std::string s = "";

    if (syntax::id_to_non_terminal.find(symbolId) != syntax::id_to_non_terminal.end()) {
        s += syntax::id_to_non_terminal[symbolId];
    } else if (lex::tokenId2String.find(symbolId) != lex::tokenId2String.end()) {
        s += lex::tokenId2String[symbolId];
    } else if (symbolId == SYNTAX_EPSILON_IDX) {
        s += SYNTAX_EPSILON_STR;
    } else if (symbolId == SYNTAX_TOKEN_BEGIN) {
        s += SYNTAX_TOKEN_BEGIN_MARK;
    } else if (symbolId == SYNTAX_TOKEN_END) {
        s += SYNTAX_TOKEN_END_MARK;
    } else {
        Log::singleton(WARN) >> "symbolId not found!" >> newline;
        s += std::to_string(symbolId);
    }

    return s;
}

};  /* namespace comm */
};  /* namespace jhin */


