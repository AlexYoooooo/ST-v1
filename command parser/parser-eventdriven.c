struct Parser {
	uint16_t parser; //Current parser location
	uint8_t nestLevel; //Nest level of current loop
	uint16_t currentCtr; //The current loop needs to run x more times
	uint16_t nestCtr[14]; //When enter child loop, save current counter in this stack
	uint16_t nestLocation[14]; //When enter a child loop, save the beginning location of current loop of the query in this stack
	/* The size of this struct should within 64 bytes accompany with the AVR LD/ST with displacement instruction */
};

void parserReset(volatile struct Parser *axis, uint16_t queryLocation) {
	axis->parser = queryLocation;
	axis->nestLevel = 0;
	axis->currentCtr = 0;
}

uint16_t phase(volatile struct Parser *axis) {
	//Read query
	uint16_t runCtr = (query[axis->parser]<<8) | query[axis->parser+1];
	uint16_t dirAndDelay = (query[axis->parser+2]<<8) | query[axis->parser+3];
	
	//Enter a loop
	if (dirAndDelay == 0x8000) {
		#ifdef SIMULATOR
			deadloop = 1;
		#endif
		
		axis->nestCtr[axis->nestLevel] = runCtr;
		axis->nestLocation[axis->nestLevel++] = axis->parser + 4;
		/* parser now points to the beginning of the loop, which indicates the execution count of the loop. */
		/* parser + 4 (a instruction word is 4 bytes) points to the first actual instruction of the loop.  */
		
		axis->parser += 4; //Move to next instruction word in the query
		return phase(axis); //Process nest instruction word in the query
	}
	
	//Exit a loop
	else if (dirAndDelay == 0x0000) {
		if (axis->nestCtr[axis->nestLevel-1]) { //Loop in progress
			axis->parser = axis->nestLocation[axis->nestLevel-1]; //Move to beginning of the loop of the queery
			if (axis->nestCtr[axis->nestLevel-1] != 0xFFFF) //Except special case (Dead loop)
				axis->nestCtr[axis->nestLevel-1]--;
			
			#ifdef SIMULATOR
				if (axis->nestCtr[axis->nestLevel-1] == 0xFFFF)
					deadloop = 2;
			#endif
		}
		
		else { //End of the loop
			axis->nestLevel--;
			axis->parser += 4; //Move to next instruction word in the query
		}
		
		return phase(axis);
	}
	
	//Regular instruction (setup motor delay)
	else {
		if (axis->currentCtr) { //Current query word not finished
			if (axis->currentCtr == 1) {
				/* One more round: Parser move to next instruction word */
				axis->parser += 4;
			}
			axis->currentCtr--;
		}
		
		else { //Read new query word
			axis->currentCtr = runCtr;
		}
		
		return dirAndDelay;
	}
}
