#include "analisador.h" 
#include "parser.h"     
#include <stdio.h> 

const char* keywords[] = {"if", "else", "while", "for", "return", "int", "float", "char", "fun", "main", "void"};
const char* operators = "+-*/=%!<>&|";
const char* delimiters = "();{},";
const char string_delimiter = '\"';
const char char_delimiter = '\'';

Position current_pos = {1, 1}; // Inicia na linha 1, coluna 1

void update_position(char c) {
    if (c == '\n') {
        current_pos.line++;
        current_pos.column = 1;
    } else {
        current_pos.column++;
    }
}

int is_letter(char c) {
    return isalpha(c) || c == '_'; // Adicionado suporte a underscore
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


Token handle_eof(Position start_pos) {
    Token token;
    token.type = TOKEN_EOF;
    strcpy(token.lexeme, "EOF");
    token.line = start_pos.line;
    token.column = start_pos.column;
    return token;
}

Token handle_string(const char* src, int* index, Position start_pos) {
    Token token = {TOKEN_STRING, {0}, start_pos.line, start_pos.column};
    int i = 0;
    char delimiter = src[(*index)++];
    update_position(delimiter);
    
    token.lexeme[i++] = delimiter;
    
    while (src[*index] != delimiter && src[*index] != '\0') {
        if (src[*index] == '\\') {
            if (i < sizeof(token.lexeme) - 1) {
                token.lexeme[i++] = src[(*index)++];
                update_position('\\');
            }
            if (src[*index] != '\0' && i < sizeof(token.lexeme) - 1) {
                token.lexeme[i++] = src[(*index)++];
                update_position(token.lexeme[i-1]);
            }
        } else {
            if (i < sizeof(token.lexeme) - 1) {
                token.lexeme[i++] = src[(*index)++];
                update_position(token.lexeme[i-1]);
            } else {
                (*index)++;
                update_position(src[*index - 1]);
            }
        }
    }
    
    if (src[*index] == delimiter) {
        if (i < sizeof(token.lexeme) - 1) {
            token.lexeme[i++] = src[(*index)++];
            update_position(delimiter);
        } else {
            (*index)++;
            update_position(delimiter);
        }
    } else {
        token.type = TOKEN_UNKNOWN;
    }
    
    token.lexeme[i] = '\0';
    
    if (delimiter == '\'' && i == 3) {
        token.type = TOKEN_CHAR;
    } else if (delimiter == '\'' && i != 3) {
        token.type = TOKEN_UNKNOWN;
    }
    
    return token;
}

Token handle_line_comment(const char* src, int* index, Position start_pos ) {
    Token token = {TOKEN_COMMENT, {0}, start_pos.line, start_pos.column};
    int i = 0;
    
    // Adiciona os '//' iniciais
    token.lexeme[i++] = src[(*index)++];
    token.lexeme[i++] = src[(*index)++];
    
    // Captura até o final da linha (incluindo o '\n')
    while (src[*index] != '\n' && src[*index] != '\0' && i < sizeof(token.lexeme) - 1) {
        token.lexeme[i++] = src[(*index)++];
    }
    
    // Inclui a quebra de linha no token
    if (src[*index] == '\n' && i < sizeof(token.lexeme) - 1) {
        update_position('\n'); 
        //(*index)++;
        token.lexeme[i++] = src[(*index)++];
    }
    
    token.lexeme[i] = '\0';
    return token;
}


Token handle_block_comment(const char* src, int* index, Position start_pos) {
    Token token = {TOKEN_COMMENT, {0}, start_pos.line, start_pos.column};
    int i = 0;
    
    // Adiciona o '/*' inicial
    token.lexeme[i++] = src[(*index)++];
    update_position('/');
    token.lexeme[i++] = src[(*index)++];
    update_position('*');
    
    // Consome até encontrar '*/'
    while (!(src[*index] == '*' && src[*index + 1] == '/') && src[*index] != '\0') {
        if (i < sizeof(token.lexeme) - 1) {
            token.lexeme[i++] = src[(*index)++];
            update_position(token.lexeme[i-1]);
        } else {
            (*index)++;
            update_position(src[*index-1]);
        }
    }
    
    // Verifica se encontrou o fechamento
    if (src[*index] == '*' && src[*index + 1] == '/') {
        token.lexeme[i++] = src[(*index)++];
        update_position('*');
        token.lexeme[i++] = src[(*index)++];
        update_position('/');
    } else {
        token.type = TOKEN_UNKNOWN; // Comentário não fechado
    }
    
    token.lexeme[i] = '\0';
    return token;
}

Token handle_numbers(const char* src, int* index, Position start_pos) {
    Token token = {TOKEN_INT, {0}, start_pos.line, start_pos.column};
    int i = 0;
    
    while (is_digit(src[*index])) {
        token.lexeme[i++] = src[(*index)++];
        update_position(token.lexeme[i-1]);
    }
    
    // Verificar ponto decimal (para floats)
    if (src[*index] == '.') {
        token.lexeme[i++] = src[(*index)++];
        update_position('.');
        while (is_digit(src[*index])) {
            token.lexeme[i++] = src[(*index)++];
            update_position(token.lexeme[i-1]);
        }
        token.type = TOKEN_FLOAT;
    }
    // Verificar letras após número (erro léxico)
    else if (is_letter(src[*index])) {
        while (is_letter(src[*index]) || is_digit(src[*index])) {
            if (i < sizeof(token.lexeme) - 1) {
                token.lexeme[i++] = src[(*index)++];
                update_position(token.lexeme[i-1]);
            }
        }
        token.lexeme[i] = '\0';
        token.type = TOKEN_UNKNOWN;
        return token;
    }
    
    token.lexeme[i] = '\0';
    return token;
}

Token handle_identifiers(const char* src, int* index, Position start_pos) {
    Token token = {TOKEN_IDENTIFIER, {0}, start_pos.line, start_pos.column};
    int i = 0;

    if (!is_letter(src[*index])) {
        return handle_unknown(src, index, start_pos);
    }

    token.lexeme[i++] = src[(*index)++];
    update_position(token.lexeme[i-1]);
    
    while (is_letter(src[*index]) || is_digit(src[*index])) {
        token.lexeme[i++] = src[(*index)++];
        update_position(token.lexeme[i-1]);
    }

    token.lexeme[i] = '\0';
    token.type = is_keyword(token.lexeme) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
    return token;
}

Token handle_operators(const char* src, int* index, Position start_pos) {
    Token token = {TOKEN_OPERATOR, {0}, start_pos.line, start_pos.column};
    char current = src[*index];
    char next = src[*index + 1];
    int i = 0;
    
    token.lexeme[i++] = current;
    
    // Verifica comentários antes de operadores
    if (current == '/' && next == '/') {
        return handle_line_comment(src, index, start_pos);
    }
    else if (current == '/' && next == '*') {
        return handle_block_comment(src, index, start_pos);
    }

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
        update_position(next);
    }
    
    token.lexeme[i] = '\0';
    (*index)++;
    update_position(current);
    return token;
}

