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
		
		std::map<User *, AllocaInst *> shadowMap;
		std::set<User *> regSet;

		virtual bool runOnFunction(Function &F) {
			
			// build shadow map for every register
			BasicBlock *EntryBB = &(F.getEntryBlock());

			for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; BB++) {
				for (BasicBlock::iterator II = BB->begin(), IE = BB->end(); II != IE; ) {
					Instruction &I = *II++;
					if (!dyn_cast<StoreInst>(&I)
							&& !dyn_cast<BranchInst>(&I)
							&& !dyn_cast<CallInst>(&I)
							&& !dyn_cast<TerminatorInst>(&I)
					) {
						User *U = (User *) &I;
						
						// if current BB is entryBB, insert shadow before I, otherwise insert before entryBB's terminator
						Instruction * successorInst = (BB == *EntryBB) ? &I : EntryBB->getTerminator();

						// if current inst is allocca inst, simply clone it for shadow, otherwise alloca new mem for shadow
						shadowMap[U] = (dyn_cast<AllocaInst>(&I)) ? dyn_cast<AllocaInst>(I.clone()) : new AllocaInst(U->getType(), "shadow");
						
						shadowMap[U]->insertBefore(successorInst);
					}
				}
			}
			
			// insert duplication for each instruction
			for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; BB++) {
				for (BasicBlock::iterator II = BB->begin(), IE = BB->end(); II != IE; ) {
					Instruction &I = *II++;
					
					if (!dyn_cast<AllocaInst>(&I)
							&& !dyn_cast<BranchInst>(&I)
							&& !dyn_cast<CallInst>(&I)
							&& !dyn_cast<TerminatorInst>(&I)
					) {
						
						Instruction *cloned = I.clone();
						cloned->insertAfter(&I);
						
						// use shadow variables as operands
						int opidx = 0;
						for (User::op_iterator oi = I.op_begin(); oi != I.op_end(); ++oi) {
							if (User *operand = dyn_cast<User>(oi)) {
								if (shadowMap.find(operand) != shadowMap.end()) {
									if (dyn_cast<AllocaInst>(operand)) {
										// if this operand is a ptr itself, use the allocated ptr directly
										cloned->setOperand(opidx, shadowMap[operand]);

									} else {
										// for the rest operands which are not ptr, first load shadow var from mem to reg
										LoadInst *LdShadow = new LoadInst(shadowMap[operand], "ldShadow");
										LdShadow->insertBefore(cloned);
										cloned->setOperand(opidx, LdShadow);
									}
								}								
							}
							opidx++;
						}
						
						// store cloned instruction to correct shadow
						if (User *dest = dyn_cast<User>(&I)) {
							if (shadowMap.find(dest) != shadowMap.end()) {
								StoreInst * StShadow = new StoreInst(cloned, shadowMap[dest]);
								StShadow->insertAfter(cloned);
							}
						}
						
					}
				}
			}
			
			//TODO add check before each critical inst (store, br, call, etc.)
			for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; BB++) {
				for (BasicBlock::iterator II = BB->begin(), IE = BB->end(); II != IE; ) {
					Instruction &I = *II++;
					if (dyn_cast<StoreInst>(&I)
							|| dyn_cast<BranchInst>(&I)
							|| dyn_cast<CallInst>(&I)
					) {
						//TODO
							
					}
				}
			}
			
			//TODO add GSR (General Signature Register) and RTS (Run-Time Signature)	
			for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; BB++) {
				
			}
			
			return true;	
		}
	};
}

char Swift::ID = 0;
static RegisterPass<Swift> X("swift", "SoftWare-Implemented Fault Tolerance", false, false);
