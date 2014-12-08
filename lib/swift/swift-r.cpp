/*
 * File: swift-r.cpp
 *
 * Description:
 *  SoftWare-Implemented Fault Tolerance - Recovery
 */

#include <iostream>
#include <fstream>

/* LLVM Header File
#include "llvm/Support/DataTypes.h"
*/

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
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
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

/* Header file global to this project */
#include <map>

using namespace llvm;

namespace {
	struct SwiftR : public ModulePass {
		static char ID;

		SwiftR() : ModulePass(ID) {
			// Constructor, TBD
			
		}
		
		~SwiftR() {
			// Destructor, TBD
			
		}
		
		static Function *CreateMajorityFunction(Module *M, LLVMContext &Context) {
		  // Create the majority function and insert it into module M.
		  Function *MajF = cast<Function>(M->getOrInsertFunction("majority",
		    Type::getInt32Ty(Context),
			  Type::getInt32Ty(Context),
		    Type::getInt32Ty(Context),
			  Type::getInt32Ty(Context),
				(Type *)0));

		  // Get pointers to the integer argument
		  Function::arg_iterator arg = MajF->arg_begin();
		  Value *A = arg++;
		  A->setName("a");
		  Value *B = arg++;
		  B->setName("b");
		  Value *C = arg++;
		  C->setName("c");
			
		  BasicBlock *cmp1BB = BasicBlock::Create(Context, "cmp a b", MajF);
		  BasicBlock *cmp2BB = BasicBlock::Create(Context, "cmp b c", MajF);
			
		  // Create the "return a" block
		  BasicBlock *RetABB = BasicBlock::Create(Context, "return a", MajF);
		  // Create the "return b" block
		  BasicBlock *RetBBB = BasicBlock::Create(Context, "return b", MajF);
		  // Create the "return c" block
		  BasicBlock *RetCBB = BasicBlock::Create(Context, "return c", MajF);
			
			// Create "if (a==b) return a;"
		  Value *CondInst = new ICmpInst(*cmp1BB, ICmpInst::ICMP_EQ, A, B, "cond");
		  BranchInst::Create(RetABB, cmp2BB, CondInst, cmp1BB);
			
		  // Create: ret a
		  ReturnInst::Create(Context, A, RetABB);
		
		  // Create "if (b==c) return b;"
		  CondInst = new ICmpInst(*cmp2BB, ICmpInst::ICMP_EQ, B, C, "cond");
		  BranchInst::Create(RetBBB, RetCBB, CondInst, cmp2BB);
			
		  // Create: ret b
		  ReturnInst::Create(Context, B, RetBBB);
			
		  // Create: ret c
		  ReturnInst::Create(Context, C, RetCBB);
			
		  return MajF;
		}
		
		static void replaceOperandWithShadow(Instruction * cloned, User * operand, int opidx, std::map<User *, AllocaInst *> &shadowMap) {
			if (dyn_cast<AllocaInst>(operand)) {
				cloned->setOperand(opidx, shadowMap[operand]);
			} else {
				LoadInst *LdShadow = new LoadInst(shadowMap[operand], "ldShadow");
				LdShadow->insertBefore(cloned);
				cloned->setOperand(opidx, LdShadow);
			}
		}
		
		static void storeClonedIntoShadow(Instruction * cloned, User * dest, std::map<User *, AllocaInst *> &shadowMap) {
			StoreInst * StShadow = new StoreInst(cloned, shadowMap[dest]);
			StShadow->insertAfter(cloned);
		}

