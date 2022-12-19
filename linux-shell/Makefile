SOURCE_DIR := src
INCLUDE_DIR := include
BIN_DIR := bin

#! Compiler Settings
CC := gcc
CFLAGS := $(CFLAGS) -c
# CFLAGS := $(CFLAGS) -Wall -Werror -Wextra -Wpedantic
CFLAGS := $(CFLAGS) -I$(INCLUDE_DIR)
# CFLAGS := $(CFLAGS) -MMD -MP -ggdb
# CFLAGS := $(CFLAGS) -fsanitize=address -fno-omit-frame-pointer

#! Linker Settings
LD := gcc
LDFLAGS := -lreadline
# LDFLAGS := $(LDFLAGS) -fsanitize=address
LDPATH :=

#! Files
##? Sources: to create neames of all c files form the source directory
C_SRCS := $(wildcard $(SOURCE_DIR)/*.c)
##? Objects: to create names of all .o files with the name of .c files
OBJS := $(patsubst $(SOURCE_DIR)/%.c, $(BIN_DIR)/%.o, $(C_SRCS))
##? Headers: to create names of all .h files with the name of .c files
HDRS := $(wildcard $(INCLUDE_DIR)/*.h)
##? Dependencies
DEPS := $(patsubst $(SOURCE_DIR)/%.c, $(BIN_DIR)/%.d, $(C_SRCS))
##? Executable
BIN = $(BIN_DIR)/myshell

#!Targets
.PHONY: all clean

all: $(BIN_DIR) $(BIN)


$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@ $(LDPATH)

$(BIN_DIR):
	mkdir $(BIN_DIR)

-include $(DEPS)
$(BIN_DIR)/%.o: $(SOURCE_DIR)/%.c Makefile
	$(CC) $(CFLAGS) $< -o $@

clean:
	@rm -rvf null.d
	@$(RM) -rvf $(BIN_DIR)
