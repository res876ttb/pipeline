#include "define.hpp"

int main() {
	init();
	print_all();
	do {
		cycle += 1;
		command = memi[PC / 4];
		
		decoder();
		
		if (opcode == HALT) {
			finalize(0);
		} else if ((command & 0xfc1fffff) == 0) {
			PC += 4;
			print_diff();
			continue;
		}
		control();
		regi_read();
		sign_extend();
		ALU();
		data_memory();
		regi_write();
		PC_control();
		
		print_error();
		print_diff();
		
		if (cycle%500000==0){printf("In cycle %d, Press Enter to continue...\n", cycle);getchar();}
	} while (true);
	finalize(0);
	return 0;
}

// input: PC, cmp, opcode, r2500, extend_result, regi[r2521]<=use other method to get
// output: PC(Next cycle)
inline void PC_control() {
	int NPC = PC + 4; // Next PC
	int BPC = NPC + (extend_result << 2); // Branch PC
	if (cmp) {
		PC = BPC;
	} else if (opcode == J) {
		PC = ((NPC & 0xf0000000)|(r2500 << 2));
	} else if (opcode == JAL) {
		PC = ((NPC & 0xf0000000)|(r2500 << 2));
		regi[31] = NPC;
	} else if (opcode == 0 && r0500 == JR) {
		PC = regi[r2521];
	} else {
		PC = NPC;
	}
}

// input: RegDst, RegWrite, MemRead, regi_input, DM_result, ALU_result, opcode, 
//        r2016, r1511, r0500, moveflag_HI, moveflag_LO, HI, LO
// output: NONE
inline void regi_write() {
	regi_input = (RegDst) ? r2016 : r1511;
	if (RegWrite) {
		regi[regi_input] = (MemRead) ? DM_result : ALU_result;
		if (opcode == 0 && r0500 == MFHI) {
			regi[regi_input] = HI;
			moveflag_HI = true;
		}
		if (opcode == 0 && r0500 == MFLO) {
			regi[regi_input] = LO;
			moveflag_LO = true;
		}
		if (regi_input == 0) {
			errorflag_write0 = true;
			regi[0] = 0;
		}
	}
}

// input: MemRead, ALU_result, regi_output2
// output: DM_result
inline void data_memory() {
	if (MemRead == 111) {
		if (ALU_result < 1024 && ALU_result >= 0)
			DM_result = memd2reg1(ALU_result);
		else 
			errorflag_memoryOverflow = true;
	} else if (MemRead == 112) {
		if (ALU_result %2 != 0) errorflag_missingAlign = true;
		if (ALU_result < 1023 && ALU_result >= 0) 
			DM_result = memd2reg2(ALU_result);
		else
			errorflag_memoryOverflow = true;
	} else if (MemRead == 114) {
		if (ALU_result %4 != 0) errorflag_missingAlign = true;
		if (ALU_result < 1021 &&ALU_result >= 0)
			DM_result = memd2reg4(ALU_result);
		else
			errorflag_memoryOverflow = true;
	} else if (MemRead == 121) {
		if (ALU_result < 1024 && ALU_result >= 0)
			DM_result = memd2reg1(ALU_result) & 0x000000ff;
		else
			errorflag_memoryOverflow = true;
	} else if (MemRead == 122) {
		if (ALU_result %2 != 0) errorflag_missingAlign = true;
		if (ALU_result < 1023 && ALU_result >= 0)
			DM_result = memd2reg2(ALU_result) & 0x0000ffff;
		else
			errorflag_memoryOverflow = true;
	} else {
		DM_result = 0;
	}
	if (MemWrite == 211) {
		if (ALU_result < 1024 && ALU_result >= 0)
			memd[ALU_result]   =  regi_output2      & 0x000000ff;
		else
			errorflag_memoryOverflow = true;
	} else if (MemWrite == 212) {
		if (ALU_result %2 != 0) errorflag_missingAlign = true;
		if (ALU_result < 1023 && ALU_result >= 0) {
			memd[ALU_result  ] = (regi_output2>> 8) & 0x000000ff;
			memd[ALU_result+1] = (regi_output2    ) & 0x000000ff;
		} else {
			errorflag_memoryOverflow = true;
		}
	} else if (MemWrite == 214) {
		if (ALU_result %4 != 0) errorflag_missingAlign = true;
		if (ALU_result < 1021 && ALU_result >= 0) {
			memd[ALU_result  ] = (regi_output2>>24) & 0x000000ff;
			memd[ALU_result+1] = (regi_output2>>16) & 0x000000ff;
			memd[ALU_result+2] = (regi_output2>>8 ) & 0x000000ff;
			memd[ALU_result+3] = (regi_output2    ) & 0x000000ff;
		} else {
			errorflag_memoryOverflow = true;
		}
	}
}

