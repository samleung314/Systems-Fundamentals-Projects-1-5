// DO NOT MODIFY THE CONTENTS OF THIS FILE IT WILL BE REPLACED DURING GRADING
#include "const.h"

/* This variable will be in this format/name, but the string it is initalized to
 * will be at most 31 characters. Any characters which do not appear in the alphabet
 * string should be preserved.
**/
char *polybius_alphabet = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

// The Key for the Fractionated Morse encoding
const char *fm_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

const char *morse_table[] = {"-.-.--", ".-..-.", "", "", "", "", ".----.", "-.--.", "-.--.-", "", "", "--..--", "-....-", ".-.-.-", "", "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.", "---...", "-.-.-.", "", "-...-", "", "..--..", ".--.-.", ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};
// Fractionated Morse Code: this is always the same.
// Fractionated works: key[0] = fractionated[0] after the morse translation
const char *fractionated_table[26] = {
    "...", "..-", "..x", ".-.", ".--", ".-x", ".x.", ".x-", ".xx", "-..",
    "-.-", "-.x", "--.", "---", "--x", "-x.", "-x-", "-xx", "x..", "x.-", "x.x",
    "x-.", "x--", "x-x", "xx.", "xx-"
};

// The value of the -k program argument goes here
const char *key;

// A buffer for the letters in the polybius cipher table
char polybius_table[257] = {0};

// A buffer for the letters in the Fractionated Morse cipher table
char fm_key[27] = {0};
