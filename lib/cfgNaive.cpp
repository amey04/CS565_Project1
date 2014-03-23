#include "passes.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"

#define MAX_BLOCKS 15

namespace cs565 {
	bool CFGNaive::runOnFunction(Function &F) {

        SmallPtrSet<BasicBlock*, MAX_BLOCKS> Reachable;
        
        // Mark all reachable blocks using dfs iterator
        for (df_ext_iterator<Function*, SmallPtrSet<BasicBlock*, MAX_BLOCKS> > dfsItor =
             df_ext_begin(&F, Reachable); dfsItor != df_ext_end(&F, Reachable); ++dfsItor);
        
        std::vector<BasicBlock*> unreachableBlocks;
        
        for (Function::iterator blockItor = F.begin(); blockItor != F.end(); ++blockItor) {
            // check if block is reachable or not
            if (!Reachable.count(blockItor)) {
                BasicBlock *BB = blockItor;
                unreachableBlocks.push_back(BB);
                
                /**
                 * Note basic blocks which are not visited in DFS traversal above are not
                 * directly removed from CFG as first we need to drop all references to 
                 * those blocks. Hence a separate list of unreachable blocks is created in this loop
                 **/
                
                // Remove this BasicBlock from Predecessor list of all its successors
                for (succ_iterator succItor = succ_begin(BB); succItor != succ_end(BB); ++succItor) {
                    (*succItor)->removePredecessor(BB);
                }
                // Drop all references to this BasicBlock
                BB->dropAllReferences();
            }
        }
        
        // Actually remove the unreachable blocks.
        errs() << "\n";
        for (int i = 0; i != (int)unreachableBlocks.size(); ++i) {
            errs() << unreachableBlocks[i]->getName() << " is unreachable\n";
            unreachableBlocks[i]->eraseFromParent();
        }
        errs() << "\n";
        
        // check if we modified the grammar
        if (unreachableBlocks.size() == 0) {
            return false; // No modifications made
        }
        return true; // modifications made
	}
}

char cs565::CFGNaive::ID = 1;
static RegisterPass<cs565::CFGNaive> X("cfgNaive", "(CS 565) -Naive Unreachable Basic Blocks Removal: LLVM IR", false, false);


