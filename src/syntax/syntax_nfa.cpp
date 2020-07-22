#include <unordered_map>
#include <unordered_set>
#include "non_terminal.h"
#include "../lex/nfa.h"
#include "../lex/keywords.h"

namespace jhin
{
namespace syntax
{

#define SYNTAX_TOKEN_END    1025

enum SyntaxSymbolKind
{
    SYN_SYM_TOKEN = 1,
    SYN_SYM_NON_TERMINAL,
    SYN_SYM_EPSILON,
    SYN_SYM_UNKNOWN,
};

class Syntax
{
    public:
        std::unordered_set<unsigned> genFirstSetTerminal(unsigned id)
        {
            return std::unordered_set<unsigned>{id};
        }

        /* can include EPSILON */
        std::unordered_set genFirstSetNonTerminal(unsigned id)
        {
            /*
             * NOTATION:
             * if { P1 -> P2 m n }, and
             *    { P2 -> EPSILON }
             *  then EPSILON IS NOT a member of the first set of P1
             *  */
            std::unordered_set<unsigned> s;
            std::queue<unsigned> worklist;
            worklist.push(id);
            unordered_set<unsigned> handledNonTer{id};

            while (!worklist.empty()) {
                unsigned i = worklist.top(); worklist.pop();
                assert((productionIDs.find(i) != productionIDs.end()));
                for (const auto& vec: productionIDs[i]) {
                    assert(!vec.empty());
                    if (vec[0] < SYNTAX_EPSILON_IDX) {
                        /* new non-terminal symbol */
                        if (handledNonTer.find(vec[0]) == handledNonTer.end()) {
                            worklist.push(vec[0]);
                            handledNonTer.insert(vec[0]);
                            /*
                             * A -> B C D
                             * if B may be EPSILON, then first-set of C should be included into first-set of A
                             * the algorithm works for C as well.
                             * */
                            unsigned vSize = vec.size();
                            for (unsigned idx = 0; idx < vSize - 1; idx++) {
                                if (vec[idx] < SYNTAX_EPSILON_IDX && isNonTerminalEPSILON(vec[idx])) {
                                    worklist.push(vec[idx+1]);
                                    handledNonTer.insert(vec[idx+1]);
                                } else {
                                    break;
                                }
                            }
                        }
                    } else if (vec[0] == SYNTAX_EPSILON_IDX) {
                        /* s.insert(SYNTAX_EPSILON_IDX);
                         * handle out of this while statement. */
                    } else if (vec[0] > SYNTAX_EPSILON_IDX){
                        /* token */
                        s.insert(vec[0]);
                    }
                }
            }

            if (isNonTerminalEPSILON(id)) s.insert(SYNTAX_EPSILON_IDX);

            return s;
        }

        std::unordered_set<unsigned> genFollowSet(unsigned id)
        {
            /*
             * A -> B C D C B, id is C
             * */
            for (const auto& item: productionIDs) {
                for (const auto& pa: item.second) {
                    int idx = 0;
                    for (unsigned symbolId: pa.first) {
                        DO IT
                    }
                }
            }
        }

        auto genNFA()
        {
            /* start from Prog' */


        }

        auto parse()
        {
            if (init() != true) return false;


        }

    private:
        /* if nonTerminal can product EPSILON */
        bool isNonTerminalEPSILON(unsigned nonTerminal)
        {
            std::unordered_set<unsigned> nonTerminalHandling;
            return isNonTerminalEPSILON(nonTerminal, nonTerminalHandling)
        }

