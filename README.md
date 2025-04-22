# Analisador Léxico em C

## Autor

Dilvonei Alves Lacerda Junior  
Ano: 2025

---

## Especificações

Este programa é um analisador léxico simples implementado em linguagem C. Ele realiza a leitura de um código-fonte contido em um arquivo `.txt` e identifica os **tokens** presentes no texto, classificando-os nas seguintes categorias:

### Categorias de Tokens Reconhecidos

| Tipo de Token         | Exemplos                                  |
|-----------------------|-------------------------------------------|
| `TOKEN_KEYWORD`       | `if`, `else`, `while`, `for`, `return`, `int`, `float` |
| `TOKEN_IDENTIFIER`    | Nomes de variáveis e funções              |
| `TOKEN_INT`           | Números inteiros (`123`, `-456`)          |
| `TOKEN_FLOAT`         | Números decimais (`3.14`, `-0.001`)       |
| `TOKEN_STRING`        | Strings entre aspas (`"texto"`)           |
| `TOKEN_CHAR`          | Caracteres entre aspas simples (`'a'`)    |
| `TOKEN_OPERATOR`      | `+`, `-`, `*`, `/`, `=`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`, `++`, `--` |
| `TOKEN_DELIMITER`     | `(`, `)`, `{`, `}`, `;`, `,`             |
| `TOKEN_COMMENT`       | `// comentário`, `/* comentário */`       |
| `TOKEN_EOF`           | Fim do arquivo                            |
| `TOKEN_UNKNOWN`       | Caracteres não reconhecidos               |

---

## Limitações Conhecidas

### Strings e Caracteres
- Sequências de escape (`\n`, `\t`, `\"`, etc.) não são totalmente suportadas
- Caracteres multibyte/não-ASCII podem causar problemas
- Strings que quebram linhas não são tratadas corretamente

### Números
- Não suporta notação científica (`1.23e-4`)
- Não reconhece números hexadecimais (`0xFF`) ou octais (`077`)
- Números com múltiplos pontos (`123.45.67`) não são detectados como erro

### Outras Limitações
- Tamanho máximo fixo para lexemas (100 caracteres)
- Não guarda posição (linha/coluna) dos tokens para mensagens de erro
- Tratamento de erros genérico (apenas marca como `TOKEN_UNKNOWN`)
- Não suporta diretivas de pré-processador (`#include`, `#define`)
- Não reconhece todos os operadores compostos (`+=`, `-=`, etc.)

---
## Execução
Compile e execute o código com :

    gcc analisador.c -o analisador
    ./analisador