#include "MidiFile.h"
#include "Options.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <wiringPi.h>

using namespace std;

long double OneTick_seconds(int TPQ, MidiEvent * mev)
{
	long int T = ((*mev)[3] << 16) + ((*mev)[4] << 8) + (*mev)[5];
	long double result = (long double)T / TPQ;
	return result;
}

int play_MIDI(int argc, char **argv)
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
    midifile.joinTracks();
    
	int TPQ = midifile.getTicksPerQuarterNote();
    MidiEvent* mev = &midifile[0][0];
	int event = 0;

	while( !((*mev)[0] == 0xff && (*mev)[1] == 0x51) )
	{
		mev = &midifile[0][++event];
	}
	long double timer_microseconds = OneTick_seconds(TPQ,mev);
	
	while(tick < (int)(*mev[i]).tick)
	{
		delayMicrosecond(timer_microseconds);
	}

	return 1;
}
int main(int argc, char **argv)
{
	
	int a = play_MIDI(argc, argv);
	
}
