#ifndef __LEX_H__
#define __LEX_H__

#include <vector>
#include <string>
#include <map>
#include <set>
#include <cassert>
#include <unordered_set>
#include <queue>

namespace jhin
{
namespace lex
{
/* NodeId of non-terminator must be less than TERMINATOR */
#define TERMINATOR      65537
/* number of KeyWords must be less than MAX_KEY_WORDS */
#define MAX_KEY_WORDS   512
/* Regular rule symbol */
#define TERMINATOR_RE   (TERMINATOR + MAX_KEY_WORDS)

/* define EPSILON as '#' */
#define EPSILON         35
/* define NOT_IN_CHARSET, deal with illegal chars appears outside string */
#define NOT_IN_CHARSET  20
/* define CHAR_NOT_DOUQUO for any char used in strings but '\"', and we should handle baskslash case */
#define CHAR_NOT_DOUQUO 21

/* initial id is 0 */
enum EInitialId
{
    INITIALID = 0,
};

/* the order in VKeyWords must be the same with that in EKeyWords */
/* any change to keyWords should be synchronized to the file lexical.lex */
/* 65537 ~ 65537+512 */
enum EKeyWords
{
    /* start mark, not a keyword */
    START_MARK = TERMINATOR,

    /* keywords */
    CLASS,
    INHERITS,
    THIS,
    OBJECT,
    BOOL,
    INT,
    FLOAT,
    DOUBLE,
    LONG,
    STRING,
    VOID,
    TYPE,
    MAIN,
    LAMBDA,
    LET,
    IN,
    WHILE,
    DO,
    IF,
    ELIF,
    ELSE,
    CASE,
    OF,
    OTHERWISE,
    NEW,
    TRUE,
    FALSE,
    ISVOID,
    DATA,
    RETURN,
    CALLCC,
    BREAK,
    TRY,
    CATCH,
    EXCEPT,

    /* symbol */
    UNDERS,
    COLON,
    SEMICO,
    COMMA,
    ASSIGN,
    EQ,
    GT,
    GE,
    LT,
    LE,
    ARROW,
    INFER,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    BACKSLA,
    LPAREN,
    RPAREN,
    LCURLY,
    RCURLY,
    LBRACK,
    RBRACK,
    VBAR,
    DOT,
    PERCENT,
    AT,
    COMMENT,
    DOUQUO,
    NOT,

    /* final mark, not a keyword */
    FINAL_MARK,
};

/* regular expression */
/* 65537+512+1 ~ ? */
enum ERESymbol
{
    RE_START_MARK = TERMINATOR_RE + 1,

    RE_INT,
    RE_DECIMAL,
    RE_ID,
    RE_VALUE,
    RE_STRING,

    RE_FINAL_MARK,
};

/* TERMINATOR_RE+128 ~ ?, must be greater than RE_FINAL_MARK */
enum EError
{
    ERR_ERROR = TERMINATOR_RE + 128,
};

const std::vector<std::string> VKeyWords =
{
    "class", "inherits", "this", "Object", "Bool", "Int", "Float", "Double", "Long", "String", "Void", "Type", "main",
    "lambda", "let", "in", "while", "do", "if", "elif", "else", "case", "of", "otherwise", "new",
    "True", "False", "isvoid", "data", "return", "callcc", "break", "try", "catch", "except",
    /* symbol */
    "_", ":", ";", ",", "<-", "==", ">", ">=", "<", "<=", "->", "=>",
    "+", "-", "*", "/", "\\", "(", ")", "{", "}", "[", "]", "|",
    ".", "%", "@", "--", "\"", "!"
};

/* NFA definition */
struct NFANode
{
    /* current max normal node id */
    /* maxId maintain the old value even if normal node id is updated */
    static unsigned int maxId;

    /* 0~65536: normal node, 65538~MAXINT: terminal node */
    unsigned int id;

    /*
     * edges, and Epsilon is representd as '#'
     * but '#' is not a chacracter of this language
     * for the reason, this lex analyser will complain when feeded with '#'
     * */
    std::map<char, std::vector<NFANode*>> mNodes;

    /* normal node */
    NFANode()
    {
        /* id starts from 1 */
        maxId += 1;
        id = maxId;
        assert(id < TERMINATOR);
    }

    /* initial node */
    NFANode(EInitialId initialId)
    {
        id = static_cast<unsigned int>(initialId);
    }

    /* terminal node */
    NFANode(EKeyWords terminalId)
    {
        id = static_cast<unsigned int>(terminalId);
    }

    /* error node */
    NFANode(EError terminalId)
    {
        id = static_cast<unsigned int>(terminalId);
    }

