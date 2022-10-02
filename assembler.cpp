
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
#include <ctype.h>
#include <iostream>
#include <istream>
#include <algorithm>
#include <sstream>

#include "assembler.hpp"
#include "mips.hpp"

#define debugLog cout

using namespace std;

const operator_t* getOperator(INST inst) {
    for (const operator_t& op: operators) {
        if (op.inst == inst)
            return &(op);
    }
    return NULL;
}
const operator_t* getOperator(string token) {
    for (const operator_t& op: operators) {
        if (op.name == token)
            return &(op);
    }
    return NULL;
}

// syntax definitions
bool labelp(string token) {
	return token.back() == ':';
}
bool commentp(string token) {
    return token.length() > 0 and token[0] == '#';
}

void skipwhitespace(istream &is) {
    while (is.peek() == ' '
           || is.peek() == '\t')
        is.get();
}

bool nexttoken(istream &is, string &token) {
	char c;
	token = ""; // reset token to empty string

    skipwhitespace(is);

	while ((c = is.peek()) != EOF) {
		if (c == ' ' || c == '\n') {
			is.get(c); // consume the delim
			break;
		}
		token += c;
		is.get(c); // consume char
	}
	// cout << "read: " << token << endl;
	return c != EOF;
}

bool peektoken(istream &is, string &token) {
	streampos sp = is.tellg();

    // prevent cout
    // cout.setstate(ios_base::failbit);
	bool ret = nexttoken(is, token);
    // cout.clear();

	is.seekg(sp);
	return ret;
}

// util string split fn

vector<string> split(string text, char delim) {
	string tmp;
	vector<string> strs;
	stringstream ss(text);
	while (getline(ss, tmp, delim)) {
		strs.push_back(tmp);
	}
	return strs;
}

// tagtype parse definition

tagtype parseParamTagtype(string token) {
    transform(token.begin(), token.end(), token.begin(), ::tolower);
	if (token == "const") return tagtype::Const;
	if (token == "mem")   return tagtype::Mem;
	if (token == "reg")   return tagtype::Reg;
	if (token == "label") return tagtype::Label;
	if (token == "nil")   return tagtype::Nil;
    std::cout << "invalid param tag type: " << token << std::endl;
    exit(1);
}

//

// int_t parseRegValue(string token) {
//     if (token == "ret") return 0;
//     if (token == "continue") return 31;
//     int_t val = std::stoi(token);
//     return val;
// }

int_t parseParamValue(string token, int_t limit, const char* errmsg) {
    int_t val = stoi(token);
    expect(val < limit, errmsg, val);
	return val;
}

vector<string> parseDotParam(string token, const char* syntax) {
	vector<string> tokens = split(token, '.');
	expect(tokens.size() == 2,
		   "invalid param syntax (%s) \"%s\"\n", syntax, token.c_str());
    return tokens;
}

Param parseParam(string token) {
	// <ParamType>.<value>
	vector<string> tokens = parseDotParam(token, "<ParamType>.<value>");
	tagtype type = parseParamTagtype(tokens[0]);
	if (type == tagtype::Label)
		return Param(type, tokens[1].c_str());
	else if (type == tagtype::Reg) {
		return Param(type, parseParamValue(tokens[1], 32, "register %d out of range!"));
    }
	else {
        int_t val = stoi(tokens[1]);
		return Param(type, val);
    }
}

Param readParam(istream &is) {
	string token;
	nexttoken(is, token);
    debugLog << "read param: " << token << endl;
	return parseParam(token);
}

Instruction readNInstruction(istream &is, INST inst, uint n) {
    expect(n <= 2, "reading instruction having more than two parameters");
    Param params[2];
    for (int i = 0; i < n; ++i) {
        params[i] = readParam(is);
    }
	return Instruction(inst, params[0], params[1]);
}

//
Instruction readInstruction(string cmd, istream &is) {
	const operator_t* op = getOperator(cmd);
	expect(op != NULL, "expecting an operator token but got: %s\n", cmd.c_str());
    debugLog << "read Inst: " << cmd << endl;
    return readNInstruction(is, op->inst, op->numparams);
}

// read instructions until encountering a label (label token not consumed)
Insts readUntilLabel(istream &is) {
	Insts insts;
	string token;

	while (peektoken(is, token)
		   && !labelp(token)) {
        if (commentp(token) || token.length() == 0) {
            // consume the whole line
            getline(is, token, '\n');
            debugLog << "comment: " << token << endl;
        } else {
            nexttoken(is, token);
            insts.push_back(readInstruction(token, is));
        }
	}
	if (insts.size() == 0)
        insts.push_back(Instruction());
    return insts;
}

Segment* findSegment(Segments segments, string label) {
    for (Segment& s: segments) {
        if (s.label == label)
            return &(s);
    }
    return NULL;
}

