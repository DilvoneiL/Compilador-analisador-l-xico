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
    TOKEN_COMMENT,
    TOKEN_UNKNOWN
} TokenType;


typedef struct {
    TokenType type;
    char lexeme[100]; // conteúdo real do token
} Token;

int is_letter(char c);
int is_digit(char c);
int is_keyword(const char* str);
Token handle_eof();
Token handle_numbers(const char* src, int* index);
Token handle_identifiers(const char* src, int* index);
Token handle_operators(const char* src, int* index);
Token handle_delimiters(const char* src, int* index);
Token handle_unknown(const char* src, int* index);
Token handle_line_comment(const char* src, int* index);
Token handle_block_comment(const char* src, int* index);
Token next_token(const char* src, int* index);
const char* token_type_to_string(TokenType type);