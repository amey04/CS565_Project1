#pragma once
#include "llvm/Pass.h"
#include "llvm/IR/Instruction.h"

#include <set>

using namespace llvm;

namespace cs565 {
	struct PrettyPrint : public FunctionPass {
		static char ID;
		PrettyPrint() : FunctionPass(ID) {}
		
		virtual bool runOnFunction(Function &F);
		
		virtual void getAnalysisUsage(AnalysisUsage &Info) const;
	};
    struct CFGNaive : public FunctionPass {
		static char ID;
		CFGNaive() : FunctionPass(ID) {}
		
		virtual bool runOnFunction(Function &F);
	};
    
    struct LiveVarAnalysis : public FunctionPass {
        static char ID;
        LiveVarAnalysis() : FunctionPass(ID) {}

        virtual bool runOnFunction(Function &F);

    };
    
    /*
     * class to hold GEN and KILL sets
     */
    class genKillSet {
    public:
        std::set<const Instruction*> gen;
        std::set<const Instruction*> kill;
        genKillSet() {
            gen.clear();
            kill.clear();
        }
    };
    
    /*
     * class to hold IN and OUT sets
     */
    class inOutSet {
    public:
        std::set<const Instruction*> in;
        std::set<const Instruction*> out;
        inOutSet() {
            in.clear();
            out.clear();
        }
    };
}
