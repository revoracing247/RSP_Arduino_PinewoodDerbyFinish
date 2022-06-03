/*
File:   Test7SegDisplay.ino
Author: Colby Robbins
Date:   05\09\2022
Description: 
	** starting program for the pinewood derby timer. mainly focused on controlling 7 segment
	** displays through shift registers
*/

// +==============================+
// |       Defines Defines        |
// +==============================+
#define OE_DISABLED HIGH
#define OE_ENABLED  LOW

// +==============================+
// |        7-SEG DEFINES         |
// +==============================+
#define DIGIT_OFF 0b00000000
#define DIGIT_ON  0b11111111
#define DIGIT_0   0b11111100
#define DIGIT_1   0b01100000
#define DIGIT_2   0b11011010
#define DIGIT_3   0b11110010
#define DIGIT_4   0b01100110
#define DIGIT_5   0b10110110
#define DIGIT_6   0b10111110
#define DIGIT_7   0b11100000
#define DIGIT_8   0b11111110
#define DIGIT_9   0b11100110

// +==============================+
// |         PIN DEFINES          |
// +==============================+
#define SHIFT_OE     2  // PIN_A0 // 20 // D2
#define SHIFT_CLK    3  // PIN_F5 // 21 // D3
#define SHIFT_DAT1   4  // PIN_C6 // 22 // D4
#define SHIFT_DAT2   5  // PIN_B2 // 23 // D5
#define SHIFT_DAT3   6  // PIN_F4 // 24 // D6
#define SHIFT_DAT4   7  // PIN_A1 // 25 // D7
#define LANE_1_INPUT 8  // PIN_E3 // 26 // D8
#define LANE_2_INPUT 9  // PIN_B0 // 27 // D9
#define LANE_3_INPUT 10 // PIN_B1 // 28 // D10
#define LANE_4_INPUT 11 // PIN_E0 // 29 // D11
#define START_INPUT  12 // PIN_E1 // 30 // D12
#define LED_OUTPUT   13 // PD13   // 31 // D13

// +==============================+
// |        CONFIG DEFINES        |
// +==============================+
#define WAIT_COUNT 0 // sets transfer speed right now

// +==============================+
// |           GLOBALS            |
// +==============================+
int DisplayNum = 0; // number to output
bool lastLane1 = false;

