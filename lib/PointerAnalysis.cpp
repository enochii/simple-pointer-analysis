#include "PointerAnalysis.h"
#include "AndersonSolver.h"

void PAPass::solveConstraints() {
  AndersonPTG ptg(nodeFactory.getNumNode(), constraints);
  ptg.solve();
  ptg.dumpPtsSet(*this);
}
