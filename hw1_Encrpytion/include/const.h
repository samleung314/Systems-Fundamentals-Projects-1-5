/*
 * DO NOT MODIFY THE CONTENTS OF THIS FILE.
 * IT WILL BE REPLACED DURING GRADING
 */
#ifndef CONST_H
#define CONST_H

#include <stdio.h>

extern char *polybius_alphabet;
extern char polybius_table[257];

extern const char *morse_table[];
extern const char *fm_alphabet;
extern const char *fractionated_table[];

extern char fm_key[27];

extern const char *key;

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
unsigned short validargs(int argc, char **argv);

#define USAGE(program_name, retcode) do{                                                        \
fprintf(stderr, "USAGE: %s %s\n", program_name,                                                         \
"[-h] -p|-f -e|-d [-k KEY] [-r ROWS] [-c COLUMNS]\n" \
"    -p       Polybius Cipher\n"                                              \
"             Additional parameters: [-e | -d] [-i INPUT_FILE] [-o OUTPUT_FILE] [-k KEY] [-r ROWS] [-c COLUMNS]\n"        \
"                 -e           Encrypt using the Polybius cipher.\n"          \
"                 -d           Decrypt using the Polybius cipher.\n"          \
"                 -k           KEY is the key to be used in the cipher.\n"   \
"                              It must have no repeated characters and each character in the key\n" \
"                              must be in the alphabet in `const.c`.\n"         \
"                 -r           ROWS is the number of rows to be used in the Polybius cipher table.\n" \
"                              Must be between 10 and 16, inclusive. Defaults to 10\n"              \
"                 -c           COLUMNS is the number of columns to be used in the Polybius cipher table.\n" \
"                              Must be between 10 and 16, inclusive. Defaults to 10\n"              \
"\n"                                                                              \
"    -f       Fractionated Morse Cipher\n"                                              \
"             Additional parameters: [-e | -d] [-i INPUT_FILE] [-o OUTPUT_FILE] [-k KEY]\n"        \
"                 -e           Encrypt using the Fractionated Morse cipher.\n"          \
"                 -d           Decrypt using the Fractionated Morse cipher.\n"          \
"                 -k           KEY is the key to be used in the cipher.\n"   \
"                              It must have no repeated characters and each character in the key\n" \
"                              must be in the alphabet in `const.c`.\n"         \
"\n"                                                                              \
"    -h       Display this help menu.");                                          \
exit(retcode);                                                                    \
} while(0)

#endif
