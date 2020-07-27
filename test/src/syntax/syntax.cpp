#include <iostream>
#include "../../../src/syntax/syntax.h"
#include "../../../src/lex/keywords.h"

int main()
{
    jhin::lex::setTokenId2String();
    jhin::syntax::Syntax* syn = new jhin::syntax::Syntax();
    bool b = syn->parse();
    std::cout << b << std::endl;

    return 0;
}
