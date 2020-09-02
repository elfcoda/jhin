#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "../ast/ast_node.h"

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
 * -----------  ... rbp
 * | args... |  ... link with symbol table
 * -----------
 * | ret adr |  ... callq
 * -----------
 * | loc arg |  ... link with symbol table
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
    {E_COMPARE_TYPE_GT, "j "},
    {E_COMPARE_TYPE_GE, "j "},
    {E_COMPARE_TYPE_LT, "j "},
    {E_COMPARE_TYPE_LE, "j "}
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
    td::string sJmp = mCmpType2Instr.at(eType);
    return sJmp + lbl;
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
    std::shared_ptr<symbolItem> ptr = st::symbolTable::find_symbol_in_scope(symbolName);
    if (ptr == nullptr) {
        /* varibles should be in this scope */
        assert(!"symbol should be in this scope.");
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
    s += "movq %rbx, -" + delta + "(%rbp)\n";
    return s;
}

std::string genDeclRemainSpace(unsigned declNum)
{
    std::string s = "";
    s += "subq " + SIZE_OF_STACK_ELEMENT * declNum + ", %rsp\n";
    return s;
}

std::string genDecl(const std::string& symbolName, const std::string& value)
{
    std::string s = "";
    unsigned delta = getSymbolIdx(symbolName) * SIZE_OF_STACK_ELEMENT;
    s += "movq " + value + ", -" + delta + "(%rbp)\n";
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

std::string genLeaf()
{
    std::string s = "";
    return s;
}

/*
 * instruction
 * if label
 * instruction
 * other label
 * */
std::string genIf(ast::pASTNode pNode)
{
    assert(pNode != nullptr && pNode->size() == 2);
    ast::pASTNode nodeCond = pNode->getChild(0);
    ast::pASTNode nodeIfBody = pNode->getChild(1);

    genCompare()
}

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