    /* switch normal node to terminal node */
    void setId(unsigned int terminalId)
    {
        id = terminalId;
    }
};
unsigned int NFANode::maxId = 0;
using pNFANode = NFANode*;
using pNFAPair = std::pair<pNFANode, pNFANode>;


pNFAPair genSingle(char c)
{
    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    pStart->mNodes[c] = std::vector<pNFANode>{pEnd};

    return std::make_pair(pStart, pEnd);
}

pNFAPair genEpsilon()
{
    return genSingle(EPSILON);
}

/* 0-9 a-z A-Z */
pNFAPair genOrByRange(char start, char end)
{
    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    for (char c = start; c <= end; c++) {
        pNFANode p1 = new NFANode();
        pNFANode p2 = new NFANode();
        p1->mNodes[c] = std::vector<pNFANode>{p2};
        p2->mNodes[EPSILON] = std::vector<pNFANode>{pEnd};
        pStart->mNodes[EPSILON].push_back(p1);
    }

    return std::make_pair(pStart, pEnd);
}

#define GENTYPE_DIGITS  1
#define GENTYPE_LOWERC  2
#define GENTYPE_UPPERC  3
pNFAPair genOrByRange(int gentype)
{
    if (gentype == GENTYPE_DIGITS) return genOrByRange('0', '9');
    if (gentype == GENTYPE_LOWERC) return genOrByRange('a', 'z');
    if (gentype == GENTYPE_UPPERC) return genOrByRange('A', 'Z');

    assert(false);
    return std::make_pair(nullptr, nullptr);
}

pNFAPair genOrByString(const std::string& s)
{
    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    for (char c: s) {
        pNFANode p1 = new NFANode();
        pNFANode p2 = new NFANode();
        p1->mNodes[c] = std::vector<pNFANode>{p2};
        p2->mNodes[EPSILON] = std::vector<pNFANode>{pEnd};
        pStart->mNodes[EPSILON].push_back(p1);
    }

    return std::make_pair(pStart, pEnd);
}

pNFAPair genAndByString(const std::string& s)
{
    assert(s != "");
    pNFANode pStart = new NFANode();
    pNFANode p1 = pStart, p2 = nullptr;
    for (char c: s) {
        p2 = new NFANode();
        p1->mNodes[c] = std::vector<pNFANode>{p2};
        p1 = p2;
    }

    return std::make_pair(pStart, p2);
}

pNFAPair genStar(const pNFAPair& M)
{
    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    pStart->mNodes[EPSILON] = std::vector<pNFANode>{pEnd};
    pEnd->mNodes[EPSILON] = std::vector<pNFANode>{M.first};
    M.second->mNodes[EPSILON] = std::vector<pNFANode>{pEnd};

    return std::make_pair(pStart, pEnd);
}

pNFAPair connectAndNodes(const pNFAPair& M1, const pNFAPair& M2)
{
    assert(M1 != M2);
    M1.second->mNodes[EPSILON].push_back(M2.first);

    return std::make_pair(M1.first, M2.second);
}

pNFAPair connectAndNodesV(const std::vector<pNFAPair>&& M)
{
    int n = M.size();
    assert(n >= 2);

    for (int i = 1; i < n; i++) {
        M[i-1].second->mNodes[EPSILON].push_back(M[i].first);
    }

    return std::make_pair(M[0].first, M[n-1].second);
}

pNFAPair connectOrNodes(const pNFAPair& M1, const pNFAPair& M2)
{
    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    pStart->mNodes[EPSILON] = std::vector<pNFANode>{M1.first, M2.first};
    M1.second->mNodes[EPSILON].push_back(pEnd);
    M2.second->mNodes[EPSILON].push_back(pEnd);

    return std::make_pair(pStart, pEnd);
}

/* [a-z]? OR c?, aka. M | EPSILON */
pNFAPair genExist(const pNFAPair& M)
{
    pNFAPair ME = genEpsilon();
    return connectOrNodes(M, ME);
}

pNFAPair connectOrNodesV(const std::vector<pNFAPair>&& M)
{
    assert(M.size() >= 2);

    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    for (const pNFAPair& m: M) {
        pStart->mNodes[EPSILON].push_back(m.first);
        m.second->mNodes[EPSILON].push_back(pEnd);
    }

    return std::make_pair(pStart, pEnd);
}

/*
 * INT     [0-9]+
 * DECIMAL [0-9]+.[0-9]*(e(+|-)?[0-9]+)?   -- may be float or double
 * ID      [a-z](a-zA-Z0-9|_)*
 * VALUE   [A-Z](a-zA-Z|_)*  -- value constructor
 * */
pNFAPair genReINT()
{
    pNFAPair M1 = genOrByRange(GENTYPE_DIGITS);
    pNFAPair M2 = genOrByRange(GENTYPE_DIGITS);
    pNFAPair M2Star = genStar(M2);

    return connectAndNodes(M1, M2Star);
}

pNFAPair genReDECIMAL()
{
    pNFAPair M1 = genReINT();
    pNFAPair M2 = genSingle('.');
    pNFAPair M3 = genStar(genOrByRange(GENTYPE_DIGITS));

    pNFAPair t0 = genSingle('e');
    pNFAPair t1 = genOrByString("+-");
    pNFAPair t2 = genExist(t1);
    pNFAPair t3 = genReINT();
    pNFAPair t4 = connectAndNodesV({t0, t2, t3});
    pNFAPair M4 = genExist(t4);

    pNFAPair M = connectAndNodesV({M1, M2, M3, M4});

    return M;
}

pNFAPair genReID()
{
    pNFAPair M1 = genOrByRange(GENTYPE_LOWERC);

    pNFAPair t0 = genOrByRange(GENTYPE_LOWERC);
    pNFAPair t1 = genOrByRange(GENTYPE_UPPERC);
    pNFAPair t2 = genOrByRange(GENTYPE_DIGITS);
    pNFAPair t3 = genSingle('_');
    pNFAPair t4 = connectOrNodesV({t0, t1, t2, t3});

    pNFAPair M2 = genStar(t4);

    return connectAndNodes(M1, M2);
}

pNFAPair genReVALUE()
{
    pNFAPair M1 = genOrByRange(GENTYPE_UPPERC);

    pNFAPair t0 = genOrByRange(GENTYPE_LOWERC);
    pNFAPair t1 = genOrByRange(GENTYPE_UPPERC);
    pNFAPair t2 = genSingle('_');
    pNFAPair t3 = connectOrNodesV({t0, t1, t2});

    pNFAPair M2 = genStar(t3);

    return connectAndNodes(M1, M2);
}

pNFAPair genReSTRING()
{
    pNFANode p1 = new NFANode();
    pNFANode p2 = new NFANode();
    pNFANode p3 = new NFANode();
    p1->mNodes['\"'].push_back(p2);
    p2->mNodes['\"'].push_back(p3);
    p2->mNodes[CHAR_NOT_DOUQUO].push_back(p2);

    return std::make_pair(p1, p3);
}

void connectAndSetRe(pNFANode init, pNFAPair M, ERESymbol terminalId)
{
    init->mNodes[EPSILON].push_back(M.first);
    M.second->setId(static_cast<unsigned int>(terminalId));
}


/* DFA definition */
struct DFANode
{
    /* record max node id*/
    static unsigned int maxId;

