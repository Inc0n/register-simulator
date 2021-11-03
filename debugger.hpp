#ifndef _DEBUGGER_H
#define _DEBUGGER_H

/* debugger.hpp
   The cmd repl style interface for debugging the program.
   It's used to drop into when the program has hit a breakpoint.
 */

#include "mips.hpp"

void repl(MIPS& mips);

#endif // _DEBUGGER_H