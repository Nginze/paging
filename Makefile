CC=gcc
CFLAGS=-I./lib

# List of source files
SOURCES=lib/dTypes.c lib/vmTypes.c lib/main.c

# Output executable
TARGET=vm_sim

# Compilation rule
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^

# Clean rule
clean:
	rm -f $(TARGET)
