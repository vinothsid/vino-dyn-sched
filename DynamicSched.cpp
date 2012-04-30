#include <iostream>
#include <fstream>
#include <sstream>
#include "string.h"
#include <iomanip>
#include "stdlib.h"
#include "DynamicSched.h"

RegFile::RegFile() {
	for(int i=0;i<NUM_REGS;i++) {
		ready[i] = true;
		tag[i] = -1;
	}	
}

bool RegFile::isReady(int regNo) {
	
	return ready[regNo];
}

int RegFile::setTag(int regNo,int tf) {

	tag[regNo] = tf;
	return 0;	
}

int RegFile::getTag(int regNo) {
	return tag[regNo];
}

int RegFile::setReady(int regNo,bool value) {

	ready[regNo] = value;
	return 0;
}

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

int Instruction::setParams(int tg,int optype,int dst,int s1,int s2,bool readyS1,bool readyS2) {
	tag = tg;
	operType = optype;
	dest = dst;
	src1 = s1;
	src2 = s2;
	readySrc1= readyS1;
	readySrc2 = readyS2;	
	return 0;
}

int Instruction::resetParams() {

        tag = -1;
        operType = -1;
        dest = -1;
        src1 = -1;
        src2 = -1;
	readySrc1 = false;
	readySrc2 = false;
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

int printList( list<Instruction *> *li ) {
	for(list<Instruction *>::iterator it = li->begin(); it!= li->end() ; it++) 
		(*it)->print();

}

myqueue::myqueue(int s) {
	len = s;
}

bool myqueue::isFull() {
	return len==size();
}

mylist::mylist(int s) {
	len =s;
}

bool mylist::isFull() {
	return len==size();
}


bool  DispatchList::removeCondition(Instruction * in) {
	cout << "Remove condition : " << in->getStage() << endl;
	return ((in)->getStage() == IS) ;
	
}

int DispatchList::cleanup() {
	remove_if(DispatchList::removeCondition);
}


bool  IssueList::removeCondition(Instruction * in) {
        cout << "Remove condition : " << in->getStage() << endl;
        return ((in)->getStage() == EX) ;

}
int IssueList::cleanup() {
        remove_if(IssueList::removeCondition);
}

bool  ExeList::removeCondition(Instruction * in) {
        cout << "Remove condition : " << in->getStage() << endl;
        return ((in)->getStage() == WB) ;

}

int ExeList::cleanup() {
        remove_if(ExeList::removeCondition);
}

Processor::Processor(int ROB_size,int disList_size,int issueList_size,int exeList_size) {
	totalIns = 0;
	totalCycle = 0;
	numScalarWay  = exeList_size;
	robEntryIndex = 0;
	ins = new Instruction[ROB_size];
	robQ = new myqueue(ROB_size);
	dispatchList = new DispatchList(disList_size);
	issueList = new IssueList(issueList_size);
	exeList = new ExeList(exeList_size);

	ins[0].setParams(0,0,2,0,1,true,true);
	ins[0].setStage(IS);

	ins[1].setParams(1,1,-1,0,1,true,true);
	ins[1].setStage(ID);

	ins[2].setParams(2,0,-1,0,-1,true,true);
	ins[2].setStage(IF);

	//myqueue robQ(ROB_SIZE);
	robQ->push(ins);
	robQ->push(ins+1);
	robQ->push(ins+2);

	cout << "queue size " << robQ->size() << endl ;

	//list<Instruction *> dispatchList;
	Instruction *i;

	while( robQ->size() > 0 ) {
		i = robQ->front();
		i->print();
		dispatchList->push_back(i);
		robQ->pop();
	}

	cout<<"Traversing list" << endl;
/*
	for(list<Instruction *>::iterator it = dispatchList.begin(); it!= dispatchList.end() ; it++) {
//		(*it)->print();

		if( (*it)->getStage() == ID ) {
			(*it)->setStage(IS);
			issueList.push_back(*it);	
		}

		if( (*it)->getStage() == IS) {
			(*it)->setStage(EX);
			exeList.push_back(*it);
		}
	}
*/

	cout <<"Dispatch List" << endl;
	printList(dispatchList);
	cout<<"Issue List" << endl;
	printList(issueList);
	cout<<"Execute List" << endl;
	printList(exeList);	

	dispatchList->cleanup();
	cout <<"Dispatch List" << endl;
	printList(dispatchList);

}

int Processor::renameSrcRegs(Instruction *i) {

	if( regFile.isReady(i->src1) ) {
		i->readySrc1 = true;
	} else {
#ifdef DEBUG
		cout << "Renaming register : " << i->src1 << " with " << regFile.getTag(i->src1) << endl;
#endif
		i->readySrc1 = false;
		i->src1 = regFile.getTag(i->src1);
	}

	if( regFile.isReady(i->src2) ) {
		i->readySrc1 = true;
	} else {
#ifdef DEBUG
		cout << "Renaming register : " << i->src1 << " with " << regFile.getTag(i->src1) << endl;
#endif
		i->readySrc1 = false;
		i->src2 = regFile.getTag(i->src2);
	}

	return 0;

}

int Processor::fakeRetire() {

}

int Processor::execute() {

}

int Processor::issue() {

}

int Processor::dispatch() {

}

int Processor::fetch(ifstream &myfile) {

	string line;
	char address[9];
	int i=0;
	int operType,dest,src1,src2;	
	if(myfile.is_open() ) {
		while( i < numScalarWay && !dispatchList->isFull() && getline(myfile,line) ) {
			
		//	cout << myfile.eof() << endl;
			stringstream str(line);

			str >> address;
			str >> operType;
			str >> dest;
			str >> src1;
			str >> src2;

#ifdef DEBUG
			cout <<  "INS : " << totalIns << ": " <<  address << " " << operType << " " << dest << " " << src1 << " " << src2 << endl;
#endif


			i++;
			totalIns++;
		}

		return 0;
	} else {
#ifdef DEBUG
		cout << "File is not open" << endl;
#endif

		return -1;
	}
}

int Processor::run(char *fileName) {

	ifstream myfile(fileName);

	fetch(myfile);

}

int main(int argc,char *argv[]) {

#ifdef DEBUG
	cout << "IF " << IF << " WB " << WB << endl;
#endif

	Processor proc(ROB_SIZE,8,16,4);

	proc.run("debug_trace_gcc1");
//	proc.run("tmp");
	
	//Instruction *ins = new Instruction[ROB_SIZE];
}
