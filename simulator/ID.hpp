#include "define.hpp"

inline void ID() {
	control();
	
	regi_read();
	
	forwarding_ID();
	
	if (idex.regi_output1 == idex.regi_output2 && idex.opcode == BEQ) {
		idex.cmp = 1;
	} else if (idex.regi_output1 != idex.regi_output2 && idex.opcode == BNE) {
		idex.cmp = 1;
	} else if (idex.regi_output1 > 0 && idex.opcode == BGTZ) {
		idex.cmp = 1;
	} else {
		idex.cmp = 0;
	}
	
	sign_extend();
	
	hazard_unit();
	
	PC_control();
}

inline void forwarding_ID() {
	IDexdmf = IDdmwbf = false;
	IDrsf   = IDrtf   =     0;
	// the forwarding rule here is just for beq, bne, and bgtz
	if (ifidn.opcode == BEQ || ifidn.opcode == BNE) {
		// DM to ID
		if (ifidn.r2521 == exdmn.RegNum) {
			IDexdmf = true;
			IDrsf   = ifidn.r2521;
			ifidn.regi_output1 = exdmn.ALU_result;
		}
		if (ifidn.r2016 == exdmn.RegNum) {
			IDexdmf = true;
			IDrtf   = ifidn.r2016;
			ifidn.regi_output2 = exdmn.ALU_result;
		}
		// WB ot ID
		if (ifidn.r2521 == dmwbn.RegNum) {
			IDdmwbf = true;
			IDrsf   = ifidn.r2521;
			ifidn.regi_output1 = dmwbn.DM_result;
		}
		if (ifidn.r2016 == dmwbn.RegNum) {
			IDdmwbf = true;
			IDrtf   = ifidn.r2016;
			ifidn.regi_output2 = dmwbn.DM_result;
		}
	} else if (ifidn.opcode == BGTZ) {
		// DM to ID
		if (ifidn.r2521 == exdmn.RegNum) {
			IDexdmf = true;
			IDrsf   = ifidn.r2521;
			ifidn.regi_output1 = exdmn.ALU_result;
		}
		// WB to ID
		if (ifidn.r2521 == dmwbn.RegNum) {
			IDdmwbf = true;
			IDrsf   = ifidn.r2521;
			ifidn.regi_output1 = exdmn.DM_result;
		}
	}
}

inline void hazard_unit() {
	// flush
	if (ifidn.opcode == JAL || ifidn.opcode == J || 
	    (idex.cmp && (ifidn.opcode == BNE || ifidn.opcode == BEQ || ifidn.opcode == BGTZ)) ||
	    (ifidn.opcode == 0 && ifidn.r0500 == JR)) {
		// ifidn = ifid0; // TODO: some problem here, timing and position is wrong
		idex.nop = true;
		flush = true;
	}
	
	// stall
	if (ifidn.opcode == BNE || ifidn.opcode == BEQ) {
		if (idexn.RegWrite && (ifidn.r2521 == idexn.RegNum || ifidn.r2016 == idexn.RegNum)) {
			stall = 2;
		} else if (exdmn.RegWrite && (ifidn.r2521 == exdmn.RegNum || ifidn.r2016 == exdmn.RegNum)) {
			stall = 1;
		}
	} else if (ifidn.opcode == BGTZ) {
		if (idexn.RegWrite && ifidn.r2521 == idexn.RegNum) {
			stall = 2;
		} else if (exdmn.RegWrite && ifidn.r2521 == exdmn.RegNum) {
			stall = 1;
		}
	}
}

// input: r2521, r2016
// output: regi_output1, regi_output2
inline void regi_read() {
	idex.regi_output1 = regi[ifidn.r2521];
	idex.regi_output2 = regi[ifidn.r2016];
}

// input: opcode, r1500, r0500
// output: extend_result
inline void sign_extend() {
	if (ifidn.opcode == ANDI || ifidn.opcode == ORI || ifidn.opcode == NORI) {
		idex.extend_result = ifidn.r1500;
	} else if (ifidn.opcode) { // extend r1500 to 32 bit
		idex.extend_result = (ifidn.r1500 >= 32768) ? (0xffff0000 | ifidn.r1500) : ifidn.r1500;
	} else {             // extend r0500 to 32 bit but unsigned 
		idex.extend_result = ifidn.r0500;
	}
}

