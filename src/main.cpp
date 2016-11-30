#include<thread>
#include<stdio.h>
#include<termios.h>
#include<unistd.h>
#include<chrono>
#include<ctime>
#include<limits.h>

// wiringPi h including
#include<wiringPi.h>
#include<wiringShift.h>

using namespace std;
using namespace chrono;

void togglePin(int, int);
void tick();

/** 
  constant fileds
	*/
#define MAX_POS 158
#define MAX_TOGGLE_POS 153
#define MIN_TOGGLE_POS 10

#define NOT_USED -2
#define EMPTY 0

#define FORWARD LOW
#define BACKWARD HIGH

#define FDD1_MOT_PIN 0;
#define FDD1_DIR_PIN 1;
#define FDD2_MOT_PIN 2;
#define FDD2_DIR_PIN 3;
#define FDD3_MOT_PIN 4;
#define FDD3_DIR_PIN 5;
#define FDD4_MOT_PIN 6;
#define FDD4_DIR_PIN 7;
#define FDD5_MOT_PIN 8;
#define FDD5_DIR_PIN 9;
#define FDD6_MOT_PIN 10;
#define FDD6_DIR_PIN 11;

#define DATA_PIN 12;
#define CLOC_PIN 13;
#define CLEN_PIN 14;
#define PL_PIN 15;

#define RESOLUTION 1000


/// period formula
/// 1,000,000 / (frequency * 2 * resolution)
/// NOTE : please note the unit of data
/// frequency (Hz)
/// resolution (ns)
int get_period(int f, int R){
	return 500000000U/(f * R);
}

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

void timer()
{
	steady_clock::time_point present;
	steady_clock::time_point begin=steady_clock::now();
	while(1)
	{
		present=steady_clock::now();
		if(duration_cast<nanoseconds>(present-begin) >= nanoseconds(RESOLUTION))
		{
			tick();
			begin=present;
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
  else if (current_pos <= MIN_TOGGLE_POS) {
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

void setup() {
	// GPIO library setup
	wiringPiSetup();
	
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
	pinMode(DATA_PIN, INPUNT);

	reset_all();
}

void loop() {
  while(1)
 	{

 	}
}

int main(void)
{
	setup();
	thread lp(loop);
	thread kb(set_tune);
	thread tk(timer); 
	lp.join();
	kb.join();
	tk.join();
	return 0;
}

void tick()
{
	if(PLAY_BTN_pushed_public)
	{
		if(current_period > 0)
		{
			current_tick++;
			if(current_period<=current_tick)
			{
				current_tick=0;
				togglePin(0,1);
			}
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
