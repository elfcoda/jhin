#pragma once

#include <memory>
#include <stack>
#include "client.h"
#include "lex_client.h"
#include "syntax_client.h"
#include "ast_client.h"
#include "ts_client.h"
#include "../../comm/log.h"
#include "../../comm/jhin_assert.h"

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

    symbolTable::initSymbolTable();

    std::unique_ptr<ast::ASTBase> base = client::astClient(lexResult, pDFAStart);
    outs() << "AST Completed: " << base->getASTName() << ", start to codegen\n";

    // clear symbol table to be reused in codegen module
    symbolTable::initSymbolTable();

    // codegen
    base->codegen();

    outs() << "We just constructed this LLVM module:\n" << *(mdl::TheModule.get());

    // client::tsClient(pRoot);
    // outs() << "TS Completed.\n";

    return 0;
}


