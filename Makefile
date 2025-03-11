TARGET = ./dist/quark
SRC = ./src/quark.c
LIBS = -lncurses -lm

$(TARGET): $(SRC)
	gcc $(SRC) $(LIBS) -o $(TARGET)

.PHONY: clean run

run: $(TARGET)
	./$(TARGET) ./src/quark.c

clean:
	rm -f $(TARGET)