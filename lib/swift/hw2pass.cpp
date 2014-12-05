#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ProfileInfo.h"
#include <stdlib.h>
#include <time.h>
//#include "/home/jzzfrank/workspace/hw1part2/llvm-test/include/LAMP/LAMPLoadProfile.h"
//#include "/home/jzzfrank/workspace/hw1part2/llvm-test/include/LAMP/LAMPProfiling.h"
#include <iostream>
#include <fstream>
#include <queue>
#include "llvm/IR/Constants.h"
#include "llvm/IR/LLVMContext.h"
using namespace std;

//#include "llvm/LoopPass.h"
using namespace llvm;

namespace{
	struct memProf: public ModulePass{
		static char ID;
		//LAMPLoadProfile* LLP;
		//ProfileInfo* PI;

		memProf(): ModulePass(ID){}

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
			//errs()<<"entering!!!!!!!!!!!!!!!!!!!"<<'\n';
			//LLP = &getAnalysis<LAMPLoadProfile>();
			//PI = &getAnalysis<ProfileInfo>();
			//ofstream file;
			//file.open("mem.txt");
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
							//errs()<<"Valid Inst ID: "<<count<<" "<<*i<<'\n';
						}
						count++;
						//errs()<<LLP->InstToIdMap[i]<<'\n';
					}

				}

			}
			//for (vector<int>::iterator it = valid.begin();it!=valid.end();++it)
		//	{
				//errs()<<"valid Inst ID: "<<*it<<"  "<<'\n';
			//}
			//int random = rand()%count;
			//errs()<<"Instruction number is: "<<count<<'\n';
			//errs()<<"Random number is: "<<random<<'\n';

			int valid_size = valid.size();
			int random = rand()%valid_size;
			int random_error = rand()%256;
			int target = valid[random];
			//errs()<<"Target number: "<<target<<'\n';
			int in = 0;
			//bool find = false;
			//while(!find)
			//{	
				in = 0;
				//int random = rand()%count;
				for (Module::iterator f = M.begin(); f!=M.end(); ++f)
				{
					for (Function::iterator b = f->begin(); b!= f->end();++b)
					{
						for (BasicBlock::iterator i = b->begin(); i != b->end();++i)
						{
							//int temp = i->getOpcode();
							if (target==in)
							{
								//int code = i->getOpcode();
								//if (validInst(code))
								//{
							
								errs()<<"Before: "<<*i<<'\n';
								int user_id = 0;
								for (User::op_iterator op = i->op_begin(); op!=i->op_end(); ++op)
								{
								//	errs()<<op<<'\n';
									user_id++;	
								}
								int random_user = rand()%user_id;
								//errs()<<"Random user:"<<random_user<<'\n';
								//ConstantInt::get(Type::getInt32Ty(b->getContext()),1);
								//	find = true;
								i->setOperand(random_user,ConstantInt::get(Type::getInt32Ty(b->getContext()),random_error));


								//errs()<<"Random error number: "<<random_error<<'\n';
								errs()<<"After : "<<*i<<'\n';
								//	errs()<<"Random number is: "<<random<<'\n';
									//i->setOpcode(0);
									//break;
								//}

							}
							in++;
							//errs()<<LLP->InstToIdMap[i]<<'\n';
						}

					}

				}
			//}
	
/*
			errs()<<"FuncName: ";
			errs().write_escaped(F.getName())<<'\t'<<'\t';
			errs()<<"DynOpCount: "<<DynOpCount<<'\t';
			errs()<<"IALU: "<<IALU<<'\t';
			errs()<<"FALU: "<<FALU<<'\t';
			errs()<<"MEM: "<<MEM<<'\t';
			errs()<<"Biased-BRANCH: "<<BIASED<<'\t';
			errs()<<"Unbiased-BRANCH: "<<INBIASED<<'\t';
			errs()<<"OTHER: "<<OTHER<<'\n';
			//errs()<<"Instruction number is: "<<intAddCount<<'\n';
*/
			//file.close();
			return false;
		}
		void getAnalysisUsage(AnalysisUsage &AU) const 
		{
			//cout<<"I AM IN THE au!!!"<<endl;
			//AU.addRequired<LAMPLoadProfile>();
			//AU.addRequired<ProfileInfo>();
		}
	};
char memProf::ID = 0;
//std::out<<"com out"<<std::endl;
static RegisterPass<memProf> Y("hw2pass", "Homework 2 Pass", false, false);
}
