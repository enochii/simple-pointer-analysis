//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "PointerAnalysis.h"

#include <string>

using namespace std;
using namespace llvm;
static ManagedStatic<LLVMContext> GlobalContext;
static LLVMContext &getGlobalContext() { return *GlobalContext; }
/* In LLVM 5.0, when  -O0 passed to clang , the functions generated with clang will
 * have optnone attribute which would lead to some transform passes disabled, like mem2reg.
 */
struct EnableFunctionOptPass: public FunctionPass {
    static char ID;
    EnableFunctionOptPass():FunctionPass(ID){}
    bool runOnFunction(Function & F) override{
        if(F.hasFnAttribute(Attribute::OptimizeNone))
        {
            F.removeFnAttr(Attribute::OptimizeNone);
        }
        return true;
    }
};

char EnableFunctionOptPass::ID=0;

cl::opt<bool> DumpCons("dump-cons",
                                 cl::desc("Dump constraints"),
                                 cl::init(false), cl::Hidden);
cl::opt<bool> DumpModuleInfo("dump-module",
                                 cl::desc("Dump Module info into stderr"),
                                 cl::init(false), cl::Hidden);

bool PAPass::runOnModule(Module &M) {
  if(DumpModuleInfo) {
    M.dump();
    errs()<<"------------------------------\n";
  }
  collectConstraintsForGlobal(M);
  for(Function& f:M) {
    collectConstraintsForFunction(&f);
  }
  if(DumpCons) dumpConstraints();
  /// solve
  solveConstraints();
  return false;
}


char PAPass::ID = 0;
static RegisterPass<PAPass> X("my-anderson", "My Anderson implementation");

static cl::opt<std::string>
InputFilename(cl::Positional,
              cl::desc("<filename>.bc"),
              cl::init(""));


int main(int argc, char **argv) {
   LLVMContext &Context = getGlobalContext();
   SMDiagnostic Err;
   // Parse the command line to read the Inputfilename
   cl::ParseCommandLineOptions(argc, argv,
                              "PAPass \n My first LLVM too which does not do much.\n");


   // Load the input module
   std::unique_ptr<Module> M = parseIRFile(InputFilename, Err, Context);
   if (!M) {
      Err.print(argv[0], errs());
      return 1;
   }

   llvm::legacy::PassManager Passes;
   	
   ///Remove functions' optnone attribute in LLVM5.0
   Passes.add(new EnableFunctionOptPass());
   ///Transform it to SSA
   Passes.add(llvm::createPromoteMemoryToRegisterPass());

   /// Your pass to print Function and Call Instructions
   Passes.add(new PAPass());
   Passes.run(*M.get());
}

