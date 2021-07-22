#pragma once

#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "../jhin_module.h"
#include "../../comm/jhin_assert.h"
#include "../../comm/log.h"
#include "../../comm/type_tree.h"

using namespace llvm;
using namespace jhin;

namespace jhin
{
namespace ast
{

    /// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
    /// the function.  This is used for mutable variables etc.
    static AllocaInst *CreateEntryBlockAlloca(Function *TheFunction, Type *tp, StringRef VarName)
    {
        IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());
        return TmpB.CreateAlloca(tp, nullptr, VarName);
    }

    //===----------------------------------------------------------------------===//
    // Abstract Syntax Tree (aka Parse Tree)
    //===----------------------------------------------------------------------===//

    class ASTBase
    {
        public:
            virtual Value *codegen() = 0;
            virtual std::string toString() = 0;
            virtual std::string getASTName() = 0;
            virtual std::string getName() = 0;

            virtual bool IsEmpty() { return false; }

            virtual ~ASTBase() = default;
    };
    using pASTBase = ASTBase *;

    /// EmptyAST - for EPSILON and other empty case
    class EmptyAST: public ASTBase
    {
        public:
            Value *codegen() override { return nullptr; }
            bool IsEmpty() override { return true; }
            std::string toString() override { return ""; }
            std::string getASTName() override { return "EmptyAST"; }
            virtual std::string getName() override { return ""; }
    };

    class FormalAST : public ASTBase
    {
        public:
            virtual Value *codegen() override = 0;
            virtual std::string getName() override = 0;
            std::string toString() override { return ""; }
            std::string getASTName() override { return "FormalAST"; }
    };

    class FormalsAST : public ASTBase
    {
        private:
            std::vector<std::unique_ptr<FormalAST>> Formals;

        public:
            FormalsAST(std::vector<std::unique_ptr<FormalAST>> Formals)
                       : Formals(std::move(Formals)) {}

            FormalsAST(std::unique_ptr<FormalAST> FormalNode)
            {
                Formals.push_back(std::move(FormalNode));
            }

            FormalsAST() {}

            Value *codegen() override
            {
                codegenFormals();
                // TODO
                return nullptr;
            }

            virtual std::string getName() override { return ""; };

            std::vector<Value*> codegenFormals()
            {
                std::vector<Value*> ans;
                for (const  auto& p: Formals)
                {
                    ans.push_back(p->codegen());
                }
                return ans;
            }

