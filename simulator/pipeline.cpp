#include "define.hpp"

int main() {
	init();
	print_all();
	
	IF_Stage();
	
	do {
		cycle++;

		WB_Stage();
		DM_Stage();
		EX_Stage();
		ID_Stage();
		IF_Stage();
		
		print_error();
		print_diff();
		
		if (IDEX_Stall > 0) IDEX_Stall--;
		if (IFID_Stall > 0) IFID_Stall--;
		
		if (WB_State == DM_State && DM_State == EX_State && EX_State == ID_State && 
		    scan_command(31,26) == 0x3F) {
			break;
		}
		#ifdef pause
			printf("Press enter to continue...\n");
			getchar();
		#endif
	} while(true);
	finalize(0);
	return 0;
}

//======================================================== inline void WB_Stage() =========
inline void WB_Stage() {
	#ifdef log
		printf(">>> WB_Stage()\n");
		printf(">>> cycle = %d\n", cycle);
	#endif
		
	WB_State = DM_State;
	if (WB_State == "NOP") return;
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
		printf(">>> WB_Stage() done\n");
	#endif
}

//======================================================== inline void DM_Stage() =========
inline void DM_Stage() {
	#ifdef log
		printf(">>> DM_Stage()\n");
	#endif
	
	DM_State = EX_State;
	
	//==================================================== forwarding unit input =========
	FU_input_DM_Rd = EXMEM_Register_WB_Number;
	FU_input_DM_MemtoReg = EXMEM_MemtoReg;
	
	//==================================================== pass control signal to WB =========
	MEMWB_RegWrB = MEMWB_RegWr;
	MEMWB_MemtoReg = EXMEM_MemtoReg;
	MEMWB_RegWr = EXMEM_RegWr;
	
	if (DM_State == "NOP") return;
	
	DM_DataMemory();
	
	MEMWB_Register_WB_Number = EXMEM_Register_WB_Number;
	
	#ifdef log
		printf(">>> DM_Stage() done\n");
	#endif
}

//======================================================== inline void EX_Stage() =========
inline void EX_Stage() {
	#ifdef log
		printf(">>> EX_Stage()\n");
	#endif
	
	EX_State = ID_StateN;
	EXMEM_MemWB = EXMEM_MemW;
	EXMEM_MemtoRegB = EXMEM_MemtoReg;
	EX_mux3();
	
	int ALU_input1, ALU_input2;
	forwarding_unit();
	EX_mux1();
	EX_mux2();
	EX_mux4();
	ALU();
	
	#ifdef log
		printf(">>> EX_Stage() done\n");
	#endif
}

//======================================================== inline void ID_Stage() =========
inline void ID_Stage() {
	#ifdef log
		printf(">>> ID_Stage()\n");
	#endif
	if (!IDEX_Stall && !IDEX_Flush) {
		ID_State = IF_State;
	}
	
	IDEX_Register_RtB = IDEX_Register_Rt;
	IDEX_Register_RdB = IDEX_Register_Rd;
	IDEX_Register_Rs  = r2521;
	IDEX_Register_Rt  = r2016;
	IDEX_Register_Rt  = r2016;
	IDEX_Register_Rd  = r1511;

	hazard_detector();
	controller();
	registers();
	
	if (IDEX_Flush) {
		ID_StateN     = "NOP";
		IDEX_ALUSrc   = FLUSH;
		IDEX_ALUOp    = FLUSH;
		IDEX_RegDst   = FLUSH;
		IDEX_MemW     = FLUSH;
		IDEX_Branch   = FLUSH;
		IDEX_MemtoReg = FLUSH;
		IDEX_RegWr 	  = FLUSH;
	} else {
		if (IDEX_Stall) {
			ID_StateN = "NOP";
		} else {
			ID_StateN = ID_State;
		}
	}
	
	write_backN = write_back;
	MEMWB_Register_WB_NumberN = MEMWB_Register_WB_Number;
	
	#ifdef log
		printf(">>> ID_Stage() done\n");
	#endif
}

