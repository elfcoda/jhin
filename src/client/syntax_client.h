#ifndef __SYNTAX_CLIENT_H__
#define __SYNTAX_CLIENT_H__

#include <memory>
#include "../syntax/syntax.h"

namespace jhin
{
namespace client
{

jhin::comm::pSyntaxDFA syntaxClient()
{
    std::shared_ptr<jhin::syntax::Syntax> syn = std::make_shared<jhin::syntax::Syntax>();
    jhin::comm::pSyntaxDFA pDFAStart = syn->parse();

    assert(pDFAStart != nullptr);

    return pDFAStart;
}


};  /* namespace client */
};  /* namespace jhin */

#endif

