#pragma once

#include <set>
#include <unordered_set>
#include <stack>
#include <queue>
#include <string>
#include "llvm/Support/Casting.h"
#include "pt.h"
#include "ast_node.h"
#include "ast_node_semantic.h"
#include "../st/symbol_table.h"
#include "../../comm/log.h"
#include "../../comm/comm.h"
#include "../../comm/jhin_assert.h"
#include "../lex/keywords.h"
#include "../syntax/non_terminal.h"
#include "../syntax/syntax_nfa.h"

namespace jhin
{
namespace ast
{

const std::unordered_set<std::string> cutSetStr = {"LPAREN", "RPAREN", "LCURLY", "RCURLY", "LBRACK", "RBRACK", "NEWLINE", "COMMA"};
const std::string EpsilonStr = "EPSILON";

template <typename T>
std::unique_ptr<T> dynamic_cast_ast(std::unique_ptr<ASTBase> base)
{
    T *ptr = dynamic_cast<T *>(base.get());
    std::unique_ptr<T> derivedPtr;
    if (ptr != nullptr) {
        base.release();
        derivedPtr.reset(ptr);
    }

    return derivedPtr;
}

class AST
{
    public:
        pASTNode parseTree2AST(pASTNode pRoot)
        {
            init();
            cut(pRoot, nullptr, 0);

            /* generate AST */
            liftTokens(pRoot);

            comm::Log::singleton(INFO) >> mASTNonLeafStr2Id >> comm::newline;
            // ASTNode::showTree(pRoot);

            return pRoot;
        }

        // std::set<std::string> typeSymbolString = {
        //     "OBJECT",
        //     "BOOL",
        //     "INT",
        //     "FLOAT",
        //     "DOUBLE",
        //     "LONG",
        //     "STRING",
        //     "UNIT"
        // };

        // bool isTypeSymbolString(const std::string& str)
        // {
        //     return typeSymbolString.find(str) != typeSymbolString.end();
        // }


        std::unique_ptr<CallExprAST> updateFnCallArgs(std::unique_ptr<CallExprAST> FnCall, pASTNode genArgs)
        {
            // NOTE_QUES(Exp_ar)
            std::string genArgsStr0 = genArgs->getChild(0)->getText();
            if ("Exp_ar" == genArgsStr0) {
                std::unique_ptr<ExprAST> ExpArExp = dynamic_cast_ast<ExprAST>(parseTree2LLVMAST(genArgs->getChild(0)->getChild(0)));
                FnCall->addExp(std::move(ExpArExp));

                auto genRestExp = genArgs->getChild(0)->getChild(1);
                while (genRestExp != nullptr) {
                    std::string genRestExpStr0 = genRestExp->getChild(0)->getText();
                    if ("," == genRestExpStr0) {
                        std::unique_ptr<ExprAST> ExpArRestExpNode = dynamic_cast_ast<ExprAST>(parseTree2LLVMAST(genRestExp->getChild(1)));
                        FnCall->addExp(std::move(ExpArRestExpNode));
                        genRestExp = genRestExp->getChild(2);
                    } else if (EpsilonStr == genRestExpStr0) {
                        return FnCall;
                    } else {
                        JHIN_ASSERT_STR("FnCall Error!");
                    }
                }
            } else if (EpsilonStr == genArgsStr0) {
                return FnCall;
            } else {
                JHIN_ASSERT_STR("FnCall Error!");
            }

            return nullptr;
        }

        template <typename AST, typename DataType>
        std::unique_ptr<AST> makeASTAndDeclType(const DataType& data)
        {
            std::unique_ptr<AST> uniqAST = std::make_unique<AST>(data);
            uniqAST->typeDecl();
            return uniqAST;
        }

        template <typename AST, typename... Ds>
        std::unique_ptr<AST> makeASTAndDeclTypeArgs(Ds... ds)
        {
            std::unique_ptr<AST> uniqAST = std::make_unique<AST>(ds...);
            uniqAST->typeDecl();
            return uniqAST;
        }