//======================================================== inline void IF_Stage() =========
inline void IF_Stage() {
	#ifdef log
		printf(">>> IF_Stage()\n");
	#endif
	
	// printf("in cycle %d, IFID_Stall =  %d, IFID_Stall_P = %d\n", cycle, IFID_Stall, IFID_Stall_P);
	
	if (!IFID_Stall_P) {
		// printf("In cycle %d, PCSel = %d\n", cycle, PCSel);
		if (PCSel == 1) {
			PC = IDEX_PC;
		} else if (PCSel == 2) {
			PC = 0x80000180;
		} else if (PCSel == 3) {
			PC = IFID_NPC;
		}
	}
	IFID_NPC = PC + 4;
	command = memi[PC / 4];	
	decoder();
	set_state();
	IFID_Stall_P = IFID_Stall;
	
	
	#ifdef log
		printf(">>> IF_Stage() done\n");
	#endif
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
	
	WB_State  = "NOP";
	DM_State  = "NOP";
	EX_State  = "NOP";
	ID_State  = "NOP";
	ID_StateN = "NOP";
	IF_State  = "NOP";
	
	PCSel = 3;
	IFID_NPC = PC;

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

	oprintf("PC: 0x%08X\n", PC);
	
	oprintf("IF: 0x%08X", command);
	if (IFID_Stall_P || IFID_Stall) {
		oprintf(" to_be_stalled\n");
	} else if (IFID_Flush) {
		oprintf(" to_be_flushed\n");
	} else {
		oprintf("\n");
	}
	
	oprintf("ID: %s", ID_State.c_str());
	if (IDEX_Stall) {
		oprintf(" to_be_stalled\n");
	} else if (IDEX_Flush) {
		oprintf(" to_be_flushed\n");
	} else if (forwarding) {
		// oprintf("\n");
		oprintf(" fwd_EX-DM_%s_$%d\n", FU_source_type, FU_source_register);
	} else {
		oprintf("\n");
	}
	
	oprintf("EX: %s\n", EX_State.c_str());
	oprintf("DM: %s\n", DM_State.c_str());
	oprintf("WB: %s\n", WB_State.c_str());
	oprintf("\n\n");
}

//======================================================== inline void print_error() =========
inline void print_error() {
	// print error messages
	if (errorflag_write0) {
		eprintf("In cycle %d: Write $0 Error\n", cycle);
	    // eout<<"In cycle "<<dec<<cycle<<": Write $0 Error"<<endl;
	    errorflag_write0=false;
	}
	
	if (errorflag_overflow) {
		eprintf("In cycle %d: Number Overflow\n", cycle);
	    // eout<<"In cycle "<<dec<<cycle<<": Number Overflow"<<endl;
	    errorflag_overflow=false;
	}
	
	if (errorflag_overwrite) {
		eprintf("In cycle %d: Overwrite HI-LO registers\n", cycle);
	    // eout<<"In cycle "<<dec<<cycle<<": Overwrite HI-LO registers"<<endl;
	    errorflag_overwrite=false;
	}
	
	if (errorflag_memoryOverflow) {
		eprintf("In cycle %d: Address Overflow\n", cycle);
	    // eout<<"In cycle "<<dec<<cycle<<": Address Overflow"<<endl;
	}
	
	if (errorflag_missingAlign) {
		eprintf("In cycle %d: Misalignment Error\n", cycle);
	    // eout<<"In cycle "<<dec<<cycle<<": Misalignment Error"<<endl;
	}
	
	if (errorflag_memoryOverflow || errorflag_missingAlign) {
	    finalize(1);
	}
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
		
		if (errorflag_memoryOverflow || errorflag_missingAlign) {
			return;
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
	// printf("input1 = %d, input2 = %d, ALUOP = %d\n", ALU_input1, ALU_input2, IDEX_ALUOp);
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
			ALU_result = 0;
			break;
		default:
			printf("<<< ERROR! invalid IDEX_ALUOp in /EX_Stage/ALU()\n");
			finalize(1);
			break;
	}
	EXMEM_ALU_Result = ALU_result;
	
	#ifdef log
	printf("done\n");
	#endif
}

