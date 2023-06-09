#include "Typer.h"
#include "OLED_Utils.h"
#include "key_code.h"


static char messageString[MAX_STRING_LENGTH];
static unsigned int messageColors[MAX_STRING_LENGTH]; // the rgb hex code for each character
unsigned int messageIndex = 0;

static key_code last_key = ERROR_KEY; 
static char last_char = '\0';
static uint64_t last_code = 0; // used for double press detection and also multi send detection
static unsigned int color = WHITE; // used for color changing

static unsigned char timeout = 0; // used to allow you to type two of the same character in a row after a delay

static unsigned char message_complete = 0;


void typer_init(){
    int i = 0;
    for(i = 0; i < MAX_STRING_LENGTH; i++){
        messageString[i] = '\0';
        messageColors[i] = WHITE;
    }
    messageIndex = 0;
    last_key = ERROR_KEY;
    last_char = '\0';
    last_code = 0;
    color = WHITE;
    timeout = 0;
    message_complete = 0;
}



// this function should take a key_code and last_char and return the next char in the sequence
// if the key_code is the same as the last key_code, it should return the next char in the sequence
// if the key_code is different, it should return the first char in the sequence
// if the key_code is 0, it should return a space
void type(key_code key, char* output){
    switch(key){
        case ZERO:
            *output = ' ';
            break;
        case ONE:
            return;
        case TWO:
            switch(last_char){
                case 'a':
                    *output = 'b';
                    break;
                case 'b':
                    *output = 'c';
                    break;
                case 'c':
                    *output = 'a';
                    break;
                default:
                    *output = 'a';
                    break;
            }
            break;
        case THREE:
            switch(last_char){
                case 'd':
                    *output = 'e';
                    break;
                case 'e':
                    *output = 'f';
                    break;
                case 'f':
                    *output = 'd';
                    break;
                default:
                    *output = 'd';
                    break;
            }
            break;
        case FOUR:
            switch(last_char){
                case 'g':
                    *output = 'h';
                    break;
                case 'h':
                    *output = 'i';
                    break;
                case 'i':
                    *output = 'g';
                    break;
                default:
                    *output = 'g';
                    break;
            }
            break;
        case FIVE:
            switch(last_char){
                case 'j':
                    *output = 'k';
                    break;
                case 'k':
                    *output = 'l';
                    break;
                case 'l':
                    *output = 'j';
                    break;
                default:
                    *output = 'j';
                    break;
            }
            break;
        case SIX:
            switch(last_char){
                case 'm':
                    *output = 'n';
                    break;
                case 'n':
                    *output = 'o';
                    break;
                case 'o':
                    *output = 'm';
                    break;
                default:
                    *output = 'm';
                    break;
            }
            break;  
        case SEVEN:
            switch(last_char){
                case 'p':
                    *output = 'q';
                    break;
                case 'q':
                    *output = 'r';
                    break;
                case 'r':
                    *output = 's';
                    break;
                case 's':
                    *output = 'p';
                    break;
                default:
                    *output = 'p';
                    break;
            }
            break;
        case EIGHT:
            switch(last_char){
                case 't':
                    *output = 'u';
                    break;
                case 'u':
                    *output = 'v';
                    break;
                case 'v':
                    *output = 't';
                    break;
                default:
                    *output = 't';
                    break;
            }
            break;
        case NINE:
            switch(last_char){
                case 'w':
                    *output = 'x';
                    break;
                case 'x':
                    *output = 'y';
                    break;
                case 'y':
                    *output = 'z';
                    break;
                case 'z':
                    *output = 'w';
                    break;
                default:
                    *output = 'w';
                    break;
            }
            break;
        case ENTER:
            return;
    }
    last_key = key;
    last_char = *output;
}

void next_color(){
    switch(color){
        case WHITE:
            color = BLUE;
            break;
        case BLUE:
            color = GREEN;
            break;
        case GREEN:
            color = CYAN;
            break;
        case CYAN:
            color = RED;
            break;
        case RED:
            color = MAGENTA;
            break;
        case MAGENTA:
            color = YELLOW;
            break;
        case YELLOW:
            color = WHITE;
            break;
        default:
            color = WHITE;
            break;
    }
}



void process_input(uint64_t code){
    if(code == last_code){
        return;
    }
    last_code = code;
    unsigned char key_code = decode_remote(code);
    if(key_code == ERROR_KEY){
        return;
    }
    unsigned char key_code_without_modifier = key_code & 0x7F;
    unsigned char last_key_code_without_modifier = last_key & 0x7F;

    if(messageIndex >= MAX_STRING_LENGTH){
        // TODO: error handling
        messageIndex = 0;
    }

    if(key_code_without_modifier == ENTER){
        if(messageIndex < MAX_STRING_LENGTH - 1){
            messageIndex++;
        }
        messageString[messageIndex] = '\0';
        message_complete = 1;
        last_key = ERROR_KEY;
        last_char = '\0';
    }else if(key_code_without_modifier == ONE){
        next_color();
        messageColors[messageIndex] = color;
    } else if (key_code_without_modifier == DELETE){
        if(messageIndex > 0){
            if(messageString[messageIndex] != '\0'){
                messageString[messageIndex] = '\0';
            } else {
                messageIndex--;
                messageString[messageIndex] = '\0';
            }
        } else {
            messageString[messageIndex] = '\0';
        }
        last_key = ERROR_KEY;
        last_char = '\0';
    } else {
        // if you type a new key automatically advance to the next char, otherwise stay on the same char pool
        // unless it's been a while since you typed the last key, then you can advance to the next char pool even if it's the same key
        if((key_code_without_modifier != last_key_code_without_modifier || timeout > 0) && last_key != ERROR_KEY){
            next_char();
        }
        char next_char;
        type(key_code_without_modifier, &next_char);
        messageString[messageIndex] = next_char;
    }
}


const char * get_current_string(){
    return messageString;
}

uint8_t is_message_complete(){
    return message_complete;
}

void reset_message(){
    message_complete = 0;
    messageIndex = 0;
    messageString[0] = '\0';
    messageColors[0] = color;
    int i = 0;
    for(i = 1; i < MAX_STRING_LENGTH; i++){
        messageString[i] = '\0';
        messageColors[i] = WHITE;
    }
}

unsigned int get_current_string_length(){
    return messageIndex;
}


unsigned int* get_current_string_color(){
    return messageColors;
}

// the packed string is in a format such that for each character, the first 8 bits are the char, and the last 24 bits are the color
unsigned int* get_current_packed_string(){
    unsigned int* packed_string = malloc(sizeof(unsigned int) * MAX_STRING_LENGTH);
    int i = 0;
    for(i = 0; i < MAX_STRING_LENGTH; i++){
        packed_string[i] = 0;
    }
    for(i = 0; i < messageIndex; i++){
        packed_string[i] = ((messageColors[i] << 8) & 0xFFFFFF00) | (messageString[i] & 0xFF);
    }
    return packed_string;
}

void next_char(){
    messageIndex++;
    messageString[messageIndex] = '\0';
    messageColors[messageIndex] = color;
    last_key = 0;
    last_code = 0;
    last_char = '\0';
}


void set_timeout(unsigned char timeout_value){
    timeout = timeout_value;
}

unsigned int get_current_color(){
    return color;
}

unsigned char get_timeout(){
    return timeout;
}