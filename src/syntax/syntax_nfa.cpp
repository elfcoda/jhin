#include <unordered_map>
#include <unordered_set>
#include "non_terminal.h"
#include "../lex/nfa.h"
#include "../lex/keywords.h"

namespace jhin
{
namespace syntax
{

unsigned getSymbolID(const std::string& s)
{
    if (non_terminal_to_id.find(s) != non_terminal_to_id.end()) {
        return non_terminal_to_id[s];
    } else if (lex::string2TokenId.find(s) != lex::string2TokenId.end()) {
        return lex::string2TokenId[s];
    }

    /* ERROR: s not found, please check your syntax file */
    assert(false);
    return 0;
}

auto switchString2ID()
{
    /* pre-check */
    for (const auto& item: non_terminal_to_id) {
        if (lex::string2TokenId.find(item.first) != lex::string2TokenId.end()) {
            /* non-terminal's name is conflict with that of TOKEN NAME, gen nfa failed */
            assert(false);
        }
    }

    /* keyID -> production vectors, pair: production -> current LR(1) idx */
    std::unordered_map<unsigned, std::vector<std::pair<std::vector<unsigned>, unsigned>>> productionIDs;

    for (const auto& item: all_production) {
        unsigned keyID = getSymbolID(item.first);
        std::vector<std::pair<std::vector<unsigned>, unsigned>> vv;
        for (const auto& item2: item.second) {
            std::vector<unsigned> v;
            for (const auto& strElement: item2) {
                v.push_back(getSymbolID(strElement));
            }
            vv.emplace_back(std::make_pair(v, 0));
        }
        productionIDs[keyID] = vv;
    }

    return productionIDs;
}

class Syntax
{
    public:
        std::unordered_set genFirstSet()
        {
            std::unordered_set s;

            return s;
        }

        auto genNFA()
        {

        }

        auto parse()
        {
        }

        bool init()
        {
            NFANode::maxId = 0;

            return true;
        }

};



};  /* namespace syntax*/
};  /* namespace jhin */

int main(){

    return 0;
}


