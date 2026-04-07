CC = gcc
CFLAGS = -g -Wall
CLINK = -lpthread

SRCDIR = src
INCLDIR = include
DBDIR = data

TARGET = ml_threaded

all: $(TARGET)

extra: CFLAGS += -DBUFFER_SIZE=100
extra: clean $(TARGET)

$(TARGET): main.o producer.o consumer.o
	$(CC) -o $@ $^ -I$(INCLDIR) $(CFLAGS) $(CLINK)


main.o: $(SRCDIR)/main.c $(INCLDIR)/ml_thread.h
	$(CC) -o $@ -I$(INCLDIR) -c $(SRCDIR)/main.c $(CFLAGS)

producer.o: $(SRCDIR)/producer.c $(INCLDIR)/ml_thread.h
	$(CC) -o $@ -I$(INCLDIR) -c $(SRCDIR)/producer.c $(CFLAGS)

consumer.o: $(SRCDIR)/consumer.c $(INCLDIR)/ml_thread.h
	$(CC) -o $@ -I$(INCLDIR) -c $(SRCDIR)/consumer.c $(CFLAGS)


# Worker Thread: 1, 4, and 8
# Buffer size = 10k
test: $(TARGET)
	@echo "Running tests with different worker counts (Buffer=10k) ..."
	@echo "---------------------------------------"
	./$(TARGET) 1
	@echo "---------------------------------------"
	./$(TARGET) 4
	@echo "---------------------------------------"
	./$(TARGET) 8
	@echo "---------------------------------------"
	@rm -f *.bin
	@echo "Test completed."


# Buffer size = 100
test_extra: extra
	@echo "Running tests with different worker counts (Buffer=100) ..."
	@echo "---------------------------------------"
	./$(TARGET) 1
	@echo "---------------------------------------"
	./$(TARGET) 4
	@echo "---------------------------------------"
	./$(TARGET) 8
	@echo "---------------------------------------"
	@rm -f *.bin
	@echo "Test completed."


DIR_NAME = $(shell basename $(CURDIR))

zip: clean
	@echo "Creating submission zip file: $(DIR_NAME).zip"
	@zip -r $(DIR_NAME).zip . -x "include/*" "data/*" "output/*" "Makefile" "*.git*" "*__pycache__*" ".DS_Store"
	@echo "Done. Please upload $(DIR_NAME).zip to Gradescope."

.PHONY: clean test test_extra all extra zip

clean:
	rm -f *.o $(TARGET) *.bin