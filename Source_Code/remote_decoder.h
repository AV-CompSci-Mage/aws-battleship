#ifndef __REMOTE_DECODER_H__
#define __REMOTE_DECODER_H__
#include <stdint.h>
#include "key_code.h"

extern unsigned char decode_remote(uint64_t code);
extern char get_char_from_key_code(unsigned char key_code);



#endif // __REMOTE_DECODER_H__