// the setup function runs once when you press reset or power the board
void shiftNumOut(int number, bool decimalPoint)
{
	int bitValu1 = 0x00;
	int bitValu2 = 0x00;
	int bitValu3 = 0x00;
	int bitValu4 = 0x00;

	     if((number)    == 10000) { bitValu1 = DIGIT_OFF; }
	else if((number%10) == 9)     { bitValu1 = DIGIT_9; }
	else if((number%10) == 8)     { bitValu1 = DIGIT_8; }
	else if((number%10) == 7)     { bitValu1 = DIGIT_7; }
	else if((number%10) == 6)     { bitValu1 = DIGIT_6; }
	else if((number%10) == 5)     { bitValu1 = DIGIT_5; }
	else if((number%10) == 4)     { bitValu1 = DIGIT_4; }
	else if((number%10) == 3)     { bitValu1 = DIGIT_3; }
	else if((number%10) == 2)     { bitValu1 = DIGIT_2; }
	else if((number%10) == 1)     { bitValu1 = DIGIT_1; }
	else if((number%10) == 0)     { bitValu1 = DIGIT_0; }
	if(decimalPoint)              { bitValu1 |= 0x01; }
	
	     if(number == 10000)      { bitValu2 = DIGIT_OFF; }
	else if((number%100) >= 90)   { bitValu2 = DIGIT_9; }
	else if((number%100) >= 80)   { bitValu2 = DIGIT_8; }
	else if((number%100) >= 70)   { bitValu2 = DIGIT_7; }
	else if((number%100) >= 60)   { bitValu2 = DIGIT_6; }
	else if((number%100) >= 50)   { bitValu2 = DIGIT_5; }
	else if((number%100) >= 40)   { bitValu2 = DIGIT_4; }
	else if((number%100) >= 30)   { bitValu2 = DIGIT_3; }
	else if((number%100) >= 20)   { bitValu2 = DIGIT_2; }
	else if((number%100) >= 10)   { bitValu2 = DIGIT_1; }
	else if((number%100) >= 00)   { bitValu2 = DIGIT_0; }
	if(decimalPoint)              { bitValu2 |= 0x01; }
	
		 if(number == 10000)      { bitValu3 = DIGIT_OFF; }
	else if((number%1000) >= 900) { bitValu3 = DIGIT_9; }
	else if((number%1000) >= 800) { bitValu3 = DIGIT_8; }
	else if((number%1000) >= 700) { bitValu3 = DIGIT_7; }
	else if((number%1000) >= 600) { bitValu3 = DIGIT_6; }
	else if((number%1000) >= 500) { bitValu3 = DIGIT_5; }
	else if((number%1000) >= 400) { bitValu3 = DIGIT_4; }
	else if((number%1000) >= 300) { bitValu3 = DIGIT_3; }
	else if((number%1000) >= 200) { bitValu3 = DIGIT_2; }
	else if((number%1000) >= 100) { bitValu3 = DIGIT_1; }
	else if((number%1000) >= 000) { bitValu3 = DIGIT_0; }
	if(decimalPoint)              { bitValu3 |= 0x01; }
	
		 if(number == 10000)        { bitValu4 = DIGIT_OFF; }
	else if((number%10000) >= 9000) { bitValu4 = DIGIT_9; }
	else if((number%10000) >= 8000) { bitValu4 = DIGIT_8; }
	else if((number%10000) >= 7000) { bitValu4 = DIGIT_7; }
	else if((number%10000) >= 6000) { bitValu4 = DIGIT_6; }
	else if((number%10000) >= 5000) { bitValu4 = DIGIT_5; }
	else if((number%10000) >= 4000) { bitValu4 = DIGIT_4; }
	else if((number%10000) >= 3000) { bitValu4 = DIGIT_3; }
	else if((number%10000) >= 2000) { bitValu4 = DIGIT_2; }
	else if((number%10000) >= 1000) { bitValu4 = DIGIT_1; }
	else if((number%10000) >= 0000) { bitValu4 = DIGIT_0; }
	if(decimalPoint)                { bitValu4 |= 0x01; }
	
	digitalWrite(SHIFT_OE, OE_DISABLED); // don't show shifting the bits in
	// bit 0
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, (bitValu4 & (0x01<<0))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT2, (bitValu3 & (0x01<<0))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT3, (bitValu2 & (0x01<<0))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT4, (bitValu1 & (0x01<<0))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_CLK,  HIGH);                   // delay(WAIT_COUNT);
		
	// bit 1
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, (bitValu4 & (0x01<<1))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT2, (bitValu3 & (0x01<<1))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT3, (bitValu2 & (0x01<<1))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT4, (bitValu1 & (0x01<<1))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_CLK,  HIGH);                   // delay(WAIT_COUNT);
		
	// bit 2
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, (bitValu4 & (0x01<<2))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT2, (bitValu3 & (0x01<<2))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT3, (bitValu2 & (0x01<<2))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT4, (bitValu1 & (0x01<<2))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_CLK,  HIGH);                   // delay(WAIT_COUNT);
		
	// bit 3
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, (bitValu4 & (0x01<<3))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT2, (bitValu3 & (0x01<<3))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT3, (bitValu2 & (0x01<<3))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT4, (bitValu1 & (0x01<<3))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_CLK,  HIGH);                   // delay(WAIT_COUNT);
		
	// bit 4
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, (bitValu4 & (0x01<<4))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT2, (bitValu3 & (0x01<<4))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT3, (bitValu2 & (0x01<<4))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT4, (bitValu1 & (0x01<<4))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_CLK,  HIGH);                   // delay(WAIT_COUNT);
		
	// bit 5
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, (bitValu4 & (0x01<<5))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT2, (bitValu3 & (0x01<<5))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT3, (bitValu2 & (0x01<<5))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT4, (bitValu1 & (0x01<<5))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_CLK,  HIGH);                   // delay(WAIT_COUNT);
		
	// bit 6
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, (bitValu4 & (0x01<<6))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT2, (bitValu3 & (0x01<<6))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT3, (bitValu2 & (0x01<<6))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT4, (bitValu1 & (0x01<<6))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_CLK,  HIGH);                   // delay(WAIT_COUNT);
		
	// bit 7
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, (bitValu4 & (0x01<<7))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT2, (bitValu3 & (0x01<<7))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT3, (bitValu2 & (0x01<<7))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_DAT4, (bitValu1 & (0x01<<7))); // delay(WAIT_COUNT);
	digitalWrite(SHIFT_CLK,  HIGH);                   // delay(WAIT_COUNT);
	
	// one extra for latch register
	digitalWrite(SHIFT_CLK, LOW);                     // delay(WAIT_COUNT);
	digitalWrite(SHIFT_CLK, HIGH);                    // delay(WAIT_COUNT);
	digitalWrite(SHIFT_CLK, LOW);
	digitalWrite(SHIFT_OE, OE_ENABLED);
		
}

