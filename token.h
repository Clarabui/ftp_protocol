/**
 * @file    token.h
 * @authors  Tram, Navin
 * @details implements tokenise function
 */

#define MAX_NUM_TOKENS 4
#define tokenSeparators " \t\n"

int tokenise(char inputline[], char *token[]);
