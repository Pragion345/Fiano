#include<TimerOne.h>
#define MAX_POS 158
#define MAX_PERIOD 49
#define RESOLUTION 40

int current_period, current_tick;
int current_pos;
int currentState, currentdirState;

void togglePin(byte pin, byte direction_pin) {

  //Switch directions if end has been reached
  if (current_pos >= 158) {
    currentdirState = HIGH;
    digitalWrite(direction_pin,HIGH);
  } 
  else if (current_pos <= 0) {
    currentdirState = LOW;
    digitalWrite(direction_pin,LOW);
  }

  //Update currentPosition
  if (currentdirState==HIGH){
    current_pos--;
  } 
  else {
    current_pos++;
  }

  //Pulse the control pin
  digitalWrite(pin,currentState);
  currentState = ~currentState;
}

void reset(byte pin)
{
  digitalWrite(pin+1,HIGH); // Go in reverse
  for (byte s=0;s<158;s+=2){ //Half max because we're stepping directly (no toggle)
    digitalWrite(pin,HIGH);
    digitalWrite(pin,LOW);
    delay(5);
  }
  current_pos = 0; // We're reset.
  digitalWrite(pin+1,LOW);
  currentdirState = LOW; // Ready to go forward.
}

int btn7_lasted = 0;
int btn8_lasted = 0;
int btn7_clicking = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(7,INPUT);
  pinMode(8,INPUT);

	current_period = 1;
	current_tick = 0;
	current_pos = 0;

	currentState = LOW;
	currentdirState = LOW;
  reset(2);

	Timer1.initialize(RESOLUTION); // Set up a timer at the defined resolution
  Timer1.attachInterrupt(tick); // Attach the tick function

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
	int btn7_pushed = digitalRead(7);
	int btn8_pushed = digitalRead(8);

  if (btn7_pushed != btn7_lasted)
  {
    delay(5);
    btn7_pushed = digitalRead(7);
  }
	if (btn8_pushed != btn8_lasted)
	{
		delay(5);
		btn8_pushed = digitalRead(8);
	}

  if (btn7_pushed != btn7_lasted)
  {
    btn7_clicking = ~btn7_clicking;
  }
	if (btn8_pushed != btn8_lasted && btn8_pushed)
	{
			current_period++;
			if (current_period > MAX_PERIOD)
					current_period = 0;	
      Serial.println(current_period);
	}

	btn7_lasted = btn7_pushed;
	btn8_lasted = btn8_pushed;
}

void tick() {
	if (btn7_clicking)
	{
			if (current_period > 0)
			{
					current_tick++;
					if (current_period <= current_tick)
					{
							current_tick = 0;
							togglePin(2,3);
					}
			}
	}
}

