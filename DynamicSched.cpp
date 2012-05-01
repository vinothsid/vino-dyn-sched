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

int RegFile::print() {
	for(int i=0;i<NUM_REGS;i++) {
		cout << i << " " << ready[i] << " " << tag[i] << endl;
	}
}
Instruction::Instruction() {
	tag = -1;
	operType = -1;
	dest = -1;
	src1 = -1;
	src2 = -1;
	origSrc1 = -1;
	origSrc2 = -1;
	readySrc1 = true;
	readySrc2 = true;
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

int Instruction::setParams(int tg,int optype,int dst,int s1,int s2,bool readyS1 ,bool readyS2 ) {
	tag = tg;
	operType = optype;
	dest = dst;
	src1 = s1;
	src2 = s2;
	origSrc1 = s1;
	origSrc2 = s2;
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
	readySrc1 = true ;
	readySrc2 = true ;
	origSrc1 = -1;
	origSrc2 = -1;
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

	cout << tag << " fu{" << operType << "} src{" << origSrc1 << "," << origSrc2 << "} dst{" << dest << "}" ; //IF{<begin-cycle>,<duration>} ID{…} IS{…} EX{…} WB{…}
	for(int i=0;i<NUM_STAGES;i++) {
		cout << " " << stageNames[i] << "{" << bCycle[i] << "," << duration[i] << "}" ;
	}
#ifdef DEBUG
	cout << " CurStage:" << curStage << " readySrc {" << readySrc1 << "," << readySrc2 << "}" << " renamed src{" << src1 << "," << src2 << "}"  ;
#endif
	cout << endl;

}

int printList( list<Instruction *> *li ) {
	for(list<Instruction *>::iterator it = li->begin(); it!= li->end() ; it++) 
		(*it)->print();

}
/*
myqueue::myqueue(int s) {
	len = s;
}

bool myqueue::isFull() {
	return len==size();
}
*/
int myqueue::cleanup() {
	Instruction *i = front();
	
	while ( size()!= 0 && i->isDone() ) {
		i->print();
		i->resetParams();
		pop_front();
		i = front();
	}
	
}

mylist::mylist(int s) {
	len =s;
}

bool mylist::isFull() {
//	cout << " length : " << len << "size() " << size() << endl;
	return len==size();
}


bool  DispatchList::removeCondition(Instruction * in) {
	//cout << "Remove condition : " << in->getStage() << endl;
	return ((in)->getStage() == IS) ;
	
}

int DispatchList::cleanup() {
	remove_if(DispatchList::removeCondition);
}


bool  IssueList::removeCondition(Instruction * in) {
    //    cout << "Remove condition : " << in->getStage() << endl;
        return ((in)->getStage() == EX) ;

}
int IssueList::cleanup() {
        remove_if(IssueList::removeCondition);
}

bool  ExeList::removeCondition(Instruction * in) {
      //  cout << "Remove condition : " << in->getStage() << endl;
        return ((in)->getStage() == WB) ;

}

int ExeList::cleanup() {
        remove_if(ExeList::removeCondition);
}

struct comp {
	bool operator()(Instruction *i,Instruction *j) {
		return ( i->tag < j->tag);
	}
} mycomp;

bool myfunc(Instruction *i,Instruction *j) {
	return ( i->tag < j->tag);
}
Processor::Processor(int ROB_size,int disList_size,int issueList_size,int nWay,char *fileName ) : myfile(fileName)
{
	totalIns = 0;
	totalCycle = 0;
	numScalarWay  = nWay;
	robEntryIndex = 0;
	schedQueueSize = issueList_size;
	ins = new Instruction[ROB_size];
	robQ = new myqueue(ROB_size);
	dispatchList = new DispatchList(disList_size);
	issueList = new IssueList(issueList_size);
	exeList = new ExeList( numScalarWay * MAX_EXEC_CYCLE);

/*
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


	cout <<"Dispatch List" << endl;
	printList(dispatchList);
	cout<<"Issue List" << endl;
	printList(issueList);
	cout<<"Execute List" << endl;
	printList(exeList);	

	dispatchList->cleanup();
	cout <<"Dispatch List" << endl;
	printList(dispatchList);
*/
}

int Processor::renameSrcRegs(Instruction *i) {

	if ( i->src1 != -1) {
		if( regFile.isReady(i->src1) ) {
			i->readySrc1 = true;
		} else {
#ifdef DEBUG
			cout << "Renaming register : " << i->src1 << " with " << regFile.getTag(i->src1) << endl;
#endif
			i->readySrc1 = false;
			i->src1 = regFile.getTag(i->src1);
		}

	} else {
		i->readySrc1 = true;
	}

	if ( i->src2 != -1) {

		if( regFile.isReady(i->src2) ) {
			i->readySrc2 = true;
		} else {
#ifdef DEBUG
			cout << "Renaming register : " << i->src2 << " with " << regFile.getTag(i->src2) << endl;
#endif
			i->readySrc2 = false;
			i->src2 = regFile.getTag(i->src2);
		}

	} else {
		i->readySrc2 = true;
	}
	return 0;

}

int Processor::advanceCycle() {
	totalCycle++;
	if (myfile.eof() && robQ->size() == 0)
		return false;
	else
		return true;	
}
int Processor::fakeRetire() {
/*
        list<Instruction *>::iterator it;
        for(it = robQ->begin(); it != robQ->end() ; it++) {
                if( (*it)->getStage() == WB)
                        (*it)->incrDuration(WB);
        }
*/
#ifdef DEBUG
	cout << "After fake retire " << endl;
#endif
	robQ->cleanup();
}

int Processor::execute() {
	list<Instruction *>::iterator it;
	for(it = exeList->begin(); it != exeList->end() ; it++) {
		if( (*it)->getStage() == EX)
			(*it)->incrDuration(EX);
		else
			cout << "Error: Invalid state instruction is seen in execute List: " << (*it)->getStage() << endl ; 
	}	

	it = exeList->begin();
	int operType;
	while(  exeList->size() != 0 && it != exeList->end() ) {
		operType = (*it)->operType;
		
		if( execTime4Oper[ operType ] == (*it)->duration[EX] ) {
			(*it)->setStage(WB);
			(*it)->setBeginCycle(totalCycle,WB);
			(*it)->incrDuration(WB);
			wakeup(issueList,(*it)->dest,(*it)->tag);
		}

		it++;
	}
	exeList->cleanup();

#ifdef DEBUG
	cout << "After executing :" << endl;
	cout << "Dispatch list\n";
	printList(dispatchList);
	cout << "Issue list\n";
	printList(issueList);
	cout << "Exe list\n";
	printList(exeList);
	cout << "Reg File\n";
	regFile.print();
#endif
}

int Processor::issue() {
	int i=0;
	list<Instruction *>::iterator it;
	list<Instruction *> tempList;
	for(it = issueList->begin(); it!= issueList->end();it++) {
		if((*it)->readySrc1 == true && (*it)->readySrc2 == true)
			tempList.push_back( *it );
		
		if( (*it)->getStage() == IS ) 
			(*it)->incrDuration(IS);
		else
			cout << "Error: Invalid state instruction is seen in Issue List: " << (*it)->getStage() << endl ; 
			
	}
	tempList.sort(myfunc);
	//sort(tempList.begin(),tempList.end(),myfunc);

#ifdef DEBUG
	cout << "TempList size" << tempList.size() << endl;
	printList(&tempList);
#endif
	it = tempList.begin();
	while(i<numScalarWay && tempList.size()!=0 && !exeList->isFull() && it!= tempList.end() ) {
		(*it)->setStage(EX);
		(*it)->setBeginCycle(totalCycle,EX);
		
		exeList->push_back((*it));

		i++;
		it++;
	}	

	issueList->cleanup();

#ifdef DEBUG
	cout << "After issueing :" << endl;
	cout << "Dispatch list\n";
	printList(dispatchList);
	cout << "Issue list\n";
	printList(issueList);
	cout << "Exe list\n";
	printList(exeList);
#endif
}

int Processor::dispatch() {

	int i=0;
	list<Instruction *>::iterator it ;
	for(it = dispatchList->begin() ; it != dispatchList->end();it++) {
                if((*it)->getStage() == ID) {
                        (*it)->incrDuration(ID);
                } else if((*it)->getStage() == IF) {
                        (*it)->incrDuration(IF);
                } else {
			cout << "Error: Invalid state instruction is seen in Dispatch List: " << (*it)->getStage() << endl ; 
		}
        }


	it = dispatchList->begin();
	while(i < numScalarWay && dispatchList->size() != 0 && !issueList->isFull() && it!=dispatchList->end() ) {

		if( (*it)->getStage() == ID ) {
			renameSrcRegs( *it );
			(*it)->setStage(IS);
			(*it)->setBeginCycle(totalCycle,IS);
			issueList->push_back( *it );	

			if ( (*it)->dest != -1 ) {
#ifdef DEBUG
				cout << "Setting tag for " << (*it)->dest << " as " << (*it)->tag << endl;
#endif
				regFile.setTag((*it)->dest,(*it)->tag);			
				regFile.setReady( (*it)->dest,false);
			}

			i++;
		}

		it++;
	} 

	for(it = dispatchList->begin(),i=0 ; it != dispatchList->end() && i < numScalarWay ;it++) {
		if((*it)->getStage() == IF) {
			(*it)->setStage(ID);
			(*it)->setBeginCycle(totalCycle,ID);
			i++;
		}
	}

	dispatchList->cleanup();
#ifdef DEBUG
	cout << "After dispatching :" << endl;
	cout << "Dispatch list\n";
	printList(dispatchList);
	cout << "Issue list\n";
	printList(issueList);
#endif
}

int Processor::fetch( ) {

	string line;
	char address[9];
	int i=0;
	int operType,dest,src1,src2;	
	Instruction *instr;
	//if(myfile.is_open() ) {
		while( i < numScalarWay && !dispatchList->isFull() && getline(myfile,line) ) {
			
		//	cout << myfile.eof() << endl;
			stringstream str(line);

			str >> address;
			str >> operType;
			str >> dest;
			str >> src1;
			str >> src2;

#ifdef DEBUG
			//cout <<  "INS : " << totalIns << ": " <<  address << " " << operType << " " << dest << " " << src1 << " " << src2 << endl;
#endif


			instr = ins + robEntryIndex;
			instr->setParams(totalIns,operType,dest,src1,src2);
			instr->setStage(IF);
			instr->setBeginCycle(totalCycle,IF);
			//instr->incrDuration(IF);
	
			robQ->push_back(instr);		
			dispatchList->push_back(instr);
				
			i++;
			totalIns++;
			robEntryIndex = (robEntryIndex+1)%ROB_SIZE;
		}

#ifdef DEBUG
		cout << "Dispatch List After fetching : " << totalIns-1 << endl;
		printList(dispatchList);
#endif
		return 0;
/*	} else {
#ifdef DEBUG
		cout << "File is not open" << endl;
#endif

		return -1;
	}*/
}

int Processor::run( ) {

	do {
		fakeRetire();
		execute();
		issue();
		dispatch();
		fetch();

	} while ( advanceCycle());
/*
	fetch();
	
#ifdef DEBUG
	cout << "After fetching. Dispatch list\n";
	printList(dispatchList);
#endif

	dispatch();
	issue();

	totalCycle++;
	fetch();
	dispatch();
	issue();
	totalCycle++;
#ifdef DEBUG
	cout << "After dispatching .Dispatch List\n";
	printList(dispatchList);
	cout << "IssueList\n";
	printList(issueList);
	cout << "Exe List\n";
	printList(exeList);
	regFile.print();
#endif
*/
}

int Processor::wakeup(list<Instruction *> *li,int regNo,int tg ) {

	for( list<Instruction *>::iterator it = li->begin(); it != li->end() ; it++ ) {
		if( (*it)->readySrc1 == false && (*it)->src1 == tg )
			 (*it)->readySrc1 = true;

		if( (*it)->readySrc2 == false && (*it)->src2 == tg )
                         (*it)->readySrc2 = true;
	}

	if ( regFile.getTag(regNo) == tg )
		regFile.setReady(regNo,true);

}

int Processor::printStats() {
	cout << "CONFIGURATION\n";
	cout << " superscalar bandwidth (N) = " << numScalarWay << endl;
	cout << " dispatch queue size (2*N) = " << 2* numScalarWay << endl;
 	cout << " schedule queue size (S)   = " << schedQueueSize << endl;
	cout << "RESULTS\n";
	totalCycle--;
	cout << " number of instructions = " << totalIns << endl;
	cout << " number of cycles       = " << totalCycle << endl;
	cout << " IPC                    = " << (float)totalIns/totalCycle << endl;

}
int main(int argc,char *argv[]) {

	if(argc!=4) {
		cout << "USAGE : sim <S> <N> <trace_file>" << endl;
		exit(1);
	}

#ifdef DEBUG
	cout << "IF " << IF << " WB " << WB << endl;
#endif
	char *file = argv[3];
	int S = atoi(argv[1]);
	int N = atoi(argv[2]);

	Processor proc(ROB_SIZE, 2*N ,S , N , file);

	proc.run();
	proc.printStats();
//	proc.run("tmp");
	
	//Instruction *ins = new Instruction[ROB_SIZE];
}