// the setup function runs once when you press reset or power the board
void setup()
{
	Serial.begin(9600);
	// initialize digital pin LED_OUTPUT as an output.
	pinMode(LED_OUTPUT, OUTPUT);
	pinMode(SHIFT_OE, OUTPUT);
	pinMode(SHIFT_CLK, OUTPUT);
	pinMode(SHIFT_DAT1, OUTPUT);
	pinMode(SHIFT_DAT2, OUTPUT);
	pinMode(SHIFT_DAT3, OUTPUT);
	pinMode(SHIFT_DAT4, OUTPUT);
	pinMode(LANE_1_INPUT, INPUT);
  
	digitalWrite(SHIFT_DAT1, HIGH);
	digitalWrite(SHIFT_DAT2, HIGH);
	digitalWrite(SHIFT_DAT3, HIGH);
	digitalWrite(SHIFT_DAT4, HIGH);
	digitalWrite(SHIFT_CLK, LOW);
	digitalWrite(SHIFT_OE, OE_ENABLED);
	lastLane1 = digitalRead(LANE_1_INPUT);
	
	shiftNumOut(0, false);
	DisplayNum = 9999;
}

// the loop function runs over and over again forever
void loop()
{
	#if 1 // timed number shift
	digitalWrite(LED_OUTPUT, HIGH);   // turn the LED on (HIGH is the voltage level)
  
	shiftNumOut(DisplayNum, (DisplayNum == 10000) ? true : false); // output new number
	Serial.println(DisplayNum);
	if(DisplayNum == 0) { DisplayNum = 10000; }
	else                { DisplayNum--; } 
	
	delay(5);                       // wait for a second
	digitalWrite(LED_OUTPUT, LOW);    // turn the LED off by making the voltage LOW
	delay(1000);                       // wait for a second
	#else // on sensor trigger
		if(lastLane1 == LOW && digitalRead(LANE_1_INPUT) == HIGH)
		{
			digitalWrite(LED_OUTPUT, HIGH);   // turn the LED on (HIGH is the voltage level)
			shiftNumOut(DisplayNum, false); // output new number
			if(DisplayNum >= 99) { DisplayNum = 0; }
			else                 { DisplayNum++; } 
			lastLane1 = digitalRead(LANE_1_INPUT);
		}
		else if(lastLane1 == HIGH && digitalRead(LANE_1_INPUT) == LOW)
		{
			shiftNumOut(100, true); // erase number
			digitalWrite(LED_OUTPUT, LOW);    // turn the LED off by making the voltage LOW
			lastLane1 = digitalRead(LANE_1_INPUT);
		}
	#endif
  
  
}
