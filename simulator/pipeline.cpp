#include "define.hpp"
#include "IF.hpp"
#include "ID.hpp"
#include "EX.hpp"
#include "DM.hpp"
#include "WB.hpp"

inline void log_command();

// pipeline buffer
bufr ifid, ifidn, ifid0;
bufr idex, idexn, idex0;
bufr exdm, exdmn, exdm0;
bufr dmwb, dmwbn, dmwb0;

FILE *fout, *ferr;
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
bool flush;
int stall;

bool IDexdmf = false;
bool IDdmwbf = false;
int  IDrsf   = 0;
int  IDrtf   = 0;
bool EXexdmf = false;
bool EXdmwbf = false;
int  EXrsf   = 0;
int  EXrtf   = 0;

bool moveflag_HI = false;
bool moveflag_LO = false;
bool errorflag_write0 = false;
bool errorflag_overflow = false;
bool errorflag_overwrite = false;
bool errorflag_memoryOverflow = false;
bool errorflag_missingAlign = false;

int main() {
	init();
	IF();
	ifidn = ifid;
	print_all();
	do {
		cycle += 1;
		ID();
		IF();
		if (ifidn.halt && idexn.halt && exdmn.halt && dmwbn.halt) {
			finalize(0);
		}
		log_command();
		EX();
		DM();
		print_diff();
		WB();
		print_error();
		// register move forward
		if (stall) {
			idexn = idex0;
			exdmn = exdm;
			dmwbn = dmwb;
		} else if (flush) {
			ifidn = ifid0;
			idexn = idex;
			exdmn = exdm;
			dmwbn = dmwb;
		} else {
			ifidn = ifid;
			idexn = idex;
			exdmn = exdm;
			dmwbn = dmwb;
		}
		
		if(cycle%500000==0){printf("In cycle %d, Press Enter to continue...\n", cycle);getchar();}
	} while (true);
	finalize(0);
	return 0;
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

// global variable: fout, ferr
inline void init() {
	fout = fopen("snapshot.rpt", "wb");
	ferr = fopen("error_dump.rpt", "wb");
	if (!fout) printf("snapshot.rpt cannot be opened!\n");
	if (!ferr) printf("error_dump.rpt cannot be opened!\n");
	read_data();
}

// input: regi[]. HI, LO, PC
inline void print_all() {
    oprintf("cycle 0\n");
    for (int i = 0; i < 32; i++) {
    	oprintf("$%02d: 0x%08X\n", i, regi[i]);
    }
    oprintf("$HI: 0x%08X\n", HI);
    oprintf("$LO: 0x%08X\n", LO);
    oprintf("PC: 0x%08X\n", PC);
    oprintf("IF: 0x%08X\n", command);
    oprintf("ID: NOP\n");
    oprintf("EX: NOP\n");
    oprintf("DM: NOP\n");
    oprintf("WB: NOP\n\n\n");
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
    oprintf("PC: 0x%08X\n", PC);
    
    oprintf("IF: 0x%08X", command);
    if (stall) {
    	oprintf(" to_be_stalled\n");
    } else if (flush) {
    	oprintf(" to_be_flushed\n");
    } else {
    	oprintf("\n");
    }
    
    oprintf("ID: %s", idex.opc.c_str());
    if (stall) {
    	oprintf(" to_be_stalled\n");
    }
    if (IDexdmf || IDdmwbf) {
    	if (IDexdmf) {
    		if (IDrsf) {
    			oprintf(" fwd_EX-DM_rs_$%d", IDrsf);
    		}
    		if (IDrtf) {
    			oprintf(" fwd_EX-DM_rt_$%d", IDrtf);
    		}
    		oprintf("HERE 1");
    		oprintf("\n");
    	}
    	if (IDdmwbf) {
    		if (IDrsf) {
    			oprintf(" fwd_DM-WB_rs_$%d", IDrsf);
    		}
    		if (IDrtf) {
    			oprintf(" fwd_DM-WB_rt_$%d", IDrtf);
    		}
    		oprintf("HERE 2");
    		oprintf("\n");	
    	}
    } else {
    	oprintf("\n");
    }
    
    oprintf("EX: %s", idexn.opc.c_str());
    if (EXexdmf) {
    	if (EXrsf) {
    		oprintf(" fwd_EX-DM_rs_$%d", EXrsf);
    	}
    	if (EXrtf) {
    		oprintf(" fwd_EX-DM_rt_$%d", EXrtf);
    	}
    	oprintf("\n");
    }
    if (EXdmwbf) {
    	if (EXrsf) {
    		oprintf(" fwd_DM-WB_rs_$%d", EXrsf);
    	}
    	if (EXrtf) {
    		oprintf(" fwd_DM-WB_rt_$%d", EXrtf);
    	}
    	oprintf("\n");	
    }
    
    oprintf("DM: %s\n", exdmn.opc.c_str());
    oprintf("WB: %s\n\n\n", dmwbn.opc.c_str());
}

// global variables: errorflag_write0, errorflag_overflow, errorflag_overwrite, 
//                   errorflag_memoryOverflow, errorflag_missingAlign
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

// api
int scan_command(int start, int end) {
    unsigned int b=0xFFFFFFFF;
    return (command>>end)&(b>>(end+31-start));
}
int memd2reg1(int start) {return memd[start];
}
int memd2reg2(int start) {
	int tmp=(((unsigned char)memd[start])<<8)|((unsigned char)memd[start+1]);
	if (tmp<0x00008000) {
	    return tmp;
	} else {
	    return tmp+0xffff0000;   
	}
}
int memd2reg4(int start) {
	return ((((unsigned char)memd[start  ])<<24)|
	        (((unsigned char)memd[start+1])<<16)|
	        (((unsigned char)memd[start+2])<< 8)|
	        (((unsigned char)memd[start+3])));
}
int memd2reg(int start, int len) {
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
	switch(ifid.opcode) {
		case 0:
			switch(ifid.r0500) {
				case ADD: 	lprintf("%s $%d, $%d, $%d\n", "add", 	ifid.r1511, ifid.r2521, ifid.r2016); break;
				case ADDU: 	lprintf("%s $%d, $%d, $%d\n", "addu", 	ifid.r1511, ifid.r2521, ifid.r2016); break;
				case SUB: 	lprintf("%s $%d, $%d, $%d\n", "sub", 	ifid.r1511, ifid.r2521, ifid.r2016); break;
				case AND: 	lprintf("%s $%d, $%d, $%d\n", "and", 	ifid.r1511, ifid.r2521, ifid.r2016); break;
				case OR: 	lprintf("%s $%d, $%d, $%d\n", "or", 	ifid.r1511, ifid.r2521, ifid.r2016); break;
				case XOR: 	lprintf("%s $%d, $%d, $%d\n", "xor", 	ifid.r1511, ifid.r2521, ifid.r2016); break;
				case NOR: 	lprintf("%s $%d, $%d, $%d\n", "nor", 	ifid.r1511, ifid.r2521, ifid.r2016); break;
				case NAND: 	lprintf("%s $%d, $%d, $%d\n", "nand",	ifid.r1511, ifid.r2521, ifid.r2016); break;
				case SLT: 	lprintf("%s $%d, $%d, $%d\n", "slt", 	ifid.r1511, ifid.r2016, ifid.r2016); break;
				case SLL: 	lprintf("%s $%d, $%d,  %d\n", "sll", 	ifid.r1511, ifid.r2016, ifid.r1006); break;
				case SRL: 	lprintf("%s $%d, $%d,  %d\n", "srl", 	ifid.r1511, ifid.r2016, ifid.r1006); break;
				case SRA: 	lprintf("%s $%d, $%d,  %d\n", "sra", 	ifid.r1511, ifid.r2016, ifid.r1006); break;
				case JR: 	lprintf("%s $%d          \n", "jr", 	ifid.r2521                        ); break;
				case MULT: 	lprintf("%s $%d, $%d     \n", "mult", 	ifid.r2521, ifid.r2016            ); break;
				case MULTU: lprintf("%s $%d, $%d     \n", "multu", 	ifid.r2521, ifid.r2016            ); break;
				case MFHI: 	lprintf("%s $%d          \n", "mfhi", 	ifid.r1511                        ); break;
				case MFLO: 	lprintf("%s $%d          \n", "mflo", 	ifid.r1511                        ); break;
			}
			break;
		case ADDI: 	lprintf("%s $%d, $%d, %d\n", 	"addi", ifid.r2016, ifid.r2521, ifid.r1500); break;
		case ADDIU: lprintf("%s $%d, $%d, %d\n", 	"addiu",ifid.r2016, ifid.r2521, ifid.r1500); break;
		case LW: 	lprintf("%s $%d, $%d, %d\n", 	"lw", 	ifid.r2016, ifid.r2521, ifid.r1500); break;
		case LH: 	lprintf("%s $%d, $%d, %d\n", 	"lh", 	ifid.r2016, ifid.r2521, ifid.r1500); break;
		case LHU: 	lprintf("%s $%d, $%d, %d\n", 	"lhu", 	ifid.r2016, ifid.r2521, ifid.r1500); break;
		case LB: 	lprintf("%s $%d, $%d, %d\n", 	"lb", 	ifid.r2016, ifid.r2521, ifid.r1500); break;
		case LBU: 	lprintf("%s $%d, $%d, %d\n", 	"lbu", 	ifid.r2016, ifid.r2521, ifid.r1500); break;
		case SW: 	lprintf("%s $%d, $%d, %d\n", 	"sw", 	ifid.r2016, ifid.r2521, ifid.r1500); break;
		case SH: 	lprintf("%s $%d, $%d, %d\n", 	"sh", 	ifid.r2016, ifid.r2521, ifid.r1500); break;
		case SB: 	lprintf("%s $%d, $%d, %d\n", 	"sb", 	ifid.r2016, ifid.r2521, ifid.r1500); break;
		case LUI: 	lprintf("%s $%d,  %d    \n", 	"lui", 	ifid.r2016, ifid.r1500            ); break;
		case ANDI: 	lprintf("%s $%d, $%d, %d\n", 	"andi", ifid.r2016, ifid.r2521, ifid.r1500); break;
		case ORI: 	lprintf("%s $%d, $%d, %d\n", 	"ori", 	ifid.r2016, ifid.r2521, ifid.r1500); break;
		case NORI: 	lprintf("%s $%d, $%d, %d\n", 	"nori", ifid.r2016, ifid.r2521, ifid.r1500); break;
		case SLTI: 	lprintf("%s $%d, $%d, %d\n", 	"slti", ifid.r2016, ifid.r2521, ifid.r1500); break;
		case BEQ: 	lprintf("%s $%d, $%d, %d\n", 	"beq", 	ifid.r2016, ifid.r2521, ifid.r1500); break;
		case BNE: 	lprintf("%s $%d, $%d, %d\n", 	"bne", 	ifid.r2016, ifid.r2521, ifid.r1500); break;
		case BGTZ: 	lprintf("%s $%d,  %d    \n", 	"bgtz", ifid.r2521, ifid.r1500            ); break;
		case J:     lprintf("%s             \n", 	"halt"                                    ); break;
		case JAL:   lprintf("%s             \n", 	"halt"                                    ); break;
	}
}
