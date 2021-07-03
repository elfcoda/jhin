#pragma once

#include <memory>
#include "../ast/pt.h"
#include "../ast/ast.h"
#include "../ast/ast_node.h"

namespace jhin
{
namespace client
{

ast::pASTNode astClient(const std::vector<std::pair<unsigned, std::string>>& lexResult, comm::pSyntaxDFA pDFAStart)
{
    std::shared_ptr<ast::ParseTree> pt = std::make_shared<ast::ParseTree>();
    ast::pASTNode pRoot = pt->parse(lexResult, pDFAStart);

    std::shared_ptr<ast::AST> ast = std::make_shared<ast::AST>();
    pRoot = ast->parseTree2AST(pRoot);
    return pRoot;
}

}   /* namespace client */
}   /* namespace jhin */



