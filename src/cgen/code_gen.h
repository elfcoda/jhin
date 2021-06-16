#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "../ast/ast_node.h"
#include "../st/symbol_def.h"
#include "../st/symbol_table.h"
#include "../../comm/type_tree.h"

namespace jhin
{
namespace cgen
{
/* memory layout:
 * -----------
 * | code    |       Low address
 * -----------
 * | heap    |
 * -----------
 * | ...     |
 * -----------
 * | stack   |       High address
 * -----------
 *
 * register:
 * rbp: bottom of stack ptr
 * rsp: top of stack ptr
 * rip: code ptr
 * rax: function return value
 * rbx: result register
 * rdx: tmp register
 *
 * one register stack machine:
 * -----------      High address
 * | ...     |
 * -----------
 * | ret adr |  ... pushq
 * -----------
 * | rbp     |  ... rbp
 * -----------
 * | args... |  ... link with symbol table
 * -----------
 * | loc var |  ... link with symbol table, and jmp
 * | ...     |
 * -----------  ... rsp (available address)
 * | ...     |
 * -----------      Low address
 * */

const unsigned SIZE_OF_STACK_ELEMENT = 8;
const std::string lblPrefix = "jhin_label_";

enum ELabelType
{
    E_LABEL_TYPE_IF = 0,
    E_LABEL_TYPE_ELSE,
    E_LABEL_TYPE_WHILE,
    E_LABEL_TYPE_DECIMAL,
    E_LABEL_TYPE_OTHER,
};

enum ECompareType
{
    E_COMPARE_TYPE_EQ = 0,
    E_COMPARE_TYPE_NEQ,
    E_COMPARE_TYPE_GT,
    E_COMPARE_TYPE_GE,
    E_COMPARE_TYPE_LT,
    E_COMPARE_TYPE_LE,
};
const std::unordered_map<ECompareType, std::string> mCmpType2Instr = {
    {E_COMPARE_TYPE_EQ, "je "},
    {E_COMPARE_TYPE_NEQ, "jne "},
    {E_COMPARE_TYPE_GT, "jg "},
    {E_COMPARE_TYPE_GE, "jge "},
    {E_COMPARE_TYPE_LT, "jl "},
    {E_COMPARE_TYPE_LE, "jle "}
};

/**/
union float2Int
{
    float f;
    int i;
};
std::string float2IntStr(float f)
{
    float2Int fI;
    fI.f = f;
    return std::to_string(fI.i);
}

union double2Long
{
    double d;
    long l;
};
std::string double2LongStr(double d)
{
    double2Long dL;
    dL.d = d;
    return std::to_string(dL.l);
}

std::string genCompareType(ECompareType eType, const std::string& lbl)
{
    assert(mCmpType2Instr.find(eType) != mCmpType2Instr.end());
    std::string sJmp = mCmpType2Instr.at(eType);
    return sJmp + lbl + "\n";
}


const std::unordered_map<ELabelType, std::string> mLabelType2LabelBody = {
    {E_LABEL_TYPE_IF, "if_"},
    {E_LABEL_TYPE_ELSE, "else_"},
    {E_LABEL_TYPE_WHILE, "while_"},
    {E_LABEL_TYPE_DECIMAL, "LCPI_"},
    {E_LABEL_TYPE_OTHER, "lbl_"}
};

std::string getLabelBodyByLabelType(ELabelType lbltype)
{
    assert(mLabelType2LabelBody.find(lbltype) != mLabelType2LabelBody.end());
    return mLabelType2LabelBody.at(lbltype);
}

std::string genLabel(ELabelType lbltype = E_LABEL_TYPE_OTHER)
{
    static unsigned long long lblIdx = 0;
    lblIdx += 1;
    assert(lblIdx < ULLONG_MAX);
    std::string labelBody = getLabelBodyByLabelType(lbltype);
    return lblPrefix + labelBody + std::to_string(lblIdx);
}

unsigned getSymbolIdx(const std::string& symbolName)
{
    std::shared_ptr<st::symbolItem> ptr = st::symbolTable::find_symbol_in_fn(symbolName);
    if (ptr == nullptr) {
        /* varibles should be in this scope */
        assert(!"symbol should be in this fn scope.");
    }
    return ptr->getIdx();
}


/* %rbx is the register which store value calculated by each expresion.
 * %rdx is temporary register.
 *
 * TODO: + - * / for float and double
 * */
std::string genPlus(const std::string& asmCode1, const std::string& asmCode2)
{
    std::string s = "";
    s += asmCode1;
    s += "pushq %rbx\n";
    s += asmCode2;
    s += "popq %rdx\n";
    s += "addq %rdx, %rbx\n";
    return s;
}

std::string genMinus(const std::string& asmCode1, const std::string& asmCode2)
{
    std::string s = "";
    s += asmCode2;
    s += "pushq %rbx\n";
    s += asmCode1;
    s += "popq %rdx\n";
    s += "subq %rdx, %rbx\n";
    return s;
}

std::string genStar(const std::string& asmCode1, const std::string& asmCode2)
{
    std::string s = "";
    s += asmCode1;
    s += "pushq %rbx\n";
    s += asmCode2;
    s += "popq %rax\n";
    s += "mulq %rbx\n";
    s += "movq %rax, %rbx\n";
    return s;
}

std::string genSlash(const std::string& asmCode1, const std::string& asmCode2)
{
    std::string s = "";
    s += asmCode1;
    s += "pushq %rbx\n";
    s += asmCode2;
    s += "popq %rax\n";
    s += "divq %rbx\n";
    s += "movq %rax, %rbx\n";
    return s;
}

std::string genCompare(const std::string& asmCode1, const std::string& asmCode2)
{
    std::string s = "";
    s += asmCode1;
    s += "pushq %rbx\n";
    s += asmCode2;
    s += "popq %rdx\n";
    s += "cmpq %rdx, %rbx\n";
    return s;
}

std::string genAssign(const std::string& symbolName, const std::string& asmCode2)
{
    std::string s = "";
    s += asmCode2;
    unsigned delta = getSymbolIdx(symbolName) * SIZE_OF_STACK_ELEMENT;
    /* save the data to memory */
    s += "movq %rbx, -" + std::to_string(delta) + "(%rbp)\n";
    return s;
}

std::string genDeclRemainSpace(unsigned declNum)
{
    std::string s = "";
    s += "subq " + std::to_string(SIZE_OF_STACK_ELEMENT * declNum) + ", %rsp\n";
    return s;
}

std::string genDecl(const std::string& symbolName, const std::string& value)
{
    std::string s = "";

/*
    // 生成代码的这个时候，符号已经刚刚被加入到符号表，所以能找到符号表中的这个符号
    unsigned fnArgsCnt = comm::fnArgsNumber(st::getRecentFn());

    unsigned symbolIdx = getSymbolIdx(symbolName);
    if (symbolIdx < fnArgsCnt) {
        // 暂时不支持默认参数，所以如果idx属于函数的参数列表的话，不生成代码
        return "";
    }

    unsigned delta = symbolIdx * SIZE_OF_STACK_ELEMENT;
    s += "movq " + value + ", -" + std::to_string(delta) + "(%rbp)\n";
*/

    return s;
}

std::string genREInt(const std::string& text)
{
    std::string s = "";
    s += "movq $" + text + ", %rbx\n";
    return s;
}

std::string genRELong(const std::string& text)
{
    std::string s = "";
    s += "movq $" + text + ", %rbx\n";
    return s;
}

std::string genBool(const std::string& text)
{
    std::string s = "";
    if (text == "True") s += "movq $1, %rbx\n";
    else if (text == "False") s += "movq $0, %rbx\n";
    else assert(!"Bool value error!");

    return s;
}

std::string genREString(const std::string& text)
{
    assert(!"todo");
    std::string s = "";
    return s;
}

std::string genDecimal(const std::string& text)
{
    assert(!"todo");
    std::string s = "";
    return s;
}



std::string genLeaf()
{
    std::string s = "";
    return s;
}

/*
 * cond: instruction
 * if-body: instruction
 * other instruction
 * */
std::string genIf(ast::pASTNode pNode, std::shared_ptr<st::symbolGenRtn> rtn1, std::shared_ptr<st::symbolGenRtn> rtn2)
{
    std::string s = "";
    assert(pNode != nullptr && rtn1 != nullptr && rtn2 != nullptr);
    /* if-condition's branch */
    ast::pASTNode pIf = pNode->getChild(0);
    assert(pIf != nullptr);
    std::string ifConditionText = pIf->getText();
    std::string s1 = rtn1->getAsmCode();
    std::string sIfBody = rtn2->getAsmCode();
    if (ifConditionText == "True") {
        return sIfBody;
    } else if (ifConditionText == "False") {
        return "";
    } else if (ifConditionText == "!") {
        assert(!"Not operator should be implemented first!");
    } else {
        unsigned childrenNum = pIf->size();
        assert(childrenNum == 2);
        // ast::pASTNode pLeft = pIf->getChild(0);
        // ast::pASTNode pRight = pIf->getChild(1);
        // std::string L = genSymbolTable(pLeft)->getAsmCode();
        // std::string R = genSymbolTable(pRight)->getAsmCode();
        // std::string sLR = genCompare(L, R);
        std::string ifLabel = genLabel(E_LABEL_TYPE_IF);
        s = s1;

        if (ifConditionText == "==") {
            s += genCompareType(E_COMPARE_TYPE_NEQ, ifLabel);
        } else if (ifConditionText == "!=") {
            s += genCompareType(E_COMPARE_TYPE_EQ, ifLabel);
        } else if (ifConditionText == ">") {
            s += genCompareType(E_COMPARE_TYPE_GE, ifLabel);
        } else if (ifConditionText == ">=") {
            s += genCompareType(E_COMPARE_TYPE_GT, ifLabel);
        } else if (ifConditionText == "<") {
            s += genCompareType(E_COMPARE_TYPE_LE, ifLabel);
        } else if (ifConditionText == "<=") {
            s += genCompareType(E_COMPARE_TYPE_LT, ifLabel);
        } else {
            assert(!"undefined syntax!");
        }

        s += sIfBody;
        s += ifLabel + ":";
    }

    return s;
}

/*
 * cond: instruction
 * while-body: instruction
 * other instruction
 * */
std::string genWhile(ast::pASTNode pNode, std::shared_ptr<st::symbolGenRtn> rtn1, std::shared_ptr<st::symbolGenRtn> rtn2)
{
    std::string s = "";
    assert(pNode != nullptr && rtn1 != nullptr && rtn2 != nullptr);
    /* while-condition's branch */
    ast::pASTNode pWhile = pNode->getChild(0);
    assert(pWhile != nullptr);
    std::string whileConditionText = pWhile->getText();
    std::string s1 = rtn1->getAsmCode();
    std::string sWhileBody = rtn2->getAsmCode();
    if (whileConditionText == "True") {
        std::string sLbl = genLabel(E_LABEL_TYPE_WHILE);
        s = sLbl + ":\n";
        s += sWhileBody;
        s += "jmp " + sLbl + "\n";
        return s;
    } else if (whileConditionText == "False") {
        return "\n";
    } else if (whileConditionText == "!") {
        assert(!"Not operator should be implemented first!");
    } else {
        unsigned childrenNum = pWhile->size();
        assert(childrenNum == 2);
        // ast::pASTNode pLeft = pWhile->getChild(0);
        // ast::pASTNode pRight = pWhile->getChild(1);
        // std::string L = genSymbolTable(pLeft)->getAsmCode();
        // std::string R = genSymbolTable(pRight)->getAsmCode();
        // std::string sLR = genCompare(L, R);
        std::string startLbl = genLabel(E_LABEL_TYPE_WHILE);
        std::string whileLabel = genLabel(E_LABEL_TYPE_WHILE);
        s = startLbl + ":\n";
        s += s1;

        if (whileConditionText == "==") {
            s += genCompareType(E_COMPARE_TYPE_NEQ, whileLabel);
        } else if (whileConditionText == "!=") {
            s += genCompareType(E_COMPARE_TYPE_EQ, whileLabel);
        } else if (whileConditionText == ">") {
            s += genCompareType(E_COMPARE_TYPE_GE, whileLabel);
        } else if (whileConditionText == ">=") {
            s += genCompareType(E_COMPARE_TYPE_GT, whileLabel);
        } else if (whileConditionText == "<") {
            s += genCompareType(E_COMPARE_TYPE_LE, whileLabel);
        } else if (whileConditionText == "<=") {
            s += genCompareType(E_COMPARE_TYPE_LT, whileLabel);
        } else {
            assert(!"undefined syntax!");
        }

        s += sWhileBody;
        s += "jmp " + startLbl + "\n";
        s += whileLabel + ":\n";
    }

    return s;
}

/*
 * cond: instruction
 * j labelIf
 * if-body: instruction
 * j labelElse
 * labelIf:
 * else-body: instruction
 * labelElse:
 * other instruction
 * */
std::string genIfElse(ast::pASTNode pNode,
                      std::shared_ptr<st::symbolGenRtn> rtn1,
                      std::shared_ptr<st::symbolGenRtn> rtn2,
                      std::shared_ptr<st::symbolGenRtn> rtn3)
{
    std::string s = "";
    assert(pNode != nullptr && rtn1 != nullptr && rtn2 != nullptr && rtn3 != nullptr);
    /* if-condition's branch */
    ast::pASTNode pIf = pNode->getChild(0);
    assert(pIf != nullptr);
    std::string ifConditionText = pIf->getText();
    std::string s1 = rtn1->getAsmCode();
    std::string sIfBody = rtn2->getAsmCode();
    std::string sElseBody = rtn3->getAsmCode();
    if (ifConditionText == "True") {
        return sIfBody;
    } else if (ifConditionText == "False") {
        return sElseBody;
    } else if (ifConditionText == "!") {
        assert(!"Not operator should be implemented first!");
    } else {
        unsigned childrenNum = pIf->size();
        assert(childrenNum == 2);
        std::string ifLabel = genLabel(E_LABEL_TYPE_IF);
        std::string elseLabel = genLabel(E_LABEL_TYPE_ELSE);
        s = s1;

        if (ifConditionText == "==") {
            s += genCompareType(E_COMPARE_TYPE_NEQ, ifLabel);
        } else if (ifConditionText == "!=") {
            s += genCompareType(E_COMPARE_TYPE_EQ, ifLabel);
        } else if (ifConditionText == ">") {
            s += genCompareType(E_COMPARE_TYPE_GE, ifLabel);
        } else if (ifConditionText == ">=") {
            s += genCompareType(E_COMPARE_TYPE_GT, ifLabel);
        } else if (ifConditionText == "<") {
            s += genCompareType(E_COMPARE_TYPE_LE, ifLabel);
        } else if (ifConditionText == "<=") {
            s += genCompareType(E_COMPARE_TYPE_LT, ifLabel);
        } else {
            assert(!"undefined syntax!");
        }

        s += sIfBody;
        s += "jmp " + elseLabel + "\n";
        s += ifLabel + ":\n";
        s += sElseBody;
        s += elseLabel + ":\n";
    }

    return s;
}

std::string genFnTail()
{
    std::string s = "";
    s += "movq %rbp, %rsp\n";
    s += "popq %rbp\n";
    s += "popq %rip\n"; /* retq */
    return s;
}

/* function call */
std::string genFnCall(const std::string& fnName, pTypeTree pArgsTree)
{
    return "";
    /*
    std::string fnLabel = "_" + fnName;
    std::string s = "";
    s += "pushq %rip\n";
    s += "pushq %rbp\n";
    s += "movq %rsp, %rbp\n";
    if (pArgsTree == nullptr || !pArgsTree->hasChildren()) {
        // call function
        s += "jmp " + fnLabel + "\n";
        return s;
    }

    for (int i = 0; i < pArgsTree->size(); i++) {
        pTypeTree pTT = pArgsTree->getChild(i);
        auto argType = pTT->getType();
        unsigned delta = pTT->getIdx() * SIZE_OF_STACK_ELEMENT;
        if (argType == SYMBOL_TYPE_INT) {
            // Int Value
            s += "movq " + pTT->getValue() + ", " + std::to_string(delta) + "(%rbp)\n";
        } else if (argType == SYMBOL_TYPE_BOOL) {
            // Bool value
            std::string sValue = "";
            if (pTT->getValue() == "True") sValue = "1";
            else if (pTT->getValue() == "False") sValue = "0";
            else assert(!"invalid bool value!");
            s += "movq " + sValue + ", " + std::to_string(delta) + "(%rbp)\n";
        } else {
            // unsupported value
            assert(!"unsupported value type!");
        }
    }

    s += "jmp " + fnLabel + "\n";
    return s;
    */
}

std::string genBySymbol(pTypeTree pTT)
{
    return "";
    /*
    assert(pTT != nullptr);
    std::string s = "";
    unsigned delta = 0;
    // pTypeTree里的value值是声明时的值，在声明的时候已经放入栈中，这里使用
    // 这个符号的时候要去栈中取
    if (pTT->getType() == SYMBOL_TYPE_INT) {
        delta = pTT->getIdx() * SIZE_OF_STACK_ELEMENT;
        s = "movq " + std::to_string(delta) + "(%rbp), %rbx\n";
    } else if (pTT->getType() == SYMBOL_TYPE_BOOL) {
        delta = pTT->getIdx() * SIZE_OF_STACK_ELEMENT;
        s = "movq " + std::to_string(delta) + "(%rbp), %rbx\n";
    } else {
        assert(!"unsupported value type!");
    }

    return s;
    */
}

////////////////////////////////simd instructions
std::string genREFloat(const std::string& text)
{
    std::string s = "";
    std::string fInt = float2IntStr(std::stof(text));
    s += "movss $" + fInt + ", \%xmm0\n";
    return s;
}

std::string genREDouble(const std::string& text)
{
    std::string s = "";
    std::string dLong = double2LongStr(std::stod(text));
    s += "movsd $" + dLong + ", \%xmm0\n";
    return s;
}


};  /* namespace cgen */
};  /* namespace jhin */

