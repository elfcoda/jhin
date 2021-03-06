#pragma once

#include <memory>
#include <stack>
#include "client.h"
#include "lex_client.h"
#include "syntax_client.h"
#include "ast_client.h"
#include "ts_client.h"
#include "../../comm/log.h"

using namespace jhin;

int compiler()
{
    std::vector<std::pair<unsigned, std::string>> lexResult = client::lexClient();
    comm::Log::singleton() >> "Token Id to String: \n" >> lex::tokenId2String >> comm::newline;

    /* append $ */
    lexResult.push_back(std::make_pair(SYNTAX_TOKEN_END, SYNTAX_TOKEN_END_MARK));
    outs() << "Lex Completed.\n";

    /* syntaxResult */
    comm::pSyntaxDFA pDFAStart = client::syntaxClient();
    outs() << "Syntax Completed.\n";

    std::unique_ptr<ast::ASTBase> base = client::astClient(lexResult, pDFAStart);
    outs() << "AST Completed: " << base->getASTName() << ", start to codegen\n";

    base->codegen();


    // TODO: type checker

    // client::tsClient(pRoot);
    // outs() << "TS Completed.\n";

    return 0;
}


