#include <iostream>
#include <memory>
#include <stack>
#include "client.h"
#include "lex_client.h"
#include "syntax_client.h"
#include "ast_client.h"
#include "ts_client.h"
#include "../../comm/log.h"


int main()
{
    std::vector<std::pair<unsigned, std::string>> lexResult = jhin::client::lexClient();
    jhin::comm::Log::singleton() >> "Token Id to String: \n" >> jhin::lex::tokenId2String >> jhin::comm::newline;

    /* append $ */
    lexResult.push_back(std::make_pair(SYNTAX_TOKEN_END, SYNTAX_TOKEN_END_MARK));

    /* syntaxResult */
    jhin::comm::pSyntaxDFA pDFAStart = jhin::client::syntaxClient();

    jhin::client::astClient(lexResult, pDFAStart);

    return 0;
}


