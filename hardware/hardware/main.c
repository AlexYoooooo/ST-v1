#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>


/************************************************************************/
/* Config                                                               */
/************************************************************************/

#define RC_CLOCK_FREQ 16000000
#define UART_BAUD 19200


/************************************************************************/
/* Application SFRs                                                     */
/************************************************************************/

//Analysis purpose
volatile uint8_t systemLoad = 0x00;

//Motor query and phasor
volatile uint8_t query[1024*4]; //1024 instruction word in total, 4-byte per word
volatile struct Phasor axisW, axisX, axisY, axisZ;



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


// MAIN ROUTINES ---------------------------------------------------------

#include "phasor.c"
#include "motor.c"

//Init hardwares
int main(void) {
	//Init phasor
	phasorReset(&axisW, 0);
	phasorReset(&axisX, 0);
	phasorReset(&axisY, 0);
	phasorReset(&axisZ, 0);
	
	//Init UART
	UBRR0 = RC_CLOCK_FREQ/16/UART_BAUD-1;
	UCSR0B = (1<<RXEN0) | (1<<TXEN0); //Enable Tx, Rx
	UCSR0C = (1<<USBS0) | (3<<UCSZ00); //Async USART (UART), no parity, 2 stop bits, 8 bits data
	
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
	
	
	
	//System ready
	sei();
	for(;;) {
		
		
	}
	
	return 0;
}


// ISR Interrupt Service Routine -----------------------------------------


/************************************************************************/
/* Known issues                                                         */
/************************************************************************/
