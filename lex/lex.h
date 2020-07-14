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
            row = 0;
            col = 0;
        }

        bool LexInit()
        {
            setTokenId2String();
            m_charSet = initCharSet();

            return true;
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

        std::pair<bool, std::string> parse(const std::string& source)
        {
            /* init first */
            if (LexInit() == false) return std::make_pair(false, "ERROR: Lex init error");

            pDFANode dfaInit = NFA2DFA(genNFA());
            pDFANode pCur = dfaInit;
            status = LEX_STATUS_NORMAL;
            bool backslash = false;

            TOKEN lastToken = UINT_MAX;;
            std::string parseStr = "";
            std::vector<std::pair<std::string, std::string>> parseResult;

            for (char c: source) {
                char origin_c = c;
                if (c == EOF) continue;

                if (c != '\n') { col ++; }
                else { row++; col = 0; }

                /* handle backslash */
                if (c == '\\') {
                    if (status != LEX_STATUS_STRING) return std::make_pair(false, "ERROR: backslash");
                    backslash = true;
                    continue;
                }
                if (backslash == true) {
                    c = CHAR_NOT_DOUQUO;
                    backslash = false;
                }

                /* backslash is false, handle char c */
                c = switchChar(c);

                /* feed char c to next dfa node */
handle_non_blank:
                if (pCur->mEdges.find(c) == pCur->mEdges.end()) {
                    /* unknown error, return false */
                    if (pCur == dfaInit && !isBlank(c)) return std::make_pair(false, "ERROR: unknown error, initial dfa node can not accept non-blank char");

                    /* match token, continue */
                    if (lastToken != UINT_MAX) {
                        /* normal token */
                        parseResult.push_back(std::make_pair(getStringByTokenId(lastToken), parseStr));
                        pCur = dfaInit;
                        lastToken = UINT_MAX;
                        parseStr = "";
                    }
                    /* match blank, skip */
                    if (!isBlank(c)) {
                        /* and now pCur is definitely dfaInit, we goto handle_non_blank to redo char c again. pCur should accept it, or compiler will raise an error. */
                        goto handle_non_blank;
                    }
                } else {
                    pCur = pCur->mEdges[c];
                    parseStr.push_back(origin_c);
                    lastToken = pCur->terminalId;
                    if (lastToken == UINT_MAX) return std::make_pair(false, "ERROR: unknown error");
                    else if (lastToken == static_cast<unsigned int>(ERR_ERROR)) {
                        /* match error node, aka. not in charset, return false */
                        return std::make_pair(false, "ERROR: not in charset error");
                    }
                    /* normal terminalId*/
                }

                /* switch status */
                if (status == LEX_STATUS_NORMAL && c == '\"') status = LEX_STATUS_STRING;
                else if (status == LEX_STATUS_STRING && c == '\"') status = LEX_STATUS_NORMAL;
            }

            /* end with backslash */
            if (backslash == true) {
                return std::make_pair(false, "ERROR: file end with backslash; incomplete String");
            }

            return std::make_pair(true, "");
        }

        void drivenByDFATable(pDFANode init)
        {
            /*
             * TODO
             * NOT NECESSARY
             * */
        }

    private:
        char status;
        /* char's position in src code file */
        int row;
        int col;
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

            /* add blank char */
            s.insert(' ');
            s.insert('\n');
            s.insert('\t');

            return s;
        }

        bool is_NOT_IN_CHARSET(char c)
        {
            return !isInCharSet(c);
        }

        bool isBlank(char c)
        {
            if (c == ' ' || c == '\t' || c == '\n') return true;
            return false;
        }

        char switchChar(char c)
        {
            if (status == LEX_STATUS_NORMAL) {
                if (is_NOT_IN_CHARSET(c)) return NOT_IN_CHARSET;
                else return c;
            } else if (status == LEX_STATUS_STRING) {
                if (c == '\"') {
                    return c;
                } else {
                    return CHAR_NOT_DOUQUO;
                }
            }

            /* unreachable*/
            return 0;
        }

};

};  /* namespace lex */
};  /* namespace jhin */
#endif

