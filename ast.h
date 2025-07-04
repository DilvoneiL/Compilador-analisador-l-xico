#ifndef AST_H
#define AST_H

#include "analisador.h" // Para Position

// Tipos de nós da AST
typedef enum {
    NODE_PROGRAM,
    NODE_VAR_DECL,
    NODE_FUNC_DEF,
    NODE_MAIN_DEF,
    NODE_PARAM,
    NODE_BLOCK,
    NODE_IF,
    NODE_FOR,
    NODE_RETURN,
    NODE_ASSIGN,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_FUNC_CALL,
    NODE_IDENTIFIER,
    NODE_INT_LITERAL,
    NODE_FLOAT_LITERAL,
    NODE_STRING_LITERAL,
    NODE_CHAR_LITERAL
} NodeType;

// Estrutura para uma lista de nós (usada para parâmetros, argumentos, statements)
typedef struct ASTNodeList {
    struct ASTNode* node;
    struct ASTNodeList* next;
} ASTNodeList;

// A estrutura principal de um nó da AST
typedef struct ASTNode {
    NodeType type;
    Position pos; // Linha e coluna para relatórios de erro

    union {
        // Programa: lista de declarações globais, funções, e o main
        struct { ASTNodeList* declarations; } program;

        // Declaração de variável: int x; ou int x = 5;
        struct {
            char* type_name;
            char* var_name;
            struct ASTNode* initial_value;
        } var_decl;
        
        // Definição de função: fun nome(params) { corpo }
        struct {
            char* func_name;
            ASTNodeList* params;
            struct ASTNode* body;
        } func_def;

        // Bloco main: main { corpo }
        struct { struct ASTNode* body; } main_def;

        // Parâmetro de função: int x
        struct { char* type_name; char* param_name; } param;

        // Bloco de código: { statements }
        struct { ASTNodeList* statements; } block;

        // Comando if: if (cond) { corpo_if } else { corpo_else }
        struct {
            struct ASTNode* condition;
            struct ASTNode* if_body;
            struct ASTNode* else_body; // Pode ser NULL
        } if_stmt;
        
        // Comando for: for(init; cond; inc) { corpo }
        struct {
            struct ASTNode* init;
            struct ASTNode* condition;
            struct ASTNode* increment;
            struct ASTNode* body;
        } for_stmt;
        
        // Comando return: return expressao;
        struct { struct ASTNode* return_value; } return_stmt;

        // Atribuição: variavel = expressao;
        struct {
            struct ASTNode* lvalue; // Lado esquerdo (deve ser identificador)
            struct ASTNode* rvalue; // Lado direito (expressão)
        } assign_expr;

        // Operação binária: a + b
        struct {
            char* op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary_op;

        // Operação unária: -a ou !a
        struct {
            char* op;
            struct ASTNode* operand;
        } unary_op;

        // Chamada de função: nome(args)
        struct {
            char* func_name;
            ASTNodeList* args;
        } func_call;

        // Literais e identificadores
        char* identifier_name;
        int int_literal;
        float float_literal;
        char* string_literal;
        char char_literal;
    } data;

} ASTNode;

// Funções para criar nós e listas
ASTNode* create_node(NodeType type, Position pos);
ASTNodeList* create_node_list(ASTNode* node);
ASTNodeList* append_node_list(ASTNodeList* list, ASTNode* node);
void free_ast(ASTNode* node);
void print_ast(ASTNode* node, int indent);

#endif // AST_H
