#include "MidiFile.h"
#include "Options.h"
#include <iostream>
#include<stdio.h>
#include <iomanip>


using namespace std;
int Max = 0;
int Min = 1000;
int main(int argc, char** argv) {
    Options options;
    options.process(argc, argv);
    MidiFile midifile;
    if (options.getArgCount() > 0) {
        midifile.read(options.getArg(1));
    } else {
        midifile.read(cin);
    }
    
    int tracks = midifile.getTrackCount();
    cout << "TPQ: " << midifile.getTicksPerQuarterNote() << endl;
    if (tracks > 1) {
        cout << "TRACKS: " << tracks << endl;
    }
    for (int track=0; track < tracks; track++) {
        if (tracks > 1) {
            cout << "\nTrack " << track << endl;
        }
        for (int event=0; event < midifile[track].size(); event++) {
    
			cout << dec << midifile[track][event].tick;
            cout << '\t' << hex;
  
			if(Max < (int)midifile[track][event][1])
				Max = (int)midifile[track][event][1];
			if(Min > (int)midifile[track][event][1])
				Min = (int)midifile[track][event][1];
	
			cout << (int)midifile[track][event][1] << ' ';
            cout << '\t' << hex;
   			printf("%d\n", (int)midifile[track][event][1]);
        }
    }
 	printf("Max : %d\t Min : %d\n",Max, Min);  
    return 0;
}
