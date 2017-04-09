#include "../data.hpp"
#include "../define.hpp"

void init() {
    // initialize all memory to 0
    for (int i=0; i<32; i++) {
        regi[i]=0;
    }
    for (int i=0; i<256; i++) {
        memi[i]=memd[i]=0;
    }
    HI=LO=PC=cycle=total_ins=0;

    // read file
    read_memory_file();
    
    ferr.open("error_dump.rpt");
    fout.open("snapshot.rpt");
}

void read_memory_file() {
    // read iimiage.bin and dimage.bin
    char buff[4];
    ifstream find,fini;
    find.open("dimage.bin",ios::binary);
    fini.open("iimage.bin",ios::binary);
    
    // check if file open successfully
    if (!find) {
        cout<<"No dimage.bin"<<endl;
    }
    
    if (!fini) {
        cout<<"No iimage.bin"<<endl;
    }
    
    // set initial value of $sp
    find.read(buff,4);
    SP_offset=regi[29]=regi2[29]=(((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));
    mem_limit=1024;
    
    // the number of data in dimage.bin
    find.read(buff,4);
    int data_size=(((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));

    // read all data in dimage.bin
    find.read(memd,4*data_size);

    // set PC start point
    fini.read(buff,4);
    PC_offset=PC=(((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));
    PC_limit=PC_offset+1023;
    
    // set the number if total instruction
    fini.read(buff,4);
    total_ins=(((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));

    for (int i=PC_offset/4; fini.read(buff,4)&&i<256; i++) {
        memi[i]=(((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));
    }

    find.close();
    fini.close();
}

void finalize() {
    // close open file
    ferr.close();
    fout.close();
}