/*
File:   RSP_PinewoodDerbyFinish.ino
Author: Colby Robbins
Date:   06\03\2022
Description: 
	** pinewood derby timer that uses back to back 7 segment displays over each of the 4 lanes
	** with a data line for each shift register and shared clk and outpuy enables lines. This
	** project uses IR LEDs over each lane and a reciever underneath to detect when cars cross
	**
	** Using: Arduino Nano Every
*/

#include <avr/interrupt.h>

// +==============================+
// |       Defines Defines        |
// +==============================+
#define OE_DISABLED HIGH
#define OE_ENABLED  LOW

#define PULLUP   HIGH
#define PULLDOWN LOW

#define NOT_FIN    0
#define POS_FIRST  1
#define POS_SECOND 2
#define POS_THIRD  3
#define POS_FOURTH 4

#define BIT_0 0b00000001
#define BIT_1 0b00000010
#define BIT_2 0b00000100
#define BIT_3 0b00001000
#define BIT_4 0b00010000
#define BIT_5 0b00100000
#define BIT_6 0b01000000
#define BIT_7 0b10000000

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;

typedef float     real32;
typedef float     r32;
typedef double    real64;
typedef double    r64;

#define IsBitSet(BitField, Bit) (((BitField) & (Bit)) != 0)
#define BitSet(BitField, Bit) (BitField) |= (Bit)
#define BitUnset(BitField, Bit) (BitField) &= ~(Bit)

// +==============================+
// |        CONFIG DEFINES        |
// +==============================+
#define STARTUP_SEQUENCE_TIME   20 // ms (how fast the sequence runs at startup)
#define SENSOR_CHECK_TIME     2000 // ms (how long the wait time for sensor check is)
#define RACE_SEQUENCE_TIME     200 // ms (how fast the sequence runs in a race)

// +==============================+
// |        7-SEG DEFINES         |
// +==============================+
// segment format   abcdefgDP
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

// startup sequence
#define DIGIT_S01 0b00010000
#define DIGIT_S02 0b00111000
#define DIGIT_S03 0b00111010
#define DIGIT_S04 0b01111110
#define DIGIT_S05 0b11111110
#define DIGIT_S06 0b00000001

// wait sequence
#define DIGIT_W01 0b00010000
#define DIGIT_W02 0b00101000
#define DIGIT_W03 0b00000010
#define DIGIT_W04 0b01000100
#define DIGIT_W05 0b10000000

// +==============================+
// |         PIN DEFINES          |
// +==============================+
#define SHIFT_OE     2  // PIN_A0 // 20 // D2
#define SHIFT_CLK    3  // PIN_F5 // 21 // D3
#define SHIFT_DAT1   4  // PIN_C6 // 22 // D4
#define SHIFT_DAT2   5  // PIN_B2 // 23 // D5
#define SHIFT_DAT3   6  // PIN_F4 // 24 // D6
#define SHIFT_DAT4   7  // PIN_A1 // 25 // D7

#define LANE_1_INPUT 8  // PIN_E3 // 26 // D8 (external pullup)
#define LANE_2_INPUT 9  // PIN_B0 // 27 // D9 (external pullup)
#define LANE_3_INPUT 10 // PIN_B1 // 28 // D10 (external pullup)
#define LANE_4_INPUT 11 // PIN_E0 // 29 // D11 (external pullup)
#define GATE_INPUT   12 // PIN_E1 // 30 // D12 (external pullup)
#define LED_OUTPUT   13 // PD13   // 31 // D13

// +==============================+
// |           GLOBALS            |
// +==============================+
// output to displays
u8 Display1Bits = DIGIT_OFF;
u8 Display2Bits = DIGIT_OFF;
u8 Display3Bits = DIGIT_OFF;
u8 Display4Bits = DIGIT_OFF;

// others
bool PrintChangeNeeded = false;
u8 WaitState = 0;
u8 PositionAvailable = POS_FIRST;
u8 Lane1Position = 0x00;
u8 Lane2Position = 0x00;
u8 Lane3Position = 0x00;
u8 Lane4Position = 0x00;
r32 RaceSequenceTime = 0;

void printDigits(u8 display0, u8 display1, u8 display2, u8 display3)
{
	Display1Bits = display0;
	Display2Bits = display1;
	Display3Bits = display2;
	Display4Bits = display3;
	updateDisplays();
}
	
