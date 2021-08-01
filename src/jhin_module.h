#pragma once

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Support/raw_ostream.h"
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

static std::unique_ptr<LLVMContext> TheContext;
static std::unique_ptr<Module> TheModule;
static std::unique_ptr<IRBuilder<>> Builder;
static std::unique_ptr<legacy::FunctionPassManager> TheFPM;
static std::map<std::string, std::unique_ptr<ast::PrototypeAST>> FunctionProtos;
static ExitOnError ExitOnErr;

}   // namespace mdl
}   // namespace jhin