//======================================================== void inline forwarding_unit() =========
inline void forwarding_unit() {
	if (opcode != HALT) {
		if (EXMEM_RegWr && EXMEM_Register_WB_Number != 0) {
			if (EXMEM_Register_WB_Number == IDEX_Register_Rs) {
				forwarding = 1;
				FU_source_register = IDEX_Register_Rs;
				FU_source_type = rs;
				FU_output_mux1 = 2;
				FU_output_mux2 = 0;
			} else if (EXMEM_Register_WB_Number == IDEX_Register_Rt) {
				forwarding = 1;
				FU_source_register = IDEX_Register_Rt;
				FU_source_type = rt;
				FU_output_mux1 = 0;
				FU_output_mux2 = 2;
			} else {
				forwarding = 0;
				FU_output_mux1 = 0;
				FU_output_mux2 = 0;
			}
		} else if (MEMWB_RegWrB && MEMWB_Register_WB_Number != 0) {
			if (MEMWB_Register_WB_Number == IDEX_Register_Rs) {
				forwarding = 1;
				FU_source_register = IDEX_Register_Rs;
				FU_source_type = rs;
				FU_output_mux1 = 1;
				FU_output_mux2 = 0;
			} else if (MEMWB_Register_WB_Number == IDEX_Register_Rt) {
				forwarding = 1;
				FU_source_register = IDEX_Register_Rt;
				FU_source_type = rt;
				FU_output_mux1 = 0;
				FU_output_mux2 = 1;	
			} else {
				forwarding = 0;
				FU_output_mux1 = 0;
				FU_output_mux2 = 0;	
			}
		} else {
			forwarding = 0;
			FU_output_mux1 = 0;
			FU_output_mux2 = 0;	
		}	
	} else {
		forwarding = 0;
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
			printf("<<< ERROR! invalid judge code in /EX_Stage/MUX1\n");
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
			printf("<<< ERROR! invalid judge code in /EX_Stage/MUX2\n");
			finalize(1);
			break;
	}
}

//======================================================== inline void EX_mux3() =========
inline void EX_mux3() {
	if (IDEX_RegDst) {
		EXMEM_Register_WB_Number = IDEX_Register_Rd;
	} else {
		EXMEM_Register_WB_Number = IDEX_Register_Rt;
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
			printf("<<< ERROR! invalid judge code in /EX_Stage/MUX4\n");
			finalize(1);
			break;
	}
}

//======================================================== inline void hazard_detector() =========
inline void hazard_detector() {
	// TODO: need to specify the branch more clearly
	if (IDEX_Branch) {
		if ((opcode == BEQ  &&  reg_output_equal) || 
		    (opcode == BNE  && !reg_output_equal) || 
		    (opcode == BGTZ && (r2521 > 0))) {
			IFID_Flush = 1;
		} else if (opcode == J || opcode == JAL) {
			IFID_Flush = 1;
		} else {
			IFID_Flush = 0;
		}
		PCSel = 1;
	} else {
		PCSel = 3;
	}
	
	// TODO
	HD_output_EX_flush = 0;
	
	#ifdef debug
	printf(">>> cycle = %d\n", cycle);
	printf(">>> opcode = %X, %X\n", opcode, BEQ);
	// printf(">>> command = 0x%08X\n", command);
	printf(">>> EXMEM_MemtoRegB = %d, EXMEM_MemWB = %d\n", EXMEM_MemtoRegB, EXMEM_MemWB);
	printf(">>> EXMEM_MemtoReg  = %d, EXMEM_MemW  = %d\n", EXMEM_MemtoReg , EXMEM_MemW );
	printf(">>> IDEX_Register_RtB = %d, IDEX_Register_Rs = %d, \n    IDEX_Register_Rt = %d, IDEX_Register_Rd = %d,\n    IDEX_Register_RdB = %d\n\n", IDEX_Register_RtB, IDEX_Register_Rs, IDEX_Register_Rt, IDEX_Register_Rd, IDEX_Register_RdB);
	printf(">>><<< judge: %d\n", (IDEX_MemW / 100 == 1));
	#endif
	
	if (cycle == 3) printf("IDEX_Stall = %d\n", IDEX_Stall);
	if (EXMEM_MemtoReg || EXMEM_MemtoRegB) {
		if ((EXMEM_MemW / 100 == 1) && (
		    (IDEX_Register_RtB == IDEX_Register_Rs) ||
		    (IDEX_Register_RtB == IDEX_Register_Rt) ||
		    ((opcode == BEQ || opcode == BNE || opcode == BGTZ) && (
		      (IDEX_Register_RdB == IDEX_Register_Rs) ||
		      (IDEX_Register_RdB == IDEX_Register_Rt))))) {
			#ifdef debug
			printf("================= stall =================\n");
			printf("In cycle %d\n", cycle);
			#endif
			IFID_Stall = 3;
			IDEX_Stall = 3;
		} else if ((EXMEM_MemWB / 100 == 1) && (
		    (IDEX_Register_RtB == IDEX_Register_Rs) ||
		    (IDEX_Register_RtB == IDEX_Register_Rt) ||
		    ((opcode == BEQ || opcode == BNE || opcode == BGTZ) && (
		      (IDEX_Register_RdB == IDEX_Register_Rs) ||
		      (IDEX_Register_RdB == IDEX_Register_Rt))))) {
			#ifdef debug
			printf("================= stall =================\n");
			printf("In cycle %d\n", cycle);
			#endif
			IFID_Stall = 2;
			IDEX_Stall = 2;
		} else {
			IFID_Stall = 0;
			IDEX_Stall = 0;
		}
	} else {
		IFID_Stall = 0;
		IDEX_Stall = 0;
	}
	if (cycle == 3) printf("IDEX_Stall = %d\n", IDEX_Stall);
}

