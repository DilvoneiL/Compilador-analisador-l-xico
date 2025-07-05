#ifndef ANALISADOR_SEMANTICO_H
#define ANALISADOR_SEMANTICO_H

#include "ast.h"

/**
 * @brief Inicia o processo de análise semântica na AST.
 */
void analyze_semantics(ASTNode* root);

/**
 * @brief Obtém o número total de erros semânticos encontrados.
 * @return O número de erros.
 */
int get_semantic_error_count();

#endif // ANALISADOR_SEMANTICO_H
