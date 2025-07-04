// ----------- analisador.h -----------
#ifndef ANALISADOR_H
#define ANALISADOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SOURCE_SIZE 10000

typedef enum {
    TOKEN_INT, TOKEN_FLOAT, TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_CHAR,
    TOKEN_KEYWORD, TOKEN_OPERATOR, TOKEN_DELIMITER, TOKEN_EOF,
    TOKEN_COMMENT, TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[100];
    int line;
    int column;
} Token;

typedef struct {
    int line;
    int column;
} Position;

extern Position current_pos;

Token next_token(const char* src, int* index);
const char* token_type_to_string(TokenType type);

#endif // ANALISADOR_H