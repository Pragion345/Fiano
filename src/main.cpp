#include<stdio.h>
#include<stdlib.h>
#include<thread>
#include<termios.h>
#include<unistd.h>
#include<chrono>
#include<ctime>
#include<limits.h>
#include<math.h>

#ifndef DEBUG
// wiringPi h including
#include<wiringPi.h>
#include<wiringShift.h>
#else

#define MSBFIRST 1
#define LOW 0
#define HIGH 1
#define OUTPUT 1
#define INPUT 1

int digitalRead(int a)
{return 0;}
void digitalWrite(int a, int b)
{return;}
void delay(int a)
{return;}
void delayMicroseconds(int a)
{return;}
void pinMode(int a, int b)
{return;}
void wiringPiSetup()
{return;}
#endif

using namespace std;
using namespace chrono;

void init_note_period(char*);

void togglePin(int, int);
void timer();
void tick();
void get_key(int, int,int, int, uint8_t);

void reset(int, int);
void reset_all();

/** 
  constant fileds
	*/
#define MAX_POS 158
#define MAX_TOGGLE_POS 153
#define MIN_TOGGLE_POS 10

#define NOT_USED -99
#define EMPTY 0

#define FORWARD LOW
#define BACKWARD HIGH

#define FDD1_MOT_PIN 0
#define FDD1_DIR_PIN (FDD1_MOT_PIN + 1)
#define FDD2_MOT_PIN 2
#define FDD2_DIR_PIN (FDD2_MOT_PIN + 1)
#define FDD3_MOT_PIN 4
#define FDD3_DIR_PIN (FDD3_MOT_PIN + 1)
#define FDD4_MOT_PIN 6
#define FDD4_DIR_PIN (FDD3_MOT_PIN + 1)
#define FDD5_MOT_PIN 8
#define FDD5_DIR_PIN (FDD3_MOT_PIN + 1)
#define FDD6_MOT_PIN 10
#define FDD6_DIR_PIN (FDD3_MOT_PIN + 1)

#define DATA_PIN 3
#define CLOC_PIN 4
#define CLEN_PIN 5
#define PL_PIN 2

#define RESOLUTION 10
#define FIRST_NOTE "C1"


/// period formula
/// 1,000,000 / (frequency * 2 * resolution)
/// NOTE : please note the unit of data
/// frequency (Hz)
/// resolution (ns)
int get_period(double f){
	return 500000/(f * RESOLUTION);
}

int musical_note_period[49] = { 0 }; // should calcuate with init_note_period() func

int current_pos[12] = {
	0, NOT_USED, 0, NOT_USED, 0, NOT_USED, 0, NOT_USED, 0, NOT_USED, 0, NOT_USED
};

int current_dir[12] = {
	NOT_USED, FORWARD, NOT_USED, FORWARD, NOT_USED, FORWARD, NOT_USED, FORWARD, NOT_USED, FORWARD, NOT_USED, FORWARD
};

int current_period[12] = {
	EMPTY, NOT_USED, EMPTY, NOT_USED, EMPTY, NOT_USED, EMPTY, NOT_USED, EMPTY, NOT_USED, EMPTY, NOT_USED
};

int current_tick[12] = {
	0, NOT_USED, 0, NOT_USED, 0, NOT_USED, 0, NOT_USED, 0, NOT_USED, 0, NOT_USED 
};

int current_state[12] = {
	HIGH, NOT_USED, HIGH, NOT_USED, HIGH, NOT_USED, HIGH, NOT_USED, HIGH, NOT_USED, HIGH, NOT_USED
};

uint8_t btn_state = 0;

void setup() {
	// GPIO library setup
	wiringPiSetup();

	char note[] = FIRST_NOTE;
	init_note_period(note);
	
	pinMode(FDD1_MOT_PIN, OUTPUT);
	pinMode(FDD1_DIR_PIN, OUTPUT);
	pinMode(FDD2_MOT_PIN, OUTPUT);
	pinMode(FDD2_DIR_PIN, OUTPUT);
	pinMode(FDD3_MOT_PIN, OUTPUT);
	pinMode(FDD3_DIR_PIN, OUTPUT);
	pinMode(FDD4_MOT_PIN, OUTPUT);
	pinMode(FDD4_DIR_PIN, OUTPUT);
	pinMode(FDD5_MOT_PIN, OUTPUT);
	pinMode(FDD5_DIR_PIN, OUTPUT);
	pinMode(FDD6_MOT_PIN, OUTPUT);
	pinMode(FDD6_DIR_PIN, OUTPUT);

	pinMode(PL_PIN, OUTPUT);
	pinMode(CLOC_PIN, OUTPUT);
	pinMode(CLEN_PIN, OUTPUT);
	pinMode(DATA_PIN, INPUT);

	reset_all();
}

