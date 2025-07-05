#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tabela_simbolos.h"

#define TABLE_SIZE 101
static Symbol* symbol_table[TABLE_SIZE];
static int current_scope_level = 0;

static void populate_builtins();

static unsigned long hash_function(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % TABLE_SIZE;
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

void init_symbol_table() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        symbol_table[i] = NULL;
    }
    current_scope_level = 0;
    populate_builtins();
}

void enter_scope() {
    current_scope_level++;
    printf("INFO (Tabela de Símbolos): Entrando no escopo, nível %d\n", current_scope_level);
}

void exit_scope() {
    printf("INFO (Tabela de Símbolos): Saindo do escopo, voltando para o nível %d\n", current_scope_level - 1);
    if (current_scope_level <= 0) return;

    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbol* current = symbol_table[i];
        Symbol* prev = NULL;
        while (current != NULL) {
            if (current->scope_level == current_scope_level) {
                Symbol* to_free = current;
                printf("INFO (Tabela de Símbolos): Removendo símbolo '%s' do escopo %d\n", to_free->name, current_scope_level);
                if (prev == NULL) {
                    symbol_table[i] = current->next;
                } else {
                    prev->next = current->next;
                }
                current = current->next;
                if(to_free->name) free(to_free->name);
                free(to_free);
            } else {
                prev = current;
                current = current->next;
            }
        }
    }
    current_scope_level--;
}

void add_symbol(const char* name, DataType type, ASTNode* node) {
    // <<< CORREÇÃO: A verificação de erro foi movida para o analisador semântico >>>
    // Apenas adiciona o símbolo
    
    printf("INFO (Tabela de Símbolos): Adicionando símbolo '%s' (tipo: %s) ao escopo %d\n", name, datatype_to_string(type), current_scope_level);

    unsigned long index = hash_function(name);
    Symbol* new_symbol = (Symbol*)malloc(sizeof(Symbol));
    if(!new_symbol){
        fprintf(stderr, "Erro de Memória: falha ao alocar memória para novo símbolo.\n");
        exit(EXIT_FAILURE);
    }

    new_symbol->name = safe_strdup(name);
    new_symbol->type = type;
    new_symbol->scope_level = current_scope_level;
    new_symbol->node = node;
    new_symbol->next = symbol_table[index];
    symbol_table[index] = new_symbol;
}

Symbol* lookup_symbol(const char* name) {
    unsigned long index = hash_function(name);
    Symbol* current = symbol_table[index];
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

Symbol* lookup_symbol_in_current_scope(const char* name) {
    unsigned long index = hash_function(name);
    Symbol* current = symbol_table[index];
    while (current != NULL) {
        if (strcmp(current->name, name) == 0 && current->scope_level == current_scope_level) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

DataType string_to_datatype(const char* type_str) {
    if (strcmp(type_str, "int") == 0) return TYPE_INT;
    if (strcmp(type_str, "float") == 0) return TYPE_FLOAT;
    if (strcmp(type_str, "char") == 0) return TYPE_CHAR;
    if (strcmp(type_str, "void") == 0) return TYPE_VOID;
    return TYPE_UNKNOWN;
}

const char* datatype_to_string(DataType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_CHAR: return "char";
        case TYPE_VOID: return "void";
        case TYPE_FUNCTION: return "function";
        default: return "unknown";
    }
}

static void populate_builtins() {
    add_symbol("print", TYPE_FUNCTION, NULL);
}
