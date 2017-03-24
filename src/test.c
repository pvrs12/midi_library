#include "include/midi.h"
#include "include/midi_helper.h"
#include "include/midi_constants.h"

#include <string.h>

void test_varlen(){
	size_t size;
	uint8_t* num = int_to_varlen(0x0FFFFFFF, &size);
	size_t read_size;
	uint32_t len = varlen_to_int(num, &read_size);
	for(size_t i = 0; i < size;++i){
		printf("%hhx ",num[i]);
	}
	printf("\t|\t%x\n", len);
	free(num);
}

void test_read_write(){
	struct Midi* m = malloc(sizeof(struct Midi));
	new_midi(m);
	//mode 1. 3 tracks. 120 ticks/quarternote
	midi_add_header(m, 1, 3, 384);
	
	struct MidiTrackChunk* tempo_track = midi_add_track(m);
	// {{{ Tempo track
	// set the time signature
	// 0x04/2^(0x02) (4/4)
	// 0x18 midi clocks per metronome tick (24 midi clocks per quarter note (in this signature))
	// 0x08 32notes per 24 midi clocks
	uint8_t time_sig_event[] = {0xff, 0x58, 0x04, 0x04, 0x02, 0x18, 0x08};
	track_add_event_full(tempo_track, 0x00, time_sig_event, 7);
	// set tempo
	// microseconds per quarter note
	// 0x08 0x52 0xae (545454) #default to 120bpm
	uint8_t tempo_event[] = {0xff, 0x51, 0x03, 0x08, 0x52, 0xae};
	track_add_event_full(tempo_track, 0x00, tempo_event, 6);
	// stop the track
	uint8_t stop_event[] = {0xff, 0x2f, 0x00};
	track_add_event_full(tempo_track, 0x00, stop_event, 3);
	//}}}

	//let's make a scale!
	struct MidiTrackChunk* track1 = midi_add_track(m);

	uint8_t track_name[] = {0xFF,0x03,0x08,0x54,0x72,0x6f,0x6d,0x62,0x6f,0x6e,0x65};
	track_add_event_full(track1, 0,	track_name, 11);
	//{{{ Ascending ionian scale trombone: c4-c5
	//set instrument = trombone
	uint8_t instrument_event[2] = {0xC0, 58};
	track_add_event_full(track1, 0, instrument_event, 2);
	
	//noteOn, ch0, middle_c, mf
	uint8_t ev[3] = {0x90, 0x3C, 0x40};
	track_add_event_full(track1, 0, ev, 3);
	ev[0] = 0x80;
	track_add_event_full(track1, 384, ev, 3);
	ev[0] = 0x90;
	ev[1] = 62;
	track_add_event_full(track1, 0, ev, 3);
	ev[0] = 0x80;
	track_add_event_full(track1, 384,ev,3);
	ev[0] = 0x90;
	ev[1] = 64;
	track_add_event_full(track1, 0,ev,3);
	ev[0] = 0x80;
	track_add_event_full(track1, 384,ev,3);
	ev[0] = 0x90;
	ev[1] = 65;
	track_add_event_full(track1, 0,ev,3);
	ev[0] = 0x80;
	track_add_event_full(track1, 384,ev,3);
	ev[0] = 0x90;
	ev[1] = 67;
	track_add_event_full(track1, 0,ev,3);
	ev[0] = 0x80;
	track_add_event_full(track1, 384,ev,3);
	ev[0] = 0x90;
	ev[1] = 69;
	track_add_event_full(track1, 0,ev,3);
	ev[0] = 0x80;
	track_add_event_full(track1, 384,ev,3);
	ev[0] = 0x90;
	ev[1] = 71;
	track_add_event_full(track1, 0,ev,3);
	ev[0] = 0x80;
	track_add_event_full(track1, 384,ev,3);
	ev[0] = 0x90;
	ev[1] = 72;
	track_add_event_full(track1, 0,ev,3);
	ev[0] = 0x80;
	track_add_event_full(track1, 384,ev,3);

	ev[0] = 0xFF;
	ev[1] = 0x2F;
	ev[2] = 0x00;
	track_add_event_full(track1, 0, ev, 3);
	//}}}

	//and the descending version on a second track!
	struct MidiTrackChunk* track2 = midi_add_track(m);
	//{{{ Descending ionian scale piano: c5-c4
	uint8_t ev2[3] = {0x91, 72, 0x40};
	track_add_event_full(track2, 0, ev2, 3);
	ev2[0] = 0x81;
	track_add_event_full(track2, 384, ev2, 3);
	ev2[0] = 0x91;
	ev2[1] = 71;
	track_add_event_full(track2, 0, ev2, 3);
	ev2[0] = 0x81;
	track_add_event_full(track2, 384,ev2,3);
	ev2[0] = 0x91;
	ev2[1] = 69;
	track_add_event_full(track2, 0,ev2,3);
	ev2[0] = 0x81;
	track_add_event_full(track2, 384,ev2,3);
	ev2[0] = 0x91;
	ev2[1] = 67;
	track_add_event_full(track2, 0,ev2,3);
	ev2[0] = 0x81;
	track_add_event_full(track2, 384,ev2,3);
	ev2[0] = 0x91;
	ev2[1] = 65;
	track_add_event_full(track2, 0,ev2,3);
	ev2[0] = 0x81;
	track_add_event_full(track2, 384,ev2,3);
	ev2[0] = 0x91;
	ev2[1] = 64;
	track_add_event_full(track2, 0,ev2,3);
	ev2[0] = 0x81;
	track_add_event_full(track2, 384,ev2,3);
	ev2[0] = 0x91;
	ev2[1] = 62;
	track_add_event_full(track2, 0,ev2,3);
	ev2[0] = 0x81;
	track_add_event_full(track2, 384,ev2,3);
	ev2[0] = 0x91;
	ev2[1] = 60;
	track_add_event_full(track2, 0,ev2,3);
	ev2[0] = 0x81;
	track_add_event_full(track2, 384,ev2,3);

	ev2[0] = 0xFF;
	ev2[1] = 0x2F;
	ev2[2] = 0x00;
	track_add_event_full(track2, 0, ev2, 3);
	//}}}

	FILE* f = fopen("test.mid", "wb");
	write_midi(m, f);
	fclose(f);
	free_midi(m);
	free(m);
	printf("Wrote to test.mid\n");

	FILE* fr = fopen("test.mid", "rb");
	struct Midi* mid = read_midi(fr);
	fclose(f);
	printf("Read test.mid\n\n\n");
	printf("It has %d tracks\nit is a %d format midi\nand its divisions are %d\n\n", mid->header->tracks, mid->header->format, mid->header->division);
	for(size_t i = 1; i < mid->header->tracks + 1;++i){
		struct MidiTrackChunk* track = ((struct MidiTrackChunk*)mid->chunks[i]->chunk);
		printf("\tTrack[%zu] has %zu events\n", i, track->event_count);
		for(size_t j = 0; j < track->event_count; ++j){
			struct MidiEvent* event = track->events[j];
			printf("\t\tEvent[%zu]: <%u> - ", j, event->delta_time);
			for(size_t k = 0; k < event->event_len; ++k){
				printf("%02X ", event->event[k]);
			}
			printf("\n");
		}
	}
	free_midi(mid);
	free(mid);
}