void tick()
{
	int i;

	for(i = 0; i < 12; i += 2)
	{
		if(current_period[i] > 0)
		{
			current_tick[i]++;
			if(current_period[i]<=current_tick[i])
			{
				current_tick[i]=0;
				togglePin(i, i+1);
			}
		}
	}
}
/// togglePin make header of FDD move
/// it makes sounds
void togglePin(int pin, int dir_pin) {

  //Switch directions if end has been reached
  if (current_pos[pin] >= MAX_TOGGLE_POS) {
    current_dir[dir_pin] = BACKWARD;
    digitalWrite(dir_pin, BACKWARD);
  } 
  else if (current_pos[pin] <= MIN_TOGGLE_POS) {
		current_dir[dir_pin] = FORWARD;
    digitalWrite(dir_pin, FORWARD);
  }
  //Update currentPosition
  if (current_dir[dir_pin] == BACKWARD){
    current_pos[pin]--;
  } 
  else {
    current_pos[pin]++;
  }

  //Pulse the control pin
  digitalWrite(pin,current_state[pin]);
  current_state[pin] = ~current_state[pin];
	delay(1);	
 // printf("%d %d\n",current_state[pin],current_pos[pin]);
}

void reset(int pin, int dir_pin)
{
  digitalWrite(dir_pin, BACKWARD); // Go in reverse

  int s;
  for (s=0;s<MAX_POS;s+=2){ //Half max because we're stepping directly (no toggle)
    digitalWrite(pin,HIGH);
    digitalWrite(pin,LOW);
    delay(5);
  }

  current_pos[pin] = 0; // We're reset.
  digitalWrite(dir_pin, FORWARD);
  current_dir[dir_pin] = FORWARD; // Ready to go forward.
}

void reset_all()
{
	reset(FDD1_MOT_PIN, FDD1_DIR_PIN);
	reset(FDD2_MOT_PIN, FDD2_DIR_PIN);
	reset(FDD3_MOT_PIN, FDD3_DIR_PIN);
	reset(FDD4_MOT_PIN, FDD4_DIR_PIN);
	reset(FDD5_MOT_PIN, FDD5_DIR_PIN);
	reset(FDD6_MOT_PIN, FDD6_DIR_PIN);
}


void loop() {
  while(1)
 	{
		get_key(DATA_PIN, CLOC_PIN, CLEN_PIN, PL_PIN, MSBFIRST);

		if (btn_state & 1)
			current_period[FDD1_MOT_PIN] = get_period(261.63);
		if (btn_state & 2)
			current_period[FDD1_MOT_PIN] = get_period(293.66);
		if (btn_state & 4)
			current_period[FDD1_MOT_PIN] = get_period(329.63);
		if (!(btn_state & 7))
			current_period[FDD1_MOT_PIN] = 0;
		//printf("%d\n",current_period[FDD1_MOT_PIN]);
	}
}

