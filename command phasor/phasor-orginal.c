/*
This program is used to verify the algorithm of the command phasor
The algorithm will be used in the MCU; however, due to the low computation power and memory capacity of the MCU, optimized version will be used (lower nest level and ect.).

About this algorithm:
Assume the command given by PC is:
5+1056,2-234,7(,3+444,7),3(,2(,2-233,4+20000,2),5-3333,2),0(,1+11,0)

Format of the command: CDT,CDT,CDT...
C: Unsigned 16-bit: Counter, repeat this command for C times (actual execute count = C + 1)
D: 1-bit: Forward direction = 1; Backward direction = 0
T: Unsigned 15-bit: Wait for T cycles before next instruction
If CT = 0x8000: Enter a loop
If CT = 0x0000: Exit a loop

For example:
5+1056: Run forwards for 6 times, interval between each run = 1056
5+1056,2-234: Run forwards for 6 times, interval between each run = 1056; then, Run backwards for 3 times, interval between each run = 234
2(,5+1056,2): Run forwards for 6 times, interval between each run = 1056. Repeat this for 2 more times. (total execution count = 6 * 3 = 18)
7(,5+1056,2-234,7): Run forwards for 6 times, interval between each run = 1056; then, Run backwards for 3 times, interval between each run = 234. Then, repeat this for 7 more times
65535(,5+1056,2-234,65535): Run forwards for 6 times, interval between each run = 1056; then, Run backwards for 3 times, interval between each run = 234. Repeat this forever
*/

#include <stdio.h>
#include <stdint.h>
#include <windows.h>

uint32_t step[] = {
	(5<<16)|	(1<<15)|	1056,
	(2<<16)|	(0<<15)|	234,
	(7<<16)|	(1<<15)|	0,
	(3<<16)|	(1<<15)|	444,
	(7<<16)|	(0<<15)|	0,
	(3<<16)|	(1<<15)|	0,
	(2<<16)|	(1<<15)|	0,
	(2<<16)|	(0<<15)|	233,
	(4<<16)|	(1<<15)|	20000,
	(2<<16)|	(0<<15)|	0,
	(5<<16)|	(0<<15)|	3333,
	(3<<16)|	(0<<15)|	0,
	(65535<<16)|	(1<<15)|	0,
	(1<<16)|	(1<<15)|	11,
	(65535<<16)|	(0<<15)|	0
};

int main() {
	//Command read pointer
	uint16_t phasor;
	uint16_t dataLength = 15;
	
	//Loop control (max nest level = 256)
	uint8_t nestLevel = 0;
	uint16_t nestCounter[256]; //Counts execution time for current iteration
	uint16_t nestLocationStart[256];
	
	//Loop control processor
	uint8_t currentNestLevel;
	uint16_t currentLocation;
	
	//Operation counter
	uint16_t currentCounter;
	
	for (phasor = 0; phasor < dataLength; phasor++) {
		printf("\t\tPhasor = %d\n",phasor);
		
		//Enter a loop
		if ( (step[phasor]&0xFFFF) == 0x8000) {
			printf("%d --> Nest++ --> %d\n",nestLevel,nestLevel+1);
			nestCounter[nestLevel] = step[phasor] >> 16;
			
			//Record start location of this iteration
			nestLocationStart[nestLevel] = phasor;
			
			nestLevel++;
		}
		
		//Leave a loop
		else if ( (step[phasor]&0xFFFF) == 0x0000) {
			printf("%d --> Nest-- --> %d (remainder %d) \n",nestLevel,nestLevel-1,nestCounter[nestLevel-1]);
			
			//Dead loop (repeat for 0 times means repeat forever)
			if (nestCounter[nestLevel-1] == 65535) {
				phasor = nestLocationStart[nestLevel-1];
			}
			
			//Loop in progress
			else if (nestCounter[nestLevel-1] > 0) {
				phasor = nestLocationStart[nestLevel-1];
				nestCounter[nestLevel-1]--;
			}
			
			//Loop end
			else {
				nestLevel--;
			}
			
			
		}
		
		//Execute operation
		else
			for (currentCounter = step[phasor]>>16; currentCounter >= 0; currentCounter--) {
				printf("Dir %d, Set timer %d\n",(step[phasor]&0x8000)>>15,step[phasor]&0x7FFF);
				Sleep(10); //Debug using
			}
	}
	
	return 0;
}
