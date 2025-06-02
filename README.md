Claro! Aqui está o texto completo do `README.md` que elaboramos, pronto para ser salvo em um arquivo com a extensão `.md`:

```markdown
# Compilador para Linguagem Customizada em C (Front-End)

## Autor

Dilvonei Alves Lacerda Junior  
Ano: 2025 (Versão com Analisador Sintático)

---

## 1. Visão Geral

Este projeto implementa o front-end de um compilador para uma linguagem de programação simples e customizada. Ele é desenvolvido em C e é composto por duas fases principais de análise:

* **Analisador Léxico (Tokenizer)**: Lê o código fonte de um arquivo `.txt` e o divide em uma sequência de tokens.
* **Analisador Sintático (Parser)**: Recebe os tokens do analisador léxico e verifica se a sequência forma uma estrutura gramatical válida de acordo com as regras da linguagem customizada.

O objetivo é processar o código escrito na linguagem customizada, identificar seus componentes, validar sua sintaxe e reportar erros de forma clara.

---

## 2. Especificações da Linguagem Customizada

A linguagem de programação reconhecida por este compilador possui as seguintes características:

### 2.1. Estrutura Geral do Programa

Um programa válido deve seguir a seguinte estrutura:
1.  **Declarações Globais (Opcionais)**:
    * Variáveis globais podem ser declaradas no início do arquivo (ex: `int contador;`).
2.  **Definições de Funções `fun` (Opcionais)**:
    * Funções customizadas podem ser definidas usando a palavra-chave `fun` (ex: `fun dobrar(int x) { return x * 2; }`).
3.  **Bloco `main` (Obrigatório)**:
    * Um bloco `main { ... }` deve existir e conter o ponto de entrada principal do programa. Ele deve vir após todas as declarações globais e definições de `fun`.
    * Nenhum código ou declaração é permitido após o fechamento do bloco `main`.

**Exemplo de Estrutura Válida:**
```c
// Variável global (opcional)
int meu_contador;

// Função 'fun' (opcional)
fun adicionar(int a, int b) {
    int resultado_soma;
    resultado_soma = a + b;
    return resultado_soma;
}

// Bloco 'main' (obrigatório)
main {
    int valor;
    valor = adicionar(5, 3);
    // ... mais código aqui ...
}
// Fim do arquivo aqui
```

### 2.2. Palavras-Chave

As seguintes palavras são reservadas pela linguagem:
* `if`, `else`, `while`, `for`, `return`
* `int`, `float`, `char` (para especificação de tipos)
* `fun` (para definir funções customizadas)
* `main` (para definir o bloco principal do programa)
* `void` (pode ser usada em contextos como parâmetros de função, ex: `fun minha_funcao(void)`, embora o exemplo principal de `fun` não use)

### 2.3. Tipos de Tokens Reconhecidos pelo Analisador Léxico

| Tipo de Token        | Exemplos                                                                 |
|----------------------|--------------------------------------------------------------------------|
| `TOKEN_KEYWORD`      | `if`, `else`, `while`, `for`, `return`, `int`, `float`, `char`, `fun`, `main`, `void` |
| `TOKEN_IDENTIFIER`   | Nomes de variáveis e funções (ex: `contador`, `dobrar`, `x2`)              |
| `TOKEN_INT`          | Números inteiros (ex: `123`, `-456`)                                     |
| `TOKEN_FLOAT`        | Números de ponto flutuante (ex: `3.14`, `-0.001`)                          |
| `TOKEN_STRING`       | Sequências de caracteres entre aspas duplas (ex: `"texto"`)                |
| `TOKEN_CHAR`         | Caracteres únicos entre aspas simples (ex: `'a'`)                          |
| `TOKEN_OPERATOR`     | `+`, `-`, `*`, `/`, `=`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`, `++`, `--` |
| `TOKEN_DELIMITER`    | `(`, `)`, `{`, `}`, `;`, `,`                                             |
| `TOKEN_COMMENT`      | Comentários de linha `// ...` ou em bloco `/* ... */`                      |
| `TOKEN_EOF`          | Marcador de fim de arquivo                                               |
| `TOKEN_UNKNOWN`      | Caracteres ou sequências não reconhecidas                                  |

### 2.4. Declarações e Estruturas Suportadas (Visão Sintática)
* **Declarações de Variáveis**: `tipo identificador;` ou `tipo identificador = expressao;` (locais e globais).
* **Estruturas de Controle**: `if-else`, `for`, `while` (assumindo que `while` seria similar ao `for` ou `if` na sua implementação).
* **Retorno de Funções**: `return expressao?;`.
* **Blocos de Código**: `{ declaracoes_ou_comandos* }`.
* **Expressões**: Aritméticas, lógicas, relacionais, chamadas de função, literais, identificadores.

---

## 3. Fases do Compilador Implementadas

Este projeto foca nas duas primeiras fases de um compilador: a análise léxica e a análise sintática.

### 3.1. Análise Léxica (Tokenizer)

**Responsabilidade**: Ler o código fonte como uma sequência de caracteres e convertê-lo em uma sequência de **tokens**. Cada token representa uma unidade lexical indivisível da linguagem.

