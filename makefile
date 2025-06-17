# Makefile para dmmr-server

# Diretórios base
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

# Arquivo final
TARGET := $(BIN_DIR)/dmr-server

# Lista de arquivos .c recursivamente
SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Busca todos os diretórios chamados "include" dentro de src/
INCLUDE_DIRS := $(shell find $(SRC_DIR) -type d -name include)
INCLUDES := $(addprefix -I, $(INCLUDE_DIRS))

# Flags comuns
CFLAGS := -Wall -Wextra -std=gnu11 $(INCLUDES)
LDFLAGS :=
LDFLAGS += -lrt


# Configurações por tipo de build
ifeq ($(BUILD),release)
    CFLAGS += -O2 -fPIC -DPIC
    BUILD_TYPE := Release
else
    CFLAGS += -O0 -ggdb
    BUILD_TYPE := Debug
endif

# Para builds seguros (ex: cross-compilação para ARM, RISC-V)
CFLAGS += -DARCH_SAFE_MEMSET

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
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)/%/
	@echo "[CC] $< → $@"
	$(CC) $(CFLAGS) -c $< -o $@

# Garante que o diretório de destino exista
$(BUILD_DIR)/%/:
	mkdir -p $(dir $@)

# Criação dos diretórios bin
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Gerar scanner.c a partir de scanner.l, se usado
LEX = flex
LFLAGS = --header-file=scanner.h

scanner.c: scanner.l
	$(LEX) $(LFLAGS) -o $@ $<

# Limpeza
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
