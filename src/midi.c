#include "midi.h"
#include "midi_constants.h"

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <assert.h>

//for htonl 
#include <netinet/in.h>

uint32_t varlen_to_int (const uint8_t* var, size_t* size) {
	uint32_t val = 0;
	size_t _size = 1;
	while(1) {
		val |= (*var) & 0x7F;
		if(!((*var) >> 7)){
			break;
		}
		val <<= 7;
		var++;
		_size++;
	}
	if(size){
		(*size) = _size;
	}
	return val;	
}

uint8_t* int_to_varlen(uint32_t val, size_t* _size)	{
	uint32_t val2 = val;
	//determine the necessary size of the variable length quantity
	size_t count = 1;
	while(val2 >>= 1){
		count++;
	}

	//determine the number of sets of 7 there are
	size_t size = count / 7 + (count % 7 ? 1 : 0);
	uint8_t* var = malloc(sizeof(uint8_t) * size);
	if(_size){
		(*_size) = size;
	}

	for(size_t i = 0; i < size; ++i){
		//zero out the MSB
		var[size - i - 1] = val & 0x7F;
		if(i){
			//if this is not the lowest byte
			//then set the MSB to 1
			var[size - i - 1] |= 0x80;
		}
		//do next set of 7
		val >>= 7;
	}
	return var;
}

void new_midichunk(struct MidiChunk* chunk, enum ChunkType type){
	if(type == CHUNK_HEADER){
		memcpy(chunk->type, "MThd", TYPE_LEN);
	} else {
		memcpy(chunk->type, "MTrk", TYPE_LEN);
	}
	chunk->type_e = type;

	chunk->chunk = NULL;
}

void free_midichunk(struct MidiChunk* chunk){
	if(chunk->type_e == CHUNK_HEADER){
		struct MidiHeaderChunk* header = (struct MidiHeaderChunk*) chunk->chunk;
		free_midi_header(header);
		free(header);
	} else {
		struct MidiTrackChunk* track = (struct MidiTrackChunk*) chunk->chunk;
		free_midi_track(track);
		free(track);
	}
}

void new_midi(struct Midi* midi) {
	midi->chunk_count = 0;
	midi->chunks = malloc(sizeof(struct MidiChunk*) * midi->chunk_count);
	midi->header = NULL;
}

void free_midi(struct Midi* midi){
	for(size_t i = 0; i < midi->chunk_count;++i){
		free_midichunk(midi->chunks[i]);
		free(midi->chunks[i]);
	}
	free(midi->chunks);
}

struct MidiChunk* midi_add_chunk(struct Midi* midi){
	midi->chunk_count++;
	midi->chunks = realloc(midi->chunks, sizeof(struct MidiChunk*) * midi->chunk_count);

	struct MidiChunk* chunk = malloc(sizeof(struct MidiChunk));
	midi->chunks[midi->chunk_count - 1] = chunk;
	return chunk;
}

struct MidiHeaderChunk* midi_add_header(struct Midi* midi, uint16_t format, uint16_t tracks, uint16_t division){
	// A Midi cannot have more than one header track
	// assert that it does not already have one
	assert(!midi->header); 

	struct MidiChunk* chunk = midi_add_chunk(midi);
	new_midichunk(chunk, CHUNK_HEADER);
	struct MidiHeaderChunk* header = malloc(sizeof(struct MidiHeaderChunk));
	new_midi_header(header, HEADER_LEN, format, tracks, division);
	chunk->chunk = header;
	midi->header = header;
	return header;
}

struct MidiTrackChunk* midi_add_track(struct Midi* midi){
	struct MidiChunk* chunk = midi_add_chunk(midi);
	new_midichunk(chunk, CHUNK_TRACK);
	struct MidiTrackChunk* track = malloc(sizeof(struct MidiTrackChunk));
	new_midi_track(track);
	chunk->chunk = track;
	return track;
}

void new_midi_header(struct MidiHeaderChunk* header, uint32_t length, uint16_t format, uint16_t tracks, uint16_t division){
	header->length = length;

	header->format = format;
	header->tracks = tracks;
	header->division = division;
}

void free_midi_header(struct MidiHeaderChunk* header){
	//nothing is allocated
}

void new_midi_event(struct MidiEvent* event, uint32_t delta_time, const uint8_t* ev, size_t event_length){
	event->delta_time = delta_time;
	event->event = malloc(sizeof(uint8_t) * event_length);
	event->event_len = event_length;
	memcpy(event->event, ev, event_length);
}

void free_midi_event(struct MidiEvent* event){
	free(event);
}

struct MidiEvent* parse_midi_event(const uint8_t* event, size_t* size_read){
	size_t delta_time_size;
	uint32_t delta_time = varlen_to_int(event, &delta_time_size);
	const uint8_t* event_code = event + delta_time_size;
	uint8_t event_type = event_code[0];

	size_t event_size;
	if((event_type & 0xF0) < 0x80){
		//this should be an error...
		//no wait. it should jsut add to the previous event.
		//fuck running status
	} else if((event_type & 0xF0) < 0xF0){
		//this is a midi voice or mode message
		event_size = parse_midi_voice_event(event_code);
	} else {
		if((event_type == 0xF0) || event_type == 0xF7){
			//this is a sysex message (<type> <len> <data>)
			event_size = parse_midi_sysex_event(event_code);
		} else {
			//this is a meta message (<type> <subtype> <len> <data)
			event_size = parse_midi_meta_event(event_code);
		}
	} 
	struct MidiEvent* e = malloc(sizeof(struct MidiEvent));
	new_midi_event(e, delta_time, event_code, event_size);

	if(size_read){
		(*size_read) = event_size + delta_time_size;
	}
	return e;
}

