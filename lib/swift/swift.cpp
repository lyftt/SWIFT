/*
 * File: swift.cpp
 *
 * Description:
 *  SoftWare-Implemented Fault Tolerance
 */

#include <iostream>
#include <fstream>

/* LLVM Header File
#include "llvm/Support/DataTypes.h"
*/

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/ProfileInfo.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"

/* Header file global to this project */
#include <map>

using namespace llvm;

namespace {
	struct Swift : public FunctionPass {
		static char ID;

		Swift() : FunctionPass(ID) {
			// Constructor, TBD
			
		}
		
		~Swift() {
			// Destructor, TBD
			
		}
		
		std::map<Value *, Value *> shadowMap;

		virtual bool runOnFunction(Function &F) {
	
			for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; BB++) {
				// iterate through each BasicBlock BB
				
				for (BasicBlock::iterator II = BB->begin(), IE = BB->end(); II != IE; ) {
					// iterate through each Instruction i
					Instruction &I = *II++;
					
					if (AllocaInst *AI = dyn_cast<AllocaInst>(&I)) {
						//TODO alloca inst
						errs() << "alloca inst: " << *AI << "\n";
					
					} else if (StoreInst *ST = dyn_cast<StoreInst>(&I)) {
						//TODO store inst
						errs() << "store inst: " << *ST << "\n";

					} else if (BranchInst *BR = dyn_cast<BranchInst>(&I)) {
						//TODO branch inst
						errs() << "branch inst: " << *BR << "\n";
						
					} else if (TerminatorInst *TI = dyn_cast<TerminatorInst>(&I)) {
						//TODO terminator inst
						errs() << "terminator: " << *TI << "\n";
						
					} else {
						// regular inst
						errs() << "regular inst: " << I << "\n";
						Instruction *cloned = I.clone();
						cloned->insertAfter(&I);
					}
				}
			}
			
			return true;	
		}
	};
}

char Swift::ID = 0;
static RegisterPass<Swift> X("swift", "SoftWare-Implemented Fault Tolerance", false, false);
