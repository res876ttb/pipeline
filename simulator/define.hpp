// if define log: print out data path
#define log

// variable definition
int regi[32];
int memi[256];
char memd[1024];
int regi2[32];
int PC;
int IF_PC;
int IF_Instruction_code;
int ID_Register_Result[2];

// functions definition
inline void init();
inline void read_data();
