#ifndef HW_H
#define HW_H

#include "const.h"

int charCompare(char *a, char *b);
int checkInt(int check);
int checkKey(char *alpha, char *keyArg);
int checkKeyConst(const char *alpha, char *keyArg);
int checkLength(int rows, int cols);
int polybius(unsigned short mode);
int getRow(int position, int cols);
int getCol(int position, int cols);
void cipher(const char *key, int size);
int checkBuffer(char buffer, int size);
int convertHex(char buffer);
char decrypt(int x, int y, int rows, int cols);
int morse(unsigned short mode);
void setupMorseTable(const char *key);
const char *checkMorse(char buffer);
char printEncrypt();

#endif

