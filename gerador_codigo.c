#include "gerador_codigo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Protótipos de Funções Estáticas ---
static FILE* outfile;
static void gen_node(ASTNode* node);
static void gen_expression(ASTNode* node);

// --- Implementação ---

void generate_code(ASTNode* root, const char* output_filename) {
    outfile = fopen(output_filename, "w");
    if (!outfile) {
        perror("Não foi possível abrir o arquivo de saída para geração de código");
        exit(EXIT_FAILURE);
    }
    fprintf(outfile, "// --- Código Gerado pelo Compilador ---\n");
    fprintf(outfile, "#include <stdio.h>\n\n");
    gen_node(root);
    fclose(outfile);
}

static void gen_node(ASTNode* node) {
    if (!node) return;
    switch (node->type) {
        case NODE_PROGRAM:
            for (ASTNodeList* l = node->data.program.declarations; l; l = l->next) {
                gen_node(l->node);
                fprintf(outfile, "\n");
            }
            break;
        case NODE_VAR_DECL:
            fprintf(outfile, "%s %s", node->data.var_decl.type_name, node->data.var_decl.var_name);
            if (node->data.var_decl.initial_value) {
                fprintf(outfile, " = ");
                gen_expression(node->data.var_decl.initial_value);
            }
            fprintf(outfile, ";\n");
            break;
        case NODE_FUNC_DEF:
            fprintf(outfile, "int %s(", node->data.func_def.func_name);
            for (ASTNodeList* l = node->data.func_def.params; l; l = l->next) {
                ASTNode* param = l->node;
                fprintf(outfile, "%s %s", param->data.param.type_name, param->data.param.param_name);
                if (l->next) fprintf(outfile, ", ");
            }
            fprintf(outfile, ")");
            gen_node(node->data.func_def.body);
            break;
        case NODE_MAIN_DEF:
            fprintf(outfile, "int main() ");
            gen_node(node->data.main_def.body);
            break;
        case NODE_BLOCK:
            fprintf(outfile, "{\n");
            for (ASTNodeList* l = node->data.block.statements; l; l = l->next) gen_node(l->node);
            fprintf(outfile, "}\n");
            break;
        case NODE_IF:
            fprintf(outfile, "if (");
            gen_expression(node->data.if_stmt.condition);
            fprintf(outfile, ") ");
            gen_node(node->data.if_stmt.if_body);
            if (node->data.if_stmt.else_body) {
                fprintf(outfile, "else ");
                gen_node(node->data.if_stmt.else_body);
            }
            break;
        case NODE_RETURN:
            fprintf(outfile, "return ");
            if (node->data.return_stmt.return_value) gen_expression(node->data.return_stmt.return_value);
            fprintf(outfile, ";\n");
            break;
        default:
            gen_expression(node);
            fprintf(outfile, ";\n");
            break;
    }
}

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
            // <<< CORREÇÃO: Tratamento especial para a função 'print' >>>
            if (strcmp(node->data.func_call.func_name, "print") == 0) {
                fprintf(outfile, "printf(\"%%d\\n\", ");
                if (node->data.func_call.args) {
                    gen_expression(node->data.func_call.args->node);
                }
                fprintf(outfile, ")");
            } else { // Geração de código para funções normais
                fprintf(outfile, "%s(", node->data.func_call.func_name);
                for (ASTNodeList* l = node->data.func_call.args; l; l = l->next) {
                    gen_expression(l->node);
                    if (l->next) fprintf(outfile, ", ");
                }
                fprintf(outfile, ")");
            }
            break;
        default:
            fprintf(stderr, "Aviso: Nenhum caso de geração de código de expressão para o tipo de nó %d\n", node->type);
            break;
    }
}
