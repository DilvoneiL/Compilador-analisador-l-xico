#include "otimizador.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h" // Incluído para free_ast

// --- Implementação ---

void optimize_ast(ASTNode* node) {
    if (!node) {
        return;
    }

    // --- Passo 1: Otimizar os filhos primeiro (travessia em pós-ordem) ---
    switch (node->type) {
        case NODE_PROGRAM:
            for (ASTNodeList* l = node->data.program.declarations; l; l = l->next) optimize_ast(l->node);
            break;
        case NODE_MAIN_DEF:
            optimize_ast(node->data.main_def.body);
            break;
        case NODE_BLOCK:
            for (ASTNodeList* l = node->data.block.statements; l; l = l->next) optimize_ast(l->node);
            break;
        case NODE_FUNC_DEF:
            optimize_ast(node->data.func_def.body);
            break;
        case NODE_IF:
            optimize_ast(node->data.if_stmt.condition);
            optimize_ast(node->data.if_stmt.if_body);
            if (node->data.if_stmt.else_body) optimize_ast(node->data.if_stmt.else_body);
            break;
        case NODE_FOR:
            optimize_ast(node->data.for_stmt.init);
            optimize_ast(node->data.for_stmt.condition);
            optimize_ast(node->data.for_stmt.increment);
            optimize_ast(node->data.for_stmt.body);
            break;
        case NODE_ASSIGN:
            optimize_ast(node->data.assign_expr.rvalue);
            break;
        case NODE_RETURN:
            optimize_ast(node->data.return_stmt.return_value);
            break;
        case NODE_UNARY_OP:
            optimize_ast(node->data.unary_op.operand);
            break;
        case NODE_FUNC_CALL:
            for (ASTNodeList* l = node->data.func_call.args; l; l = l->next) optimize_ast(l->node);
            break;
        case NODE_BINARY_OP:
            optimize_ast(node->data.binary_op.left);
            optimize_ast(node->data.binary_op.right);
            break;
        case NODE_VAR_DECL:
            if (node->data.var_decl.initial_value) optimize_ast(node->data.var_decl.initial_value);
            break;
        case NODE_PARAM:
        case NODE_IDENTIFIER:
        case NODE_INT_LITERAL:
        case NODE_FLOAT_LITERAL:
        case NODE_CHAR_LITERAL:
        case NODE_STRING_LITERAL:
            break;
    }

    // --- Passo 2: Tentar otimizar o nó atual ---
    if (node->type == NODE_BINARY_OP) {
        ASTNode* left = node->data.binary_op.left;
        ASTNode* right = node->data.binary_op.right;

        if (left && right && left->type == NODE_INT_LITERAL && right->type == NODE_INT_LITERAL) {
            int result = 0;
            const char* op = node->data.binary_op.op;

            if (strcmp(op, "+") == 0) result = left->data.int_literal + right->data.int_literal;
            else if (strcmp(op, "-") == 0) result = left->data.int_literal - right->data.int_literal;
            else if (strcmp(op, "*") == 0) result = left->data.int_literal * right->data.int_literal;
            else if (strcmp(op, "/") == 0) {
                if (right->data.int_literal == 0) return; // Evita otimização de divisão por zero
                result = left->data.int_literal / right->data.int_literal;
            } else {
                return; // Não otimiza outros operadores
            }
            
            printf("Otimização: Expressão '%d %s %d' na linha %d foi calculada como '%d'.\n",
                   left->data.int_literal, op, right->data.int_literal, node->pos.line, result);

            free(node->data.binary_op.op);
            free_ast(left);
            free_ast(right);

            node->type = NODE_INT_LITERAL;
            node->data.int_literal = result;
        }
    }
}
