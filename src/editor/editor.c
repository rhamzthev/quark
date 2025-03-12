#include "editor.h"
#include <stdio.h>
#include <stdlib.h>

void open_editor(Editor *editor) {
    FILE *file = fopen(editor->path, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error: Could not open file '%s'\n", editor->path);
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long capacity = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the buffer (+1 for null terminator)
    char* buffer = (char*)malloc(capacity + 1);
    if (buffer == NULL) {
        fclose(file);
        exit(1);
    }
    
    // Read file content into buffer
    size_t size = fread(buffer, 1, capacity, file);
    buffer[size] = '\0'; // Add null terminator
    
    fclose(file);

    editor->buffer.buffer = buffer;
    editor->buffer.size = size;
    editor->buffer.capacity = capacity + 1;
}