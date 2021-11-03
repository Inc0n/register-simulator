
#include "mips.hpp"
#include "assembler.hpp"
#include "debugger.hpp"

#include <iostream>
#include <stack>
#include <cassert>
#include <cstdio>
#include <sstream>
#include <fstream>
// #include <algorithm>

using namespace std;

MIPS::MIPS() :
	registers(vector<int_t>(NUMREGISTERS,0)),
	memory(vector<int_t>(NUMMEMORIES, 0)),
    stack(std::stack<int_t>()),
	code(vector<Instruction>()),
    breakpoints(vector<Breakpoint>()),
	pc(0)
{}

Param readParam(ifstream &file) {
	char c;
	file.read(&c, 1);
	//
	char num[4];
	file.read(num, 4);
	return Param((tagtype)c, reinterpret_cast<const int_t &>(num));
	// p.value = reinterpret_cast<const int_t &>(num);
}

Instruction readInst(ifstream &file) {
	char c;
	file.read(&c, 1);
	return Instruction((INST)c, readParam(file), readParam(file));
}

//

int_t MIPS::evalparam(Param p) {
	switch (p.gettype()) {
	case tagtype::Const:
    case tagtype::LabelIdx:
        return p.getvalue();
	case tagtype::Mem:
        return memory[p.getvalue()];
	case tagtype::Reg:
        return registers[p.getvalue()];
	case tagtype::Label:
	case tagtype::Nil:
	default:
		error(2,"unexpected param type %d", p.gettype());
	}
}


bool atBreakpointp(vector<Breakpoint> breakpoints, uint pc) {
    for (const Breakpoint& b : breakpoints) {
        if (b.pc == pc) return true;
    }
    return false;
}

void MIPS::executeNextInst() {
    Instruction &inst = this->code[this->pc];
    if (this->flagonp(MIPSFlag::trace)) {
        // TODO: trace label
        cout << "exec: " << inst << endl;
    }
    this->count += 1;
    this->evaluate(inst);
}

void MIPS::execute() {
    this->count = 0;
	while (this->pc < this->code.size()) {
        if (this->flagonp(MIPSFlag::debug)) {
            if (atBreakpointp(breakpoints, (uint)pc)) {
                repl(*this);
            }
        }
        executeNextInst();
	}
}

void MIPS::reset() {
	this->count = 0;
	this->flag = 0;
	std::fill(registers.begin(), registers.end(), 0);
	std::fill(memory.begin(), memory.end(), 0);
}


void MIPS::addbreakpoint(string labelname, int_t idx) {
    Segment* seg = findSegment(segments, labelname);
    if (seg != NULL)
        breakpoints.push_back((Breakpoint) { seg, idx, idx + seg->idx });
    else
        cout << "Can't find " << labelname << " in the program" << endl;
}

// accessors with guards
int_t MIPS::getMem(uint idx) {
    expect(idx < memory.size(),
           "memory address out of bound %d, enable trace to see instruction of cause",
           idx);
    return this->memory[idx];
}

int_t MIPS::getReg(uint idx) {
    expect(idx < registers.size(),
           "memory address out of bound %d, enable trace to see instruction of cause",
           idx);
    return this->registers[idx];
}

// setter with guards
void MIPS::setReg(uint idx, int_t val) {
	if (flagonp(MIPSFlag::print)) {
		int_t before = registers[idx];
		cout << "reg." << idx << " was: " << before << ", now: " << val << endl;
	}
    expect(idx < registers.size(),
           "register number out of bound, enable print to see instruction of cause");
	registers[idx] = val;
}

void MIPS::setMem(uint idx, int_t val) {
	if (flagonp(MIPSFlag::print)) {
		int_t before = memory[idx];
		cout << "mem." << idx << " was: " << before << ", now: " << val << endl;
	}
    expect(idx < memory.size(),
           "memory addres sout of bound, enable print to see instruction of cause");
	memory[idx] = val;
}

// setter for param interface
void MIPS::assign(Param p, int_t val) {
	switch (p.gettype()) {
	case tagtype::Reg:
		setReg(p.getvalue(), val);
		break;
	case tagtype::Mem:
		setMem(p.getvalue(), val);
		break;
	default:
		error(1, "assign can't have param (%d . %d) as first param",
			  p.gettype(), p.getvalue());
	}
}

void MIPS::evalMov(vector<Param> args) {
	int_t val = evalparam(args[1]);
	Param target = args[0];
	assign(target, val);
	this->pc++;
}

void MIPS::evalAdd(vector<Param> args) {
	Param target = args[0];
	int_t val = evalparam(args[0]) + evalparam(args[1]);
	assign(target, val);
	this->pc++;
}

void MIPS::evalMinus(vector<Param> args) {
	Param target = args[0];
	int_t val = evalparam(args[0]) - evalparam(args[1]);
	assign(target, val);
	this->pc++;
}

void MIPS::evalShift(vector<Param> args) {
	Param target = args[0];
	int_t val = evalparam(args[0]);
	signed num = (signed)evalparam(args[1]);
    int_t newval = (num > 0) ? val << num: val >> (-num);
	assign(target, newval);
	this->pc++;
}

void MIPS::evalLoad(vector<Param> args) {
	int_t regidx = evalparam(args[0]);
	int_t memidx = evalparam(args[1]);
    int_t val = getMem(memidx);
    setReg(regidx, val);
	this->pc++;
}


void MIPS::evalStore(vector<Param> args) {
	int_t idx = evalparam(args[0]);
	int_t val = evalparam(args[1]);
    setMem(idx, val);
	this->pc++;
}

void MIPS::evalTest(vector<Param> args) {
	// ignore args[1] !!!
    int_t flag = evalparam(args[0]);
	this->flag = (signed)flag;
	this->pc++;
}

void MIPS::evalJump(vector<Param> args) {
	// ignore args[1] !!!
	int_t dest = evalparam(args[0]);
	this->pc = dest;
}

void MIPS::evalBranch(vector<Param> args) {
	// ignore args[1] !!!
	if (this->flag) evalJump(args);
	else this->pc++;
}

void MIPS::evalPush(vector<Param> args) {
	// ignore args[1] !!!
    int_t val = evalparam(args[0]);
    stack.push(val);
	this->pc++;
}

void MIPS::evalPop(vector<Param> args) {
	// ignore args[1] !!!
    Param& p = args[0];
    int_t val = this->stack.top();
    setReg(p.getvalue(), val);
    this->stack.pop();
	this->pc++;
}

// evaluate instruction
void MIPS::evaluate(Instruction &inst) {
    const vector<Param> &params = inst.params;

	switch (inst.getInstCode()) {
	case INST::MOV: return evalMov(params);
	case INST::ADD: return evalAdd(params);
	case INST::MINUS: return evalMinus(params);
	case INST::SHIFT: return evalShift(params);
	case INST::STORE: return evalStore(params);
	case INST::LOAD: return evalLoad(params);
	case INST::TEST: return evalTest(params);
	case INST::JUMP: return evalJump(params);
	case INST::BRANCH: return evalBranch(params);
	case INST::PUSH: return evalPush(params);
	case INST::POP: return evalPop(params);
    case INST::NOP: this->pc += 1; // just increment the counter by 1
	}
}

// assembly

void MIPS::assembleCode(istream &is) {
    this->segments = assemble(is);
    this->code = segmentsToCode(this->segments);
}

void MIPS::assembleCode(string text) {
	stringstream ss(text);
	this->assembleCode(ss);
}