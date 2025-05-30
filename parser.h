#ifndef PARSER_H
#define PARSER_H

#include "analisador.h" // Para Token, TokenType, next_token, e MAX_SOURCE_SIZE

// Função principal para iniciar a análise sintática
void parse_program(const char* source_code);

// Opcional: Definições de nós da AST poderiam vir aqui em uma versão mais avançada.
// typedef struct ASTNode { ... } ASTNode;

#endif // PARSER_H