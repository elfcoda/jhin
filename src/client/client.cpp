#include <iostream>
#include <memory>
#include <stack>
#include "client.h"
#include "lex_client.h"
#include "syntax_client.h"
#include "../../comm/log.h"

int main()
{
    std::vector<std::pair<unsigned, std::string>> lexResult = jhin::client::lexClient();
    jhin::comm::Log::singleton() >> "Token Id to String: \n" >> jhin::lex::tokenId2String >> jhin::comm::newline;

    /* append $ */
    lexResult.push_back(std::make_pair(SYNTAX_TOKEN_END, SYNTAX_TOKEN_END_MARK));

    /* syntaxResult */
    jhin::comm::pSyntaxDFA pDFAStart = jhin::client::syntaxClient();

    std::stack<std::pair<unsigned, jhin::comm::pSyntaxDFA>> st;
    st.push(std::make_pair(SYNTAX_TOKEN_BEGIN, pDFAStart));

    jhin::syntax::pSyntaxNFAData pNFA = nullptr;
    for (unsigned idx = 0; idx < lexResult.size(); ) {
        const std::pair<unsigned, std::string>& item = lexResult[idx];
        if ((pNFA = st.top().second->canReduce(item.first)) == nullptr) {
            /* shift:
             * Prog'  should shift $ to nullptr
             * */
            st.push(std::make_pair(item.first, st.top().second->mEdges[item.first]));
            idx ++;
        } else {
            /* reduce */
            if (!pNFA->isEPSILON()) {
                /* production E -> EPSILON. should not pop stack */
                jhin::client::stackPopN(st, pNFA->production.size());
            }
            st.push(std::make_pair(pNFA->nonTerminal, st.top().second->mEdges[pNFA->nonTerminal]));
        }
    }
    /* $ has been shifted
     * stack: <'#': pStart, '$': nullptr>
     * */


    return 0;
}
