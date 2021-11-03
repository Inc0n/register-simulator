#ifndef _ASSEMBLER_H
#define _ASSEMBLER_H

/* Assembler.hpp
   Defines the syntax of our machine language
   including the operator, and the params

   A operator looks like:
   <operator> <param1> <param2>
   param2 is optional for some operators

   A param looks like:
   <Param type>.<Param value>
 */

#include <vector>
#include <string>
#include <istream>
#include "instruction-set.hpp"

// type definition aliases
typedef unsigned int uint;
typedef std::vector<Instruction> Insts;

// A code segment between labels
struct Segment {
	uint idx;          // segment pc index from 0, to be calculated during assembly
    std::string label; // segment label
	Insts insts;       // the instructions in this segment
    // constructor
    Segment(std::string label, Insts insts) :
        idx(0),
        label(label),
        insts(insts) {}
};

// alias
typedef std::vector<Segment> Segments;

// segments related fns
Segment* findSegment(Segments segments, std::string label);
Segments assemble(std::istream &is);
Insts segmentsToCode(Segments segments);

// machine code
Insts readCodeFromStream(std::istream &s);
std::vector<int_t> encodeParam(const Param &p);
std::vector<int_t> encodeInstruction(const Instruction &i);

// Operator, opcode

struct operator_t {
    std::string name;
	INST inst;
    uint numparams;
	// Instruction (*reader)(std::istream &is, INST inst);
};

const operator_t* getOperator(INST inst);
const operator_t* getOperator(std::string name);

// Operator definitions
const std::vector<operator_t> operators = {
	{"mov",		INST::MOV,	 2},
	{"add",		INST::ADD,	 2},
	{"minus",	INST::MINUS, 2},
	{"shift",	INST::SHIFT, 2},
	{"load",	INST::LOAD,  2},
	{"store",	INST::STORE, 2},
	{"test",	INST::TEST,	 1},
	{"jump",	INST::JUMP,	 1},
	{"branch", INST::BRANCH, 1},
    {"push",    INST::PUSH,  1},
    {"pop",     INST::POP,   1},
    {"nop",     INST::NOP,   0}
};

#endif // _ASSEMBLER_H