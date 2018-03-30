#include "hw1.h"
#include <stdlib.h>

int morse(unsigned short mode){
    char buffer, *newline="\n", *space=" ", *tab="\t";

    int isDecrypt = 0x2000 & mode;

    int morseCount = 0;

    //Setup table
    setupMorseTable(key);

    if(isDecrypt){

    }else{
        buffer = getchar();
        while(buffer != EOF){

            if(buffer == *space){
                printf(" ");
            }else if(buffer == *newline){
                printf("\n");
            }else if(buffer == *tab){
                printf("\t");
            }else{
                if(!checkMorse(buffer)) return 0; //Not in morse_table
                char *newline="\n", *space=" ", *tab="\t";

                char const *toEncrypt = checkMorse(buffer);

                while(*toEncrypt){

                    if(*toEncrypt == *space){

                    }else if(*toEncrypt == *newline){

                    }else if(*toEncrypt == *tab){

                    }

                    //Full buffer, now print encrypt
                    if(morseCount == 3){
                        printf("%c",printEncrypt());
                        morseCount = 0;
                    }

                    //Fill the buffer
                    if(morseCount<3){
                        *(polybius_table + morseCount) = *toEncrypt;
                        morseCount+=1;
                    }

                    toEncrypt++;
                }

            }

            buffer = getchar();
        }
    }

    return 1;
}

const char *checkMorse(char buffer){
    char *newline="\n", *space=" ", *tab="\t";

    if(buffer == *space){
        return space;
    }else if(buffer == *newline){
        return newline;
    }else if(buffer == *tab){
        return tab;
    }

    int position = buffer - 33; // '!' is index 0
    const char *morsePoint = *(morse_table + position);

    if(*morsePoint){
        return morsePoint;
    }
    return 0;
}

int encryptMorse(const char morse){
    char *tablePoint = polybius_table;

    return 0;
}

void setupMorseTable(const char *key){
    const char *alphaPointer = fm_alphabet, *keyPointer = key;
    char *tablePointer = fm_key;

    if(*key){
        //First, add key to table
        while(*keyPointer){
            *tablePointer = *keyPointer;
            keyPointer++;
            tablePointer++;
        }

        //Then, add reset of alphabet
        while(*alphaPointer){

            //Skip if letter is in key
            keyPointer = key; //reset key pointer
            int found = 0;
            while(*keyPointer){
                if(*alphaPointer == *keyPointer){
                    alphaPointer++;   //skip letter
                    found = 1;
                    break;
                }
                keyPointer++;
            }

            if(found) continue;
            *tablePointer = *alphaPointer;
            alphaPointer++;
            tablePointer++;
        }

    }else{
        for(int i=0; i<27; i++){
            if(*alphaPointer){
                *tablePointer = *alphaPointer;
                alphaPointer++;
                tablePointer++;
            }else{
                *tablePointer = 0;
                tablePointer++;
            }
        }
    }
    // tablePointer = fm_key;
    // for(int i=0; i<27; i++){
    //     printf("%d %c\n", i, *tablePointer);
    //     tablePointer++;
    // }
}

char printEncrypt(){
    int position = 0, found = 0;
    char *encrypt = "xyz", *fmPoint = fm_key;


    printf("Encrypt1: %c", *(encrypt + 0));
    printf("Encrypt2: %c", *(encrypt + 1));
    printf("Encrypt3: %c", *(encrypt + 2));

    // *(encrypt + 0) = *(polybius_table + 0);
    // *(encrypt + 1) = *(polybius_table + 1);
    // *(encrypt + 2) = *(polybius_table + 2);

    // while(!found){
    //     const char *letter = *(fractionated_table + position);

    //     if((*letter + 0) == *(encrypt + 0)){
    //         if((*letter + 1) == *(encrypt + 1)){
    //             if((*letter + 2) == *(encrypt + 2)){
    //                 found = 1;
    //             }
    //         }
    //     }
    //     if(!found) position++;
    // }

    // return *(fm_key + position);

    return  33;
}