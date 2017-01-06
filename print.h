// misc print functions, for lots of info in the serial monitor.
// this stuff probably slows things down and would need to go
// for any hope of keeping up with full ethernet data rate!

void print(const char *s)
{
	Serial.print(s);
}

void print(const char *s, int num)
{
	Serial.print(s);
	Serial.print(num);
}

void printhex(const char *s, int num)
{
	Serial.print(s);
	Serial.println(num, HEX);
}

void printmac(const uint8_t *data)
{
	Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X",
		data[0], data[1], data[2], data[3], data[4], data[5]);
}

void printpacket(const uint8_t *data, unsigned int len)
{
#if 1
	unsigned int i;

	for (i=0; i < len; i++) {
		Serial.printf(" %02X", *data++);
		if ((i & 15) == 15) Serial.println();
	}
	Serial.println();
#endif
}
