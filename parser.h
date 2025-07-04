#ifndef PARSER_H
#define PARSER_H

#include "analisador.h"
#include "ast.h"

ASTNode* parse_program(const char* source_code);

#endif // PARSER_H