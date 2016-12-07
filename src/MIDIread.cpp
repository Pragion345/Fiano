#include "MidiFile.h"
#include "Options.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <ctime>
#include <unistd.h>
#include <chrono>
#include <thread>
#include<stdlib.h>
#include<thread>
#include<termios.h>
#include<limits.h>
#include<math.h>

using namespace std;
using namespace chrono;


int Mtrack = 0;
int tick = 0;
int Max_tick = 0;
int on_off = 0;
extern int musical_note_period[49];
extern int current_period[12];

/*
	해당 틱에 도착했을때 검사할게 여러가지있습니다
	
	1. On(144 ~ 159),(0x90 ~ 0x9F) or OFF (128 ~ 143),(0x80 ~ 0x8F)
	2. velocity On (0) off(!0)
	3. octave (C1 ~ B4),(36 ~ 83) 48개
   */

int check(MidiEvent* mev)
{
	int on1 = 0x90;
	int on2 = 0x9f;
	int off1 = 0x80;
	int off2 = 0x8f;
	
	int off = 0;
	int on = 1;
	
	int info1 = (int)(*mev)[0];
	int info2 = (int)(*mev)[2];

	if(info1 >= on1 && info1 <= on2)
	{
		if(info2)
			on_off = on;
		else
			on_off = off;
		return 1;
	}
	else if(info1 >= off1 && info1 <= off2)
	{
		on_off = off;
		return 1;
	}
	else
	{
		return 0;
	}
	return 0;
}

void push(MidiFile midifile)
{
	int count = 0;
	MidiEvent *mev = &midifile[0][0];
	int event = 1;
	while(tick < Max_tick + 1000)
	{
		if(tick >= mev -> tick)
		{
			if(check(mev))
			{
				int note = (int)(*mev)[1] - 36;
				
				printf("KEY : %d, ONOFF: %d\n", musical_note_period[note], on_off);

			}
			else
			{
				if((*mev)[0] == 0xFF && tick > 1)
					count++;
				if(count == Mtrack - 1)
					break;
			}
			mev = &midifile[0][event++];
		}
	}

}

void timer(long double seconds)
{
	steady_clock::time_point present, begin;
	begin = steady_clock::now();
	int i;
	while(tick < Max_tick)
	{
		present = steady_clock::now();
		if((i=duration_cast<duration<int,micro>>(present - begin).count())     >= seconds)
		{
			begin = present;
			tick++;
		}
	}
}

long double OneTick_seconds(int TPQ, MidiEvent * mev)
{
	long int T = ((*mev)[3] << 16) + ((*mev)[4] << 8) + (*mev)[5];
	long double result = (long double)T / TPQ;
	return result;
}

long double play_MIDI(int argc, char **argv)
{
	int tick = 0;
    Options options;
    options.process(argc, argv);
    MidiFile midifile; 
	
	if (options.getArgCount() > 0) {
        midifile.read(options.getArg(1));
    } else {
        midifile.read(cin);
    }
	Mtrack = midifile.getTrackCount();
    midifile.joinTracks();
    
	int TPQ = midifile.getTicksPerQuarterNote();
    MidiEvent* mev = &midifile[0][0];
	int event = 0;

	while( !((*mev)[0] == 0xff && (*mev)[1] == 0x51) )
	{
		mev = &midifile[0][++event];
	}
	long double timer_microseconds = OneTick_seconds(TPQ,mev);
	mev = &midifile[0][midifile[0].size() - 1];
	Max_tick = mev -> tick;
	printf("Max tick %d\n", Max_tick);
	printf("Music Start\n");
	sleep(1);
	thread ph(push,midifile);
	thread tk(timer,timer_microseconds);
	tk.join();
	ph.join();
	printf("Music Finish\n");
	return timer_microseconds;
}

void init_note_period(char * first_note);

int main(int argc, char **argv)
{
	// ./a.out 이 1개
	char ** M = "Tetris.mid";
	char note[] = "C1";
	init_note_period(note);
	int a = play_MIDI(1, M);

	return 0;
}
