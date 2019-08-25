#include <stdio.h>
#include <stdint.h>

#define SIMMULATION_PHASORDETAIL 1 //Show detail
unsigned long long int timestamp = 0;
signed long long int motorDegree = 0;
unsigned long long int eventNum = 0;

uint8_t query[1024*4];
struct Phasor axisW;

#include "phasor-eventdriven.c"

//Init hardwares
int main(void) {
	
	//See phasor-orginal.c
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
		//Append a dead loop to prevent instruction phasor buffer overflow
		(65535<<16)|	(1<<15)|	0,
		(1<<16)|	(1<<15)|	11,
		(65535<<16)|	(0<<15)|	0
	};
	
	//Copy the orginal query into hardware query memory
	for (unsigned int i = 0; i < sizeof(step) >> 2; i++) {
		query[4*i+0] = (step[i]>>24) & 0xFF;
		query[4*i+1] = (step[i]>>16) & 0xFF;
		query[4*i+2] = (step[i]>>8)  & 0xFF;
		query[4*i+3] = (step[i]>>0)  & 0xFF;
		printf("Step %d copied: step=0x%08x\n",i,step[i]);
	}
	
#ifdef SIMMULATION_PHASORDETAIL	
	printf("QUERY @ SRAM:\n");
	for (unsigned int i = 0; i < sizeof(query) / 16; i++) {
		printf("%03d\t",i);
		for (uint8_t j = 0; j < 16; j++)
			printf("| %02x\t",query[i*16+j]);
		printf("\n");
	}
#endif	
	
	//Init phasor
	phasorReset(&axisW, 0);
	
	uint8_t dummy;
	printf("Hit enter to fire an event.\n");
	
	//Simulate timer event
	for (;;) {
		scanf("%c",&dummy);
		uint16_t motorCmd = phase(&axisW);
#ifdef SIMMULATION_PHASORDETAIL
		printf("Set motor direction: %d\n",motorCmd>>15);
		printf("Set timer delay: %d\n",motorCmd&0x7FFF);
#endif
		
		//Send pulse to motor
		if (motorCmd>>15)
			motorDegree++;
		else
			motorDegree--;
		
		printf("+++ Event %llu, time: %llu, motor location: %lli +++\n",eventNum++,timestamp,motorDegree);
		
		timestamp += motorCmd&0x7FFF; //Setup timer, the next event will be fire then
	}
	
}