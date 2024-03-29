CC=gcc
CFLAGS=-Wall -O2 -z noseparate-code -Wl,--build-id=none
EXECUTABLE=xorfile
OBJECTS=xorfile.o

all: $(EXECUTABLE)

# Now we'll compile!
# Explicit rule needed 'cause we have multiple objects.
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	@echo "Deleting \"$(OBJECTS)\" and \"$(EXECUTABLE)\" binary..."
	-rm -f $(OBJECTS) $(EXECUTABLE)
