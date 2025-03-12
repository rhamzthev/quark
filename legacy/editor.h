#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"

#define SCROLL_MARGIN 5
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

void displayBuffer(Buffer *buffer, int cursor_x, int cursor_y);
void handleInput(Buffer *buffer, int ch, int *cursor_x, int *cursor_y);
void initEditor(void);
void cleanupEditor(void);

#endif // EDITOR_H