		virtual bool runOnModule(Module &M) {
			
			Constant *Majority = CreateMajorityFunction(&M, M.getContext());
			
			for (Module::iterator FB = M.begin(), FE = M.end(); FB != FE;) {
				Function &F = *FB++;
				
				std::map<User *, AllocaInst *> shadowMapA;
				std::map<User *, AllocaInst *> shadowMapB;
				
				errs() << "get entry block...\n";	
				// build shadow map for every register
				bool isEntryBB = true;
				BasicBlock *EntryBB;
				for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; BB++) {
					if (isEntryBB) EntryBB = BB;
					for (BasicBlock::iterator II = BB->begin(), IE = BB->end(); II != IE; ) {
						Instruction &I = *II++;
						if (!dyn_cast<AllocaInst>(&I)
								&& !dyn_cast<BranchInst>(&I)
								&& !dyn_cast<CallInst>(&I)
								&& !dyn_cast<PHINode>(&I)	//TODO fix PHINode!!!
								&& !dyn_cast<StoreInst>(&I)
								&& !dyn_cast<TerminatorInst>(&I)
						) {
							User *U = (User *) &I;
							
							// if current BB is entryBB, insert shadow before I, otherwise insert before entryBB's terminator
							Instruction * successorInst = isEntryBB ? &I : EntryBB->getTerminator();
	
							shadowMapA[U] = new AllocaInst(U->getType(), "shadowA");
							shadowMapA[U]->insertBefore(successorInst);
							shadowMapB[U] = new AllocaInst(U->getType(), "shadowB");
							shadowMapB[U]->insertBefore(successorInst);
						}
					}
					isEntryBB = false;
				}
				
				errs() << "shadow map built!" << "\n";

				// insert duplication for each instruction
				for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; BB++) {
					for (BasicBlock::iterator II = BB->begin(), IE = BB->end(); II != IE; ) {
						
						Instruction &I = *II++;
						//errs() << I << "\n";

						if (!dyn_cast<AllocaInst>(&I)
								&& !dyn_cast<BranchInst>(&I)
								&& !dyn_cast<CallInst>(&I)
								&& !dyn_cast<PHINode>(&I)	//TODO fix PHINode!!!
								&& !dyn_cast<StoreInst>(&I)
								&& !dyn_cast<TerminatorInst>(&I)
						) {
							
							Instruction *clonedA = I.clone();
							clonedA->insertAfter(&I);
							Instruction *clonedB = I.clone();
							clonedB->insertAfter(clonedA);
							
							// use shadow variables as operands
							int opidx = 0;
							for (User::op_iterator oi = I.op_begin(); oi != I.op_end(); ++oi) {
								User *operand = dyn_cast<User>(oi);
								if (shadowMapA.find(operand) != shadowMapA.end()) {
									replaceOperandWithShadow(clonedA, operand, opidx, shadowMapA);
									replaceOperandWithShadow(clonedB, operand, opidx, shadowMapB);	
								}
								opidx++;
							}
							
							//errs() << " --> " <<  *clonedA << "\n";
							//errs() << " --> " <<  *clonedB << "\n";

							// store cloned instruction to correct shadow
							User *dest = dyn_cast<User>(&I);
							if (shadowMapA.find(dest) != shadowMapA.end()) {
									storeClonedIntoShadow(clonedA, dest, shadowMapA);
									storeClonedIntoShadow(clonedB, dest, shadowMapB);
							}
						}
					}
				}
				
				errs() << "duplications inserted!\n";

				// add check before each critical inst (store, br, call, etc.)
				
				for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; BB++) {
					for (BasicBlock::iterator II = BB->begin(), IE = BB->end(); II != IE; ) {
						Instruction &I = *II++;
						if (dyn_cast<StoreInst>(&I)
								|| dyn_cast<CallInst>(&I)
								|| dyn_cast<BranchInst>(&I)
								|| dyn_cast<SwitchInst>(&I)
						) {
							
							int opidx = 0;
							for (User::op_iterator oi = I.op_begin(); oi != I.op_end(); ++oi) {
								User *operand = dyn_cast<User>(I.getOperand(opidx));
								if (operand && !dyn_cast<AllocaInst>(operand) && shadowMapA.find(operand) != shadowMapA.end()) {
									if (operand->getType() == Type::getInt32Ty(F.getContext())) {
										LoadInst *shadowA = new LoadInst(shadowMapA[operand], "ldShadow");
										shadowA->insertBefore(&I);
										LoadInst *shadowB = new LoadInst(shadowMapB[operand], "ldShadow");
										shadowB->insertBefore(&I);
																
										std::vector<Value *> Args(3);
										Args[0] = operand;
										Args[1] = shadowA;
										Args[2] = shadowB;
										
										CallInst *callMajority = CallInst::Create(Majority, Args, "call");
										callMajority->insertBefore(&I);
										I.setOperand(opidx, callMajority);
									}
								}
							}
						}
					}
						
					//errs() << "\nprint basic block:\n";
					//errs() << *BB << "\n";
				}
				
				errs() << F << "\n";
			}
			return true;
		}
	};
}

char SwiftR::ID = 0;
static RegisterPass<SwiftR> X("swift-r", "SoftWare-Implemented Fault Tolerance - Recovery", false, false);
