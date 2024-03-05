CC := gcc
CFLAGS := -std=gnu99

SRC_DIR := src
OBJ_DIR := obj
OBJ_SRC_DIR := $(OBJ_DIR)/src
OBJ_VENDOR_DIR := $(OBJ_DIR)/vendor
VENDOR_DIR := vendor

SRCS := $(wildcard $(SRC_DIR)/**/*.c) $(wildcard $(SRC_DIR)/*.c) $(wildcard $(VENDOR_DIR)/**/*.c) $(wildcard $(VENDOR_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_SRC_DIR)/%.o,$(filter $(SRC_DIR)/%.c,$(SRCS))) $(patsubst $(VENDOR_DIR)/%.c,$(OBJ_VENDOR_DIR)/%.o,$(filter $(VENDOR_DIR)/%.c,$(SRCS)))

LIBS := -lpthread -lm # Compiler Libs
CO := -O3 # Compiler Options
TARGET := AplikasiKasir # Output file

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(CO) $(LIBS)

$(OBJ_SRC_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_SRC_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_VENDOR_DIR)/%.o: $(VENDOR_DIR)/%.c | $(OBJ_VENDOR_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_SRC_DIR) $(OBJ_VENDOR_DIR):
	mkdir -p $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)