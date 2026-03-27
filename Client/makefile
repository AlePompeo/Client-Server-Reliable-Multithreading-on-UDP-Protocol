# Variabili
CC = gcc
CFLAGS = -O2 -pthread -Wall -Wextra -Wshadow -Wuninitialized -Wformat -Wnull-dereference -Wunused-variable -Wunused-parameter -Wunused-function -pedantic -g -Iinclude
LDFLAGS = -pthread -L$(HOME)/local_bin/zlib/lib -lz  # Aggiungi il percorso di zlib
SRC_DIR = src
INCLUDE_DIR = include
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(SRC_DIR)/%.o, $(SRC_FILES))
TARGET = my_client

# Regola per creare l'eseguibile finale, specificando che `main.o` è incluso
$(TARGET): $(OBJ_FILES)
	$(CC) -o $(TARGET) $(OBJ_FILES) $(LDFLAGS)

# Regola per generare ciascun file .o a partire dai file .c
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Pulizia dei file di compilazione
clean:
	rm -f $(OBJ_FILES) $(TARGET)
