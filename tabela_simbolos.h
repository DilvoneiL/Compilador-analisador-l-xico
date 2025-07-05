#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

#include "ast.h"

// <<< MODIFICAÇÃO: Adicionado TYPE_STRING >>>
typedef enum {
    TYPE_INT, TYPE_FLOAT, TYPE_CHAR, TYPE_STRING, TYPE_VOID, TYPE_FUNCTION, TYPE_UNKNOWN
} DataType;

typedef struct Symbol {
    char* name;
    DataType type;
    int scope_level;
    ASTNode* node;
    struct Symbol* next;
} Symbol;

void init_symbol_table();
void enter_scope();
void exit_scope();
void add_symbol(const char* name, DataType type, ASTNode* node);
Symbol* lookup_symbol(const char* name);
Symbol* lookup_symbol_in_current_scope(const char* name);
DataType string_to_datatype(const char* type_str);
const char* datatype_to_string(DataType type);

#endif // TABELA_SIMBOLOS_H
