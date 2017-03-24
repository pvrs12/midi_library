#ifndef MIDI_H
#define MIDI_H 

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define TYPE_LEN 4
#define HEADER_LEN 6
#define MAX_EVENT_LEN 50

/*
 * Represents the type of chunk internally.
 */
enum ChunkType {
	CHUNK_HEADER, 
	CHUNK_TRACK
};

/*
 * Converts a variable-length quantity to an integer
 */
uint32_t varlen_to_int(const uint8_t* var, size_t* size);

/*
 * Converts an integer to a variable-length quantity
 */
uint8_t* int_to_varlen(uint32_t val, size_t* size);

/*
 * Represents a chunk of a MIDI file. 
 * There are 2 types of chunks: Headers and Tracks. 
 *
 * This should never be used directly
 */
struct MidiChunk {
	uint8_t type[TYPE_LEN];
	enum ChunkType type_e;

	void* chunk;
};

/*
 * Construct a MidiChunk with the provided ChunkType
 *
 * This should never be used directly
 */
void new_midichunk(struct MidiChunk* chunk, enum ChunkType type);

/*
 * Calls the relevant free method for MidiChunk->chunk based on what type of chunk is being stored.
 *
 * This should never be used directly
 */
void free_midichunk(struct MidiChunk* chunk);

/*
 * The top level container of a MIDI file. This should be allocated and free'd by the caller
 */
struct Midi {
	uint32_t chunk_count;
	struct MidiChunk** chunks;

	struct MidiHeaderChunk* header;
};

/*
 * Construct a MIDI. 
 *
 * This is the main container for the data
 */
void new_midi(struct Midi* midi);

/*
 * Free all of the children of this Midi file. 
 *
 * This does not free the Midi iteslf, that is still up to the caller.
 */
void free_midi(struct Midi* midi);

/* 
 * Allocates and creates a new chunk for in the Midi. 
 *
 * This should not be called directly
 */
struct MidiChunk* midi_add_chunk(struct Midi* midi);
/*
 * Allocates and creates a new header chunk for the Midi. 
 *
 * This will be freed with its parent `Midi`
 */
struct MidiHeaderChunk* midi_add_header(struct Midi* midi, uint16_t format, uint16_t tracks, uint16_t division);
/* 
 * Allocates and creates a new track chunk for the Midi. 
 *
 * This will be freed with its parent Midi
 */
struct MidiTrackChunk* midi_add_track(struct Midi* midi);

/*
 * Represents the header chunk of a Midi file
 *
 * This is often handled entirely by `midi_add_header`
 */
struct MidiHeaderChunk {
	uint32_t length;

	uint16_t format;
	uint16_t tracks;
	uint16_t division;
};

/*
 * Create a fill a MidiHeader with the given details
 */
void new_midi_header(struct MidiHeaderChunk* header, uint32_t length, uint16_t format, uint16_t tracks, uint16_t division);
/*
 * Free the MidiHeader. 
 *
 * This will be called on your behalf. It also currently does nothing
 */
void free_midi_header(struct MidiHeaderChunk* header);

/*
 * This represents an event within a track. 
 *
 * This is usually created with a helper method.
 */
struct MidiEvent {
	uint32_t delta_time;

	size_t event_len;
	uint8_t* event;
};

/*
 * This populates an event with the given details. 
 *
 * A copy of ev is made, so it can be deallocated at any time
 *
 * This is usually called for you
 */
void new_midi_event(struct MidiEvent* event, uint32_t delta_time, const uint8_t* ev, size_t event_length);
/*
 * This frees the `MidiEvent`
 *
 * This is usually called on your behalf (i.e. if the event is part of a track);
 */
void free_midi_event(struct MidiEvent* event);

/*
 * Used to pull a `MidiEvent` from a buffer which may contain multiple `MidiEvent`s.
 *
 * It will allocate a new MidiEvent, and determine how much data was used from the buffer.
 * This is called by `read_midi`
 */
struct MidiEvent* parse_midi_event(const uint8_t* event, size_t* size_read);
/*
 * Used by parse_midi_event. 
 *
 * This should not be called manually
 */
size_t parse_midi_voice_event(const uint8_t* event_code);
size_t parse_midi_sysex_event(const uint8_t* event_code);
size_t parse_midi_meta_event(const uint8_t* event_code);

/*
 * This represents a track containing a series of `MidiEvent`s. 
 *
 * This is usually allocated by `midi_add_track` and freed with the Midi which contains it
 */
struct MidiTrackChunk {
	size_t event_count;

	struct MidiEvent** events;
};

/*
 * Construct the track
 *
 * This is usually called for you
 */
void new_midi_track(struct MidiTrackChunk* track);
/*
 * Free the `MidiTrackChunk`
 *
 * This is usually called on your behalf (i.e. if the track is a part of a Midi)
 */
void free_midi_track(struct MidiTrackChunk* track);

/*
 * This creates a new empty event within the given track. This event can then be populated with details. 
 *
 * It will be freed automatically with the track.
 */
struct MidiEvent* track_add_event(struct MidiTrackChunk* track);
/*
 * This creates a new event within the given track. This event is created with the given details.
 *
 * It will be freed automatically with the track.
 */
struct MidiEvent* track_add_event_full(struct MidiTrackChunk* track, uint32_t delta_time, const uint8_t* event_data, size_t event_data_len);
/*
 * This adds an existing `MidiEvent` to the given track.
 *
 * It will be freed automatically with the track.
 */
void track_add_event_existing(struct MidiTrackChunk* track, struct MidiEvent* event);
/*
 * This calculates the total size of the track. It is used within `write_midi`
 */
size_t track_length(struct MidiTrackChunk* track);

/*
 * Used to write uint16_t and uin32_t in big-endian format. 
 */
void write_uint16_t(uint16_t data, FILE* f);
void write_uint32_t(uint32_t data, FILE* f);

/*
 * Writes the Midi to the given opened `FILE`. 
 */
void write_midi(struct Midi* m, FILE* f);

/*
 * Used to read uint16_t and uint32_t from big-endian format
 */
uint16_t read_uint16_t(FILE* f);
uint32_t read_uint32_t(FILE* f);

/*
 * Reads a `FILE` in from Midi format and returns a `Midi` containing it
 */
struct Midi* read_midi(FILE* f);

//www.personal.kent.edu/~sbirch/Music_Production/MP-II/MIDI/midi_file_format.htm

#endif /* MIDI_H */
