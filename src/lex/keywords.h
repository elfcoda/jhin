#ifndef __KEYWORDS_H__
#define __KEYWORDS_H__

#include <vector>
#include <string>
#include <climits>
#include <unordered_map>
#include <unordered_set>

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

/* #define UNACCEPTABLE_START  '=' */


/* parse status */
#define LEX_STATUS_NORMAL   1    /* normal parse */
#define LEX_STATUS_STRING   2    /* parse string starting with '\"', see also CHAR_NOT_DOUQUO */
// #define LEX_STATUS_INITIAL  4    /* parse not in charset error, ses also NOT_IN_CHARSET */

/* can not find a token string by token id */
#define TOKEN_NOT_FOUND     "NOT_A_TOKEN"

/* initial id is 0 */
enum EInitialId
{
    INITIALID = 0,
};


/* the order in VKeyWords must be the same with that in EKeyWords */
/* any change to keyWords should be synchronized to the file lexical.md */
/* 65537 ~ 65537+512 */
/* any change must synchronized to v1 */
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
    UNIT,
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
    IMPORT,
    FOR,
    DEF,

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
    NEWLINE,

    /* unacceptable symbol */
    CLASSIC_ASSIGN,

    /* final mark, not a keyword */
    FINAL_MARK,
};

/* regular expression */
/* 65537+512+1 ~ ? */
/* any change must synchronized to v2 */
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
/* not-in-charset error */
/* any change must synchronized to v3 */
enum EError
{
    /* not in charset */
    ERR_ERROR = TERMINATOR_RE + 128,
};

using TOKEN = unsigned int;

/* *
 * TOKEN kind:
 * EKeyWords
 * ERESymbol
 * EError
 */
// /* unmatched */
// enum EUINTMAX
// {
//     E_UINT_MAX = UINT_MAX,
// };
//
// /* TOKEN */
// union TOKEN
// {
//     EKeyWords   token_keywords;
//     ERESymbol   token_re;
//     EError      token_error;
//     EUINTMAX    token_max;
// };

/* any change must synchronized to EKeyWords */
const std::vector<std::string> VKeyWords = {
    "class", "inherits", "this", "Object", "Bool", "Int", "Float", "Double", "Long", "String", "Unit", "Type", "main",
    "lambda", "let", "in", "while", "do", "if", "elif", "else", "case", "of", "otherwise", "new",
    "True", "False", "isvoid", "data", "return", "callcc", "break", "try", "catch", "except", "import", "for", "def",
    /* symbol */
    "_", ":", ";", ",", "<-", "==", ">", ">=", "<", "<=", "->", "=>",
    "+", "-", "*", "/", "\\", "(", ")", "{", "}", "[", "]", "|",
    ".", "%", "@", "--", "\"", "!", "\n",
    /* unacceptable symbol.
     * when dealing with "==", first '=' would go to non-terminal node
     * for the purpose that dfaInit should accept any non-blank char to terminal status
     * however '=' is basically not defined in this language
     * */
    "="
};


/* FOR DEBUGGING AND SYNTAX PARSING TOOLS */
/* get Token String By Id */
std::unordered_map<unsigned, std::string> tokenId2String = {};
std::unordered_map<std::string, unsigned> string2TokenId = {};
std::unordered_set<unsigned> tokenSet = {};

/* start from TERMINATOR+1 */
static const std::vector<std::string> v1 = {
    /* keywords */
    "CLASS", "INHERITS", "THIS", "OBJECT", "BOOL", "INT", "FLOAT", "DOUBLE", "LONG", "STRING", "UNIT", "TYPE", "MAIN", "LAMBDA", "LET", "IN", "WHILE", "DO", "IF", "ELIF", "ELSE",
    "CASE", "OF", "OTHERWISE", "NEW", "TRUE", "FALSE", "ISVOID", "DATA", "RETURN", "CALLCC", "BREAK", "TRY", "CATCH", "EXCEPT", "IMPORT", "FOR", "DEF",
    /* symbol */
    "UNDERS", "COLON", "SEMICO", "COMMA", "ASSIGN", "EQ", "GT", "GE", "LT", "LE", "ARROW", "INFER", "PLUS", "MINUS", "STAR", "SLASH", "BACKSLA", "LPAREN", "RPAREN", "LCURLY", "RCURLY", "LBRACK",
    "RBRACK", "VBAR", "DOT", "PERCENT", "AT", "COMMENT", "DOUQUO", "NOT", "NEWLINE",
    /* unacceptable symbol */
    "CLASSIC_ASSIGN"
};

/* start from TERMINATOR_RE+2 */
static const std::vector<std::string> v2 = {
    "RE_INT", "RE_DECIMAL", "RE_ID", "RE_VALUE", "RE_STRING"
};

/* start from TERMINATOR_RE+128 */
static const std::vector<std::string> v3 = {
    "ERR_ERROR"
};

void setTokenId2String()
{
    unsigned idx = TERMINATOR + 1;
    for (std::string s: v1) {
        string2TokenId[s] = idx;
        tokenSet.insert(idx);
        tokenId2String[idx++] = s;
    }
    idx = TERMINATOR_RE + 2;
    for (std::string s: v2) {
        string2TokenId[s] = idx;
        tokenSet.insert(idx);
        tokenId2String[idx++] = s;
    }
    idx = TERMINATOR_RE + 128;
    for (std::string s: v3) {
        string2TokenId[s] = idx;
        tokenSet.insert(idx);
        tokenId2String[idx++] = s;
    }
}

std::string getStringByTokenId(unsigned id)
{
    if (tokenId2String.find(id) == tokenId2String.end()) return TOKEN_NOT_FOUND;
    return tokenId2String[id];
}



};  /* namespace lex */
};  /* namespace jhin */


#endif

