#include <util/delay_basic.h>

//Axis-W Timer1
void startW() { //Timer scaler <- 1
	TCCR1B |= 0b00000001;
}
void stopW() { //Timer scaler <- 0
	TCCR1B &= 0b11111000;
}
void setW(uint16_t delay) { //Set compare value
	OCR1A = delay;
}
uint8_t getW() {
	return TCCR1B & 0b00000111;
}
void stepW(uint8_t dir) { //Motor step forward/backward
	cli(); //STEP = D.7, DIR = D.6
	if (dir)
		PORTD |= 0b01000000;
	else
		PORTD &= 0b10111111;
	_delay_loop_1(3); //9 CPU cycles = 9/16us, A5958 requires 400ns setup time
	PORTD |= 0b10000000; //STEP rise
	_delay_loop_1(5); //15 CPU cycles = 15/16us, plus the instruction "PORTD |= 0b10000000", in total gets 1us. A5958 requires 1us pulse time
	PORTD &= 0b01111111; //STEP fall
	sei();
}

//Axis-X Timer3
void startX() {
	TCCR3B |= 0b00000001;
}
void stopX() {
	TCCR3B &= 0b11111000;
}
void setX(uint16_t delay) {
	OCR3A = delay;
}
uint8_t getX() {
	return TCCR3B & 0b00000111;
}
void stepX(uint8_t dir) {
	cli(); //STEP = C.0, DIR = G.1
	if (dir)
		PORTG |= 0b00000010;
	else
		PORTG &= 0b11111101;
	_delay_loop_1(3);
	PORTC |= 0b00000001;
	_delay_loop_1(5);
	PORTC &= 0b11111110;
	sei();
}

//Axis-Y Timer4
void startY() {
	TCCR4B |= 0b00000001;
}
void stopY() {
	TCCR4B &= 0b11111000;
}
void setY(uint16_t delay) {
	OCR4A = delay;
}
uint8_t getY() {
	return TCCR4B & 0b00000111;
}
void stepY(uint8_t dir) {
	cli(); //STEP = C.7, DIR = C.5
	if (dir)
		PORTC |= 0b00100000;
	else
		PORTC &= 0b11011111;
	_delay_loop_1(3);
	PORTC |= 0b10000000;
	_delay_loop_1(5);
	PORTC &= 0b01111111;
	sei();
}

//Axis-Z Timer5
void startZ() {
	TCCR5B |= 0b00000001;
}
void stopZ() {
	TCCR5B &= 0b11111000;
}
void setZ(uint16_t delay) {
	OCR5A = delay;
}
uint8_t getZ() {
	return TCCR5B & 0b00000111;
}
void stepZ(uint8_t dir) {
	cli(); //STEP = L.7, DIR = L.6
	if (dir)
		PORTL |= 0b01000000;
	else
		PORTL &= 0b10111111;
	_delay_loop_1(3);
	PORTL |= 0b10000000;
	_delay_loop_1(5);
	PORTL &= 0b01111111;
	sei();
}