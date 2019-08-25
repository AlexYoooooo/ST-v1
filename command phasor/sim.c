/******************************************************************************
The front-end software should generate a hexdecimal-coded file called "instruction.txt" that
contains the instruction which will be sent to the hardware.
This program will read that file and simulate how the hardware will react with
the given query.
******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define SIMULATOR 1
uint8_t deadloop = 0; //Detect dead loop
unsigned long long int eventNumWD = 0, eventNumXD = 0, eventNumYD = 0, eventNumZD = 0;

unsigned long long int timestampW = 0, timestampX = 0, timestampY = 0, timestampZ = 0;
signed long long int motorDegreeW = 0, motorDegreeX = 0, motorDegreeY = 0, motorDegreeZ = 0;
unsigned long long int eventNumW = 0, eventNumX = 0, eventNumY = 0, eventNumZ = 0;

uint8_t query[1024*4];
struct Phasor axisW, axisX, axisY, axisZ;

#include "phasor-eventdriven.c"

uint8_t charToInt(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else
		return 0xFF;
}

int main(void) {
	uint8_t dummy;
	
	//Read instruction file
	FILE *fp;
	fp = fopen("instruction.txt","r");
	if (!fp) {
		printf("Cannot find instruction file \"instruction.txt\".\n");
		scanf("%c",dummy);
		fclose(fp);
		return -1;
	}
	
	rewind(fp);
	char bitH, bitL;
	uint8_t numH, numL;
	for (uint16_t i = 0; i < 1024 * 4; i++) {
		if (fscanf(fp,"%c%c",&bitH,&bitL) == EOF) {
			printf("Command query length should be 4096, but get %d.\n",i);
			scanf("%c",dummy);
			fclose(fp);
			return -2;
		}
		numH = charToInt(bitH);
		numL = charToInt(bitL);
		if (numH > 15 || numL > 15) {
			printf("Invalid character in command query at %d, get 0x%c%c.\n",i,bitH,bitL);
			scanf("%c",dummy);
			fclose(fp);
			return -3;
		}
		query[i] = numH << 4 | numL;
	}
	if (fscanf(fp,"%c",&bitH) != EOF) {
		printf("Command query length should be 4096, but get more than that.\n");
		scanf("%c",dummy);
		fclose(fp);
		return -4;
	}
	
	
	fclose(fp);
	printf("QUERY @ SRAM:\n");
	for (unsigned int i = 0; i < sizeof(query) / 16; i++) {
		printf("%03d\t",i);
		for (uint8_t j = 0; j < 16; j++)
			printf("| %02x\t",query[i*16+j]);
		printf("\n");
	}
	uint16_t phasorStartW = (query[4096-4*2]<<8) | query[4096-4*2+1];
	printf("phasorStartW = 0x%04x (%d)\n",phasorStartW,phasorStartW);
	uint16_t phasorStartX = (query[4096-3*2]<<8) | query[4096-3*2+1];
	printf("phasorStartX = 0x%04x (%d)\n",phasorStartX,phasorStartX);
	uint16_t phasorStartY = (query[4096-2*2]<<8) | query[4096-2*2+1];
	printf("phasorStartY = 0x%04x (%d)\n",phasorStartY,phasorStartY);
	uint16_t phasorStartZ = (query[4096-1*2]<<8) | query[4096-1*2+1];
	printf("phasorStartZ = 0x%04x (%d)\n",phasorStartZ,phasorStartZ);
	
	//Init phasor
	phasorReset(&axisW, phasorStartW);
	phasorReset(&axisX, phasorStartX);
	phasorReset(&axisY, phasorStartY);
	phasorReset(&axisZ, phasorStartZ);
	
	fp = fopen("instruction.dump","w");
	
	//Simulate timer event - W
	fprintf(fp,"\n=== AXIS-W SIMULATION ===\n");
	printf("\n=== AXIS-W SIMULATION ===\n");
	for (;;) {
		uint16_t motorCmd = phase(&axisW);
		
		//Send pulse to motor
		if (motorCmd>>15)
			motorDegreeW++;
		else
			motorDegreeW--;
		
		if (deadloop == 1) {
			deadloop = 0; //Reset indecator, it is possible that a small dead loop in the current loop
			eventNumWD = eventNumW;
		}
		
		if (deadloop == 2) {
			deadloop = 0;
			fprintf(fp,"### Dead loop: go back to event %llu ###\n",eventNumWD);
			printf("### Dead loop: go back to event %llu ###\n",eventNumWD);
			break;
		}
		
		fprintf(fp,"+++ Event %llu, time: %llu, motor location: %lli +++\n",eventNumW,timestampW,motorDegreeW);
		printf("+++ Event %llu, time: %llu, motor location: %lli +++\n",eventNumW,timestampW,motorDegreeW);
		
		timestampW += motorCmd&0x7FFF; //Setup timer, the next event will be fire then
		eventNumW++;
	}
	
	//Simulate timer event - X
	fprintf(fp,"\n=== AXIS-X SIMULATION ===\n");
	printf("\n=== AXIS-X SIMULATION ===\n");
	for (;;) {
		uint16_t motorCmd = phase(&axisX);
		
		//Send pulse to motor
		if (motorCmd>>15)
			motorDegreeX++;
		else
			motorDegreeX--;
		
		if (deadloop == 1) {
			deadloop = 0; //Reset indecator, it is possible that a small dead loop in the current loop
			eventNumXD = eventNumX;
		}
		
		if (deadloop == 2) {
			deadloop = 0;
			fprintf(fp,"### Dead loop: go back to event %llu ###\n",eventNumXD);
			printf("### Dead loop: go back to event %llu ###\n",eventNumXD);
			break;
		}
		
		fprintf(fp,"+++ Event %llu, time: %llu, motor location: %lli +++\n",eventNumX,timestampX,motorDegreeX);
		printf("+++ Event %llu, time: %llu, motor location: %lli +++\n",eventNumX,timestampX,motorDegreeX);
		
		timestampX += motorCmd&0x7FFF; //Setup timer, the next event will be fire then
		eventNumX++;
	}
	
	//Simulate timer event - Y
	fprintf(fp,"\n=== AXIS-Y SIMULATION ===\n");
	printf("\n=== AXIS-Y SIMULATION ===\n");
	for (;;) {
		uint16_t motorCmd = phase(&axisY);
		
		//Send pulse to motor
		if (motorCmd>>15)
			motorDegreeY++;
		else
			motorDegreeY--;
		
		if (deadloop == 1) {
			deadloop = 0; //Reset indecator, it is possible that a small dead loop in the current loop
			eventNumYD = eventNumY;
		}
		
		if (deadloop == 2) {
			deadloop = 0;
			fprintf(fp,"### Dead loop: go back to event %llu ###\n",eventNumYD);
			printf("### Dead loop: go back to event %llu ###\n",eventNumYD);
			break;
		}
		
		fprintf(fp,"+++ Event %llu, time: %llu, motor location: %lli +++\n",eventNumY,timestampY,motorDegreeY);
		printf("+++ Event %llu, time: %llu, motor location: %lli +++\n",eventNumY,timestampY,motorDegreeY);
		
		timestampY += motorCmd&0x7FFF; //Setup timer, the next event will be fire then
		eventNumY++;
	}
	
	//Simulate timer event - Z
	fprintf(fp,"\n=== AXIS-Z SIMULATION ===\n");
	printf("\n=== AXIS-Z SIMULATION ===\n");
	for (;;) {
		uint16_t motorCmd = phase(&axisZ);
		
		//Send pulse to motor
		if (motorCmd>>15)
			motorDegreeZ++;
		else
			motorDegreeZ--;
		
		if (deadloop == 1) {
			deadloop = 0; //Reset indecator, it is possible that a small dead loop in the current loop
			eventNumZD = eventNumZ;
		}
		
		if (deadloop == 2) {
			deadloop = 0;
			fprintf(fp,"### Dead loop: go back to event %llu ###\n",eventNumZD);
			printf("### Dead loop: go back to event %llu ###\n",eventNumZD);
			break;
		}
		
		fprintf(fp,"+++ Event %llu, time: %llu, motor location: %lli +++\n",eventNumZ,timestampZ,motorDegreeZ);
		printf("+++ Event %llu, time: %llu, motor location: %lli +++\n",eventNumZ,timestampZ,motorDegreeZ);
		
		timestampZ += motorCmd&0x7FFF; //Setup timer, the next event will be fire then
		eventNumZ++;
	}
	
	fclose(fp);
	return 0;
}