#ifndef __KEYWORDS_H__
#define __KEYWORDS_H__

#include <vector>
#include <string>

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
/* any change to keyWords should be synchronized to the file lexical.md */
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

};  /* namespace lex */
};  /* namespace jhin */


#endif

