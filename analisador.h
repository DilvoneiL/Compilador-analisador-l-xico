#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_SOURCE_SIZE 10000


typedef enum {
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_OPERATOR,   // tipo novo
    TOKEN_DELIMITER,  // tipo novo
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;


typedef struct {
    TokenType type;
    char lexeme[100]; // conteúdo real do token
} Token;

const char* keywords[] = {"if", "else", "while", "return", "int", "float"};
const char* operators = "+-*/=%!<>&|";
const char* delimiters = "();{},";

