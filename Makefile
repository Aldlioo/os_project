# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -g

# Source files
SRC = shell.c

# Header files
HDR = shell.h

# Output executable
OUT = shell

# Default target: Build and run
all: $(OUT)
	./$(OUT)

# Link object files and create the executable
$(OUT): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC)

# Clean up build artifacts
clean:
	rm -f $(OUT)

