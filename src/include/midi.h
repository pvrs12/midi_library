#ifndef MIDI_H
#define MIDI_H 

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define TYPE_LEN 4
#define HEADER_LEN 6
#define MAX_EVENT_LEN 50

enum ChunkType {
	CHUNK_HEADER, 
	CHUNK_TRACK
};

///Converts a variable-length quantity to an integer
uint32_t varlen_to_int(const uint8_t* var, size_t* size);

///Converts an integer to a variable-length quantity
uint8_t* int_to_varlen(uint32_t val, size_t* size);

struct MidiChunk {
	uint8_t type[TYPE_LEN];
	enum ChunkType type_e;

	void* chunk;
};

void new_midichunk(struct MidiChunk* chunk, enum ChunkType type);
void free_midichunk(struct MidiChunk* chunk);

struct Midi {
	uint32_t chunk_count;

	struct MidiChunk** chunks;
	struct MidiHeaderChunk* header;
};

void new_midi(struct Midi* midi);
void free_midi(struct Midi* midi);

struct MidiChunk* midi_add_chunk(struct Midi* midi);
struct MidiHeaderChunk* midi_add_header(struct Midi* midi, uint16_t format, uint16_t tracks, uint16_t division);
struct MidiTrackChunk* midi_add_track(struct Midi* midi);

struct MidiHeaderChunk {
	uint32_t length;

	uint16_t format;
	uint16_t tracks;
	uint16_t division;
};

void new_midi_header(struct MidiHeaderChunk* header, uint32_t length, uint16_t format, uint16_t tracks, uint16_t division);
void free_midi_header(struct MidiHeaderChunk* header);

struct MidiEvent {
	uint32_t delta_time;

	size_t event_len;
	uint8_t* event;
};

void new_midi_event(struct MidiEvent* event, uint32_t delta_time, const uint8_t* ev, size_t event_length);
void free_midi_event(struct MidiEvent* event);
struct MidiEvent* parse_midi_event(const uint8_t* event, size_t* size_read);
size_t parse_midi_voice_event(const uint8_t* event_code);
size_t parse_midi_sysex_event(const uint8_t* event_code);
size_t parse_midi_meta_event(const uint8_t* event_code);

struct MidiTrackChunk {
	size_t event_count;

	struct MidiEvent** events;
};

void new_midi_track(struct MidiTrackChunk* track);
void free_midi_track(struct MidiTrackChunk* track);

struct MidiEvent* track_add_event(struct MidiTrackChunk* track);
struct MidiEvent* track_add_event_full(struct MidiTrackChunk* track, uint32_t delta_time, const uint8_t* event_data, size_t event_data_len);
void track_add_event_existing(struct MidiTrackChunk* track, struct MidiEvent* event);
size_t track_length(struct MidiTrackChunk* track);

void write_uint16_t(uint16_t data, FILE* f);
void write_uint32_t(uint32_t data, FILE* f);
void write_midi(struct Midi* m, FILE* f);

uint16_t read_uint16_t(FILE* f);
uint32_t read_uint32_t(FILE* f);
struct Midi* read_midi(FILE* f);

//www.personal.kent.edu/~sbirch/Music_Production/MP-II/MIDI/midi_file_format.htm

#endif /* MIDI_H */
