#include "PointerAnalysis.h"
#include "UnionSet.h"

#include <map>
#include <set>

typedef set<NodeIdx> PointsToSet;

class SteensgardPTG {
  UnionSet uset;
  // pointer analysis as type inference
  map<USetIdx, PointsToSet> type;

  USetIdx createDeferenceNode(USetIdx deferencedIdx);
  void run(vector<PAConstraint> &constraints);
  USetIdx join(USetIdx x, USetIdx y);
  void joinPts(USetIdx ptr);
  void insert(USetIdx dest, USetIdx loc);
  void handleEqual(USetIdx x, USetIdx y);
public:
  SteensgardPTG(int locationCnt, vector<PAConstraint> &constraints):uset(locationCnt){
    run(constraints);
  }
  void solve();
  void dumpGraph(PAPass& pass);
};