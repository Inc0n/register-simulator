#ifndef _INSTRUCTION_SET_H
#define _INSTRUCTION_SET_H

/* instruction-set.hpp
   Defines the underlying c structure of the syntax of the language

   An operative instruction or operator can have up to two params.
   A param is tag typed and has a value.

   A label param is to be assembled and converted to labelIdx as defined in assembler.cpp
 */

#include <assert.h>
#include <string.h>
#include <string>
#include <vector>
#include <iostream>

// first register is usually the output

enum class INST {
	MOV = 0, // mov register1 register2
	ADD,     // add register1 register2
	MINUS,	 // minus register1 register2
	SHIFT,	 // shift register1 num
	LOAD,    // load register1 <param2>
	STORE,   // store register1 <param2>
	TEST,	 // test register register0
	JUMP,	 // jump place register0
	BRANCH,  // branch place register0
	PUSH,    // push register
	POP,     // pop register
	NOP      // NOP
};

enum class tagtype {
	Const = 0, // immediate
	Mem,
	Reg,
	Label,    // temporary holder for label name during assemble
	LabelIdx, // assembled label index
	Nil       // used to indicate empty param
};

// tagtype parseParamTagtype(std::string token);

// my custom int type, should be 4 bytes
typedef unsigned int_t;

struct Param {
private:
	tagtype type;
	union {
		int_t value = 0;
		char* label;
	};
public:
    Param();
	// constructors
	Param(tagtype type, const char* label);
	Param(tagtype type, int_t value);
	// accessors
	tagtype gettype() const { return type; }
	int_t getvalue() const {
		assert(type != tagtype::Label);
		return value;
	}
	char* getlabel() const {
		assert(type == tagtype::Label);
		return label;
	}
	size_t size();
	bool operator==(const Param& p) const;
};

struct Instruction {
// private:
	INST inst;
    std::vector<Param> params;
// public:
    INST getInstCode() const { return inst; }
	//
	Instruction();
	Instruction(INST inst, Param param1);
    Instruction(INST inst, Param param1, Param param2);
    Instruction(INST inst, std::vector<Param> params);
    //
	size_t size() { return 1 + params[0].size() + params[1].size(); };
	bool operator==(const Instruction& inst) const;
};

// output as string interface
std::ostream& operator<<(std::ostream& os, const Param& p);
std::ostream& operator<<(std::ostream& os, const Instruction& i);

#endif // _INSTRUCTION_SET_H
