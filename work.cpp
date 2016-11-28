#include<thread>
#include<stdio.h>
#include<wiringPi.h>
#include<termios.h>
#include<unistd.h>
#include<chrono>
#include<ctime>
#include<limits.h>
using namespace std;
using namespace chrono;
#define MAX_POS 158
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
int current_pos=0,current_direct=HIGH,current_period,current_tick;
int timing=850,currentState[1],currentdirState;
bool btnp;

void tick()
{
	if(btnp)
	{
		if(current_period>0)
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
		if(duration_cast<nanoseconds>(present-begin) >=nanoseconds(timing))
		{
		//	togglePin(0,1);
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
  //  delay(2);
  } 
  else if (current_pos <= 10) {
currentdirState = LOW;
    digitalWrite(direction_pin,LOW);
  //  delay(2);
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
	delay(5);
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
void setup() {
	wiringPiSetup();
  // put your setup code here, to run once:
  pinMode(0,OUTPUT);
  pinMode(1,OUTPUT);
  pinMode(3,INPUT);
  reset(0);
}

int last_pushed = 0;
void loop() {
  while(1)
  {
  int pushed;
  pushed=digitalRead(2);
  if (pushed != last_pushed)
  {
    delay(5);
    pushed = digitalRead(2);
  }
  if(pushed)
  {
   // togglePin(0,1);
  	btnp=true;
  }
  else
  {
     btnp=false;
	  /*if(currentState==HIGH)
    {Serial.println("HIGH");}
    else
    {Serial.println("LOW");}
    //Serial.println("off");
    //digitalWrite(2,LOW);*/
  }
  last_pushed = pushed;
 	
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
				printf("fa");
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
					printf("input timing : ");
					scanf(" %d",&timing);
					printf("%d\n",timing);
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
