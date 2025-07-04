#ifndef OTIMIZADOR_H
#define OTIMIZADOR_H

#include "ast.h" // <<< CORREÇÃO: Adicionada a inclusão de ast.h

/**
 * @brief Otimiza a Árvore Sintática Abstrata (AST) fornecida.
 *
 * A principal otimização realizada é o "Constant Folding", onde expressões
 * com valores constantes são calculadas em tempo de compilação e
 * substituídas pelo seu resultado. A otimização é feita in-place,
 * modificando a própria árvore.
 *
 * @param node O nó raiz da AST a ser otimizada.
 */
void optimize_ast(ASTNode* node);

#endif // OTIMIZADOR_H