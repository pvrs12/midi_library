#ifndef MIDI_HELPER_H
#define MIDI_HELPER_H 

#include "midi.h"

/*
 * These methods aid in creating `MidiEvent`s. 
 *
 * They function as decorators by adding elements to the `EventString`
 */

/*
 * Used to help construct `MidiEvent`s
 */
struct EventString{
	uint8_t* event_string;
	size_t event_string_len;
};

/*
 * Constructs a new `EventString` 
 *
 * This must be called before any of the other functions are used to decorate the `EventString`. 
 */
struct EventString* new_event_string(struct EventString* event);
/*
 * Deallocates the `EventString`
 *
 * This must be called before `free`ing the `EventString`
 */
void free_event_string(struct EventString* event);
/*
 * Adds the given buffer to the `EventString`.
 *
 * This should not be called directly
 */
struct EventString* add_to_event(struct EventString* event, const uint8_t* s, size_t size);

/*
 * Decorates the `EventString` with a Voice event. 
 *
 * The low 4 bits of status are discarded and the high 4 of channel are discarded
 *		e.g.	status &= 0xF0;
 *					channel &= 0x0F;
 */
struct EventString* add_voice_message(struct EventString* event, uint8_t status, uint8_t channel);
/*
 * Decorates the `EventString` with a Sysex event.
 *
 * This simply adds `type` to the `EventString`. In order to complete the Sysex event a buffer must follow.
 * `type` must be 0xF0 or 0xF7
 */
struct EventString* add_sysex_message(struct EventString* event, uint8_t type);
/*
 * Decorates the `EventString` with a meta event
 *
 * This simply adds 0xFF `subtype` to the `EventString`. In order to complete the Meta event a buffer must follow.
 * `subtype` must be one of the subtypes defined in `midi_constants.h` with the prefix META_
 */
struct EventString* add_meta_message(struct EventString* event, uint8_t subtype);

/*
 * Decorates the `EventString` with the given string.
 *
 * This forwards to `add_buffer`
 */
struct EventString* add_string(struct EventString* event, char* str, size_t s);
/*
 * Decorates the `EventString` with the given buffer.
 *
 * In order to be in valid MIDI format, the size of the string must be stored as a variable length quantity. 
 * This appends the variable length quantity and the string to the `EventString`
 */
struct EventString* add_buffer(struct EventString* event, uint8_t* str, size_t s);
/*
 * Decorates the `EventString` with a single byte
 *
 * Simply appends the given byte onto the `EventString`
 */
struct EventString* add_byte(struct EventString* event, uint8_t b);


#endif /* MIDI_HELPER_H */
