#ifndef ANALISADOR_SEMANTICO_H
#define ANALISADOR_SEMANTICO_H

#include "ast.h" // <<< CORREÇÃO: Adicionada a inclusão de ast.h

/**
 * @brief Inicia o processo de análise semântica na AST.
 *
 * Esta função percorre a árvore, preenche a tabela de símbolos e
 * verifica a correção semântica do programa, como declarações,
 * tipos e escopos. O programa será encerrado se um erro for encontrado.
 *
 * @param root O nó raiz da Árvore Sintática Abstrata.
 */
void analyze_semantics(ASTNode* root);

#endif // ANALISADOR_SEMANTICO_H
