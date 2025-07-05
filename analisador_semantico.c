#include "analisador_semantico.h"
#include "tabela_simbolos.h"
#include <stdio.h>
#include <stdlib.h>

// --- Variáveis e Funções de Controlo de Erro ---
static int semantic_error_count = 0;

static void semantic_error(const char* message, int line, int column) {
    fprintf(stderr, "Erro Semântico (Linha %d, Coluna %d): %s\n", line, column, message);
    semantic_error_count++;
}

int get_semantic_error_count() {
    return semantic_error_count;
}

// --- Protótipos de Funções Estáticas ---
static void visit_node(ASTNode* node);
static DataType get_expression_type(ASTNode* node);

// --- Implementação ---

void analyze_semantics(ASTNode* root) {
    init_symbol_table();
    semantic_error_count = 0; // Zera o contador para uma nova análise
    visit_node(root);
}

static void visit_node(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM:
            enter_scope();
            for (ASTNodeList* l = node->data.program.declarations; l; l = l->next) visit_node(l->node);
            exit_scope();
            break;
        case NODE_MAIN_DEF:
            visit_node(node->data.main_def.body);
            break;
        case NODE_BLOCK:
            enter_scope();
            for (ASTNodeList* l = node->data.block.statements; l; l = l->next) visit_node(l->node);
            exit_scope();
            break;
        case NODE_VAR_DECL: {
            // Verifica se o símbolo já existe ANTES de o adicionar
            if (lookup_symbol_in_current_scope(node->data.var_decl.var_name)) {
                char msg[256];
                sprintf(msg, "Redeclaração do identificador '%s'.", node->data.var_decl.var_name);
                semantic_error(msg, node->pos.line, node->pos.column);
            } else {
                DataType type = string_to_datatype(node->data.var_decl.type_name);
                add_symbol(node->data.var_decl.var_name, type, node);
            }
            if (node->data.var_decl.initial_value) {
                DataType lvalue_type = string_to_datatype(node->data.var_decl.type_name);
                DataType rvalue_type = get_expression_type(node->data.var_decl.initial_value);
                if (lvalue_type != rvalue_type && rvalue_type != TYPE_UNKNOWN) {
                    semantic_error("Tipos incompatíveis na inicialização.", node->pos.line, node->pos.column);
                }
            }
            break;
        }
        case NODE_FUNC_DEF:
            if (lookup_symbol_in_current_scope(node->data.func_def.func_name)) {
                 semantic_error("Redeclaração da função.", node->pos.line, node->pos.column);
            } else {
                 add_symbol(node->data.func_def.func_name, TYPE_FUNCTION, node);
            }
            enter_scope();
            for (ASTNodeList* l = node->data.func_def.params; l; l = l->next) visit_node(l->node);
            visit_node(node->data.func_def.body);
            exit_scope();
            break;
        case NODE_PARAM: {
             DataType param_type = string_to_datatype(node->data.param.type_name);
             add_symbol(node->data.param.param_name, param_type, node);
             break;
        }
        case NODE_ASSIGN: {
            if (node->data.assign_expr.lvalue->type != NODE_IDENTIFIER) {
                semantic_error("O lado esquerdo de uma atribuição deve ser uma variável.", node->pos.line, node->pos.column);
            } else {
                const char* var_name = node->data.assign_expr.lvalue->data.identifier_name;
                Symbol* symbol = lookup_symbol(var_name);
                if (!symbol) {
                    char msg[256];
                    sprintf(msg, "Variável '%s' não declarada.", var_name);
                    semantic_error(msg, node->data.assign_expr.lvalue->pos.line, node->data.assign_expr.lvalue->pos.column);
                } else {
                    DataType lvalue_type = symbol->type;
                    DataType rvalue_type = get_expression_type(node->data.assign_expr.rvalue);
                    if (lvalue_type != rvalue_type && rvalue_type != TYPE_UNKNOWN) {
                        semantic_error("Tipos incompatíveis na atribuição.", node->pos.line, node->pos.column);
                    }
                }
            }
            break;
        }
        case NODE_IDENTIFIER:
            if (!lookup_symbol(node->data.identifier_name)) {
                char msg[256];
                sprintf(msg, "Identificador '%s' não declarado.", node->data.identifier_name);
                semantic_error(msg, node->pos.line, node->pos.column);
            }
            break;
        case NODE_BINARY_OP:
            visit_node(node->data.binary_op.left);
            visit_node(node->data.binary_op.right);
            DataType left_type = get_expression_type(node->data.binary_op.left);
            DataType right_type = get_expression_type(node->data.binary_op.right);
            if (left_type != TYPE_UNKNOWN && right_type != TYPE_UNKNOWN && left_type != right_type) {
                semantic_error("Tipos incompatíveis em operação binária.", node->pos.line, node->pos.column);
            }
            break;
        case NODE_FUNC_CALL: {
            Symbol* func_symbol = lookup_symbol(node->data.func_call.func_name);
            if (!func_symbol) {
                char msg[256];
                sprintf(msg, "Função '%s' não declarada.", node->data.func_call.func_name);
                semantic_error(msg, node->pos.line, node->pos.column);
            } else if (func_symbol->type != TYPE_FUNCTION) {
                char msg[256];
                sprintf(msg, "'%s' não é uma função.", node->data.func_call.func_name);
                semantic_error(msg, node->pos.line, node->pos.column);
            }
            for (ASTNodeList* l = node->data.func_call.args; l; l = l->next) visit_node(l->node);
            break;
        }
        case NODE_IF:
            visit_node(node->data.if_stmt.condition);
            visit_node(node->data.if_stmt.if_body);
            if (node->data.if_stmt.else_body) visit_node(node->data.if_stmt.else_body);
            break;
        case NODE_RETURN:
            if (node->data.return_stmt.return_value) visit_node(node->data.return_stmt.return_value);
            break;
        case NODE_UNARY_OP:
            visit_node(node->data.unary_op.operand);
            break;
        default:
            break;
    }
}

static DataType get_expression_type(ASTNode* node) {
    if (!node) return TYPE_UNKNOWN;
    switch (node->type) {
        case NODE_INT_LITERAL: return TYPE_INT;
        case NODE_FLOAT_LITERAL: return TYPE_FLOAT;
        case NODE_CHAR_LITERAL: return TYPE_CHAR;
        case NODE_IDENTIFIER: {
            Symbol* symbol = lookup_symbol(node->data.identifier_name);
            return symbol ? symbol->type : TYPE_UNKNOWN;
        }
        case NODE_BINARY_OP:
            return get_expression_type(node->data.binary_op.left);
        case NODE_FUNC_CALL:
            return TYPE_INT;
        default:
            return TYPE_UNKNOWN;
    }
}
