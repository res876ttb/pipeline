// if define log: print out data path
#define log
#define debug

// define ALUop
#define ADD 				11
#define ADDU				12
#define SUB					21
#define MULT 				31
#define MULTU 				32
#define AND 				41
#define OR 					51
#define NAND 				61
#define NOR 				71
#define XOR 				81
#define SHIFT_LEFT 			91
#define SHIFT_RIGHT 		101
#define SHIFT_RIGHT_ARITH 	102


// variable definition
	FILE *fout, *ferr;
	int regi[32];
	int regi2[32];
	int memi[256];
	char memd[1024];
	int PC, PC2;
	int HI, HI2;
	int LO, LO2;
	int cycle;
	int command;
	
// Error detection
	bool errorflag_write0=false;
	bool errorflag_overflow=false;
	bool errorflag_overwrite=false;
	bool errorflag_memoryOverflow=false;
	bool errorflag_missingAlign=false;
	
// IF/ID
	int IFID_NPC;
	int IFID_Instruction_code;
// ID/EX
	int IDEX_Register_Result[2];
	int IDEX_Signed_extend;
	int IDEX_Register_Rs;
	int IDEX_Register_Rt1;
	int IDEX_Register_Rt2;
	int IDEX_Register_Rd;
	int IDEX_ExtOp;
	int IDEX_ALUSrc;
	int IDEX_ALUOp;
	int IDEX_RegDst;
	int IDEX_MemW;
	int IDEX_Branch;
	int IDEX_MemtoReg;
	int IDEX_RegWr;
// EX/MEM
	int EXMEM_MemW;
	int EXMEM_Branch;
	int EXMEM_MemtoReg;
	int EXMEM_RegWr;
	int EXMEM_ALU_Result;
	int EXMEM_ALU_Overflow;
	int EXMEM_ALU_input2;
	int EXMEM_Register_WB_Number;
// MEM/WB
	int MEMWB_MemtoReg;
	int MEMWB_RegWr;
	int MEMWB_Data_Result;
	int MEMWB_ALU_Result;
	int MEMWB_Register_WB_Number;

	int write_back;
	
// Hazard detection
	int HD_output_PC_Stall;
	int HD_output_ID_flush;
	int HD_output_EX_flush;
	int HD_input_Instruction_code;
	int HD_input_r2016;
	// int HD_input_???

// forwarding unit
	int FU_input_WB_Rd;
	int FU_input_WB_MemtoReg;
	int FU_input_DM_Rd;
	int FU_input_DM_MemtoReg;
	int FU_input_IDEX_Rs;
	int FU_input_IDEX_rt;
	int FU_output_mux1;
	int FU_output_mux2;

// functions definition
inline void init();
inline void read_data();
inline void print_all();
inline void print_diff();
inline void finalize(int n);
inline void WB_State();
inline void DM_State();
inline void EX_State();
inline void ID_State();
inline void IF_State();
inline void ALU();
inline int memd2reg(int start, int len);

// define macro setting
// __VA_ARGS__ means a changable variable. the following code is example
// see more example at http://www.cash.idv.tw/wordpress/?p=1531
#ifdef debug
	#define oprintf(...) printf(__VA_ARGS__)
	#define eprintf(...) printf(__VA_ARGS__)
#else
	#define oprintf(...) fprintf(fout, __VA_ARGS__)
	#define eprintf(...) fprintf(ferr, __VA_ARGS__)
#endif