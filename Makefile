# Makefile for Home Stock Manager
# For MinGW-w64 with UTF-8 support

CC = gcc
WINDRES = windres
CFLAGS = -Wall -Wextra -std=c99 -D_WIN32_WINNT=0x0600 -DUNICODE -D_UNICODE -finput-charset=UTF-8 -fexec-charset=UTF-8
LDFLAGS = -mwindows -luser32 -lgdi32 -lcomctl32 -lcomdlg32 -lkernel32 -lmsimg32 -luxtheme

# Files
SOURCES = main.c stock.c theme.c
OBJECTS = main.o stock.o theme.o resource.o
EXECUTABLE = home_stock_manager.exe
RESOURCE_RC = resource.rc
RESOURCE_O = resource.o

# Default target
all: $(EXECUTABLE)

# Main program
$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile resource file with Unicode support
$(RESOURCE_O): $(RESOURCE_RC)
	$(WINDRES) --input-format=rc --output-format=coff --target=pe-x86-64 --codepage=65001 $< -o $@

# Clean
clean:
	del /Q *.o $(EXECUTABLE) 2>nul || true

# Rebuild
rebuild: clean all

# Run
run: $(EXECUTABLE)
	./$(EXECUTABLE)

# Debug version
debug: CFLAGS += -g -DDEBUG
debug: $(EXECUTABLE)

# Release version
release: CFLAGS += -O2 -DNDEBUG
release: $(EXECUTABLE)

# Dependencies
main.o: main.c stock.h resource.h theme.h
stock.o: stock.c stock.h resource.h theme.h
theme.o: theme.c theme.h
resource.o: resource.rc resource.h

.PHONY: all clean rebuild run debug release