        /// Index All From "jhin/src/syntax/non_terminal.h"
        std::unique_ptr<ASTBase> parseTree2LLVMAST(pASTNode pRoot)
        {
            // top to down
            JHIN_ASSERT_BOOL(pRoot != nullptr);
            std::string symbolStr = ast::getSymbolString(pRoot);

            // collect all raw text in vector
            std::vector<std::string> vSymStr;
            unsigned clrSize = pRoot->size();
            for (unsigned i = 0; i < clrSize; i++) {
                vSymStr.push_back(pRoot->getChild(i)->getText());
            }

            comm::Log::singleton(INFO) >> "symbolStr: " >> symbolStr >> comm::newline;
            // travel ParseTree according to syntax definition file
            if ("Prog'" == symbolStr) {
                return parseTree2LLVMAST(pRoot->getChild(1));
            } else if ("Prog" == symbolStr) {
                pASTNode genProg = pRoot->getChild(0);

                std::unique_ptr<ProgUnitsAST> decls = std::make_unique<ProgUnitsAST>();
                while (genProg != nullptr) {
                    std::string genProgStr0 = genProg->getChild(0)->getText();
                    if ("Prog_u" == genProgStr0) {
                        // get a class or function definition
                        std::unique_ptr<ProgUnitAST> ProgDeclNode = dynamic_cast_ast<ProgUnitAST>(parseTree2LLVMAST(genProg->getChild(0)));
                        decls->addProgUnit(std::move(ProgDeclNode));
                        genProg = genProg->getChild(1);
                    } else if (EpsilonStr == genProgStr0) {
                        decls->typeDecl();
                        return decls;
                    } else {
                        JHIN_ASSERT_STR("Prog Error!");
                    }
                }
            } else if ("Prog_u" == symbolStr) {
                return parseTree2LLVMAST(pRoot->getChild(0));
            } else if ("Cmd" == symbolStr) {
                return parseTree2LLVMAST(pRoot->getChild(0));
            } else if ("CmdN" == symbolStr) {
                std::string name = pRoot->getChild(0)->getText();
                std::unique_ptr<ExprAST> CmdNExp = dynamic_cast_ast<ExprAST>(parseTree2LLVMAST(pRoot->getChild(2)));

                auto uniqAST = std::make_unique<AssignCmdAST>(name, std::move(CmdNExp));
                uniqAST->typeDecl();
                return uniqAST;
            } else if ("CmdC" == symbolStr) {
                std::string CmdCStr0 = vSymStr[0];
                std::unique_ptr<ExprAST> CmdCCond = dynamic_cast_ast<ExprAST>(parseTree2LLVMAST(pRoot->getChild(2)));
                symbolTable::add_symbol_tag(ST_THEN_SCOPE);
                std::unique_ptr<FormalsAST> CmdCThen = dynamic_cast_ast<FormalsAST>(parseTree2LLVMAST(pRoot->getChild(6)));
                symbolTable::pop_symbol_block();
                if ("while" == CmdCStr0) {
                    auto uniqAST = std::make_unique<WhileCmdAST>(std::move(CmdCCond), std::move(CmdCThen));
                    uniqAST->typeDecl();
                    return uniqAST;
                } else if ("if" == CmdCStr0) {
                    if (8 == clrSize) {
                        auto uniqAST = std::make_unique<IfCmdAST>(std::move(CmdCCond), std::move(CmdCThen), nullptr);
                        uniqAST->typeDecl();
                        return uniqAST;
                    } else if (12 == clrSize) {
                        symbolTable::add_symbol_tag(ST_ELSE_SCOPE);
                        std::unique_ptr<FormalsAST> CmdCElse = dynamic_cast_ast<FormalsAST>(parseTree2LLVMAST(pRoot->getChild(10)));
                        symbolTable::pop_symbol_block();
                        auto uniqAST = std::make_unique<IfCmdAST>(std::move(CmdCCond), std::move(CmdCThen), std::move(CmdCElse));
                        uniqAST->typeDecl();
                        return uniqAST;
                    } else {
                        JHIN_ASSERT_STR("CmdC Error!");
                    }
                } else {
                    JHIN_ASSERT_STR("CmdC Error!");
                }
            } else if ("CmdU" == symbolStr) {
                std::string CmdUStr0 = vSymStr[0];
                std::string CmdUStr6 = vSymStr[6];
                if ("while" == CmdUStr0) {
                    std::unique_ptr<ExprAST> WhileExpr = dynamic_cast_ast<ExprAST>(parseTree2LLVMAST(pRoot->getChild(2)));
                    symbolTable::add_symbol_tag(ST_THEN_SCOPE);
                    std::unique_ptr<FormalAST> WhileFormal = dynamic_cast_ast<FormalAST>(parseTree2LLVMAST(pRoot->getChild(5)));
                    symbolTable::pop_symbol_block();
                    auto uniqAST = std::make_unique<WhileCmdAST>(std::move(WhileExpr), std::move(WhileFormal));
                    uniqAST->typeDecl();
                    return uniqAST;
                } else if ("if" == CmdUStr0) {
                    // get Exp, FormalU
                    std::unique_ptr<ExprAST> IfCond = dynamic_cast_ast<ExprAST>(parseTree2LLVMAST(pRoot->getChild(2)));
                    symbolTable::add_symbol_tag(ST_THEN_SCOPE);
                    std::unique_ptr<FormalAST> IfThen = dynamic_cast_ast<FormalAST>(parseTree2LLVMAST(pRoot->getChild(5)));
                    symbolTable::pop_symbol_block();
                    if ("Newlq" == CmdUStr6) {
                        symbolTable::add_symbol_tag(ST_ELSE_SCOPE);
                        std::unique_ptr<FormalAST> IfElse = dynamic_cast_ast<FormalAST>(parseTree2LLVMAST(pRoot->getChild(9)));
                        symbolTable::pop_symbol_block();
                        auto uniqAST = std::make_unique<IfCmdAST>(std::move(IfCond), std::move(IfThen), std::move(IfElse));
                        uniqAST->typeDecl();
                        return uniqAST;
                    } else if ("'back_n'" == CmdUStr6) {
                        auto uniqAST = std::make_unique<IfCmdAST>(std::move(IfCond), std::move(IfThen), nullptr);
                        uniqAST->typeDecl();
                        return uniqAST;
                    } else {
                        JHIN_ASSERT_STR("CmdU Error!");
                    }
                } else {
                    JHIN_ASSERT_STR("CmdU Error!");
                }
            } else if ("Formals" == symbolStr) {
                return parseTree2LLVMAST(pRoot->getChild(1));
            } else if ("Formalt" == symbolStr) {
                std::string FormaltStr0 = vSymStr[0];
                if ("Formal" == FormaltStr0) {
                    std::unique_ptr<FormalAST> FormalNode = dynamic_cast_ast<FormalAST>(parseTree2LLVMAST(pRoot->getChild(0)));
                    pASTNode genFormal = pRoot->getChild(1);
                    std::string genFormalStr0 = genFormal->getChild(0)->getText();
                    if ("Newlp" == genFormalStr0) {
                        std::unique_ptr<FormalsAST> Formals = dynamic_cast_ast<FormalsAST>(parseTree2LLVMAST(genFormal->getChild(1)));
                        Formals->addFormal(std::move(FormalNode));
                        return Formals;
                    } else if (EpsilonStr == genFormalStr0) {
                        auto uniqAST = std::make_unique<FormalsAST>(std::move(FormalNode));
                        uniqAST->typeDecl();
                        return uniqAST;
                    } else {
                        JHIN_ASSERT_STR("Formalt Error!");
                    }
                } else if (EpsilonStr == FormaltStr0) {
                    auto uniqAST = std::make_unique<FormalsAST>();
                    uniqAST->typeDecl();
                    return uniqAST;
                } else {
                    JHIN_ASSERT_STR("Formalt Error!");
                }
            } else if ("Formal" == symbolStr) {
                return parseTree2LLVMAST(pRoot->getChild(0));
            } else if ("FormalU" == symbolStr) {
                return parseTree2LLVMAST(pRoot->getChild(0));
            } else if ("Newlp" == symbolStr ||
                       "Newls" == symbolStr ||
                       "Newlq" == symbolStr)
            {
                auto uniqAST = std::make_unique<EmptyAST>();
                uniqAST->typeDecl();
                return uniqAST;
            } else if ("Decls" == symbolStr) {
                return parseTree2LLVMAST(pRoot->getChild(1));
            } else if ("Declt" == symbolStr) {
                std::string DecltStr0 = vSymStr[0];
                if ("Decl" == DecltStr0) {
                    std::unique_ptr<DeclarationAST> DeclAST = dynamic_cast_ast<DeclarationAST>(parseTree2LLVMAST(pRoot->getChild(0)));
                    pASTNode genNode = pRoot->getChild(1);

                    std::string genStr0 = genNode->getChild(0)->getText();
                    if ("Newlp" == genStr0) {
                        // Declt   ::= Decl NOTE_QUES(Newlp Declt)
                        std::unique_ptr<DeclarationsAST> DecltAST = dynamic_cast_ast<DeclarationsAST>(parseTree2LLVMAST(genNode->getChild(1)));   // Declt
                        DecltAST->addDecl(std::move(DeclAST));
                        return DecltAST;
                    } else if (EpsilonStr == genStr0) {
                        auto uniqAST = std::make_unique<DeclarationsAST>(std::move(DeclAST));
                        uniqAST->typeDecl();
                        return uniqAST;
                    } else {
                        JHIN_ASSERT_STR("Declt Error!");
                    }
                } else if (EpsilonStr == DecltStr0) {
                    auto uniqAST = std::make_unique<DeclarationsAST>();
                    uniqAST->typeDecl();
                    return uniqAST;
                } else {
                    JHIN_ASSERT_STR("Declt Error!");
                }
            } else if ("Decl" == symbolStr) {
                return parseTree2LLVMAST(pRoot->getChild(0));
            } else if ("DeclN" == symbolStr) {
                std::string DeclNStr0 = pRoot->getChild(0)->getText();
                unsigned DeclNSymbolId0 = pRoot->getChild(0)->getSymbolId();
                outs() << "DeclN: " << DeclNSymbolId0 << " " << RE_ID << "\n";
                if (static_cast<unsigned>(RE_ID) == DeclNSymbolId0) { // ("RE_ID" == DeclNStr0)
                    std::string DeclName = pRoot->getChild(0)->getText();
                    std::unique_ptr<TypeExprAST> DeclType = dynamic_cast_ast<TypeExprAST>(parseTree2LLVMAST(pRoot->getChild(2)));
                    if (pRoot->size() < 3) {
                        JHIN_ASSERT_STR("DeclN Error");
                    } else if (pRoot->size() == 3) {
                        auto ans = std::make_unique<DeclarationAST>(DeclName, std::move(DeclType));
                        ans->typeDecl();
                        symbolTable::add_symbol(ans->getName(), ans->getDeclpTT(), nullptr, ST_DEFAULT_SYMBOL);
                        return ans;
                    } else if (pRoot->size() == 5) {
                        std::unique_ptr<ExprAST> DeclVal = dynamic_cast_ast<ExprAST>(parseTree2LLVMAST(pRoot->getChild(4)));
                        auto ans = std::make_unique<DeclarationAST>(DeclName, std::move(DeclType), std::move(DeclVal));
                        outs() << "Declggx "<< "\n";
                        ans->typeDecl();
                        symbolTable::add_symbol(ans->getName(), ans->getDeclpTT(), nullptr, ST_DEFAULT_SYMBOL);
                        symbolTable::print();
                        return ans;
                    } else {
                        JHIN_ASSERT_STR("DeclN Error on size");
                    }
                } else if ("Proc" == DeclNStr0) {
                    return parseTree2LLVMAST(pRoot->getChild(0));
                } else {
                    JHIN_ASSERT_STR("DeclN Error!");
                }
            } else if ("Class" == symbolStr) {
                symbolTable::add_symbol_tag(ST_CLASS_SCOPE);
                std::string ClassStr2 = pRoot->getChild(2)->getText();
                std::string className = pRoot->getChild(1)->getText();
                if ("Newls" == ClassStr2) {
                    // TODO
                } else if ("inherits" == ClassStr2) {
                    // TODO
                } else {
                    JHIN_ASSERT_STR("Class Error!");
                }

                TypeTable::addType(className);

                symbolTable::pop_symbol_block();
            } else if ("Proc" == symbolStr) {   // Functions
                symbolTable::add_symbol_tag(ST_FUNCTION_SCOPE);

                // Function Name
                std::string FunctionName = pRoot->getChild(1)->getText();

                // Args
                std::vector<std::unique_ptr<DeclarationAST>> Args;

                pASTNode node = pRoot->getChild(3)->getChild(0)->getChild(0);
                std::string procArgs0 = ast::getSymbolString(node);
                if (EpsilonStr == procArgs0) {
                    // empty args, pass
                } else if ("Proc_ar" == procArgs0) {
                    std::string ArgName = node->getChild(0)->getText();
                    std::unique_ptr<TypeExprAST> ArgType = dynamic_cast_ast<TypeExprAST>(parseTree2LLVMAST(node->getChild(2)));
                    auto uniqAST0 = std::make_unique<DeclarationAST>(ArgName, std::move(ArgType));
                    uniqAST0->typeDecl();
                    Args.push_back(std::move(uniqAST0));
                    pASTNode genNode = node->getChild(3);

                    while (genNode != nullptr) {
                        std::string genStr0 = genNode->getChild(0)->getText();
                        if (EpsilonStr == genStr0) {
                            // do nothing
                            break;
                        } else if ("," == genStr0) {
                            // RE_ID
                            std::string ArgNameGen = genNode->getChild(1)->getText();
                            std::unique_ptr<TypeExprAST> ArgTypeGen = dynamic_cast_ast<TypeExprAST>(parseTree2LLVMAST(genNode->getChild(3)));
                            auto uniqAST = std::make_unique<DeclarationAST>(ArgNameGen, std::move(ArgTypeGen));
                            uniqAST->typeDecl();
                            Args.push_back(std::move(uniqAST));

                            // GEN_TERMINAL_x recursively
                            genNode = genNode->getChild(4);
                        } else {
                            JHIN_ASSERT_STR("Proc_ar Error!");
                            break;
                        }
                    }
                } else {
                    JHIN_ASSERT_STR("Args Error!");
                }

                // FnRetTp
                std::unique_ptr<TypeExprAST> RetTypeNode = dynamic_cast_ast<TypeExprAST>(parseTree2LLVMAST(pRoot->getChild(5)));

                /// 1. make PrototypeAST
                std::unique_ptr<PrototypeAST> FuncProto = std::make_unique<PrototypeAST>(FunctionName, std::move(Args), std::move(RetTypeNode));
                FuncProto->typeDecl();

                symbolTable::add_symbol(FunctionName, nullptr, nullptr, ST_DEFAULT_SYMBOL);

                /// 2. make Body: Formals
                std::unique_ptr<FormalsAST> FuncBody = dynamic_cast_ast<FormalsAST>(parseTree2LLVMAST(pRoot->getChild(8)));

                // pop symbol scope
                symbolTable::pop_symbol_block();

                auto uniqAST = std::make_unique<FunctionAST>(std::move(FuncProto), std::move(FuncBody));
                uniqAST->typeDecl();
                return uniqAST;
            } else if ("FnRetTp" == symbolStr) {
                std::string FnRetTpStr0 = ast::getSymbolString(pRoot->getChild(0)->getChild(0));
                comm::Log::singleton(INFO) >> "FnRetTpStr0: " >> FnRetTpStr0 >> comm::newline;

                if (EpsilonStr == FnRetTpStr0) {
                    auto uniqAST = std::make_unique<EmptyAST>();
                    uniqAST->typeDecl();
                    return uniqAST;
                } else if (":" == FnRetTpStr0) {
                    return parseTree2LLVMAST(pRoot->getChild(0)->getChild(1));
                } else {
                    JHIN_ASSERT_STR("FnRetTp Error!");
                }
            } else if ("Type" == symbolStr) {
                return parseTree2LLVMAST(pRoot->getChild(0));
            } else if ("Exp" == symbolStr) {
                std::string ExpSymbolStr0 = ast::getSymbolString(pRoot->getChild(0));
                unsigned ExpSymbolId0 = pRoot->getChild(0)->getSymbolId();
                if ("return" == ExpSymbolStr0) {
                    // TODO
                    JHIN_ASSERT_STR("Should be implemented first!");
                } else if ("case" == ExpSymbolStr0) {
                    JHIN_ASSERT_STR("Should be implemented first!");
                } else if ("THIS" == ExpSymbolStr0) {
                    JHIN_ASSERT_STR("Should be implemented first!");
                } else if ("let" == ExpSymbolStr0) {
                    JHIN_ASSERT_STR("Should be implemented first!");
                } else if (static_cast<unsigned>(RE_ID) == ExpSymbolId0) { // ("RE_ID" == ExpSymbolStr0)
                    // updateFnCallArgs
                    std::string CallSymbolStr1 = ast::getSymbolString(pRoot->getChild(1));
                    if ("." == CallSymbolStr1) {
                        JHIN_ASSERT_STR("Should be implemented first!");
                    } else if ("@" == CallSymbolStr1) {
                        JHIN_ASSERT_STR("Should be implemented first!");
                    } else {
                        JHIN_ASSERT_STR("Call Function Error!");
                    }
                } else if ("(" == ExpSymbolStr0) {
                    // updateFnCallArgs
                    std::string ExpCallSymbolStr3 = ast::getSymbolString(pRoot->getChild(3));
                    if ("." == ExpCallSymbolStr3) {
                        JHIN_ASSERT_STR("Should be implemented first!");
                    } else if ("@" == ExpCallSymbolStr3) {
                        JHIN_ASSERT_STR("Should be implemented first!");
                    } else {
                        JHIN_ASSERT_STR("Exp Function Call Error!");
                    }
                } else if ("lambda" == ExpSymbolStr0) {
                    std::string ExpLambdaSymbolStr1 = ast::getSymbolString(pRoot->getChild(1));
                    if ("->" == ExpLambdaSymbolStr1) {
                    } else if ("DeclN" == ExpLambdaSymbolStr1) {
                    } else {
                        JHIN_ASSERT_STR("Exp Lambda Error!");
                    }
                } else if ("Exp0" == ExpSymbolStr0) {
                    return parseTree2LLVMAST(pRoot->getChild(0));
                } else {
                    JHIN_ASSERT_STR("Exp Error!");
                }
            } else if (("Exp0" == symbolStr && "Exp1" == ast::getSymbolString(pRoot->getChild(0))) ||    // Exp0 Exp1 Exp2
                       ("Exp1" == symbolStr && "Exp2" == ast::getSymbolString(pRoot->getChild(0))) ||
                       ("Exp2" == symbolStr && "Exp3" == ast::getSymbolString(pRoot->getChild(0)))) {
                return parseTree2LLVMAST(pRoot->getChild(0));
            } else if ("Exp0" == symbolStr || "Exp1" == symbolStr || "Exp2" == symbolStr) {
                // biOp: == < <= > >= + - * /
                // std::string biOp = ast::getSymbolString(pRoot->getChild(1));
                std::string biOp = pRoot->getChild(1)->getText();

                std::unique_ptr<ExprAST> L = dynamic_cast_ast<ExprAST>(parseTree2LLVMAST(pRoot->getChild(0)));
                std::unique_ptr<ExprAST> R = dynamic_cast_ast<ExprAST>(parseTree2LLVMAST(pRoot->getChild(2)));

                auto uniqAST = std::make_unique<BinaryExprAST>(static_cast<EKeyWords>(getTokenIdByKeyWord(biOp)), std::move(L), std::move(R));
                uniqAST->typeDecl();
                return uniqAST;
            } else if ("Exp3" == symbolStr) {  // Exp3
                return parseTree2LLVMAST(pRoot->getChild(0));
            } else if ("NotExp" == symbolStr) {
                std::unique_ptr<ASTBase> astNode = parseTree2LLVMAST(pRoot->getChild(1));
                std::unique_ptr<BoolExprAST> boolNode = dynamic_cast_ast<BoolExprAST>(std::move(astNode));
                boolNode->inverse();
                return boolNode;
            } else if ("VoidExp" == symbolStr) {
                // TODO: add is void method
                JHIN_ASSERT_STR("Should be implemented first!");
            } else if ("NewObj" == symbolStr) {
                // TODO: new a class
                JHIN_ASSERT_STR("Should be implemented first!");
            } else if ("ExpN" == symbolStr) {    // ExpN
                return parseTree2LLVMAST(pRoot->getChild(0));
            } else if ("ReInt" == symbolStr) {
                return makeASTAndDeclType<IntExprAST, std::string>(pRoot->getChild(0)->getText());
            } else if ("ReDeci" == symbolStr) {
                return makeASTAndDeclType<FloatExprAST, float>(std::stof(pRoot->getChild(0)->getText()));
            } else if ("ReStr" == symbolStr) {
                return makeASTAndDeclType<StringExprAST, std::string>(pRoot->getChild(0)->getText());
            } else if ("ReId" == symbolStr) {
                return makeASTAndDeclType<VariableExprAST, std::string>(pRoot->getChild(0)->getText());
            } else if ("TRUE" == symbolStr) {
                return makeASTAndDeclType<BoolExprAST, bool>(true);
            } else if ("FALSE" == symbolStr) {
                return makeASTAndDeclType<BoolExprAST, bool>(false);
            } else if ("NewExp" == symbolStr) {
                return parseTree2LLVMAST(pRoot->getChild(1));
            } else if ("ReValue" == symbolStr) {
                JHIN_ASSERT_STR("Should be implemented first!");
            } else if ("OBJECT" == symbolStr) {
                return makeASTAndDeclType<TypeExprAST, std::string>(pRoot->getText());
            } else if ("BOOL" == symbolStr) {
                return makeASTAndDeclType<TypeExprAST, std::string>(pRoot->getText());
            } else if ("INT" == symbolStr) {
                return makeASTAndDeclType<TypeExprAST, std::string>(pRoot->getText());
            } else if ("FLOAT" == symbolStr) {
                return makeASTAndDeclType<TypeExprAST, std::string>(pRoot->getText());
            } else if ("DOUBLE" == symbolStr) {
                return makeASTAndDeclType<TypeExprAST, std::string>(pRoot->getText());
            } else if ("LONG" == symbolStr) {
                return makeASTAndDeclType<TypeExprAST, std::string>(pRoot->getText());
            } else if ("STRING" == symbolStr) {
                return makeASTAndDeclType<TypeExprAST, std::string>(pRoot->getText());
            } else if ("UNIT" == symbolStr) {
                return makeASTAndDeclType<TypeExprAST, std::string>(pRoot->getText());
            } else if ("FnCall" == symbolStr) {
                std::string fnName = pRoot->getChild(0)->getText();
                std::unique_ptr<CallExprAST> FnCall = makeASTAndDeclType<CallExprAST, std::string>(fnName);

                pASTNode genArgs = pRoot->getChild(2)->getChild(0);

                FnCall = updateFnCallArgs(std::move(FnCall), genArgs);
                return FnCall;
            } else {
                JHIN_ASSERT_STR("Parse AST Error!");
            }

            return nullptr;
        }

