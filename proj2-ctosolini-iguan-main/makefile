# Compiler and compiler flags
CC = gcc
CFLAGS = -Wall -g

# Executable name
TARGET = fetchmail

# Source files
SOURCES = universal.c login.c select.c retrieve.c parse.c mime.c list.c client.c
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Rule to compile object files from source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target to remove build artifacts
clean:
	rm -f $(TARGET) $(OBJECTS)

