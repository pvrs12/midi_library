# C MIDI Library
A very simple MIDI library which allows for reading and writing MIDI files in C.

Currently all MIDI events must be manually written, but the other structural aspects of the MIDI are present.

For reference on the format and the event structure check [this page](http://www.personal.kent.edu/~sbirch/Music_Production/MP-II/MIDI/midi_file_format.htm) out

Here is a program to create a __very__ simple MIDI.

```C
#include <stdio.h>
#include <string.h>

#include "midi.h"
#include "midi_helper.h"
#include "midi_constants.h"

int main(){
	//create a new midi 
	struct Midi* m = malloc(sizeof(struct Midi));
	new_midi(m);
	//set it to mode 0, with 1 track, and 384 ticks per quarternote
	midi_add_header(m, 0, 1, 384);

	//add a track
	struct MidiTrackChunk* track = midi_add_track(m);

	struct EventString* e = malloc(sizeof(struct EventString));

	//add a meta event which specifies the track name as "Trumpet"
	e = new_event_string(e);
	e = add_meta_message(e, META_TRACK_NAME);
	e = add_string(e, "Trumpet", strlen("Trumpet"));
	track_add_event_full(track, 0, e->event_string, e->event_string_len);
	free_event_string(e);

	//Add a voice event to change the instrument to the trumpet program
	//it's cheaper to reuse the same EventString (although not much)
	e = new_event_string(e);
	e = add_voice_message(e, VOICE_PROGRAM_CHANGE, CHANNEL_0);
	e = add_byte(e, INSTRUMENT_TRUMPET);
	track_add_event_full(track, 0, e->event_string, e->event_string_len);
	free_event_string(e);

	//start playing a C4 at mezzo-forte
	e = new_event_string(e);
	e = add_voice_message(e, VOICE_NOTE_ON, CHANNEL_0);
	e = add_byte(e, NOTE_C4);
	e = add_byte(e, VELOCITY_MEZZOFORTE);
	track_add_event_full(track, 0, e->event_string, e->event_string_len);
	free_event_string(e);

	//after 3072 ticks (2 whole notes) stop playing the C4
	e = new_event_string(e);
	e = add_voice_message(e, VOICE_NOTE_OFF, CHANNEL_0);
	e = add_byte(e, NOTE_C4);
	e = add_byte(e, VELOCITY_MEZZOFORTE);
	track_add_event_full(track, 3072, e->event_string, e->event_string_len);
	free_event_string(e);

	//end the track
	e = new_event_string(e);
	e = add_meta_message(e, META_END);
	e = add_string(e, "", 0);
	track_add_event_full(track, 0, e->event_string, e->event_string_len);
	free_event_string(e);

	free(e);


	FILE* f = fopen("test.mid", "wb");
	write_midi(m, f);
	fclose(f);
	printf("Write test.mid\n");
	

	return 0;
}
```

This could then be compiled by `gcc test.c -lmidi`. 

You can also simply run `make test` to build and and execute a small test program which will generate, write, then read a small MIDI file
