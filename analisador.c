#include "analisador.h"

const char* keywords[] = {"if", "else", "while", "return", "int", "float"};
const char* operators = "+-*/=%!<>&|";
const char* delimiters = "();{},";
const char* comment = "///**/";

int is_letter(char c) {
    return isalpha(c);
}

int is_digit(char c) {
    return isdigit(c);
}

int is_keyword(const char* str) {
    for (int i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
        if (strcmp(str, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

Token handle_eof() {
    Token token;
    token.type = TOKEN_EOF;
    strcpy(token.lexeme, "EOF");
    return token;
}
Token handle_comment(const char* src, int* index){
    Token token;
    if(src[*index] == '/' && src[*index + 1] == '/'){
        while(src[*index] != '\n' && src[*index] != '\0')(*index)++;
        return handle_comment(src, index);
    } else if (src[*index] == '/' && src[*index + 1] == '*') {
        // Comentário de bloco
        (*index) += 2; // Avançar para o começo do comentário
        while (!(src[*index] == '*' && src[*index + 1] == '/')) {
            if (src[*index] == '\0') {
                token.type = TOKEN_ERROR;
                strcpy(token.lexeme, "Comentário não fechado");
                return token;
            }
            (*index)++;
        }
        (*index) += 2; // Pular os caracteres "*/"
        return next_token(src, index); // Retorna o próximo token após o comentário
    }
}

Token handle_numbers(const char* src, int* index) {
    Token token = {TOKEN_INT, {0}};
    int i = 0;
    
    while (is_digit(src[*index])) {
        token.lexeme[i++] = src[(*index)++];
    }
    
    // Verificar se há ponto decimal (para floats)
    if (src[*index] == '.') {
        token.lexeme[i++] = src[(*index)++];
        while (is_digit(src[*index])) {
            token.lexeme[i++] = src[(*index)++];
        }
        token.type = TOKEN_FLOAT;
    }
    // Verificar se há letras após o número (erro léxico)
    else if (is_letter(src[*index])) {
        // Coletar toda a sequência inválida
        while (is_letter(src[*index]) || is_digit(src[*index])) {
            if (i < sizeof(token.lexeme) - 1) {  // Prevenir buffer overflow
                token.lexeme[i++] = src[(*index)++];
            }
        }
        token.lexeme[i] = '\0';
        token.type = TOKEN_UNKNOWN;  // Marcar como erro léxico
        return token;
    }
    
    token.lexeme[i] = '\0';
    return token;
}

Token handle_identifiers(const char* src, int* index) {
    Token token = {TOKEN_IDENTIFIER, {0}};
    int i = 0;

    if (!is_letter(src[*index])) {
        return handle_unknown(src, index);
    }

    token.lexeme[i++] = src[(*index)++];
    
    while (is_letter(src[*index]) || is_digit(src[*index])) {
        token.lexeme[i++] = src[(*index)++];
    }

    token.lexeme[i] = '\0';
    token.type = is_keyword(token.lexeme) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
    return token;
}

Token handle_operators(const char* src, int* index) {
    Token token = {TOKEN_OPERATOR, {0}};
    char current = src[*index];
    char next = src[*index + 1];
    int i = 0;
    
    token.lexeme[i++] = current;
    
    // Verificar operadores compostos
    if ((current == '=' && next == '=') ||
        (current == '!' && next == '=') ||
        (current == '<' && next == '=') ||
        (current == '>' && next == '=') ||
        (current == '+' && next == '+') ||
        (current == '-' && next == '-') ||
        (current == '&' && next == '&') ||
        (current == '|' && next == '|')) {
        
        token.lexeme[i++] = next;
        (*index)++;
    }
    
    token.lexeme[i] = '\0';
    (*index)++;
    return token;
}

Token handle_delimiters(const char* src, int* index) {
    Token token = {TOKEN_DELIMITER, {0}};
    token.lexeme[0] = src[(*index)++];
    token.lexeme[1] = '\0';
    return token;
}

Token handle_unknown(const char* src, int* index) {
    Token token = {TOKEN_UNKNOWN, {0}};
    token.lexeme[0] = src[(*index)++];
    token.lexeme[1] = '\0';
    return token;
}

Token next_token(const char* src, int* index) {
    // Pular espaços
    while (isspace(src[*index])) (*index)++;

    // Verificar EOF
    if (src[*index] == '\0') {
        return handle_eof();
    }

    // Determinar o tipo de token
    if (is_digit(src[*index])) {
        return handle_numbers(src, index);
    }
    else if (is_letter(src[*index])) {
        return handle_identifiers(src, index);
    }
    else if (strchr(operators, src[*index])) {
        return handle_operators(src, index);
    }
    else if (strchr(delimiters, src[*index])) {
        return handle_delimiters(src, index);
    }
    else if(strchr(comment, src[*index])){
        return handle_comment(src,index);
    }
    else {
        return handle_unknown(src, index);
    }
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