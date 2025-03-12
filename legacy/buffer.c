#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

void initBuffer(Buffer *buffer)
{
    buffer->content = malloc(INITIAL_BUFFER_SIZE);
    if (buffer->content == NULL)
    {
        endwin();
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    buffer->content[0] = '\0';
    buffer->size = 0;
    buffer->capacity = INITIAL_BUFFER_SIZE;
}

void loadFile(Buffer *buffer, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        endwin();
        fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        exit(1);
    }

    // Read file content into buffer
    int ch;
    while ((ch = fgetc(file)) != EOF)
    {
        if (buffer->size + 1 >= buffer->capacity)
        {
            buffer->capacity *= 2;
            char *new_content = realloc(buffer->content, buffer->capacity);
            if (new_content == NULL)
            {
                endwin();
                fprintf(stderr, "Memory reallocation failed\n");
                fclose(file);
                exit(1);
            }
            buffer->content = new_content;
        }
        buffer->content[buffer->size++] = ch;
    }

    fclose(file);
}

void insertChar(Buffer *buffer, int pos, char ch)
{
    if (buffer->size + 1 >= buffer->capacity)
    {
        buffer->capacity *= 2;
        buffer->content = realloc(buffer->content, buffer->capacity);
        if (buffer->content == NULL)
        {
            endwin();
            fprintf(stderr, "Memory reallocation failed\n");
            exit(1);
        }
    }

    memmove(&buffer->content[pos + 1], &buffer->content[pos], buffer->size - pos + 1);
    buffer->content[pos] = ch;
    buffer->size++;
}

void deleteChar(Buffer *buffer, int pos)
{
    if (pos < buffer->size)
    {
        memmove(&buffer->content[pos], &buffer->content[pos + 1], buffer->size - pos);
        buffer->size--;
    }
}