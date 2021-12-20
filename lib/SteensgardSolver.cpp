
#include <map>
#include <set>
#include <fstream>
#include "SteensgardSolver.h"


static void merge(PointsToSet& dest, const PointsToSet& src) {
  dest.insert(src.begin(), src.end());
}

USetIdx SteensgardPTG::createDeferenceNode(USetIdx deferencedIdx) {
  return 0;
}

void SteensgardPTG::joinPts(USetIdx ptr) {
  auto &pts = type[ptr];
  if(pts.size() < 2) return;
  auto it = pts.begin();
  auto x = *it;
  auto px = uset.find(x);
  for(++it; it!= pts.end(); ++it) {
    auto py = uset.find(*it);
    px = join(px, py);
  }
  pts.clear();
  pts.insert(px);
}

void SteensgardPTG::handleEqual(USetIdx x, USetIdx y) {
  x = uset.find(x);
  y = uset.find(y);
  if(x == y) return;
  auto&px = type[x];
  auto&py = type[y];
  assert(px.size() <= 1 && py.size() == 1);
  merge(px, py);
  type.erase(y);
  joinPts(x);
  type[y] = type[x];
}


USetIdx SteensgardPTG::join(USetIdx x, USetIdx y) {
  x = uset.find(x);
  y = uset.find(y);
  if(x == y) return x;
  USetIdx ret = uset.merge(x, y);
  if(ret != x && type.count(x)) {
    merge(type[ret], type[x]);
    type.erase(x);
  }
  if(ret != y && type.count(y)) {
    merge(type[ret], type[y]);
    type.erase(y);
  }
  joinPts(ret);
  return ret;
}

void SteensgardPTG::insert(USetIdx dest, USetIdx loc) {
  type[dest].insert(loc);
  joinPts(dest);
}

void SteensgardPTG::run(vector<PAConstraint> &constraints) {
  
  for(auto &cons:constraints) {
    if(PAConstraint::AddressOf == cons.getTy()) {
      /// dest = &src
      // llvm::errs() << cons.getDest() << " <- &" << cons.getSrc() << "\n";
      insert(cons.getDest(), cons.getSrc());
    } else if(PAConstraint::Copy == cons.getTy()) {
      /// dest = src
      // llvm::errs() << cons.getDest() << " <- " << cons.getSrc() << "\n";
      handleEqual(cons.getDest(), cons.getSrc());
    }
  }

  for(auto &cons:constraints) {
    switch (cons.getTy()) {
    case PAConstraint::Load: {
      /// dest = *src
      auto srcPar = uset.find(cons.getSrc());
      auto &pstSrc = type[srcPar];
      assert(pstSrc.size() == 1 && "pts.size() should be 1");
      handleEqual(cons.getDest(), *pstSrc.begin());
      break;
    }
    case PAConstraint::Store: {
        /// *dest = src
        // llvm::errs() << "*" << cons.getDest() << " <- " << cons.getSrc() << "\n";
        auto destPar = uset.find(cons.getDest());
        auto &pstDest = type[destPar];
        assert(pstDest.size() == 1 && "pts.size() should be 1");
        handleEqual(*pstDest.begin(), cons.getSrc());
        break;
      }
    default:
      break;
    }
  }
}

void SteensgardPTG::solve() {

}

static string set2str(const set<USetIdx> & pst, PAPass& pass) {
  string s = "{";
  for(auto x:pst) s += pass.idx2str(x) + ", ";
  s += "}";
  return s;
}

static string tabAndNewLine(string s) {
  return "\t" + s + ";\n";
} 

static string quote(string s) {
  return "\"" + s + "\"";
}

void SteensgardPTG::dumpGraph(PAPass& pass) {
  // uset.dumpClasses();
  auto unionClass = uset.getClasses();
  ofstream dotFile("output/ptg.dot");
  dotFile << "digraph unification_ptg {\n";
  dotFile << tabAndNewLine("graph [label=\"Steensgard Pointer Analysis\",labelloc=t,fontsize=30]");
	dotFile << tabAndNewLine("node [color=blue]");

  for(const auto& kv:unionClass) {
    assert(kv.first == uset.find(kv.first));
    auto &objRoot = type[kv.first];
    if(objRoot.empty()) continue;
    assert(objRoot.size() == 1);
    auto &objSet = unionClass[uset.find(*objRoot.begin())];
    auto ptr = set2str(kv.second, pass);
    auto obj = set2str(objSet, pass);
    dotFile << tabAndNewLine(quote(ptr) + " -> " + quote(obj));
  }
  dotFile << "}";
}