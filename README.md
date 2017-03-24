# C MIDI Library
A very simple MIDI library which allows for reading and writing MIDI files in C.

Currently all MIDI events must be manually written, but the other structural aspects of the MIDI are present.

For reference on the format and the event structure check [this page](http://www.personal.kent.edu/~sbirch/Music_Production/MP-II/MIDI/midi_file_format.htm) out

Here is a __very__ simple program to create a MIDI.

```C
#include <stdio.h>

#include "midi.h"

int main(){
	struct Midi* m = malloc(sizeof(struct Midi));
	new_midi(m);
	//Set the MIDI to Mode 1, with 2 tracks, and 384 ticks per quarternote
	midi_add_header(m, 1, 2, 384);

	//A mode 1 MIDI file has a special "tempo map" track
	//this track contains meta events like Time Signature and Set Tempo
	struct MidiTrackChunk* tempo_track = midi_add_track(m);
	//set the time signature to 4/4, with 24 midi clocks per quarternote
	uint8_t time_sig_event[] = {0xFF, 0x58, 0x04, 0x04, 0x02, 0x18, 0x08}; 
	track_add_event_full(tempo_track, 0x00, time_sig_event, 7);

	//Set the tempo to 545454 micro seconds per quarternote
	uint8_t tempo_event[] = {0xFF, 0x51, 0x03, 0x08, 0x52, 0xAE};
	track_add_event_full(tempo_track, 0x00, tempo_event, 6);

	//Send the end of track event
	uint8_t end_event[] = {0xFF, 0x2F, 0x00};
	track_add_event_full(tempo_track, 0x00, end_event, 3);

	//Now comes the first "music" track
	struct MidiTrackChunk* track1 = midi_add_track(m);

	//set the name of the track to "Electric Piano"
	uint8_t track_name_command[] = {0xFF,0x03};
	char track_name[] = "Electric Piano";
	size_t name_length = strlen(track_name);
	size_t name_varlength_size;
	//MIDI uses variable length fields, we need this field and to know its size
	uint8_t* name_varlength = int_to_varlen(name_length, &name_varlength_size);
	//create space for the event (<event_type> <length> <event_data>)
	char* track_name_event = malloc(sizeof(char) * (2 + name_varlength_size + name_length))
	strncat(track_name_event, track_name_command, 2);
	strncat(track_name_event, name_varlength, name_varlength_size);
	strncat(track_name_event, track_name, name_length);
	track_add_event_full(track1, 0, track_name_event, 2 + name_varlength_size + name_length);

	//set the instrument to use on this channel to "Electric Piano" (this is the default, but whatever)
	uint8_t instrument_event[] = {0xC0, 0x01};
	track_add_event_full(track1, 0, instrument_event, 2);

	//add some note on/offs
	//this is Turn on Middle C, at about Mezzo-Forte
	uint8_t note[] = {0x90, 0x3C, 0x40};
	track_add_event_full(track1, 0, ev, 3);
	//and then turn it off (i'm going to reuse the array)
	note[0] = 0x80;
	//turn it off after 2 whole notes
	track_add_event_full(track1, 3072, ev, 3);
	//and stop the track (using the array from before)
	track_add_event_full(track1, 0x00, end_event, 3);

	FILE* f = fopen("test.midi", "wb");
	write_midi(m, f);
	fclose(f);
	printf("Finshed writing MIDI 'test.midi'\n");

	//don't forget to free after done
	//free_midi takes care of all of the things it contains as well (MidiTrackChunk, MidiEvent)
	free_midi(m);
	free(m);

	return 0;
}

```
