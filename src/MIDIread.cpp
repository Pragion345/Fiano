
#include "MidiFile.h"
#include "Options.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include<dirent.h>
using namespace std;
using namespace chrono;


int Mtrack = 0;
int tick = 0;
int Max_tick = 0;
int on_off = 0;
extern int musical_note_period[49];
extern int current_period[12];
char *arr[100] = {NULL};
extern bool isitplayingmode;
extern uint64_t btn_state;
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

int timer(long double seconds, MidiFile midifile)
{
    int count = 0;
    MidiEvent *mev = &midifile[0][0];
    int event = 1;
	steady_clock::time_point present, begin;
	begin = steady_clock::now();
	int i;
	while(tick < Max_tick)
	{
		if(isitplayingmode == false)
			return 1000;
		present = steady_clock::now();
		if((i=duration_cast<duration<int,micro>>(present - begin).count())     >= seconds)
		{
			begin = present;
			tick++;
            
		}
		else if(btn_state) 
		{
			switch(btn_state)
			{
				case 1:
					return -1;
				case 2:
					return 1;
				default:
					break;
			}
		}
        else
        {
            if(tick >= mev -> tick)
            {
                
                if(check(mev))
                {
                    int note = (int)(*mev)[1] - 36;
                    if(note < 0)
					{
						while(note >= 0)
							note += 12;
					}
					else if (note > 48)
					{
						while(note <= 48)
							note -= 48;
					}
					else
						;

                    if(on_off)
                    {
                        current_period[((mev -> track) - 1) * 2] = musical_note_period[note];
                    }
                    else
                    {
                        current_period[((mev -> track) - 1) * 2] = 0;
                    }
                    
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
	return 1;
}

long double OneTick_seconds(int TPQ, MidiEvent * mev)
{
	long int T = ((*mev)[3] << 16) + ((*mev)[4] << 8) + (*mev)[5];
	long double result = (long double)T / TPQ;
	return result;
}

long double play_MIDI(char *arp[100], int loc)
{
	int tick = 0;
    MidiFile midifile; 
	
   	midifile.read(*(arp + loc));
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
	sleep(1);
	return timer(timer_microseconds, midifile);
}

int call_midi()
{
	DIR *midi;
	struct dirent *dirp;
	midi = opendir("midi");
	int i = 0;
	while((dirp = readdir(midi)))
	{
		if(strstr(dirp -> d_name, "mid"))
		{
			arr[i] = (char *)malloc(sizeof(char) * (strlen(dirp -> d_name) + 1));
			strcpy(arr[i], dirp -> d_name);
			i++;
		}
		else
		{
			;
		}
	}
	return i - 1;
}

void init_note_period(char * first_note);
void MMM()
{
	// ./a.out 이 1개

	int Music_count = 1 + call_midi();
	int a = 1;
	btn_state = 1;
	while(a)
	{
		a += play_MIDI(arr, a - 1);
		if(a == Music_count + 1)
		{
			a = 1;
		}
		else if(a == 0)
		{
			a = Music_count;
		}
		else if( a > Music_count)
		{
			a = 0;
		}
		else
			;
	}
}

#ifdef DEBUG
int main()
{
	MMM();
}
#endif
