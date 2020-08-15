#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <unordered_set>
#include <unordered_map>
#include "../../comm/log.h"
#include "../../comm/comm.h"

namespace jhin
{
namespace ast
{

#define AST_NON_LEAF_START  1
#define AST_LEAF_START      2048
#define AST_NOTATION_START  4096
#define AST_DEFAULT_TEXT    ""

/* nonLeaf and leaf nodes are basically all tokens
 * but ast should redefine the nodes ID, which is diff from lex::TokenID and syntax::NonterminalID */
const std::unordered_map<unsigned, std::string> astNonLeaf = {};
const std::unordered_map<unsigned, std::string> astLeaf = {};

enum EASTNonLeaf
{
    AST_NON_LEAF_ASSIGN = AST_NON_LEAF_START,
    AST_NON_LEAF_WHILE,
    AST_NON_LEAF_IF,
    AST_NON_LEAF_IF_ELSE,
    AST_NON_LEAF_CLASS,
    AST_NON_LEAF_CLASS_INHERITS,
    AST_NON_LEAF_FUNCTION,
    AST_NON_LEAF_IS_VOID,
    AST_NON_LEAF_NOT,
    AST_NON_LEAF_NEW,

    AST_NON_LEAF_PLUS,
    AST_NON_LEAF_MINUS,
    AST_NON_LEAF_STAR,
    AST_NON_LEAF_SLASH,

    AST_NON_LEAF_EQ,
    AST_NON_LEAF_LT,
    AST_NON_LEAF_LE,
    AST_NON_LEAF_GT,
    AST_NON_LEAF_GE,

    AST_NON_LEAF_LAMBDA,
    AST_NON_LEAF_FN_CALL,
    AST_NON_LEAF_OBJECT_FN_CALL,
    AST_NON_LEAF_OBJECT_FN_CALL_DISPATCH,
    AST_NON_LEAF_EXP_OBJECT_FN_CALL,
    AST_NON_LEAF_EXP_OBJECT_FN_CALL_DISPATCH,

    AST_NON_LEAF_LET_IN,
    AST_NON_LEAF_RETURN,
    AST_NON_LEAF_CASE_OF_OTHERWISE,

    AST_NON_LEAF_DCL,
};

enum EASTLeaf
{
    AST_LEAF_RE_ID = AST_LEAF_START,
    AST_LEAF_RE_VALUE,
    AST_LEAF_RE_INT,
    AST_LEAF_RE_DECIMAL,
    AST_LEAF_RE_STRING,

    AST_LEAF_TRUE,
    AST_LEAF_FALSE,

    AST_LEAF_OBJECT,
    AST_LEAF_BOOL,
    AST_LEAF_INT,
    AST_LEAF_FLOAT,
    AST_LEAF_DOUBLE,
    AST_LEAF_LONG,
    AST_LEAF_STRING,
    AST_LEAF_UNIT,

    AST_LEAF_THIS,
    AST_LEAF_TYPE,
};

enum EASTNotation
{
    AST_NOTATION_FN_ARGS = AST_NOTATION_START,

};

struct ASTNodeData;
using pASTNodeData = ASTNodeData*;

struct ASTNode;
using pASTNode = ASTNode*;

using pChildrenList = std::vector<pASTNode>*;

struct ASTNodeData
{
    /* non-terminal id or token id */
    unsigned symbolId;
    /* semantics notation, eg, if this node is a function parameter */
    unsigned notation;

    /* raw string */
    std::string text;

    std::string toString()
    {
        if (text != AST_DEFAULT_TEXT) {
            if (text == "\n") return "'/n'";
            else return text;
        }

        return comm::symbolId2String(symbolId) + "[" + comm::symbolId2String(notation) + "]";
    }

    ASTNodeData(unsigned symbolId, const std::string& text)
    {
        this->symbolId = symbolId;
        this->text = text;
        this->notation = symbolId;
    }

    ASTNodeData(ASTNodeData&& ptnode)
    {
        this->symbolId = ptnode.symbolId;
        this->text = ptnode.text;
        this->notation = ptnode.symbolId;
    }
};

struct ASTNode
{
    pASTNodeData data;
    pChildrenList children;

    ASTNode(ASTNodeData&& data, pChildrenList children)
    {
        this->data = new ASTNodeData(std::move(data));
        this->children = children;
    }

    /* free memory */
    bool free()
    {
        return true;
    }

    void setSymbolId(unsigned symbolId)
    {
        data->symbolId = symbolId;
    }

    void setNotation(unsigned notation)
    {
        data->notation = notation;
    }

    unsigned getNotation()
    {
        return data->notation;
    }

    unsigned getSymbolId()
    {
        return data->symbolId;
    }

    /* single child */
    pASTNode getSingle()
    {
        if (children == nullptr) return nullptr;
        if ((*children).size() == 1) return (*children)[0];
        return nullptr;
    }

    bool hasChildren()
    {
        return children != nullptr && !(*children).empty();
    }

    bool erase(unsigned idx)
    {
        unsigned size = (*children).size();
        if (idx >= size) {
            assert(false);
            return false;
        }

        if (size == 1) {
            children->clear();
        } else {
            for (int i = idx; i < size - 1; i++) {
                (*children)[i] = (*children)[i + 1];
            }
            children->resize(size - 1);
        }
        return true;
    }

    unsigned size()
    {
        return (*children).size();
    }

    static void showTree(pASTNode pRoot);
    static std::tuple<std::string, unsigned, unsigned> parseTree2String(pASTNode pRoot, int indent);
    static bool switchAST(pASTNode pRoot);
};

void ASTNode::showTree(pASTNode pRoot)
{
    std::string graphFile = "/Users/luwenjie/git/jhin/jhin/src/ast/graph.py";
    std::tuple<std::string, unsigned, unsigned> tree = parseTree2String(pRoot, 0);
    std::string sPy = "";
    sPy += "depth = " + std::to_string(std::get<2>(tree)) + "\n";
    sPy += "tree = \\\n";
    comm::Log::singleton(INFO, false, graphFile) >> sPy >> std::get<0>(tree) >> comm::newline;
}

/* ret: <tree list string, width, depth> */
std::tuple<std::string, unsigned, unsigned> ASTNode::parseTree2String(pASTNode pRoot, int indent)
{
    if (pRoot == nullptr) return std::make_tuple("[]", 0, 0);

    std::string sBlank(indent * 4, ' ');
    /* python tuple: (symbolString, width, symboltype) */
    SymbolType stype = comm::symbolId2Type(pRoot->data->symbolId);
    std::string s1 = "[(\"" + pRoot->data->toString() + "\", ";
    std::string s2 = ", " + std::to_string(stype) + ")";
    unsigned curWidth = 1;
    unsigned maxDepth = 0;
    if (pRoot->children != nullptr) {
        for (pASTNode p: *(pRoot->children)) {
            /* pASTNodes we push_back before can not be nullptr */
            assert(p != nullptr);
            auto tu = parseTree2String(p, indent + 1);
            maxDepth = std::max(std::get<2>(tu), maxDepth);
            curWidth += std::get<1>(tu);
            s2 += ", \n" + sBlank + std::get<0>(tu);
        }
    }
    curWidth = (curWidth == 1 ? 1 : curWidth - 1);
    s2 += "]";

    return std::make_tuple(s1 + std::to_string(curWidth) + s2, curWidth, maxDepth + 1);
}

bool ASTNode::switchAST(pASTNode pRoot)
{

    return true;
}


};  /* namsspace ast */
};  /* namespace jhin */
