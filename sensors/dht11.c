#define dht11Pin 7

struct dht11Result {
	uint8_t ok;
	uint8_t temperature;
	uint8_t humidity;
};

struct dht11Result dht11Read() {
	struct dht11Result result;
	result.ok = 0;

	// BUFFER TO RECEIVE
	uint8_t bits[5];
	uint8_t cnt = 7;
	uint8_t idx = 0;

	// EMPTY BUFFER
	bzero(bits, 5);

	// REQUEST SAMPLE
	pinMode(dht11Pin, OUTPUT);
	digitalWrite(dht11Pin, LOW);
	delay(18);
	digitalWrite(dht11Pin, HIGH);
	delayMicroseconds(40);
	pinMode(dht11Pin, INPUT);

	// ACKNOWLEDGE or TIMEOUT
	unsigned int loopCnt = 10000;
	while(digitalRead(dht11Pin) == LOW) if(loopCnt-- == 0) return result;

	loopCnt = 10000;
	while(digitalRead(dht11Pin) == HIGH) if(loopCnt-- == 0) return result;

	// READ OUTPUT - 40 BITS => 5 BYTES or TIMEOUT
	uint8_t i;
	for(i=0; i<40; i++) {
		loopCnt = 10000;
		while(digitalRead(dht11Pin) == LOW) if(loopCnt-- == 0) return result;

		unsigned long t = micros();

		loopCnt = 10000;
		while(digitalRead(dht11Pin) == HIGH) if(loopCnt-- == 0) return result;

		if((micros() - t) > 40) bits[idx] |= (1 << cnt);
		if(cnt == 0)   // next byte?
		{
			cnt = 7;    // restart at MSB
			idx++;      // next byte!
		}
		else cnt--;
	}

	// WRITE TO RIGHT VARS
	// as bits[1] and bits[3] are allways zero they are omitted in formulas.
	result.humidity = bits[0]; 
	result.temperature = bits[2]; 

	// CHECKSUM
	if(bits[4] != bits[0] + bits[2]) return result;
	result.ok = 1;
	return result;
}

struct dht11Result dht11(uint8_t tryNumber) {
	struct dht11Result result = dht11Read();
	if(!result.ok && ++tryNumber < 10) {
		delay(100);
		return dht11(tryNumber);
	}
	return result;
}
