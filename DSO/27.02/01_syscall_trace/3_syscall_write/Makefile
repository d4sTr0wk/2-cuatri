# Makefile to build some apps from their source files using a simple rule

CC=gcc
EXEC=1_write_to_file 2_write_to_stdout 3_mmap_file

.PHONY: all clean       # not targets

all: $(EXEC)

% : %.c    # any target with its corresponding C source code (same filename) will be compiled with this rule
	$(CC) -o $@ $^
#  $@ is the target (izq), $^ is the prerrequisite (der)     

clean: 
	rm $(EXEC)
