#include <map>

#include "passes.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"

namespace cs565 {
	bool PrettyPrint::runOnFunction(Function &F) {
        int instNo=1;
        std::map<Instruction *, int> instTable;
        
		errs() << "FUNCTION ";
		errs().write_escaped(F.getName()) << "\n\n";
        
        for (Function::iterator blockItor = F.begin() ;  blockItor != F.end(); ++blockItor)
        {
            errs() << "\nBASIC BLOCK " << blockItor->getName() << "\n";
            for (BasicBlock::iterator instItor = blockItor->begin() ; instItor != blockItor->end() ; ++instItor)
            {
                instTable.insert(make_pair(instItor, instNo));
                
                errs() << "%" << instNo++ << ":\t" << instItor->getOpcodeName() << " ";
                int ops = instItor->getNumOperands();
                for(int j=0 ; j<ops ; ++j)
                {
                    //Check if operand is instruction operand
                    std::map<Instruction*,int>::iterator it;
                    
                    if((it=instTable.find(reinterpret_cast<Instruction*>(instItor->getOperand(j)))) != instTable.end())
                    {
                        errs() << "%" << it->second << " ";
                    }
                    
                    //check if operand is named operand
                    else if(instItor->getOperand(j)->hasName())
                    {
                        errs() << instItor->getOperand(j)->getName() << " ";
                    }
                    //check if operand is unnamed operand
                    else
                    {
                        ConstantInt *i = dyn_cast<ConstantInt>(instItor->getOperand(j));
                        if(i)
                            errs() << i->getValue() << " ";
                        else
                            errs() << "XXX ";
                    }
                }
                
                /*for (User::op_iterator i = instItor->op_begin(); i != instItor->op_end(); ++i) {
                 Value *v = *i;
                 errs() << " " << v->getName() << " " << v->getValueName()<< " ";
                 }
                 for (Value::use_iterator i = instItor->use_begin(); i != instItor->use_end(); ++i) {
                 //Value *v = *i;
                 errs() << *i << " ";
                 }*/
                errs() << "\n";
            }
        }
		return false;
	}
	
	void PrettyPrint::getAnalysisUsage(AnalysisUsage &Info) const {
		Info.setPreservesAll();
	}
}

char cs565::PrettyPrint::ID = 0;
static RegisterPass<cs565::PrettyPrint> X("prettyPrint", "(CS 565) - Pretty Print LLVM IR", false, false);
