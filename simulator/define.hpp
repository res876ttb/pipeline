// #define debug

#include <cstdio>
#include <cstdlib>
#include <string>
using namespace std;

#ifndef __BUFR__
#define __BUFR__
typedef struct _bufr {
	bool nop = true;
	bool halt = false;
	int stall = 0;
	string opc = "NOP";
	
	int cmp;        // used to specify if jump to branch
	int opcode, r2521, r2016, r1511, r1006, r2500, r1500, r0500; // output of decoder
	int extend_result;              // signed extend result 
	int regi_output1, regi_output2; // register read result
	int RegDst, RegWrite, MemtoReg, ALUSrc, ALUOp, MemRead, MemWrite, RegNum; // control output
	int ALU_result;
	int DM_result; // Dtype memory access result
} bufr;
#endif

// pipeline buffer
extern bufr ifid, ifidn, ifid0;
extern bufr idex, idexn, idex0;
extern bufr exdm, exdmn, exdm0;
extern bufr dmwb, dmwbn, dmwb0;

#define ADD 	0x020
#define ADDU 	0x021
#define SUB 	0x022
#define AND  	0x024
#define OR 		0x025
#define XOR 	0x026
#define NOR 	0x027
#define NAND 	0x028
#define SLT 	0x02A
#define SLL 	0x000
#define SRL 	0x002
#define SRA 	0x003
#define JR 		0x008
#define MULT 	0x018
#define MULTU 	0x019
#define MFHI 	0x010
#define MFLO 	0x012
#define ADDI 	0x008
#define ADDIU 	0x009
#define LW 		0x023
#define LH 		0x021
#define LHU 	0x025
#define LB 		0x020
#define LBU 	0x024
#define SW 		0x02B
#define SH 		0x029
#define SB 		0x028
#define LUI 	0x00F
#define ANDI 	0x00C
#define ORI 	0x00D
#define NORI 	0x00E
#define SLTI 	0x00A
#define BEQ 	0x004
#define BNE 	0x005
#define BGTZ 	0x007
#define J 		0x002
#define JAL		0x003
#define HALT	0x03F
#define COMP    0x100
#define NONE    0xFFF

// global variables
extern FILE *fout, *ferr;
extern int regi[32];
extern int memi[256];
extern char memd[1024];
extern int total_ins;
extern int PC;
extern int HI;
extern int LO;
extern int cycle;
extern int regi2[32];
extern int PC2;
extern int HI2;
extern int LO2;
extern int command;
extern int mem_limit;
extern int PC_offset;
extern int PC_limit;
extern int SP_offset;
extern bool flush;
extern int stall;

void finalize(int n);

inline void init();

inline void read_data();

inline void print_all();

inline void print_diff();
	extern bool IDexdmf;
	extern bool IDdmwbf;
	extern int  IDrsf;
	extern int  IDrtf;
	extern bool EXexdmf;
	extern bool EXdmwbf;
	extern int  EXrsf;
	extern int  EXrtf;

inline void print_error();
	extern bool moveflag_HI;
	extern bool moveflag_LO;
	extern bool errorflag_write0;
	extern bool errorflag_overflow;
	extern bool errorflag_overwrite;
	extern bool errorflag_memoryOverflow;
	extern bool errorflag_missingAlign;

inline void decoder();
	// int opcode;
	// int r2521;
	// int r2016;
	// int r1511;
	// int r1006;
	// int r2500;
	// int r1500;
	// int r0500;

inline void control();
	// int RegDst;
	// int MemRead;
	// int MemtoReg;
	// int ALUOp;
	// int MemWrite;
	// int ALUSrc;
	// int RegWrite;
	// int Branch;
	// int Jump;
	// int cmp;

inline void regi_read();
	// int regi_output1;
	// int regi_output2;
	
inline void sign_extend();
	// int extend_result;
	
inline void ALU();
	// int ALU_input1;
	// int ALU_input2;
	// int ALU_result;

inline void data_memory();
	// int DM_result;

inline void regi_write();
	// int regi_input;

inline void PC_control();

inline void hazard_unit();

inline void forwarding_EX();

inline void forwarding_ID();

extern int scan_command(int start, int end);
extern int memd2reg1(int start);
extern int memd2reg2(int start);
extern int memd2reg4(int start);
extern int memd2reg(int start, int len);

#ifdef debug
	#define oprintf(...) printf(__VA_ARGS__)
	#define eprintf(...) printf(__VA_ARGS__)
#else
	#define oprintf(...) fprintf(fout, __VA_ARGS__)
	#define eprintf(...) fprintf(ferr, __VA_ARGS__)
#endif