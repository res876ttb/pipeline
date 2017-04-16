#include <stdio.h>
#include <stdlib.h>

#include "define.hpp"

int main() {
	init();
	print_all();
	do {
		cycle++;
		command = memi[PC/4];
		#ifdef log
		printf("Command = 0x%08X\n", command);
		#endif
		// skip command sll $0 $0 0, but how ??????
		if ((command & 0xfc1fffff) == 0) {
			// TODO
			PC += 4;
			print_diff();
			continue;
		}
		
		WB_State();
		DM_State();
		EX_State();
		ID_State();
		IF_State();
		
		print_error();
		print_diff();
	} while(0);
	finalize(0);
	return 0;
}

//======================================================== inline void WB_State() =========
inline void WB_State() {
	#ifdef log
		printf(">>> WB_State()\n");
	#endif
	//==================================================== mux =========
	if (MEMWB_MemtoReg) {
		write_back = MEMWB_Data_Result;
	} else {
		write_back = MEMWB_ALU_Result;
	}
	
	//==================================================== forwarding unit input =========
	FU_input_WB_Rd = MEMWB_Register_WB_Number;
	FU_input_WB_MemtoReg = MEMWB_MemtoReg;
	
	#ifdef log
		printf(">>> WB_State() done\n");
	#endif
}

//======================================================== inline void DM_State() =========
inline void DM_State() {
	#ifdef log
		printf(">>> DM_State()\n");
	#endif
	
	DM_DataMemory();
	
	//==================================================== forwarding unit input =========
	FU_input_DM_Rd = EXMEM_Register_WB_Number;
	FU_input_DM_MemtoReg = EXMEM_MemtoReg;
	
	//==================================================== pass control signal to WB =========
	MEMWB_MemtoReg = EXMEM_MemtoReg;
	MEMWB_RegWr = EXMEM_RegWr;
	
	#ifdef log
		printf(">>> DM_State() done\n");
	#endif
}

//======================================================== inline void EX_State() =========
inline void EX_State() {
	#ifdef log
		printf(">>> EX_State()\n");
	#endif
	int ALU_input1, ALU_input2;
	forwarding_unit();
	EX_mux1();
	EX_mux2();
	EX_mux3();
	EX_mux4();
	ALU();
	#ifdef log
		printf(">>> EX_State() done\n");
	#endif
}

//======================================================== inline void ID_State() =========
inline void ID_State() {
	#ifdef log
		printf(">>> ID_State()\n");
	#endif
	
	decoder();
	hazard_detector();
	controller();
	registers();
	
	#ifdef log
		printf(">>> ID_State() done\n");
	#endif
}

//======================================================== inline void IF_State() =========
inline void IF_State() {
	
}

//======================================================== inline void init() =========
inline void init() {
	#ifdef log
		printf(">>> init()\n");	
	#endif
		
	#ifdef log
		printf(">>> opening files...\n");
	#endif
	fout = fopen("snapshot.rpt", "wb");
	ferr = fopen("error_dump.rpt", "wb");
	if (!fout) {
		printf("snapshot.rpt cannot be opened!\n");
	}
	if (!ferr) {
		printf("error_dump.rpt cannot be opened!\n");
	}
	#ifdef log
		printf(">>> opening files...  done\n");
	#endif
	
	read_data();
	
	#ifdef log
		printf(">>> init() done\n");
	#endif
}

