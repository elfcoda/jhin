#pragma once

#include <memory>
#include "../ts/ts.h"
#include "../ast/ast_node.h"

namespace jhin
{
namespace client
{

ts::pTypeTree tsClient(ast::pASTNode pRoot)
{
    std::shared_ptr<ts::TypeSystem> typeSystem = std::make_shared<ts::TypeSystem>();
    ts::pTypeTree pTT = typeSystem->genSymbolTable(pRoot);
    return pTT;
}

};  /* namespace client */
};  /* namespace jhin */

