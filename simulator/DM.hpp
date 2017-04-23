#include "define.hpp"

inline void DM() {
	data_memory();
}

// input: MemRead, ALU_result, regi_output2
// output: DM_result
inline void data_memory() {
	if (exdmn.MemRead == 111) {
		if (exdmn.ALU_result < 1024 && exdmn.ALU_result >= 0)
			dmwb.DM_result = memd2reg1(exdmn.ALU_result);
		else 
			errorflag_memoryOverflow = true;
	} else if (exdmn.MemRead == 112) {
		if (exdmn.ALU_result %2 != 0) errorflag_missingAlign = true;
		if (exdmn.ALU_result < 1023 && exdmn.ALU_result >= 0) 
			dmwb.DM_result = memd2reg2(exdmn.ALU_result);
		else
			errorflag_memoryOverflow = true;
	} else if (exdmn.MemRead == 114) {
		if (exdmn.ALU_result %4 != 0) errorflag_missingAlign = true;
		if (exdmn.ALU_result < 1021 && exdmn.ALU_result >= 0)
			dmwb.DM_result = memd2reg4(exdmn.ALU_result);
		else
			errorflag_memoryOverflow = true;
	} else if (exdmn.MemRead == 121) {
		if (exdmn.ALU_result < 1024 && exdmn.ALU_result >= 0)
			dmwb.DM_result = memd2reg1(exdmn.ALU_result) & 0x000000ff;
		else
			errorflag_memoryOverflow = true;
	} else if (exdmn.MemRead == 122) {
		if (exdmn.ALU_result %2 != 0) errorflag_missingAlign = true;
		if (exdmn.ALU_result < 1023 && exdmn.ALU_result >= 0)
			dmwb.DM_result = memd2reg2(exdmn.ALU_result) & 0x0000ffff;
		else
			errorflag_memoryOverflow = true;
	} else {
		dmwb.DM_result = 0;
	}
	if (exdmn.MemWrite == 211) {
		if (exdmn.ALU_result < 1024 && exdmn.ALU_result >= 0)
			memd[exdmn.ALU_result]   =  exdmn.regi_output2      & 0x000000ff;
		else
			errorflag_memoryOverflow = true;
	} else if (exdmn.MemWrite == 212) {
		if (exdmn.ALU_result %2 != 0) errorflag_missingAlign = true;
		if (exdmn.ALU_result < 1023 && exdmn.ALU_result >= 0) {
			memd[exdmn.ALU_result  ] = (exdmn.regi_output2>> 8) & 0x000000ff;
			memd[exdmn.ALU_result+1] = (exdmn.regi_output2    ) & 0x000000ff;
		} else {
			errorflag_memoryOverflow = true;
		}
	} else if (exdmn.MemWrite == 214) {
		if (exdmn.ALU_result %4 != 0) errorflag_missingAlign = true;
		if (exdmn.ALU_result < 1021 && exdmn.ALU_result >= 0) {
			memd[exdmn.ALU_result  ] = (exdmn.regi_output2>>24) & 0x000000ff;
			memd[exdmn.ALU_result+1] = (exdmn.regi_output2>>16) & 0x000000ff;
			memd[exdmn.ALU_result+2] = (exdmn.regi_output2>>8 ) & 0x000000ff;
			memd[exdmn.ALU_result+3] = (exdmn.regi_output2    ) & 0x000000ff;
		} else {
			errorflag_memoryOverflow = true;
		}
	}
}
