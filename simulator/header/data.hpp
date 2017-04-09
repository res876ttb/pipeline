#ifndef _DATA_HPP
#define _DATA_HPP

// define variables
extern ofstream ferr; // used to print error to error_dump.rpt
extern ofstream fout; // used to print snapshot

extern int regi[32];
extern char memi[1024];
extern char memd[1024];

// data process function
void init();
void read_memory_file();
void finalize();

#endif