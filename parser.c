// Define _DEFAULT_SOURCE para habilitar funções de extensão POSIX como strdup
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "analisador.h"
#include "ast.h"

// --- Variáveis de estado do Parser ---
static Token current_token;
static const char* source_code_ptr;
static int current_parser_index;
static int main_block_found_flag;

// --- Protótipos de Funções ---
static void advance_and_skip_comments();
static int token_is(TokenType type, const char* lexeme);
static void eat(TokenType type, const char* expected_lexeme);
static void syntax_error(const char* message);
static char* safe_strdup(const char* s);
static ASTNode* parse_expression();
static ASTNode* parse_primary_expression();
static ASTNode* parse_top_level_declaration();
static ASTNode* parse_variable_declaration();
static ASTNode* parse_standard_function_definition();
static ASTNodeList* parse_parameter_list();
static ASTNode* parse_parameter();
static ASTNode* parse_main_function_definition();
static ASTNodeList* parse_statement_list();
static ASTNode* parse_statement();
static ASTNode* parse_expression_statement();
static ASTNode* parse_if_statement();
static ASTNode* parse_for_statement();
static ASTNode* parse_return_statement();
static ASTNode* parse_block_statement();
static ASTNode* parse_assignment_expression();
static ASTNode* parse_logical_or_expression();
static ASTNode* parse_logical_and_expression();
static ASTNode* parse_equality_expression();
static ASTNode* parse_relational_expression();
static ASTNode* parse_additive_expression();
static ASTNode* parse_multiplicative_expression();
static ASTNode* parse_unary_expression();
static ASTNodeList* parse_argument_list();

// (Implementação de create_node, free_ast, etc. permanece a mesma)
ASTNode* create_node(NodeType type, Position pos) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Falha ao alocar memória para o nó da AST\n");
        exit(EXIT_FAILURE);
    }
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    node->pos = pos;
    return node;
}

ASTNodeList* create_node_list(ASTNode* node) {
    ASTNodeList* list = (ASTNodeList*)malloc(sizeof(ASTNodeList));
    if (!list) {
        fprintf(stderr, "Falha ao alocar memória para a lista de nós da AST\n");
        exit(EXIT_FAILURE);
    }
    list->node = node;
    list->next = NULL;
    return list;
}

ASTNodeList* append_node_list(ASTNodeList* list, ASTNode* node) {
    if (!list) return create_node_list(node);
    ASTNodeList* current = list;
    while (current->next != NULL) current = current->next;
    current->next = create_node_list(node);
    return list;
}

