# Default target
default: all


#==============================================================================#
# Build Options                                                                #
#==============================================================================#

TARGET := dcm2xm

#==============================================================================#
# Target Executable and Sources                                                #
#==============================================================================#

# BUILD_DIR is the location where all build artifacts are placed
BUILD_DIR := build
PROGRAM := $(BUILD_DIR)/$(TARGET)

# Directories containing source files
SRC_DIRS := src

# Source code files
C_FILES := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

# Object files
O_FILES := $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o))

#==============================================================================#
# Compiler Options                                                             #
#==============================================================================#

CC := gcc

INCLUDE_DIRS := include

CFLAGS := $(foreach i,$(INCLUDE_DIRS),-I$(i))
CFLAGS += -pedantic -Wall -Wextra -Wstrict-prototypes

LDFLAGS :=

LIBS = -lm


#==============================================================================#
# Main Targets                                                                 #
#==============================================================================#

all: program

program: $(PROGRAM)

clean:
	$(RM) -r $(BUILD_DIR)

ALL_DIRS := $(addprefix $(BUILD_DIR)/,$(SRC_DIRS))

# Make sure build directory exists before compiling anything
$(shell mkdir -p $(ALL_DIRS))


#==============================================================================#
# Compilation Recipes                                                          #
#==============================================================================#

# Compile C code
$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(PROGRAM): $(O_FILES)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)


.PHONY: all clean default program
# with no prerequisites, .SECONDARY causes no intermediate target to be removed
.SECONDARY:

# Remove built-in rules, to improve performance
MAKEFLAGS += --no-builtin-rules

# --- Debugging
# run `make print-VARIABLE` to debug that variable
print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
