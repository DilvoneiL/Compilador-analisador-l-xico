#ifndef GERADOR_CODIGO_H
#define GERADOR_CODIGO_H

#include "ast.h"

/**
 * @brief Gera o código-alvo em Python a partir da Árvore Sintática Abstrata.
 *
 * Esta implementação funciona como um "transpilador", traduzindo o código
 * da linguagem customizada para um script Python. A função percorre a AST
 * e escreve o código Python equivalente no arquivo de saída.
 *
 * @param root O nó raiz da AST (preferencialmente já otimizada).
 * @param output_filename O nome do arquivo onde o código Python será salvo (ex: "output.py").
 */
void generate_code(ASTNode* root, const char* output_filename);

#endif // GERADOR_CODIGO_H