void updateDisplays(void)
{
	
	digitalWrite(SHIFT_OE, OE_DISABLED); // don't show shifting the bits in
	// bit 0
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, IsBitSet(Display1Bits, BIT_0));
	digitalWrite(SHIFT_DAT2, IsBitSet(Display2Bits, BIT_0));
	digitalWrite(SHIFT_DAT3, IsBitSet(Display3Bits, BIT_0));
	digitalWrite(SHIFT_DAT4, IsBitSet(Display4Bits, BIT_0));
	digitalWrite(SHIFT_CLK,  HIGH);
	// might want delay here between port writes? but it seems to work without....
		
	// bit 1
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, IsBitSet(Display1Bits, BIT_1));
	digitalWrite(SHIFT_DAT2, IsBitSet(Display2Bits, BIT_1));
	digitalWrite(SHIFT_DAT3, IsBitSet(Display3Bits, BIT_1));
	digitalWrite(SHIFT_DAT4, IsBitSet(Display4Bits, BIT_1));
	digitalWrite(SHIFT_CLK,  HIGH);
	// might want delay here between port writes? but it seems to work without....
		
	// bit 2
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, IsBitSet(Display1Bits, BIT_2));
	digitalWrite(SHIFT_DAT2, IsBitSet(Display2Bits, BIT_2));
	digitalWrite(SHIFT_DAT3, IsBitSet(Display3Bits, BIT_2));
	digitalWrite(SHIFT_DAT4, IsBitSet(Display4Bits, BIT_2));
	digitalWrite(SHIFT_CLK,  HIGH);
	// might want delay here between port writes? but it seems to work without....
		
	// bit 3
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, IsBitSet(Display1Bits, BIT_3));
	digitalWrite(SHIFT_DAT2, IsBitSet(Display2Bits, BIT_3));
	digitalWrite(SHIFT_DAT3, IsBitSet(Display3Bits, BIT_3));
	digitalWrite(SHIFT_DAT4, IsBitSet(Display4Bits, BIT_3));
	digitalWrite(SHIFT_CLK,  HIGH);
	// might want delay here between port writes? but it seems to work without....
		
	// bit 4
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, IsBitSet(Display1Bits, BIT_4));
	digitalWrite(SHIFT_DAT2, IsBitSet(Display2Bits, BIT_4));
	digitalWrite(SHIFT_DAT3, IsBitSet(Display3Bits, BIT_4));
	digitalWrite(SHIFT_DAT4, IsBitSet(Display4Bits, BIT_4));
	digitalWrite(SHIFT_CLK,  HIGH);
	// might want delay here between port writes? but it seems to work without....
		
	// bit 5
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, IsBitSet(Display1Bits, BIT_5));
	digitalWrite(SHIFT_DAT2, IsBitSet(Display2Bits, BIT_5));
	digitalWrite(SHIFT_DAT3, IsBitSet(Display3Bits, BIT_5));
	digitalWrite(SHIFT_DAT4, IsBitSet(Display4Bits, BIT_5));
	digitalWrite(SHIFT_CLK,  HIGH);
	// might want delay here between port writes? but it seems to work without....
		
	// bit 6
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, IsBitSet(Display1Bits, BIT_6));
	digitalWrite(SHIFT_DAT2, IsBitSet(Display2Bits, BIT_6));
	digitalWrite(SHIFT_DAT3, IsBitSet(Display3Bits, BIT_6));
	digitalWrite(SHIFT_DAT4, IsBitSet(Display4Bits, BIT_6));
	digitalWrite(SHIFT_CLK,  HIGH);
	// might want delay here between port writes? but it seems to work without....
		
	// bit 7
	digitalWrite(SHIFT_CLK,  LOW);
	digitalWrite(SHIFT_DAT1, IsBitSet(Display1Bits, BIT_7));
	digitalWrite(SHIFT_DAT2, IsBitSet(Display2Bits, BIT_7));
	digitalWrite(SHIFT_DAT3, IsBitSet(Display3Bits, BIT_7));
	digitalWrite(SHIFT_DAT4, IsBitSet(Display4Bits, BIT_7));
	digitalWrite(SHIFT_CLK,  HIGH);
	// might want delay here between port writes? but it seems to work without....
	
	// one extra for latch register
	digitalWrite(SHIFT_CLK, LOW);
	digitalWrite(SHIFT_CLK, HIGH);
	digitalWrite(SHIFT_CLK, LOW);
	digitalWrite(SHIFT_OE, OE_ENABLED);
}

