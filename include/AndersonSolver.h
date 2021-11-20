#include "PointerAnalysis.h"
#include <queue>

using namespace std;
/*
  Constraints solving.

  graph defnition:
  - insertEdge
  - propagate points-to info
*/

typedef set<NodeIdx> NodeSet;
class PointsToNode;

class AndersonPTG {
  vector<PointsToNode> graph;
  // typedef pair<NodeIdx, NodeSet> WLItem;
  typedef NodeIdx WLItem;
  queue<WLItem> worklist;
public:
  AndersonPTG(unsigned n, vector<PAConstraint>& constraints);
  
  void solve();

  const vector<PointsToNode>& getGraph() const;
private:
  void initGraph(vector<PAConstraint>& constraints);
  void insertEdge(NodeIdx src, NodeIdx dest);
  void propagate(NodeIdx dest, const NodeSet& st);
  void propagate(NodeIdx dest, NodeIdx src);

public:
  void dumpGraph(PAPass& pass);
};

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