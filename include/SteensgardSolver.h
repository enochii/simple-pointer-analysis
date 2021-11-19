#include "PointerAnalysis.h"
#include "UnionSet.h"

#include <map>
#include <set>

class PointsToSet {
  set<NodeIdx> st;

public:
  void merge(const PointsToSet& src) {
    st.insert(src.st.begin(), src.st.end());
  }
};

class SteensgradPTG {
  UnionSet uset;
  // pointer analysis as type inference
  map<USetIdx, PointsToSet> type;

  USetIdx createDeferenceNode(USetIdx deferencedIdx);
public:
  SteensgradPTG(int locationCnt, vector<PointerAnalysisConstraint> &constraints);
  void solve();
  void dumpGraph(PAPass& pass);
};