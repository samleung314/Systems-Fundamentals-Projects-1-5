#include "hw1.h"
#include <stdlib.h>

int polybius(unsigned short mode){
    char buffer = getchar(), *temp, *newline="\n", *space=" ", *tab="\t";
    int isDecrypt = 0x2000 & mode; //0010 0000 0000 0000

    int rows = 0x00F0 & mode, cols = 0x000F & mode;
    rows >>= 4;

    int size = rows*cols;

    //Setup table
    cipher(key, size);

    if(isDecrypt){
        while(buffer != EOF){
            if(buffer == *space){
                printf(" ");
            }else if(buffer == *newline){
                printf("\n");
            }else if(buffer == *tab){
                printf("\t");
            }else{
                //Get x,y
                int x = convertHex(buffer);
                int y = convertHex(getchar());
                printf("%c", decrypt(x, y, rows, cols));
            }
            buffer = getchar();
        }
    }else{
        while(buffer != EOF){
            temp = polybius_table;
            int position = 1;
            while(*temp){
                if(*temp == buffer) break;
                temp++;
                position++;
            }

            if(buffer == *space){
                printf(" ");
            }else if(buffer == *newline){
                printf("\n");
            }else if(buffer == *tab){
                printf("\t");
            }else{
                if(!checkBuffer(buffer, size)) return 0;

            //printf("BUFFER: %c %d (%X,%X)\n", buffer, position, getRow(position, cols), getCol(position, cols)); //FOR DEBUGGING
                printf("%X%X", getRow(position, cols), getCol(position, cols));
            }
            buffer = getchar();
        }
    }

    return 1;
}

int getRow(int position, int cols){
    int r = position/cols;
    if(position % cols == 0 && position >= cols) return r - 1;
    return r;
}

int getCol(int position, int cols){
    int c = position % cols;

    if(c == 0) return cols - 1;
    return c - 1;
}

void cipher(const char *key, int size){
    const char *keyPoint = key;
    char *tablePoint = polybius_table, *alphaPoint = polybius_alphabet;

    if(*keyPoint){
        int i=0;

        //First, add key to table
        while(*keyPoint){
            *tablePoint = *keyPoint;
            keyPoint++;
            tablePoint++;
            i++;
        }

        //Add rest of alphabet
        while(i<size){
            if(*alphaPoint){

                keyPoint = key; //reset key pointer
                int found = 0;
                // Scan if letter in key
                while(*keyPoint){
                    if(*alphaPoint == *keyPoint){
                        alphaPoint++;   //skip letter
                        found = 1;
                        break;
                    }
                    keyPoint++;
                }

                if(found) continue;
                *tablePoint = *alphaPoint;
                alphaPoint++;

            // After adding alphabet, add nulls to table
            }else{
                *tablePoint = 0;
            }

            tablePoint++;
            i++;
        }

    }else{
        for(int i=0; i<size; i++){
            if(*alphaPoint){
                *tablePoint = *alphaPoint;
                alphaPoint++;
                tablePoint++;
            }else{
                *tablePoint = 0;
                tablePoint++;
            }
        }
    }

    // tablePoint = polybius_table;
    // for(int i=0; i<size; i++){
    //     printf("i: %d TABLE: %c\n",i, *tablePoint);
    //     tablePoint++;
    // }
}

int checkBuffer(char buffer, int size){
    char *tablePoint = polybius_table;

    for(int i=0; i<size; i++){
        if(buffer == *tablePoint) return 1;
        tablePoint++;
    }

    return 0;
}

int convertHex(char buffer){
    switch(buffer){
        case '0':
            return 0;
            break;

        case '1':
            return 1;
            break;

        case '2':
            return 2;
            break;

        case '3':
            return 3;
            break;

        case '4':
            return 4;
            break;

        case '5':
            return 5;
            break;

        case '6':
            return 6;
            break;

        case '7':
            return 7;
            break;

        case '8':
            return 8;
            break;

        case '9':
            return 9;
            break;

        case 'A':
            return 10;
            break;

        case 'B':
            return 11;
            break;

        case 'C':
            return 12;
            break;

        case 'D':
            return 13;
            break;

        case 'E':
            return 14;
            break;

        case 'F':
            return 15;
            break;
    }
    return 0;
}

char decrypt(int x, int y, int rows, int cols){
    int position = 0;

    position += (x * cols); //Add tens
    position += (y); //Add ones

    char *tablePointer = polybius_table;

    tablePointer += position;
    return *tablePointer;
}