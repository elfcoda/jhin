#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <unordered_set>
#include <unordered_map>
#include "ast_leaf.h"
#include "ast_non_leaf.h"
#include "../../comm/log.h"
#include "../../comm/comm.h"

namespace jhin
{
namespace ast
{

#define AST_NOTATION_START  2048
#define AST_DEFAULT_TEXT    "AST"

enum EASTNotation
{
    AST_NOTATION_FN_ARGS = AST_NOTATION_START,
    AST_NOTATION_FN_RET_TYPE,
};

struct ASTNodeData;
using pASTNodeData = ASTNodeData*;

struct ASTNode;
using pASTNode = ASTNode*;

using pChildrenList = std::vector<pASTNode>*;

enum E_AST_MARK
{
    E_AST_MARK_FN = 0,
    E_AST_MARK_FN_HAS_ARGS,
};

struct ASTNodeData
{
    /* non-terminal id or token id */
    unsigned symbolId;
    /* semantics notation, eg, if this node is a function parameter */
    unsigned notation;

    /* raw string */
    std::string text;

    /* ast symbol id */
    unsigned astSymbolId;

    /* all kinds of bit-marks
     * bit-0: isFunction
     * */
    //////
    unsigned mark;

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
        this->notation = symbolId;
        this->text = text;
        this->astSymbolId = 0;
        this->mark = 0;
    }

    ASTNodeData(ASTNodeData&& ptnode)
    {
        this->symbolId = ptnode.symbolId;
        this->notation = ptnode.symbolId;
        this->text = ptnode.text;
        this->astSymbolId = 0;
        this->mark = 0;
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

    std::string toString() const
    {
        std::string s = "";
        s += "symbolId: " + std::to_string(data->symbolId) +
             ", notation: " + std::to_string(data->notation) +
             ", text: " + data->text +
             ", astSymbolId: " + std::to_string(data->astSymbolId) +
             ", mark: " + std::to_string(data->mark);
        s += "\n";
        return s;
    }

    /* free memory */
    bool free()
    {
        return true;
    }

    pASTNode getChild(unsigned idx)
    {
        assert(children != nullptr && idx < size());
        return (*children)[idx];
    }

    void setSymbolId(unsigned symbolId)
    {
        data->symbolId = symbolId;
    }

    void setNotation(unsigned notation)
    {
        data->notation = notation;
    }

    void setAstSymbolId(unsigned astSymbolId)
    {
        data->astSymbolId = astSymbolId;
    }

    void setText(const std::string& text)
    {
        data->text = text;
    }

    void setMark(unsigned mark)
    {
        data->mark = mark;
    }

    unsigned getNotation()
    {
        return data->notation;
    }

    unsigned getSymbolId()
    {
        return data->symbolId;
    }

    unsigned getAstSymbolId()
    {
        return data->astSymbolId;
    }

    std::string getText()
    {
        return data->text;
    }

    unsigned getMark()
    {
        return data->mark;
    }

    /* single child */
    pASTNode getSingle()
    {
        if (children == nullptr) return nullptr;
        if (children->size() == 1) return (*children)[0];
        return nullptr;
    }

    bool hasChildren()
    {
        return children != nullptr && !children->empty();
    }

    bool erase(unsigned idx)
    {
        unsigned size = children->size();
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
        if (children == nullptr) return 0;
        return children->size();
    }

    bool checkMark(E_AST_MARK idx)
    {
        unsigned u_idx = static_cast<unsigned>(idx);
        auto m = data->mark;
        unsigned bitNum = sizeof(decltype(m)) * 8;
        assert(u_idx < bitNum);
        return m & (1 << u_idx);
    }

    bool isFunction()
    {
        return checkMark(E_AST_MARK_FN);
    }

    bool fnHasArgs()
    {
        return checkMark(E_AST_MARK_FN_HAS_ARGS);
    }

    /* @isAdd: default to add mark */
    void setMark(E_AST_MARK idx, bool isAdd = true)
    {
        unsigned u_idx = static_cast<unsigned>(idx);
        unsigned bitNum = sizeof(decltype(data->mark)) * 8;
        assert(u_idx < bitNum);
        if (isAdd) {
            data->mark |= (1 << u_idx);
        } else {
            data->mark = (1 << u_idx);
        }
    }


    static void showTree(pASTNode pRoot);
    static std::tuple<std::string, unsigned, unsigned> parseTree2String(pASTNode pRoot, int indent);
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
            /* pASTNode(s) we push_back before can not be nullptr */
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

};  /* namsspace ast */
};  /* namespace jhin */
