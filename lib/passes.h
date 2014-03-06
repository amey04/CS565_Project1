#pragma once
#include "llvm/Pass.h"

using namespace llvm;

namespace cs565 {
	struct PrettyPrint : public FunctionPass {
		static char ID;
		PrettyPrint() : FunctionPass(ID) {}
		
		virtual bool runOnFunction(Function &F);
		
		virtual void getAnalysisUsage(AnalysisUsage &Info) const;
	};
}
