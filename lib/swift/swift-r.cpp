/*
 * File: swift-r.cpp
 *
 * Description:
 *  SoftWare-Implemented Fault Tolerance - Recovery
 */

#include <iostream>
#include <fstream>
#include <sstream>

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
#include <set>

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
		
	
		static Function *CreateMajorityFunction(Module *M, LLVMContext &Context, Type *type, int cnt) {
			// Create the majority function and insert it into module M.
			std::stringstream sstm;
			sstm << "majority" << cnt;
			std::string funcName = sstm.str();
		  Function *MajF = cast<Function>(M->getOrInsertFunction(funcName,
				type, type, type, type, (Type *)0));
			
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
		
		static void replacePHINodeIncomingValue(PHINode * phi, int i, User * operand, std::map<User *, AllocaInst *> &shadowMap) {
			BasicBlock * incoming = phi->getIncomingBlock(i);
			LoadInst *LdShadow = new LoadInst(shadowMap[operand], "ldShadow");
			LdShadow->insertBefore(incoming->getTerminator());
			phi->setIncomingValue(i, LdShadow);
		}

		static void storeClonedIntoShadow(Instruction * cloned, User * dest, std::map<User *, AllocaInst *> &shadowMap) {
			StoreInst * StShadow = new StoreInst(cloned, shadowMap[dest]);
			StShadow->insertAfter(cloned);
		}
		
		static CallInst * callMajorityBeforeCriticalInst(Instruction * I, User * operand, 
				Module &M, std::map<Type *, Constant *> &majorityFuncMap, std::set<Constant *> &majorityFuncSet,
				std::map<User *, AllocaInst *> &shadowMapA, std::map<User *, AllocaInst *> &shadowMapB
		) {
			
			//errs() << "insert majority() for " << *operand << "\n";
		
			Type * type = operand->getType();
			if (majorityFuncMap.find(type) == majorityFuncMap.end()) {
				majorityFuncMap[type] = CreateMajorityFunction(&M, M.getContext(), type, majorityFuncMap.size());
				majorityFuncSet.insert(majorityFuncMap[type]);
			}
			
			LoadInst *shadowA = new LoadInst(shadowMapA[operand], "ldShadow");
			shadowA->insertBefore(I);
			LoadInst *shadowB = new LoadInst(shadowMapB[operand], "ldShadow");
			shadowB->insertBefore(I);
			
			std::vector<Value *> Args(3);
			Args[0] = operand;
			Args[1] = shadowA;
			Args[2] = shadowB;
			
			CallInst *callMajority = CallInst::Create(majorityFuncMap[type], Args, "call");
			//errs() << *callMajority << "\n";
			return callMajority;
		}

		virtual bool runOnModule(Module &M) {
			
			std::map<Type *, Constant *> majorityFuncMap;
			std::set<Constant *> majorityFuncSet;

			for (Module::iterator FB = M.begin(), FE = M.end(); FB != FE;) {
				Function &F = *FB++;
				
				if (majorityFuncSet.find(&F) != majorityFuncSet.end()) continue;

				std::map<User *, AllocaInst *> shadowMapA;
				std::map<User *, AllocaInst *> shadowMapB;
				std::map<LoadInst *, Instruction *> loadInstMapA;
				std::map<LoadInst *, Instruction *> loadInstMapB;
				
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
				

				// insert duplication for each instruction
				for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; BB++) {
					for (BasicBlock::iterator II = BB->begin(), IE = BB->end(); II != IE; ) {
						
						Instruction &I = *II++;

						if (!dyn_cast<AllocaInst>(&I)
								&& !dyn_cast<BranchInst>(&I)
								&& !dyn_cast<CallInst>(&I)
								&& !dyn_cast<StoreInst>(&I)
								&& !dyn_cast<TerminatorInst>(&I)
						) {
							
							Instruction *clonedA = I.clone();
							clonedA->insertAfter(&I);
							Instruction *clonedB = I.clone();
							clonedB->insertAfter(clonedA);
							
							// use shadow variables as operands
							if (PHINode * phi = dyn_cast<PHINode>(&I)) {
								PHINode * phiA = dyn_cast<PHINode>(clonedA);
								PHINode * phiB = dyn_cast<PHINode>(clonedB);
								for (int i = 0; i < 2; i++) {
									User *operand = dyn_cast<User>(phi->getIncomingValue(i));
									if (operand && shadowMapA.find(operand) != shadowMapA.end()) {
										replacePHINodeIncomingValue(phiA, i, operand, shadowMapA);
										replacePHINodeIncomingValue(phiB, i, operand, shadowMapB);
									}
								}
							} else {
								int opidx = 0;
								for (User::op_iterator oi = I.op_begin(); oi != I.op_end(); ++oi) {
									User *operand = dyn_cast<User>(oi);
									if (shadowMapA.find(operand) != shadowMapA.end()) {
										replaceOperandWithShadow(clonedA, operand, opidx, shadowMapA);
										replaceOperandWithShadow(clonedB, operand, opidx, shadowMapB);	
									}
									opidx++;
								}
							}
							
							// store cloned instruction to correct shadow
							User *dest = dyn_cast<User>(&I);
							if (shadowMapA.find(dest) != shadowMapA.end()) {
								storeClonedIntoShadow(clonedA, dest, shadowMapA);
								storeClonedIntoShadow(clonedB, dest, shadowMapB);
							}
							
							if (LoadInst * LD = dyn_cast<LoadInst>(&I)) {
								loadInstMapA[LD] = clonedA;
								loadInstMapB[LD] = clonedB;
							}
						}
					}
				}
				

				// add check before each critical inst (store, br, call, etc.)
				
				for (Function::iterator BB = F.begin(), BE = F.end(); BB != BE; BB++) {
					for (BasicBlock::iterator II = BB->begin(), IE = BB->end(); II != IE; ) {
						Instruction &I = *II++;
						if (LoadInst * LD = dyn_cast<LoadInst>(&I)) {
							User *operand = dyn_cast<User>(LD->getPointerOperand());
							if (operand && shadowMapA.find(operand) != shadowMapA.end()) {
								CallInst * callMajority = callMajorityBeforeCriticalInst(LD, operand,
									M, majorityFuncMap, majorityFuncSet, shadowMapA, shadowMapB);
								callMajority->insertBefore(LD);
								int opidx = LD->getPointerOperandIndex();
								LD->setOperand(opidx, callMajority);
								loadInstMapA[LD]->setOperand(opidx, callMajority);
								loadInstMapB[LD]->setOperand(opidx, callMajority);
							}
						} else if (dyn_cast<StoreInst>(&I)
								|| dyn_cast<CallInst>(&I)
						) {
							
							int opidx = 0;
							for (User::op_iterator oi = I.op_begin(); oi != I.op_end(); ++oi) {
								User *operand = dyn_cast<User>(I.getOperand(opidx));
								if (operand && shadowMapA.find(operand) != shadowMapA.end()) {
									CallInst * callMajority = callMajorityBeforeCriticalInst(&I, operand, 
										M, majorityFuncMap, majorityFuncSet, shadowMapA, shadowMapB);
									callMajority->insertBefore(&I);
									I.setOperand(opidx, callMajority);
								}
							}
						} else if (BranchInst *BR = dyn_cast<BranchInst>(&I)) {
							if (BR->isConditional()) {
								User *cond = dyn_cast<User>(BR->getCondition());
								if (cond && shadowMapA.find(cond) != shadowMapA.end()) {
									CallInst * callMajority = callMajorityBeforeCriticalInst(BR, cond, 
										M, majorityFuncMap, majorityFuncSet, shadowMapA, shadowMapB);
									callMajority->insertBefore(BR);
									BR->setCondition(callMajority);
								}
							}
						} else if (SwitchInst *SW = dyn_cast<SwitchInst>(&I)) {
							User *cond = dyn_cast<User>(SW->getCondition());
							if (cond && shadowMapA.find(cond) != shadowMapA.end()) {
								CallInst * callMajority = callMajorityBeforeCriticalInst(SW, cond,
									M, majorityFuncMap, majorityFuncSet, shadowMapA, shadowMapB);
								callMajority->insertBefore(SW);
								SW->setCondition(callMajority);
							}
						}
					}
				}
			}
			return true;
		}
	};
}

char SwiftR::ID = 0;
static RegisterPass<SwiftR> X("swift-r", "SoftWare-Implemented Fault Tolerance - Recovery", false, false);