//======================================================== inline void controller() =========
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
	
	// TODO: flush signal and next pc signal
}

inline void decoder() {
	opcode = scan_command(31,26);
	r2521  = scan_command(25,21);
	r2016  = scan_command(20,16);
	r1511  = scan_command(15,11);
	r1006  = scan_command(10, 6);
	r0500  = scan_command( 5, 0);
	r2500  = scan_command(25, 0);
	r1500  = scan_command(15, 0);
}

inline void registers() {
	// printf("cycle %d in registers(), MEMWB_Register_WB_NumberN = %d\n", cycle, MEMWB_Register_WB_NumberN);
	// if (cycle == 5) {
		// printf("cycle %d in registers(), input data = 0x%08X\n", cycle, write_backN);
	// }
	
	if (registers_tmp1) {
		if (registers_tmp2 == 0 && registers_tmp4 != "NOP") {
			errorflag_write0 = true;
		} else {
			// printf("cycle %d in registers(), input data = 0x%08X\n", cycle, registers_tmp2);
			regi[registers_tmp2] = registers_tmp3;
		}
	}
	IDEX_Register_Result[0] = regi[r2521];
	IDEX_Register_Result[1] = regi[r2016];
	reg_output_equal = (IDEX_Register_Result[0] == IDEX_Register_Result[1]);
	registers_tmp1 = MEMWB_RegWrB;
	registers_tmp2 = MEMWB_Register_WB_NumberN;
	registers_tmp3 = write_back;
	registers_tmp4 = WB_State;
}

inline int scan_command(int start, int end) {
    unsigned int b=0xFFFFFFFF;
    return (command>>end)&(b>>(end+31-start));
}

inline void set_state() {
	switch(opcode) {
		case 0: // R type instruction
			switch(r0500) {
				case ADD: 	IF_State = "ADD"; 	break;
				case ADDU: 	IF_State = "ADDU"; 	break;
				case SUB: 	IF_State = "SUB"; 	break;
				case AND: 	IF_State = "AND"; 	break;
				case OR: 	IF_State = "OR"; 	break;
				case XOR: 	IF_State = "XOR"; 	break;
				case NOR: 	IF_State = "NOR"; 	break;
				case NAND: 	IF_State = "NAND"; 	break;
				case SLT: 	IF_State = "SLT"; 	break;
				case SLL: 	IF_State = "SLL"; 	break;
				case SRL: 	IF_State = "SRL"; 	break;
				case SRA: 	IF_State = "SRA"; 	break;
				case JR: 	IF_State = "JR"; 	break;
				case MULT: 	IF_State = "MULT"; 	break;
				case MULTU:	IF_State = "MULTU";	break;
				case MFHI: 	IF_State = "MFHI"; 	break;
				case MFLO: 	IF_State = "MFLO"; 	break;
			}
			break;
		case ADDI: 	IF_State = "ADDI"; 	break;
		case ADDIU: IF_State = "ADDIU"; break;
		case LW: 	IF_State = "LW"; 	break;
		case LH: 	IF_State = "LH"; 	break;
		case LHU: 	IF_State = "LHU"; 	break;
		case LB: 	IF_State = "LB"; 	break;
		case LBU: 	IF_State = "LBU"; 	break;
		case SW: 	IF_State = "SW"; 	break;
		case SH: 	IF_State = "SH"; 	break;
		case SB: 	IF_State = "SB"; 	break;
		case LUI: 	IF_State = "LUI"; 	break;
		case ANDI: 	IF_State = "ANDI"; 	break;
		case ORI: 	IF_State = "ORI"; 	break;
		case NORI: 	IF_State = "NORI"; 	break;
		case SLTI: 	IF_State = "SLTI"; 	break;
		case BEQ: 	IF_State = "BEQ"; 	break;
		case BNE: 	IF_State = "BNE"; 	break;
		case BGTZ: 	IF_State = "BGTZ"; 	break;
		case J: 	IF_State = "J"; 	break;
		case JAL: 	IF_State = "JAL"; 	break;
		case HALT: 	IF_State = "HALT"; 	break;
	}
}