void test_helper_midi(){
	struct Midi* m = malloc(sizeof(struct Midi));
	new_midi(m);
	midi_add_header(m, 0, 1, 384);
	struct MidiTrackChunk* track = midi_add_track(m);

	struct EventString* e = malloc(sizeof(struct EventString));

	e = new_event_string(e);
	e = add_meta_message(e, META_TRACK_NAME);
	e = add_string(e, "Trumpet", strlen("Trumpet"));
	track_add_event_full(track, 0, e->event_string, e->event_string_len);
	free_event_string(e);

	//it's cheaper to reuse the same EventString (although not much)
	e = new_event_string(e);
	e = add_voice_message(e, VOICE_PROGRAM_CHANGE, CHANNEL_0);
	e = add_byte(e, INSTRUMENT_TRUMPET);
	track_add_event_full(track, 0, e->event_string, e->event_string_len);
	free_event_string(e);

	e = new_event_string(e);
	e = add_voice_message(e, VOICE_NOTE_ON, CHANNEL_0);
	e = add_byte(e, NOTE_C4);
	e = add_byte(e, VELOCITY_MEZZOFORTE);
	track_add_event_full(track, 0, e->event_string, e->event_string_len);
	free_event_string(e);

	e = new_event_string(e);
	e = add_voice_message(e, VOICE_NOTE_OFF, CHANNEL_0);
	e = add_byte(e, NOTE_C4);
	e = add_byte(e, VELOCITY_MEZZOFORTE);;
	track_add_event_full(track, 3072, e->event_string, e->event_string_len);
	free_event_string(e);

	e = new_event_string(e);
	e = add_meta_message(e, META_END);
	e = add_string(e, "", 0);
	track_add_event_full(track, 0, e->event_string, e->event_string_len);
	free_event_string(e);

	free(e);


	FILE* f = fopen("helper.mid", "wb");
	write_midi(m, f);
	fclose(f);
	printf("Write helper.mid\n");
	
}

void test_errors(){
	struct Midi* m = malloc(sizeof(struct Midi));
	new_midi(m);
	midi_add_header(m, 0, 1, 384);
	
	//this will crash the program due to the failed assertion
	midi_add_header(m, 0, 1, 384);

	free_midi(m);
	free(m);
}

int main(){
	test_varlen();
	test_read_write();
	test_helper_midi();

	//test_errors();
	return 0;
}