        bool init()
        {
            initCutSet();
            return true;
        }
    private:
        /* return: current sub-tree should be cut or not
         * @idx: pNode is the idx children of parent. necessary when parent is not nullptr */
        bool cut(pASTNode pNode, pASTNode parent, unsigned idx)
        {
            /**/
            JHIN_ASSERT_BOOL(pNode != nullptr);

            pASTNode p = pNode, singleChild = nullptr;
            while (true) {
                if (!p->hasChildren()) {
                    if (inCutSet(p)) return true;
                    else return false;
                } else if ((singleChild = p->getSingle()) != nullptr) {
                    if (parent != nullptr) {
                        (*(parent->children))[idx] = singleChild;
                        singleChild->setNotation(p->getNotation());
                        p = singleChild;
                    } else if (!singleChild->hasChildren()) {
                        // p->setSymbolId(singleChild->getSymbolId());
                        // p->children = nullptr;
                        return false;
                    } else {
                        /* if single child has children, we move the children to p */
                        // p->children = singleChild->children;
                        return false;
                    }
                } else {
                    for (unsigned i = 0; i < p->children->size();) {
                        pASTNode child = (*(p->children))[i];
                        bool isCut = cut(child, p, i);
                        if (isCut) {
                            p->erase(i);
                        } else {
                            i++;
                        }
                    }
                    if (p->children->size() > 1) return false;
                    if (p->children->size() == 0) continue;
                    if (p->children->size() == 1) {
                        singleChild = (*(p->children))[0];
                        if (parent != nullptr) {
                            (*(parent->children))[idx] = singleChild;
                            singleChild->setNotation(p->getNotation());
                            return false;
                        } else if (!singleChild->hasChildren()) {
                            // p->setSymbolId(singleChild->getSymbolId());
                            // p->children = nullptr;
                            return false;
                        } else {
                            // p->children = singleChild->children;
                            return false;
                        }
                    }
                }
            }
        }

