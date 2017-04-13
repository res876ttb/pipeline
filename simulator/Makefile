CC = g++
CFLAGS = -std=c++11 -O3 -Wall

main: pipeline.cpp
	$(CC) $(CFLAGS) pipeline.cpp -o pipeline

ass: ass
	$(CC) $(CFLAGS) Assembler.cpp -o ass

clean: 
	if [ -f pipeline ]; then rm pipeline; fi;
	if [ -f ass ]; then rm ass; fi;
