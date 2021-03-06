#include<stdio.h>
#include<stdlib.h>
#include<thread>
#include<termios.h>
#include<unistd.h>
#include<time.h>
#include<limits.h>
#include<cmath>
#include<string.h>

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
int getch()
{
  struct termios oldt,newt;
  int    ch;
  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
  return ch;
}
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

void init_note_period(char*);

void togglePin(int, int);
void timer(void);
void tick();
void get_key(int, int,int, int);

void reset(int, int);
void reset_all();

void play_for_debug(int i);

extern void MMM();
/** 
  constant fileds
	*/
#define MAX_POS 158
#define MAX_TOGGLE_POS 150
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
#define FDD4_DIR_PIN (FDD4_MOT_PIN + 1)
#define FDD5_MOT_PIN 10
#define FDD5_DIR_PIN (FDD5_MOT_PIN + 1)
#define FDD6_MOT_PIN 12
#define FDD6_DIR_PIN (FDD6_MOT_PIN + 1)

#define DATA_PIN 21
#define CLOC_PIN 22
#define CLEN_PIN 23
#define PL_PIN 24

#define DATA_WIDTH 49
#define CLOC_PULSE_WIDTH 5

#define RESOLUTION 8000
#define SLEEP_DELAY_LOOP 1000
#define FIRST_NOTE "C0"


/// period formula
/// 1,000,000 / (frequency * 2 * resolution)
/// NOTE : please note the unit of data
/// frequency (Hz)
/// resolution (ns)
int get_period(double f){
	return 500000000L/(f * RESOLUTION);
}
#ifdef MIDI
bool isitplayingmode=true;
#else
bool isitplayingmode=false;
#endif
int musical_note_period[49] = { 0 }; // should calcuate with init_note_period() func

int current_pos[14] = {			//position of FDD headers
	0, NOT_USED, 0, NOT_USED, 0, NOT_USED, 0, NOT_USED, 0, NOT_USED, 0, NOT_USED, NOT_USED
};

int current_dir[14] = {			//FDD header's direction
	NOT_USED, FORWARD, NOT_USED, FORWARD, NOT_USED, FORWARD, NOT_USED, FORWARD, NOT_USED, FORWARD, NOT_USED, FORWARD, NOT_USED, FORWARD
};

int current_period[14] = {		//currently playing tune data
	EMPTY, NOT_USED, EMPTY, NOT_USED, EMPTY, NOT_USED, EMPTY, NOT_USED, EMPTY, NOT_USED, EMPTY, NOT_USED, EMPTY, NOT_USED
};

int current_tick[14] = {		//currently ticking time
	0, NOT_USED, 0, NOT_USED, 0, NOT_USED, 0, NOT_USED, 0, NOT_USED, 0, NOT_USED, 0, NOT_USED
};

int current_state[14] = {		//currenly is it working?
	HIGH, NOT_USED, HIGH, NOT_USED, HIGH, NOT_USED, HIGH, NOT_USED, HIGH, NOT_USED, HIGH, NOT_USED, HIGH, NOT_USED
};
int assigned_fdd[50]={0,};		//0 means not assigned = not playing this tune!
uint64_t btn_state = 0;

void setup() {
	// GPIO library setup
	wiringPiSetup();
	//memset(assigned_fdd,-1,sizeof(int)*50);
	char note[] = FIRST_NOTE;
	init_note_period(note);

	int i;
	for(int i=0;i<50;i++)
	{
		assigned_fdd[i]=-1;
	}
#ifdef MUSICAL_NOTE_CHECK
	for (i = 0; i < 49; i++)
	{
		printf("setup : %d\n", musical_note_period[i]);
	}
#endif

	
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
	for(i = 0; i < 14; i += 2)
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
    digitalWrite(dir_pin, HIGH);
  } 
  else if (current_pos[pin] <= MIN_TOGGLE_POS) {
	current_dir[dir_pin] = FORWARD;
    digitalWrite(dir_pin, LOW);
  }
  //Update currentPosition
  if (current_dir[dir_pin] == BACKWARD){
    current_pos[pin]--;
  } 
  else {
    current_pos[pin]++;
  }

   	//Pulse the control pin
  delayMicroseconds(1);
  digitalWrite(pin,current_state[pin]);
  current_state[pin] = (current_state[pin] == HIGH)?LOW:HIGH;
}

