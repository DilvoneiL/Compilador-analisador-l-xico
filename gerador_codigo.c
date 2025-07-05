#include "gerador_codigo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Variáveis de Estado do Gerador ---
static FILE* outfile;
static int indent_level = 0; // Controla a indentação do Python

// --- Protótipos de Funções Estáticas ---
static void gen_node(ASTNode* node);
static void gen_expression(ASTNode* node);
static void print_indent();

// --- Implementação ---

void generate_code(ASTNode* root, const char* output_filename) {
    outfile = fopen(output_filename, "w");
    if (!outfile) {
        perror("Não foi possível abrir o arquivo de saída para geração de código");
        exit(EXIT_FAILURE);
    }

    fprintf(outfile, "# --- Código Gerado pelo Compilador ---\n\n");
    gen_node(root);
    fclose(outfile);
}

// Função auxiliar para imprimir a indentação correta
static void print_indent() {
    for (int i = 0; i < indent_level; ++i) {
        fprintf(outfile, "    "); // 4 espaços por nível de indentação
    }
}

// Função de despacho principal que chama o gerador apropriado para cada tipo de nó.
static void gen_node(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM: {
            // Em Python, as declarações globais e funções vêm primeiro.
            ASTNode* main_node = NULL;
            for (ASTNodeList* l = node->data.program.declarations; l; l = l->next) {
                if (l->node->type == NODE_MAIN_DEF) {
                    main_node = l->node; // Guarda o nó main para mais tarde
                } else {
                    gen_node(l->node); // Gera funções e variáveis globais
                }
            }
            // Agora, gera o bloco principal
            if (main_node) {
                fprintf(outfile, "\n\nif __name__ == \"__main__\":\n");
                indent_level++;
                gen_node(main_node->data.main_def.body);
                indent_level--;
            }
            break;
        }
        case NODE_VAR_DECL:
            print_indent();
            // Em Python, a declaração de tipo é implícita na atribuição.
            fprintf(outfile, "%s", node->data.var_decl.var_name);
            if (node->data.var_decl.initial_value) {
                fprintf(outfile, " = ");
                gen_expression(node->data.var_decl.initial_value);
            } else {
                fprintf(outfile, " = None"); // Variáveis não inicializadas tornam-se None
            }
            fprintf(outfile, "\n");
            break;
        case NODE_FUNC_DEF:
            fprintf(outfile, "\n");
            print_indent();
            fprintf(outfile, "def %s(", node->data.func_def.func_name);
            for (ASTNodeList* l = node->data.func_def.params; l; l = l->next) {
                fprintf(outfile, "%s", l->node->data.param.param_name);
                if (l->next) fprintf(outfile, ", ");
            }
            fprintf(outfile, "):\n");
            indent_level++;
            gen_node(node->data.func_def.body);
            indent_level--;
            break;
        case NODE_BLOCK:
            // Se o bloco estiver vazio, Python requer a palavra-passe 'pass'
            if (node->data.block.statements == NULL) {
                print_indent();
                fprintf(outfile, "pass\n");
            } else {
                for (ASTNodeList* l = node->data.block.statements; l; l = l->next) {
                    gen_node(l->node);
                }
            }
            break;
        case NODE_IF:
            print_indent();
            fprintf(outfile, "if ");
            gen_expression(node->data.if_stmt.condition);
            fprintf(outfile, ":\n");
            indent_level++;
            gen_node(node->data.if_stmt.if_body);
            indent_level--;
            if (node->data.if_stmt.else_body) {
                print_indent();
                fprintf(outfile, "else:\n");
                indent_level++;
                gen_node(node->data.if_stmt.else_body);
                indent_level--;
            }
            break;
        case NODE_RETURN:
            print_indent();
            fprintf(outfile, "return ");
            if (node->data.return_stmt.return_value) {
                gen_expression(node->data.return_stmt.return_value);
            }
            fprintf(outfile, "\n");
            break;
        default:
            // Trata expressões como statements (ex: chamadas de função)
            print_indent();
            gen_expression(node);
            fprintf(outfile, "\n");
            break;
    }
}

// Função específica para gerar código para qualquer nó de expressão.
static void gen_expression(ASTNode* node) {
    if (!node) return;
    switch (node->type) {
        case NODE_INT_LITERAL: fprintf(outfile, "%d", node->data.int_literal); break;
        case NODE_FLOAT_LITERAL: fprintf(outfile, "%f", node->data.float_literal); break;
        case NODE_IDENTIFIER: fprintf(outfile, "%s", node->data.identifier_name); break;
        case NODE_ASSIGN:
            gen_expression(node->data.assign_expr.lvalue);
            fprintf(outfile, " = ");
            gen_expression(node->data.assign_expr.rvalue);
            break;
        case NODE_BINARY_OP:
            fprintf(outfile, "(");
            gen_expression(node->data.binary_op.left);
            fprintf(outfile, " %s ", node->data.binary_op.op);
            gen_expression(node->data.binary_op.right);
            fprintf(outfile, ")");
            break;
        case NODE_FUNC_CALL:
            // A função 'print' da nossa linguagem mapeia-se diretamente para 'print' do Python.
            fprintf(outfile, "%s(", node->data.func_call.func_name);
            for (ASTNodeList* l = node->data.func_call.args; l; l = l->next) {
                gen_expression(l->node);
                if (l->next) fprintf(outfile, ", ");
            }
            fprintf(outfile, ")");
            break;
        default:
            break;
    }
}