    /* node id */
    unsigned int id;

    /* node id in NFA(normal and terminal) */
    std::set<unsigned int> vNodeData;

    /* edges */
    std::map<char, DFANode*> mEdges;

    DFANode()
    {
        /* id starts from 1 */
        maxId += 1;
        id = maxId;
        assert(id < UINT_MAX);
    }

};
unsigned int DFANode::maxId = 0;
using pDFANode = DFANode*;


template <class T>
bool isSetEqual(const std::set<T>& s1, const std::set<T>& s2)
{
    auto it1 = s1.begin();
    auto it2 = s2.begin();
    unsigned n1 = s1.size(), n2 = s2.size();
    if (n1 != n2) return false;
    for (; it1 != s1.end() && it2 != s2.end(); it1++, it2++) {
        if (*it1 != *it2) return false;
    }

    return true;
}

std::set<pNFANode> genEPClosure(pNFANode node)
{
    std::queue<pNFANode> qu;
    std::set<pNFANode> se;
    qu.push(node);
    while (!qu.empty()) {
        pNFANode n = qu.front();
        qu.pop();
        const std::vector<pNFANode>& v = n->mNodes[EPSILON];
        for (pNFANode p: v) {
            if (se.find(p) == se.end()) {
                qu.push(p);
            }
        }
        se.insert(n);
    }

    return se;
}

std::set<pNFANode> genClosure(pNFANode node, char c)
{

}

class Lex
{
    public:
        Lex() {
            m_charSet = initCharSet();
        }

        void genNFA()
        {
            /* make sure that EKeyWords and ERESymbol and EError won't overlap each other */
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
        }

        void NFA2DFA(pNFANode init)
        {
            pDFANode pDFA = new DFANode();
            std::queue<pNFANode> qWorkList;
            qWorkList.push(init);
            while (!qWorkList.empty()) {
                pNFANode node = qWorkList.front();
                qWorkList.pop();
                for (const auto& it: node->mNodes) {
                    char c = it.first;
                    for (pNFANode p: it.second) {

                    }
                }
            }

        }

        void drivenByDFATable()
        {

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

