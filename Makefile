TARGET = ./dist/quark
SRC = ./src/quark.c

$(TARGET): $(SRC)
	gcc $(SRC) -lncurses -o $(TARGET)

.PHONY: clean run

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)