// +--------------------------------------------------------------+
// |                             Init                             |
// +--------------------------------------------------------------+
// the setup function runs once when you press reset or power the board
void setup()
{
	// +==============================+
	// |           Pin Init           |
	// +==============================+
	// outputs
	pinMode(LED_OUTPUT, OUTPUT);
	pinMode(SHIFT_OE,   OUTPUT);
	pinMode(SHIFT_CLK,  OUTPUT);
	pinMode(SHIFT_DAT1, OUTPUT);
	pinMode(SHIFT_DAT2, OUTPUT);
	pinMode(SHIFT_DAT3, OUTPUT);
	pinMode(SHIFT_DAT4, OUTPUT);
	// inputs
	pinMode(LANE_1_INPUT, INPUT);
	pinMode(LANE_2_INPUT, INPUT);
	pinMode(LANE_3_INPUT, INPUT);
	pinMode(LANE_4_INPUT, INPUT);
	pinMode(GATE_INPUT,   INPUT);
	 // set initial outputs
	digitalWrite(SHIFT_DAT1, HIGH);
	digitalWrite(SHIFT_DAT2, HIGH);
	digitalWrite(SHIFT_DAT3, HIGH);
	digitalWrite(SHIFT_DAT4, HIGH);
	digitalWrite(SHIFT_CLK, LOW);
	digitalWrite(SHIFT_OE, OE_ENABLED);
	
	// +==============================+
	// |      sensor check Init       |
	// +==============================+
	r32 sensorOkStartTime = 0;
	if(!digitalRead(LANE_1_INPUT) && !digitalRead(LANE_2_INPUT) && !digitalRead(LANE_3_INPUT) && !digitalRead(LANE_4_INPUT))
	{
		sensorOkStartTime = millis();
	}

	// +==============================+
	// |       Startup Sequence       |
	// +==============================+
	// up
	printDigits(DIGIT_OFF, DIGIT_OFF, DIGIT_OFF, DIGIT_OFF); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S01, DIGIT_OFF, DIGIT_OFF, DIGIT_OFF); delay(STARTUP_SEQUENCE_TIME);	
	printDigits(DIGIT_S01, DIGIT_S01, DIGIT_OFF, DIGIT_OFF); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S01, DIGIT_S01, DIGIT_S01, DIGIT_OFF); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S01, DIGIT_S01, DIGIT_S01, DIGIT_S01); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S02, DIGIT_S01, DIGIT_S01, DIGIT_S01); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S02, DIGIT_S02, DIGIT_S01, DIGIT_S01); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S02, DIGIT_S02, DIGIT_S02, DIGIT_S01); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S02, DIGIT_S02, DIGIT_S02, DIGIT_S02); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S03, DIGIT_S02, DIGIT_S02, DIGIT_S02); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S03, DIGIT_S03, DIGIT_S02, DIGIT_S02); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S03, DIGIT_S03, DIGIT_S03, DIGIT_S02); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S03, DIGIT_S03, DIGIT_S03, DIGIT_S03); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S04, DIGIT_S03, DIGIT_S03, DIGIT_S03); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S04, DIGIT_S04, DIGIT_S03, DIGIT_S03); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S04, DIGIT_S04, DIGIT_S04, DIGIT_S03); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S04, DIGIT_S04, DIGIT_S04, DIGIT_S04); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S05, DIGIT_S04, DIGIT_S04, DIGIT_S04); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S05, DIGIT_S05, DIGIT_S04, DIGIT_S04); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S05, DIGIT_S05, DIGIT_S05, DIGIT_S04); delay(STARTUP_SEQUENCE_TIME);
	
	// flash
	printDigits(DIGIT_S05, DIGIT_S05, DIGIT_S05, DIGIT_S05); delay(STARTUP_SEQUENCE_TIME * 10);
	printDigits(DIGIT_S06, DIGIT_S06, DIGIT_S06, DIGIT_S06); delay(STARTUP_SEQUENCE_TIME * 10);
	printDigits(DIGIT_S05, DIGIT_S05, DIGIT_S05, DIGIT_S05); delay(STARTUP_SEQUENCE_TIME * 10);
	printDigits(DIGIT_S06, DIGIT_S06, DIGIT_S06, DIGIT_S06); delay(STARTUP_SEQUENCE_TIME * 10);
	printDigits(DIGIT_S05, DIGIT_S05, DIGIT_S05, DIGIT_S05); delay(STARTUP_SEQUENCE_TIME * 10);
	
	// down
	printDigits(DIGIT_S05, DIGIT_S05, DIGIT_S05, DIGIT_S05); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S05, DIGIT_S05, DIGIT_S05, DIGIT_S04); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S05, DIGIT_S05, DIGIT_S04, DIGIT_S04); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S05, DIGIT_S04, DIGIT_S04, DIGIT_S04); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S04, DIGIT_S04, DIGIT_S04, DIGIT_S04); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S04, DIGIT_S04, DIGIT_S04, DIGIT_S03); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S04, DIGIT_S04, DIGIT_S03, DIGIT_S03); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S04, DIGIT_S03, DIGIT_S03, DIGIT_S03); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S03, DIGIT_S03, DIGIT_S03, DIGIT_S03); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S03, DIGIT_S03, DIGIT_S03, DIGIT_S02); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S03, DIGIT_S03, DIGIT_S02, DIGIT_S02); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S03, DIGIT_S02, DIGIT_S02, DIGIT_S02); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S02, DIGIT_S02, DIGIT_S02, DIGIT_S02); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S02, DIGIT_S02, DIGIT_S02, DIGIT_S01); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S02, DIGIT_S02, DIGIT_S01, DIGIT_S01); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S02, DIGIT_S01, DIGIT_S01, DIGIT_S01); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S01, DIGIT_S01, DIGIT_S01, DIGIT_S01); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S01, DIGIT_S01, DIGIT_S01, DIGIT_OFF); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S01, DIGIT_S01, DIGIT_OFF, DIGIT_OFF); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_S01, DIGIT_OFF, DIGIT_OFF, DIGIT_OFF); delay(STARTUP_SEQUENCE_TIME);
	printDigits(DIGIT_OFF, DIGIT_OFF, DIGIT_OFF, DIGIT_OFF); delay(STARTUP_SEQUENCE_TIME);
	
	// +==============================+
	// |     Startup Sensor Check     |
	// +==============================+
	u8 tempDisplay1 = DIGIT_OFF;
	u8 tempDisplay2 = DIGIT_OFF;
	u8 tempDisplay3 = DIGIT_OFF;
	u8 tempDisplay4 = DIGIT_OFF;
	bool allSensorsOkay = false;
	u8 sensorCheckState = 0x00;
	while(!allSensorsOkay)
	{
		if(digitalRead(LANE_1_INPUT)) { tempDisplay1 = DIGIT_OFF; }
		else                          { tempDisplay1 = DIGIT_S06; }
		if(digitalRead(LANE_2_INPUT)) { tempDisplay2 = DIGIT_OFF; }
		else                          { tempDisplay2 = DIGIT_S06; }
		if(digitalRead(LANE_3_INPUT)) { tempDisplay3 = DIGIT_OFF; }
		else                          { tempDisplay3 = DIGIT_S06; }
		if(digitalRead(LANE_4_INPUT)) { tempDisplay4 = DIGIT_OFF; }
		else                          { tempDisplay4 = DIGIT_S06; }
		
		if(!digitalRead(LANE_1_INPUT) && !digitalRead(LANE_2_INPUT) && !digitalRead(LANE_3_INPUT) && !digitalRead(LANE_4_INPUT))
		{
			if(sensorOkStartTime == 0){ sensorOkStartTime = millis(); sensorCheckState = 0x00; }
				 if(sensorCheckState == 0x05 && (millis() - sensorOkStartTime) > (SENSOR_CHECK_TIME/5)*5) { allSensorsOkay = true; }
			else if(sensorCheckState == 0x04 && (millis() - sensorOkStartTime) > (SENSOR_CHECK_TIME/5)*4) { sensorCheckState = 0x05; printDigits((DIGIT_S05|tempDisplay1), (DIGIT_S05|tempDisplay2), (DIGIT_S05|tempDisplay3), (DIGIT_S05|tempDisplay4)); }
			else if(sensorCheckState == 0x03 && (millis() - sensorOkStartTime) > (SENSOR_CHECK_TIME/5)*3) { sensorCheckState = 0x04; printDigits((DIGIT_S04|tempDisplay1), (DIGIT_S04|tempDisplay2), (DIGIT_S04|tempDisplay3), (DIGIT_S04|tempDisplay4)); }
			else if(sensorCheckState == 0x02 && (millis() - sensorOkStartTime) > (SENSOR_CHECK_TIME/5)*2) { sensorCheckState = 0x03; printDigits((DIGIT_S03|tempDisplay1), (DIGIT_S03|tempDisplay2), (DIGIT_S03|tempDisplay3), (DIGIT_S03|tempDisplay4)); }
			else if(sensorCheckState == 0x01 && (millis() - sensorOkStartTime) > (SENSOR_CHECK_TIME/5)*1) { sensorCheckState = 0x02; printDigits((DIGIT_S02|tempDisplay1), (DIGIT_S02|tempDisplay2), (DIGIT_S02|tempDisplay3), (DIGIT_S02|tempDisplay4)); }
			else if(sensorCheckState == 0x00 && (millis() - sensorOkStartTime) > 10)   { sensorCheckState = 0x01; printDigits((DIGIT_S01|tempDisplay1), (DIGIT_S01|tempDisplay2), (DIGIT_S01|tempDisplay3), (DIGIT_S01|tempDisplay4)); }
		}
		else
		{
			sensorOkStartTime = 0;
			printDigits(tempDisplay1, tempDisplay2, tempDisplay3, tempDisplay4);
			sensorCheckState = 0x00;
			delay(100);
		}
	}
	
	// +==============================+
	// |        setup for race        |
	// +==============================+
	Lane1Position = NOT_FIN;
	Lane2Position = NOT_FIN;
	Lane3Position = NOT_FIN;
	Lane4Position = NOT_FIN;
	PositionAvailable = POS_FIRST;
	digitalWrite(LED_OUTPUT, LOW);
	RaceSequenceTime = millis() - RACE_SEQUENCE_TIME;// trigger immediately once in loop()
		
	// +==============================+
	// |        Interrupt Init        |
	// +==============================+
	cli(); // disable interrupts while we change stuff
	PORTE.PIN3CTRL = 0x02; // LANE_1_INPUT physical pin 8  (PE3, D8) interrupt enable on RISING EDGE
	PORTB.PIN0CTRL = 0x02; // LANE_2_INPUT physical pin 9  (PB0, D9) interrupt enable on RISING EDGE
	PORTB.PIN1CTRL = 0x02; // LANE_3_INPUT physical pin 10 (PB1, D10) interrupt enable on RISING EDGE
	PORTE.PIN0CTRL = 0x02; // LANE_4_INPUT physical pin 11 (PE0, D11) interrupt enable on RISING EDGE
	// PORTE.PIN1CTRL = 0x02; // GATE_INPUT   physical pin 12 (PE1, D12) interrupt enable on RISING EDGE NOTE: interrupt not used currently
	sei(); // enable interrupts
}

