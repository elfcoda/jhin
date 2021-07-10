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

    Module *M = mdl::TheModule.get();
    outs() << "We just constructed this LLVM module:\n\n" << *M;

    auto TSM = llvm::orc::ThreadSafeModule(std::move(mdl::TheModule), std::move(mdl::TheContext));
    mdl::ExitOnErr(mdl::TheJIT->addModule(std::move(TSM)));
    mdl::InitializeModuleAndPassManager();

    auto ExprSymbol = mdl::ExitOnErr(mdl::TheJIT->lookup("func"));
    // Get the symbol's address and cast it to the right type (takes no
    // arguments, returns a double) so we can call it as a native function.
    double (*FP)() = (double (*)())(intptr_t)ExprSymbol.getAddress();
    fprintf(stderr, "Evaluated to %f\n", FP());


    // TODO: type checker

    // client::tsClient(pRoot);
    // outs() << "TS Completed.\n";

    return 0;
}


