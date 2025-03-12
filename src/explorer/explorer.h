#ifndef EXPLORER_H
#define EXPLORER_H

typedef struct Explorer {
    char *name;
    char *full_path;
    int is_directory;
    struct Explorer *parent;
    struct Explorer **children;
    int children_count;
} Explorer;

void print_explorer(Explorer *node, int depth);
void populate_explorer(Explorer *node);
void free_explorer(Explorer *node);

#endif