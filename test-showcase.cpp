
#include "mips.hpp"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
    MIPS m;
    std::fstream fs;

		if (argc < 2) {
				std::cout << "Pass in a program file to run!";
				exit(1);
		}

		string program = argv[1]; // "showcase.prog"

    // get code file
    fs.open(program, std::fstream::in);
    std::cout << "processing file" << std::endl;
    m.assembleCode(fs);
    fs.close();

    // debug setup
    // m.setflag(MIPSFlag::trace);
    //// m.setflag(MIPSFlag::print);
    m.setflag(MIPSFlag::debug);
    // m.addbreakpoint("square-aux", 0);

    // exec mips
    m.execute();

    std::cout << sizeof(INST) << " " << sizeof(tagtype) << std::endl;

    // std::cout << "result: " << m.getReg(2) << std::endl;
    std::vector<uint> squares(100, 0);
    for (int i = 0; i < 100; ++i) {
        squares[i] = i * i;
    }
    for (int i = 0; i < 5; ++i) {
        std::cout << "result: " << m.getMem(i) << std::endl;
    }

    for (int i = 0; i < 100; ++i) {
        unsigned int square = i * i;
        if (square != m.getReg(i))
            printf("unmatched square %d %d expected: %d", i, m.getReg(i), square);
        // TODO: uncomment this
        // CHECK_EQUAL(squares[i], m.getReg(i));
    }
    std::cout << " instruction used " << m.getCount();
    return 0;
}
