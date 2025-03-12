#ifndef EDITOR_H
#define EDITOR_H

typedef struct Buffer {
    char *buffer;
    int size;
    int capacity;
} Buffer;

typedef struct Cursor {
    int x;
    int y;
} Cursor;

typedef struct Viewport {
    int x;
    int y;
    int width;
    int height;
} Viewport;

typedef struct Selection {
    Cursor start;
    Cursor end;
} Selection;

typedef struct Editor {
    char* path;
    Buffer buffer;
} Editor;

void open_editor(Editor *editor);

#endif