/*
This program is used to verify the algorithm of the command phasor
The algorithm will be used in the MCU; however, due to the low computation power and memory capacity of the MCU, optimized version will be used (lower nest level and ect.).

About this algorithm:
Assume the command given by PC is:
5A,2B,7(,3A,7),3(,2(,2B,4A,2),5E,2),0(,1U,0)

Format of the command: TO,TO,TO
T: Run this command for T times (unsigned 16-bit int)
O: Operation pointer (unsigned 16-bit int, pointer to operation table)
	( = 0xFFFE: this create a subloop
	) = 0xFFFF: this end a subloop
	Notice: max nest level = 256

5A: This will do operation A for 5 times
2F,3M: This will do F for 2 times, then M for 3 times
2(,5B,2): This will do 5B for 2 times
7(,2C,2A,7): This will do 2C then 2A, the combination of 2C,2A will be executed for 7 times
7(,2(,4Z,2B,2),20B,7): This will do 4Z,2B for 2 times, then 20B. After this, the combination of 2(,4Z,2B,2),20B will be executed for 2 times
0(,5B,3Z,0): This will run 5B,3Z forever

For the above example, assume operation table is located at SRAM 0x0000
	Notice: Each operation takes 4 bytes (2 bytes for direction, 2 bytes for speed)
Assume Operation A means Operation 0, B means 1 and so on.
The PC should should send the following code to the MCU:
(Message header is not shown)
0x0005, 0x0000, 0x0002, 0x0001, 0x0007, 0xFFFE, 0x0003, 0x0000, 0x0007, 0xFFFF,
0x0003, 0xFFFE, 0x0002, 0xFFFE, 0x0002, 0x0001, 0x0004, 0x0000, 0x0002, 0xFFFF,
0x0005, 0x0004, 0x0002, 0xFFFF, 0x0000, 0xFFFE, 0x0001, 0x0020, 0x0000, 0xFFFF
(Checksum and message footer is not shown)
*/

#include <stdio.h>
#include <stdint.h>
#include <windows.h>

uint16_t input[] = {
	0x0005, 0x0000, //0
	0x0002, 0x0001, //1
	0x0007, 0xFFFE, //2
	0x0003, 0x0000, //3
	0x0007, 0xFFFF, //4
	0x0003, 0xFFFE, //5
	0x0002, 0xFFFE, //6
	0x0002, 0x0001, //7
	0x0004, 0x0000, //8
	0x0002, 0xFFFF, //9
	0x0005, 0x0004, //10
	0x0002, 0xFFFF, //11
	0x0000, 0xFFFE, //12
	0x0001, 0x0020, //13
	0x0000, 0xFFFF  //14
};

int main() {
	//Command read pointer
	uint16_t phasor;
	uint16_t dataLength = 15;
	
	//Loop control (max nest level = 256)
	uint8_t nestLevel = 0;
	uint16_t nestCounter[256];
	uint16_t nestLocationStart[256];
	uint16_t nestLocationEnd[256];
	
	//Loop control processor
	uint8_t currentNestLevel;
	uint16_t currentLocation;
	
	//Operation counter
	uint16_t currentCounter;
	
	for (phasor = 0; phasor < dataLength; phasor++) {
		printf("\t\tPhasor = %d\n",phasor);
		
		//Enter a loop
		if (input[phasor*2+1] == 0xFFFE) {
			printf("%d - Nest++ - %d\n",nestLevel,nestLevel+1);
			nestCounter[nestLevel] = input[phasor*2];
			
			//Record start location (this is used to repeat the loop)
			nestLocationStart[nestLevel] = phasor;
			
			//Record end location (this is used to end the loop and go to next segment)
			currentNestLevel = 0;
			currentLocation = phasor;
			for(;;) {
				printf("Current location %d (%d), level %d\n",currentLocation,input[currentLocation*2+1],currentNestLevel);
				
				/*
				To find the end of the loop, we need to find the ")" mark (charcode 0xFFFF)
				However, because we have nested loop, we need to find the ")" with the smae nest level as the loop
				*/
				
				if (input[currentLocation*2+1] == 0xFFFE)
					currentNestLevel++;
				if (input[currentLocation*2+1] == 0xFFFF)
					currentNestLevel--;
				
				currentLocation++;
				
				if (currentNestLevel == 0) {
					nestLocationEnd[nestLevel] = currentLocation-1;
					break;
				}
			}
			
			nestLevel++;
		}
		
		//Leave a loop
		else if (input[phasor*2+1] == 0xFFFF) {
			printf("%d - Nest-- - %d (%d) \n",nestLevel,nestLevel-1,nestCounter[nestLevel-1]);
			
			nestCounter[nestLevel-1]--;
			
			//Dead loop (loop for 0 times means loop forever)
			if (nestCounter[nestLevel-1] == 0xFFFF) {
				nestCounter[nestLevel-1] = 0;
				phasor = nestLocationStart[nestLevel-1];
			}
			
			//Loop in progress
			else if (nestCounter[nestLevel-1] > 0)
				phasor = nestLocationStart[nestLevel-1];
			
			//Loop end
			else {
				phasor = nestLocationEnd[nestLevel-1];
				nestLevel--;
			}
			
			
		}
		
		//Execute operation
		else
			for (currentCounter = input[phasor*2]; currentCounter > 0; currentCounter--) {
				printf("Operation %d\n",input[phasor*2+1]);
				Sleep(100); //Debug using
			}
	}
	
	return 0;
}
