#pragma once

#include <memory>
#include "../ast/pt.h"
#include "../ast/ast.h"
#include "../ast/ast_node.h"
#include "../ast/ast_node_semantic.h"

namespace jhin
{
namespace client
{

std::unique_ptr<ast::ASTBase> astClient(const std::vector<std::pair<unsigned, std::string>>& lexResult, comm::pSyntaxDFA pDFAStart)
{
    std::shared_ptr<ast::ParseTree> pt = std::make_shared<ast::ParseTree>();
    ast::pASTNode pRoot = pt->parse(lexResult, pDFAStart);

    std::shared_ptr<ast::AST> astTree = std::make_shared<ast::AST>();
    // pRoot = astTree->parseTree2AST(pRoot);

    std::unique_ptr<ast::ASTBase> base = astTree->parseTree2LLVMAST(pRoot);
    return base;
}

}   /* namespace client */
}   /* namespace jhin */



