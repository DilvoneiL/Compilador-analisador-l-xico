#include "analisador.h"

int is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

int is_keyword(const char* str) {
    for (int i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
        if (strcmp(str, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

Token next_token(const char* src, int* index) {
    Token token;
    int i = 0;

    memset(token.lexeme, 0, sizeof(token.lexeme)); // limpar lixo

    // Verifica fim do código ANTES de tudo
    if (src[*index] == '\0') {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    // Pular espaços
    while (isspace(src[*index])) (*index)++;

    // Verifica novamente fim após pular espaços
    if (src[*index] == '\0') {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    if (is_letter(src[*index])) {
        while (is_letter(src[*index]) || is_digit(src[*index])) {
            token.lexeme[i++] = src[(*index)++];
        }
        token.lexeme[i] = '\0';
        token.type = is_keyword(token.lexeme) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
    } else if (is_digit(src[*index])) {
        while (is_digit(src[*index])) {
            token.lexeme[i++] = src[(*index)++];
        }
        token.lexeme[i] = '\0';
        token.type = TOKEN_INT;
    } else if (strchr(operators, src[*index])) {
        char current = src[*index];
        char next = src[*index + 1];
    
        // Verificar operadores compostos
        if ((current == '=' && next == '=') ||
            (current == '!' && next == '=') ||
            (current == '<' && next == '=') ||
            (current == '>' && next == '=') ||
            (current == '+' && next == '+') ||
            (current == '-' && next == '-') ||
            (current == '&' && next == '&') ||
            (current == '|' && next == '|')) {
            
            token.lexeme[0] = current;
            token.lexeme[1] = next;
            token.lexeme[2] = '\0';
            *index += 2;
        } else {
            // operador simples
            token.lexeme[0] = current;
            token.lexeme[1] = '\0';
            (*index)++;
        }
    
        token.type = TOKEN_OPERATOR;
    } else if (strchr(delimiters, src[*index])) {
        token.lexeme[0] = src[(*index)++];
        token.lexeme[1] = '\0';
        token.type = TOKEN_DELIMITER;
    } else {
        token.lexeme[0] = src[(*index)++];
        token.lexeme[1] = '\0';
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
        case TOKEN_EOF: return "TOKEN_EOF";
        default: return "TOKEN_UNKNOWN";
    }
}



int main() {
    FILE* file = fopen("codigo.txt", "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    char source[MAX_SOURCE_SIZE];
    int length = fread(source, 1, MAX_SOURCE_SIZE - 1, file);
    source[length] = '\0'; // garante o final da string
    fclose(file);

    int index = 0;
    Token token;

    do {
        token = next_token(source, &index);
        printf("Token: %-10s | Tipo: %s\n", token.lexeme, token_type_to_string(token.type));
    } while (token.type != TOKEN_EOF);

    return 0;
}