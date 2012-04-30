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
int execTime4Oper[3] = {1,2,5};

class Instruction;

int printList( list<Instruction *> *li ) ;

class myqueue : public queue<Instruction *> {
        int len;
public:
        myqueue(int s);
        bool isFull();

};

class mylist : public list<Instruction *> {
        int len;
public:
        mylist(int s);
        bool isFull();
        virtual int cleanup() = 0 ;
};

class DispatchList : public mylist {

public:
        DispatchList(int s) : mylist(s) {}
        bool static removeCondition(Instruction *in );
        int cleanup();

};

class IssueList : public mylist {
public:
        IssueList(int s) : mylist(s) {}
        bool static removeCondition(Instruction *in );
        int cleanup();

};

class ExeList : public mylist {
public:
        ExeList(int s) : mylist(s) {}
        bool static removeCondition(Instruction *in );
        int cleanup();

};



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


class Processor;
class Instruction {
        int tag;
        int operType;
        int dest;
        int src1;
        int src2;
        bool readySrc1;
        bool readySrc2;
        pipe_stage_t curStage;
        int bCycle[NUM_STAGES]; // begin cycle time
        int duration[NUM_STAGES];
        friend class Processor;

public:
        Instruction();
        int setStage(pipe_stage_t state);
        pipe_stage_t getStage();
        int setParams(int tg,int optype,int dst,int s1,int s2,bool readyS1,bool readyS2);
        int resetParams();
        int setBeginCycle(int ct,pipe_stage_t state);
        int incrDuration( pipe_stage_t state);
        bool isDone();
        int print();

};

class Processor {
	int totalIns;
	int totalCycle;
	int numScalarWay;
	int robEntryIndex;
	RegFile regFile;
	Instruction *ins;
	myqueue *robQ;
	DispatchList *dispatchList;
	IssueList *issueList;
	ExeList *exeList;

public:
	Processor(int ROB_size,int disList_size,int issueList_size,int exeList_size);
	int renameSrcRegs(Instruction *i);
	int fakeRetire();
	int execute() ;
	int issue() ;
	int dispatch();
	int fetch(ifstream &myfile) ;
	int run(char *fileName);
			
};
