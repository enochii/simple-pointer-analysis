#include "PointerAnalysis.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"


cl::opt<bool> DumpDebugInfo("debug-info",
                                 cl::desc("Dump debug info into out"),
                                 cl::init(false), cl::Hidden);
cl::opt<bool> DumpTrace("trace",
                                 cl::desc("Dump trace into err"),
                                 cl::init(false), cl::Hidden);
cl::opt<bool> DumpInst("dump-inst",
                                 cl::desc("Dump instructions"),
                                 cl::init(false), cl::Hidden);
cl::opt<bool> Node2Name("node2name",
                                 cl::desc("Dump node by index or name"),
                                 cl::init(false), cl::Hidden);

void AndersonPass::collectConstraintsForGlobal(Module &M) {
  for(Function & f:M) {
    if(f.isIntrinsic() || f.isDeclaration()) continue;
    if(f.getType()->isPointerTy()) 
      nodeFactory.createRetNode(&f);
  }
}
void AndersonPass::collectConstraintsForFunction(const Function *f) {
  for (const_inst_iterator itr = inst_begin(f), ite = inst_end(f); itr != ite;
        ++itr) {
    auto inst = &*itr.getInstructionIterator();
    if(inst->getType()->isPointerTy())
      nodeFactory.createValNode(inst);
  }
  for (const_inst_iterator itr = inst_begin(f), ite = inst_end(f); itr != ite;
        ++itr) {
    auto inst = &*itr.getInstructionIterator();
    collectConstraintsForInstruction(inst);
  }
}

void AndersonPass::collectConstraintsForInstruction(const Instruction* inst) {
  if(DumpInst) inst->dump();
  switch (inst->getOpcode())
  {
    case Instruction::Alloca: {
      assert(inst->getType()->isPointerTy());
      NodeIdx src = nodeFactory.createObjNode(inst);
      NodeIdx dest = nodeFactory.getValNode(inst);
      constraints.emplace_back(dest, src, PointerAnalysisConstraints::AddressOf);
      break;
    }
    case Instruction::Load:
      if(inst->getType()->isPointerTy()) {
        NodeIdx dest = nodeFactory.getValNode(inst);
        NodeIdx src = nodeFactory.getValNode(inst->getOperand(0));
        constraints.emplace_back(dest, src, PointerAnalysisConstraints::Load);
      }
      break;
    
    case Instruction::Store:
      // type of store instruction is "void"
      if(inst->getOperand(0)->getType()->isPointerTy()) {
        NodeIdx src = nodeFactory.getValNode(inst->getOperand(0));
        NodeIdx dest = nodeFactory.getValNode(inst->getOperand(1));
        constraints.emplace_back(dest, src, PointerAnalysisConstraints::Store);
      }
      break;
    case Instruction::PHI: 
      if(inst->getType()->isPointerTy()) {
        const PHINode *phiNode = cast<PHINode>(inst);
        NodeIdx dest = nodeFactory.getValNode(inst);
        for(unsigned i=0; i<phiNode->getNumIncomingValues(); i++) {
          NodeIdx src = nodeFactory.getValNode(phiNode->getIncomingValue(i));
          constraints.emplace_back(dest, src, PointerAnalysisConstraints::Copy);
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
        constraints.emplace_back(dest, src, PointerAnalysisConstraints::Copy);
      }
      break;
    case Instruction::GetElementPtr: {
      /// field-insensitive
      NodeIdx dest = nodeFactory.getValNode(inst);
      NodeIdx src = nodeFactory.getValNode(inst->getOperand(0));
      constraints.emplace_back(dest, src, PointerAnalysisConstraints::Copy);
    }      
    default:
      break;
  }
}

/// constraints: ret & call, parameter passing
void AndersonPass::addConstraintsForCall(ImmutableCallSite cs) {
  /// direct call
  if(const Function* f = cs.getCalledFunction()) {
    if(f->isIntrinsic() || f->isDeclaration()) {
      if(DumpDebugInfo) llvm::outs() << "external call: " << f->getName() << "\n";
      return;
    } else {
      // 
      NodeIdx dest = nodeFactory.getValNode(cs.getInstruction());
      NodeIdx src = nodeFactory.getRetNode(f);
      constraints.emplace_back(dest, src, PointerAnalysisConstraints::Copy);
      addArgConstraints(cs, f);
    } 

  } else {
    // TODO
    assert("Not implemented yet");
  }
}

void AndersonPass::addArgConstraints(ImmutableCallSite cs, const Function* f) {
  auto argIt = cs.arg_begin();
  auto parIt = f->arg_begin();

  while(argIt != cs.arg_end() && parIt != f->arg_end()) {
    const Value* arg = *argIt;
    const Value* par = &*parIt;
    if(arg->getType()->isPointerTy() && par->getType()->isPointerTy()) {
      NodeIdx dest = nodeFactory.getValNode(par);
      NodeIdx src = nodeFactory.getValNode(arg);
      constraints.emplace_back(dest, src, PointerAnalysisConstraints::Copy);
    }
    argIt++; 
    parIt++;
  }
}

void AndersonPass::dumpConstraints() {
  llvm::errs() << "Constraints " << constraints.size() << "\n";
  for(auto &item: constraints) {
    auto srcStr = idx2str(item.getSrc());
    auto destStr = idx2str(item.getDest());
    switch(item.getTy()) {
      case PointerAnalysisConstraints::AddressOf:
        llvm::errs() << destStr << " <- &" << srcStr << "\n";
        break;
      case PointerAnalysisConstraints::Copy:
        llvm::errs() << destStr << " <- " << srcStr << "\n";
        break;
      case PointerAnalysisConstraints::Load:
        llvm::errs() << destStr << " <- *" << srcStr << "\n";          
        break;
      case PointerAnalysisConstraints::Store:
        llvm::errs() << "*" << destStr << " <- " << srcStr << "\n";
        break; 
    }
  }
}


// code from https://github.com/jarulraj/llvm/ , the find name is too trivial...
static std::string getValueName (const Value *v) {
  // If we can get name directly
  if (v->getName().str().length() > 0) {
      return v->getName().str();
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
string AndersonPass::idx2str(NodeIdx idx, bool visualize) {
  if(Node2Name || visualize) {
    string suffix = nodeFactory.isValueNode(idx)? "":"(obj)";
    auto value = nodeFactory.getValueByNodeIdx(idx);
    // return string(value->getName());
    return getValueName(value) + suffix;
  } else return to_string(idx);
}