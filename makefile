# Makefile para dmmr-server

# Diretórios
SRC_DIR := src
INC_DIR := include
BUILD_DIR := build
BIN_DIR := bin

# Arquivo final
TARGET := $(BIN_DIR)/dmr-server

# Lista de arquivos .c
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Includes no estilo <dmmr_parser.h>
INCLUDES := -I$(INC_DIR)

# Flags comuns
CFLAGS := -Wall -Wextra -std=gnu11 $(INCLUDES)
LDFLAGS :=

# Configurações por tipo de build
ifeq ($(BUILD),release)
    CFLAGS += -O2 -fPIC -DPIC
    BUILD_TYPE := Release
else
    CFLAGS += -O0 -ggdb
    BUILD_TYPE := Debug
endif

# Targets principais
.PHONY: all debug release clean

all: debug

debug:
	$(MAKE) BUILD=debug $(TARGET)

release:
	$(MAKE) BUILD=release $(TARGET)

# Link final
$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo "[LD] $@ ($(BUILD_TYPE))"
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compilação de .c → .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "[CC] $< → $@"
	$(CC) $(CFLAGS) -c $< -o $@

# Criação dos diretórios bin e build se não existirem
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)
	
LEX = flex
LFLAGS = --header-file=scanner.h

scanner.c: scanner.l
	$(LEX) $(LFLAGS) -o $@ $<

# Limpeza
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
