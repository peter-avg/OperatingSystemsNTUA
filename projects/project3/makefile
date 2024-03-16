TARGETS = project3

# Targets to build when running `make`
all: $(TARGETS)

# Phony targets. Executed always.
.PHONY: all clean

# Targets to build `project` binary
project3: project3.o
	gcc project3.o -o ask3
project3.o: project3.c
	gcc project3.c -c -o project3.o

# Run `make clean` to delete binaries and object files.
clean:
	rm -rf $(TARGETS) *.o

