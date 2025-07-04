
### **`README.md` (Atualizado)**

````markdown
# Compilador Didático Multi-Fase em C

## Autor

Dilvonei Alves Lacerda Junior
Ano: 2025 (Versão com Análise Semântica, Otimização e Geração de Código)

---

## 1. Visão Geral

Este projeto implementa um compilador didático para uma linguagem de programação simples e customizada. Desenvolvido inteiramente em C, o compilador processa o código-fonte através de múltiplas fases, transformando-o de texto puro em um programa C executável.

O objetivo principal é demonstrar de forma clara e modular cada etapa do processo de compilação:

* **Análise Léxica**: Converte o código-fonte em uma sequência de tokens.
* **Análise Sintática**: Valida a estrutura do código e constrói uma **Árvore Sintática Abstrata (AST)**.
* **Análise Semântica**: Usa uma **Tabela de Símbolos** para verificar o significado do código (tipos, declarações, escopo).
* **Otimização**: Modifica a AST para melhorar a eficiência do código (ex: calculando expressões constantes).
* **Geração de Código**: "Transpila" a AST final para um código-fonte na linguagem C.

---

## 2. Especificações da Linguagem Customizada

A linguagem de programação reconhecida por este compilador possui as seguintes características:

### 2.1. Estrutura Geral do Programa

Um programa válido deve seguir a seguinte estrutura:
1.  **Declarações Globais (Opcionais)**: Variáveis e funções customizadas (`fun`) podem ser declaradas no escopo global.
2.  **Bloco `main` (Obrigatório)**: Um bloco `main { ... }` deve existir e conter o ponto de entrada principal do programa. Ele deve ser a última estrutura de alto nível no arquivo.

