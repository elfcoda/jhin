#ifndef __LEX_H__
#define __LEX_H__

#include <vector>
#include <string>
#include <map>
#include <cassert>

namespace jhin
{
/* NodeId of non-terminator must be less than TERMINATOR */
#define TERMINATOR 65537

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

std::vector<std::string> VKeyWords =
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

/*
 * INT     [0-9]+
 * DECIMAL [0-9]+.[0-9]*(e(+|-)?[0-9]+)?   -- may be float or double
 * ID      [a-z](a-zA-Z0-9|_)*
 * VALUE   [A-Z](a-zA-Z|_)*  -- value constructor
 * */

pNFAPair genSingle(char c)
{
    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    pStart->mNodes[c] = std::vector<pNFANode>{pEnd};

    return std::make_pair(pStart, pEnd);
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
        p2->mNodes['#'] = std::vector<pNFANode>{pEnd};
        pStart->mNodes['#'].push_back(p1);
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
        p2->mNodes['#'] = std::vector<pNFANode>{pEnd};
        pStart->mNodes['#'].push_back(p1);
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
    pStart->mNodes['#'] = std::vector<pNFANode>{pEnd};
    pEnd->mNodes['#'] = std::vector<pNFANode>{M.first};
    M.second->mNodes['#'] = std::vector<pNFANode>{pEnd};

    return std::make_pair(pStart, pEnd);
}

pNFAPair connectAndNodes(const pNFAPair& M1, const pNFAPair& M2)
{
    assert(M1 != M2);
    M1.second->mNodes['#'].push_back(M2.first);

    return std::make_pair(M1.first, M2.second);
}

pNFAPair connectOrNodes(const pNFAPair& M1, const pNFAPair& M2)
{
    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    pStart->mNodes['#'] = std::vector<pNFANode>{M1.first, M2.first};
    M1.second->mNodes['#'].push_back(pEnd);
    M2.second->mNodes['#'].push_back(pEnd);

    return std::make_pair(pStart, pEnd);
}

pNFAPair connectOrNodesV(const std::vector<pNFAPair>& M)
{
    pNFANode pStart = new NFANode();
    pNFANode pEnd = new NFANode();
    for (const pNFAPair& m: M) {
        pStart->mNodes['#'].push_back(m.first);
        m.second->mNodes['#'].push_back(pEnd);
    }

    return std::make_pair(pStart, pEnd);
}

class Lex
{
    public:
        Lex() {}

        void genNFA()
        {

        }

        void NFA2DFA()
        {

        }

        void drivenByDFATable()
        {

        }
};

};  /* namespace jhin */
#endif

