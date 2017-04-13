#include <stdio.h>

#include "define.hpp"

int main() {
	init();
	return 0;
}

void init() {
#ifdef log
	printf(">>> init()\n");	
#endif
	
	read_data();
	
#ifdef log
	printf(">>> init() done\n");
#endif
}

void read_data() {
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
	for (int i = PC/4; tmp = fread(buff, sizeofchar, 4, iimage); i++) {
		memi[i] = (((unsigned char)buff[0])<<24)|(((unsigned char)buff[1])<<16)|(((unsigned char)buff[2])<<8)|(((unsigned char)buff[3]));
	}
#ifdef log
	printf("done\n");
#endif
	
#ifdef log
	printf(">>> read_data() done\n");
#endif
}
