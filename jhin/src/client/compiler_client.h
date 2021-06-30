#pragma once

#include <iostream>
#include <memory>
#include <stack>
#include "client.h"
#include "lex_client.h"
#include "syntax_client.h"
#include "ast_client.h"
#include "ts_client.h"
#include "../../comm/log.h"

using namespace jhin;
using namespace std;


int compiler()
{
    std::vector<std::pair<unsigned, std::string>> lexResult = client::lexClient();
    comm::Log::singleton() >> "Token Id to String: \n" >> lex::tokenId2String >> comm::newline;

    /* append $ */
    lexResult.push_back(std::make_pair(SYNTAX_TOKEN_END, SYNTAX_TOKEN_END_MARK));
    cout << "Lex Completed." << endl;

    /* syntaxResult */
    comm::pSyntaxDFA pDFAStart = client::syntaxClient();
    cout << "Syntax Completed." << endl;

    ast::pASTNode pRoot = client::astClient(lexResult, pDFAStart);
    cout << "AST Completed." << endl;

    client::tsClient(pRoot);
    cout << "TS Completed." << endl;

    return 0;
}