            void addFormal(std::unique_ptr<FormalAST> FormalNode)
            {
                Formals.push_back(std::move(FormalNode));
            }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "FormalsAST"; }
    };

    /// ExprAST - Base class for all expression nodes
    class ExprAST : public FormalAST
    {
        public:
            virtual Value *codegen() override = 0;
            virtual std::string getName() override = 0;

            std::string toString() override { return ""; }
            std::string getASTName() override { return "ExprAST"; }
    };

    /// TypeExprAST - type for all type expression nodes
    class TypeExprAST final : public ExprAST {
        private:
            std::string typeName;
            pTypeTree pTT;
        public:
            TypeExprAST(std::string typeName): typeName(typeName)
            {
                comm::Log::singleton(INFO) >> "typeName: " >> typeName >> comm::newline;
                pTT = new TypeTree(SYMBOL_TYPE_BASIC, nullptr, "", "", "", nullptr);

                if ("Double" == typeName) {
                    pTT->setType(Type::getDoubleTy(*mdl::TheContext));
                } else if ("Float" == typeName) {
                    pTT->setType(Type::getFloatTy(*mdl::TheContext));
                } else if ("Int" == typeName) {
                    pTT->setType(IntegerType::get(*mdl::TheContext, 4 * 8));
                } else if ("String" == typeName) {
                    JHIN_ASSERT_STR("typeName Error! string");
                } else if ("Bool" == typeName) {
                    pTT->setType(Type::getInt1Ty(*mdl::TheContext));
                } else {
                    JHIN_ASSERT_STR("typeName Error!");
                }
            }
            Value *codegen() override { return nullptr; }
            virtual std::string getName() override { return ""; }

            pTypeTree getpTT() { return pTT; }
            Type* getType() { return pTT->getType(); }
            std::string toString() override { return ""; }
            std::string getASTName() override { return "TypeExprAST"; }
    };

    /// ComplitedTypeExprAST - type for all complited type expression nodes
    class ComplitedTypeExprAST final : public ExprAST
    {
        private:
            std::vector<Type *> CplType;
        public:
            ComplitedTypeExprAST()
            {
            }
            Value *codegen() override { return nullptr; }
            std::vector<Value*> codegenType()
            {
                std::vector<Value*> ans;
                // TODO
                //
                return ans;
            }

            virtual std::string getName() override { return ""; };

            std::string toString() override { return ""; }
            std::string getASTName() override { return "ComplitedTypeExprAST"; }
    };

    /// BoolExprAST - Expression class for Bool literals like "True".
    class BoolExprAST final : public ExprAST
    {
        private:
            bool Val;

        public:
            BoolExprAST(bool Val) : Val(Val) {}
            void inverse() {
                Val = !Val;
            }

            Value *codegen() override
            {
                return ConstantInt::get(*mdl::TheContext, APInt(1, Val, 2));
            }

            virtual std::string getName() override { return ""; }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "BoolExprAST"; }
    };

    /// IntExprAST - Expression class for Int literals like "1".
    class IntExprAST final : public ExprAST
    {
        private:
            std::string Val;

        public:
            IntExprAST(std::string Val) : Val(Val) {}

            Value *codegen() override
            {
                return ConstantInt::get(*mdl::TheContext, APInt(32, Val, 10));
            }
            virtual std::string getName() override { return ""; }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "IntExprAST"; }
    };

    /// FloatExprAST - Expression class for float literals like "1.0".
    class FloatExprAST final : public ExprAST
    {
        private:
            float Val;

        public:
            FloatExprAST(float Val) : Val(Val) {}

            Value *codegen() override
            {
                // return nullptr;
                return ConstantFP::get(*mdl::TheContext, APFloat(Val));
            }

            virtual std::string getName() override { return ""; }
            std::string toString() override { return ""; }
            std::string getASTName() override { return "FloatExprAST"; }
    };

    /// StringExprAST - Expression class for String literals like "str".
    class StringExprAST final : public ExprAST
    {
        private:
            std::string Val;

        public:
            StringExprAST(std::string Val) : Val(Val) {}

            Value *codegen() override { return nullptr; }

            virtual std::string getName() override { return ""; }
            std::string toString() override { return ""; }
            std::string getASTName() override { return "StringExprAST"; }
    };

    /// VariableExprAST - Expression class for referencing a variable, like "a".
    class VariableExprAST final : public ExprAST {
        private:
            std::string Name;


        public:
            VariableExprAST(const std::string& Name) : Name(Name) {}

            Value *codegen() override
            {
                // Look this variable up in the function.
                Value *V = NamedValues[Name];
                if (!V)
                {
                    JHIN_ASSERT_STR("variable does not exist");
                }

                // Load the value of this type.
                return Builder->CreateLoad(find_symbol(Name)->getType(), V, Name.c_str());
            }

            virtual std::string getName() override
            {
                return Name;
            }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "VariableExprAST"; }
    };

    /// BinaryExprAST - Expression class for a binary operator.
    class BinaryExprAST final : public ExprAST {
        private:
            // Op: == < <= > >= + - * /
            std::string Op;

            std::unique_ptr<ExprAST> LHS, RHS;

        public:
            BinaryExprAST(std::string Op, std::unique_ptr<ExprAST> LHS,
                                   std::unique_ptr<ExprAST> RHS)
                          : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

            Value *codegen() override
            {
                Value *L = LHS->codegen();
                Value *R = RHS->codegen();
                if (!L || !R)
                    return nullptr;

                if ("+" == Op)
                {
                    return Builder->CreateFAdd(L, R, "addtmp");
                }
                else if ("-" == Op)
                {
                    //return Builder->CreateFSub(L, R, "subtmp");
                }
                else if ("*" == Op)
                {
                    //return Builder->CreateFMul(L, R, "multmp");
                }
                else if ("/" == Op)
                {

                }
                else if ("==" == Op)
                {

                }
                else if ("<" == Op)
                {
                    //L = Builder->CreateFCmpULT(L, R, "cmptmp");
                    // Convert bool 0/1 to double 0.0 or 1.0
                    //return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext), "booltmp");

                }
                else if ("<=" == Op)
                {

                }
                else if (">" == Op)
                {

                }
                else if (">=" == Op)
                {

                }
                else
                {
                    return nullptr;
                }
                
                return nullptr;
            }
            
            virtual std::string getName() override { return ""; }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "BinaryExprAST"; }
    };

    /// CallExprAST - Expression class for function calls.
    class CallExprAST final : public ExprAST {
        private:
            std::string Callee;
            std::vector<std::unique_ptr<ExprAST>> Args;

        public:
            CallExprAST(const std::string& Callee,
                        std::vector<std::unique_ptr<ExprAST>> Args)
                        : Callee(Callee), Args(std::move(Args)) {}

            CallExprAST(const std::string& Callee)
                        : Callee(Callee) {}

            void addExp(std::unique_ptr<ExprAST> Arg)
            {
                Args.push_back(std::move(Arg));
            }

            Value *codegen() override { return nullptr; }
            virtual std::string getName() override { return ""; }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "CallExprAST"; }
    };

    /// CommandAST - Base class for all cmd nodes
    class CommandAST : public FormalAST
    {
        public:
            virtual Value *codegen() override = 0;
            virtual std::string getName() override = 0;

            std::string toString() override { return ""; }
            std::string getASTName() override { return "CommandAST"; }
    };

    /// IfCmdAST - Command class for if/then/else.
    class IfCmdAST final : public CommandAST
    {
        private:
            std::unique_ptr<ExprAST> Cond;
            std::unique_ptr<FormalsAST> Then, Else;

        public:
            IfCmdAST(std::unique_ptr<ExprAST> Cond,
                     std::unique_ptr<FormalsAST> Then,
                     std::unique_ptr<FormalsAST> Else)
                     : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}

            IfCmdAST(std::unique_ptr<ExprAST> Cond,
                     std::unique_ptr<FormalAST> FormalThen,
                     std::unique_ptr<FormalAST> FormalElse)
                     : Cond(std::move(Cond))
            {
                Then = std::make_unique<FormalsAST>(std::move(FormalThen));
                Else = std::make_unique<FormalsAST>(std::move(FormalElse));
            }

            Value *codegen() override { return nullptr; }
            virtual std::string getName() override { return ""; }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "IfCmdAST"; }
    };

    /// ForCmdAST2 - Another command class for for/in.
    class ForCmdAST2 final : public CommandAST
    {
        private:
            std::unique_ptr<ExprAST> Initial, Cond, Upt, Body;

        public:
            ForCmdAST2(std::unique_ptr<ExprAST> Initial,
                       std::unique_ptr<ExprAST> Cond,
                       std::unique_ptr<ExprAST> Upt,
                       std::unique_ptr<ExprAST> Body)
                       : Initial(std::move(Initial)), Cond(std::move(Cond)),
                       Upt(std::move(Upt)), Body(std::move(Body)) {}

            Value *codegen() override { return nullptr; }
            virtual std::string getName() override { return ""; }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "ForCmdAST2"; }
    };

    /// ForCmdAST - Command class for for/in.
    class ForCmdAST final : public CommandAST
    {
        private:
            std::string VarName;
            std::unique_ptr<ExprAST> Start, End, Step, Body;

        public:
            ForCmdAST(const std::string& VarName, std::unique_ptr<ExprAST> Start,
                       std::unique_ptr<ExprAST> End, std::unique_ptr<ExprAST> Step,
                       std::unique_ptr<ExprAST> Body)
                       : VarName(VarName), Start(std::move(Start)), End(std::move(End)),
                       Step(std::move(Step)), Body(std::move(Body)) {}

            Value *codegen() override { return nullptr; }
            virtual std::string getName() override { return ""; }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "ForCmdAST"; }
    };

    /// WhileCmdAST - Command class for while
    class WhileCmdAST final : public CommandAST
    {
        private:
            std::unique_ptr<ExprAST> Cond;
            std::unique_ptr<FormalsAST> Body;

        public:
            WhileCmdAST(std::unique_ptr<ExprAST> Cond,
                        std::unique_ptr<FormalsAST> Body)
                        : Cond(std::move(Cond)), Body(std::move(Body)) {}

            WhileCmdAST(std::unique_ptr<ExprAST> Cond,
                        std::unique_ptr<FormalAST> FormalBody)
                        : Cond(std::move(Cond))
            {
                Body = std::make_unique<FormalsAST>(std::move(FormalBody));
            }

            Value *codegen() override { return nullptr; }
            virtual std::string getName() override { return ""; }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "WhileCmdAST"; }
    };

    /// AssignCmdAST - Command class for assignment(<-)
    class AssignCmdAST final : public CommandAST
    {
        private:
            std::string name;
            std::unique_ptr<ExprAST> exp;

        public:
            AssignCmdAST(std::string name,
                         std::unique_ptr<ExprAST> exp)
                         : name(name), exp(std::move(exp)) {}

            Value *codegen() override { return nullptr; }
            virtual std::string getName() override { return ""; }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "AssignCmdAST"; }
    };

    /// DeclAST - Base class for all declarations nodes
    class DeclAST : public FormalAST
    {
        public:
            virtual Value *codegen() override = 0;
            virtual std::string getName() override = 0;

            std::string toString() override { return ""; }
            std::string getASTName() override { return "DeclAST"; }
    };

    enum ClassMemberType
    {
        MEMBER_TYPE_PRIVATE = 0,
        MEMBER_TYPE_PUBLIC,
    };

    struct ClassMemberItem
    {
        std::unique_ptr<DeclAST> decl;
        ClassMemberType visibility;
    };

    /// ProgUnitAST - including class and function AST which represents program unit
    class ProgUnitAST : public DeclAST
    {
        public:
            virtual Value *codegen() override = 0;
            virtual std::string getName() override = 0;

            std::string toString() override { return ""; }
            std::string getASTName() override { return "ProgUnitAST"; }
    };

    /// ProgUnitsAST - multiple ProgUnitAST
    class ProgUnitsAST : public DeclAST
    {
        private:
            std::vector<std::unique_ptr<ProgUnitAST>> ProgUnits;

        public:
            ProgUnitsAST(std::vector<std::unique_ptr<ProgUnitAST>> ProgUnits)
                         : ProgUnits(std::move(ProgUnits)) {}

            ProgUnitsAST() {}

            void addProgUnit(std::unique_ptr<ProgUnitAST> progUnit)
            {
                ProgUnits.push_back(std::move(progUnit));
            }

            Value *codegen() override
            {
                codegenProgUnits();
                return nullptr;
            }

            virtual std::string getName() override { return ""; }

            std::vector<Value*> codegenProgUnits()
            {
                std::vector<Value*> ans;
                for (const auto& p: ProgUnits)
                {
                    comm::Log::singleton(INFO) >> "testing codegenProgUnits" >> comm::newline;
                    ans.push_back(p->codegen());
                }
                return ans;
            }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "ProgUnitsAST"; }
    };

    /// ClassAST - class definition AST
    /// SymbolTable
    class ClassAST final : public ProgUnitAST
    {
        private:
            // class name
            std::string name;

            // inherit class name
            std::string inheritName;

            // declarations
            std::vector<ClassMemberItem> decls;

        public:
            Value *codegen() override { return nullptr; }
            virtual std::string getName() override
            {
                return name;
            }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "ClassAST"; }
    };

    /// DeclarationAST - single declaration node, regular declaration
    /// SymbolTable
    class DeclarationAST final : public DeclAST
    {
        private:
            std::string name;
            std::unique_ptr<TypeExprAST> type;
            std::unique_ptr<ExprAST> value;

        public:
            DeclarationAST(std::string name,
                           std::unique_ptr<TypeExprAST> type)
                           : name(name), type(std::move(type)) {}

            DeclarationAST(std::string name,
                           std::unique_ptr<TypeExprAST> type,
                           std::unique_ptr<ExprAST> value)
                           : name(name), type(std::move(type)), value(std::move(value)) {}

            virtual std::string getName() override
            {
                return name;
            }
            pTypeTree getpTT() { return type->getpTT(); }
            Type* getType() { return type->getType(); }
            ExprAST* getValue() {return value.get(); }

            Value *codegen() override { return nullptr; }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "DeclarationAST"; }
    };

    /// DeclarationsAST - multiple declarations nodes
    class DeclarationsAST: public DeclAST
    {
        private:
            std::vector<std::unique_ptr<DeclarationAST>> Decls;

        public:
            DeclarationsAST(std::vector<std::unique_ptr<DeclarationAST>> Decls)
                            : Decls(std::move(Decls)) {}

            DeclarationsAST(std::unique_ptr<DeclarationAST> declAST) {
                Decls.push_back(std::move(declAST));
            }

            DeclarationsAST()
            {
            }

            void addDecl(std::unique_ptr<DeclarationAST> declAST) {
                Decls.push_back(std::move(declAST));
            }

            Value *codegen() override { return nullptr; }

            virtual std::string getName() override { return ""; }

            std::vector<Value*> codegenDecl()
            {
                std::vector<Value*> ans;
                for (const auto& p: Decls)
                {
                    ans.push_back(p->codegen());
                }
                return ans;
            }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "DeclarationsAST"; }
    };

    /// PrototypeAST - This class represents the "prototype" for a function,
    /// which captures its name, and its argument names (thus implicitly the number
    /// of arguments the function takes).
    /// SymbolTable
    class PrototypeAST final : public ProgUnitAST
    {
        private:
            std::string Name;
            std::vector<std::unique_ptr<DeclarationAST>> Args;
            std::unique_ptr<TypeExprAST> RetType;

        public:
            PrototypeAST(const std::string& Name,
                         std::vector<std::unique_ptr<DeclarationAST>> Args,
                         std::unique_ptr<TypeExprAST> RetType)
                         : Name(Name), Args(std::move(Args)), RetType(std::move(RetType)) {}

            Value *codegen() override
            {
                codegenFunc();
                return nullptr;
            }

            virtual std::string getName() override
            {
                return Name;
            }

            // code gen for function
            Function *codegenFunc()
            {
                comm::Log::singleton(INFO) >> "testing codegenFunc Proto" >> comm::newline;
                // Make the function type
                std::vector<Type *> ArgsType;
                for (unsigned i = 0; i < Args.size(); i++)
                {
                    ArgsType.push_back(Args[i]->getType());
                }

                FunctionType *FT = FunctionType::get(RetType->getType(), ArgsType, false);

                // Set into Module
                Function *F = Function::Create(FT, Function::ExternalLinkage, Name, mdl::TheModule.get());

                // Set names for all arguments.
                unsigned Idx = 0;
                for (auto &Arg : F->args())
                    Arg.setName(Args[Idx++]->getName());

                return F;
            }

            const std::string &getName() const { return Name; }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "PrototypeAST"; }
    };

    /// FunctionAST - This class represents a function definition itself.
    /// SymbolTable
    class FunctionAST final : public ProgUnitAST {
        private:
            std::unique_ptr<PrototypeAST> Proto;
            std::unique_ptr<FormalsAST> Body;

        public:
            FunctionAST(std::unique_ptr<PrototypeAST> Proto,
                        std::unique_ptr<FormalsAST> Body)
                        : Proto(std::move(Proto)), Body(std::move(Body)) {}

            Value *codegen() override
            {
                codegenFunc();
                return nullptr;
            }

            virtual std::string getName() override
            {
                return Proto->name();
            }

            // code gen for function
            Function *codegenFunc()
            {
                comm::Log::singleton(INFO) >> "testing codegenFunc" >> comm::newline;
                Function *TheFunction = Proto->codegenFunc();
                if (!TheFunction)
                    return nullptr;

                // Create a new basic block to start insertion into.
                BasicBlock *BB = BasicBlock::Create(*mdl::TheContext, "entry", TheFunction);
                mdl::Builder->SetInsertPoint(BB);

                // Record the function arguments in the NamedValues map.
                mdl::NamedValues.clear();
                for (auto &Arg : TheFunction->args()) {
                    // Create an alloca for this variable.
                    AllocaInst *Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getType(), Arg.getName());

                    // Store the initial value into the alloca.
                    mdl::Builder->CreateStore(&Arg, Alloca);

                    // Add arguments to variable symbol table.
                    mdl::NamedValues[std::string(Arg.getName())] = Alloca;
                }

                if (Value *RetVal = Body->codegen()) {
                    // Finish off the function.
                    mdl::Builder->CreateRet(RetVal);

                    // Validate the generated code, checking for consistency.
                    verifyFunction(*TheFunction);

                    // Run the optimizer on the function.
                    mdl::TheFPM->run(*TheFunction);

                    return TheFunction;
                }

                // Error reading body, remove function.
                TheFunction->eraseFromParent();

                return nullptr;
            }

            std::string toString() override { return ""; }
            std::string getASTName() override { return "FunctionAST"; }

    };

}   /* namespace ast */
}   /* namespace jhin */
