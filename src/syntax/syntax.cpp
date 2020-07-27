#include <unordered_set>
#include "non_terminal.h"
#include "syntax_nfa.h"
#include "../lex/nfa.h"
#include "../lex/keywords.h"
#include "../../comm/dfa.h"

namespace jhin
{
namespace syntax
{

class Syntax
{
    public:
        std::unordered_set<unsigned> genFirstSet(unsigned id)
        {
            if (isNonTerminal(id)) {
                return genFirstSetNonTerminal(id);
            } else if (isToken(id)) {
                return genFirstSetTerminal(id);
            }

            /* id can not be EPSILON and others */
            assert(false);
            return std::unordered_set<unsigned>{};
        }

        std::unordered_set<unsigned> genFirstSetTerminal(unsigned id)
        {
            return std::unordered_set<unsigned>{id};
        }

        /* can include EPSILON */
        std::unordered_set<unsigned> genFirstSetNonTerminal(unsigned id)
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
            std::unordered_set<unsigned> handledNonTer{id};

            while (!worklist.empty()) {
                unsigned i = worklist.front(); worklist.pop();
                assert((productionIDs.find(i) != productionIDs.end()));
                for (const std::vector<unsigned>& vec: productionIDs[i]) {
                    assert(!vec.empty());
                    if (isNonTerminal(vec[0])) {
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
                                if (isNonTerminal(vec[idx]) && isNonTerminalEPSILON(vec[idx])) {
                                    worklist.push(vec[idx+1]);
                                    handledNonTer.insert(vec[idx+1]);
                                } else {
                                    break;
                                }
                            }
                        }
                    } else if (isEPSILON(vec[0])) {
                        /* s.insert((vec[0]);
                         * handle out of this while statement. */
                    } else if (isToken(vec[0])) {
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
             * EPSILON is always excluded in follow-set
             * */
            std::unordered_set<unsigned> sFollow = {};
            std::unordered_set<unsigned> sHandled = {};
            std::queue<unsigned> worklist;
            worklist.push(id);
            sHandled.insert(id);

            while (!worklist.empty()) {
                unsigned workId = worklist.front(); worklist.pop();
                for (const auto& item: productionIDs) {
                    for (const std::vector<unsigned>& v: item.second) {
                        for (int idx = 0; idx < v.size(); idx++) {
                            unsigned symbolId = v[idx];
                            if (symbolId == workId) {
                                int idx2 = idx + 1;
                                for (; idx2 < v.size(); idx2++) {
                                    if (isEPSILON(v[idx2])) continue;
                                    setUnion(sFollow, genFirstSet(v[idx2]));
                                    if (isToken(v[idx2]) || !isNonTerminalEPSILON(v[idx2])) break;
                                }
                                if (idx2 == v.size() && sHandled.find(item.first) == sHandled.end()) {
                                    worklist.push(item.first);
                                    sHandled.insert(item.first);
                                }
                            }
                        }
                    }
                }
            }

            return sFollow;
        }

        pSyntaxNFAData genAllNFANodes()
        {
            pSyntaxNFAData pStart = nullptr;
            for (const auto& item: productionIDs) {
                for (const auto& v: item.second) {
                    for (unsigned position = 0; position <= v.size(); position++) {
                        pSyntaxNFAData p = new SyntaxNFAData(item.first, v, position);
                        SyntaxNFAData::mHash[p->hash].push_back(p);
                        /* start node must has only a production: Prog' -> Prog */
                        if (item.first == 1 && position == 0) pStart = p;
                    }
                }
            }

            return pStart;
        }

        pSyntaxNFAData genNFA()
        {
            /* start from Prog' */
            pSyntaxNFAData pStart = genAllNFANodes();
            std::queue<pSyntaxNFAData> worklist;
            std::unordered_set<pSyntaxNFAData> handled;
            worklist.push(pStart);
            handled.insert(pStart);

            while (!worklist.empty()) {
                /* A -> EPSILON */
                pSyntaxNFAData p = worklist.front(); worklist.pop();
                unsigned size = p->production.size();
                if (p->position < size) {
                    /* find node* */
                    pSyntaxNFAData pNFA = getSyntaxNFANode(p->nonTerminal, p->production, p->position + 1);
                    p->mNodes[p->production[p->position]].push_back(pNFA);
                    if (handled.find(pNFA) == handled.end()) {
                        worklist.push(pNFA);
                        handled.insert(pNFA);
                    }
                    if (isNonTerminal(p->production[p->position])) {
                        assert(productionIDs.find(p->production[p->position]) != productionIDs.end());
                        for (const std::vector<unsigned>& v: productionIDs[p->production[p->position]]) {
                            pSyntaxNFAData pNFA2 = getSyntaxNFANode(p->production[p->position], v, 0);
                            p->mNodes[SYNTAX_EPSILON_IDX].push_back(pNFA2);
                            if (handled.find(pNFA2) == handled.end()) {
                                worklist.push(pNFA2);
                                handled.insert(pNFA2);
                            }
                        }
                    }
                }
            }

            return pStart;
        }

