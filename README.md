# Analisador Léxico em C

## Autor

Dilvonei Alves Lacerda Junior  
Ano: 2025

---

## Especificações

Este programa é um analisador léxico simples implementado em linguagem C. Ele realiza a leitura de um código-fonte contido em um arquivo `.txt` e identifica os **tokens** presentes no texto, classificando-os nas seguintes categorias:

- `TOKEN_KEYWORD` – Palavras-chave (ex: `int`, `if`, `while`, `return`)
- `TOKEN_IDENTIFIER` – Identificadores (nomes de variáveis e funções)
- `TOKEN_INT` – Números inteiros
- `TOKEN_OPERATOR` – Operadores simples e compostos (`=`, `==`, `!=`, `+`, `++`, `>=`, etc.)
- `TOKEN_DELIMITER` – Delimitadores como `(`, `)`, `{`, `}`, `;`, `,`
- `TOKEN_EOF` – Fim do arquivo
- `TOKEN_UNKNOWN` – Qualquer caractere não reconhecido

---

## Limitações

- Não faz o reconhecimento de **strings** (`"texto"`) ou **caracteres** (`'a'`)
- Não há tratamento de **erros léxicos específicos**, apenas marca como `TOKEN_UNKNOWN`

---
## Execução
Compile o código com:

    gcc analisador.c -o analisador
    E execute com: ./analisador