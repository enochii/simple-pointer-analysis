#include "PointerAnalysis.h"
#include "AndersonSolver.h"
#include "SteensgardSolver.h"

void PAPass::solveConstraints() {
  // AndersonPTG ptg(nodeFactory.getNumNode(), constraints);
  SteensgardPTG ptg(nodeFactory.getNumNode(), constraints);

  ptg.solve();
  ptg.dumpGraph(*this);
}
