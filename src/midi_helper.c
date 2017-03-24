#include "include/midi_helper.h"

#include <string.h>

void new_event_string(struct EventString* event){
	event->event_type_len = 0;
	event->event_string = malloc(sizeof(char) * event->event_type_len);
}

void free_event_string(struct EventString* event){
	free(event->event_string);
}

struct EventString* add_to_event(struct EventString* event, const uint8_t* s, size_t size){
	event->event_type_len += size;
	event->event_string = realloc(event->event_string, sizeof(uint8_t) * event->event_type_len);
	strncat(event->event_string, s, size);
	return event;
}

struct EventString* add_voice_message(struct EventString* event, uint8_t status, uint8_t channel){
	uint8_t msg = (status & 0xF0) | (channel & 0x0F);
	event = add_to_event(event, &msg, 1);
	return event;
}

struct EventString* add_sysex_message(struct EventString* event, uint8_t type){
	event = add_to_event(event, &type, 1);
	return event;
}

struct EventString* add_meta_message(struct EventString* event, uint8_t subtype){
	uint8_t msg[] = {0xFF, subtype};
	event = add_to_event(event, msg, 2);
	return event;
}
