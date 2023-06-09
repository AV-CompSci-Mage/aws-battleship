#ifndef __TYPER_H__
#define __TYPER_H__

// This is a typing class for T9 input
// i.e. 2 = abc, 3 = def, etc.
// it should take a number and return the corresponding letter, or a space if the number is 0
// it needs to keep track of the current letter and return the next letter when the number is pressed again, or switch to a new category of letters
#include "remote_decoder.h"

#define MAX_STRING_LENGTH 256


extern void typer_init();
extern void type(key_code key, char* output);
extern void process_input(uint64_t code);

extern unsigned int get_current_string_length();
extern const char * get_current_string();
extern unsigned int* get_current_string_color();
extern unsigned int* get_current_packed_string();
extern unsigned int get_current_color();

extern uint8_t is_message_complete();
extern void reset_message();
extern void next_char();

extern void set_timeout(unsigned char timeout_value);
extern unsigned char get_timeout();

#endif // __TYPER_H__