// +--------------------------------------------------------------+
// |                          Main Loop                           |
// +--------------------------------------------------------------+
// the loop function runs over and over again forever
void loop()
{
	// +==============================+
	// |       Racing Sequence        |
	// +==============================+
	if(PositionAvailable == POS_FIRST && (millis() - RaceSequenceTime) >= RACE_SEQUENCE_TIME)
	{
		if     (WaitState == 0){ WaitState++; printDigits(DIGIT_W01, DIGIT_W02, DIGIT_W03, DIGIT_W04); }
		else if(WaitState == 1){ WaitState++; printDigits(DIGIT_W02, DIGIT_W01, DIGIT_W02, DIGIT_W03); }
		else if(WaitState == 2){ WaitState++; printDigits(DIGIT_W03, DIGIT_W02, DIGIT_W01, DIGIT_W02); }
		else if(WaitState == 3){ WaitState++; printDigits(DIGIT_W04, DIGIT_W03, DIGIT_W02, DIGIT_W01); }
		else if(WaitState == 4){ WaitState++; printDigits(DIGIT_W05, DIGIT_W04, DIGIT_W03, DIGIT_W02); }
		else if(WaitState == 5){ WaitState++; printDigits(DIGIT_W04, DIGIT_W05, DIGIT_W04, DIGIT_W03); }
		else if(WaitState == 6){ WaitState++; printDigits(DIGIT_W03, DIGIT_W04, DIGIT_W05, DIGIT_W04); }
		else if(WaitState == 7){ WaitState++; printDigits(DIGIT_W02, DIGIT_W03, DIGIT_W04, DIGIT_W05); }
		if(WaitState == 8) { WaitState = 0; }
		RaceSequenceTime = millis();// so we don't use Delay(); (better for responsiveness with interrupts changing app state)
	}
	// +==============================+
	// |       Print Positions        |
	// +==============================+
	else if(PrintChangeNeeded)
	{
		if(Lane1Position == NOT_FIN   ){ Display1Bits = DIGIT_OFF; }
		if(Lane1Position == POS_FIRST ){ Display1Bits = DIGIT_1; }
		if(Lane1Position == POS_SECOND){ Display1Bits = DIGIT_2; }
		if(Lane1Position == POS_THIRD ){ Display1Bits = DIGIT_3; }
		if(Lane1Position == POS_FOURTH){ Display1Bits = DIGIT_4; }
		
		if(Lane2Position == NOT_FIN   ){ Display2Bits = DIGIT_OFF; }
		if(Lane2Position == POS_FIRST ){ Display2Bits = DIGIT_1; }
		if(Lane2Position == POS_SECOND){ Display2Bits = DIGIT_2; }
		if(Lane2Position == POS_THIRD ){ Display2Bits = DIGIT_3; }
		if(Lane2Position == POS_FOURTH){ Display2Bits = DIGIT_4; }
		
		if(Lane3Position == NOT_FIN   ){ Display3Bits = DIGIT_OFF; }
		if(Lane3Position == POS_FIRST ){ Display3Bits = DIGIT_1; }
		if(Lane3Position == POS_SECOND){ Display3Bits = DIGIT_2; }
		if(Lane3Position == POS_THIRD ){ Display3Bits = DIGIT_3; }
		if(Lane3Position == POS_FOURTH){ Display3Bits = DIGIT_4; }
		
		if(Lane4Position == NOT_FIN   ){ Display4Bits = DIGIT_OFF; }
		if(Lane4Position == POS_FIRST ){ Display4Bits = DIGIT_1; }
		if(Lane4Position == POS_SECOND){ Display4Bits = DIGIT_2; }
		if(Lane4Position == POS_THIRD ){ Display4Bits = DIGIT_3; }
		if(Lane4Position == POS_FOURTH){ Display4Bits = DIGIT_4; }
		updateDisplays();
		PrintChangeNeeded = false;
	}
	// +==============================+
	// |          Reset Race          |
	// +==============================+
	if(!digitalRead(GATE_INPUT))
	{
		Lane1Position = NOT_FIN;
		Lane2Position = NOT_FIN;
		Lane3Position = NOT_FIN;
		Lane4Position = NOT_FIN;
		PositionAvailable = POS_FIRST;
		digitalWrite(LED_OUTPUT, LOW);
	}
}

