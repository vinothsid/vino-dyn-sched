#include <iostream>
#include <fstream>
#include <sstream>
#include "string.h"
#include <iomanip>
#include "stdlib.h"
#include <algorithm>
#include <queue>
#include <list>
#include <deque>
#define NUM_REGS 128
#define ROB_SIZE 1024

#define DEBUG 1

using namespace std;

enum pipe_stage_t {IF,ID,IS,EX,WB,NUM_STAGES,INVALID_STAGE};
string stageNames[NUM_STAGES] = {"IF","ID","IS","EX","WB"};

class Instruction {
	int tag;
	int operType;
	int dest;
	int src1;
	int src2;
	pipe_stage_t curStage;
	int bCycle[NUM_STAGES]; // begin cycle time
	int duration[NUM_STAGES];
	
public:
	Instruction();
	int setStage(pipe_stage_t state);
	pipe_stage_t getStage();
	int setParams(int tg,int optype,int dst,int s1,int s2);
	int resetParams();
	int setBeginCycle(int ct,pipe_stage_t state);
	int incrDuration( pipe_stage_t state);
	bool isDone();
	int renameSrcRegs();
	int print();
};

Instruction::Instruction() {
	tag = -1;
	operType = -1;
	dest = -1;
	src1 = -1;
	src2 = -1;
	curStage = INVALID_STAGE;
	for(int i=0;i<NUM_STAGES;i++) {
		bCycle[i] = -1;
		duration[i] = 0;
	}

}

int Instruction::setStage(pipe_stage_t state) {
	curStage = state;
	return 0;

}

pipe_stage_t Instruction::getStage() {
	return curStage;
}

int Instruction::setParams(int tg,int optype,int dst,int s1,int s2) {
	tag = tg;
	operType = optype;
	dest = dst;
	src1 = s1;
	src2 = s2;

	return 0;
}

int Instruction::resetParams() {

        tag = -1;
        operType = -1;
        dest = -1;
        src1 = -1;
        src2 = -1;
        curStage = INVALID_STAGE;
        for(int i=0;i<NUM_STAGES;i++) {
                bCycle[i] = -1;
                duration[i] = 0;
        }

}

int Instruction::setBeginCycle(int ct,pipe_stage_t stage) {
	bCycle[stage] = ct;
	return 0;

}

int Instruction::incrDuration( pipe_stage_t state) {
	duration[state]++;
	return 0;
}

bool Instruction::isDone() {
	return (curStage == WB) ;
}

int Instruction::renameSrcRegs() {

}

int Instruction::print() {

	cout << tag << " fu{" << operType << "} src{" << src1 << "," << src2 << "} dst{" << dest << "}" ; //IF{<begin-cycle>,<duration>} ID{…} IS{…} EX{…} WB{…}
	for(int i=0;i<NUM_STAGES;i++) {
		cout << " " << stageNames[i] << "{" << bCycle[i] << "," << duration[i] << "}" ;
	}
#ifdef DEBUG
	cout << " CurStage:" << curStage;
#endif
	cout << endl;

}
class RegFile {
	bool ready[NUM_REGS];
	int tag[NUM_REGS];
public:
	RegFile();
	bool isReady(int regNo);
	int setTag(int regNo,int tg);
	int getTag(int regNo);
	int setReady(int regNo,bool value);

};

class myqueue : public queue<Instruction *> {
	int len;
public:
	myqueue(int s);
	bool isFull();

};

myqueue::myqueue(int s) {
	len = s;
}

bool myqueue::isFull() {
	return len==size();
}

class mylist : public list<Instruction *> {
	int len;
public:
	mylist(int s);
	bool isFull();
	virtual int cleanup() = 0 ;
};

mylist::mylist(int s) {
	len =s;
}

bool mylist::isFull() {
	return len==size();
}

class DispatchList : public mylist {

public:
	DispatchList(int s) : mylist(s) {}
	bool static removeCondition(Instruction *in );
	int cleanup();

};

bool  DispatchList::removeCondition(Instruction * in) {
	cout << "Remove condition : " << in->getStage() << endl;
	return ((in)->getStage() == IS) ;
	
}

int DispatchList::cleanup() {
	remove_if(DispatchList::removeCondition);
}


class IssueList : public mylist {
public:
        IssueList(int s) : mylist(s) {}
        bool static removeCondition(Instruction *in );
        int cleanup();

};
bool  IssueList::removeCondition(Instruction * in) {
        cout << "Remove condition : " << in->getStage() << endl;
        return ((in)->getStage() == EX) ;

}
int IssueList::cleanup() {
        remove_if(IssueList::removeCondition);
}


class ExeList : public mylist {
public:
        ExeList(int s) : mylist(s) {}
        bool static removeCondition(Instruction *in );
        int cleanup();

};

bool  ExeList::removeCondition(Instruction * in) {
        cout << "Remove condition : " << in->getStage() << endl;
        return ((in)->getStage() == WB) ;

}

int ExeList::cleanup() {
        remove_if(ExeList::removeCondition);
}

class Processor {
	int totalIns;
	int totalCycle;
	Instruction *ins;
	myqueue *robQ;
        DispatchList *dispatchList;
        IssueList *issueList;
        ExeList *exeList;	
	//dispatch list
	//issue list
	//execute list
	//ROB queue

public:
	Processor(int ROB_size,int disList_size,int issueList_size,int exeList_size);

};

int printList( list<Instruction *> &li ) {
	for(list<Instruction *>::iterator it = li.begin(); it!= li.end() ; it++) 
		(*it)->print();

}
int main(int argc,char *argv[]) {

#ifdef DEBUG
	cout << "IF " << IF << " WB " << WB << endl;
#endif

	Instruction *ins = new Instruction[ROB_SIZE];
	ins[0].setParams(0,0,2,0,1);
	ins[0].setStage(IS);

	ins[1].setParams(1,1,-1,0,1);
	ins[1].setStage(ID);

	ins[2].setParams(2,0,-1,0,-1);
	ins[2].setStage(IF);

	myqueue robQ(ROB_SIZE);
	robQ.push(ins);
	robQ.push(ins+1);
	robQ.push(ins+2);

	cout << "queue size " << robQ.size() << endl ;

	//list<Instruction *> dispatchList;
	DispatchList dispatchList(5);
	list<Instruction *> issueList;
	list<Instruction *> exeList;
	Instruction *i;

	while( robQ.size() > 0 ) {
		i = robQ.front();
		i->print();
		dispatchList.push_back(i);
		robQ.pop();
	}

	cout<<"Traversing list" << endl;

	for(list<Instruction *>::iterator it = dispatchList.begin(); it!= dispatchList.end() ; it++) {
//		(*it)->print();

/*		if( (*it)->getStage() == ID ) {
			(*it)->setStage(IS);
			issueList.push_back(*it);	
		}

		if( (*it)->getStage() == IS) {
			(*it)->setStage(EX);
			exeList.push_back(*it);
		}
*/
	}

	cout <<"Dispatch List" << endl;
	printList(dispatchList);
	cout<<"Issue List" << endl;
	printList(issueList);
	cout<<"Execute List" << endl;
	printList(exeList);	

	dispatchList.cleanup();
	cout <<"Dispatch List" << endl;
	printList(dispatchList);
}
