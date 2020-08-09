#ifndef __PT_CLIENT_H__
#define __PT_CLIENT_H__

#include <memory>
#include "../ast/pt.h"

namespace jhin
{
namespace client
{

void astClient(const std::vector<std::pair<unsigned, std::string>>& lexResult, comm::pSyntaxDFA pDFAStart)
{
    std::shared_ptr<ast::ParseTree> pt = std::make_shared<ast::ParseTree>();

    pt->parse(lexResult, pDFAStart);
}

};  /* namespace client */
};  /* namespace jhin */


#endif

