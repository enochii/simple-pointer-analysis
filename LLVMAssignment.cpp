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

#include <llvm/Support/CommandLine.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/ToolOutputFile.h>

#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils.h>

#include <llvm/IR/Function.h>
#include <llvm/Pass.h>
#include <llvm/Support/raw_ostream.h>

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Bitcode/BitcodeWriter.h>


#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CallSite.h"

#include <map>
#include <set>
#include <string>
#include <stack>

#include "NodeFactory.h"

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

cl::opt<bool> DumpModuleInfo("dump-module",
                                 cl::desc("Dump Module info into stderr"),
                                 cl::init(false), cl::Hidden);
// cl::opt<bool> DumpDebugInfo("dbg",
//                                  cl::desc("Dump debug info into out"),
//                                  cl::init(false), cl::Hidden);
cl::opt<bool> DumpTrace("trace",
                                 cl::desc("Dump trace into err"),
                                 cl::init(false), cl::Hidden);
cl::opt<bool> DumpInst("dump-inst",
                                 cl::desc("Dump instructions"),
                                 cl::init(false), cl::Hidden);

///!TODO TO BE COMPLETED BY YOU FOR ASSIGNMENT 2
///Updated 11/10/2017 by fargo: make all functions
///processed by mem2reg before this pass.
struct FuncPtrPass : public ModulePass {
  static char ID; // Pass identification, replacement for typeid
  FuncPtrPass() : ModulePass(ID) {}

  
  bool runOnModule(Module &M) override {
    if(DumpModuleInfo) {
      M.dump();
      errs()<<"------------------------------\n";
    }
    for(Function& f:M) {
      collectConstraintsForFunction(&f);
    }
    return false;
  }

  void collectConstraintsForFunction(const Function *f) {
    for (const_inst_iterator itr = inst_begin(f), ite = inst_end(f); itr != ite;
         ++itr) {
      auto inst = &*itr.getInstructionIterator();
      // TODO: create value node here
    }
    for (const_inst_iterator itr = inst_begin(f), ite = inst_end(f); itr != ite;
         ++itr) {
      auto inst = &*itr.getInstructionIterator();
      collectConstraintsForInstruction(inst);
    }
  }

  void collectConstraintsForInstruction(const Instruction* inst) {
    if(inst->getType()->isPointerTy()) return;

    if(DumpInst) inst->dump();
    switch (inst->getOpcode())
    {
    case Instruction::Load:
      break;
    
    case Instruction::Store:
      break;
    
    default:
      break;
    }
  }
};

char FuncPtrPass::ID = 0;
static RegisterPass<FuncPtrPass> X("my-anderson", "My Anderson implementation");

static cl::opt<std::string>
InputFilename(cl::Positional,
              cl::desc("<filename>.bc"),
              cl::init(""));


int main(int argc, char **argv) {
   LLVMContext &Context = getGlobalContext();
   SMDiagnostic Err;
   // Parse the command line to read the Inputfilename
   cl::ParseCommandLineOptions(argc, argv,
                              "FuncPtrPass \n My first LLVM too which does not do much.\n");


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
   Passes.add(new FuncPtrPass());
   Passes.run(*M.get());
}

