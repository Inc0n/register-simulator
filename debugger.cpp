
#include "debugger.hpp"
#include "assembler.hpp"

#include <istream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

// (private) helper function from assembler.cpp
bool nexttoken(istream &is, string &token);
vector<string> parseDotParam(string token, const char* syntax);
Param readParam(istream &is);
Instruction readInstruction(string cmd, istream &is);

// add breakpoint to the program
void breakpointCommand(istream &is, MIPS &mips) {
    string token;
    nexttoken(is, token);
	vector<string> tokens = parseDotParam(token, "<labelname>.<offsetIdex>");
    string labelname = tokens[0];
    uint offsetIdx = (uint)std::stoi(tokens[1]);
    mips.addbreakpoint(labelname, offsetIdx);
}

void stepCommand(istream& is, MIPS& mips) {
    bool wason = mips.flagonp(MIPSFlag::trace);
    // force trace instruction
    if (!wason)
        mips.setflag(MIPSFlag::trace);
    mips.executeNextInst();
    if (!wason)
        mips.unsetflag(MIPSFlag::trace);
}

// introspect memory and register
void readCommand(istream& is, MIPS& mips) {
    Param param = readParam(is);
    int_t val = mips.evalparam(param);
    std::cout << val << std::endl;
}

// return if false to stop repl
bool evaluateCmd(MIPS &mips, string line) {
    std::string cmd;
    std::stringstream ss(line);
    getline(ss, cmd, ' ');
    if (cmd == "exit" || cmd == "quit") {
        exit(1);
    }
    else if (cmd == "continue") return false; // return control back to machine
    else if (cmd == "breakpoint") breakpointCommand(cin, mips);
    else if (cmd == "step") stepCommand(cin, mips);
    else if (cmd == "read") readCommand(cin, mips);
    else {
        Instruction inst = readInstruction(cmd, cin);
        mips.evaluate(inst);
    }
    return true;
}

void repl(MIPS &mips) {
    string line;
    do {
        cout << "REPL> ";
        // read
        getline(cin, line);
        // getline(cin, cmd);
    } while (evaluateCmd(mips, line));
}