string readUntilFirstLabel(istream &is) {
	string token;

	while (peektoken(is, token)
		   && !labelp(token)) {
        if (commentp(token) || token.length() == 0) {
            // consume the whole line
            getline(is, token, '\n');
            debugLog << "comment: " << token << endl;
        } else {
            // encountered non-comment, non-label token
            // nexttoken(is, token);
            return ":"; // unnamed label
        }
	}
    nexttoken(is, token); // consume token
    return token;
}

Segments readSegments(istream &is) {
	Segments segments;
	string label = readUntilFirstLabel(is); // unnamed label

	// read all of the assembly (code)
    // only loop while there's tmp left in the stream
	while (label.length() > 0) {
        expect(labelp(label),
               "expecting a labelname but got %s\n",
               label.c_str());
        // remove last : from label string
        label = label.substr(0, label.length() - 1);
        debugLog << "Segment label: " << label << endl;

        expect(findSegment(segments, label) == NULL,
               "label %s name already exist\n",
               label.c_str());
        Segment s = Segment(label, readUntilLabel(is));
        segments.push_back(s);
        nexttoken(is, label); // update label
	}
	return segments;
}

// convert tagtype::Label -> tagtype::LabelIdx
void parseIfLabelParam(Instruction &inst, uint i, const Segments &segments) {
	const Param &p = inst.params[i];
	if (p.gettype() == tagtype::Label) {
		Segment* seg = findSegment(segments, p.getlabel());
		expect(seg != NULL,
               "label %s not found\n",
               p.getlabel());
		inst.params[i] = Param(tagtype::LabelIdx, seg->idx);
	}
}

// compute the label PC indexes
Segments parselabels(Segments segments) {
	// EncodedInst einst = (EncodedInst) {inst, encodeInstruction(inst)};
	uint idx = 0;
	for (Segment &s: segments) {
		s.idx = idx;
		// new index
		idx += s.insts.size();
	}
	for (Segment &s: segments) {
		for (Instruction &inst: s.insts) {
			switch (inst.getInstCode()) {
			case INST::MOV:
            case INST::STORE:
				parseIfLabelParam(inst, 1, segments);
				break;
			case INST::JUMP:
			case INST::BRANCH:
			case INST::PUSH:
				parseIfLabelParam(inst, 0, segments);
				break;
			case INST::ADD:
			case INST::MINUS:
			case INST::SHIFT:
            case INST::LOAD:
			case INST::TEST:
			case INST::POP:
			case INST::NOP:
				break;
			}
		}
	}
	return segments; // parsed segments
}

// main assembly function
Segments assemble(istream &is) {
    return parselabels(readSegments(is));
}

// get rid of labels
// and concatenate all of the instructions
Insts segmentsToCode(Segments segments) {
	vector<Instruction> code;
	for (Segment &s: segments) {
		Insts insts = s.insts;
		code.insert(code.end(), insts.begin(), insts.end());
	}
    return code;
}

// read machine code instruction
Param readParamCode(istream &s) {
    int_t tag, val;
    s.read((char*)&tag, sizeof(int_t));
    s.read((char*)&val, sizeof(int_t));
    return Param((tagtype)tag, val);
}

Instruction readInstructionCode(istream &s) {
    int_t opcode;
    s.read((char*)&opcode, sizeof(int_t));
    const operator_t *op = getOperator((INST)opcode);
    if (op == NULL)
        debugLog <<  "cannot find instruction with opcode " << opcode << endl;
    // expect(op != NULL,
    const uint numparams = op != NULL ? op->numparams : 0;
    Param param1 = numparams > 0 ? readParamCode(s) : Param();
    Param param2 = numparams > 1 ? readParamCode(s) : Param();
    return Instruction((INST)opcode, param1, param2);
}

// read machine code from stream
Insts readCodeFromStream(istream &s) {
    Insts insts;
    while (!s.eof()) {
        insts.push_back(readInstructionCode(s));
    }
    return insts;
}

// machine code
std::vector<int_t> encodeParam(const Param &p) {
	switch (p.gettype()) {
	case tagtype::Nil:
		return {};
	case tagtype::Label:
        // maybe throw error?
		return {(int_t)p.gettype(), 0};
	default:
		return {(int_t)p.gettype(), p.getvalue()};
	}
}


std::vector<int_t> encodeInstruction(const Instruction &i) {
    INST inst = i.getInstCode();
	// const operator_t* op = getOperator(inst);
    std::vector<int_t> encode = {(int_t)inst};
	std::vector<int_t> param = encodeParam(i.params[0]);
	encode.insert(encode.end(), param.begin(), param.end());
	//
	param = encodeParam(i.params[1]);
	encode.insert(encode.end(), param.begin(), param.end());
	return encode;
}
