#ifndef ANALISADOR_H
#define ANALISADOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_SOURCE_SIZE 10000

typedef enum {
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_CHAR,
    TOKEN_KEYWORD,
    TOKEN_OPERATOR,  
    TOKEN_DELIMITER,  
    TOKEN_EOF,
    TOKEN_COMMENT,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[100];
    int line;       // Nova: linha onde o token começa
    int column;     // Nova: coluna onde o token começa
} Token;

// Estrutura para rastrear posição (nova)
typedef struct {
    int line;
    int column;
} Position;

// Variável global para posição atual (nova)
extern Position current_pos;

// Protótipos atualizados
int is_letter(char c);
int is_digit(char c);
int is_keyword(const char* str);
Token handle_eof(Position start_pos);
Token handle_numbers(const char* src, int* index, Position start_pos);
Token handle_identifiers(const char* src, int* index, Position start_pos);
Token handle_operators(const char* src, int* index, Position start_pos);
Token handle_delimiters(const char* src, int* index, Position start_pos);
Token handle_unknown(const char* src, int* index, Position start_pos);
Token handle_line_comment(const char* src, int* index, Position start_pos);
Token handle_block_comment(const char* src, int* index, Position start_pos);
Token handle_string(const char* src, int* index, Position start_pos);
Token next_token(const char* src, int* index);
const char* token_type_to_string(TokenType type);
void update_position(char c); // Nova função

#endif