// +--------------------------------------------------------------+
// |                    PIN Change Interrrupt                     |
// +--------------------------------------------------------------+
ISR (PORTE_PORT_vect) // port with lanes 1 and 4 on it (And GATE_INPUT)
{
	cli(); // disable interrupts
	digitalWrite(LED_OUTPUT, HIGH);
	if((IsBitSet(PORTE.INTFLAGS, BIT_0) && Lane4Position == NOT_FIN && ((IsBitSet(PORTE.INTFLAGS, BIT_3) && Lane1Position == NOT_FIN) || (IsBitSet(PORTB.INTFLAGS, BIT_0) && Lane2Position == NOT_FIN) || (IsBitSet(PORTB.INTFLAGS, BIT_1) && Lane3Position == NOT_FIN))) ||
	   (IsBitSet(PORTE.INTFLAGS, BIT_3) && Lane1Position == NOT_FIN && ((IsBitSet(PORTE.INTFLAGS, BIT_0) && Lane4Position == NOT_FIN) || (IsBitSet(PORTB.INTFLAGS, BIT_0) && Lane2Position == NOT_FIN) || (IsBitSet(PORTB.INTFLAGS, BIT_1) && Lane3Position == NOT_FIN))) ) // TIE!
	{
		u8 numTie = 0;
		PrintChangeNeeded = true;
		if(IsBitSet(PORTE.INTFLAGS, BIT_3) && Lane1Position == NOT_FIN) { Lane1Position = PositionAvailable; numTie++; }
		if(IsBitSet(PORTE.INTFLAGS, BIT_0) && Lane4Position == NOT_FIN) { Lane4Position = PositionAvailable; numTie++; }
		if(IsBitSet(PORTB.INTFLAGS, BIT_0) && Lane2Position == NOT_FIN) { Lane2Position = PositionAvailable; numTie++; }
		if(IsBitSet(PORTB.INTFLAGS, BIT_1) && Lane3Position == NOT_FIN) { Lane3Position = PositionAvailable; numTie++; }
		PositionAvailable += numTie;
	}
	if(IsBitSet(PORTE.INTFLAGS, BIT_0) && Lane4Position == NOT_FIN) { PrintChangeNeeded = true; Lane4Position = PositionAvailable; PositionAvailable++; }// LANE_4_INPUT crossed the line
	if(IsBitSet(PORTE.INTFLAGS, BIT_3) && Lane1Position == NOT_FIN) { PrintChangeNeeded = true; Lane1Position = PositionAvailable; PositionAvailable++; }// LANE_1_INPUT crossed the line
	PORTE.INTFLAGS = 0xFF; // clear all flags
	sei(); // enable interrupts
}

