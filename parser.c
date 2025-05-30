#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "analisador.h" // Para token_type_to_string, etc.

// Variáveis globais para o estado do parser
static Token current_token;
static const char* source_code_ptr;
static int current_parser_index;
static Position last_token_end_pos;
static int main_block_found_flag; // Para rastrear se o bloco main foi encontrado

// Protótipos de funções internas do parser
static void advance_and_skip_comments();
static void eat(TokenType type, const char* expected_lexeme);
static void syntax_error(const char* message, TokenType expected_type, const char* expected_lexeme_val);

// Funções de parsing de Topo (Novas/Revisadas)
static void parse_top_level_declaration();
static void parse_variable_declaration_global();
static void parse_standard_function_definition(); // Para 'fun nome(params) { ... }'
static void parse_parameter_list();
static void parse_parameter();
static void parse_main_function_definition();     // Para 'main { ... }'

// Funções de parsing para statements (dentro de blocos)
static void parse_statement_list();
static void parse_statement();
static void parse_declaration_statement(); // Declarações locais
static int is_type_specifier(Token t);
static void parse_expression_statement();
static void parse_if_statement();
static void parse_for_statement();
static void parse_return_statement();
static void parse_block_statement();

// Funções de Parsing de Expressão
static void parse_expression();
static void parse_assignment_expression();
static void parse_logical_or_expression();
static void parse_logical_and_expression();
static void parse_equality_expression();
static void parse_relational_expression();
static void parse_additive_expression();
static void parse_multiplicative_expression();
static void parse_unary_expression();
static void parse_primary_expression();

// Implementação das funções

static void advance_and_skip_comments() {
    if (current_token.type != TOKEN_EOF) {
        last_token_end_pos = current_pos;
    }
    current_token = next_token(source_code_ptr, &current_parser_index);
    while (current_token.type == TOKEN_COMMENT) {
        if (current_token.type != TOKEN_EOF) {
             last_token_end_pos = current_pos;
        }
        current_token = next_token(source_code_ptr, &current_parser_index);
    }
    // printf("Parser advanced to: Lexeme: '%s', Type: %s, Line: %d, Col: %d\n",
    //        current_token.lexeme, token_type_to_string(current_token.type), current_token.line, current_token.column);
}

static void eat(TokenType type, const char* expected_lexeme) {
    if (current_token.type == type) {
        if (expected_lexeme != NULL && strcmp(current_token.lexeme, expected_lexeme) != 0) {
            char error_msg[256];
            sprintf(error_msg, "Lexema inesperado. Esperava '%s' para o token %s", expected_lexeme, token_type_to_string(type));
            syntax_error(error_msg, type, expected_lexeme);
        }
        advance_and_skip_comments();
    } else {
        syntax_error("Token inesperado.", type, expected_lexeme);
    }
}

static void syntax_error(const char* message, TokenType expected_type, const char* expected_lexeme_val) {
    fprintf(stderr, "\nErro Sintático (Linha %d, Coluna %d): %s\n",
            current_token.line, current_token.column, message);
    if (expected_type != TOKEN_UNKNOWN) {
        if (expected_lexeme_val != NULL) {
            fprintf(stderr, "  Esperava o token '%s' com lexema '%s'.\n",
                    token_type_to_string(expected_type), expected_lexeme_val);
        } else {
            fprintf(stderr, "  Esperava um token do tipo '%s'.\n", token_type_to_string(expected_type));
        }
    }
    fprintf(stderr, "  Mas encontrou o token '%s' (Lexema: '%s').\n",
            token_type_to_string(current_token.type), current_token.lexeme);
    exit(EXIT_FAILURE);
}

static int is_type_specifier(Token t) {
    if (t.type == TOKEN_KEYWORD) {
        return strcmp(t.lexeme, "int") == 0 ||
               strcmp(t.lexeme, "float") == 0 ||
               strcmp(t.lexeme, "char") == 0;
    }
    return 0;
}

