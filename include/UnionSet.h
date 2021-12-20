#include <vector>

using namespace std;

/// Union set with dynamic size(only increasing)
typedef unsigned USetIdx;
class UnionSet {
private:
  /// when parent[i] < 0, means it is a root
  /// and its tree size == -parent[i]
  vector<int> parent;
public:
  USetIdx createNode() {
    USetIdx index = parent.size();
    parent.push_back(-1);
    return index;
  }

  UnionSet(int sz=0) {
    for(int i=0; i<sz; i++) 
      parent.push_back(-1);
  }

  USetIdx find(USetIdx from) {
    assert(from >= 0);
    int par = parent[from];
    if(par < 0) return from;
    return parent[from] = find(par);
  }

  int merge(int x, int y) {
    USetIdx px = find(x);
    USetIdx py = find(y);

    // subtree with px as root is bigger
    if(parent[px] < parent[py]) {
      parent[px] += parent[py];
      parent[py] = px;
      return px;
    } else {
      parent[py] += parent[px];
      parent[px] = py;
      return py;
    }
  }

  map<USetIdx, set<USetIdx>> getClasses() {
    map<USetIdx, set<USetIdx>> ret;
    for(USetIdx i=0; i<parent.size(); i++) {
      auto pi = find(i);
      ret[pi].insert(i);
    }
    return ret;
  }

  void dumpClasses() {
    auto cls = getClasses();
    for(auto kv:cls) {
      llvm::errs() << kv.first << ": {";
      for(auto c:kv.second) llvm::errs() << c << ",";
      llvm::errs() << "}\n";
    }
  }
};