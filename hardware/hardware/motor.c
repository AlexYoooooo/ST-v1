//Axis-W Timer0
void startW() { //Timer scaler <- 1
	TCCR1B |= 0b00000001;
}
void stopW() { //Timer scaler <- 0
	TCCR1B &= 0b11111000;
}
void setW(uint16_t delay) { //Set compare value
	OCR1A = delay;
	
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