void parse_program(const char* source) {
    source_code_ptr = source;
    current_parser_index = 0;
    current_pos.line = 1;
    current_pos.column = 1;
    main_block_found_flag = 0;

    printf("Iniciando análise sintática...\n");
    advance_and_skip_comments();

    if (current_token.type == TOKEN_EOF) {
        syntax_error("Arquivo de código fonte vazio. Bloco 'main { ... }' é obrigatório.", TOKEN_KEYWORD, "main");
    }

    // 1. Analisa declarações globais (variáveis e funções 'fun') ANTES do bloco 'main'
    while (current_token.type != TOKEN_EOF &&
           !(current_token.type == TOKEN_KEYWORD && strcmp(current_token.lexeme, "main") == 0)) {
        
        if (is_type_specifier(current_token) || (current_token.type == TOKEN_KEYWORD && strcmp(current_token.lexeme, "fun") == 0)) {
            parse_top_level_declaration();
        } else {
            syntax_error("Declaração de topo inválida. Esperava 'int', 'float', 'char', 'fun', ou o bloco 'main'.",
                         current_token.type, // Reporta o tipo de token encontrado
                         NULL); // Não há um lexema específico esperado aqui, pode ser vários
        }
    }

    // 2. O bloco 'main' DEVE ser encontrado agora
    if (current_token.type == TOKEN_KEYWORD && strcmp(current_token.lexeme, "main") == 0) {
        parse_main_function_definition();
        main_block_found_flag = 1;
    } else {
        // Se o loop terminou (EOF ou outra coisa) e 'main' não foi encontrado
        syntax_error("Bloco 'main { ... }' obrigatório não encontrado no final das declarações de topo.", TOKEN_KEYWORD, "main");
    }

    // 3. Após o bloco 'main', NADA MAIS é permitido, apenas EOF.
    if (main_block_found_flag) { // Só executa se o main foi encontrado e parseado
        if (current_token.type != TOKEN_EOF) {
            syntax_error("Conteúdo adicional encontrado após o bloco 'main'. Esperava fim de arquivo.", TOKEN_EOF, "EOF");
        } else {
            printf("Token: %-30s | Tipo: %-15s | Linha: %-4d | Coluna: %-4d\n",
                   current_token.lexeme, token_type_to_string(current_token.type), current_token.line, current_token.column);
            printf("Análise sintática concluída com sucesso!\n");
        }
    }
    // Se main_block_found_flag é 0, syntax_error() já foi chamado e o programa terminou.
}

// TopLevelDeclaration -> VariableDeclarationGlobal | StandardFunctionDefinition
static void parse_top_level_declaration() {
    if (is_type_specifier(current_token)) {
        parse_variable_declaration_global();
    } else if (current_token.type == TOKEN_KEYWORD && strcmp(current_token.lexeme, "fun") == 0) {
        parse_standard_function_definition();
    } else {
        // Esta condição não deveria ser alcançada se a lógica em parse_program estiver correta,
        // pois parse_program já filtra os tipos de tokens válidos para este ponto.
        syntax_error("Erro interno: parse_top_level_declaration chamada com token inesperado.", current_token.type, current_token.lexeme);
    }
}

// VariableDeclarationGlobal -> TypeSpecifier Identifier ';'
static void parse_variable_declaration_global() {
    printf("Parsing Variavel Global: Tipo '%s'\n", current_token.lexeme);
    eat(TOKEN_KEYWORD, current_token.lexeme); // Consome 'int', 'float', ou 'char'

    if (current_token.type != TOKEN_IDENTIFIER) {
        syntax_error("Esperava um identificador para a variável global.", TOKEN_IDENTIFIER, NULL);
    }
    printf("Parsing Variavel Global: Identificador '%s'\n", current_token.lexeme);
    eat(TOKEN_IDENTIFIER, NULL);

    // Simplificação: variáveis globais não podem ser inicializadas na declaração nesta linguagem.
    // Se pudessem: if (current_token.type == TOKEN_OPERATOR && strcmp(current_token.lexeme, "=") == 0) { ... }

    eat(TOKEN_DELIMITER, ";");
    // printf("Ponto e vírgula da Variavel Global consumido.\n"); // Opcional
}