        comm::pDFANode<pSyntaxNFAData> NFA2DFA(pSyntaxNFAData init)
        {
            std::queue<pSyntaxNFAData> qu;
            qu.push(init);
            std::set<pSyntaxNFAData> sNFA = comm::genEPClosure(qu, SYNTAX_EPSILON_IDX);
            unsigned startHash = jhin::comm::genHash(sNFA);
            /* create first DFA node */
            comm::pDFANode<pSyntaxNFAData> pStart = new comm::DFANode<pSyntaxNFAData>(startHash, sNFA);
            comm::propagateDFA<pSyntaxNFAData>(pStart, SYNTAX_EPSILON_IDX);

            return pStart;
        }

        pSyntaxNFAData getSyntaxNFANode(unsigned nonTerminal, const std::vector<unsigned>& production, unsigned position)
        {
            pSyntaxNFAData p = new SyntaxNFAData(nonTerminal, production, position);
            assert(SyntaxNFAData::mHash.find(p->hash) != SyntaxNFAData::mHash.end());
            const std::vector<pSyntaxNFAData>& vec = SyntaxNFAData::mHash[p->hash];
            for (pSyntaxNFAData pNFA: vec) {
                if (pNFA->isDataSame(p)) {
                    delete p;
                    return pNFA;
                }
            }

            assert(false);
            delete p;
            return nullptr;
        }

        bool parse()
        {
            if (init() != true) return false;

            return true;
        }

    private:
        /* union second set to the first set */
        void setUnion(std::unordered_set<unsigned>& fst, const std::unordered_set<unsigned>&& snd)
        {
            for (unsigned e: snd) { fst.insert(e); }
        }

        bool isNonTerminal(unsigned id) { return id < SYNTAX_EPSILON_IDX; }
        bool isEPSILON(unsigned id) { return id == SYNTAX_EPSILON_IDX; }
        bool isToken(unsigned id) { return lex::tokenSet.find(id) != lex::tokenSet.end(); }

        /* if nonTerminal can product EPSILON */
        bool isNonTerminalEPSILON(unsigned nonTerminal)
        {
            std::unordered_set<unsigned> nonTerminalHandling;
            return isNonTerminalEPSILON(nonTerminal, nonTerminalHandling);
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
            for (const auto& v: productionIDs[nonTerminal]) {
                if (isVectorEPSILON(v, nonTerminalHandling)) {
                    /* TODO: if nonTerminal would be rehandled later, we can optimize it by caching the handled nonTerminals */
                    nonTerminalHandling.erase(nonTerminal);
                    return true;
                }
            }

            nonTerminalHandling.erase(nonTerminal);
            return false;
        }


        bool isVectorEPSILON(const std::vector<unsigned>& v,
                             std::unordered_set<unsigned>& nonTerminalHandling)
        {
            for (auto id: v) {
                if (isEPSILON(id)) {
                    continue;
                } else if (isToken(id)) {
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
                return std::make_pair(SYNTAX_EPSILON_IDX, SYN_SYM_EPSILON);
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
                std::vector<std::vector<unsigned>> vv;
                for (const auto& item2: item.second) {
                    std::vector<unsigned> v;
                    for (const auto& strElement: item2) {
                        v.push_back(getSymbolID(strElement));
                    }
                    vv.emplace_back(v);
                }
                productionIDs[keyID] = vv;
            }
        }

        /* keyID -> production vectors, production */
        std::unordered_map<unsigned, std::vector<std::vector<unsigned>>> productionIDs;
};



};  /* namespace syntax*/
};  /* namespace jhin */

int main(){

    return 0;
}

