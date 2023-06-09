#include "remote_decoder.h"

unsigned char decode_remote(uint64_t code){
    unsigned char key = 0;
    // zero
    if(code == 0x400000){
        key |= ZERO;
    } else if (code == 0x100000){
        key |= ZERO;
        key |= SECOND_PRESS;
    } 
    // one
    else if (code == 0x100002){
        key |= ONE;
    } else if (code == 0x400002){
        key |= ONE;
        key |= SECOND_PRESS;
    }
    // two
    else if (code == 0x100003){
        key |= TWO;
    } else if (code == 0x40003){
        key |= TWO;
        key |= SECOND_PRESS;
    }
    // three
    else if (code == 0x100008){
        key |= THREE;
    } else if (code == 0x400008){
        key |= THREE;
        key |= SECOND_PRESS;
    }
    // four
    else if (code == 0x10000c){
        key |= FOUR;
    } else if (code == 0x4000c){
        key |= FOUR;
        key |= SECOND_PRESS;
    }
    // five
    else if (code == 0x4000e){
        key |= FIVE;
    } else if (code == 0x10000e){
        key |= FIVE;
        key |= SECOND_PRESS;
    }
    // six
    else if (code == 0x100009){
        key |= SIX;
    } else if (code == 0x40009){
        key |= SIX;
        key |= SECOND_PRESS;
    }
    // seven
    else if (code == 0x100020){
        key |= SEVEN;
    } else if (code == 0x400020){
        key |= SEVEN;
        key |= SECOND_PRESS;
    }
    // eight
    else if (code == 0x100030){
        key |= EIGHT;
    } else if (code == 0x40030){
        key |= EIGHT;
        key |= SECOND_PRESS;
    }
    // nine
    else if (code == 0x40032){
        key |= NINE;
    } else if (code == 0x100032){
        key |= NINE;
        key |= SECOND_PRESS;
    }
    // delete
    else if (code == 0x100c3){
        key |= DELETE;
    } else if (code == 0x400c3){
        key |= DELETE;
        key |= SECOND_PRESS;
    }
    // enter
    else if (code == 0x4000e0){
        key |= ENTER;
    } else if (code == 0x1c00e0){
        key |= ENTER;
        key |= SECOND_PRESS;
    }
    // else
    else {
        key = ERROR_KEY;
    }
    return key;
}



char get_char_from_key_code(unsigned char key_code){
    char c = 0;
    // second press is stored as 0x80, so we need to remove it
    key_code &= 0x7f;
    switch(key_code){
        case ZERO:
            c = '0';
            break;
        case ONE:
            c = '1';
            break;
        case TWO:
            c = '2';
            break;
        case THREE:
            c = '3';
            break;
        case FOUR:
            c = '4';
            break;
        case FIVE:
            c = '5';
            break;
        case SIX:
            c = '6';
            break;
        case SEVEN:
            c = '7';
            break;
        case EIGHT:
            c = '8';
            break;
        case NINE:
            c = '9';
            break;
        case DELETE:
            c = 'd';
            break;
        case ENTER:
            c = 'e';
            break;
        default:
            c = 0;
            break;
    }
    return c;
}
