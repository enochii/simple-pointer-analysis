#include "AndersonSolver.h"

#include <fstream>
  
AndersonPTG::AndersonPTG(unsigned n, vector<PointerAnalysisConstraints>& constraints)
    : graph(n) { initGraph(constraints); }

void AndersonPTG::solve() {
  while(!worklist.empty()) {
    NodeIdx idx = worklist.front();
    worklist.pop();
    auto& node = graph[idx];
    for(NodeIdx succ:node.getSuccessors()) propagate(succ, node.getPtsSet());
    
    for(NodeIdx pointee:node.getPtsSet()) {
      for(NodeIdx load:node.getLoads()) insertEdge(pointee, load);
      for(NodeIdx store:node.getStores()) insertEdge(store, pointee);
    }
  }
}

const vector<PointsToNode>& AndersonPTG::getGraph() const {
  return graph;
}
void AndersonPTG::initGraph(vector<PointerAnalysisConstraints>& constraints) {
  // llvm::errs() << "init points-to graph\n";
  for(auto& cons:constraints) {
    switch (cons.getTy())
    {
    case PointerAnalysisConstraints::Copy :
      graph[cons.getSrc()].addSuccessor(cons.getDest());
      break;
    case PointerAnalysisConstraints::Load :
      // <- *
      graph[cons.getSrc()].addLoad(cons.getDest());
      break;
    case PointerAnalysisConstraints::Store:
      // * <-
      graph[cons.getDest()].addStore(cons.getSrc());
      break;
    case PointerAnalysisConstraints::AddressOf: {
      /// init
      NodeIdx destIdx = cons.getDest();
      graph[destIdx].addPointee(cons.getSrc());
      /// add to WL
      worklist.push(destIdx);
      break;
    }
    default:
      break;
    }
  }
}

void AndersonPTG::insertEdge(NodeIdx src, NodeIdx dest) {
  auto& srcNode = graph[src];
  if(!srcNode.hasSuccessor(dest)) {
    srcNode.addSuccessor(dest);
    if(!srcNode.getPtsSet().empty()) worklist.push(src);
  }
}

void AndersonPTG::propagate(NodeIdx dest, const NodeSet& st) {
  bool changed = graph[dest].addPointee(st);
  if(changed) worklist.push(dest);
}
void AndersonPTG::propagate(NodeIdx dest, NodeIdx src) {
  bool changed = graph[dest].addPointee(src);
  if(changed) worklist.push(dest);
}


void AndersonPTG::dumpPtsSet(AndersonPass& andersonPass) {
  llvm::errs() << "---------------------------------\n";
  string dotStr = "digraph anderson_ptg {\n";
  for(unsigned i=0; i<graph.size(); i++) {
    if(graph[i].getPtsSet().empty()) continue;
    string ptr = andersonPass.idx2str(i);
    llvm::errs() << ptr << ": { ";
    for(NodeIdx idx:graph[i].getPtsSet()) {
      string target = andersonPass.idx2str(idx);
      llvm::errs() << target << ", ";
      dotStr += "\t\"" + ptr + "\" -> \"" + target + "\"\n";
    }
    llvm::errs() << " }\n";
  }
  dotStr += "}";
  ofstream dotFile("output/ptg.dot");
  dotFile << dotStr;
}