void reset(int pin, int dir_pin)
{
  digitalWrite(dir_pin, BACKWARD); // Go in reverse

  int s;
  for (s=0;s<MAX_POS;s+=2){ //Half max because we're stepping directly (no toggle)
    digitalWrite(pin,HIGH);
    delayMicroseconds(300);
    digitalWrite(pin,LOW);
    delayMicroseconds(300);
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

void assign_FDD(int tune)
{
	if(assigned_fdd[tune]<0)
	{
	for(int i=0;i<=12;i+=2)
	{
		if(i==8) continue;
		if(!current_period[i])		//if this FDD is not working right now
		{
			assigned_fdd[tune]=i;
			switch(i)
			{
				case 0:
				current_period[FDD1_MOT_PIN]=musical_note_period[tune];
				printf("%d assigned at 1\n",tune);
				break;
				case 2:
				current_period[FDD2_MOT_PIN]=musical_note_period[tune];
				printf("%d assigned at 2\n",tune);
				break;
				case 4:
				current_period[FDD3_MOT_PIN]=musical_note_period[tune];
				printf("%d assigned at 3\n",tune);
				break;
				case 6:
				current_period[FDD4_MOT_PIN]=musical_note_period[tune];
				printf("%d assigned at 4\n",tune);
				break;
				case 10:
				current_period[FDD5_MOT_PIN]=musical_note_period[tune];
				printf("%d assigned at 5\n",tune);
				break;
				case 12:
				current_period[FDD6_MOT_PIN]=musical_note_period[tune];
				printf("%d assigned at 6\n",tune);
				break;
			}
			break;
		}
	}
	}
}
void free_FDD(int tune)
{
	switch(assigned_fdd[tune])
	{
		case FDD1_MOT_PIN:
		current_period[FDD1_MOT_PIN]=0;
		printf("FDD 1 stopped playing %d\n",tune);	
		break;
		case FDD2_MOT_PIN:
		current_period[FDD2_MOT_PIN]=0;
		printf("FDD 2 stopped playing %d\n",tune);	
		break;
		case FDD3_MOT_PIN:
		current_period[FDD3_MOT_PIN]=0;
		printf("FDD 3 stopped playing %d\n",tune);	
		break;
		case FDD4_MOT_PIN:
		current_period[FDD4_MOT_PIN]=0;
		printf("FDD 4 stopped playing %d\n",tune);	
		break;
		case FDD5_MOT_PIN:
		current_period[FDD5_MOT_PIN]=0;
		printf("FDD 5 stopped playing %d\n",tune);	
		break;
		case FDD6_MOT_PIN:
		current_period[FDD6_MOT_PIN]=0;
		printf("FDD 6 stopped playing %d\n",tune);	
		break;
	}
	assigned_fdd[tune]=-1;
}

void loop() {
	int k,bk=0;
  while(1)
 	{
		get_key(DATA_PIN, CLOC_PIN, CLEN_PIN, PL_PIN);
		
#ifdef MODEBTN
		if(btn_state & MODEBTN)
			isitplayingmode=!isitplayingmode;
#endif
		if(isitplayingmode)
		{
			printf("start MMMM\n");
			//	digitalWrite(MODELED,HIGH);
			MMM();	//start MIDI playing mode
			printf("done!\n");
		}
		else
		{
#ifndef KEYBOARD
			for(int i=0;i<49;i++)
			{
				if(btn_state & (1LL<<i))
					assign_FDD(i);
				else free_FDD(i);
			}
#endif
#ifdef KEYBOARD
		k= getch();
		if(k!=bk &&bk!=0 )
		{
			free_FDD(bk);
			switch(k)
			{
				   case 'z':
					   assign_FDD(36);
					   break;
				   case 'x':
					   assign_FDD(38);
				   break;
				   case 'c':
				   	  assign_FDD(40);
					  break;
				   case 'v':
					  assign_FDD(41);
					  break;
				   case 'b':
					  assign_FDD(43);
					  break;
				   case 'n':
					  assign_FDD(45);
					  break;
				   case 'm':
					  assign_FDD(49);
					  break;
				   case ',':
					  assign_FDD(51);
					  break;
			}
			bk=k;
		}
#endif
			/*if (btn_state & 1)
				assign_FDD(36);
			else free_FDD(36);
			if (btn_state & 2)
				assign_FDD(38);
			else free_FDD(38);
			if (btn_state & 4)
				assign_FDD(40);
			else free_FDD(40);
			if (btn_state & 8)
				assign_FDD(41);
			else free_FDD(41);
			if (btn_state & 16)
				assign_FDD(43);
			else free_FDD(43);*/
		}
/*
		if (btn_state & 1)
			assign_FDD(36);
		else free_FDD(36);
		if (btn_state & 2)
			assign_FDD(38);
		else free_FDD(38);
		if (btn_state & 4)
			assign_FDD(40);
		else free_FDD(40);
		if (btn_state & 8)
			assign_FDD(41);
		else free_FDD(41);
		if (btn_state & 16)
			assign_FDD(43);
		else free_FDD(43);
*/
#ifdef DOREMI

		//doremi code
		if (btn_state & 1)
			current_period[FDD2_MOT_PIN] = musical_note_period[36];
		if (btn_state & 2)
			current_period[FDD2_MOT_PIN] = musical_note_period[38];
		if (btn_state & 4)
			current_period[FDD2_MOT_PIN] = musical_note_period[40];
		if (btn_state & 8)
			current_period[FDD2_MOT_PIN] = musical_note_period[41];
		if (btn_state & 16)
			current_period[FDD2_MOT_PIN] = musical_note_period[43];
		if (!(btn_state & 31))
			current_period[FDD2_MOT_PIN] = 0;
#endif
/*
		play_for_debug(28);
		play_for_debug(26);
		play_for_debug(24);
		play_for_debug(28);
		play_for_debug(28);
		play_for_debug(28);
		play_for_debug(26);
		play_for_debug(26);
		play_for_debug(26);
		play_for_debug(28);
		play_for_debug(26);
		play_for_debug(24);
		play_for_debug(26);
		play_for_debug(28);
		play_for_debug(28);
		play_for_debug(28);
		play_for_debug(26);
		play_for_debug(26);
		play_for_debug(28);
		play_for_debug(26);
		play_for_debug(24);
	//		usleep(SLEEP_DELAY_LOOP);
	*/

		delay(1);
	}
}

void get_key(int data, int clock, int clock_enable, int pl)
{
	int i;
	static uint64_t last_btn = 0;
	uint64_t btn = 0;
	uint64_t temp_bit = 0;

	// Parallel load
	digitalWrite(clock_enable, HIGH);
	digitalWrite(pl, LOW);
	delayMicroseconds(5);
	digitalWrite(pl, HIGH);
	digitalWrite(clock_enable, LOW);


	for (i = (DATA_WIDTH - 1); i >= 0; i--)
	{
		temp_bit =  digitalRead(data);
		btn |= temp_bit << i;
		 
		digitalWrite(clock, HIGH);
		delayMicroseconds(5);
		digitalWrite(clock, LOW);
	}

	if (last_btn != btn)
	{
		digitalWrite(clock_enable, HIGH);
		digitalWrite(pl, LOW);
		delayMicroseconds(5);
		digitalWrite(pl, HIGH);
		digitalWrite(clock_enable, LOW);
		
		for (i = (DATA_WIDTH - 1); i >= 0; i--)
		{
			temp_bit =  digitalRead(data);
			btn |= temp_bit << i;
				 
			digitalWrite(clock, HIGH);
			delayMicroseconds(5);
			digitalWrite(clock, LOW);
		}

#ifdef SHIFT_TEST
		if (last_btn != btn)
		{
			uint64_t mask = 1LLU << (DATA_WIDTH - 1);
			for (i = 0; i < DATA_WIDTH; i++)
			{
				printf("%d", (btn & mask)?1:0);
				mask >>= 1;
			}
			printf("\n");
		}
#else
		if (last_btn != btn)
			btn_state = btn;
#endif
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

void play_for_debug(int i)
{
	current_period[FDD1_MOT_PIN] = get_period(musical_note_period[i]); 
	delay(55);
	current_period[FDD1_MOT_PIN] = 0;
	delay(20);
}
int main(void)
{
	setup();

	thread tk(timer);
	thread lp(loop);
	lp.join();
	tk.join();
	return 0;
}
void timer(void)
{
	struct timespec present, begin;
	clock_gettime(CLOCK_MONOTONIC, &begin);
	long elapsed = 0;
	#ifndef INTERVAL_CHECK
	while(1)
	{
		clock_gettime(CLOCK_MONOTONIC, &present);
		elapsed = present.tv_nsec - begin.tv_nsec;
		if(elapsed >= RESOLUTION)
		{
			begin=present;
			tick();
		}
		else if(elapsed < 0)
			begin.tv_nsec -= 1000000000L;
	}
	#else
	int cnt = 0;
	long aa;
	while(1)
	{
		clock_gettime(CLOCK_MONOTONIC, &present);
		if((aa = present.tv_nsec - begin.tv_nsec) >= RESOLUTION)
		{
			begin = present;
			tick();
			if (cnt++ >= 600)
			{
				cnt = 0;
				printf("timer : %ld\n", aa);
			}

		}
		if (aa< 0)
			begin.tv_nsec -= 1000000000L;
	}
	#endif
}

// It not used, For test musical note
/*int getch() {
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
