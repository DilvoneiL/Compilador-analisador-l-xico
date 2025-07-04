# Nome do compilador C (GNU C Compiler)
CC = gcc

# Flags de compilação: -Wall (todos os warnings), -g (informações de debug), -std=c99 (padrão C99)
CFLAGS = -Wall -g -std=c99

# Nome do executável final
TARGET = compilador

# Arquivos-fonte (.c)
SOURCES = main.c analisador.c parser.c tabela_simbolos.c analisador_semantico.c otimizador.c gerador_codigo.c

# Arquivos-objeto (.o) gerados a partir dos fontes
OBJECTS = $(SOURCES:.c=.o)

# Regra principal: compila o programa
all: $(TARGET)

# Regra de ligação: cria o executável a partir dos arquivos-objeto
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lm

# Regra de compilação: cria um arquivo-objeto a partir de um arquivo-fonte
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra de limpeza: remove os arquivos gerados (objetos e o executável)
clean:
	rm -f $(OBJECTS) $(TARGET) output.c meu_programa

# Comando para rodar todo o processo: compila, executa, compila a saída e roda o final
run: all
	./$(TARGET) codigo.txt
	@echo "\n--- Compilando o código C gerado (output.c)... ---"
	$(CC) output.c -o meu_programa
	@echo "--- Executando o programa final (meu_programa)... ---"
	./meu_programa

.PHONY: all clean run
