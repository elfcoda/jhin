#pragma once

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "jhin_JIT.h"

namespace jhin
{

namespace ast
{
    class PrototypeAST;
}

namespace mdl
{

using namespace llvm;
using namespace llvm::orc;


struct CodeGenCollect
{
    private:
        CodeGenCollect() {}
        static CodeGenCollect* pInstance;
    public:
        static CodeGenCollect* singleton()
        {
            if (pInstance == nullptr)
            {
                pInstance = new CodeGenCollect();
            }
            return pInstance;
        }
        static std::unique_ptr<LLVMContext> TheContext;
        static std::unique_ptr<Module> TheModule;
        static std::unique_ptr<IRBuilder<>> Builder;
        static std::map<std::string, AllocaInst *> NamedValues;
        static std::unique_ptr<legacy::FunctionPassManager> TheFPM;
        static std::unique_ptr<jit::JhinJIT> TheJIT;
        static std::map<std::string, std::unique_ptr<ast::PrototypeAST>> FunctionProtos;
        static ExitOnError ExitOnErr;
};
CodeGenCollect* CodeGenCollect::pInstance = nullptr;
std::unique_ptr<LLVMContext> CodeGenCollect::TheContext = nullptr;
std::unique_ptr<Module> CodeGenCollect::TheModule = nullptr;
std::unique_ptr<IRBuilder<>> CodeGenCollect::Builder = nullptr;
std::map<std::string, AllocaInst *> CodeGenCollect::NamedValues = {};
std::unique_ptr<legacy::FunctionPassManager> CodeGenCollect::TheFPM = nullptr;
std::unique_ptr<jit::JhinJIT> CodeGenCollect::TheJIT = nullptr;
std::map<std::string, std::unique_ptr<ast::PrototypeAST>> CodeGenCollect::FunctionProtos = {};
ExitOnError CodeGenCollect::ExitOnErr;

static void InitializeModuleAndPassManager()
{
    // Open a new module.
    CodeGenCollect::TheContext = std::make_unique<LLVMContext>();
    CodeGenCollect::TheModule = std::make_unique<Module>("my cool jit", *CodeGenCollect::TheContext);
    CodeGenCollect::TheModule->setDataLayout(CodeGenCollect::TheJIT->getDataLayout());

    // Create a new builder for the module.
    CodeGenCollect::Builder = std::make_unique<IRBuilder<>>(*CodeGenCollect::TheContext);

    // Create a new pass manager attached to it.
    CodeGenCollect::TheFPM = std::make_unique<legacy::FunctionPassManager>(CodeGenCollect::TheModule.get());

    // Promote allocas to registers.
    CodeGenCollect::TheFPM->add(createPromoteMemoryToRegisterPass());
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    CodeGenCollect::TheFPM->add(createInstructionCombiningPass());
    // Reassociate expressions.
    CodeGenCollect::TheFPM->add(createReassociatePass());
    // Eliminate Common SubExpressions.
    CodeGenCollect::TheFPM->add(createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    CodeGenCollect::TheFPM->add(createCFGSimplificationPass());

    CodeGenCollect::TheFPM->doInitialization();
}

}   // namespace mdl
}   // namespace jhin
