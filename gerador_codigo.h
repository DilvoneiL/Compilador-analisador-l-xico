#ifndef GERADOR_CODIGO_H
#define GERADOR_CODIGO_H

#include "ast.h" // <<< CORREÇÃO: Adicionada a inclusão de ast.h

/**
 * @brief Gera o código-alvo a partir da Árvore Sintática Abstrata.
 *
 * Esta implementação funciona como um "transpilador", traduzindo o código
 * da linguagem customizada para a linguagem C. A função percorre a AST
 * e escreve o código C equivalente no arquivo de saída.
 *
 * @param root O nó raiz da AST (preferencialmente já otimizada).
 * @param output_filename O nome do arquivo onde o código C será salvo (ex: "output.c").
 */
void generate_code(ASTNode* root, const char* output_filename);

#endif // GERADOR_CODIGO_H
