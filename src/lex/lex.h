#ifndef __LEX_H__
#define __LEX_H__

#include <vector>
#include <string>
#include <map>
#include <set>
#include <cassert>
#include <climits>
#include <unordered_set>
#include <queue>
#include <ctime>
#include "keywords.h"
#include "nfa.h"
#include "../../comm/dfa.h"


namespace jhin
{
namespace lex
{

class Lex
{
    public:
        Lex()
        {
            row = 1;
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

        comm::pDFANode<pNFANode> NFA2DFA(pNFANode init)
        {
            std::queue<pNFANode> qu;
            qu.push(init);
            std::set<pNFANode> sNFA = comm::genEPClosure(qu, EPSILON);
            unsigned int startHash = jhin::comm::genHash(sNFA);
            /* create first DFA node */
            comm::pDFANode<pNFANode> pStart = new comm::DFANode<pNFANode>(startHash, sNFA);
            comm::propagateDFA<pNFANode>(pStart, EPSILON);

            return pStart;
        }

        std::pair<bool, std::string> parse(const std::string& source, const std::string& filename, std::vector<std::pair<unsigned, std::string>>& parseResult)
        {
            return parseSrc(source, filename, parseResult);
        }

        std::pair<bool, std::string> parseSrc(const std::string& source, const std::string& filename, std::vector<std::pair<unsigned, std::string>>& parseResult)
        {
            /* init first */
            if (LexInit() == false) return std::make_pair(false, getErrorMsg("Lex init error", filename));

            /* about 2 seconds to generate NFA and DFA nodes */
            comm::pDFANode<pNFANode> dfaInit = NFA2DFA(genNFA());
            comm::pDFANode<pNFANode> pCur = dfaInit;
            status = LEX_STATUS_NORMAL;
            bool backslash = false;
            bool commentFlag = false;

            TOKEN lastToken = UINT_MAX;;
            std::string parseStr = "";
            parseResult.clear();

            for (char c: source) {
                char origin_c = c;

                if (c == EOF) continue;

                if (c != '\n') { col++; }
                else { row++; col = 0; }

                /* handle COMMENT case */
                if (commentFlag == true) {
                    if (origin_c == '\n') {
                        commentFlag = false;
                    } else {
                        continue;
                    }
                }

                /* handle backslash */
                if (c == '\\') {
                    if (status != LEX_STATUS_STRING) return std::make_pair(false, getErrorMsg("backslash error", filename));
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
                    if (pCur == dfaInit && !isBlank(c)) return std::make_pair(false, getErrorMsg("unknown error, initial dfa node can not accept non-blank char.", filename));

                    /* match token, continue */
                    if (lastToken != UINT_MAX) {
                        /* normal token found */
                        if (lastToken == static_cast<unsigned int>(COMMENT)) {
                            /* handle COMMENT case
                             * to the end of the line
                             * */
                            if (origin_c != '\n') commentFlag = true;
                        } else {
                            /* handle other cases */
                            if (lastToken >= static_cast<unsigned int>(CLASSIC_ASSIGN) && lastToken < static_cast<unsigned int>(FINAL_MARK)) {
                                /* there is no definition for unacceptable symbols, in this case, '=', in this language */
                                return std::make_pair(false, getErrorMsg("pattern \"" + VKeyWords[lastToken - TERMINATOR - 1] + "\" is not defined in this language.", filename));
                            }

                            parseResult.push_back(std::make_pair(lastToken, parseStr));
                        }
                        pCur = dfaInit;
                        lastToken = UINT_MAX;
                        parseStr = "";
                    }

                    if (pCur != dfaInit) return std::make_pair(false, getErrorMsg("pCur should be dfaInit when lastToken is UINT_MAX.", filename));

                    /* match blank, skip */
                    if (!isBlank(c) && !commentFlag) {
                        /* and now pCur is definitely dfaInit, we goto handle_non_blank to redo non-blank char c again. pCur should accept it, or compiler will raise an error. */
                        goto handle_non_blank;
                    }
                } else {
                    pCur = pCur->mEdges[c];
                    parseStr.push_back(origin_c);
                    lastToken = pCur->terminalId;
                    if (lastToken == UINT_MAX) {
                        return std::make_pair(false, getErrorMsg("unknown error, char: " + std::to_string(origin_c), filename));
                    } else if (lastToken == static_cast<unsigned int>(ERR_ERROR)) {
                        /* match error node, aka. not in charset, return false */
                        return std::make_pair(false, getErrorMsg("not in charset error", filename));
                    }
                    /* normal terminalId */
                }

                /* switch status */
                if (status == LEX_STATUS_NORMAL && c == '\"') status = LEX_STATUS_STRING;
                else if (status == LEX_STATUS_STRING && c == '\"') status = LEX_STATUS_NORMAL;
            }

            /* end with backslash */
            if (backslash == true) {
                return std::make_pair(false, getErrorMsg("file end with backslash; incomplete String", filename));
            }

            /* deal with last token */
            if (lastToken != UINT_MAX) {
                /* normal token */
                parseResult.push_back(std::make_pair(lastToken, parseStr));
                pCur = dfaInit;
                lastToken = UINT_MAX;
                parseStr = "";
            }

            return std::make_pair(true, "");
        }

        void drivenByDFATable(comm::pDFANode<pNFANode> init)
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

        std::string getErrorMsg(std::string msg, const std::string& filename)
        {
            return "ERROR near [" + std::to_string(row) + ", " + std::to_string(col) + "] in " + filename + ": " + msg;
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
            if (c == ' ' || c == '\t') return true;
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

