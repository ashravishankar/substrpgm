# Makefile for Substr Application

CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -g
LIBS = -lsqlite3 -lcjson

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)
TARGET = substrpgm

# Test configuration
TEST_SRC = test/test_query_builder.c
TEST_OBJ = $(filter-out src/main.o, $(OBJ))  # Exclude main.o to avoid multiple main functions
TEST_TARGET = test_runner

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LIBS)

clean:
	rm -f $(OBJ) $(TARGET) $(TEST_TARGET)

run: $(TARGET)
	./$(TARGET)

# Test targets
$(TEST_TARGET): $(TEST_OBJ) $(TEST_SRC)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) $(TEST_OBJ) $(LIBS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean-test:
	rm -f $(TEST_TARGET)

