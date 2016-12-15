#include <stdio.h>
#include <string.h>
main(int arg,char *arc[])
{
	switch(arc[1][0])
	{

		case 'B': system("g++ -D MODEBTN main.cpp MIDIread.cpp -I../midifile/include libmidifile.a -std=c++11 -lwiringPi -lpthread -o modebtn");
				  printf("compile : modebtn\n");
		break;
		case '1': system("g++ main.cpp MIDIread.cpp -I../midifile/include libmidifile.a -std=c++11 -lwiringPi -lpthread");
				  printf("compile with no define\n");
		break;
		case 'S':  system("g++ main.cpp MIDIread.cpp -I../midifile/include libmidifile.a -std=c++11 -lwiringPi -lpthread -D SHIFT_TEST -o shifttest");
				   printf("compile : shifttest\n");
		break;
		case 'K':	system("g++ main.cpp MIDIread.cpp -I../midifile/include libmidifile.a -std=c++11 -lwiringPi -lpthread -D KEYBOARD -o keyboard");
					printf("compile : keyboard\n");
		break;
		case 'M':	system("g++ main.cpp MIDIread.cpp -I../midifile/include libmidifile.a -std=c++11 -lwiringPi -lpthread -D MIDI -o playmidi");
					printf("compile : playmidi\n");
		break;
		case 'A':

		break;
		default :
					system("g++ main.cpp MIDIread.cpp -I../midifile/include libmidifile.a -std=c++11 -lwiringPi -lpthread");
				  printf("compile with no define\n");
		break;
	}
}