ISR (PORTB_PORT_vect) // port with lanes 2 and 3 on it
{
	cli(); // disable interrupts
	digitalWrite(LED_OUTPUT, HIGH);
	if((IsBitSet(PORTB.INTFLAGS, BIT_0) && Lane2Position == NOT_FIN && ((IsBitSet(PORTB.INTFLAGS, BIT_1) && Lane3Position == NOT_FIN) || (IsBitSet(PORTE.INTFLAGS, BIT_0) && Lane4Position == NOT_FIN) || (IsBitSet(PORTE.INTFLAGS, BIT_3) && Lane1Position == NOT_FIN))) ||
	   (IsBitSet(PORTB.INTFLAGS, BIT_1) && Lane3Position == NOT_FIN && ((IsBitSet(PORTB.INTFLAGS, BIT_0) && Lane2Position == NOT_FIN) || (IsBitSet(PORTE.INTFLAGS, BIT_0) && Lane4Position == NOT_FIN) || (IsBitSet(PORTE.INTFLAGS, BIT_3) && Lane1Position == NOT_FIN))) ) // TIE!
	{
		u8 numTie = 0;
		PrintChangeNeeded = true;
		if(IsBitSet(PORTE.INTFLAGS, BIT_3) && Lane1Position == NOT_FIN) { Lane1Position = PositionAvailable; numTie++; }
		if(IsBitSet(PORTE.INTFLAGS, BIT_0) && Lane4Position == NOT_FIN) { Lane4Position = PositionAvailable; numTie++; }
		if(IsBitSet(PORTB.INTFLAGS, BIT_0) && Lane2Position == NOT_FIN) { Lane2Position = PositionAvailable; numTie++; }
		if(IsBitSet(PORTB.INTFLAGS, BIT_1) && Lane3Position == NOT_FIN) { Lane3Position = PositionAvailable; numTie++; }
		PositionAvailable += numTie;
	}
	if(IsBitSet(PORTB.INTFLAGS, BIT_0) && Lane2Position == NOT_FIN) { PrintChangeNeeded = true; Lane2Position = PositionAvailable; PositionAvailable++; }// LANE_2_INPUT crossed the line
	if(IsBitSet(PORTB.INTFLAGS, BIT_1) && Lane3Position == NOT_FIN) { PrintChangeNeeded = true; Lane3Position = PositionAvailable; PositionAvailable++; }// LANE_3_INPUT crossed the line
	PORTB.INTFLAGS = 0xFF; // clear all flags
	sei(); // enable interrupts
}
