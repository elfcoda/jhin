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


int main()
{
    std::vector<std::pair<unsigned, std::string>> lexResult = client::lexClient();
    comm::Log::singleton() >> "Token Id to String: \n" >> lex::tokenId2String >> comm::newline;

    /* append $ */
    lexResult.push_back(std::make_pair(SYNTAX_TOKEN_END, SYNTAX_TOKEN_END_MARK));

    /* syntaxResult */
    comm::pSyntaxDFA pDFAStart = client::syntaxClient();

    ast::pASTNode pRoot = client::astClient(lexResult, pDFAStart);

    client::tsClient(pRoot);

    return 0;
}


