#pragma once

#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include <string>
#include <memory>
#include <vector>


using namespace llvm;

namespace jhin
{
namespace semnode
{

    //===----------------------------------------------------------------------===//
    // Abstract Syntax Tree (aka Parse Tree)
    //===----------------------------------------------------------------------===//

    class ASTBase
    {
        public:
            virtual bool IsEmpty() { return false; }

            virtual ~ASTBase() = default;
    };

    /// EmptyAST - for EPSILON and other empty case
    class EmptyAST: public ASTBase
    {
        public:
            bool IsEmpty() override { return true; }
    };

    class FormalAST : public ASTBase
    {
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

            void addFormal(std::unique_ptr<FormalAST> FormalNode)
            {
                Formals.push_back(std::move(FormalNode));
            }
    };

    /// ExprAST - Base class for all expression nodes
    class ExprAST: public FormalAST
    {
        public:
            virtual Value *codegen() = 0;
    };

    /// TypeExprAST - type for all type expression nodes
    class TypeExprAST : public ExprAST {
        private:
            std::string typeName;

        public:
            TypeExprAST(std::string typeName): typeName(typeName) {};
            Value *codegen() override;
    };

    /// BoolExprAST - Expression class for Bool literals like "True".
    class BoolExprAST : public ExprAST {
        private:
            bool Val;

        public:
            BoolExprAST(bool Val) : Val(Val) {}
            void inverse() {
                Val = !Val;
            }

            Value *codegen() override;
    };

    /// IntExprAST - Expression class for Int literals like "1".
    class IntExprAST : public ExprAST {
        private:
            int Val;

        public:
            IntExprAST(int Val) : Val(Val) {}

            Value *codegen() override;
    };

    /// FloatExprAST - Expression class for float literals like "1.0".
    class FloatExprAST : public ExprAST {
        private:
            float Val;

        public:
            FloatExprAST(float Val) : Val(Val) {}

            Value *codegen() override;
    };

    /// StringExprAST - Expression class for String literals like "str".
    class StringExprAST : public ExprAST {
        private:
            std::string Val;

        public:
            StringExprAST(std::string Val) : Val(Val) {}

            Value *codegen() override;
    };

    /// VariableExprAST - Expression class for referencing a variable, like "a".
    class VariableExprAST : public ExprAST {
        private:
            std::string Name;

        public:
            VariableExprAST(const std::string& Name) : Name(Name) {}

            Value *codegen() override;
    };

    /// BinaryExprAST - Expression class for a binary operator.
    class BinaryExprAST : public ExprAST {
        private:
            // Op: == < <= > >= + - * /
            std::string Op;

            std::unique_ptr<ExprAST> LHS, RHS;

        public:
            BinaryExprAST(std::string Op, std::unique_ptr<ExprAST> LHS,
                                   std::unique_ptr<ExprAST> RHS)
                          : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

            Value *codegen() override;
    };

    /// CallExprAST - Expression class for function calls.
    class CallExprAST : public ExprAST {
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

            Value *codegen() override;
    };

    /// CommandAST - Base class for all cmd nodes
    class CommandAST : public FormalAST
    {
        public:
            virtual Value *codegen() = 0;
    };

    /// IfCmdAST - Command class for if/then/else.
    class IfCmdAST : public CommandAST
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

            Value *codegen() override;
    };

    /// ForCmdAST2 - Another command class for for/in.
    class ForCmdAST2 : public CommandAST
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

            Value *codegen() override;
    };

    /// ForCmdAST - Command class for for/in.
    class ForCmdAST : public CommandAST
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

            Value *codegen() override;
    };

    /// WhileCmdAST - Command class for while
    class WhileCmdAST : public CommandAST
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

            Value *codegen() override;
    };

    /// AssignCmdAST - Command class for assignment(<-)
    class AssignCmdAST : public CommandAST
    {
        private:
            std::string name;
            std::unique_ptr<ExprAST> exp;

        public:
            AssignCmdAST(std::string name,
                         std::unique_ptr<ExprAST> exp)
                         : name(name), exp(std::move(exp)) {}

            Value *codegen() override;
    };

    /// DeclAST - Base class for all declarations nodes
    class DeclAST : public FormalAST
    {
        public:
            virtual Value *codegen() = 0;
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
            Value *codegen() override;
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

            Value *codegen() override;
    };

    /// ClassAST - class definition AST
    class ClassAST : public ProgUnitAST
    {
        private:
            // class name
            std::string name;

            // inherit class name
            std::string inheritName;

            // declarations
            std::vector<ClassMemberItem> decls;

        public:
            Value *codegen() override;
    };

    /// DeclarationAST - single declaration node, regular declaration
    class DeclarationAST: public DeclAST
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

            Value *codegen() override;
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

            Value *codegen() override;
    };

    /// PrototypeAST - This class represents the "prototype" for a function,
    /// which captures its name, and its argument names (thus implicitly the number
    /// of arguments the function takes).
    class PrototypeAST : public ProgUnitAST
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

            Value *codegen() override;

            // code gen for function
            Function *codegenFunc ();
            const std::string &getName() const { return Name; }
    };

    /// FunctionAST - This class represents a function definition itself.
    class FunctionAST : public ProgUnitAST {
        private:
            std::unique_ptr<PrototypeAST> Proto;
            std::unique_ptr<FormalsAST> Body;

        public:
            FunctionAST(std::unique_ptr<PrototypeAST> Proto,
                        std::unique_ptr<FormalsAST> Body)
                        : Proto(std::move(Proto)), Body(std::move(Body)) {}

            Value *codegen() override;

            // code gen for function
            Function *codegenFunc();
    };

}   /* namespace semnode */
}   /* namespace jhin */
