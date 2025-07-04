#include "analisador.h"
#include <stdio.h>

// Definições da linguagem para o analisador léxico
const char* keywords[] = {"if", "else", "while", "for", "return", "int", "float", "char", "fun", "main", "void"};
const char* operators = "+-*/=%!<>&|";
const char* delimiters = "();{},";

// Definição da variável global de posição, inicializada para o início do arquivo.
Position current_pos = {1, 1};

// --- Funções Auxiliares Internas ---

static void update_position(char c) {
    if (c == '\n') {
        current_pos.line++;
        current_pos.column = 1;
    } else {
        current_pos.column++;
    }
}

static int is_letter(char c) {
    return isalpha(c) || c == '_';
}

static int is_digit(char c) {
    return isdigit(c);
}

static int is_keyword(const char* str) {
    for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(str, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

// --- Função Principal do Módulo ---

Token next_token(const char* src, int* index) {
    // Pular espaços em branco e comentários
    while (1) {
        // Pula espaços em branco
        while (isspace(src[*index])) {
            update_position(src[*index]);
            (*index)++;
        }

        // Verifica e pula comentários
        if (src[*index] == '/' && src[*index + 1] == '/') {
            while (src[*index] != '\n' && src[*index] != '\0') {
                update_position(src[*index]);
                (*index)++;
            }
            continue; // Volta ao início para pular mais espaços/comentários
        }
        if (src[*index] == '/' && src[*index + 1] == '*') {
            (*index) += 2;
            update_position('/'); update_position('*');
            while (!(src[*index] == '*' && src[*index + 1] == '/') && src[*index] != '\0') {
                update_position(src[*index]);
                (*index)++;
            }
            if (src[*index] != '\0') {
                (*index) += 2;
                update_position('*'); update_position('/');
            }
            continue; // Volta ao início
        }
        break; // Sai do loop se não for espaço em branco nem comentário
    }

    Position start_pos = current_pos;
    Token token = {.line = start_pos.line, .column = start_pos.column};
    
    if (src[*index] == '\0') {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    char lexeme_buffer[100] = {0};
    int i = 0;

    if (is_letter(src[*index])) {
        while (is_letter(src[*index]) || is_digit(src[*index])) {
            lexeme_buffer[i++] = src[(*index)++];
            update_position(lexeme_buffer[i - 1]);
        }
        strcpy(token.lexeme, lexeme_buffer);
        token.type = is_keyword(token.lexeme) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
    } else if (is_digit(src[*index])) {
        int has_dot = 0;
        while (is_digit(src[*index]) || (src[*index] == '.' && !has_dot)) {
            if (src[*index] == '.') has_dot = 1;
            lexeme_buffer[i++] = src[(*index)++];
            update_position(lexeme_buffer[i - 1]);
        }
        strcpy(token.lexeme, lexeme_buffer);
        token.type = has_dot ? TOKEN_FLOAT : TOKEN_INT;
    } else if (strchr(operators, src[*index])) {
        lexeme_buffer[i++] = src[(*index)++];
        update_position(lexeme_buffer[i - 1]);
        if ((lexeme_buffer[0] == '=' && src[*index] == '=') ||
            (lexeme_buffer[0] == '!' && src[*index] == '=') ||
            (lexeme_buffer[0] == '<' && src[*index] == '=') ||
            (lexeme_buffer[0] == '>' && src[*index] == '=') ||
            (lexeme_buffer[0] == '&' && src[*index] == '&') ||
            (lexeme_buffer[0] == '|' && src[*index] == '|')) {
            lexeme_buffer[i++] = src[(*index)++];
            update_position(lexeme_buffer[i - 1]);
        }
        strcpy(token.lexeme, lexeme_buffer);
        token.type = TOKEN_OPERATOR;
    } else if (strchr(delimiters, src[*index])) {
        lexeme_buffer[i++] = src[(*index)++];
        update_position(lexeme_buffer[i - 1]);
        strcpy(token.lexeme, lexeme_buffer);
        token.type = TOKEN_DELIMITER;
    } else {
        lexeme_buffer[i++] = src[(*index)++];
        update_position(lexeme_buffer[i - 1]);
        strcpy(token.lexeme, lexeme_buffer);
        token.type = TOKEN_UNKNOWN;
    }

    return token;
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_INT: return "TOKEN_INT";
        case TOKEN_FLOAT: return "TOKEN_FLOAT";
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_KEYWORD: return "TOKEN_KEYWORD";
        case TOKEN_OPERATOR: return "TOKEN_OPERATOR";
        case TOKEN_DELIMITER: return "TOKEN_DELIMITER";
        case TOKEN_STRING: return "TOKEN_STRING";
        case TOKEN_CHAR: return "TOKEN_CHAR";
        case TOKEN_EOF: return "TOKEN_EOF";
        case TOKEN_COMMENT: return "TOKEN_COMMENT";
        default: return "TOKEN_UNKNOWN";
    }
}
