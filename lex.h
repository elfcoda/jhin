#ifndef __LEX_H__
#define __LEX_H__

#include <vector>
#include <string>
#include <map>
#include <cassert>

namespace jhin
{
namespace lex
{
/* NodeId of non-terminator must be less than TERMINATOR */
#define TERMINATOR  65537

/* define EPSILON as '#' */
#define EPSILON     35

/* initial id is 0 */
enum EInitialId
{
    INITIALID = 0,
};

enum EError
{
    ERROR = 0,
};

/* the order in VKeyWords must be the same with that in EKeyWords */
/* any change to keyWords should be synchronized to the file lexical.lex */
enum EKeyWords
{
    CLASS = TERMINATOR + 1,
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

struct NFANode
{
    /* current max normal node id */
    /* maxId maintain the old value even if normal node id is modified */
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
        id = (unsigned int)initialId;
    }

    /* terminal node */
    NFANode(EKeyWords terminalId)
    {
        id = (unsigned int)terminalId;
    }

    /* switch normal node to terminal node */
    void setId(EKeyWords terminalId)
    {
        id = (unsigned int)terminalId;
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

/* [a-z]? OR c?, aka. M | EPSILON */
pNFAPair genExist(const pNFAPair& M)
{
    pNFAPair ME = genEpsilon();
    return connectOrNodes(M, ME);
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

    const pNFAPair& p1 = M[0];
    const pNFAPair& p2 = M[1];
    p1.second->mNodes[EPSILON].push_back(p2.first);
    for (int i = 2; i < n; i++) {
        p1 = p2;
        p2 = M[i];
        p1.second->mNodes[EPSILON].push_back(p2.first);
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

class Lex
{
    public:
        Lex() {}

        void genNFA()
        {
            /* gen initial node by node id 0 */
            pNFANode init = new NFANode(INITIALID);

            /* gen key words and symbols */
            for (const std::string& s: VKeyWords) {
                pNFAPair = genAndByString(s);


            }
            /* gen re */

            /* gen errors */
        }

        void NFA2DFA()
        {

        }

        void drivenByDFATable()
        {

        }
};

};  /* namespace lex */
};  /* namespace jhin */
#endif

