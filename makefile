# Makefile para dmmr-server

# Diretórios base
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin
LEX_SRC := $(SRC_DIR)/parser/scanner.l
LEX_C := $(SRC_DIR)/parser/scanner.c
LEX_H := $(SRC_DIR)/parser/scanner.h

# Arquivo final
TARGET := $(BIN_DIR)/dmmr_orbit

# Lista de arquivos .c, incluindo scanner.c
SRCS := $(shell find $(SRC_DIR) -name '*.c')
SRCS += $(LEX_C)

# Geração dos .o equivalentes
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)

# Busca todos os diretórios "include" dentro de src/
INCLUDE_DIRS := $(shell find $(SRC_DIR) -type d -name include)
INCLUDES := $(addprefix -I, $(INCLUDE_DIRS))

# Flags de compilação
CFLAGS := -Wall -Wextra -std=gnu11 $(INCLUDES)
LDFLAGS := -lrt

ifeq ($(BUILD),release)
    CFLAGS += -O2 -fPIC -DPIC
    BUILD_TYPE := Release
else
    CFLAGS += -O0 -ggdb -mcmodel=large
    BUILD_TYPE := Debug
endif

CFLAGS += -DARCH_SAFE_MEMSET

# Ferramentas de scanner
LEX := flex
LFLAGS := --header-file=$(LEX_H)

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
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)/%/
	@echo "[CC] $< → $@"
	$(CC) $(CFLAGS) -c $< -o $@

# Garante a criação de subdiretórios recursivamente
$(BUILD_DIR)/%/:
	@mkdir -p $(dir $@)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Scanner automático com Flex
$(LEX_C) $(LEX_H): $(LEX_SRC)
	@echo "[LEX] $< → $@"
	$(LEX) $(LFLAGS) -o $(LEX_C) $(LEX_SRC)

# Limpeza
clean:
	@echo "[CLEAN]"
	@rm -rf $(BUILD_DIR) $(BIN_DIR) $(LEX_C) $(LEX_H) 2> /dev/null
