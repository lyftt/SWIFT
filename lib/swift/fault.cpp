#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ProfileInfo.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <queue>
#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
using namespace std;

using namespace llvm;

namespace{
	struct Fault: public ModulePass{
		static char ID;

		Fault(): ModulePass(ID){}

		bool validInst(int code)
		{
			if ((code>=8&&code<=25))// || (code==27))
				return true;

			return false;
		}
		virtual bool runOnModule(Module &M)
		{
			int count = 0;
			vector<int> valid;
			srand((unsigned int)time(NULL));
			
			for (Module::iterator f = M.begin(); f!=M.end(); ++f)
			{
				for (Function::iterator b = f->begin(); b!= f->end();++b)
				{
					for (BasicBlock::iterator i = b->begin(); i != b->end();++i)
					{
						int temp = i->getOpcode();
						if (validInst(temp))
						{
							valid.push_back(count);
						}
						count++;
					}

				}

			}

			int valid_size = valid.size();
			int random = rand()%valid_size;
			int random_error = rand()%256;
			int target = valid[random];
			int in = 0;
				
			for (Module::iterator f = M.begin(); f!=M.end(); ++f)
				{
					for (Function::iterator b = f->begin(); b!= f->end();++b)
					{
						for (BasicBlock::iterator i = b->begin(); i != b->end();++i)
						{
							if (target==in)
							{
							
								errs()<<"Before: "<<*i<<'\n';
								int user_id = 0;
								for (User::op_iterator op = i->op_begin(); op!=i->op_end(); ++op)
								{
									user_id++;	
								}
								int random_user = rand()%user_id;
								
								i->setOperand(random_user, 
									ConstantInt::get(i->getOperand(random_user)->getType(), random_error));

								errs()<<"After : "<<*i<<'\n';

							}
							in++;
						}

					}

				}

		return false;
		}
	};

	char Fault::ID = 0;
	static RegisterPass<Fault> Y("fault", "simulate soft error", false, false);
}
