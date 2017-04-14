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
		
		print_diff();
	} while(0);
	finalize(0);
	return 0;
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

//======================================================== inline void WB_State() =========
inline void WB_State() {
	#ifdef log
		printf(">>> WB_State()\n");
	#endif
	//==================================================== mux =========
	if (MEMWB_MemtoReg) {
		write_back = MEMWB_ALU_Result;
	} else {
		write_back = MEMWB_Data_Result;
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
	int ALU_input1, ALU_input2;
	//==================================================== MUX1 =========
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
	//==================================================== MUX2 =========
		switch(FU_output_mux2) {
			case 0:
				ALU_input2 = IDEX_Register_Result[1];
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
	//==================================================== MUX3 =========
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
	//==================================================== MUX4 =========
		switch(HD_output_EX_flush) {
			case 0:
				EXMEM_MemW = IDEX_MemW;
				EXMEM_Branch = IDEX_Branch;
				EXMEM_MemtoReg = IDEX_MemtoReg;
				EXMEM_RegWr = IDEX_RegWr;
				break;
			case 1:
				EXMEM_MemW = 0;
				EXMEM_Branch = 0;
				EXMEM_MemtoReg = 0;
				EXMEM_RegWr = 0;
				break;
			default:
				printf("<<< ERROR! invalid judge code in /EX_State/MUX4\n");
				finalize(1);
				break;
		}
	
	ALU();
}

//======================================================== inline void ID_State() =========
inline void ID_State() {
	
}

//======================================================== inline void IF_State() =========
inline void IF_State() {
	
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

//======================================================== ALU =========
inline void ALU() {
	switch (IDEX_ALUOp) {
		case ADD:
			
			break;
		case ADDU:
			
			break;
		case SUB:
			
			break;
		case MULT:
			
			break;
		case MULTU:
			
			break;
		case AND:
			
			break;
		case OR:
			
			break;
		case NAND:
			
			break;
		case NOR:
			
			break;
		case XOR:
			
			break;
		case SHIFT_LEFT:
			
			break;
		case SHIFT_RIGHT:
			
			break;
		case SHIFT_RIGHT_ARITH:
			
			break;
		default:
			
			break;
	}
}
