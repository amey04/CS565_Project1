#include "passes.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"

namespace cs565 {
	bool CFGNaive::runOnFunction(Function &F) {
        /*GraphTraits<BasicBlock> *gt;
        df_ext_iterator<BasicBlock> *it;
        
        for (Function::iterator blockItor = F.begin() ;  blockItor != F.end(); ++blockItor)
        {
            for (BasicBlock::iterator instItor = blockItor->begin() ; instItor != blockItor->end() ; ++instItor)
            {
            }
        }*/

        //for each function iterating over the basic blocks
       /* for (Function::iterator blockItor = F.begin() ;  blockItor != F.end(); ++blockItor)
        {
            BasicBlock* BB = dyn_cast<BasicBlock>(&*blockItor);
            //for(idf_iterator<BasicBlock*> I=idf_begin(BB); I!=idf_end(BB);
                 //++I)
            for (df_ext_iterator<BasicBlock*> dfs = df_ext_begin(BB, <#SetTy &S#>); <#condition#>; <#increment#>) {
                <#statements#>
            }
            {   // for each basic block walking over the predecessors in the graph
                BasicBlock* B = *I;
            }
        }
        
		return false;*/
        
        SmallPtrSet<BasicBlock*, 8> Reachable;
        
        // Mark all reachable blocks.
        for (df_ext_iterator<Function*, SmallPtrSet<BasicBlock*, 8> > I =
             df_ext_begin(&F, Reachable), E = df_ext_end(&F, Reachable); I != E; ++I)
        /* Mark all reachable blocks */;
        
        // Loop over all dead blocks, remembering them and deleting all instructions
        // in them.
        std::vector<BasicBlock*> DeadBlocks;
        for (Function::iterator I = F.begin(), E = F.end(); I != E; ++I)
            if (!Reachable.count(I)) {
                BasicBlock *BB = I;
                DeadBlocks.push_back(BB);
                /*while (PHINode *PN = dyn_cast<PHINode>(BB->begin())) {
                    PN->replaceAllUsesWith(Constant::getNullValue(PN->getType()));
                    BB->getInstList().pop_front();
                }*/
                for (succ_iterator SI = succ_begin(BB), E = succ_end(BB); SI != E; ++SI)
                    (*SI)->removePredecessor(BB);
                BB->dropAllReferences();
            }
        
        // Actually remove the blocks now.
        errs() << "\n";
        for (int i = 0, e = (int)DeadBlocks.size(); i != e; ++i) {
            errs() << DeadBlocks[i]->getName() << " is unreachable\n";
            DeadBlocks[i]->eraseFromParent();
        }
        errs() << "\n";
        return DeadBlocks.size();
	}
}

char cs565::CFGNaive::ID = 1;
static RegisterPass<cs565::CFGNaive> X("cfgNaive", "(CS 565) -Naive Unreachable Basic Blocks Removal: LLVM IR", false, false);

