#include "buffer.h"
#include "editor.h"
#include <stdlib.h>
#include <ncurses.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    Buffer buffer;
    initBuffer(&buffer);
    loadFile(&buffer, argv[1]);

    int ch;
    int cursor_x = 0, cursor_y = 0;

    initEditor();

    while (1)
    {
        displayBuffer(&buffer, cursor_x, cursor_y);

        ch = getch();
        if (ch == 17)
        { // Ctrl+Q
            break;
        }

        handleInput(&buffer, ch, &cursor_x, &cursor_y);
    }

    cleanupEditor();
    free(buffer.content);
    return 0;
}