void get_key(int data, int clock, int clock_enable, int pl, uint8_t order)
{
	int i;
	static uint64_t last_btn = 0;
	uint8_t btn = 0;

	// Parallel load
	digitalWrite(pl, LOW);
	delayMicroseconds(1);
	digitalWrite(pl, HIGH);
	delayMicroseconds(1);

	digitalWrite(clock, HIGH);
	digitalWrite(clock_enable, LOW);

	if (order == MSBFIRST)
		for (i = 7; i >= 0; i--)
		{
			digitalWrite(clock, HIGH);
			delayMicroseconds(1);
			btn |= (uint8_t)digitalRead(data) << i;
			digitalWrite(clock, LOW);
			delayMicroseconds(1);
		}
	else
		for (i = 0; i < 8; i++)
		{
			digitalWrite(clock, HIGH);
			delayMicroseconds(1);
			btn |= (uint8_t)digitalRead(data) << i;
			digitalWrite(clock, LOW);
			delayMicroseconds(1);
		}
	digitalWrite(clock_enable, HIGH);
	
	if (last_btn != btn)
	{
		digitalWrite(pl, LOW);
		delayMicroseconds(1);
		digitalWrite(pl, HIGH);
		delayMicroseconds(1);
		
		digitalWrite(clock, HIGH);
		digitalWrite(clock_enable, LOW);

		if (order == MSBFIRST)
			for (i = 7; i >= 0; i--)
			{
				digitalWrite(clock, HIGH);
				delayMicroseconds(1);
				btn |= (uint8_t)digitalRead(data) << i;
				digitalWrite(clock, LOW);
				delayMicroseconds(1);
			}
		else
			for (i = 0; i < 8; i++)
			{
				digitalWrite(clock, HIGH);
				delayMicroseconds(1);
				btn |= (uint8_t)digitalRead(data) << i;
				digitalWrite(clock, LOW);
				delayMicroseconds(1);
			}
		digitalWrite(clock_enable, HIGH);

		if (last_btn != btn)
			btn_state = btn;
	}
	last_btn = btn;
	return;

}

void init_note_period(char* first_note)
{
	int i;
	int num_of_note; // if define standard note with A0, number of A0 = 0 
	num_of_note = (first_note[1] - 48) * 12;
	switch (first_note[0])
	{
		case 'A': break;
		case 'B': num_of_note += 2; break;
		case 'C': num_of_note -= 9; break;
		case 'D': num_of_note -= 7; break;
		case 'E': num_of_note -= 5; break;
		case 'F': num_of_note -= 4; break;
		case 'G': num_of_note -= 2; break;
		default:
				  return;
	}

	// EXAMPLE
	// C0 == -9
	// B1 == 14

	// Let calculating standard note be A2 (maybe most of case, middle of notes)
	// The frequency of A2 is 110Hz
	// And the number of A2 is 24
	// then frequency fomula is following fomulation
	// frequency = 110 * 2^((n - 24) / 12)

	for (i = 0; i < 49; i ++)
	{
		musical_note_period[i] = get_period(110 * pow(2, ((num_of_note + i - 24) / 12.0)));
	}
}



int main(void)
{
	setup();
	thread lp(loop);
	thread tk(timer); 
	lp.join();
	tk.join();
	return 0;
}

void timer()
{
	steady_clock::time_point present, begin;
	begin = steady_clock::now();
	int i;
	while(1)
	{
		present=steady_clock::now();
		if((i=duration_cast<duration<int,micro>>(present - begin).count()) >= 25)
		{
			begin=present;

			tick();
			printf("%d\n",i);
		}
	}
}


/* It not used, For test musical note
int getch( ) {
	struct termios oldt,
					  newt;
  int    ch;
  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
  return ch;
}
*/

/* not used

void set_tune()
{
	int tune;
	while(1)
	{

		tune=getch();
		switch(tune)
		{
			case 'a':
				printf("do");
				current_period=2249;
				break;
			case 's':
				printf("re");
				current_period=2003;
				break;
			case 'd':
				printf("mi");
				current_period=1785;
				break;
			case 'f':
				prin`tf("fa");
				current_period=1685;
				break;
			case 'g':
				printf("sol");
				current_period=1498;
				break;
			case 'h':
				printf("la");
				current_period=1336;
				break;
			case 'j':
				printf("ti");
				current_period=1191;
				break;
			case 'k':
				printf("Do");
				current_period=1124;
				break;
			case 'z':
				printf("Exprimental");
				current_period=INT_MAX;
				break;
			case 'x':
			case '/':
				current_period=current_period?current_period/2:1;
				
		//		printf("%d\n",current_period);
				break;
			case 'c':
			case '*':
				current_period=current_period<INT_MAX?current_period*2:INT_MAX;
		//		printf("%d\n",current_period);
				break;
			case 'v':
			case '+':
			case '=':
				current_period++;
				break;
			case 'b':
			case '-':
				current_period--;
					break;
			case ';': // interval variable was removed
					printf("input interval : ");
					scanf(" %d",&interval);
					printf("%d\n",interval);
					break;

			default:
					if(tune >='0' && tune<='9')
						if(tune == '0')
							current_period=1<<10;
						else current_period=1<<(tune-'0');
		}
		printf(" %d\n",current_period);
	}
}
*/
