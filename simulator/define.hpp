// #define debug

#include <cstdio>
#include <cstdlib>

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

int regi[32];
int memi[256];
char memd[1024];
int total_ins;
int PC;
int HI;
int LO;
int cycle;
int regi2[32];
int PC2;
int HI2;
int LO2;
int command;
int mem_limit;
int PC_offset;
int PC_limit;
int SP_offset;

void finalize(int n);

inline void init();
	FILE *fout, *ferr;

inline void read_data();

inline void print_all();

inline void print_diff();

inline void print_error();
	bool moveflag_HI=true;
	bool moveflag_LO=true;
	bool errorflag_write0=false;
	bool errorflag_overflow=false;
	bool errorflag_overwrite=false;
	bool errorflag_memoryOverflow=false;
	bool errorflag_missingAlign=false;

inline void decoder();
	int opcode;
	int r2521;
	int r2016;
	int r1511;
	int r1006;
	int r2500;
	int r1500;
	int r0500;

inline void control();
	int RegDst;
	int MemRead;
	int MemtoReg;
	int ALUOp;
	int MemWrite;
	int ALUSrc;
	int RegWrite;
	int Branch;
	int Jump;
	int cmp;

inline void regi_read();
	int regi_output1;
	int regi_output2;
	
inline void sign_extend();
	int extend_result;
	
inline void ALU();
	int ALU_input1;
	int ALU_input2;
	int ALU_result;

inline void data_memory();
	int DM_result;

inline void regi_write();
	int regi_input;

inline void PC_control();

// api
inline int scan_command(int start, int end) {
    unsigned int b=0xFFFFFFFF;
    return (command>>end)&(b>>(end+31-start));
}
inline int memd2reg1(int start) {return memd[start];
}
inline int memd2reg2(int start) {
	int tmp=(((unsigned char)memd[start])<<8)|((unsigned char)memd[start+1]);
	if (tmp<0x00008000) {
	    return tmp;
	} else {
	    return tmp+0xffff0000;   
	}
}
inline int memd2reg4(int start) {
	return ((((unsigned char)memd[start  ])<<24)|
	        (((unsigned char)memd[start+1])<<16)|
	        (((unsigned char)memd[start+2])<< 8)|
	        (((unsigned char)memd[start+3])));
}
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
inline void log_command() {
	// #define logcommand
	
	#ifdef logcommand
		#define lprintf(...) fprintf(fout, __VA_ARGS__)
	#else
		#define lprintf printf
		printf("\n\ncycle = %d\n", cycle);
		printf("PC = 0x%08X\n", PC);
	#endif

	lprintf("command = 0x%08X\n", command);
	switch(opcode) {
		case 0:
			switch(r0500) {
				case ADD: 	lprintf("%s $%d, $%d, $%d\n", "add", 	r1511, r2521, r2016); break;
				case ADDU: 	lprintf("%s $%d, $%d, $%d\n", "addu", 	r1511, r2521, r2016); break;
				case SUB: 	lprintf("%s $%d, $%d, $%d\n", "sub", 	r1511, r2521, r2016); break;
				case AND: 	lprintf("%s $%d, $%d, $%d\n", "and", 	r1511, r2521, r2016); break;
				case OR: 	lprintf("%s $%d, $%d, $%d\n", "or", 	r1511, r2521, r2016); break;
				case XOR: 	lprintf("%s $%d, $%d, $%d\n", "xor", 	r1511, r2521, r2016); break;
				case NOR: 	lprintf("%s $%d, $%d, $%d\n", "nor", 	r1511, r2521, r2016); break;
				case NAND: 	lprintf("%s $%d, $%d, $%d\n", "nand",	r1511, r2521, r2016); break;
				case SLT: 	lprintf("%s $%d, $%d, $%d\n", "slt", 	r1511, r2016, r2016); break;
				case SLL: 	lprintf("%s $%d, $%d,  %d\n", "sll", 	r1511, r2016, r1006); break;
				case SRL: 	lprintf("%s $%d, $%d,  %d\n", "srl", 	r1511, r2016, r1006); break;
				case SRA: 	lprintf("%s $%d, $%d,  %d\n", "sra", 	r1511, r2016, r1006); break;
				case JR: 	lprintf("%s $%d          \n", "jr", 	r2521              ); break;
				case MULT: 	lprintf("%s $%d, $%d     \n", "mult", 	r2521, r2016       ); break;
				case MULTU: lprintf("%s $%d, $%d     \n", "multu", 	r2521, r2016       ); break;
				case MFHI: 	lprintf("%s $%d          \n", "mfhi", 	r1511              ); break;
				case MFLO: 	lprintf("%s $%d          \n", "mflo", 	r1511              ); break;
			}
			break;
		case ADDI: 	lprintf("%s $%d, $%d, %d\n", 	"addi", r2016, r2521, r1500); break;
		case ADDIU: lprintf("%s $%d, $%d, %d\n", 	"addiu",r2016, r2521, r1500); break;
		case LW: 	lprintf("%s $%d, $%d, %d\n", 	"lw", 	r2016, r2521, r1500); break;
		case LH: 	lprintf("%s $%d, $%d, %d\n", 	"lh", 	r2016, r2521, r1500); break;
		case LHU: 	lprintf("%s $%d, $%d, %d\n", 	"lhu", 	r2016, r2521, r1500); break;
		case LB: 	lprintf("%s $%d, $%d, %d\n", 	"lb", 	r2016, r2521, r1500); break;
		case LBU: 	lprintf("%s $%d, $%d, %d\n", 	"lbu", 	r2016, r2521, r1500); break;
		case SW: 	lprintf("%s $%d, $%d, %d\n", 	"sw", 	r2016, r2521, r1500); break;
		case SH: 	lprintf("%s $%d, $%d, %d\n", 	"sh", 	r2016, r2521, r1500); break;
		case SB: 	lprintf("%s $%d, $%d, %d\n", 	"sb", 	r2016, r2521, r1500); break;
		case LUI: 	lprintf("%s $%d,  %d    \n", 	"lui", 	r2016, r1500       ); break;
		case ANDI: 	lprintf("%s $%d, $%d, %d\n", 	"andi", r2016, r2521, r1500); break;
		case ORI: 	lprintf("%s $%d, $%d, %d\n", 	"ori", 	r2016, r2521, r1500); break;
		case NORI: 	lprintf("%s $%d, $%d, %d\n", 	"nori", r2016, r2521, r1500); break;
		case SLTI: 	lprintf("%s $%d, $%d, %d\n", 	"slti", r2016, r2521, r1500); break;
		case BEQ: 	lprintf("%s $%d, $%d, %d\n", 	"beq", 	r2016, r2521, r1500); break;
		case BNE: 	lprintf("%s $%d, $%d, %d\n", 	"bne", 	r2016, r2521, r1500); break;
		case BGTZ: 	lprintf("%s $%d,  %d    \n", 	"bgtz", r2521, r1500       ); break;
		case J:     lprintf("%s             \n", 	"halt"                     ); break;
		case JAL:   lprintf("%s             \n", 	"halt"                     ); break;
	}
}

#ifdef debug
	#define oprintf(...) printf(__VA_ARGS__)
	#define eprintf(...) printf(__VA_ARGS__)
#else
	#define oprintf(...) fprintf(fout, __VA_ARGS__)
	#define eprintf(...) fprintf(ferr, __VA_ARGS__)
#endif