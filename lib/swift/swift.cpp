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

/* Header file global to this project */

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

		virtual bool runOnFunction(Function &F) {
	
			for (Function::iterator b = F.begin(); b != F.end(); b++) {
				// iterate through each BasicBlock b
				
				for (BasicBlock::iterator i = b->begin(); i != b->end(); i++) {
					// iterate through each Instruction i
					
				}
			}
			
			return true;	
		}
	};
}

char Swift::ID = 0;
static RegisterPass<Swift> X("swift", "SoftWare-Implemented Fault Tolerance", false, false);