void free_ast(ASTNode* node) {
    if (!node) return;
    switch (node->type) {
        case NODE_PROGRAM: {
            ASTNodeList* current = node->data.program.declarations;
            while (current) {
                ASTNodeList* next = current->next;
                free_ast(current->node);
                free(current);
                current = next;
            }
            break;
        }
        case NODE_VAR_DECL:
            if(node->data.var_decl.type_name) free(node->data.var_decl.type_name);
            if(node->data.var_decl.var_name) free(node->data.var_decl.var_name);
            free_ast(node->data.var_decl.initial_value);
            break;
        case NODE_FUNC_DEF:
            if(node->data.func_def.func_name) free(node->data.func_def.func_name);
            ASTNodeList* params = node->data.func_def.params;
            while (params) {
                ASTNodeList* next = params->next;
                free_ast(params->node);
                free(params);
                params = next;
            }
            free_ast(node->data.func_def.body);
            break;
        case NODE_MAIN_DEF:
            free_ast(node->data.main_def.body);
            break;
        case NODE_PARAM:
            if(node->data.param.type_name) free(node->data.param.type_name);
            if(node->data.param.param_name) free(node->data.param.param_name);
            break;
        case NODE_BLOCK: {
            ASTNodeList* current = node->data.block.statements;
            while (current) {
                ASTNodeList* next = current->next;
                free_ast(current->node);
                free(current);
                current = next;
            }
            break;
        }
        case NODE_IF:
            free_ast(node->data.if_stmt.condition);
            free_ast(node->data.if_stmt.if_body);
            free_ast(node->data.if_stmt.else_body);
            break;
        case NODE_RETURN:
            free_ast(node->data.return_stmt.return_value);
            break;
        case NODE_ASSIGN:
            free_ast(node->data.assign_expr.lvalue);
            free_ast(node->data.assign_expr.rvalue);
            break;
        case NODE_BINARY_OP:
            if(node->data.binary_op.op) free(node->data.binary_op.op);
            free_ast(node->data.binary_op.left);
            free_ast(node->data.binary_op.right);
            break;
        case NODE_UNARY_OP:
            if(node->data.unary_op.op) free(node->data.unary_op.op);
            free_ast(node->data.unary_op.operand);
            break;
        case NODE_FUNC_CALL:
            if(node->data.func_call.func_name) free(node->data.func_call.func_name);
            ASTNodeList* args = node->data.func_call.args;
            while (args) {
                ASTNodeList* next = args->next;
                free_ast(args->node);
                free(args);
                args = next;
            }
            break;
        case NODE_IDENTIFIER:
            if(node->data.identifier_name) free(node->data.identifier_name);
            break;
        case NODE_STRING_LITERAL:
            if (node->data.string_literal) free(node->data.string_literal);
            break;
        case NODE_INT_LITERAL:
        case NODE_FLOAT_LITERAL:
        case NODE_CHAR_LITERAL:
        case NODE_FOR:
            break;
    }
    free(node);
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; ++i) printf("  ");

    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program\n");
            for (ASTNodeList* l = node->data.program.declarations; l; l = l->next) {
                print_ast(l->node, indent + 1);
            }
            break;
        case NODE_VAR_DECL:
            printf("VarDecl: %s %s\n", node->data.var_decl.type_name, node->data.var_decl.var_name);
            if (node->data.var_decl.initial_value) {
                print_ast(node->data.var_decl.initial_value, indent + 1);
            }
            break;
        case NODE_FUNC_DEF:
            printf("FuncDef: fun %s\n", node->data.func_def.func_name);
            for (ASTNodeList* l = node->data.func_def.params; l; l = l->next) {
                print_ast(l->node, indent + 1);
            }
            print_ast(node->data.func_def.body, indent + 1);
            break;
        case NODE_MAIN_DEF:
            printf("MainDef\n");
            print_ast(node->data.main_def.body, indent + 1);
            break;
        case NODE_PARAM:
            printf("Param: %s %s\n", node->data.param.type_name, node->data.param.param_name);
            break;
        case NODE_BLOCK:
            printf("Block\n");
            for (ASTNodeList* l = node->data.block.statements; l; l = l->next) {
                print_ast(l->node, indent + 1);
            }
            break;
        case NODE_IF:
            printf("If\n");
            print_ast(node->data.if_stmt.condition, indent + 1);
            print_ast(node->data.if_stmt.if_body, indent + 1);
            if (node->data.if_stmt.else_body) {
                print_ast(node->data.if_stmt.else_body, indent + 1);
            }
            break;
        case NODE_RETURN:
            printf("Return\n");
            print_ast(node->data.return_stmt.return_value, indent + 1);
            break;
        case NODE_ASSIGN:
            printf("Assign\n");
            print_ast(node->data.assign_expr.lvalue, indent + 1);
            print_ast(node->data.assign_expr.rvalue, indent + 1);
            break;
        case NODE_BINARY_OP:
            printf("BinaryOp: %s\n", node->data.binary_op.op);
            print_ast(node->data.binary_op.left, indent + 1);
            print_ast(node->data.binary_op.right, indent + 1);
            break;
        case NODE_UNARY_OP:
            printf("UnaryOp: %s\n", node->data.unary_op.op);
            print_ast(node->data.unary_op.operand, indent + 1);
            break;
        case NODE_FUNC_CALL:
            printf("FuncCall: %s\n", node->data.func_call.func_name);
            for (ASTNodeList* l = node->data.func_call.args; l; l = l->next) {
                print_ast(l->node, indent + 1);
            }
            break;
        case NODE_IDENTIFIER:
            printf("Identifier: %s\n", node->data.identifier_name);
            break;
        case NODE_INT_LITERAL:
            printf("Int: %d\n", node->data.int_literal);
            break;
        case NODE_STRING_LITERAL:
             printf("String: %s\n", node->data.string_literal);
             break;
        default:
            printf("Nó Desconhecido\n");
            break;
    }
}


// --- Funções de Controlo do Parser ---
static void advance_and_skip_comments() {
    do {
        current_token = next_token(source_code_ptr, &current_parser_index);
    } while (current_token.type == TOKEN_COMMENT);
}

