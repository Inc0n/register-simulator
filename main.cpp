
#include "mips.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{
    // MIPS m;
    std::fstream fs;

    // get code file
    fs.open("showcase.prog", std::fstream::in);
    std::cout << "processing file" << std::endl;
    Segments ss = assemble(fs);
    Insts insts = segmentsToCode(ss);
    // m.assembleCode(fs);
    fs.close();

    std::ofstream fout("data.dat", ios::out | ios::binary);

    std::vector<int_t> encoded;
	for (Instruction &i: insts) {
        cout << i << "\t\t";
        auto code = encodeInstruction(i);
        cout << code.size() << endl;
		encoded.insert(encoded.end(), code.begin(), code.end());

        // for (int_t x: code) {
        //     printf("%04x", x);
        //     // cout << x;
        // }
	}
    cout << encoded.size() << endl;
    fout.write((char*)&encoded[0], encoded.size() * sizeof(int_t));
    fout.close();

    std::fstream fin("data.dat", ios::in | ios::binary);
    insts = readCodeFromStream(fin);
    for (Instruction inst: insts) {
        cout << inst << endl;
    }
    return 0;
}
