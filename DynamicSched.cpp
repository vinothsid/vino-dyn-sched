#include <iostream>
#include <fstream>
#include <sstream>
#include "string.h"
#include <iomanip>
#include "stdlib.h"
#include <algorithm>
#define NUM_REGS 128

#define DEBUG 1

using namespace std;

enum pipe_stage_t {IF,ID,IS,EX,WB,NUM_STAGES,INVALID_STAGE};
class Instruction {
	int tag;
	char operType;
	char dest;
	char src1;
	char src2;
	pipe_stage_t curStage;
	int bCycle[NUM_STAGES]; // begin cycle time
	int duration[NUM_STAGES];
	
public:
	Instruction();
	int setStage(pipe_stage_t state);
	int setParams(int tg,char optype,char dst,char s1,char s2);
	int resetParams();
	int setBeginCycle(int ct);
	int incrDuration( pipe_stage_t state);
	bool isDone();
	int renameSrcRegs();
};

class RegFile {
	char ready[NUM_REGS];
	int tag[NUM_REGS];
public:
	RegFile();
	bool isReady(int regNo);
	int setTag(int regNo,int tg);
	int getTag(int regNo);
	int setReady(int regNo);

};

class Processor {

};

int main(int argc,char *argv[]) {

#ifdef DEBUG
	cout << "IF " << IF << " WB " << WB << endl;
#endif


}
