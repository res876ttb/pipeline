#include <stdio.h>

#include "define.hpp"

int main() {
	init();
	return 0;
}

void init() {
	read_data();
}

void read_data() {
	char buff[4];
	FILE *iimage, *dimage;
	iimage = fopen("iimage.bin", "rb");
	dimage = fopen("dimage.bin", "rb");
	
	if (!dimage) {
		printf("No dimage.bin\n");
	}
	if (!iimage) {
		printf("No iimage.bin\n");
	}
	
	// read dimage.bin
	int sizeofchar = sizeof(char);
	int tmp;
	tmp = fread(buff, sizeofchar, 4, dimage);
	printf("buf = %X%X%X%X%X%X%X%X\n", buff[0]/16, buff[0]%16, buff[1]/16, buff[1]%16, buff[2]/16, buff[2]%16, buff[3]/16, buff[3]%16);
	printf("tmp = %d\n", tmp);
	tmp = fread(buff, sizeofchar, 4, dimage);
	printf("buf = %X%X%X%X%X%X%X%X\n", buff[0]/16, buff[0]%16, buff[1]/16, buff[1]%16, buff[2]/16, buff[2]%16, buff[3]/16, buff[3]%16);
	printf("tmp = %d\n", tmp);
	tmp = fread(buff, sizeofchar, 4, dimage);
	printf("buf = %X%X%X%X%X%X%X%X\n", buff[0]/16, buff[0]%16, buff[1]/16, buff[1]%16, buff[2]/16, buff[2]%16, buff[3]/16, buff[3]%16);
	printf("tmp = %d\n", tmp);
}
