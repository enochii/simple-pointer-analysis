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
#include <vector>

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
cl::opt<bool> DumpDebugInfo("debug-info",
                                 cl::desc("Dump debug info into out"),
                                 cl::init(false), cl::Hidden);
cl::opt<bool> DumpTrace("trace",
                                 cl::desc("Dump trace into err"),
                                 cl::init(false), cl::Hidden);
cl::opt<bool> DumpInst("dump-inst",
                                 cl::desc("Dump instructions"),
                                 cl::init(false), cl::Hidden);

cl::opt<bool> DumpCons("dump-cons",
                                 cl::desc("Dump constraints"),
                                 cl::init(false), cl::Hidden);

cl::opt<bool> Node2Name("node2name",
                                 cl::desc("Dump node by index or name"),
                                 cl::init(false), cl::Hidden);
struct AndersonConstraint {
  enum ConstraintType {
    Copy, AddressOf, Load, Store,
  };
  AndersonConstraint(NodeIdx dest, NodeIdx src, ConstraintType type)
                    :dest(dest), src(src), type(type) {}
  NodeIdx getDest() { return dest; }
  NodeIdx getSrc() { return src; }
  ConstraintType getTy() { return type; }
private:
  NodeIdx dest, src;
  ConstraintType type;
};

///processed by mem2reg before this pass.
struct AndersonPass : public ModulePass {
  static char ID; // Pass identification, replacement for typeid
  AndersonPass() : ModulePass(ID) {}

  NodeFactory nodeFactory;
  vector<AndersonConstraint> constraints;

  bool runOnModule(Module &M) override {
    if(DumpModuleInfo) {
      M.dump();
      errs()<<"------------------------------\n";
    }
    collectConstraintsForGlobal(M);
    for(Function& f:M) {
      collectConstraintsForFunction(&f);
    }
    if(DumpCons) dumpConstraints();
    return false;
  }

private:
  /*
  ** currently only add return node for each function, as a context-insensitive
  ** approach to handle function call. This return node kind of serves as a 
  ** "core" node which (may) links multiple call sites & return sites
  */
  void collectConstraintsForGlobal(Module &M) {
    for(Function & f:M) {
      if(f.isIntrinsic() || f.isDeclaration()) continue;
      if(f.getType()->isPointerTy()) 
        nodeFactory.createRetNode(&f);
    }
  }
  void collectConstraintsForFunction(const Function *f) {
    for (const_inst_iterator itr = inst_begin(f), ite = inst_end(f); itr != ite;
         ++itr) {
      auto inst = &*itr.getInstructionIterator();
      nodeFactory.createValNode(inst);
    }
    for (const_inst_iterator itr = inst_begin(f), ite = inst_end(f); itr != ite;
         ++itr) {
      auto inst = &*itr.getInstructionIterator();
      collectConstraintsForInstruction(inst);
    }
  }

  void collectConstraintsForInstruction(const Instruction* inst) {
    if(DumpInst) inst->dump();
    switch (inst->getOpcode())
    {
      case Instruction::Alloca: {
        assert(inst->getType()->isPointerTy());
        NodeIdx src = nodeFactory.createObjNode(inst);
        NodeIdx dest = nodeFactory.getValNode(inst);
        constraints.emplace_back(dest, src, AndersonConstraint::AddressOf);
        break;
      }
      case Instruction::Load:
        if(inst->getType()->isPointerTy()) {
          NodeIdx dest = nodeFactory.getValNode(inst);
          NodeIdx src = nodeFactory.getValNode(inst->getOperand(0));
          constraints.emplace_back(dest, src, AndersonConstraint::Load);
        }
        break;
      
      case Instruction::Store:
        // type of store instruction is "void"
        if(inst->getOperand(0)->getType()->isPointerTy()) {
          NodeIdx src = nodeFactory.getValNode(inst->getOperand(0));
          NodeIdx dest = nodeFactory.getValNode(inst->getOperand(1));
          constraints.emplace_back(dest, src, AndersonConstraint::Store);
        }
        break;
      case Instruction::PHI: 
        if(inst->getType()->isPointerTy()) {
          const PHINode *phiNode = cast<PHINode>(inst);
          NodeIdx dest = nodeFactory.getValNode(inst);
          for(unsigned i=0; i<phiNode->getNumIncomingValues(); i++) {
            NodeIdx src = nodeFactory.getValNode(phiNode->getIncomingValue(i));
            constraints.emplace_back(dest, src, AndersonConstraint::Copy);
          }
        }
        break;
      case Instruction::Call:
      case Instruction::Invoke: {
        ImmutableCallSite cs(inst);
        assert(cs && "wrong callsite?");
        addConstraintsForCall(cs);
        break;
      }
        break;
      case Instruction::Ret: 
        if(inst->getOperand(0)->getType()->isPointerTy()) {
          NodeIdx dest = nodeFactory.getRetNode(inst->getParent()->getParent());
          NodeIdx src = nodeFactory.getValNode(inst->getOperand(0));
          constraints.emplace_back(dest, src, AndersonConstraint::Copy);
        }
        break;
        break;
      default:
        break;
    }
  }

