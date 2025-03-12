#include "explorer.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PATH_MAX 4096

int is_directory(const char *path)
{
    struct stat buffer;
    int status = stat(path, &buffer);
    return status == 0 && S_ISDIR(buffer.st_mode);
}

int is_file(const char *path)
{
    struct stat buffer;
    int status = stat(path, &buffer);
    return status == 0 && S_ISREG(buffer.st_mode);
}

void print_explorer(Explorer *node, int depth)
{
    for (int i = 0; i < depth; i++)
    {
        printf("  ");
    }
    printf("%s\n", node->name);

    if (node->is_directory)
    {
        for (int i = 0; i < node->children_count; i++)
        {
            print_explorer(node->children[i], depth + 1);
        }
    }
}

void populate_explorer(Explorer *node)
{
    // open directory
    DIR *dir = opendir(node->full_path);
    struct dirent *entry;
    if (dir != NULL)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            {
                continue;
            }

            // Create full path for the entry
            char full_path[PATH_MAX];
            snprintf(full_path, PATH_MAX, "%s/%s", node->full_path, entry->d_name);

            // create explorer node
            Explorer *child = malloc(sizeof(Explorer));
            child->name = strdup(entry->d_name);
            child->full_path = strdup(full_path);
            child->children_count = 0;

            // if file, just add to children
            if (is_file(full_path))
            {
                child->is_directory = 0;
                child->parent = node;
                child->children = NULL;

                // push to node->children
                node->children = realloc(node->children, (node->children_count + 1) * sizeof(Explorer *));
                node->children[node->children_count] = child;
                node->children_count++;
            }

            // if directory, DFS its children, and then add to node's children
            if (is_directory(full_path))
            {
                child->is_directory = 1;
                child->children = NULL;
                child->parent = node;
                
                // Add to parent's children first
                node->children = realloc(node->children, (node->children_count + 1) * sizeof(Explorer *));
                node->children[node->children_count] = child;
                node->children_count++;
                
                // Then populate its children
                populate_explorer(child);
            }
        }
        closedir(dir);
    }
}

void free_explorer(Explorer *node)
{
    if (node == NULL) return;

    // Free all children recursively
    if (node->is_directory && node->children != NULL) {
        for (int i = 0; i < node->children_count; i++) {
            free_explorer(node->children[i]);
        }
        free(node->children);
    }

    // Free the node's name and the node itself
    free(node->name);
    free(node->full_path);
    free(node);
}