Token handle_delimiters(const char* src, int* index, Position start_pos) {
    Token token = {TOKEN_DELIMITER, {0}, start_pos.line, start_pos.column};
    token.lexeme[0] = src[(*index)++];
    update_position(token.lexeme[0]);
    token.lexeme[1] = '\0';
    return token;
}

Token handle_unknown(const char* src, int* index, Position start_pos) {
    Token token = {TOKEN_UNKNOWN, {0}, start_pos.line, start_pos.column};
    token.lexeme[0] = src[(*index)++];
    update_position(token.lexeme[0]);
    token.lexeme[1] = '\0';
    return token;
}

Token next_token(const char* src, int* index) {
    
    // Pular espaços
    // E na função next_token, modifique a parte que lida com espaços:
    while (isspace(src[*index])) {
        if (src[*index] == '\n') {
            current_pos.line++;
            current_pos.column = 1;
        } else {
            current_pos.column++;
        }
        (*index)++;
    }
    
    Position start_pos = current_pos;
    // Verificar EOF
    if (src[*index] == '\0') {
        return handle_eof(start_pos);
    }

        // Determinar o tipo de token
    if (src[*index] == '/' && src[*index + 1] == '/') {
        return handle_line_comment(src, index, start_pos);
    }
    else if (src[*index] == '\"' || src[*index] == '\'') {
        return handle_string(src, index, start_pos);
    }
    else if (src[*index] == '/' && src[*index + 1] == '*') {
        return handle_block_comment(src, index, start_pos);
    }
    else if (is_digit(src[*index])) {
        return handle_numbers(src, index, start_pos);
    }
    else if (is_letter(src[*index])) {
        return handle_identifiers(src, index, start_pos);
    }
    else if (strchr(operators, src[*index])) {
        return handle_operators(src, index, start_pos);
    }
    else if (strchr(delimiters, src[*index])) {
        return handle_delimiters(src, index, start_pos);
    }
    else {
        return handle_unknown(src, index, start_pos);
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
        case TOKEN_STRING: return "TOKEN_STRING";
        case TOKEN_CHAR: return "TOKEN_CHAR";
        case TOKEN_EOF: return "TOKEN_EOF";
        case TOKEN_COMMENT: return "TOKEN_COMMENT";
        default: return "TOKEN_UNKNOWN";
    }
}




int main() {
    // 1. Abrir o arquivo de código fonte
    FILE* file = fopen("codigo.txt", "r"); // Abre "codigo.txt" para leitura [cite: 2]
    if (file == NULL) {
        // Trata erro se o arquivo não puder ser aberto
        printf("Erro ao abrir o arquivo 'codigo.txt'.\n"); // [cite: 2]
        return 1; // Termina o programa com um código de erro
    }

    // 2. Ler o conteúdo do arquivo
    // MAX_SOURCE_SIZE é definido em analisador.h [cite: 1]
    char source[MAX_SOURCE_SIZE]; 
    int length = fread(source, 1, MAX_SOURCE_SIZE - 1, file); // Lê o arquivo para o buffer 'source' [cite: 2]
    source[length] = '\0'; // Adiciona o terminador nulo para formar uma string C válida [cite: 2]

    // 3. Fechar o arquivo
    fclose(file); // [cite: 2]

    // 4. Iniciar a análise sintática (parsing)
    // A variável global 'current_pos' (de analisador.h) é usada pelo lexer
    // e parser para rastrear a posição. Pode ser bom garantir que ela comece em {1,1}.
    // A função 'parse_program' em parser.c também pode inicializar isso.
    // No seu 'analisador.c', 'current_pos' é global e inicializada.
    // No 'parser.c' que forneci, 'current_pos' é resetada no início de 'parse_program'.

    parse_program(source); // Chama a função principal do parser com o código lido [cite: 2]

    // O loop antigo de impressão de tokens que você comentou pode continuar comentado,
    // já que 'parse_program' agora cuida de reportar o sucesso ou erros.

    return 0; // Indica que o programa terminou com sucesso
}