  /// constraints: ret & call, parameter passing
  void addConstraintsForCall(ImmutableCallSite cs) {
    /// direct call
    if(const Function* f = cs.getCalledFunction()) {
      if(f->isIntrinsic() || f->isDeclaration()) {
        llvm::outs() << "external call: " << f->getName() << "\n";
        return;
      } else {
        // 
        NodeIdx dest = nodeFactory.getValNode(cs.getInstruction());
        NodeIdx src = nodeFactory.getRetNode(f);
        constraints.emplace_back(dest, src, AndersonConstraint::Copy);
      } 

    } else {
      // TODO
      assert("Not implemented yet");
    }
  }

  void addArgConstraints(ImmutableCallSite cs, const Function* f) {
    auto argIt = cs.arg_begin();
    auto parIt = f->arg_begin();

    while(argIt != cs.arg_end() && parIt != f->arg_end()) {
      const Value* arg = *argIt;
      const Value* par = &*parIt;
      NodeIdx dest = nodeFactory.getValNode(par);
      NodeIdx src = nodeFactory.getValNode(arg);
      constraints.emplace_back(dest, src, AndersonConstraint::Copy);
      argIt++; 
      parIt++;
    }
  }

  void dumpConstraints() {
    llvm::errs() << "Constraints " << constraints.size() << "\n";
    for(auto &item: constraints) {
      auto srcStr = idx2str(item.getSrc());
      auto destStr = idx2str(item.getDest());
      switch(item.getTy()) {
        case AndersonConstraint::AddressOf:
          llvm::errs() << destStr << " <- &" << srcStr << "\n";
          break;
        case AndersonConstraint::Copy:
          llvm::errs() << destStr << " <- " << srcStr << "\n";
          break;
        case AndersonConstraint::Load:
          llvm::errs() << destStr << " <- *" << srcStr << "\n";          
          break;
        case AndersonConstraint::Store:
          llvm::errs() << "*" << destStr << " <- " << srcStr << "\n";
          break; 
      }
    }
  }

  // code from https://github.com/jarulraj/llvm/ , the find name is too trivial...
  std::string getValueName (const Value *v) {
      // If we can get name directly
      if (v->getName().str().length() > 0) {
          return "%" + v->getName().str();
      } else if (isa<Instruction>(v)) {
          std::string s = "";
          raw_string_ostream *strm = new raw_string_ostream(s);
          v->print(*strm);
          std::string inst = strm->str();
          size_t idx1 = inst.find("%");
          size_t idx2 = inst.find(" ", idx1);
          if (idx1 != std::string::npos && idx2 != std::string::npos && idx1 == 2) {
              return inst.substr(idx1, idx2 - idx1);
          } else {
              // nothing match
              return "";
          }
      } else if (const ConstantInt *cint = dyn_cast<ConstantInt>(v)) {
          std::string s = "";
          raw_string_ostream *strm = new raw_string_ostream(s);
          cint->getValue().print(*strm, true);
          return strm->str();
      } else {
          std::string s = "";
          raw_string_ostream *strm = new raw_string_ostream(s);
          v->print(*strm);
          std::string inst = strm->str();
          return "\"" + inst + "\"";
      }
  }

  string idx2str(NodeIdx idx) {
    if(Node2Name) {
      auto value = nodeFactory.getValueByNodeIdx(idx);
      return value->getName();
      // return getValueName(value);
    } else return to_string(idx);
  }
};

char AndersonPass::ID = 0;
static RegisterPass<AndersonPass> X("my-anderson", "My Anderson implementation");

static cl::opt<std::string>
InputFilename(cl::Positional,
              cl::desc("<filename>.bc"),
              cl::init(""));


int main(int argc, char **argv) {
   LLVMContext &Context = getGlobalContext();
   SMDiagnostic Err;
   // Parse the command line to read the Inputfilename
   cl::ParseCommandLineOptions(argc, argv,
                              "AndersonPass \n My first LLVM too which does not do much.\n");


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
   Passes.add(new AndersonPass());
   Passes.run(*M.get());
}