**Exemplo de Estrutura Válida:**
```c
// Variável global
int contador;

// Função customizada
fun dobrar(int x) {
    return x * 2;
}

// Bloco 'main' (obrigatório e final)
main {
    int resultado;
    resultado = dobrar(5);
}
// Fim do arquivo
````

### 2.2. Palavras-Chave e Tipos

  * **Palavras-chave**: `if`, `else`, `for`, `return`, `fun`, `main`.
  * **Tipos de Dados**: `int`, `float`, `char`.

### 2.3. Estruturas Suportadas

  * **Declarações de Variáveis**: `tipo identificador;` ou `tipo identificador = expressao;`.
  * **Estruturas de Controle**: `if-else`.
  * **Funções**: Definição com `fun` e chamadas de função.
  * **Expressões**: Aritméticas (`+`, `-`, `*`, `/`), lógicas e relacionais.

-----

## 3\. Arquitetura e Fases do Compilador

O compilador opera em um pipeline, onde a saída de uma fase é a entrada da próxima. A **Árvore Sintática Abstrata (AST)** é a estrutura de dados central que conecta as fases.

**Fluxo de Compilação:**
`Código Fonte` -\> `[Analisador Léxico]` -\> `Tokens` -\> `[Analisador Sintático]` -\> `AST` -\> `[Analisador Semântico]` -\> `AST Validada` -\> `[Otimizador]` -\> `AST Otimizada` -\> `[Gerador de Código]` -\> `Código C`

### 3.1. Análise Léxica (`analisador.c`)

Converte o fluxo de caracteres do arquivo de entrada em uma sequência de **tokens** (ex: `TOKEN_KEYWORD`, `TOKEN_IDENTIFIER`). Ignora espaços em branco e trata comentários.

### 3.2. Análise Sintática (`parser.c`)

Recebe os tokens e verifica se eles formam uma estrutura gramaticalmente válida. A principal responsabilidade desta fase é construir a **Árvore Sintática Abstrata (AST)**, uma representação em árvore do código que é usada por todas as fases subsequentes. A AST é definida em `ast.h`.

### 3.3. Análise Semântica (`analisador_semantico.c`)

Percorre a AST para verificar o "significado" do código. Para isso, utiliza uma **Tabela de Símbolos** (`tabela_simbolos.c`) que armazena informações sobre variáveis e funções. As principais verificações são:

  * **Declaração**: Se variáveis e funções foram declaradas antes do uso.
  * **Redeclaração**: Se um identificador foi declarado mais de uma vez no mesmo escopo.
  * **Escopo**: Distinção entre variáveis locais e globais.
  * **Checagem de Tipos**: Se os tipos em operações e atribuições são compatíveis (ex: não permitir `int x = "texto";`).

### 3.4. Otimização (`otimizador.c`)

Percorre a AST validada e a modifica para gerar um código mais eficiente. A técnica implementada é o **Constant Folding** (Dobramento de Constantes):

  * Expressões cujos operandos são constantes são calculadas em tempo de compilação.
  * **Exemplo**: O nó da AST que representa `2 + 3` é substituído por um único nó literal de valor `5`.

### 3.5. Geração de Código (`gerador_codigo.c`)

Percorre a AST final (otimizada) e gera o código-alvo. A implementação atual é um **transpilador**, que traduz a AST para um código-fonte C equivalente.

  * O código C gerado pode ser compilado por um compilador padrão como o GCC.
  * Esta abordagem modular permite que o gerador de código seja substituído no futuro para gerar Assembly ou outro formato.

-----

## 4\. Estrutura dos Arquivos

```
.
├── analisador.c          // Fase 1: Analisador Léxico
├── analisador.h
├── analisador_semantico.c  // Fase 3: Analisador Semântico
├── analisador_semantico.h
├── ast.h                 // Definição da Árvore Sintática Abstrata
├── codigo.txt            // Exemplo de código na linguagem customizada
├── gerador_codigo.c      // Fase 5: Gerador de Código (Transpilador C)
├── gerador_codigo.h
├── main.c                // Ponto de entrada que orquestra as fases
├── Makefile              // Para automação da compilação
├── otimizador.c          // Fase 4: Otimizador da AST
├── otimizador.h
├── parser.c              // Fase 2: Analisador Sintático (constrói a AST)
├── parser.h
├── README.md             // Esta documentação
├── tabela_simbolos.c     // Estrutura de dados para a Análise Semântica
└── tabela_simbolos.h
```

-----

## 5\. Como Compilar e Executar

**Pré-requisitos**: Um compilador C (como `gcc`) e o utilitário `make`.

O processo completo envolve duas etapas de compilação:

1.  Usar nosso compilador para traduzir `codigo.txt` para `output.c`.
2.  Usar um compilador C (GCC) para compilar `output.c` em um executável final.

### Passo a Passo

1.  **Compilar o seu compilador:**
    No terminal, no diretório do projeto, execute o comando:

    ```bash
    make
    ```

    Isso criará o executável `compilador`.

2.  **Executar o seu compilador:**
    Passe o arquivo de código-fonte como argumento:

    ```bash
    ./compilador codigo.txt
    ```

    Se não houver erros, o programa exibirá as fases da compilação e criará um arquivo chamado `output.c`.

3.  **Compilar o código C gerado:**
    Use o GCC (ou outro compilador C) para compilar o arquivo de saída:

    ```bash
    gcc output.c -o meu_programa
    ```

    Isso criará o executável final `meu_programa`.

4.  **Executar o programa final:**

    ```bash
    ./meu_programa
    ```

-----

## 6\. Limitações e Próximos Passos

  * **Linguagem**: A linguagem é mínima e não possui laços `while`/`for` (embora o parser original tivesse stubs), arrays, ou tipos de dados complexos.
  * **Análise Semântica**: A checagem de chamadas de função valida a existência da função, mas não o número ou tipo dos argumentos. Funções `fun` não têm tipo de retorno explícito.
  * **Geração de Código**: O gerador atual só transpila para C.

**Próximos Passos Possíveis:**

  * Expandir a linguagem com mais estruturas de controle, tipos e arrays.
  * Melhorar o analisador semântico para validar completamente as chamadas de função.
  * Implementar mais técnicas de otimização (ex: remoção de código morto).
  * Substituir o gerador de código por um que emita Assembly para uma arquitetura específica (x86, ARM).

<!-- end list -->

```
```