size_t parse_midi_voice_event(const uint8_t* event_code){
	uint8_t status = event_code[0] & 0xF0;
	size_t read;
	switch(status){
		case(VOICE_NOTE_OFF):
		case(VOICE_NOTE_ON):
		case(VOICE_POLYPHONIC_PRESSURE):
		case(VOICE_CONTROLLER_CHANGE):
		case(VOICE_PITCH_BEND):
			read = 3;
			break;
		case(VOICE_PROGRAM_CHANGE):
		case(VOICE_CHANNEL_KEY_PRESSURE):
			read = 2;
			break;
	}
	return read;
}

size_t parse_midi_sysex_event(const uint8_t* event_code){
	size_t len_size;
	uint32_t len = varlen_to_int(event_code + 1, &len_size);
	return len_size + len + 1;
}

size_t parse_midi_meta_event(const uint8_t* event_code){
	size_t len_size;
	uint32_t len = varlen_to_int(event_code + 2, &len_size);
	return len_size + len + 2;
}

void new_midi_track(struct MidiTrackChunk* track){
	track->event_count = 0;

	track->events = malloc(sizeof(struct MidiEvent*) * track ->event_count);
}

void free_midi_track(struct MidiTrackChunk* track){
	for(size_t i = 0; i < track->event_count; ++i){
		free_midi_event(track->events[i]);
	}
	free(track->events);
}

size_t track_length(struct MidiTrackChunk* track){
	size_t s = 0;
	for(size_t i = 0; i < track->event_count; ++i){
		struct MidiEvent* e = track->events[i];
		size_t sz;
		free(int_to_varlen(e->delta_time, &sz));
		s += sz;
		s += e->event_len;
	}
	return s;
}

struct MidiEvent* track_add_event(struct MidiTrackChunk* track){
	track->event_count++;
	track->events = realloc(track->events, sizeof(struct MidiEvent*) * track->event_count);

	struct MidiEvent* event = malloc(sizeof(struct MidiEvent));
	track->events[track->event_count - 1] = event;
	return event;
}

struct MidiEvent* track_add_event_full(struct MidiTrackChunk* track, uint32_t delta_time, const uint8_t* event_data, size_t event_data_len){
	struct MidiEvent* event = track_add_event(track);
	new_midi_event(event, delta_time, event_data, event_data_len);
	return event;
}

void track_add_event_existing(struct MidiTrackChunk* track, struct MidiEvent* event){
	track->event_count++;
	track->events = realloc(track->events, sizeof(struct MidiEvent*) * track->event_count);

	track->events[track->event_count - 1] = event;
}

void write_uint16_t(uint16_t data, FILE* f){
	uint16_t d = htons(data);
	fwrite(&d, sizeof(uint16_t), 1, f);
}

void write_uint32_t(uint32_t data, FILE* f){
	uint32_t d = htonl(data);
	fwrite(&d, sizeof(uint32_t), 1, f);
}

uint16_t read_uint16_t(FILE* f){
	uint16_t d;
	fread(&d, sizeof(uint16_t), 1, f);
	return ntohs(d);
}

uint32_t read_uint32_t(FILE* f){
	uint32_t d;
	fread(&d, sizeof(uint32_t), 1, f);
	return ntohl(d);
}

void write_midi(struct Midi* midi, FILE* f) {
	for(uint32_t i = 0; i < midi->chunk_count; ++i){
		struct MidiChunk* chunk = midi->chunks[i];
		fwrite(chunk->type, sizeof(uint8_t), TYPE_LEN, f);
		if(chunk->type_e == CHUNK_HEADER){
			struct MidiHeaderChunk* header = (struct MidiHeaderChunk*) chunk->chunk;
			write_uint32_t(header->length, f);
			write_uint16_t(header->format, f);
			write_uint16_t(header->tracks, f);
			write_uint16_t(header->division, f);
		} else {
			struct MidiTrackChunk* track = (struct MidiTrackChunk*) chunk->chunk;
			write_uint32_t(track_length(track), f);
			for(size_t i = 0; i < track->event_count; ++i){
				size_t time_size;
				uint8_t* time = int_to_varlen(track->events[i]->delta_time, &time_size);
				fwrite(time, sizeof(uint8_t), time_size, f);
				fwrite(track->events[i]->event, sizeof(uint8_t), track->events[i]->event_len, f);
				free(time);
			}
		}
	}
}

struct Midi* read_midi(FILE* f){	
	struct Midi* midi = malloc(sizeof(struct Midi));
	new_midi(midi);

	uint8_t chunk_head[TYPE_LEN];
	fread(chunk_head, sizeof(uint8_t), TYPE_LEN, f);
	uint32_t size_head = read_uint32_t(f);

	//The size of the header must be equal to HEADER_LEN
	assert(size_head == HEADER_LEN);
	uint16_t format = read_uint16_t(f);
	uint16_t tracks = read_uint16_t(f);
	uint16_t division = read_uint16_t(f);

	midi_add_header(midi, format, tracks, division);
	for(size_t i = 0; i < tracks; ++i){
		fread(chunk_head, sizeof(uint8_t), TYPE_LEN, f);
		uint32_t size_track = read_uint32_t(f);
		struct MidiTrackChunk* track = midi_add_track(midi);

		//read all the events in the track
		uint8_t* event = malloc(sizeof(uint8_t) * size_track);
		fread(event, sizeof(uint8_t), size_track, f);

		size_t read = 0;
		while(1){
			size_t event_size;
			struct MidiEvent* e = parse_midi_event((event + read), &event_size);
			track_add_event_existing(track, e);
			read += event_size;
			if(read == size_track){
				break;
			} else if(read > size_track){
				//error
				//somehow we read too much...
				assert(0);
				break;
			}
		}
		free(event);
	}
	return midi;
}
