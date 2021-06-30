#pragma once

#include <memory>
#include "../st/symbol_gen.h"
#include "../ast/ast_node.h"

namespace jhin
{
namespace client
{

comm::pTypeTree tsClient(ast::pASTNode pRoot)
{
    std::shared_ptr<st::SymbolGen> SG = std::make_shared<st::SymbolGen>();
    comm::pTypeTree pTT = SG->genSymbolTable(pRoot)->getTypeTreePtr();
    return pTT;
}

};  /* namespace client */
};  /* namespace jhin */

