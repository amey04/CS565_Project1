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
#define DEBUG1
namespace cs565 {
    /*
     * Use DenseMap instead of std::map as we are mapping instruction pointer and int
     * DenseMap is more efficient for small keys and values
     * http://llvm.org/docs/ProgrammersManual.html#map-like-containers-std-map-densemap-etc
     */
    DenseMap<const Instruction*, int> instructionMap;
    
    /*
     * Create a map for individual instruction. Assign unique integer to each instruction
     */
    void createMap(Function &F) {
        int id = 1;
        for (inst_iterator i = inst_begin(F); i !=  inst_end(F); ++i)
            instructionMap.insert(std::make_pair(&*i, id++));
    }
    
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
                // Add all instructions of BB to KILL set
                s.kill.insert(&*i);
            }
            genKillBB.insert(std::make_pair(&*b, s));
        }
    }
    
    /*
     * Implementing worklist data flow algorithm of live variable analsis
     */
    void calculateInOutSet(Function &F, DenseMap<const BasicBlock*, genKillSet> &genKillBB,
                           DenseMap<const BasicBlock*, inOutSet> &inOutBB)
    {
        SmallVector<BasicBlock*, 32> workList;
        int cnt=0;
        int iteration = 1;
        for (Function::iterator b = F.begin() ; b != F.end(); ++b){
            int numSucc = 0;
            for (succ_iterator si = succ_begin(&*b) ; si != succ_end(&*b) ; ++si)
                numSucc++;
            if(numSucc==0)
                workList.push_back(&*b);
        }
        
        while (!workList.empty()) {
            BasicBlock *b = workList.pop_back_val();
            cnt = 0;
#ifdef D
            errs() << "\nBB: " << b->getName();
#endif
            for (succ_iterator si = succ_begin(&*b) ; si != succ_end(&*b) ; ++si) {
                cnt++;
#ifdef D
                errs() << "\nSucc: " <<  si->getName();
#endif
            }
            inOutSet ios = inOutBB.lookup(b);
            
            bool shouldAddPred = !inOutBB.count(b);
            genKillSet b_genKill = genKillBB.lookup(b);

#ifdef DEBUG
            errs() << "\n**BB**: "<< b->getName() << " Size: " << cnt << "::  ";
            errs() << "\nIN: ";
            for(std::set<const Instruction*>::iterator it = ios.in.begin() ; it != ios.in.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }
            errs() << "\nOUT: ";
            for(std::set<const Instruction*>::iterator it = ios.out.begin() ; it != ios.out.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }
            
            errs() << "\nGEN: ";
            for(std::set<const Instruction*>::iterator it = b_genKill.gen.begin() ; it != b_genKill.gen.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }
            errs() << "\nKILL: ";
            for(std::set<const Instruction*>::iterator it = b_genKill.kill.begin() ; it != b_genKill.kill.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }
#endif
            // Take the union of all successors
            std::set<const Instruction*> a;
            for (succ_iterator SI = succ_begin(b), E = succ_end(b); SI != E; ++SI) {
                std::set<const Instruction*> s(inOutBB.lookup(*SI).in);
                a.insert(s.begin(), s.end());
                
#ifdef DEBUG
                errs() << "\nSucc: " << SI->getName() << " IN: ";
                for(std::set<const Instruction*>::iterator it = s.begin() ; it != s.end() ; ++it) {
                    errs() << instructionMap.lookup(*it) << " ";
                }
                for(std::set<const Instruction*>::iterator it = a.begin() ; it != a.end() ; ++it) {
                    errs() << instructionMap.lookup(*it) << " ";
                }
#endif
            }
            /*if(cnt == 0) {
                ios.in = b_genKill.gen;
            }*/
            if (a != ios.out){
                shouldAddPred = true;
                ios.out = a;
                // before = after - KILL + GEN
                ios.in.clear();
                ios.in.insert(b_genKill.gen.begin(), b_genKill.gen.end());
                
                std::set_difference(a.begin(), a.end(), b_genKill.kill.begin(), b_genKill.kill.end(),
                                    std::inserter(ios.in, ios.in.end()));
                
            }
            else if(a.size() == 0) {
                ios.in = b_genKill.gen;
                shouldAddPred = true;
            }
            
            inOutBB.erase(b);
            inOutBB.insert(std::make_pair(b, ios));
        
            
            if (shouldAddPred)
                for (pred_iterator PI = pred_begin(b), E = pred_end(b); PI != E; ++PI) {
                    workList.push_back(*PI);
                }
#ifdef DEBUG1
            errs() << "\n\nIteration " << iteration++;
            /*errs() << "\nBasic Block: "<< b->getName() << " Size: " << cnt << "::  ";
            errs() << "\nIN: ";
            for(std::set<const Instruction*>::iterator it = ios.in.begin() ; it != ios.in.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }
            errs() << "\nOUT: ";
            for(std::set<const Instruction*>::iterator it = ios.out.begin() ; it != ios.out.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }*/
            

            
            errs() << "\nBasic Block\t\tIN\t\tOUT\n";
            for (Function::iterator i = F.begin(); i != F.end(); ++i) {
                inOutSet s = inOutBB.lookup(&*i);
                errs() << "BB " << i->getName() << ":\t\t( ";
                
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
    
	bool LiveVarAnalysis::runOnFunction(Function &F) {
        
        // Create instruction map for this function.
        createMap(F);
        
        DenseMap<const BasicBlock*, genKillSet> genKillBB;
        // Calculate basic block's GEN and KILL sets.
        calculateGenKillSet(F, genKillBB);
        
        errs() << "\nBasic Block\t\tGEN\t\tKILL\n";
        for (Function::iterator i = F.begin(); i != F.end(); ++i) {
            genKillSet s = genKillBB.lookup(&*i);
            errs() << "BB " << i->getName() << ":\t\t( ";
            
            for(std::set<const Instruction*>::iterator it = s.gen.begin() ; it != s.gen.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }
            errs() << ")\t\t( ";
            for(std::set<const Instruction*>::iterator it = s.kill.begin() ; it != s.kill.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }
            
            errs() << ")\n";

        }
        DenseMap<const BasicBlock*, inOutSet> inOutBB;
        // Calculate basic block's IN and OUT sets.
        
        calculateInOutSet(F, genKillBB, inOutBB);
        
        DenseMap<const Instruction*, inOutSet> inOutInst;
        // Calculate instruction's IN and OUT sets.
        calculateInstructionInOutSet(F, inOutBB, inOutInst);

        /*errs() << "\nBasic Block\t\tIN\t\tOUT\n";
        for (Function::iterator i = F.begin(); i != F.end(); ++i) {
            inOutSet s = inOutBB.lookup(&*i);
            errs() << "BB " << i->getName() << ":\t\t( ";
            
            for(std::set<const Instruction*>::iterator it = s.in.begin() ; it != s.in.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }
            errs() << ")\t\t( ";
            for(std::set<const Instruction*>::iterator it = s.out.begin() ; it != s.out.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }
            
            errs() << ")\n";
        }
        
        errs() << "\nBasic Block\t\tGEN\t\tKILL\n";
        for (Function::iterator i = F.begin(); i != F.end(); ++i) {
            genKillSet s = genKillBB.lookup(&*i);
            errs() << "BB " << i->getName() << ":\t\t( ";
            
            for(std::set<const Instruction*>::iterator it = s.gen.begin() ; it != s.gen.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }
            errs() << ")\t\t( ";
            for(std::set<const Instruction*>::iterator it = s.kill.begin() ; it != s.kill.end() ; ++it) {
                errs() << instructionMap.lookup(*it) << " ";
            }
            
            errs() << ")\n";
        }*/

        

        errs() << "\n\nIN and OUT sets for instruction";
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

        return false; // No modifications to IR
    }
}

char cs565::LiveVarAnalysis::ID = 1;
static RegisterPass<cs565::LiveVarAnalysis> X("liveVarAnalysis", "(CS 565) - Live Variable Analysis Pass: LLVM", false, false);



