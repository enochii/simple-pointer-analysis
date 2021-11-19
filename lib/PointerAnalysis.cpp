#include "PointerAnalysis.h"
#include "AndersonSolver.h"

void AndersonPass::solveConstraints() {
  AndersonPTG ptg(nodeFactory.getNumNode(), constraints);
  ptg.solve();
  ptg.dumpPtsSet(*this);
}