        bool isNonTerminalEPSILON(unsigned nonTerminal,
                                  std::unordered_set<unsigned>& nonTerminalHandling)
        {
            /*
             * supposing we have productions:
             * A1 -> B1 C1 D1 (1)
             *       B2 C2 D2 (2)
             * B1 -> B3 C3 D3 (3)
             *       B4 C4 D4 (4)
             * B3 -> A1 O1    (5)
             *    -> O2 O3    (6)
             * in production (6), we rehandle A1, and A1 is in "nonTerminalHandling" now, we just return false,
             * and we say that A1 can not reach an EPSILON in production (5), why we can do that?
             * think about it:
             * when we handle A1 in production (5), whose terminal symbol is B3, whose terminal
             * symbol is B1, and B1 is in the first production of A1 (1), we can temporarily skip
             * the recursion case by directly return false[1] tempoparily, and we handle the other case which
             * can handled without recursing. if the other case return true, it means A1 do have
             * EPSILON production, or all the non-recursive cases can not infer to EPSILON. and we
             * review the recursive cases we have gatherred before by skipping, all we can do is
             * to keep recursing the recursive cases in A1, or replaced A1 by non-EPSILON
             * production, and we would get a "false" anyway. so the return value in position[1]
             * is reasonable.
             * */

            /* return false to skip */
            if (nonTerminalHandling.find(nonTerminal) != nonTerminalHandling.end()) return false;
            nonTerminalHandling.insert(nonTerminal);

            assert(productionIDs.find(nonTerminal) != productionIDs.end());
            for (const auto& pa: productionIDs[nonTerminal]) {
                const std::vector<unsigned>& v = pa.first;
                if (isVectorEPSILON(v, nonTerminalHandling)) {
                    /* TODO: if nonTerminal would be rehandled later, we can optimize it by caching the handled nonTerminals */
                    nonTerminalHandling.remove(nonTerminal);
                    return true;
                }
            }

            nonTerminalHandling.remove(nonTerminal);
            return false;
        }


        bool isVectorEPSILON(const std::vector<unsigned>& v,
                             std::unordered_set<unsigned>& nonTerminalHandling)
        {
            for (auto id: v) {
                if (id == SYNTAX_EPSILON_IDX) {
                    continue;
                } else if (id > SYNTAX_EPSILON_IDX) {
                    return false;
                } else {
                    if (isNonTerminalEPSILON(id, nonTerminalHandling)) { continue; }
                    else { return false; }
                }
            }

            return true;
        }

        bool init()
        {
            NFANode::maxId = 0;
            switchString2ID();

            return true;
        }

        unsigned getSymbolID(const std::string& s)
        {
            if (non_terminal_to_id.find(s) != non_terminal_to_id.end()) {
                return non_terminal_to_id[s];
            } else if (lex::string2TokenId.find(s) != lex::string2TokenId.end()) {
                return lex::string2TokenId[s];
            } else if (s == SYNTAX_EPSILON_STR) {
                return SYNTAX_EPSILON_IDX;
            }

            /* ERROR: s not found, please check your syntax file */
            assert(false);
            return 0;
        }

        /* pair<id, SyntaxSymbolKind> */
        std::pair<unsigned, SyntaxSymbolKind> getSymbolIDPair(const std::string& s)
        {
            if (non_terminal_to_id.find(s) != non_terminal_to_id.end()) {
                return std::make_pair(non_terminal_to_id[s], SYN_SYM_NON_TERMINAL);
            } else if (lex::string2TokenId.find(s) != lex::string2TokenId.end()) {
                return std::make_pair(lex::string2TokenId[s], SYN_SYM_TOKEN);
            } else if (s == SYNTAX_EPSILON_STR) {
                return std::make_pair(SYNTAX_EPSILON_IDX, SYN_SYM_EPSILON)
            }

            /* ERROR: s not found, please check your syntax file */
            assert(false);
            return std::make_pair(0, SYN_SYM_UNKNOWN);
        }

        void switchString2ID()
        {
            /* pre-check */
            for (const auto& item: non_terminal_to_id) {
                if (lex::string2TokenId.find(item.first) != lex::string2TokenId.end()) {
                    /* non-terminal's name is conflict with that of TOKEN NAME, gen nfa failed */
                    assert(false);
                }
            }

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
        }

        /* keyID -> production vectors, pair: production -> current LR(1) idx */
        std::unordered_map<unsigned, std::vector<std::pair<std::vector<unsigned>, unsigned>>> productionIDs;
};



};  /* namespace syntax*/
};  /* namespace jhin */

int main(){

    return 0;
}


