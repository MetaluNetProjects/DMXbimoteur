/*********************************************************************
 *	DMX512 control of 2 DC motors
 *********************************************************************/

#define BOARD Versa2

#include <fruit.h>
#include <analog.h>
#include <dcmotor.h>
#include <dmx_slave.h>
#include <ramp.h>
#include <eeparams.h>

t_delay mainDelay;
unsigned char loops;

DCMOTOR_DECLARE(A);
DCMOTOR_DECLARE(B);
t_ramp speedRampA;
t_ramp speedRampB;

unsigned int DMXchan = 1;

unsigned char oldCHANSET_SWITCH = 0;
unsigned char debug = 0;

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	digitalClear(LED);			// clear the LED

	pinModeDigitalOut(SIG_LED); // set the SIG_LED pin mode to digital out
	digitalClear(SIG_LED);		// clear the SIG_LED

	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

	pinModeDigitalIn(CHANSET_SWITCH); 	// set the CHANSET_SWITCH pin mode to digital input

//----------- Analog setup ----------------
	analogInit();		// init analog module
	analogSelect(0, MOTA_CURRENT);	// assign MotorA current sense to analog channel 0
	analogSelect(1, MOTB_CURRENT);	// assign MotorB current sense to analog channel 1

//----------- dcmotor setup ----------------

	dcmotorInit(A);
	dcmotorInit(B);
	rampInit(&speedRampA);
	rampInit(&speedRampB);
	speedRampA.maxAccel = speedRampA.maxDecel = speedRampB.maxAccel = speedRampB.maxDecel = 32767;
	speedRampA.maxSpeed = speedRampB.maxSpeed = 1200;
	
	DMXSlaveInit();
	EEreadMain();
}

int DMX_to_PWM(unsigned char id)
{
	int dmxval = DMXSlaveGet(DMXchan + id * 2 + 1) - DMXSlaveGet(DMXchan + id * 2);
	int pwmval = 0;
	
	if(dmxval < 0) pwmval = ((dmxval - 1) * 4) + 1;
	else if(dmxval > 0) pwmval = ((dmxval + 1) * 4) - 1;

	return pwmval;
}

void chanSetProcess()
{
	unsigned char chanSetSwitch = digitalRead(CHANSET_SWITCH) == 0;
	unsigned int i;
	
	if(chanSetSwitch != oldCHANSET_SWITCH) {
		oldCHANSET_SWITCH = chanSetSwitch;
		//digitalWrite(LED, chanSetSwitch);

		if(chanSetSwitch) for(i = 1; i < DMX_SLAVE_NBCHAN ; i++) {
			if(DMXSlaveGet(i) == 255) {
				DMXchan = i;
				EEwriteMain();
				break;
			}
		}
	}
}

void loop() {
// ---------- Main loop ------------
	unsigned int maxReceivedChan;

	fraiseService();	// listen to Fraise events
	analogService();	// analog management routine

	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay
		analogSend();		// send analog channels that changed
		maxReceivedChan = DMXSlaveGetMaxReceivedChannel();
		
		chanSetProcess();
		
		rampGoto(&speedRampA, DMX_to_PWM(0));
		rampGoto(&speedRampB, DMX_to_PWM(1));

		if(debug || (maxReceivedChan > 2)) {
			digitalSet(LED); 
			digitalSet(SIG_LED); 
		} else {
			digitalClear(LED);
			digitalClear(SIG_LED);
		}
		
		rampCompute(&speedRampA);
		rampCompute(&speedRampB);
		DCMOTOR(A).Vars.PWMConsign = rampGetPos(&speedRampA);
		DCMOTOR(B).Vars.PWMConsign = rampGetPos(&speedRampB);
		DCMOTOR_COMPUTE(A,SYM);
		DCMOTOR_COMPUTE(B,SYM);
		if(loops++ > 5) {
			printf("CP %d %d\n", DCMOTOR(A).Vars.PWMConsign, DCMOTOR(B).Vars.PWMConsign);
			loops = 0;
		}
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
	}
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
}


void fraiseReceive() // receive raw
{
	unsigned char c;
	//unsigned int i;
	
	c=fraiseGetChar();
	
	switch(c) {
		case 10 : DMXchan = fraiseGetInt(); debug = 1; break;
		case 11 : DMXSlaveSet(fraiseGetInt(), fraiseGetChar()); break;
		case 20 : rampInput(&speedRampA); break;
		case 21 : rampInput(&speedRampB); break;
		case 120 : DCMOTOR_INPUT(A) ; break;
		case 121 : DCMOTOR_INPUT(B) ; break;
	}
}

void lowInterrupts()
{
	DMXSlaveISR();
}

void EEdeclareMain()
{
	EEdeclareChar(&DMXchan);
}
