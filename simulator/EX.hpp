#include "define.hpp"

inline void EX() {
	forwarding_EX();
	
	ALU();
}

inline void forwarding_EX() {
	EXexdmf = EXdmwbf = false;
	EXrsf   = EXrtf   =     0;
	// forwarding condition: current instruction will need the calculate result
	// i type forwarding
	if ( idexn.opcode        && 
	    !idexn.MemWrite      && 
	    !idexn.MemRead       &&
		 idexn.opcode != LUI && 
		 idexn.opcode != BEQ && 
		 idexn.opcode != BNE && 
		 idexn.opcode != BGTZ) { 
		// DM to EX
		if (idexn.r2521 == exdmn.RegNum) {
			EXexdmf = true;
			EXrsf   = idexn.r2521;
			idexn.regi_output1 = exdmn.ALU_result;
		}
		// WB to EX
		if (idexn.r2521 == dmwbn.RegNum) {
			EXdmwbf = true;
			EXrsf   = idexn.r2521;
			idexn.regi_output1 = dmwbn.DM_result;
		}
	} 
	// r type forwarding
	else if (!idexn.opcode) {
		// add to slt, mult, multu
		if (idexn.r0500 >= 0x18) {
			// DM to EX
			if (idexn.r2521 == exdmn.RegNum) {
				EXexdmf = true;
				EXrsf   = idexn.r2521;
				idexn.regi_output1 = exdmn.ALU_result;
			}
			if (idexn.r2016 == exdmn.RegNum) {
				EXexdmf = true;
				EXrtf   = idexn.r2016;
				idexn.regi_output2 = exdmn.ALU_result;
			}
			// WB to EX
			if (idexn.r2521 == dmwbn.RegNum) {
				EXdmwbf = true;
				EXrsf   = idexn.r2521;
				idexn.regi_output1 = dmwbn.DM_result;
			} 
			if (idexn.r2016 == dmwbn.RegNum) {
				EXdmwbf = true;
				EXrtf   = idexn.r2016;
				idexn.regi_output2 = dmwbn.DM_result;
			}
		}
		// sll, srl, sra
		else if (idexn.r0500 <= 0x03) {
			// DM to EX
			if (idexn.r2016 == exdmn.RegNum) {
				EXexdmf = true;
				EXrtf   = idexn.r2016;
				idexn.regi_output2 = exdmn.ALU_result;
			}
			// WB to EX
			if (idexn.r2016 == dmwbn.RegNum) {
				EXdmwbf = true;
				EXrtf   = idexn.r2016;
				idexn.regi_output2 = dmwbn.DM_result;
			}
		}
		// jr
		else if (idexn.r0500 == JR) {
			// DM to EX
			if (idexn.r2521 == exdmn.RegNum) {
				EXexdmf = true;
				EXrsf   = idexn.r2521;
				idexn.regi_output1 = exdmn.ALU_result;
			}
			// WB to EX
			if (idexn.r2521 == dmwbn.RegNum) {
				EXdmwbf = true;
				EXrsf   = idexn.r2521;
				idexn.regi_output1 = dmwbn.DM_result;
			}
		}
	}
}

// input: regi_output1, regi_output2, ALUSrc, extend_result, ALUOp, opcode, r1006
// output: ALU_result, cmp, HI, LO
inline void ALU() {
	int ALU_input1 = idexn.regi_output1;
	int ALU_input2 = idexn.ALUSrc ? idexn.extend_result : idexn.regi_output2;
	long long tmp;
	switch (idexn.ALUOp) {
		case ADD:
			exdm.ALU_result = ALU_input1 + ALU_input2;
			if ((exdm.ALU_result >= 0 && ALU_input1 <  0 && ALU_input2 <  0) || 
			    (exdm.ALU_result <  0 && ALU_input1 >= 0 && ALU_input2 >= 0)) {
				errorflag_overflow = true;
			}
			break;
		case ADDU:
			exdm.ALU_result = ALU_input1 + ALU_input2;
			break;
		case SUB:
			ALU_input2 = ~ALU_input2 + 1;
			exdm.ALU_result = ALU_input1 + ALU_input2;
			if ((exdm.ALU_result >= 0 && ALU_input1 <  0 && ALU_input2 <  0) || 
			    (exdm.ALU_result <  0 && ALU_input1 >= 0 && ALU_input2 >= 0)) {
				errorflag_overflow = true;
			}
			break;
		case MULT:
			if (!(moveflag_HI || moveflag_LO)) {
				errorflag_overwrite = true;
			}
			moveflag_HI = moveflag_LO = false;
			tmp = (long long)ALU_input1 * (long long)ALU_input2;
			HI = tmp >> 32;
			LO = tmp & 0x00000000ffffffff;
			break;
		case MULTU:
			if (!(moveflag_HI || moveflag_LO)) {
				errorflag_overwrite = true;
			}
			moveflag_HI = moveflag_LO = false;
			tmp = (long long)((unsigned int)ALU_input1) * 
				  (long long)((unsigned int)ALU_input2);
			HI = tmp >> 32;
			LO = tmp & 0x00000000ffffffff;
			break;
		case AND:
			exdm.ALU_result = ALU_input1 & ALU_input2;
			break;
		case OR:
			exdm.ALU_result = ALU_input1 | ALU_input2;
			break;
		case NAND:
			exdm.ALU_result = ~(ALU_input1 & ALU_input2);
			break;
		case NOR:
			exdm.ALU_result = ~(ALU_input1 | ALU_input2);
			break;
		case XOR:
			exdm.ALU_result = ALU_input1 ^ ALU_input2;
			break;
		case SLL:
			exdm.ALU_result = ALU_input2 << idexn.r1006;
			break;
		case SRL:
			exdm.ALU_result = (unsigned int)ALU_input2 >> idexn.r1006;
			break;
		case SRA:
			exdm.ALU_result = ALU_input2 >> idexn.r1006;
			break;
		case LUI:
			exdm.ALU_result = ALU_input2 << 16;
			break;
		case COMP:
			exdm.ALU_result = ALU_input1 < ALU_input2;
			break;
		case NONE:
			exdm.ALU_result = 0;
			break;
		default:
			printf("<<< ERROR! invalid ALUOp\n");
			break;
	}
	
	// if (idexn.opcode == BEQ && ALU_input1 == ALU_input2)      cmp = 1;
	// else if (idexn.opcode == BNE && ALU_input1 != ALU_input2) cmp = 1;
	// else if (idexn.opcode == BGTZ && ALU_input1 > 0)               cmp = 1;
	// else                                                                cmp = 0;
}
