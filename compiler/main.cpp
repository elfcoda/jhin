#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "../src/jhin_module.h"
#include "../src/client/compiler_client.h"

using namespace jhin;
using namespace jhin::mdl;
using namespace llvm;
using namespace llvm::orc;

// Data layout should be set once in main function
static void InitializeModuleAndPassManager()
{
    // Open a new module.
    TheContext = std::make_unique<LLVMContext>();
    TheModule = std::make_unique<Module>("jhin test", *TheContext);

    // Create a new builder for the module.
    Builder = std::make_unique<IRBuilder<>>(*TheContext);

    // Try to initialize FPM
    // Create a new pass manager attached to it.
    TheFPM = std::make_unique<legacy::FunctionPassManager>(TheModule.get());

    // Promote allocas to registers.
    TheFPM->add(createPromoteMemoryToRegisterPass());
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->add(createInstructionCombiningPass());
    // Reassociate expressions.
    TheFPM->add(createReassociatePass());
    // Eliminate Common SubExpressions.
    TheFPM->add(createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    TheFPM->add(createCFGSimplificationPass());

    TheFPM->doInitialization();
}

//===----------------------------------------------------------------------===//
// "Library" functions that can be "extern'd" from user code.
//===----------------------------------------------------------------------===//

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// putchard - putchar that takes a float and returns 0.
extern "C" DLLEXPORT float putchard(float X) {
  fputc((char)X, stderr);
  return 0;
}

/// printd - printf that takes a float prints it as "%f\n", returning 0.
extern "C" DLLEXPORT float printd(float X) {
  fprintf(stderr, "%f\n", X);
  return 0;
}


int main()
{
    srand((unsigned)time(NULL));

    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    InitializeModuleAndPassManager();

    compiler();

    auto TargetTriple = sys::getDefaultTargetTriple();
    TheModule->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        errs() << Error;
        return 1;
    }

    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    auto TheTargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);
    TheModule->setDataLayout(TheTargetMachine->createDataLayout());

    auto Filename = "output.o";
    std::error_code EC;
    raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);
    if (EC) {
        errs() << "Could not open file: " << EC.message();
        return 1;
    }

    legacy::PassManager pass;
    auto FileType = CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        errs() << "TheTargetMachine can't emit a file of this type";
        return 1;
    }

    pass.run(*TheModule);

    dest.flush();

    outs() << "Wrote " << Filename << "\n";

    // use clang to link to executable file
    std::string linkCmd = std::string("clang ") + Filename + std::string(" -o output");
    system(linkCmd.c_str());

    return 0;
}

