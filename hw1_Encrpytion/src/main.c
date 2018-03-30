#include <stdlib.h>

#include "hw1.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    unsigned short mode;

    mode = validargs(argc, argv);

    debug("Mode: 0x%X", mode);

    if(!mode){          // Return 0
        USAGE(*argv, EXIT_FAILURE);
        return EXIT_FAILURE;
    }

    if(mode & 0x8000) { //Bitwise AND to compare mode and 0x8000
        USAGE(*argv, EXIT_SUCCESS);
    }

    if(mode & 0x4000){  //Fract Morse
        if(!morse(mode)) USAGE(*argv, EXIT_FAILURE);
    }else{              //Polybius
        if(!polybius(mode)) USAGE(*argv, EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */