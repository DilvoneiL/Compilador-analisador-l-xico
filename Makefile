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

# Regra de limpeza: remove os arquivos gerados
clean:
	rm -f $(OBJECTS) $(TARGET) output.py

# <<< CORREÇÃO: O comando 'run' agora executa o script Python >>>
run: all
	# Passo 1: Executa o seu compilador para gerar o output.py
	./$(TARGET) codigo.txt
	
	# Passo 2: Executa o script
	@echo "\n--- Executando o script Python gerado (output.py)... ---"
	python3 output.py

.PHONY: all clean run