// input: regi_output1, regi_output2, ALUSrc, extend_result, ALUOp, opcode, r1006
// output: ALU_result, cmp, HI, LO
inline void ALU() {
	ALU_input1 = regi_output1;
	ALU_input2 = ALUSrc ? extend_result : regi_output2;
	long long tmp;
	switch (ALUOp) {
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
			ALU_input2 = ~ALU_input2 + 1;
			ALU_result = ALU_input1 + ALU_input2;
			if ((ALU_result >= 0 && ALU_input1 <  0 && ALU_input2 <  0) || 
			    (ALU_result <  0 && ALU_input1 >= 0 && ALU_input2 >= 0)) {
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
		case SLL:
			ALU_result = ALU_input2 << r1006;
			break;
		case SRL:
			ALU_result = (unsigned int)ALU_input2 >> r1006;
			break;
		case SRA:
			ALU_result = ALU_input2 >> r1006;
			break;
		case LUI:
			ALU_result = ALU_input2 << 16;
			break;
		case COMP:
			ALU_result = ALU_input1 < ALU_input2;
			break;
		case NONE:
			ALU_result = 0;
			break;
		default:
			printf("<<< ERROR! invalid ALUOp\n");
			break;
	}
	
	if (opcode == BEQ && ALU_input1 == ALU_input2) 		cmp = 1;
	else if (opcode == BNE && ALU_input1 != ALU_input2) cmp = 1;
	else if (opcode == BGTZ && ALU_input1 > 0) 			cmp = 1;
	else 												cmp = 0;
}

// input: opcode, r1500, r0500
// output: extend_result
inline void sign_extend() {
	if (opcode == ANDI || opcode == ORI || opcode == NORI) {
		extend_result = r1500;
	} else if (opcode) { // extend r1500 to 32 bit
		extend_result = (r1500 >= 32768) ? (0xffff0000 | r1500) : r1500;
	} else {             // extend r0500 to 32 bit but unsigned 
		extend_result = r0500;
	}
}

// input: r2521, r2016
// output: regi_output1, regi_output2
inline void regi_read() {
	regi_output1 = regi[r2521];
	regi_output2 = regi[r2016];
}

// input: command
// output: opcode, r2521, r2016, t1511, r1006, r2500, r1500, r0500
inline void decoder() {
	opcode = scan_command(31, 26);
	r2521  = scan_command(25, 21);
	r2016  = scan_command(20, 16);
	r1511  = scan_command(15, 11);
	r1006  = scan_command(10,  6);
	r2500  = scan_command(25,  0);
	r1500  = scan_command(15,  0);
	r0500  = scan_command( 5,  0);
}

// input: opcode, r0500
// output: RegDst, RegWrite, MemtoReg, ALUSrc, ALUOp, MemRead, MemWrite
inline void control() {
	RegDst   = opcode; // if R type, opcode == 0; else opcode == 1
	ALUSrc   = (opcode != BNE && opcode != BEQ && opcode != BGTZ && opcode);
	MemtoReg = MemRead;
	MemRead  =       0; 
	MemWrite =       0;
	RegWrite =       0;
	switch(opcode) {
		case 0:
			switch(r0500) {
				case ADD: 	
					ALUOp    = ADD; 	
					RegWrite = 1;
					break;
				case ADDU: 	
					ALUOp    = ADDU;
					RegWrite = 1; 	
					break;
				case SUB: 	
					ALUOp    = SUB; 
					RegWrite = 1;	
					break;
				case AND: 	
					ALUOp    = AND; 
					RegWrite = 1;	
					break;
				case OR: 	
					ALUOp    = OR; 
					RegWrite = 1;	
					break;
				case XOR: 	
					ALUOp    = XOR; 
					RegWrite = 1;	
					break;
				case NOR: 	
					ALUOp    = NOR; 
					RegWrite = 1;	
					break;
				case NAND: 	
					ALUOp    = NAND;
					RegWrite = 1; 	
					break;
				case SLT: 	
					ALUOp    = COMP;
					RegWrite = 1; 	
					break;
				case SLL: 	
					ALUOp    = SLL; 
					RegWrite = 1;	
					break;
				case SRL: 	
					ALUOp    = SRL; 
					RegWrite = 1;	
					break;
				case SRA: 	
					ALUOp    = SRA; 
					RegWrite = 1;	
					break;
				case JR: 	
					ALUOp    = NONE;
					break;
				case MULT: 	
					ALUOp    = MULT;
					break;
				case MULTU: 
					ALUOp    = MULTU; 	
					break;
				case MFHI: 	
					ALUOp    = NONE; 	
					RegWrite = 1;
					break;
				case MFLO: 	
					ALUOp    = NONE;
					RegWrite = 1; 	
					break;
			}
			break;
		case ADDI: 	
			ALUOp    = ADD; 	
			RegWrite = 1;
			break;
		case ADDIU: 
			ALUOp    = ADDU;
			RegWrite = 1; 	
			break;
		case LW: 	
			ALUOp    = ADD; 
			RegWrite =   1;	
			MemRead  = 114; 
			break;
		case LH: 	
			ALUOp    = ADD; 
			RegWrite =   1;	
			MemRead  = 112; 
			break;
		case LHU: 	
			ALUOp    = ADD; 
			RegWrite =   1;	
			MemRead  = 122; 
			break;
		case LB: 	
			ALUOp    = ADD; 
			RegWrite =   1;	
			MemRead  = 111;
			break;
		case LBU: 	
			ALUOp    = ADD; 
			RegWrite =   1;	
			MemRead  = 121;
			break;
		case SW: 	
			ALUOp    = ADD; 
			MemWrite = 214;
			break;
		case SH: 	
			ALUOp    = ADD;
			MemWrite = 212;
			break;
		case SB: 	
			ALUOp    = ADD;
			MemWrite = 211;
			break;
		case LUI: 	
			ALUOp    = LUI;
			RegWrite = 1; 	
			break;
		case ANDI: 	
			ALUOp    = AND; 
			RegWrite = 1;	
			break;
		case ORI: 	
			ALUOp    = OR; 
			RegWrite = 1;	
			break;
		case NORI:  	
			ALUOp    = NOR; 
			RegWrite = 1;
			break;
		case SLTI: 	
			ALUOp    = COMP; 	
			RegWrite = 1;
			break;
		case BEQ: 	
			ALUOp    = NONE; 	
			break;
		case BNE: 	
			ALUOp    = NONE;
			break;
		case BGTZ: 	
			ALUOp    = NONE;
			break;
		case J:     
			ALUOp    = NONE;
			break;
		case JAL:   
			ALUOp    = NONE;
			break;
	}
}

// global variable: fout, ferr
inline void init() {
	fout = fopen("snapshot.rpt", "wb");
	ferr = fopen("error_dump.rpt", "wb");
	if (!fout) printf("snapshot.rpt cannot be opened!\n");
	if (!ferr) printf("error_dump.rpt cannot be opened!\n");
	read_data();
}

// output: regi[], memi[], memd[], PC
inline void read_data() {
	char buff[4];
	int sizeofchar = sizeof(char);
	int tmp, data_size;
	FILE *iimage, *dimage;
	iimage = fopen("iimage.bin", "rb");
	dimage = fopen("dimage.bin", "rb");
	
	if (!dimage) {
		printf("No dimage.bin\n");
		finalize(1);
	}
	if (!iimage) {
		printf("No iimage.bin\n");
		finalize(1);
	}
	
	// read dimage.bin
	tmp = fread(buff, sizeofchar, 4, dimage); // read initial SP address
    if (tmp == 0) printf("Read ERROR!\n");
	regi[29] = regi2[29] =  (((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));
	tmp = fread(buff, sizeofchar, 4, dimage); // read total number of data
	data_size = (((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));
	tmp = fread(memd, sizeofchar, data_size * 4, dimage);
	
	// read iimage.bin
	tmp = fread(buff, sizeofchar, 4, iimage); // read initial PC address
	PC = (((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));
	tmp = fread(buff, sizeofchar, 4, iimage); // read total instruction number
	data_size = (((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));
	for (int i = PC/4; fread(buff, sizeofchar, 4, iimage); i++) {
		memi[i] = (((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));
	}
}

// input: regi[]. HI, LO, PC
inline void print_all() {
    oprintf("cycle 0\n");
    for (int i = 0; i < 32; i++) {
    	oprintf("$%02d: 0x%08X\n", i, regi[i]);
    }
    oprintf("$HI: 0x%08X\n", HI);
    oprintf("$LO: 0x%08X\n", LO);
    oprintf("PC: 0x%08X\n\n\n", PC);
}

// input: regi[]. HI, LO, PC
inline void print_diff() {
	oprintf("cycle %d\n", cycle);
    for (int i = 0; i < 32; i++) {
    	if (regi[i] != regi2[i]) {
    		oprintf("$%02d: 0x%08X\n", i, regi[i]);
    		regi2[i] = regi[i];
    	}
    }
    if (HI != HI2) {
    	oprintf("$HI: 0x%08X\n", HI);
    	HI2 = HI;
    }
    if (LO != LO2) {
    	oprintf("$LO: 0x%08X\n", LO);
    	LO2 = LO;
    }
    oprintf("PC: 0x%08X\n\n\n", PC);
}

// global: errorflag_write0, errorflag_overflow, errorflag_overwrite, 
//         errorflag_memoryOverflow, errorflag_missingAlign
inline void print_error() {
	if (errorflag_write0) {
		eprintf("In cycle %d: Write $0 Error\n", cycle);
	    errorflag_write0=false;
	}
	if (errorflag_overflow) {
		eprintf("In cycle %d: Number Overflow\n", cycle);
	    errorflag_overflow=false;
	}
	if (errorflag_overwrite) {
		eprintf("In cycle %d: Overwrite HI-LO registers\n", cycle);
	    errorflag_overwrite=false;
	}
	if (errorflag_memoryOverflow) {
		eprintf("In cycle %d: Address Overflow\n", cycle);
	}
	if (errorflag_missingAlign) {
		eprintf("In cycle %d: Misalignment Error\n", cycle);
	}
	if (errorflag_memoryOverflow || errorflag_missingAlign) {
	    finalize(1);
	}
}

void finalize(int n) {
	fclose(ferr);
	fclose(fout);
	exit(n);
}