// StandardFunctionDefinition -> 'fun' Identifier '(' ParameterList? ')' BlockStatement
static void parse_standard_function_definition() {
    printf("Parsing Funcao (fun): Keyword '%s'\n", current_token.lexeme);
    eat(TOKEN_KEYWORD, "fun");

    if (current_token.type != TOKEN_IDENTIFIER) {
        syntax_error("Esperava um nome para a função após 'fun'.", TOKEN_IDENTIFIER, NULL);
    }
    printf("Parsing Funcao (fun): Nome '%s'\n", current_token.lexeme);
    eat(TOKEN_IDENTIFIER, NULL);

    eat(TOKEN_DELIMITER, "(");
    
    if (!(current_token.type == TOKEN_DELIMITER && strcmp(current_token.lexeme, ")") == 0)) {
        // Apenas analisa parâmetros se não for diretamente um ')'
        parse_parameter_list();
    }
    // Se for ')', o eat abaixo irá consumi-lo.

    eat(TOKEN_DELIMITER, ")");
    parse_block_statement(); 
    printf("Funcao (fun): Corpo da função analisado.\n");
}

// ParameterList -> Parameter (',' Parameter)*
static void parse_parameter_list() {
    parse_parameter(); // Analisa o primeiro parâmetro
    while (current_token.type == TOKEN_DELIMITER && strcmp(current_token.lexeme, ",") == 0) {
        eat(TOKEN_DELIMITER, ",");
        parse_parameter(); // Analisa parâmetros subsequentes
    }
}

// Parameter -> TypeSpecifier Identifier
static void parse_parameter() {
    if (!is_type_specifier(current_token)) {
        syntax_error("Esperava um especificador de tipo para o parâmetro (int, float, char).", TOKEN_KEYWORD, NULL);
    }
    printf("Parsing Parametro: Tipo '%s'\n", current_token.lexeme);
    eat(TOKEN_KEYWORD, current_token.lexeme);

    if (current_token.type != TOKEN_IDENTIFIER) {
        syntax_error("Esperava um identificador para o parâmetro.", TOKEN_IDENTIFIER, NULL);
    }
    printf("Parsing Parametro: Identificador '%s'\n", current_token.lexeme);
    eat(TOKEN_IDENTIFIER, NULL);
}

// MainFunctionDefinition -> 'main' BlockStatement
static void parse_main_function_definition() {
    printf("Parsing Bloco Principal (main)...\n");
    eat(TOKEN_KEYWORD, "main"); 
    // O próximo token deve ser '{', que será consumido por parse_block_statement
    parse_block_statement(); 
    printf("Bloco Principal (main) analisado.\n");
}

// --- Funções de Análise de Statements e Expressões (permanecem as mesmas da sua última versão) ---

static void parse_statement_list() {
    while (current_token.type != TOKEN_EOF && !(current_token.type == TOKEN_DELIMITER && strcmp(current_token.lexeme, "}") == 0) ) {
        parse_statement();
    }
}

static void parse_statement() {
    if (is_type_specifier(current_token)) {
        parse_declaration_statement(); // Declaração de variável local
    } else if (current_token.type == TOKEN_KEYWORD && strcmp(current_token.lexeme, "if") == 0) {
        parse_if_statement();
    } else if (current_token.type == TOKEN_KEYWORD && strcmp(current_token.lexeme, "for") == 0) {
        parse_for_statement();
    } else if (current_token.type == TOKEN_KEYWORD && strcmp(current_token.lexeme, "return") == 0) {
        parse_return_statement();
    } else if (current_token.type == TOKEN_DELIMITER && strcmp(current_token.lexeme, "{") == 0) {
        parse_block_statement(); // Bloco aninhado
    } else if (current_token.type == TOKEN_DELIMITER && strcmp(current_token.lexeme, ";") == 0) {
        eat(TOKEN_DELIMITER, ";"); // Statement vazio
    }
    else if (current_token.type != TOKEN_EOF && !(current_token.type == TOKEN_DELIMITER && strcmp(current_token.lexeme, "}") == 0)) {
        parse_expression_statement();
    }
}

