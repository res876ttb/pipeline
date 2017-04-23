#include "define.hpp"

inline void IF() {
	command = memi[PC / 4];
	decoder();
	if (ifid.opcode == HALT) {
		ifid.halt = true;
	} else if ((command & 0xfc1fffff) == 0) {
		PC += 4;
		print_diff();
	}
}

// input: command
// output: opcode, r2521, r2016, t1511, r1006, r2500, r1500, r0500
inline void decoder() {
	ifid.opcode = scan_command(31, 26);
	ifid.r2521  = scan_command(25, 21);
	ifid.r2016  = scan_command(20, 16);
	ifid.r1511  = scan_command(15, 11);
	ifid.r1006  = scan_command(10,  6);
	ifid.r2500  = scan_command(25,  0);
	ifid.r1500  = scan_command(15,  0);
	ifid.r0500  = scan_command( 5,  0);
}
