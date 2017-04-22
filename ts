#!/bin/bash

cd simulator
make 
if [ $? != 0 ]; then
	exit 1
fi

cp pipeline /Users/Ricky/Documents/Archi/HW1/single_cycle/testcase/single_cycle
cd /Users/Ricky/Documents/Archi/HW1/single_cycle/testcase
./testvalid