static void parse_declaration_statement() { // Para variáveis locais
    printf("Parsing Decl. Local: Tipo '%s'\n", current_token.lexeme);
    eat(TOKEN_KEYWORD, current_token.lexeme);

    if (current_token.type != TOKEN_IDENTIFIER) {
         syntax_error("Esperava um identificador após o tipo na declaração local.", TOKEN_IDENTIFIER, NULL);
    }
    printf("Parsing Decl. Local: Identificador '%s'\n", current_token.lexeme);
    eat(TOKEN_IDENTIFIER, NULL);

    if (current_token.type == TOKEN_OPERATOR && strcmp(current_token.lexeme, "=") == 0) {
        printf("Parsing Decl. Local: Atribuição '%s'\n", current_token.lexeme);
        eat(TOKEN_OPERATOR, "=");
        parse_expression();
    }

    eat(TOKEN_DELIMITER, ";");
    // printf("Ponto e vírgula da Decl. Local consumido.\n"); // Opcional
}

static void parse_expression_statement() {
    parse_expression();
    eat(TOKEN_DELIMITER, ";");
}

static void parse_if_statement() {
    eat(TOKEN_KEYWORD, "if"); 
    eat(TOKEN_DELIMITER, "(");
    parse_expression();
    eat(TOKEN_DELIMITER, ")");
    parse_statement();
    if (current_token.type == TOKEN_KEYWORD && strcmp(current_token.lexeme, "else") == 0) {
        eat(TOKEN_KEYWORD, "else");
        parse_statement();
    }
}

static void parse_for_statement() {
    eat(TOKEN_KEYWORD, "for"); 
    eat(TOKEN_DELIMITER, "("); 
    // Inicialização
    if (!(current_token.type == TOKEN_DELIMITER && strcmp(current_token.lexeme, ";") == 0)) {
        if (is_type_specifier(current_token)) { // Declaração no for: int i = 0
            eat(TOKEN_KEYWORD, current_token.lexeme); 
            eat(TOKEN_IDENTIFIER, NULL); 
            if (current_token.type == TOKEN_OPERATOR && strcmp(current_token.lexeme, "=") == 0) {
                eat(TOKEN_OPERATOR, "=");
                parse_expression();
            }
        } else { // Expressão no for: i = 0
            parse_expression(); 
        }
    }
    eat(TOKEN_DELIMITER, ";"); 
    // Condição
    if (!(current_token.type == TOKEN_DELIMITER && strcmp(current_token.lexeme, ";") == 0)) {
        parse_expression();
    }
    eat(TOKEN_DELIMITER, ";"); 
    // Incremento
    if (!(current_token.type == TOKEN_DELIMITER && strcmp(current_token.lexeme, ")") == 0)) { 
        parse_expression();
    }
    eat(TOKEN_DELIMITER, ")"); 
    parse_statement(); 
}

static void parse_return_statement() {
    eat(TOKEN_KEYWORD, "return"); 
    if (!(current_token.type == TOKEN_DELIMITER && strcmp(current_token.lexeme, ";") == 0)) {
        parse_expression();
    }
    eat(TOKEN_DELIMITER, ";"); 
}

static void parse_block_statement() {
    eat(TOKEN_DELIMITER, "{"); 
    // printf("Entrando em bloco aninhado/corpo...\n"); // Opcional
    parse_statement_list();
    eat(TOKEN_DELIMITER, "}"); 
    // printf("Saindo do bloco.\n"); // Opcional
}

static void parse_expression() {
    parse_assignment_expression();
}

static void parse_assignment_expression() {
    parse_logical_or_expression(); 
    if (current_token.type == TOKEN_OPERATOR && strcmp(current_token.lexeme, "=") == 0) {
        eat(TOKEN_OPERATOR, "="); 
        parse_assignment_expression(); 
    }
}

static void parse_logical_or_expression() {
    parse_logical_and_expression();
    while (current_token.type == TOKEN_OPERATOR && strcmp(current_token.lexeme, "||") == 0) {
        eat(TOKEN_OPERATOR, "||");
        parse_logical_and_expression();
    }
}

static void parse_logical_and_expression() {
    parse_equality_expression();
    while (current_token.type == TOKEN_OPERATOR && strcmp(current_token.lexeme, "&&") == 0) {
        eat(TOKEN_OPERATOR, "&&");
        parse_equality_expression();
    }
}

