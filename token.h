/*
 * file:    token.h
 * author:  Tram
 * aim:     implements tokenise function
 * date:    2020.07.03
 */

#define MAX_NUM_TOKENS 4
#define tokenSeparators " \t\n"

int tokenise(char inputline[], char *token[]);
