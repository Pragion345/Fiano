#include<thread>
#include<stdio.h>
#include<wiringPi.h>
#include<wiringShift.h>
#include<termios.h>
#include<unistd.h>
#include<chrono>
#include<ctime>
#include<limits.h>

using namespace std;
using namespace chrono;

/** 
	상수필드
	*/
#define MAX_POS 158

const uint8_t DATA_PIN = 2;
const uint8_t CLOC_PIN = 3;
const uint8_t PLAY_PIN = 4;

/// period 구하는 공식
/// 1,000,000 / (진동수 * 2 * 타이머간격)
int get_period(int frequency, int R){
	return 500000/(frequency * R)
}

void togglePin(int, int);

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


int current_pos=0, current_direct=HIGH, current_period, current_tick;
int interval=850, currentState[1], currentdirState;
bool PLAY_BTN_pushed_public = 1;

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

void timer()
{
	steady_clock::time_point present;
	steady_clock::time_point begin=steady_clock::now();
	while(1)
	{
		present=steady_clock::now();
		if(duration_cast<nanoseconds>(present-begin) >=nanoseconds(interval))
		{
			tick();
			begin=present;
		}
	}
}

void togglePin(int pin, int direction_pin) {

  //Switch directions if end has been reached
  if (current_pos >= 153) {
    currentdirState = HIGH;
    digitalWrite(direction_pin,HIGH);
  } 
  else if (current_pos <= 10) {
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
  digitalWrite(pin,currentState[0]);
  currentState[0] = ~currentState[0];
	//printf("%d\n",current_pos);
}

void reset(int pin)
{
  digitalWrite(pin+1,HIGH); // Go in reverse
  int s;
  for (s=0;s<158;s+=2){ //Half max because we're stepping directly (no toggle)
    digitalWrite(pin,HIGH);
    digitalWrite(pin,LOW);
    delay(5);
  }
  current_pos = 0; // We're reset.
  digitalWrite(pin+1,LOW);
  current_direct = 0; // Ready to go forward.
}

/**
	Setup 초기화 코드
 	프로그램 시작 후 단 한번만 실행됩니다.	
	*/

void setup() {
	wiringPiSetup();
  pinMode(0,OUTPUT);
  pinMode(1,OUTPUT);
	pinMode(2,INPUT);
  pinMode(3,OUTPUT);
  reset(0);
}

void loop() {
	static int last_pushed_btn = 0;
	int pushed_btn = 0;
  while(1)
 	{
		pushed_btn = digitalRead(PLAY_PIN);

		if (last_pushed_btn != pushed_btn)
		{
				delay(4);
				pushed_btn = digitalRead(PLAY_PIN);
				if (last_pushed_btn != pushed_btn)
					PLAY_BTN_pushed_public = ~PLAY_BTN_pushed_public;
		}
 	}
}

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
			case ';':
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