static void parse_equality_expression() {
    parse_relational_expression();
    while (current_token.type == TOKEN_OPERATOR &&
           (strcmp(current_token.lexeme, "==") == 0 || strcmp(current_token.lexeme, "!=") == 0)) {
        eat(TOKEN_OPERATOR, current_token.lexeme);
        parse_relational_expression();
    }
}

static void parse_relational_expression() {
    parse_additive_expression();
    while (current_token.type == TOKEN_OPERATOR &&
           (strcmp(current_token.lexeme, "<") == 0 || strcmp(current_token.lexeme, ">") == 0 ||
            strcmp(current_token.lexeme, "<=") == 0 || strcmp(current_token.lexeme, ">=") == 0)) {
        eat(TOKEN_OPERATOR, current_token.lexeme);
        parse_additive_expression();
    }
}

static void parse_additive_expression() {
    parse_multiplicative_expression();
    while (current_token.type == TOKEN_OPERATOR &&
           (strcmp(current_token.lexeme, "+") == 0 || strcmp(current_token.lexeme, "-") == 0)) {
        eat(TOKEN_OPERATOR, current_token.lexeme);
        parse_multiplicative_expression();
    }
}

static void parse_multiplicative_expression() {
    parse_unary_expression(); 
    while (current_token.type == TOKEN_OPERATOR &&
           (strcmp(current_token.lexeme, "*") == 0 || strcmp(current_token.lexeme, "/") == 0)) {
        eat(TOKEN_OPERATOR, current_token.lexeme);
        parse_unary_expression(); 
    }
}

static void parse_unary_expression() {
    if (current_token.type == TOKEN_OPERATOR &&
        (strcmp(current_token.lexeme, "!") == 0 ||  
         strcmp(current_token.lexeme, "-") == 0 ||  
         strcmp(current_token.lexeme, "++") == 0 || 
         strcmp(current_token.lexeme, "--") == 0 )) {
        eat(TOKEN_OPERATOR, current_token.lexeme);
        parse_unary_expression(); 
    } else {
        parse_primary_expression(); 
    }
}

static void parse_primary_expression() {
    if (current_token.type == TOKEN_IDENTIFIER) {
        // char id_lexeme_copy[100]; 
        // strcpy(id_lexeme_copy, current_token.lexeme); // Opcional para logs
        eat(TOKEN_IDENTIFIER, NULL); 

        if (current_token.type == TOKEN_DELIMITER && strcmp(current_token.lexeme, "(") == 0) { 
            eat(TOKEN_DELIMITER, "("); 
            if (!(current_token.type == TOKEN_DELIMITER && strcmp(current_token.lexeme, ")") == 0)) {
                parse_expression(); 
                while (current_token.type == TOKEN_DELIMITER && strcmp(current_token.lexeme, ",") == 0) {
                    eat(TOKEN_DELIMITER, ","); 
                    parse_expression();   
                }
            }
            eat(TOKEN_DELIMITER, ")"); 
        }
        else if (current_token.type == TOKEN_OPERATOR &&
                   (strcmp(current_token.lexeme, "++") == 0 || strcmp(current_token.lexeme, "--") == 0)) {
            eat(TOKEN_OPERATOR, current_token.lexeme);
        }
    } else if (current_token.type == TOKEN_INT) {
        eat(TOKEN_INT, NULL);
    } else if (current_token.type == TOKEN_FLOAT) {
        eat(TOKEN_FLOAT, NULL);
    } else if (current_token.type == TOKEN_STRING) {
        eat(TOKEN_STRING, NULL);
    } else if (current_token.type == TOKEN_CHAR) {
        eat(TOKEN_CHAR, NULL);
    } else if (current_token.type == TOKEN_DELIMITER && strcmp(current_token.lexeme, "(") == 0) { 
        eat(TOKEN_DELIMITER, "("); 
        parse_expression();
        eat(TOKEN_DELIMITER, ")"); 
    } else {
        syntax_error("Token inesperado na expressão primária. Esperava Identificador, Literal ou '('.", TOKEN_UNKNOWN, NULL);
    }
}