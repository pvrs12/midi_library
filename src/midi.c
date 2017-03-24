#include "include/midi.h"

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//for htonl 
#include <netinet/in.h>

//{{{ varlen_to_int
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
//}}}

//{{{ int_to_varlen
uint8_t* int_to_varlen(uint32_t val, size_t* _size)	{
	uint32_t val2 = val;
	//determine the necessary size of the variable length quantity
	size_t count = 1;
	while(val2>>=1){
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
//}}}

//{{{ new_midichunk
void new_midichunk(struct MidiChunk* chunk, enum ChunkType type){
	if(type == CHUNK_HEADER){
		memcpy(chunk->type, "MThd", TYPE_LEN);
	} else {
		memcpy(chunk->type, "MTrk", TYPE_LEN);
	}
	chunk->type_e = type;

	chunk->chunk = NULL;
}
//}}}

//{{{ free_minichunk
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
//}}}

//{{{ new_midi
void new_midi(struct Midi* midi) {
	midi->chunk_count = 0;
	midi->chunks = malloc(sizeof(struct MidiChunk*) * midi->chunk_count);
	midi->header = NULL;
}
//}}}

//{{{ free_midi
void free_midi(struct Midi* midi){
	for(size_t i = 0; i < midi->chunk_count;++i){
		free_midichunk(midi->chunks[i]);
		free(midi->chunks[i]);
	}
	free(midi->chunks);
}
//}}}

//{{{ midi_add_chunk
struct MidiChunk* midi_add_chunk(struct Midi* midi){
	midi->chunk_count++;
	midi->chunks = realloc(midi->chunks, sizeof(struct MidiChunk*) * midi->chunk_count);

	struct MidiChunk* chunk = malloc(sizeof(struct MidiChunk));
	midi->chunks[midi->chunk_count - 1] = chunk;
	return chunk;
}
//}}}

//{{{ midi_add_header
struct MidiHeaderChunk* midi_add_header(struct Midi* midi, uint16_t format, uint16_t tracks, uint16_t division){
	if(midi->header){
		//some error
	}
	struct MidiChunk* chunk = midi_add_chunk(midi);
	new_midichunk(chunk, CHUNK_HEADER);
	struct MidiHeaderChunk* header = malloc(sizeof(struct MidiHeaderChunk));
	new_midi_header(header, HEADER_LEN, format, tracks, division);
	chunk->chunk = header;
	midi->header = header;
	return header;
}
//}}}

//{{{ midi_add_track
struct MidiTrackChunk* midi_add_track(struct Midi* midi){
	struct MidiChunk* chunk = midi_add_chunk(midi);
	new_midichunk(chunk, CHUNK_TRACK);
	struct MidiTrackChunk* track = malloc(sizeof(struct MidiTrackChunk));
	new_midi_track(track);
	chunk->chunk = track;
	return track;
}
//}}}

//{{{ new_midi_header
void new_midi_header(struct MidiHeaderChunk* header, uint32_t length, uint16_t format, uint16_t tracks, uint16_t division){
	header->length = length;

	header->format = format;
	header->tracks = tracks;
	header->division = division;
}
//}}}

//{{{ free_midi_header
void free_midi_header(struct MidiHeaderChunk* header){
	//nothing is allocated
}
//}}}

//{{{ new_midi_event
void new_midi_event(struct MidiEvent* event, uint32_t delta_time, const uint8_t* ev, size_t event_length){
	event->delta_time = delta_time;
	event->event = malloc(sizeof(uint8_t) * event_length);
	event->event_len = event_length;
	memcpy(event->event, ev, event_length);
}
//}}}

//{{{ free_midi_event
void free_midi_event(struct MidiEvent* event){
	free(event);
}
//}}}

//{{{ parse_midi_event
struct MidiEvent* parse_midi_event(const uint8_t* event, size_t* size_read){
	size_t delta_time_size;
	uint32_t delta_time = varlen_to_int(event, &delta_time_size);
	const uint8_t* event_code = event + delta_time_size;
	uint8_t event_type = event_code[0];

	size_t event_size;
	if((event_type & 0xF0) < 0x80){
		//this should be an error...
	} else if((event_type & 0xF0) < 0xF0){
		//this is a midi voice or mode message
		event_size = parse_midi_voice_event(event_code);
	} else {
		if((event_type == 0xF0) || event_type == 0xF7){
			//this is a sysex message (<type> <len> <data>)
			event_size = parse_midi_sysex_event(event_code);
		} else {
			//this is a meta message
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
//}}}

//{{{ parse_midi_voice_event
size_t parse_midi_voice_event(const uint8_t* event_code){
	uint8_t status = event_code[0] & 0xF0;
	size_t read;
	switch(status){
		case(0x80):
		case(0x90):
		case(0xA0):
		case(0xB0):
		case(0xE0):
			read = 3;
			break;
		case(0xC0):
		case(0xD0):
			read = 2;
			break;
	}
	return read;
}
//}}}

//{{{ parse_midi_sysex_event
size_t parse_midi_sysex_event(const uint8_t* event_code){
	size_t len_size;
	uint32_t len = varlen_to_int(event_code + 1, &len_size);
	return len_size + len + 1;
}
//}}}

//{{{ parse_midi_meta_event
size_t parse_midi_meta_event(const uint8_t* event_code){
	uint8_t type = event_code[1];
	size_t read;
	switch(type){
		//sequence number
		case(0x00):
			//FF 00 02 SS SS
			read = 5;
			break;
		//text event
		case(0x01):
		//copyright notice
		case(0x02):
		//sequence/track name
		case(0x03):
		//instrument name
		case(0x04):
		//lyric
		case(0x05):
		//marker
		case(0x06):
		//cue point
		case(0x07):{
			//FF <nn> <len> <text>
			const uint8_t* len_start = event_code + 2;
			size_t len_size;
			uint32_t len = varlen_to_int(len_start, &len_size);
			read = 2 + len_size + len;
			break;
		}
		//MIDI Channel Prefix
		case(0x20):
			//FF 20 01 CC
			read = 4;
			break;
		//End of Track
		case(0x2F):
			//FF 2F 00
			read = 3;
			break;
		//Set Tempo
		case(0x51):
			//FF 51 03 tt tt tt
			read = 6;
			break;
		//SMTPE Offset
		case(0x54):
			//FF 54 05 hh mm ss fr ff
			read = 8;
			break;
		//Time Signature
		case(0x58):
			//FF 58 04 nn dd cc bb
			read = 7;
			break;
		//Key Signature
		case(0x59):
			//FF 59 02 sf mi
			read = 5;
		//Sequencer-Specific Meta-event
		case(0x7F):{
			//FF 7F <len> <id> <data>
			const uint8_t* len_start = event_code+2;
			size_t len_size;
			uint32_t len = varlen_to_int(len_start, &len_size);
			read = 2 + len_size + len;
		}
	}
	return read;
}
//}}}

//{{{ new_midi_track
void new_midi_track(struct MidiTrackChunk* track){
	track->event_count = 0;

	track->events = malloc(sizeof(struct MidiEvent*) * track ->event_count);
}
//}}}

//{{{ free_midi_track
void free_midi_track(struct MidiTrackChunk* track){
	for(size_t i = 0; i < track->event_count; ++i){
		free_midi_event(track->events[i]);
	}
	free(track->events);
}
//}}}

//{{{ track_length
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
//}}}

//{{{ track_add_event
struct MidiEvent* track_add_event(struct MidiTrackChunk* track){
	track->event_count++;
	track->events = realloc(track->events, sizeof(struct MidiEvent*) * track->event_count);

	struct MidiEvent* event = malloc(sizeof(struct MidiEvent));
	track->events[track->event_count - 1] = event;
	return event;
}
//}}}

//{{{ track_add_event_full
struct MidiEvent* track_add_event_full(struct MidiTrackChunk* track, uint32_t delta_time, const uint8_t* event_data, size_t event_data_len){
	struct MidiEvent* event = track_add_event(track);
	new_midi_event(event, delta_time, event_data, event_data_len);
	return event;
}
//}}}

//{{{ track_add_event_existing
void track_add_event_existing(struct MidiTrackChunk* track, struct MidiEvent* event){
	track->event_count++;
	track->events = realloc(track->events, sizeof(struct MidiEvent*) * track->event_count);

	track->events[track->event_count - 1] = event;
}
//}}}

//{{{ write_uint16_t
void write_uint16_t(uint16_t data, FILE* f){
	uint16_t d = htons(data);
	fwrite(&d, sizeof(uint16_t), 1, f);
}
//}}}

//{{{ write_uint32_t
void write_uint32_t(uint32_t data, FILE* f){
	uint32_t d = htonl(data);
	fwrite(&d, sizeof(uint32_t), 1, f);
}
//}}}

//{{{ read_uint16_t
uint16_t read_uint16_t(FILE* f){
	uint16_t d;
	fread(&d, sizeof(uint16_t), 1, f);
	return ntohs(d);
}
//}}}

//{{{ read_uint32_t
uint32_t read_uint32_t(FILE* f){
	uint32_t d;
	fread(&d, sizeof(uint32_t), 1, f);
	return ntohl(d);
}
//}}}

//{{{ write_midi
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
//}}}

//{{{ read_midi
struct Midi* read_midi(FILE* f){	
	struct Midi* midi = malloc(sizeof(struct Midi));
	new_midi(midi);

	uint8_t chunk_head[TYPE_LEN];
	fread(chunk_head, sizeof(uint8_t), TYPE_LEN, f);
	uint32_t size_head = read_uint32_t(f);
	if(size_head != HEADER_LEN){
		//throw an error
		printf("wrong header size\n");
	}
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
				//read too much somehow
				printf("read too much somehow [sizeleft = %zu]\n", read);
				break;
			}
		}
		free(event);
	}
	return midi;
}
//}}}

