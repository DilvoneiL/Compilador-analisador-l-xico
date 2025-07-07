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

    printf("Iniciando Fase 1 e 2: Análise Léxica e Sintática...\n");
    ASTNode* ast_root = parse_program(source_code);
    printf("Análise Sintática concluída. AST construída.\n\n");
    
    printf("Iniciando Fase 3: Análise Semântica...\n");
    analyze_semantics(ast_root);
    
    int error_count = get_semantic_error_count();
    if (error_count > 0) {
        fprintf(stderr, "\nCompilação falhou com %d erro(s) semântico(s).\n", error_count);
        free_ast(ast_root);
        free(source_code);
        return 1;
    }
    printf("Análise Semântica concluída com sucesso.\n\n");
    printf("--- Árvore ANTES da otimização ---\n");
    print_ast(ast_root, 0);
    
    printf("Iniciando Fase 4: Otimização (Constant Folding)...\n");
    optimize_ast(ast_root);
    printf("Otimização concluída.\n\n");
    printf("\n--- Árvore DEPOIS da otimização ---\n");
    print_ast(ast_root, 0);

    // <<< CORREÇÃO: Alterado o nome do ficheiro de saída e a mensagem >>>
    printf("Iniciando Fase 5: Geração de Código (Transpilando para Python)...\n");
    generate_code(ast_root, "output.py");
    
    printf("\nCompilação concluída com sucesso!\n");

    free_ast(ast_root);
    free(source_code);

    return 0;
}
