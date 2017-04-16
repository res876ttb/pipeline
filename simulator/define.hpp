// if define log: print out data path
#define log
#define debug

// define ALUop and decoder
#define SHIFT_LEFT 			0x00
#define SHIFT_RIGHT 		0x02
#define SHIFT_RIGHT_ARITH 	0x03
#define nothing             123456789
#define COMPARE 			100000
#define ADD 	0x20
#define ADDU 	0x21
#define SUB 	0x22
#define AND  	0x24
#define OR 		0x25
#define XOR 	0x26
#define NOR 	0x27
#define NAND 	0x28
#define SLT 	0x2A
#define SLL 	0x00
#define SRL 	0x02
#define SRA 	0x03
#define JR 		0x08
#define MULT 	0x18
#define MULTU 	0x19
#define MFHI 	0x10
#define MFLO 	0x12
#define ADDI 	0x08
#define ADDIU 	0x09
#define LW 		0x23
#define LH 		0x21
#define LHU 	0x25
#define LB 		0x20
#define LBU 	0x24
#define SW 		0x2B
#define SH 		0x29
#define SB 		0x28
#define LUI 	0x0F
#define ANDI 	0x0C
#define ORI 	0x0D
#define NORI 	0x0E
#define SLTI 	0x0A
#define BEQ 	0x04
#define BNE 	0x05
#define BGTZ 	0x07
#define J 		0x02
#define JAL		0x03
#define HALT	0x3F

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
	bool errorflag_write0 = false;
	bool errorflag_overflow = false;
	bool errorflag_overwrite = false;
	bool errorflag_memoryOverflow = false;
	bool errorflag_missingAlign = false;
	bool moveflag_HI = false;
	bool moveflag_LO = false;
	
// IF/ID
	int IFID_NPC;
	int IFID_PC_input1;
	int IFID_PC_input2;
	int IFID_Instruction_code;
	int IFID_Flush;
	int IFID_Stall;
	int PCSel;
// ID/EX
	int IDEX_PC;
	int IDEX_Register_Result[2];
	int IDEX_Signed_extend;
	int IDEX_Register_Rs;
	int IDEX_Register_Rt1;
	int IDEX_Register_Rt2;
	int IDEX_Register_RtB;
	int IDEX_Register_Rd;
	int IDEX_Flush;
	int IDEX_Stall;
	// int IDEX_ExtOp;
	int IDEX_ALUSrc;
	int IDEX_ALUOp;
	int IDEX_RegDst;
	int IDEX_MemW;
	int IDEX_Branch;
	int IDEX_MemtoReg;
	int IDEX_RegWr;
// EX/MEM
	int EXMEM_Flush;
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
// other
	int write_back;
	int ALU_shift;
	int ALU_input1;
	int ALU_input2;
	int reg_output_equal;
	
// Instruction decoder
	int opcode;
	int r2521;
	int r2016;
	int r1511;
	int r1006;
	int r0500;
	int r2500;
	int r1500;
	
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
inline void print_error();
inline void finalize(int n);
inline void WB_State();
inline void DM_State();
inline void EX_State();
inline void ID_State();
inline void IF_State();
inline void ALU();
inline void forwarding_unit();
inline void EX_mux1();
inline void EX_mux2();
inline void EX_mux3();
inline void EX_mux4();
inline void DM_DataMemory();
inline void hazard_detector();
inline void controller();
inline void registers();
inline void decoder();
inline int scan_command(int start, int end);
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