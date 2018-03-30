#include "hw1.h"
#include <stdlib.h>

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the program
 * and will return a unsigned short (2 bytes) that will contain the
 * information necessary for the proper execution of the program.
 *
 * IF -p is given but no (-r) ROWS or (-c) COLUMNS are specified this function
 * MUST set the lower bits to the default value of 10. If one or the other
 * (rows/columns) is specified then you MUST keep that value rather than assigning the default.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return Refer to homework document for the return value of this function.
 */
unsigned short validargs(int argc, char **argv) {

    char *help = "-h", *poly = "-p", *fract = "-f",
    *encrypt = "-e", *decrypt = "-d", *null = "\0";

    char *k = "-k", *r = "-r", *c = "-c";
    int kdone=0, rdone=0, cdone=0;

    key = null; //Initiate key with null

    unsigned short mode = 0x0000;
    if(--argc) argv++;  //Advance past bin/hw1
    if(!argc) return 0; //Check for empty arguments

    //HELP
    if(charCompare(*argv, help)){ //-h
        return 0x8000;

    //POLYBUS
    }else if(charCompare(*argv, poly)){ //-p
        if(--argc) argv++; //Advance past -p
        if(!argc) return 0; //Check for empty arguments

        //Positional
        if(charCompare(*argv, decrypt)){ //-p -d
            mode = mode|0x2000; // turns bit 13 to 1
            if(--argc) argv++; //Advance past -d
        }else if(charCompare(*argv, encrypt)){ //-p -e
            if(--argc) argv++; //Advance past -e
        }else{
            return 0; //There's no -d or -e
        }

        //Default rows and columns to 10
        mode = mode|0x00AA;
        int rows = 10, cols = 10;

        //Optional
        while(argc){
            if(charCompare(*argv, k)){
                if(kdone) return 0;
                argc--; argv++; //Advance past -k

                if(!checkKey(polybius_alphabet, *argv)) return 0;
                key = *argv; //Store the key
                kdone=1;
            }else if(charCompare(*argv, r)){
                if(rdone) return 0;
                argc--; argv++; //Advance past -r
                int row = atoi(*argv);

                if(!checkInt(row)) return 0; //Return 0 if not between 9-15
                rows = row;
                mode = mode & 0xFF0F; //Clear ROW
                row <<= 4;
                mode += row;
                rdone=1;
            }else if(charCompare(*argv, c)){
                if(cdone) return 0;
                argc--; argv++; //Advance past -c
                int col = atoi(*argv);

                if(!checkInt(col)) return 0; //Return 0 if not between 9-15
                cols = col;
                mode = mode & 0xFFF0; //Clear COL
                mode += col;
                cdone=1;
            }else{
                return 0; //Not any of -k -r -c
            }
            if(--argc) argv++; //Advance past adjacent args
        }
        //Check (rows * cols) >= length of polybius_alphabet
        if(!checkLength(rows, cols)) return 0;

    //FRACTIONAL
    }else if(charCompare(*argv, fract)){ // -f;
        mode = mode|0x4000; // turns bit 14 to 1
        argv++; //Advance past -f
        argc--;
        if(!argc) return 0;

        //Positional
        if(charCompare(*argv, decrypt)){ // -f -d
            mode = mode|0x2000; // turns bit 13 to 1
            if(--argc) argv++; //Advance past -d
        }else if(charCompare(*argv, encrypt)){ //-f -e
            if(--argc) argv++; //Advance past -e
        }else{
            return 0; //There's no -d or -e
        }

        //Optional
        while(argc){
            if(charCompare(*argv, k)){
                if(kdone) return 0;
                argc--; argv++; //Advance past -k;
                if(!checkKeyConst(fm_alphabet, *argv)) return 0;
                key = *argv; //Store the key
                kdone=1;
            }else{
                return 0;
            }
            if(--argc) argv++; //Advance past adjacent args
        }
    }
    return mode;
}

int charCompare(char *a, char *b) {
    int match = 1;
    char *null = "\0";

    while(match){
        if(*a != *b){
            return 0;
        }

        //Check for end of string
        if(*a == *null){
            if(*b == *null){
                return 1; //Both strings end at once, so match
            }
            return 0; //First string ended early, mismatch
        }
        a++; //advance pointers on both strings
        b++;
    }

    return 1;
}

int checkInt(int check){
    if(check>=9 && check<=15){
        return 1;
    }
    return 0;
}

int checkLength(int rows, int cols){
    char *temp = polybius_alphabet;
    int length = 0;

    //Get the length
    while(*temp){
        temp++;
        length++;
    }

    if(rows*cols >= length) return 1;

    return 0;
}

int checkKey(char *alpha, char *keyArg){
    char *p1 = keyArg, *p2 = keyArg+1, *null = "\0", *letter = alpha;

    while(*p1){
        if(*p1 == *p2) return 0; //Repeating character

        letter = alpha; //reset letter
        while(*letter != *null){
            if(*p1 == *letter) break;
            letter++;
        }

        if(*letter == *null) return 0; //after searching alphabet, *p1 not found

        p1++; p2++;
    }
    return 1;
}

int checkKeyConst(const char *alpha, char *keyArg){
    char *p1 = keyArg, *p2 = keyArg+1, *null = "\0";
    const char *letter = alpha;

    while(*p1 != *null){
        if(*p1 == *p2) return 0; //Repeating character

        letter = alpha; //reset letter
        while(*letter != *null){
            if(*p1 == *letter) break;
            letter++;
        }

        if(*letter == *null) return 0; //after searching alphabet, *p1 not found

        p1++; p2++;
    }
    return 1;
}