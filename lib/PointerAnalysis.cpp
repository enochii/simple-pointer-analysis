#include "PointerAnalysis.h"
#include "AndersonSolver.h"
#include "SteensgardSolver.h"

static cl::list<string>  Argv(cl::ConsumeAfter, cl::desc("<program arguments>..."));

enum PA_TYPE {
  INCLUSION, 
  UNIFICATION,
};
static string STR_ANDERSON = "ander";
static string STR_STEENSGARD = "steen";
static string STR_UNIFICATION = "unification";
static string STR_INCLUSION = "inclusion";

void PAPass::solveConstraints() {
  PA_TYPE type = INCLUSION;
  for(auto it=Argv.begin(); it!=Argv.end(); it++) {
    if(*it != "-algo") continue;
    ++it;
    assert(it != Argv.end());
    // llvm::errs() << "algo: " << *it << "\n";
    if(STR_ANDERSON == *it || STR_INCLUSION == *it) type = PA_TYPE::INCLUSION;
    else if(STR_STEENSGARD == *it || STR_UNIFICATION == *it) type = PA_TYPE::UNIFICATION;
    else {
      llvm::errs() << "Invalid parameter: -algo " << *it << "\n";
    }
    break;
  }
  if(type == INCLUSION) {
    AndersonPTG ptg(nodeFactory.getNumNode(), constraints);
    ptg.solve();
    ptg.dumpGraph(*this);
  } else {
    // llvm::errs() << "I choose you, steens!\n";
    SteensgardPTG ptg(nodeFactory.getNumNode(), constraints);
    ptg.solve();
    ptg.dumpGraph(*this);
  }
}
