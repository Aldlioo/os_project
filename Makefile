
# @file        Makefile
# @description Bu Makefile, Linux işletim sistemi üzerinde çalışan temel bir kabuk (shell) uygulamasını 
#              derlemek ve çalıştırmak için gerekli olan komutları tanımlamaktadır.
# @course      İşletim Sistemleri
# @assignment  Proje Ödevi
# @date        26 Aralık 2024
# @author      Awab KHALEFA - awab.daw@yahoo.com
# @note        Bu projeyi, grup arkadaşlarımı bulamadığım için tek başıma tamamladım.
# @github      https://github.com/Aldlioo/os_project
 


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

