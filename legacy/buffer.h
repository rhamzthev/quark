#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>

#define INITIAL_BUFFER_SIZE 1000

typedef struct
{
    char *content;
    int size;
    int capacity;
} Buffer;

void initBuffer(Buffer *buffer);
void loadFile(Buffer *buffer, const char *filename);
void insertChar(Buffer *buffer, int pos, char ch);
void deleteChar(Buffer *buffer, int pos);

#endif // BUFFER_H