        bool liftTokens(pASTNode pRoot)
        {
            std::string astLiftString = "";
            if (!pRoot->hasChildren()) {
                return true;
            }

            for (auto it = pRoot->children->begin(); it != pRoot->children->end(); ) {
                pASTNode pChild = *it;
                unsigned symbol = pChild->getSymbolId();
                if (comm::isTokenLeaf(symbol)) {
                    pChild->setAstSymbolId(getTokenLeafId(symbol));
                    it++;
                } else if (comm::isToken(symbol) && !comm::isTokenLeaf(symbol)) {
                    astLiftString += comm::symbolId2String(symbol, true) + "_";
                    it = pRoot->children->erase(it);
                } else if (comm::isNonTerminal(symbol)) {
                    liftTokens(pChild);
                    it++;
                } else {
                    JHIN_ASSERT_BOOL(false);
                }
            }
            if (astLiftString == "") astLiftString = AST_DEFAULT_TEXT;
            else astLiftString.pop_back();

            pRoot->setText(astLiftString);
            pRoot->setAstSymbolId(getNonLeafId(astLiftString));
            return true;
        }


        bool initCutSet()
        {
            for (const std::string& s: cutSetStr) {
                JHIN_ASSERT_BOOL(lex::string2TokenId.find(s) != lex::string2TokenId.end());
                cutSet.insert(lex::string2TokenId[s]);
            }

            for (unsigned nonterminal = NON_TERMINAL_IDX_MIN; nonterminal <= NON_TERMINAL_IDX_MAX; nonterminal++) {
                cutSet.insert(nonterminal);
            }

            cutSet.insert(SYNTAX_EPSILON_IDX);
            cutSet.insert(SYNTAX_TOKEN_END);
            cutSet.insert(SYNTAX_TOKEN_BEGIN);

            return true;
        }

        std::unordered_set<unsigned> cutSet;

        bool inCutSet(pASTNode p)
        {
            return cutSet.find(p->data->symbolId) != cutSet.end();
        }
};

}   /* namespace ast */
}   /* namespace jhin */