static int is_type_specifier(Token t) {
    if (t.type == TOKEN_KEYWORD) {
        return strcmp(t.lexeme, "int") == 0 ||
               strcmp(t.lexeme, "float") == 0 ||
               strcmp(t.lexeme, "char") == 0;
    }
    return 0;
}

static int token_is(TokenType type, const char* lexeme) {
    if (current_token.type != type) return 0;
    if (lexeme && strcmp(current_token.lexeme, lexeme) != 0) return 0;
    return 1;
}

static void eat(TokenType type, const char* expected_lexeme) {
    if (token_is(type, expected_lexeme)) {
        advance_and_skip_comments();
    } else {
        char error_msg[256];
        if (expected_lexeme) {
            sprintf(error_msg, "Esperava '%s' (tipo %s), mas encontrou '%s' (tipo %s).",
                    expected_lexeme, token_type_to_string(type), current_token.lexeme, token_type_to_string(current_token.type));
        } else {
            sprintf(error_msg, "Esperava tipo %s, mas encontrou tipo %s ('%s').",
                    token_type_to_string(type), token_type_to_string(current_token.type), current_token.lexeme);
        }
        syntax_error(error_msg);
    }
}

static void syntax_error(const char* message) {
    fprintf(stderr, "\nErro Sintático (Linha %d, Coluna %d): %s\n",
            current_token.line, current_token.column, message);
    exit(EXIT_FAILURE);
}

static char* safe_strdup(const char* s) {
    if (!s) return NULL;
    char* new_s = strdup(s);
    if (!new_s) {
        fprintf(stderr, "Erro de Memória: falha ao alocar memória.\n");
        exit(EXIT_FAILURE);
    }
    return new_s;
}

// --- Implementação das Funções de Parsing ---

ASTNode* parse_program(const char* source) {
    source_code_ptr = source;
    current_parser_index = 0;
    current_pos.line = 1;
    current_pos.column = 1;
    main_block_found_flag = 0;

    advance_and_skip_comments();

    ASTNode* program_node = create_node(NODE_PROGRAM, (Position){1, 1});
    program_node->data.program.declarations = NULL;

    while (!token_is(TOKEN_EOF, NULL)) {
        if (token_is(TOKEN_KEYWORD, "main")) {
            if (main_block_found_flag) {
                syntax_error("Múltiplos blocos 'main' definidos.");
            }
            ASTNode* main_node = parse_main_function_definition();
            program_node->data.program.declarations = append_node_list(program_node->data.program.declarations, main_node);
            main_block_found_flag = 1;
        } else if (is_type_specifier(current_token) || token_is(TOKEN_KEYWORD, "fun")) {
            if (main_block_found_flag) {
                syntax_error("Declaração encontrada após o bloco 'main'.");
            }
            ASTNode* top_level_decl = parse_top_level_declaration();
            program_node->data.program.declarations = append_node_list(program_node->data.program.declarations, top_level_decl);
        } else {
            syntax_error("Token inesperado no nível superior. Esperava uma declaração ou o bloco 'main'.");
        }
    }

    if (!main_block_found_flag) {
        syntax_error("Bloco 'main' obrigatório não encontrado.");
    }
    
    return program_node;
}

static ASTNode* parse_top_level_declaration() {
    if (token_is(TOKEN_KEYWORD, "fun")) {
        return parse_standard_function_definition();
    } else if (is_type_specifier(current_token)) {
        return parse_variable_declaration();
    }
    syntax_error("Esperava 'fun' ou um tipo ('int', 'float', 'char').");
    return NULL;
}

static ASTNode* parse_variable_declaration() {
    Position pos = { .line = current_token.line, .column = current_token.column };
    char* type_name = safe_strdup(current_token.lexeme);
    eat(TOKEN_KEYWORD, NULL);

    if (current_token.type != TOKEN_IDENTIFIER) {
        free(type_name);
        syntax_error("Esperava um identificador na declaração de variável.");
    }
    char* var_name = safe_strdup(current_token.lexeme);
    eat(TOKEN_IDENTIFIER, NULL);

    ASTNode* node = create_node(NODE_VAR_DECL, pos);
    node->data.var_decl.type_name = type_name;
    node->data.var_decl.var_name = var_name;
    node->data.var_decl.initial_value = NULL;

    if (token_is(TOKEN_OPERATOR, "=")) {
        eat(TOKEN_OPERATOR, "=");
        node->data.var_decl.initial_value = parse_expression();
    }

    eat(TOKEN_DELIMITER, ";");
    return node;
}

