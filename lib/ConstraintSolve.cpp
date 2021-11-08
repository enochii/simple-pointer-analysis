#include "Anderson.h"

#include <queue>
/*
  Constraints solving.

  graph defnition:
  - insertEdge
  - propagate points-to info
*/

typedef set<NodeIdx> NodeSet;
/// dump node to name, we may need this
class PointsToNode {
  /// when this node's pts-set changes, we needd to propagate changes to
  /// other nodes through "copy", "load" & "store"
  /// copy is "static", while the other 2 are "dynamic"
  NodeSet successors;
  /// load & store are both indirect!
  NodeSet loadTo; // other <- *me
  NodeSet storeFrom; // *me <- other
  // NodeIdx idx; /// ? do we need this

  NodeSet ptsSet;
public:
  void addSuccessor(NodeIdx succ) { successors.insert(succ); }
  void addLoad(NodeIdx dest) { loadTo.insert(dest); }
  void addStore(NodeIdx src) { storeFrom.insert(src); }

  bool addPointee(NodeIdx pte) { 
    if(ptsSet.count(pte)) return false;
    ptsSet.insert(pte);
    return true;
  }
  bool addPointee(const NodeSet& src) { 
    bool changed = false;
    for(NodeIdx idx:src) {
      if(!ptsSet.count(idx)) {
        changed = true;
        break;
      }
    }
    if(changed) ptsSet.insert(src.begin(), src.end());
    return changed;
  }

  /// getter
  const NodeSet& getSuccessors()const { return successors; }
  const NodeSet& getLoads()const { return loadTo; }
  const NodeSet& getStores()const { return storeFrom; }
  const NodeSet& getPtsSet()const { return ptsSet; }
  // const NodeIdx getIdx()const { return idx; }

  bool hasSuccessor(NodeIdx succ) { return successors.count(succ); }
};

class PointstoGraph {
  vector<PointsToNode> graph;
  // typedef pair<NodeIdx, NodeSet> WLItem;
  typedef NodeIdx WLItem;
  queue<WLItem> worklist;
public:
  PointstoGraph(unsigned n, vector<AndersonConstraint>& constraints)
    : graph(n) { initGraph(constraints); }
  
  void solve() {
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

  const vector<PointsToNode>& getGraph() const {
    return graph;
  }
private:
  void initGraph(vector<AndersonConstraint>& constraints) {
    // llvm::errs() << "init points-to graph\n";
    for(auto& cons:constraints) {
      switch (cons.getTy())
      {
      case AndersonConstraint::Copy :
        graph[cons.getSrc()].addSuccessor(cons.getDest());
        break;
      case AndersonConstraint::Load :
        // <- *
        graph[cons.getSrc()].addLoad(cons.getDest());
        break;
      case AndersonConstraint::Store:
        // * <-
        graph[cons.getDest()].addStore(cons.getSrc());
        break;
      case AndersonConstraint::AddressOf: {
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

  void insertEdge(NodeIdx src, NodeIdx dest) {
    auto& srcNode = graph[src];
    if(!srcNode.hasSuccessor(dest)) {
      srcNode.addSuccessor(dest);
      if(!srcNode.getPtsSet().empty()) worklist.push(src);
    }
  }

  void propagate(NodeIdx dest, const NodeSet& st) {
    bool changed = graph[dest].addPointee(st);
    if(changed) worklist.push(dest);
  }
  void propagate(NodeIdx dest, NodeIdx src) {
    bool changed = graph[dest].addPointee(src);
    if(changed) worklist.push(dest);
  }
};

void AndersonPass::solveConstraints() {
  PointstoGraph ptg(nodeFactory.getNumNode(), constraints);
  ptg.solve();
  dumpPtsSet(ptg.getGraph());
}

void AndersonPass::dumpPtsSet(const vector<PointsToNode>& graph) {
  llvm::errs() << "---------------------------------\n";
  for(unsigned i=0; i<graph.size(); i++) {
      llvm::errs() << idx2str(i) << ": { ";
      for(NodeIdx idx:graph[i].getPtsSet())
        llvm::errs() << idx2str(idx) << ", ";
      llvm::errs() << " }\n";
    }
}