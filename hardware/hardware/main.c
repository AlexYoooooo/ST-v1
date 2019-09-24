#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>


/************************************************************************/
/* Config                                                               */
/************************************************************************/

#define RC_CLOCK_FREQ 16000000
#define UART_BAUD 4800


/************************************************************************/
/* Application SFRs                                                     */
/************************************************************************/

//Analysis purpose
volatile uint8_t systemLoad = 0x00;

//Motor query and parser
volatile uint8_t query[1024*4]; //1024 instruction word in total, 4-byte per word
volatile struct Parser axisW, axisX, axisY, axisZ;


/************************************************************************/
/* ROM consts, lookup tables                                            */
/************************************************************************/


/************************************************************************/
/* Code segment                                                         */
/************************************************************************/

// UART ------------------------------------------------------------------

//Send a data, wait if buffer not empty
void sendSerialSync(uint8_t data) {
	while ( !(UCSR0A & (1<<UDRE0)) ); //Wait until last word send
	UDR0 = data;
}

//Request data, wait until data arrives
uint8_t requestSerialSync() {
	while ( !(UCSR0A & (1<<RXC0)) ); //Wait until data received
	return UDR0;
}

//External packed routine

#include "../../command parser/parser-eventdriven.c"
#include "motor.c"


// MAIN ROUTINES ---------------------------------------------------------

//Init hardwares
int main(void) {
	//Init parser
	parserReset(&axisW, 0);
	parserReset(&axisX, 0);
	parserReset(&axisY, 0);
	parserReset(&axisZ, 0);
	
	//Init UART
	UBRR0 = RC_CLOCK_FREQ/16/UART_BAUD-1;
	UCSR0B = (1<<RXEN0) | (1<<TXEN0); //Enable Tx, Rx
	UCSR0C = (0<<USBS0) | (3<<UCSZ00); //Async USART (UART), no parity, 1 stop bits, 8 bits data
	
	//Init I/O and motor mode
	DDRC = 0b10101001; // C	2-STEP		2-DIR		2-EN			1-STEP
	DDRD = 0b11111100; // D	0-STEP	0-DIR	0-EN	0/1-M3	0/1-M2	0/1-M1
	DDRG = 0b00000011; // G					XSW		1-DIR	1-EN
	DDRJ = 0b00111000; // J			2-M3	2-M2	2-M1
	DDRL = 0b11111100; // L	3-STEP	3-DIR	3-EN	3-M3	3-M2	3-M1

	PORTC = 0b00000000;
	PORTD = 0b00001100;
	PORTG = 0b00000000;
	PORTJ = 0b00011000;
	PORTL = 0b00001100;
	
	//Init timer - CTC with OCRA
	TCCR1B = 0b00001000;
	TIMSK1 = 0b00000010;
	TCCR3B = 0b00001000;
	TIMSK3 = 0b00000010;
	TCCR4B = 0b00001000;
	TIMSK4 = 0b00000010;
	TCCR5B = 0b00001000;
	TIMSK5 = 0b00000010;
	
	//System ready, listen to front-end
	sei();
	uint8_t checksum;
	uint8_t receive;
	uint8_t chunk;
	sendSerialSync(0x6D); //System OK = 0b01101101 <-- Use this pattern to check BAUD paring
	
	for(;;) { //System main loop
		receive = requestSerialSync();
		
		if (receive == 0x00) { //Send instruction query to MCU
			chunk = requestSerialSync();
			if (chunk == requestSerialSync()) { //Send chunk twice to prevent error
				sendSerialSync(1); //Chunk OK

				checksum = 0;
				for (uint16_t i = 0; i < 256; i++) {
					receive = requestSerialSync();
					query[ (chunk<<8) | i] = receive;
					checksum -= receive;
				}
			
				parserReset(&axisW, (query[4096-4*2]<<8) | query[4096-4*2+1] ); //query[4088-4089]
				parserReset(&axisX, (query[4096-3*2]<<8) | query[4096-3*2+1] ); //query[4090-4091]
				parserReset(&axisY, (query[4096-2*2]<<8) | query[4096-2*2+1] ); //query[4092-4093]
				parserReset(&axisZ, (query[4096-1*2]<<8) | query[4096-1*2+1] ); //query[4094-4095]
			
				sendSerialSync(checksum); //Return checksum	

			}
			else {
				sendSerialSync(0); //Chunk bad
			}

			
		}
		
		else { //Instant command
			switch ( (receive&0b11000000) >> 6 ) { //W-axis command
				case 3: //Start/stop: 11
					if (getW())
					stopW();
					else
					startW();
					break;
				case 2: //Forward step: 10
					stepW(1);
					break;
				case 1: //Backward step: 01
					stepW(0);
					break;
				/* case 0: //No action: 00 */
			}
			
			switch ( (receive&0b00110000) >> 4 ) { //X-axis command
				case 3:
					if (getX())
						stopX();
					else
						startX();
					break;
				case 2:
					stepX(1);
					break;
				case 1:
					stepX(0);
					break;
			}
			
			switch ( (receive&0b00001100) >> 2 ) { //Y-axis command
				case 3:
					if (getY())
						stopY();
					else
						startY();
					break;
				case 2:
					stepY(1);
					break;
				case 1:
					stepY(0);
					break;
			}
			
			switch (receive&0b00000011) { //Z-axis command
				case 3:
					if (getZ())
						stopZ();
					else
						startZ();
					break;
				case 2:
					stepZ(1);
					break;
				case 1:
					stepZ(0);
					break;
			}
			sendSerialSync(receive); //Return cmd to ACK the cmd
		}
		
	}
	
	return 0;
}


// ISR Interrupt Service Routine -----------------------------------------

ISR(TIMER1_COMPA_vect) { //Time delay reached
	uint16_t parserResult = phase(&axisW); //Parser process instruction query
	setW(parserResult & 0x7FFF); //Set delay (motor speed)
	stepW(parserResult >> 15); //Step motor forward/backward
}

ISR(TIMER3_COMPA_vect) {
	uint16_t parserResult = phase(&axisX);
	setX(parserResult & 0x7FFF);
	stepX(parserResult >> 15);
}

ISR(TIMER4_COMPA_vect) {
	uint16_t parserResult = phase(&axisY);
	setY(parserResult & 0x7FFF);
	stepY(parserResult >> 15);
}

ISR(TIMER5_COMPA_vect) {
	uint16_t parserResult = phase(&axisZ);
	setZ(parserResult & 0x7FFF);
	stepZ(parserResult >> 15);
}


/************************************************************************/
/* Known issues                                                         */
/************************************************************************/
