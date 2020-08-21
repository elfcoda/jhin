#pragma once

#include <string>
#include "log.h"
#include "../src/lex/keywords.h"
#include "../src/syntax/non_terminal.h"
#include "../src/ast/ast_leaf.h"
#include "../src/ast/ast_non_leaf.h"

namespace jhin
{
namespace comm
{

bool isNonTerminal(unsigned id)
{
    return syntax::id_to_non_terminal.find(id) != syntax::id_to_non_terminal.end();
}

bool isToken(unsigned id)
{
    return lex::tokenId2String.find(id) != lex::tokenId2String.end();
}

unsigned getTokenLeafId(unsigned token)
{
    if (lex::tokenId2String.find(token) == lex::tokenId2String.end()) return 0;
    std::string tokenString = lex::tokenId2String[token];

    if (ast::ast_string_to_leafid.find(tokenString) != ast::ast_string_to_leafid.end())
        return ast::ast_string_to_leafid.at(tokenString);

    return 0;
}

unsigned getTokenLeafId(const std::string& tokenString)
{
    if (lex::string2TokenId.find(tokenString) == lex::string2TokenId.end()) return 0;

    if (ast::ast_string_to_leafid.find(tokenString) != ast::ast_string_to_leafid.end())
        return ast::ast_string_to_leafid.at(tokenString);

    return 0;
}

bool isTokenLeaf(unsigned token)
{
    if (lex::tokenId2String.find(token) == lex::tokenId2String.end()) return false;
    std::string tokenString = lex::tokenId2String[token];

    return ast::ast_string_to_leafid.find(tokenString) != ast::ast_string_to_leafid.end();
}

bool isTokenLeaf(const std::string& tokenString)
{
    if (lex::string2TokenId.find(tokenString) == lex::string2TokenId.end()) return false;

    return ast::ast_string_to_leafid.find(tokenString) != ast::ast_string_to_leafid.end();
}

/* ast
 * symbol should be new ast symbol */
bool isASTSymbolLeaf(unsigned symbol)
{
    return ast::ast_leafid_to_string.find(symbol) != ast::ast_leafid_to_string.end();
}

bool isASTSymbolNonLeaf(unsigned symbol)
{
    return ast::mASTNonLeafId2Str.find(symbol) != ast::mASTNonLeafId2Str.end();
}

/* defined for syntax analysis*/
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

std::string symbolId2String(unsigned symbolId, bool readable = true)
{
    std::string s = "";

    if (syntax::id_to_non_terminal.find(symbolId) != syntax::id_to_non_terminal.end()) {
        s += syntax::id_to_non_terminal[symbolId];
    } else if (lex::tokenId2String.find(symbolId) != lex::tokenId2String.end()) {
        std::string tokenString = lex::tokenId2String[symbolId];
        if (readable && syntax::token_string_to_symbol.find(tokenString) != syntax::token_string_to_symbol.end()) {
            s += syntax::token_string_to_symbol[tokenString];
        } else {
            s += tokenString;
        }
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


