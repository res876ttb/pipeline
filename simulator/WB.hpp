#include "define.hpp"

inline void WB() {
	regi_write();
}

// input: RegDst, RegWrite, MemRead, DM_result, ALU_result, opcode, r2016, r1511, r0500
inline void regi_write() {
	if (dmwbn.RegWrite) {
		regi[dmwbn.RegNum] = (dmwbn.MemRead) ? dmwbn.DM_result : dmwbn.ALU_result;
		if (dmwbn.opcode == 0 && dmwbn.r0500 == MFHI) {
			regi[dmwbn.RegNum] = HI;
			moveflag_HI = true;
		}
		if (dmwbn.opcode == 0 && dmwbn.r0500 == MFLO) {
			regi[dmwbn.RegNum] = LO;
			moveflag_LO = true;
		}
		if (dmwbn.RegNum == 0) {
			errorflag_write0 = true;
			regi[0] = 0;
		}
	}
}