static ASTNode* parse_standard_function_definition() {
    Position pos = { .line = current_token.line, .column = current_token.column };
    eat(TOKEN_KEYWORD, "fun");
    
    char* func_name = safe_strdup(current_token.lexeme);
    eat(TOKEN_IDENTIFIER, NULL);

    ASTNode* node = create_node(NODE_FUNC_DEF, pos);
    node->data.func_def.func_name = func_name;
    
    eat(TOKEN_DELIMITER, "(");
    if (!token_is(TOKEN_DELIMITER, ")")) {
        node->data.func_def.params = parse_parameter_list();
    } else {
        node->data.func_def.params = NULL;
    }
    eat(TOKEN_DELIMITER, ")");

    node->data.func_def.body = parse_block_statement();
    return node;
}

static ASTNodeList* parse_parameter_list() {
    ASTNodeList* list = create_node_list(parse_parameter());
    while (token_is(TOKEN_DELIMITER, ",")) {
        eat(TOKEN_DELIMITER, ",");
        list = append_node_list(list, parse_parameter());
    }
    return list;
}

static ASTNode* parse_parameter() {
    if (!is_type_specifier(current_token)) {
        syntax_error("Esperava um tipo para o parâmetro.");
    }
    Position pos = { .line = current_token.line, .column = current_token.column };
    char* type_name = safe_strdup(current_token.lexeme);
    eat(TOKEN_KEYWORD, NULL);
    
    char* param_name = safe_strdup(current_token.lexeme);
    eat(TOKEN_IDENTIFIER, NULL);
    
    ASTNode* node = create_node(NODE_PARAM, pos);
    node->data.param.type_name = type_name;
    node->data.param.param_name = param_name;
    return node;
}

static ASTNode* parse_main_function_definition() {
    Position pos = { .line = current_token.line, .column = current_token.column };
    eat(TOKEN_KEYWORD, "main");
    ASTNode* node = create_node(NODE_MAIN_DEF, pos);
    node->data.main_def.body = parse_block_statement();
    return node;
}

static ASTNode* parse_block_statement() {
    Position pos = { .line = current_token.line, .column = current_token.column };
    eat(TOKEN_DELIMITER, "{");
    ASTNode* node = create_node(NODE_BLOCK, pos);
    node->data.block.statements = parse_statement_list();
    eat(TOKEN_DELIMITER, "}");
    return node;
}

static ASTNodeList* parse_statement_list() {
    ASTNodeList* list = NULL;
    while (!token_is(TOKEN_DELIMITER, "}") && !token_is(TOKEN_EOF, NULL)) {
        list = append_node_list(list, parse_statement());
    }
    return list;
}

static ASTNode* parse_statement() {
    if (is_type_specifier(current_token)) return parse_variable_declaration();
    if (token_is(TOKEN_KEYWORD, "if")) return parse_if_statement();
    if (token_is(TOKEN_KEYWORD, "for")) return parse_for_statement();
    if (token_is(TOKEN_KEYWORD, "return")) return parse_return_statement();
    if (token_is(TOKEN_DELIMITER, "{")) return parse_block_statement();
    return parse_expression_statement();
}

static ASTNode* parse_expression_statement() {
    ASTNode* expr = parse_expression();
    eat(TOKEN_DELIMITER, ";");
    return expr;
}

static ASTNode* parse_if_statement() {
    Position pos = { .line = current_token.line, .column = current_token.column };
    eat(TOKEN_KEYWORD, "if");
    eat(TOKEN_DELIMITER, "(");
    ASTNode* condition = parse_expression();
    eat(TOKEN_DELIMITER, ")");
    ASTNode* if_body = parse_statement();
    ASTNode* else_body = NULL;
    if (token_is(TOKEN_KEYWORD, "else")) {
        eat(TOKEN_KEYWORD, "else");
        else_body = parse_statement();
    }
    ASTNode* node = create_node(NODE_IF, pos);
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.if_body = if_body;
    node->data.if_stmt.else_body = else_body;
    return node;
}

