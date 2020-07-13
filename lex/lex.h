#ifndef __LEX_H__
#define __LEX_H__

#include <vector>
#include <string>
#include <map>
#include <set>
#include <cassert>
#include <unordered_set>
#include <queue>
#include "keywords.h"
#include "nfa.h"
#include "dfa.h"


namespace jhin
{
namespace lex
{

class Lex
{
    public:
        Lex()
        {
            m_charSet = initCharSet();
        }

        pNFANode genNFA()
        {
            /* pre-check: make sure that EKeyWords and ERESymbol and EError won't overlap each other */
            assert(EKeyWords::FINAL_MARK - EKeyWords::START_MARK < MAX_KEY_WORDS - 5);
            assert(static_cast<unsigned int>(EError::ERR_ERROR) > static_cast<unsigned int>(ERESymbol::RE_FINAL_MARK));

            /* gen initial node by node id 0 */
            pNFANode init = new NFANode(INITIALID);

            /* gen key words and symbols */
            for (int i = 0; i < VKeyWords.size(); i++) {
                const std::string& s = VKeyWords[i];
                pNFAPair MKeyWords = genAndByString(s);
                init->mNodes[EPSILON].push_back(MKeyWords.first);
                /* EKeyWords */
                MKeyWords.second->setId(TERMINATOR + 1 + i);
            }

            /* gen re */
            connectAndSetRe(init, genReINT(), RE_INT);
            connectAndSetRe(init, genReDECIMAL(), RE_DECIMAL);
            connectAndSetRe(init, genReID(), RE_ID);
            connectAndSetRe(init, genReVALUE(), RE_VALUE);
            connectAndSetRe(init, genReSTRING(), RE_STRING);

            /* gen errors */
            pNFANode pErr = new NFANode(ERR_ERROR);
            init->mNodes[NOT_IN_CHARSET].push_back(pErr);

            return init;
        }

        pDFANode NFA2DFA(pNFANode init)
        {
            std::queue<pNFANode> qu;
            qu.push(init);
            std::set<pNFANode> sNFA = genEPClosure(qu);
            unsigned int startHash = jhin::tools::genHash(sNFA);
            /* create first DFA node */
            pDFANode pStart = new DFANode(startHash, sNFA);
            propagateDFA(pStart);

            return pStart;
        }

        bool parse(const std::string& source)
        {
            pDFANode dfaInit = NFA2DFA(genNFA());

            return true;
        }

        void drivenByDFATable()
        {
            /*
             * TODO
             * NOT NECESSARY
             * */
        }
    private:
        std::unordered_set<char> m_charSet;

        bool isInCharSet(char c)
        {
            if (m_charSet.find(c) == m_charSet.end()) return false;
            return true;
        }

        std::unordered_set<char> initCharSet()
        {
            std::unordered_set<char> s = {'_', ':', ';', ',', '<', '=', '>', '+', '-', '*', '/', '\\', '(', ')', '{', '}', '[', ']', '|', '.', '%', '@', '\"', '!'};
            for (char c = 'a'; c <= 'z'; c++) s.insert(c);
            for (char c = 'A'; c <= 'Z'; c++) s.insert(c);
            for (char c = '0'; c <= '9'; c++) s.insert(c);

            return s;
        }

        bool is_NOT_IN_CHARSET()
        {
            //
            return false;
        }

        bool is_CHAR_NOT_DOUQUO()
        {
            //
            return false;
        }

        void handleBaskslash()
        {
            //
        }

        void handleError()
        {
            //
        }

};

};  /* namespace lex */
};  /* namespace jhin */
#endif

