#pragma once

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
#include <llvm/IR/CallSite.h>

#include <vector>

#include "NodeFactory.h"

using namespace llvm;
using namespace std;

class PointsToNode;
struct PointerAnalysisConstraints {
  enum ConstraintType {
    Copy, AddressOf, Load, Store,
  };
  PointerAnalysisConstraints(NodeIdx dest, NodeIdx src, ConstraintType type)
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
  vector<PointerAnalysisConstraints> constraints;

  bool runOnModule(Module &M) override;
  string idx2str(NodeIdx idx, bool visualize=true);
private:
  /*
  ** currently only add return node for each function, as a context-insensitive
  ** approach to handle function call. This return node kind of serves as a 
  ** "core" node which (may) links multiple call sites & return sites
  */
  void collectConstraintsForGlobal(Module &M);
  void collectConstraintsForFunction(const Function *f);
  void collectConstraintsForInstruction(const Instruction* inst);
  /// constraints: ret & call, parameter passing
  void addConstraintsForCall(ImmutableCallSite cs);
  void addArgConstraints(ImmutableCallSite cs, const Function* f);

  void solveConstraints();

  /// dump
  void dumpConstraints();
  void dumpPtsSet(const vector<PointsToNode>& graph);
};