static ASTNode* parse_return_statement() {
    Position pos = { .line = current_token.line, .column = current_token.column };
    eat(TOKEN_KEYWORD, "return");
    ASTNode* node = create_node(NODE_RETURN, pos);
    if (!token_is(TOKEN_DELIMITER, ";")) {
        node->data.return_stmt.return_value = parse_expression();
    } else {
        node->data.return_stmt.return_value = NULL;
    }
    eat(TOKEN_DELIMITER, ";");
    return node;
}

static ASTNode* parse_for_statement() { 
    syntax_error("O parsing do comando 'for' ainda não foi implementado.");
    return NULL; 
}

static ASTNode* parse_expression() {
    return parse_assignment_expression();
}

static ASTNode* parse_assignment_expression() {
    ASTNode* left = parse_logical_or_expression();
    if (token_is(TOKEN_OPERATOR, "=")) {
        Position pos = { .line = current_token.line, .column = current_token.column };
        eat(TOKEN_OPERATOR, "=");
        ASTNode* right = parse_assignment_expression();
        if (left->type != NODE_IDENTIFIER) {
            syntax_error("O lado esquerdo de uma atribuição deve ser um identificador.");
        }
        ASTNode* node = create_node(NODE_ASSIGN, pos);
        node->data.assign_expr.lvalue = left;
        node->data.assign_expr.rvalue = right;
        return node;
    }
    return left;
}

static ASTNode* parse_logical_or_expression() {
    ASTNode* node = parse_logical_and_expression();
    while (token_is(TOKEN_OPERATOR, "||")) {
        Position pos = { .line = current_token.line, .column = current_token.column };
        char* op = safe_strdup(current_token.lexeme);
        eat(TOKEN_OPERATOR, "||");
        ASTNode* right = parse_logical_and_expression();
        ASTNode* new_node = create_node(NODE_BINARY_OP, pos);
        new_node->data.binary_op.op = op;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = right;
        node = new_node;
    }
    return node;
}

static ASTNode* parse_logical_and_expression() {
    ASTNode* node = parse_equality_expression();
    while (token_is(TOKEN_OPERATOR, "&&")) {
        Position pos = { .line = current_token.line, .column = current_token.column };
        char* op = safe_strdup(current_token.lexeme);
        eat(TOKEN_OPERATOR, "&&");
        ASTNode* right = parse_equality_expression();
        ASTNode* new_node = create_node(NODE_BINARY_OP, pos);
        new_node->data.binary_op.op = op;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = right;
        node = new_node;
    }
    return node;
}

static ASTNode* parse_equality_expression() {
    ASTNode* node = parse_relational_expression();
    while (token_is(TOKEN_OPERATOR, "==") || token_is(TOKEN_OPERATOR, "!=")) {
        Position pos = { .line = current_token.line, .column = current_token.column };
        char* op = safe_strdup(current_token.lexeme);
        eat(TOKEN_OPERATOR, NULL);
        ASTNode* right = parse_relational_expression();
        ASTNode* new_node = create_node(NODE_BINARY_OP, pos);
        new_node->data.binary_op.op = op;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = right;
        node = new_node;
    }
    return node;
}

static ASTNode* parse_relational_expression() {
    ASTNode* node = parse_additive_expression();
    while (token_is(TOKEN_OPERATOR, "<") || token_is(TOKEN_OPERATOR, ">") ||
           token_is(TOKEN_OPERATOR, "<=") || token_is(TOKEN_OPERATOR, ">=")) {
        Position pos = { .line = current_token.line, .column = current_token.column };
        char* op = safe_strdup(current_token.lexeme);
        eat(TOKEN_OPERATOR, NULL);
        ASTNode* right = parse_additive_expression();
        ASTNode* new_node = create_node(NODE_BINARY_OP, pos);
        new_node->data.binary_op.op = op;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = right;
        node = new_node;
    }
    return node;
}

static ASTNode* parse_additive_expression() {
    ASTNode* node = parse_multiplicative_expression();
    while (token_is(TOKEN_OPERATOR, "+") || token_is(TOKEN_OPERATOR, "-")) {
        Position pos = { .line = current_token.line, .column = current_token.column };
        char* op = safe_strdup(current_token.lexeme);
        eat(TOKEN_OPERATOR, NULL);
        ASTNode* right = parse_multiplicative_expression();
        ASTNode* new_node = create_node(NODE_BINARY_OP, pos);
        new_node->data.binary_op.op = op;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = right;
        node = new_node;
    }
    return node;
}

