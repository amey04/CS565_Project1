#include <vector>
#include <set>

#include "passes.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"

#define MAX_BLOCKS 30
#define DISPLAY

namespace cs565 {
    /*
     * Use DenseMap instead of std::map as we are mapping instruction pointer and int
     * DenseMap is more efficient for small keys and values
     * http://llvm.org/docs/ProgrammersManual.html#map-like-containers-std-map-densemap-etc
     */
    DenseMap<const Instruction*, int> instructionMap;
    
    /*
     * GEN:  set of upward exposed uses.
     * KILL: set of definations
     */
    void calculateGenKillSet(Function &F, DenseMap<const BasicBlock*, genKillSet> &genKillBB)
    {
        for (Function::iterator b = F.begin() ; b != F.end() ; ++b) {
            genKillSet s;
            for (BasicBlock::iterator i = b->begin() ; i != b->end() ; ++i) {
                for (unsigned j = 0; j < i->getNumOperands(); j++) {
                    Value *v = i->getOperand(j);
                    if (isa<Instruction>(v)) {
                        Instruction *op = cast<Instruction>(v);
                        // Check if instruction operand is in KILL set
                        // Add instruction operand to GEN set if not present in KILL set
                        if (!s.kill.count(op))
                            s.gen.insert(op);
                    }
                }
                // Add all instructions of BB to KILL set (this will include all pseude-registers)
                s.kill.insert(&*i);
            }
            genKillBB.insert(std::make_pair(&*b, s));
        }
    }
    
    /*
     * Implementing worklist data flow algorithm of live variable analysis
     */
    void calculateInOutSet(Function &F, DenseMap<const BasicBlock*, inOutSet> &inOutBB, DenseMap<const BasicBlock*, genKillSet> &genKillBB)
    {
        SmallVector<BasicBlock*, MAX_BLOCKS> workList; //use SmallVector as it provides pop_with_val function unlike std::vector
        int iteration = 1;
        
        // Add BBs with no successors to worklist
        for (Function::iterator b = F.begin() ; b != F.end(); ++b){
            int num = 0;
            for (succ_iterator si = succ_begin(&*b) ; si != succ_end(&*b) ; ++si)
                num++;
            if(num == 0)
                workList.push_back(&*b);
        }
        
        while (!workList.empty()) {
            BasicBlock *b = workList.pop_back_val();
            inOutSet ios = inOutBB.lookup(b);
            
            genKillSet b_genKill = genKillBB.lookup(b);

            // Take the union of IN sets of all successors of current basic block
            std::set<const Instruction*> uni;
            for (succ_iterator SI = succ_begin(b), E = succ_end(b); SI != E; ++SI) {
                std::set<const Instruction*> s(inOutBB.lookup(*SI).in);
                uni.insert(s.begin(), s.end());
            }

            bool addToWorklist = !inOutBB.count(b);
            if (uni != ios.out){
                addToWorklist = true;
                ios.out = uni;
                // IN = GEN + (OUT - KILL)
                ios.in.clear();
                ios.in.insert(b_genKill.gen.begin(), b_genKill.gen.end());
                
                //<codeblock provided in LLVM docs>
                std::set_difference(uni.begin(), uni.end(), b_genKill.kill.begin(), b_genKill.kill.end(),
                                    std::inserter(ios.in, ios.in.end()));
                //</codeblock>
            }
            else if(uni.size() == 0) {
                ios.in = b_genKill.gen;
                addToWorklist = true;
            }
            
            inOutBB.erase(b);
            inOutBB.insert(std::make_pair(b, ios));
        
            // if OUT set of BB is changed need to recompute IN and OUT of all pred
            if (addToWorklist) {
                for (pred_iterator pi = pred_begin(b) ; pi != pred_end(b); ++pi) {
                    workList.push_back(*pi);
                }
            }
            
#ifdef DISPLAY
            errs() << "\n\nIteration " << iteration++;
            errs() << "\nBasic Block\tIN\t\tOUT\n";
            for (Function::iterator i = F.begin(); i != F.end(); ++i) {
                inOutSet s = inOutBB.lookup(&*i);
                errs() << i->getName() << ":\t\t( ";
                
                for(std::set<const Instruction*>::iterator it = s.in.begin() ; it != s.in.end() ; ++it) {
                    errs() << instructionMap.lookup(*it) << " ";
                }
                errs() << ")\t\t( ";
                for(std::set<const Instruction*>::iterator it = s.out.begin() ; it != s.out.end() ; ++it) {
                    errs() << instructionMap.lookup(*it) << " ";
                }
                
                errs() << ")\n";
            }
#endif
        }

    }
    
