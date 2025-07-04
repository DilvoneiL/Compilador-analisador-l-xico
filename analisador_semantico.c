#include "analisador_semantico.h"
#include "tabela_simbolos.h"
#include <stdio.h>
#include <stdlib.h>

// --- Protótipos de Funções Estáticas ---
static void visit_node(ASTNode* node);
static DataType get_expression_type(ASTNode* node);

// Função auxiliar para converter um tipo de nó em string (para depuração)
static const char* node_type_to_string(NodeType type) {
    switch (type) {
        case NODE_PROGRAM: return "Program";
        case NODE_VAR_DECL: return "VarDecl";
        case NODE_FUNC_DEF: return "FuncDef";
        case NODE_MAIN_DEF: return "MainDef";
        case NODE_PARAM: return "Param";
        case NODE_BLOCK: return "Block";
        case NODE_IF: return "If";
        case NODE_FOR: return "For";
        case NODE_RETURN: return "Return";
        case NODE_ASSIGN: return "Assign";
        case NODE_BINARY_OP: return "BinaryOp";
        case NODE_UNARY_OP: return "UnaryOp";
        case NODE_FUNC_CALL: return "FuncCall";
        case NODE_IDENTIFIER: return "Identifier";
        case NODE_INT_LITERAL: return "IntLiteral";
        case NODE_FLOAT_LITERAL: return "FloatLiteral";
        case NODE_STRING_LITERAL: return "StringLiteral";
        case NODE_CHAR_LITERAL: return "CharLiteral";
        default: return "Unknown Node Type";
    }
}

// --- Implementação ---

void analyze_semantics(ASTNode* root) {
    init_symbol_table();
    visit_node(root);
}

static void visit_node(ASTNode* node) {
    if (!node) return;

    // <<< TESTE DE DEPURAÇÃO >>>
    // (Pode remover estes printf's depois que tudo funcionar)
    // printf("DEBUG: Visitando nó do tipo: %s (Linha: %d)\n", node_type_to_string(node->type), node->pos.line);

    switch (node->type) {
        case NODE_PROGRAM:
            enter_scope();
            for (ASTNodeList* l = node->data.program.declarations; l; l = l->next) visit_node(l->node);
            exit_scope();
            break;
        
        // <<< CORREÇÃO: Casos separados para MainDef e Block >>>
        case NODE_MAIN_DEF:
            // O nó MainDef simplesmente delega a visitação para seu corpo (que é um Block)
            visit_node(node->data.main_def.body);
            break;
        
        case NODE_BLOCK:
            enter_scope();
            for (ASTNodeList* l = node->data.block.statements; l; l = l->next) visit_node(l->node);
            exit_scope();
            break;

        case NODE_VAR_DECL: {
            DataType type = string_to_datatype(node->data.var_decl.type_name);
            add_symbol(node->data.var_decl.var_name, type, node);
            if (node->data.var_decl.initial_value) {
                DataType value_type = get_expression_type(node->data.var_decl.initial_value);
                if (type != value_type) {
                    fprintf(stderr, "Erro Semântico (Linha %d): Tipos incompatíveis na inicialização de '%s'.\n", node->pos.line, node->data.var_decl.var_name);
                    exit(EXIT_FAILURE);
                }
            }
            break;
        }
        case NODE_FUNC_DEF:
            add_symbol(node->data.func_def.func_name, TYPE_FUNCTION, node);
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
                fprintf(stderr, "Erro Semântico (Linha %d): O lado esquerdo de uma atribuição deve ser uma variável.\n", node->pos.line);
                exit(EXIT_FAILURE);
            }
            const char* var_name = node->data.assign_expr.lvalue->data.identifier_name;
            Symbol* symbol = lookup_symbol(var_name);
            if (!symbol) {
                fprintf(stderr, "Erro Semântico (Linha %d): Variável '%s' não declarada.\n", node->data.assign_expr.lvalue->pos.line, var_name);
                exit(EXIT_FAILURE);
            }
            DataType lvalue_type = symbol->type;
            DataType rvalue_type = get_expression_type(node->data.assign_expr.rvalue);
            if (lvalue_type != rvalue_type) {
                fprintf(stderr, "Erro Semântico (Linha %d): Tipos incompatíveis na atribuição da variável '%s'.\n", node->pos.line, var_name);
                exit(EXIT_FAILURE);
            }
            break;
        }
        case NODE_IDENTIFIER:
            if (!lookup_symbol(node->data.identifier_name)) {
                fprintf(stderr, "Erro Semântico (Linha %d): Identificador '%s' não declarado.\n", node->pos.line, node->data.identifier_name);
                exit(EXIT_FAILURE);
            }
            break;
        case NODE_BINARY_OP:
            visit_node(node->data.binary_op.left);
            visit_node(node->data.binary_op.right);
            DataType left_type = get_expression_type(node->data.binary_op.left);
            DataType right_type = get_expression_type(node->data.binary_op.right);
            if (left_type != right_type) {
                fprintf(stderr, "Erro Semântico (Linha %d): Tipos incompatíveis na operação binária '%s'.\n", node->pos.line, node->data.binary_op.op);
                exit(EXIT_FAILURE);
            }
            break;
        case NODE_FUNC_CALL: {
            Symbol* func_symbol = lookup_symbol(node->data.func_call.func_name);
            if (!func_symbol) {
                fprintf(stderr, "Erro Semântico (Linha %d): Função '%s' não declarada.\n", node->pos.line, node->data.func_call.func_name);
                exit(EXIT_FAILURE);
            }
            if (func_symbol->type != TYPE_FUNCTION) {
                fprintf(stderr, "Erro Semântico (Linha %d): O identificador '%s' não é uma função.\n", node->pos.line, node->data.func_call.func_name);
                exit(EXIT_FAILURE);
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
        case NODE_INT_LITERAL:
        case NODE_FLOAT_LITERAL:
        case NODE_CHAR_LITERAL:
        case NODE_STRING_LITERAL:
        case NODE_FOR: // Não faz nada para nós não tratados para evitar erros.
            break; 
        default:
             // Não faz nada para nós desconhecidos.
            break;
    }
}

static DataType get_expression_type(ASTNode* node) {
    if (!node) return TYPE_UNKNOWN;

    // <<< TESTE DE DEPURAÇÃO >>>
    // printf("DEBUG: Obtendo tipo da expressão: %s (Linha: %d)\n", node_type_to_string(node->type), node->pos.line);

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
            return TYPE_INT; // Limitação: assumindo que funções retornam int
        default:
            return TYPE_UNKNOWN;
    }
}