static ASTNode* parse_multiplicative_expression() {
    ASTNode* node = parse_unary_expression();
    while (token_is(TOKEN_OPERATOR, "*") || token_is(TOKEN_OPERATOR, "/")) {
        Position pos = { .line = current_token.line, .column = current_token.column };
        char* op = safe_strdup(current_token.lexeme);
        eat(TOKEN_OPERATOR, NULL);
        ASTNode* right = parse_unary_expression();
        ASTNode* new_node = create_node(NODE_BINARY_OP, pos);
        new_node->data.binary_op.op = op;
        new_node->data.binary_op.left = node;
        new_node->data.binary_op.right = right;
        node = new_node;
    }
    return node;
}

static ASTNode* parse_unary_expression() {
    if (token_is(TOKEN_OPERATOR, "-") || token_is(TOKEN_OPERATOR, "!")) {
        Position pos = { .line = current_token.line, .column = current_token.column };
        char* op = safe_strdup(current_token.lexeme);
        eat(TOKEN_OPERATOR, NULL);
        ASTNode* operand = parse_unary_expression();
        ASTNode* node = create_node(NODE_UNARY_OP, pos);
        node->data.unary_op.op = op;
        node->data.unary_op.operand = operand;
        return node;
    }
    return parse_primary_expression();
}

// <<< FUNÇÃO MODIFICADA >>>
static ASTNode* parse_primary_expression() {
    Position pos = { .line = current_token.line, .column = current_token.column };
    if (token_is(TOKEN_INT, NULL)) {
        ASTNode* node = create_node(NODE_INT_LITERAL, pos);
        node->data.int_literal = atoi(current_token.lexeme);
        eat(TOKEN_INT, NULL);
        return node;
    }
    if (token_is(TOKEN_FLOAT, NULL)) {
        ASTNode* node = create_node(NODE_FLOAT_LITERAL, pos);
        node->data.float_literal = atof(current_token.lexeme);
        eat(TOKEN_FLOAT, NULL);
        return node;
    }
    if (token_is(TOKEN_STRING, NULL)) {
        ASTNode* node = create_node(NODE_STRING_LITERAL, pos);
        // Remove as aspas do início e do fim
        int len = strlen(current_token.lexeme);
        if (len > 1) {
            char* str_content = (char*)malloc(len - 1);
            strncpy(str_content, current_token.lexeme + 1, len - 2);
            str_content[len - 2] = '\0';
            node->data.string_literal = str_content;
        } else {
            node->data.string_literal = safe_strdup(""); // String vazia
        }
        eat(TOKEN_STRING, NULL);
        return node;
    }
    if (token_is(TOKEN_IDENTIFIER, NULL)) {
        char* name = safe_strdup(current_token.lexeme);
        eat(TOKEN_IDENTIFIER, NULL);
        if (token_is(TOKEN_DELIMITER, "(")) {
            eat(TOKEN_DELIMITER, "(");
            ASTNode* node = create_node(NODE_FUNC_CALL, pos);
            node->data.func_call.func_name = name;
            if (!token_is(TOKEN_DELIMITER, ")")) {
                node->data.func_call.args = parse_argument_list();
            } else {
                node->data.func_call.args = NULL;
            }
            eat(TOKEN_DELIMITER, ")");
            return node;
        } else {
            ASTNode* node = create_node(NODE_IDENTIFIER, pos);
            node->data.identifier_name = name;
            return node;
        }
    }
    if (token_is(TOKEN_DELIMITER, "(")) {
        eat(TOKEN_DELIMITER, "(");
        ASTNode* node = parse_expression();
        eat(TOKEN_DELIMITER, ")");
        return node;
    }

    syntax_error("Token inesperado em uma expressão. Esperava literal, identificador ou '('.");
    return NULL;
}

static ASTNodeList* parse_argument_list() {
    ASTNodeList* list = create_node_list(parse_expression());
    while (token_is(TOKEN_DELIMITER, ",")) {
        eat(TOKEN_DELIMITER, ",");
        list = append_node_list(list, parse_expression());
    }
    return list;
}
