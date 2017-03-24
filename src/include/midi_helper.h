#ifndef MIDI_HELPER_H
#define MIDI_HELPER_H 

#include "midi.h"

struct EventString{
	uint8_t* event_string;
	size_t event_string_len;
};

struct EventString* new_event_string(struct EventString* event);
void free_event_string(struct EventString* event);
struct EventString* add_to_event(struct EventString* event, const uint8_t* s, size_t size);

///The low 4 of status and the high 4 of channel are discarded
struct EventString* add_voice_message(struct EventString* event, uint8_t status, uint8_t channel);
struct EventString* add_sysex_message(struct EventString* event, uint8_t type);
struct EventString* add_meta_message(struct EventString* event, uint8_t subtype);

struct EventString* add_string(struct EventString* event, char* str, size_t s);
struct EventString* add_buffer(struct EventString* event, uint8_t* str, size_t s);
struct EventString* add_byte(struct EventString* event, uint8_t b);


#endif /* MIDI_HELPER_H */
