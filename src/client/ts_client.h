#pragma once

#include <memory>
#include "../ts/ts.h"
#include "../ast/ast_node.h"

namespace jhin
{
namespace client
{

void tsClient(ast::pASTNode pRoot)
{
    std::shared_ptr<ts::TypeSystem> typeSystem = std::make_shared<ts::TypeSystem>();
    ts::pTypeTree pTT = typeSystem->genSymbolTable(pRoot);
}

};  /* namespace client */
};  /* namespace jhin */

