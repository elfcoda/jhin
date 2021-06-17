#pragma once

namespace jhin
{
namespace semnode
{

    using namespace llvm;

    /// ExprAST - Base class for all expression nodes
    class ExprAST
    {
        public:
            virtual ~ExprAST() = default;

            // virtual Value *codegen() = 0;
    };

}   /* namespace semnode */
}   /* namespace jhin */