//======================================================== inline void read_data() =========
inline void read_data() {
	#ifdef log
		printf(">>> read_data()\n");
	#endif
	
	char buff[4];
	int sizeofchar = sizeof(char);
	int tmp, data_size;
	FILE *iimage, *dimage;
	iimage = fopen("iimage.bin", "rb");
	dimage = fopen("dimage.bin", "rb");
	
	if (!dimage) {
		printf("No dimage.bin\n");
	}
	if (!iimage) {
		printf("No iimage.bin\n");
	}
	
	#ifdef log
		printf(">>> Reading dimage.bin...  ");
	#endif
	// read dimage.bin
	tmp = fread(buff, sizeofchar, 4, dimage); // read initial SP address
	regi[29] = regi2[29] =  (((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));
	tmp = fread(buff, sizeofchar, 4, dimage); // read total number of data
	data_size = (((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));
	tmp = fread(memd, sizeofchar, data_size * 4, dimage);
	#ifdef log
		printf("done\n");
	#endif
	
	#ifdef log
		printf(">>> Reading iimage.bin...  ");
	#endif
	// read iimage.bin
	tmp = fread(buff, sizeofchar, 4, iimage); // read initial PC address
	PC = (((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));
	tmp = fread(buff, sizeofchar, 4, iimage); // read total instruction number
	data_size = (((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));
	for (int i = PC/4; fread(buff, sizeofchar, 4, iimage); i++) {
		memi[i] = (((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));
	}
	#ifdef log
		printf("done\n");
	#endif
		
	#ifdef log
		printf(">>> read_data() done\n");
	#endif
}

//======================================================== inline void print_all() =========
inline void print_all() {
	#ifdef log
		printf(">>> print_all()\n");
	#endif
	
	oprintf("cycle 0\n");
	for (int i = 0; i < 32; i++) {
		oprintf("$%02d: 0x%08X\n", i, regi[i]);
	}
	oprintf("$HI: 0x%08X\n", HI);
	oprintf("$LO: 0x%08X\n", LO);
	oprintf("PC: 0x%08X\n", PC);
	oprintf("IF: 0x%08X\n", memi[PC/4]);
	oprintf("ID: NOP\n");
	oprintf("EX: NOP\n");
	oprintf("DM: NOP\n");
	oprintf("WB: NOP\n");
	oprintf("\n\n");
	
	#ifdef log
		printf(">>> print_all() done\n");
	#endif
}

//======================================================== inline void finalize(int n) =========
inline void finalize(int n) {
	#ifdef log
		printf(">>> finalizing...  ");
	#endif
	fclose(ferr);
	fclose(fout);
	#ifdef log
		printf("done\n");
	#endif
	exit(n);
}

//======================================================== inline void print_diff() =========
inline void print_diff() {
	oprintf("cycle %d\n", cycle);
	for (int i = 0; i < 32; i++) {
		if (regi[i] != regi2[i]) {
			oprintf("$%02d: %08X\n", i, regi[i]);
			regi2[i] = regi[i];
		}
	}
	if (HI != HI2) {
		oprintf("$HI: %08X\n", HI);
		HI2 = HI;
	}
	if (LO != LO2) {
		oprintf("$LO: %08X\n", LO);
		HI2 = LO;
	}
	if (PC != PC2) {
		oprintf("PC: %08X\n", PC);
		HI2 = PC;
	}
}

//======================================================== inline void print_error() =========
inline void print_error() {
	
}

//======================================================== inline void DM_DataMemory() =========
inline void DM_DataMemory() {
	//==================================================== Memory error detection =========
	// Deal with memory alignment error
	// I don't know where to place this error detection
	if (EXMEM_MemW / 100 != 0) {
		if  (EXMEM_ALU_Result > 1024 - EXMEM_MemW % 10 || EXMEM_ALU_Result < 0) {
			errorflag_memoryOverflow = true;
		}
		if (EXMEM_ALU_Result % (EXMEM_MemW % 10) != 0) {
			errorflag_missingAlign = true;
		}
		
		#ifdef debug
		printf(">>> errorflag_memoryOverflow: %s\n", (errorflag_memoryOverflow?"true":"false"));
		printf(">>> errorflag_missingAlign:   %s\n", (errorflag_missingAlign  ?"true":"false"));
		#endif
	}
	
	//==================================================== Data memory =========	
	// MEMWB_Data_Result <== read data result
	switch(EXMEM_MemW) {
		case 0:
			MEMWB_ALU_Result = EXMEM_ALU_Result;
		break;
		case 111:
			MEMWB_Data_Result = memd2reg(EXMEM_ALU_Result, 1);
		break;
		case 112:
			MEMWB_Data_Result = memd2reg(EXMEM_ALU_Result, 2);
		break;
		case 114:
			MEMWB_Data_Result = memd2reg(EXMEM_ALU_Result, 4);
		break;
		case 121:
			MEMWB_Data_Result = memd2reg(EXMEM_ALU_Result, 1) & 0x000000ff;
		break;
		case 122:
			MEMWB_Data_Result = memd2reg(EXMEM_ALU_Result, 2) & 0x0000ffff;
		break;
		case 211:
			memd[EXMEM_ALU_Result] = EXMEM_ALU_input2 & 0x000000ff;
		break;
		case 212:
			memd[EXMEM_ALU_Result  ] = (EXMEM_ALU_input2>>8) & 0x000000ff;
			memd[EXMEM_ALU_Result+1] = (EXMEM_ALU_input2   ) & 0x000000ff;
		break;
		case 214:
			memd[EXMEM_ALU_Result  ] = (EXMEM_ALU_input2>>24) & 0x000000ff;
			memd[EXMEM_ALU_Result+1] = (EXMEM_ALU_input2>>16) & 0x000000ff;
			memd[EXMEM_ALU_Result+2] = (EXMEM_ALU_input2>>8 ) & 0x000000ff;
			memd[EXMEM_ALU_Result+3] = (EXMEM_ALU_input2    ) & 0x000000ff;
		break;
		default:
			printf("<<< ERROR! invalid code in EXMEM_MemW/D_memory\n");
			finalize(1);
		break;
	}
}

//======================================================== int memd2reg(int start, int len) =======
inline int memd2reg(int start, int len) {
    int tmp;
    if (len==1) {
        return memd[start];
    } else if (len==2) {
        tmp=(((unsigned char)memd[start])<<8)|((unsigned char)memd[start+1]);
        if (tmp<0x00008000) {
            return tmp;
        } else {
            return tmp+0xffff0000;   
        }
    } else if (len==4) {
        return ((((unsigned char)memd[start])<<24)|(((unsigned char)memd[start+1])<<16)|(((unsigned char)memd[start+2])<<8)|(((unsigned char)memd[start+3])));
    } else {
        return 0;
    }
}

//======================================================== inline void ALU() =========
inline void ALU() {
	#ifdef log
	printf(">>> ALU()...  ");
	#endif
	
	int ALU_result;
	int tmp_sub;
	long long tmp_mult;
	long long tmp_multu;
	switch (IDEX_ALUOp) {
		case ADD:
			ALU_result = ALU_input1 + ALU_input2;
			if ((ALU_result >= 0 && ALU_input1 <  0 && ALU_input2 <  0) || 
			    (ALU_result <  0 && ALU_input1 >= 0 && ALU_input2 >= 0)) {
				errorflag_overflow = true;
			}
			break;
		case ADDU:
			ALU_result = ALU_input1 + ALU_input2;
			break;
		case SUB:
			tmp_sub = (~ALU_input2) + 1;
			ALU_result = ALU_input1 + tmp_sub;
			if ((ALU_result >= 0 && ALU_input1 <  0 && tmp_sub <  0) || 
			    (ALU_result <  0 && ALU_input1 >= 0 && tmp_sub >= 0)) {
				errorflag_overflow = true;
			}
			break;
		case MULT:
			if (moveflag_HI || moveflag_LO) {
				errorflag_overwrite = true;
			}
			moveflag_HI = moveflag_LO = false;
			tmp_mult = (long long)ALU_input1 * (long long)ALU_input2;
			HI = tmp_mult >> 32;
			LO = tmp_mult & 0x00000000ffffffff;
			break;
		case MULTU:
			if (moveflag_HI || moveflag_LO) {
				errorflag_overwrite = true;
			}
			moveflag_HI = moveflag_LO = false;
			tmp_multu = (long long)((unsigned int)ALU_input1) * 
					    (long long)((unsigned int)ALU_input2);
			HI = tmp_multu >> 32;
			LO = tmp_multu & 0x00000000ffffffff;
			break;
		case AND:
			ALU_result = ALU_input1 & ALU_input2;
			break;
		case OR:
			ALU_result = ALU_input1 | ALU_input2;
			break;
		case NAND:
			ALU_result = ~(ALU_input1 & ALU_input2);
			break;
		case NOR:
			ALU_result = ~(ALU_input1 | ALU_input2);
			break;
		case XOR:
			ALU_result = ALU_input1 ^ ALU_input2;
			break;
		case SHIFT_LEFT:
			ALU_result = ALU_input2 << ALU_shift;
			break;
		case SHIFT_RIGHT:
			ALU_result = (unsigned int)ALU_input2 >> ALU_shift;
			break;
		case SHIFT_RIGHT_ARITH:
			ALU_result = ALU_input2 >> ALU_shift;
			break;
		case LUI:
			ALU_result = ALU_input2 << 16;
			break;
		case COMPARE:
			ALU_result = (ALU_input1 < ALU_input2);
			break;
		case nothing:
			break;
		default:
			printf("<<< ERROR! invalid IDEX_ALUOp in /EX_State/ALU()\n");
			finalize(1);
			break;
	}
	EXMEM_ALU_Result = ALU_result;
	
	#ifdef log
	printf("done\n");
	#endif
}

//======================================================== void inline forwarding_unit() =========
void inline forwarding_unit() {
	if (EXMEM_RegWr && EXMEM_Register_WB_Number != 0) {
		if (EXMEM_Register_WB_Number == IDEX_Register_Rs) {
			FU_output_mux1 = 2;
			FU_output_mux2 = 0;
		} else if (EXMEM_Register_WB_Number == IDEX_Register_Rt1) {
			FU_output_mux1 = 0;
			FU_output_mux2 = 2;
		} else {
			FU_output_mux1 = 0;
			FU_output_mux2 = 0;
		}
	} else if (MEMWB_RegWr && MEMWB_Register_WB_Number != 0) {
		if (MEMWB_Register_WB_Number == IDEX_Register_Rs) {
			FU_output_mux1 = 1;
			FU_output_mux2 = 0;
		} else if (MEMWB_Register_WB_Number == IDEX_Register_Rt1) {
			FU_output_mux1 = 0;
			FU_output_mux2 = 1;	
		} else {
			FU_output_mux1 = 0;
			FU_output_mux2 = 0;	
		}
	} else {
		FU_output_mux1 = 0;
		FU_output_mux2 = 0;	
	}
}

//======================================================== inline void EX_mux1() =========
inline void EX_mux1() {
	switch(FU_output_mux1) {
		case 0:
			ALU_input1 = IDEX_Register_Result[0];
			break;
		case 1:
			ALU_input1 = write_back;
			break;
		case 2:
			ALU_input1 = EXMEM_ALU_Result;
			break;
		default:
			printf("<<< ERROR! invalid judge code in /EX_State/MUX1\n");
			finalize(1);
			break;
	}
}

//======================================================== inline void EX_mux2() =========
inline void EX_mux2() {
	switch(FU_output_mux2) {
		case 0:
			if (IDEX_ALUSrc) {
				ALU_input2 = IDEX_Signed_extend;
			} else {
				ALU_input2 = IDEX_Register_Result[1];
			}
			break;
		case 1:
			ALU_input2 = write_back;
			break;
		case 2:
			ALU_input2 = EXMEM_ALU_Result;
			break;
		default:
			printf("<<< ERROR! invalid judge code in /EX_State/MUX2\n");
			finalize(1);
			break;
	}
}

//======================================================== inline void EX_mux3() =========
inline void EX_mux3() {
	switch(IDEX_RegDst) {
		case 0:
			EXMEM_Register_WB_Number = IDEX_Register_Rt2;
			break;
		case 1:
			EXMEM_Register_WB_Number = IDEX_Register_Rd;
			break;
		default:
			printf("<<< ERROR! invalid judge code in /EX_State/MUX3\n");
			finalize(1);
			break;
	}
}

//======================================================== inline void EX_mux4() =========
inline void EX_mux4() {
	switch(HD_output_EX_flush) {
		case 0:
			EXMEM_MemW     = IDEX_MemW;
			EXMEM_Branch   = IDEX_Branch;
			EXMEM_MemtoReg = IDEX_MemtoReg;
			EXMEM_RegWr    = IDEX_RegWr;
			break;
		case 1:
			EXMEM_MemW     = 0;
			EXMEM_Branch   = 0;
			EXMEM_MemtoReg = 0;
			EXMEM_RegWr    = 0;
			break;
		default:
			printf("<<< ERROR! invalid judge code in /EX_State/MUX4\n");
			finalize(1);
			break;
	}
}

inline void hazard_detector() {
	if (IDEX_Branch) {
		
	} else if (IDEX_MemtoReg) {
		
	}
}

inline void controller() {
	// IDEX_ExtOp = ; ?????
	//=============================== IDEX_ALUSrc ===============================
	switch(opcode) {
		case ADDI: case ADDIU: case LUI: case ANDI: case ORI: case NORI: case SLTI: 
			IDEX_ALUSrc = 1;
			break;
		default:
			IDEX_ALUSrc = 0;
			break;
	}
	
	//=============================== IDEX_ALUOp ================================
	     if (opcode == ADDI  || (opcode == 0 && r0500 == ADD )) IDEX_ALUOp = ADD;
	else if (opcode == ADDIU || (opcode == 0 && r0500 == ADDU)) IDEX_ALUOp = ADDU;
	else if (                    opcode == 0 && r0500 == SUB  ) IDEX_ALUOp = SUB;
	else if (                    opcode == 0 && r0500 == MULT ) IDEX_ALUOp = MULT;
	else if (                    opcode == 0 && r0500 == MULTU) IDEX_ALUOp = MULTU;
	else if (opcode == ANDI  || (opcode == 0 && r0500 == AND )) IDEX_ALUOp = AND;
	else if (opcode == ORI   || (opcode == 0 && r0500 == OR  )) IDEX_ALUOp = OR;
	else if (                    opcode == 0 && r0500 == NAND ) IDEX_ALUOp = NAND;
	else if (opcode == NORI  || (opcode == 0 && r0500 == NOR )) IDEX_ALUOp = NOR;
	else if (                    opcode == 0 && r0500 == XOR  ) IDEX_ALUOp = XOR;
	else if (                    opcode == 0 && r0500 == SLL  ) IDEX_ALUOp = SLL;
	else if (                    opcode == 0 && r0500 == SRL  ) IDEX_ALUOp = SRL;
	else if (                    opcode == 0 && r0500 == SRA  ) IDEX_ALUOp = SRA;
	else if (opcode == LUI                                    ) IDEX_ALUOp = LUI;
	else if (opcode == SLTI  || (opcode == 0 && r0500 == SLT )) IDEX_ALUOp = COMPARE;
	else                                                        IDEX_ALUOp = nothing;
	
	//=============================== IDEX_RegDst ===============================
	IDEX_RegDst = !opcode;
	
	//=============================== IDEX_MemW =================================
	switch(opcode) {
		case LW:  IDEX_MemW = 114; break;
		case LH:  IDEX_MemW = 112; break;
		case LHU: IDEX_MemW = 122; break;
		case LB:  IDEX_MemW = 111; break;
		case LBU: IDEX_MemW = 121; break;
		case SW:  IDEX_MemW = 214; break;
		case SH:  IDEX_MemW = 212; break;
		case SB:  IDEX_MemW = 211; break;
		default:  IDEX_MemW = 0;   break;
	}
	
	//=============================== IDEX_Branch ===============================
	IDEX_Branch = (opcode == J || opcode == JAL);
	
	//=============================== IDEX_MemtoReg =============================
	IDEX_MemtoReg = (opcode == LW  || opcode == LH || opcode == LB || 
	                 opcode == LHU || opcode == LBU);
	
	//=============================== IDEX_RegWr ================================
	IDEX_RegWr = !((opcode == 0 &&
	               (opcode == JR  || opcode == MULT || opcode == MULTU)) || 
	                opcode == SW  || opcode == SH   || opcode == SB      ||
	                opcode == BEQ || opcode == BNE  || opcode == BGTZ    ||
	                opcode == J   || opcode == JAL  || opcode == HALT);
}

inline void decoder() {
	opcode = scan_command(31,26);
	r2521  = scan_command(25,21);
	r2016  = scan_command(20,16);
	r1511  = scan_command(25,11);
	r1006  = scan_command(10, 6);
	r0500  = scan_command( 5, 0);
	r2500  = scan_command(25, 0);
	r1500  = scan_command(15, 0);
}

inline void registers() {
	if (MEMWB_RegWr) {
		regi[MEMWB_Register_WB_Number] = write_back;
	}
	IDEX_Register_Result[0] = regi[r2521];
	IDEX_Register_Result[1] = regi[r2016];
}

inline int scan_command(int start, int end) {
    unsigned int b=0xFFFFFFFF;
    return (command>>end)&(b>>(end+31-start));
}