// input: opcode, r0500
// output: RegDst, RegWrite, MemtoReg, ALUSrc, ALUOp, MemRead, MemWrite
inline void control() {
	idex.RegDst   = ifidn.opcode; // if R type, opcode == 0; else opcode == 1
	idex.ALUSrc   = (ifidn.opcode != BNE && ifidn.opcode != BEQ && 
	                 ifidn.opcode != BGTZ && ifidn.opcode);
	idex.MemRead  = 0; 
	idex.MemWrite = 0;
	idex.RegWrite = 0;
	switch(ifidn.opcode) {
		case 0:
			switch(ifidn.r0500) {
				case ADD: 	
					idex.ALUOp    = ADD; 	
					idex.RegWrite = 1;
					idex.opc      = "ADD";
					break;
				case ADDU: 	
					idex.ALUOp    = ADDU;
					idex.RegWrite = 1; 	
					idex.opc      = "ADDU";
					break;
				case SUB: 	
					idex.ALUOp    = SUB; 
					idex.RegWrite = 1;	
					idex.opc      = "SUB";
					break;
				case AND: 	
					idex.ALUOp    = AND; 
					idex.RegWrite = 1;	
					idex.opc      = "AND";
					break;
				case OR: 	
					idex.ALUOp    = OR; 
					idex.RegWrite = 1;	
					idex.opc      = "OR";
					break;
				case XOR: 	
					idex.ALUOp    = XOR; 
					idex.RegWrite = 1;	
					idex.opc      = "XOR";
					break;
				case NOR: 	
					idex.ALUOp    = NOR; 
					idex.RegWrite = 1;	
					idex.opc      = "NOR";
					break;
				case NAND: 	
					idex.ALUOp    = NAND;
					idex.RegWrite = 1; 	
					idex.opc      = "NAND";
					break;
				case SLT: 	
					idex.ALUOp    = COMP;
					idex.RegWrite = 1; 	
					idex.opc      = "SLT";
					break;
				case SLL: 	
					idex.ALUOp    = SLL; 
					idex.RegWrite = 1;	
					idex.opc      = "SLL";
					break;
				case SRL: 	
					idex.ALUOp    = SRL; 
					idex.RegWrite = 1;	
					idex.opc      = "SRL";
					break;
				case SRA: 	
					idex.ALUOp    = SRA; 
					idex.RegWrite = 1;	
					idex.opc      = "SRA";
					break;
				case JR: 	
					idex.ALUOp    = NONE;
					idex.opc      = "JR";
					break;
				case MULT: 	
					idex.ALUOp    = MULT;
					idex.opc      = "MULT";
					break;
				case MULTU: 
					idex.ALUOp    = MULTU; 	
					idex.opc      = "MULTU";
					break;
				case MFHI: 	
					idex.ALUOp    = NONE; 	
					idex.RegWrite = 1;
					idex.opc      = "MFHI";
					break;
				case MFLO: 	
					idex.ALUOp    = NONE;
					idex.RegWrite = 1; 	
					idex.opc      = "MFLO";
					break;
			}
			break;
		case ADDI: 	
			idex.ALUOp    = ADD; 	
			idex.RegWrite = 1;
			idex.opc      = "ADDI";
			break;
		case ADDIU: 
			idex.ALUOp    = ADDU;
			idex.RegWrite = 1; 	
			idex.opc      = "ADDIU";
			break;
		case LW: 	
			idex.ALUOp    = ADD; 
			idex.RegWrite =   1;	
			idex.MemRead  = 114; 
			idex.opc      = "LW";
			break;
		case LH: 	
			idex.ALUOp    = ADD; 
			idex.RegWrite =   1;	
			idex.MemRead  = 112; 
			idex.opc      = "LH";
			break;
		case LHU: 	
			idex.ALUOp    = ADD; 
			idex.RegWrite =   1;	
			idex.MemRead  = 122; 
			idex.opc      = "LHU";
			break;
		case LB: 	
			idex.ALUOp    = ADD; 
			idex.RegWrite =   1;	
			idex.MemRead  = 111;
			idex.opc      = "LB";
			break;
		case LBU: 	
			idex.ALUOp    = ADD; 
			idex.RegWrite =   1;	
			idex.MemRead  = 121;
			idex.opc      = "LBU";
			break;
		case SW: 	
			idex.ALUOp    = ADD; 
			idex.MemWrite = 214;
			idex.opc      = "SW";
			break;
		case SH: 	
			idex.ALUOp    = ADD;
			idex.MemWrite = 212;
			idex.opc      = "SH";
			break;
		case SB: 	
			idex.ALUOp    = ADD;
			idex.MemWrite = 211;
			idex.opc      = "SB";
			break;
		case LUI: 	
			idex.ALUOp    = LUI;
			idex.RegWrite = 1; 	
			idex.opc      = "LUI";
			break;
		case ANDI: 	
			idex.ALUOp    = AND; 
			idex.RegWrite = 1;	
			idex.opc      = "ANDI";
			break;
		case ORI: 	
			idex.ALUOp    = OR; 
			idex.RegWrite = 1;	
			idex.opc      = "ORI";
			break;
		case NORI:  	
			idex.ALUOp    = NOR; 
			idex.RegWrite = 1;
			idex.opc      = "NORI";
			break;
		case SLTI: 	
			idex.ALUOp    = COMP; 	
			idex.RegWrite = 1;
			idex.opc      = "SLTI";			
			break;
		case BEQ: 	
			idex.ALUOp    = NONE; 	
			idex.opc      = "BEQ";
			break;
		case BNE: 	
			idex.ALUOp    = NONE;
			idex.opc      = "BNE";
			break;
		case BGTZ: 	
			idex.ALUOp    = NONE;
			idex.opc      = "BGTZ";
			break;
		case J:     
			idex.ALUOp    = NONE;
			idex.opc      = "J";
			break;
		case JAL:   
			idex.ALUOp    = NONE;
			idex.opc      = "JAL";
			break;
	}
	idex.MemtoReg = idex.MemRead;
	idex.RegNum   = (idex.RegDst) ? idex.r2016 : idex.r1511;
}

// input: PC, cmp, opcode, r2500, extend_result, regi_output1
// output: PC(Next cycle)
inline void PC_control() {
	int NPC = PC + 4; // Next PC
	int BPC = NPC + (idex.extend_result << 2); // Branch PC
	if (idex.cmp) { // TODO: Where is cmp? maybe just after firwarting unit
		PC = BPC;
	} else if (ifidn.opcode == J) {
		PC = ((NPC & 0xf0000000)|(ifidn.r2500 << 2));
	} else if (ifidn.opcode == JAL) {
		PC = ((NPC & 0xf0000000)|(ifidn.r2500 << 2));
		regi[31] = NPC;
	} else if (ifidn.opcode == 0 && ifidn.r0500 == JR) {
		PC = idex.regi_output1;
	} else {
		PC = NPC;
	}
}