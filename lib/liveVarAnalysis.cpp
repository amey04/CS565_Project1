#include "passes.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/ADT/DenseMap.h"

#define MAX_BLOCKS 30

namespace cs565 {
    /*
     * Use DenseMap instead of std::map as we are mapping instruction pointer and int
     * DenseMap is more efficient for small keys and values
     * http://llvm.org/docs/ProgrammersManual.html#map-like-containers-std-map-densemap-etc
     */
    DenseMap<const Instruction*, int> instructionMap;
    
    /*
     * structure to hold GEN and KILL sets
     */
    struct genKillSet {
    public:
        std::set<const Instruction*> gen;
        std::set<const Instruction*> kill;
    };
    
    /*
     * Struct to hold IN and OUT sets
     */
    struct inOutSet {
    public:
        std::set<const Instruction*> in;
        std::set<const Instruction*> out;
    };
    
    /*
     * Create a map for individual instruction. Assign unique integer to each instruction
     */
    void createMap(Function &F) {
        int id = 1;
        for (inst_iterator i = inst_begin(F); i !=  inst_end(F); ++i)
            instructionMap.insert(std::make_pair(&*i, id++));
    }

    inline void print_elem(const Instruction* i) {
        errs() << instructionMap.lookup(i) << " ";
    }
    
    void calculateGenKillSet(Function &F, DenseMap<const BasicBlock*, genKillSet> &genKillBB)
    {
        for (Function::iterator b = F.begin(), e = F.end(); b != e; ++b) {
            genKillSet s;
            for (BasicBlock::iterator i = b->begin(), e = b->end(); i != e; ++i) {
                // The GEN set is the set of upwards-exposed uses:
                // pseudo-registers that are used in the block before being
                // defined. (Those will be the pseudo-registers that are defined
                // in other blocks, or are defined in the current block and used
                // in a phi function at the start of this block.)
                int n = i->getNumOperands();
                for (int j = 0; j < n; j++) {
                    Value *v = i->getOperand(j);
                    if (isa<Instruction>(v)) {
                        Instruction *op = cast<Instruction>(v);
                        // Check if instruction operand is in KILL set
                        // Add instruction operand to GEN set if not present in KILL
                        if (!s.kill.count(op))
                            s.gen.insert(op);
                    }
                }
                // Add all instructions of BB to KILL set
                // This will include all pseudo registers
                s.kill.insert(&*i);
            }
            genKillBB.insert(std::make_pair(&*b, s));
        }
    }
    
    void calculateInOutSet(Function &F, DenseMap<const BasicBlock*, genKillSet> &genKillBB,
                              DenseMap<const BasicBlock*, inOutSet> &inOutBB)
    {
        /*
         * Implementing worklist data flow algorithm studied in class
         */
        
        SmallVector<BasicBlock*, MAX_BLOCKS> workList; // Keeps track of BBs that needs to be revisited
        
        workList.push_back(--F.end());
        
        do {
            BasicBlock *b = workList.pop_back_val();
            inOutSet b_inOutSet = inOutBB.lookup(b);
            bool shouldAddPred = !inOutBB.count(b);
            genKillSet b_genKillSet = genKillBB.lookup(b);
            
            // Take the union of all successors
            std::set<const Instruction*> a;
            for (succ_iterator SI = succ_begin(b), E = succ_end(b); SI != E; ++SI) {
                std::set<const Instruction*> s(inOutBB.lookup(*SI).in);
                a.insert(s.begin(), s.end());
            }
            
            if (a != b_inOutSet.out) {
                shouldAddPred = true;
                b_inOutSet.out = a;
                // in = out - KILL + GEN
                b_inOutSet.in.clear();
                std::set_difference(a.begin(), a.end(), b_genKillSet.kill.begin(), b_genKillSet.kill.end(),
                                    std::inserter(b_inOutSet.in, b_inOutSet.in.end()));
                b_inOutSet.in.insert(b_genKillSet.gen.begin(), b_genKillSet.gen.end());
            }
            
            if (shouldAddPred) {
                for (pred_iterator PI = pred_begin(b), E = pred_end(b); PI != E; ++PI)
                    workList.push_back(*PI);
            }
        } while (!workList.empty());
    }
    
    void calculateInstructionInOutSet(Function &F, DenseMap<const BasicBlock*, inOutSet> &inOutBB,
                             DenseMap<const Instruction*, inOutSet> &inOutInst)
    {
        for (Function::iterator f = F.begin(); f != F.end(); ++f) {
            BasicBlock::iterator b = --f->end();
            std::set<const Instruction*> out(inOutBB.lookup(f).out);
            std::set<const Instruction*> in(out);
            
            while (true) {
                // IN = GEN + ( OUT - KILL )
                in.erase(b);
                
                unsigned n = b->getNumOperands();
                for (unsigned j = 0; j < n; j++) {
                    Value *v = b->getOperand(j);
                    if (isa<Instruction>(v))
                        in.insert(cast<Instruction>(v));
                }
                
                inOutSet ba;
                ba.in = in;
                ba.out = out;
                inOutInst.insert(std::make_pair(&*b, ba));
                
                out = in;
                if (b == f->begin())
                    break;
                --b;
            }
        }
    }
    
	bool LiveVarAnalysis::runOnFunction(Function &F) {
        
        // Create instruction map for this function.
        createMap(F);
        
        DenseMap<const BasicBlock*, genKillSet> genKillBB;
        // Calculate basic block's GEN and KILL sets.
        calculateGenKillSet(F, genKillBB);
        
        DenseMap<const BasicBlock*, inOutSet> inOutBB;
        // Calculate basic block's IN and OUT sets.
        calculateInOutSet(F, genKillBB, inOutBB);
        
        DenseMap<const Instruction*, inOutSet> inOutInst;
        // Calculate instruction's IN and OUT sets.
        calculateInstructionInOutSet(F, inOutBB, inOutInst);
        
        for (inst_iterator i = inst_begin(F), E = inst_end(F); i != E; ++i) {
            inOutSet s = inOutInst.lookup(&*i);
            errs() << "%" << instructionMap.lookup(&*i) << ": { ";
            std::for_each(s.in.begin(), s.in.end(), print_elem);
            errs() << "} { ";
            std::for_each(s.out.begin(), s.out.end(), print_elem);
            errs() << "}\n";
        }

        
        return false; // No modification made
    }
}

char cs565::LiveVarAnalysis::ID = 1;
static RegisterPass<cs565::LiveVarAnalysis> X("liveVarAnalysis", "(CS 565) - Live Variable Analysis Pass: LLVM", false, false);



