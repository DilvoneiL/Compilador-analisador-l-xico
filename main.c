#include <stdio.h>
#include <stdlib.h>
#include "analisador.h"
#include "parser.h"
#include "analisador_semantico.h"
#include "otimizador.h"
#include "gerador_codigo.h"
#include "ast.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_fonte>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    // Ler o conteúdo do arquivo para um buffer
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* source_code = (char*)malloc(length + 1);
    if (!source_code) {
        fprintf(stderr, "Não foi possível alocar memória para o código fonte.\n");
        fclose(file);
        return 1;
    }
    fread(source_code, 1, length, file);
    source_code[length] = '\0';
    fclose(file);

    // --- FASE 1 e 2: Análise Léxica e Sintática (Parsing para construir a AST) ---
    printf("Iniciando Fase 1 e 2: Análise Léxica e Sintática...\n");
    ASTNode* ast_root = parse_program(source_code);
    printf("Análise Sintática concluída. AST construída.\n\n");
    
    // --- FASE 3: Análise Semântica ---
    printf("Iniciando Fase 3: Análise Semântica...\n");
    analyze_semantics(ast_root);
    printf("Análise Semântica concluída com sucesso.\n\n");

    // --- FASE 4: Otimização ---
    printf("Iniciando Fase 4: Otimização (Constant Folding)...\n");
    optimize_ast(ast_root);
    printf("Otimização concluída.\n\n");

    // Opcional: Imprimir a AST otimizada para ver os resultados
    printf("--- Árvore Sintática Abstrata (Após Otimização) ---\n");
    print_ast(ast_root, 0);
    printf("---------------------------------------------------\n\n");

    // --- FASE 5: Geração de Código ---
    printf("Iniciando Fase 5: Geração de Código (Transpilando para C)...\n");
    generate_code(ast_root, "output.c");
    
    printf("\nCompilação concluída com sucesso!\n");

    // Liberar memória
    free_ast(ast_root);
    free(source_code);

    return 0;
}