**Como Funciona no Projeto (`analisador.c`, `analisador.h`)**:
* **Identificação de Lexemas**: Agrupa caracteres em palavras-chave, identificadores, números, strings, operadores e delimitadores.
* **Criação de Tokens**: Para cada lexema, um token é gerado, contendo seu tipo (ex: `TOKEN_KEYWORD`), o próprio lexema (ex: `"if"`), e sua posição (linha e coluna) no código fonte.
* **Tratamento de Espaços e Comentários**: Espaços em branco são ignorados. Comentários (`//...` e `/*...*/`) são identificados e podem ser descartados ou tratados como tokens especiais (`TOKEN_COMMENT`) que o analisador sintático subsequente ignora.
* **Interface com o Parser**: Fornece tokens um a um para o analisador sintático através da função `next_token()`.
* **Relato de Erros Léxicos**: Sequências de caracteres que não correspondem a nenhum padrão de token válido são marcadas como `TOKEN_UNKNOWN`.

### 3.2. Análise Sintática (Parser)

**Responsabilidade**: Receber a sequência de tokens do analisador léxico e verificar se ela obedece à **gramática** da linguagem. Ou seja, verifica se a "frase" (o programa) está construída corretamente.

**Como Funciona no Projeto (`parser.c`, `parser.h`)**:
* **Validação da Estrutura**: Utiliza um método de parsing descendente recursivo. Cada função de parsing (ex: `parse_program`, `parse_statement`, `parse_expression`) corresponde a uma regra da gramática da linguagem.
* **Hierarquia e Ordem**: Garante que as construções da linguagem (como declarações de variáveis, comandos `if`, loops `for`, definições de função `fun`, e o bloco `main`) apareçam na ordem correta e com a estrutura esperada (ex: parênteses, chaves, ponto e vírgula).
* **Estrutura do Programa**: Impõe a regra de que o código deve consistir em declarações globais/`fun` opcionais, seguidas por um bloco `main { ... }` obrigatório, e então o fim do arquivo.
* **Precedência de Operadores**: Ao analisar expressões, a hierarquia das funções de parsing de expressão (`parse_assignment_expression`, `parse_additive_expression`, etc.) garante que operadores com maior precedência sejam processados antes (ex: `*` antes de `+`).
* **Relato de Erros Sintáticos**: Se uma sequência de tokens viola uma regra gramatical (ex: falta de um `;`, um `else` sem `if`), a função `syntax_error()` é chamada, exibindo uma mensagem com a linha e coluna do erro, o que era esperado e o que foi encontrado. O programa é encerrado após o primeiro erro sintático.
* **Não Gera AST Explícita**: Este parser valida a sintaxe e percorre a estrutura do programa implicitamente. Ele não constrói uma Árvore Sintática Abstrata (AST) como uma estrutura de dados separada, o que seria comum em compiladores que realizam fases subsequentes como análise semântica ou geração de código.

---

## 4. Limitações Conhecidas
### 4.1. Limitações do Analisador Léxico
* **Strings e Caracteres**:
    * Suporte limitado a sequências de escape (ex: `\n`, `\t`).
    * Caracteres multibyte ou não-ASCII podem não ser tratados corretamente.
* **Números**:
    * Não suporta notação científica (ex: `1.23e-4`).
    * Não reconhece números hexadecimais (`0xFF`) ou octais (`077`).
* **Outras Limitações Léxicas**:
    * Tamanho máximo fixo para lexemas (definido por `token.lexeme[100]`).
    * Não suporta diretivas de pré-processador (`#include`, `#define`).
    * Pode não reconhecer todos os operadores compostos de atribuição (ex: `+=`, `-=`) se não estiverem explicitamente na lista `operators` e tratados em `handle_operators`.

### 4.2. Limitações do Analisador Sintático (e do Compilador em Geral)
* **Análise Semântica Rudimentar**: O parser atual foca na estrutura sintática. Ele não realiza checagens semânticas profundas, como:
    * Verificar se variáveis foram declaradas antes do uso.
    * Checagem de tipos em atribuições ou operações.
    * Resolução de escopo completa.
    * Verificar se funções `fun` são chamadas com o número e tipo corretos de argumentos.
* **Não Gera Código**: Este é um front-end; ele analisa e valida, mas não gera código executável ou intermediário.
* **Recuperação de Erros**: O parser para no primeiro erro sintático encontrado. Ele não tenta se recuperar para encontrar múltiplos erros em uma única execução.
* **Tipos de Retorno de `fun`**: A sintaxe `fun nome(params) { ... }` não especifica explicitamente um tipo de retorno para a função. A validação de `return expressao;` é sintática; a compatibilidade do tipo retornado não é verificada.

---

## 5. Compilação e Execução

1.  **Pré-requisitos**: Compilador C (como GCC).
2.  **Arquivos Necessários**:
    * `analisador.h`
    * `analisador.c`
    * `parser.h`
    * `parser.c`
    * `codigo.txt` (contendo o código fonte na linguagem customizada)
3.  **Compilação**:
    No terminal, navegue até o diretório onde os arquivos estão e execute:
    ```bash
    gcc analisador.c parser.c -o compilador
    ```
4.  **Execução**:
    ```bash
    ./compilador
    ```
    O programa lerá o `codigo.txt`. Se a sintaxe estiver correta, exibirá "Análise sintática concluída com sucesso!". Caso contrário, mostrará uma mensagem de erro sintático com a linha e coluna aproximada do problema.

```