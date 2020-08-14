#pragma once

#include <vector>
#include <string>
#include <tuple>
#include "../../comm/log.h"
#include "../../comm/comm.h"

namespace jhin
{
namespace ast
{

struct ASTNodeData;
using pASTNodeData = ASTNodeData*;

struct ASTNode;
using pASTNode = ASTNode*;

using pChildrenList = std::vector<pASTNode>*;

struct ASTNodeData
{
    /* non-terminal id or token id */
    unsigned symbolId;

    std::string toString()
    {
        std::string s = comm::symbolId2String(symbolId);
        return s;
    }

    ASTNodeData(unsigned symbolId)
    {
        this->symbolId = symbolId;
    }

    ASTNodeData(ASTNodeData&& ptnode)
    {
        this->symbolId = ptnode.symbolId;
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


};  /* namsspace ast */
};  /* namespace jhin */
