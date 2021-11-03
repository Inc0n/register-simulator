#ifndef _MIPS_H
#define _MIPS_H

#include <string>
#include <vector>
#include <stack>
#include "instruction-set.hpp"
#include "assembler.hpp"

// expects exp to be "true" otherwise error msg will be thrown
#define expect(exp, ...) {					\
		if (!(exp)) {							\
			printf(__VA_ARGS__);			\
			exit(1);							\
		}										\
	}

// throws the error with error code
#define error(code,msg,...) {					\
		printf(msg,__VA_ARGS__);				\
		exit(code);								\
}

#define NUMREGISTERS 32
#define NUMMEMORIES 1024

struct Breakpoint {
    Segment* segment; // meta info on breakpoint
    uint offSetIdx;   // index from label info
    uint pc;          // cached actual pc value to breakon
};

// The debug flags implemented in this machine
enum class MIPSFlag {
	trace = 0b001, // tracing instructions + label
	print = 0b010, // print out changes of instructions
    debug = 0b100, // attach machine to debugger
};

struct MIPS {
public:
	Segments segments;
	std::stack<int_t> stack;
	std::vector<Instruction> code;
    //
    uint getCount() { return this->count; }
    int_t getMem(uint idx);
    int_t getReg(uint idx);
	void setReg(uint idx, int_t val);
	void setMem(uint idx, int_t val);
	void assign(Param p, int_t val);
	//
	bool flagonp(MIPSFlag f) { return (flags & (int)f) == (int)f; }
    void unsetflag(MIPSFlag f) { flags &= ~(int)f; }
    void setflag(MIPSFlag f) { flags |= (int)f; }
	//
	MIPS();
	void reset();
    void addbreakpoint(std::string labelname, int_t idx);
    // TODO: Implement
    // void removebreakpoint(std::string labelname, int_t idx);
	//
	void evaluate(Instruction &inst);
	int_t evalparam(Param p);
	//
    void executeNextInst();
	void execute();
	// tool chain
    void assembleCode(std::istream &is);
	void assembleCode(std::string text);
private:
	int_t pc = 0;   // program counter
	int_t flag = 0; // machine execution jump flag
	char flags = 0; // MIPS flag
    uint count = 0; // instruction executed count
    //
	std::vector<int_t> registers; // 32 registers
	std::vector<int_t> memory; // 1024 memory blocks
    std::vector<Breakpoint> breakpoints;
	//
	void evalMov(std::vector<Param> args);
	void evalAdd(std::vector<Param> args);
	void evalMinus(std::vector<Param> args);
	void evalShift(std::vector<Param> args);
    void evalLoad(std::vector<Param> args);
    void evalStore(std::vector<Param> args);
	void evalTest(std::vector<Param> args);
	void evalJump(std::vector<Param> args);
	void evalBranch(std::vector<Param> args);
    void evalPush(std::vector<Param> args);
    void evalPop(std::vector<Param> args);
};

#endif // _MIPS_H#include <vector>