    /*
     * Calculate instruction's IN and OUT sets using BB's IN and OUT sets
     */
    void calculateInstructionInOutSet(Function &F, DenseMap<const BasicBlock*, inOutSet> &inOutBB,
                             DenseMap<const Instruction*, inOutSet> &inOutInst)
    {
        for (Function::iterator b = F.begin(); b != F.end(); ++b) {
            BasicBlock::iterator i = --b->end();
            std::set<const Instruction*> out(inOutBB.lookup(b).out);
            std::set<const Instruction*> in(out);
            
            do {
                for (unsigned j = 0; j < i->getNumOperands(); j++) {
                    Value *v = i->getOperand(j);
                    if (isa<Instruction>(v))
                        in.insert(cast<Instruction>(v));
                }
                in.erase(i);
                
                inOutSet ios;
                ios.in = in;
                ios.out = out;
                
                inOutInst.insert(std::make_pair(&*i, ios));
                
                out = in;
                --i;
            } while (i != b->begin());
        }
    }
    
    /*
     * Create a map for individual instruction. Assign unique integer to each instruction
     */
    void createMap(Function &F) {
        int id = 1;
        for (inst_iterator i = inst_begin(F); i !=  inst_end(F); ++i)
            instructionMap.insert(std::make_pair(&*i, id++));
    }
    
	bool LiveVarAnalysis::runOnFunction(Function &F) {
        
        // Create instruction map for this function.
        createMap(F);
        
        DenseMap<const BasicBlock*, genKillSet> genKillBB;
        // Calculate basic block's GEN and KILL sets.
        calculateGenKillSet(F, genKillBB);
  
#ifdef DISPLAY
        errs() << "\n\nGEN and KILL sets of Basic Blocks";
        errs() << "\nBasic Block\tGEN\t\tKILL\n";
        for (Function::iterator i = F.begin(); i != F.end(); ++i) {
            genKillSet s = genKillBB.lookup(&*i);
            errs() << i->getName() << ":\t\t( ";
            
            for(std::set<const Instruction*>::iterator it = s.gen.begin() ; it != s.gen.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }
            errs() << ")\t\t( ";
            for(std::set<const Instruction*>::iterator it = s.kill.begin() ; it != s.kill.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }
            errs() << ")\n";
        }
#endif
        
        DenseMap<const BasicBlock*, inOutSet> inOutBB;
        // Calculate basic block's IN and OUT sets.
        errs() << "\nIterative data-flow algorithm";
        calculateInOutSet(F, inOutBB, genKillBB);
        
        DenseMap<const Instruction*, inOutSet> inOutInst;
        // Calculate instruction's IN and OUT sets.
        calculateInstructionInOutSet(F, inOutBB, inOutInst);
        
#ifdef DISPLAY
        errs() << "\n\nIN and OUT sets for individual instruction";
        errs() << "\nInstruction\tIN\t\tOUT\n";
        for (inst_iterator i = inst_begin(F), E = inst_end(F); i != E; ++i) {
            inOutSet s = inOutInst.lookup(&*i);
            errs() << "%" << instructionMap.lookup(&*i) << ":\t\t( ";

            for(std::set<const Instruction*>::iterator it = s.in.begin() ; it != s.in.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
                
            }
            errs() << ")\t\t( ";
            for(std::set<const Instruction*>::iterator it = s.out.begin() ; it != s.out.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }

            errs() << ")\n";
        }
#endif
        return false; // No modifications to IR
    }
}

char cs565::LiveVarAnalysis::ID = 1;
static RegisterPass<cs565::LiveVarAnalysis> X("liveVarAnalysis", "(CS 565) - Live Variable Analysis Pass: LLVM", false, false);



