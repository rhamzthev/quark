#include "./explorer/explorer.h"
#include "./editor/editor.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#define PATH_MAX 4096

int main(int argc, char *argv[]) {
    if (argc != 2) {
        // TODO: Open an empty file
        printf("This will soon open an empty file\n");
        return 1;
    }

    // Resolve path
    char path[PATH_MAX];
    char *result = realpath(argv[1], path);
    
    if (!result) {
        fprintf(stderr, "Error: Cannot resolve path '%s'\n", argv[1]);
        return 1;
    }

    // Check if can access path
    struct stat statbuf;
    int status = stat(path, &statbuf);

    if (status != 0) {
        fprintf(stderr, "Error: Cannot access '%s'\n", path);
        return 1;
    }

    Explorer explorer = {0};
    Editor editor = {0};

    // Check if path is directory or file
    if (S_ISDIR(statbuf.st_mode)) {
        explorer = (Explorer) {
            .name = path,
            .full_path = path,
            .is_directory = 1,
            .parent = NULL,
            .children = NULL,
            .children_count = 0
        };

        // Populate explorer
        populate_explorer(&explorer);

        // Display explorer content
        print_explorer(&explorer, 0);

    } else if (S_ISREG(statbuf.st_mode)) {
        // open editor
        editor = (Editor) {
            .path = path,
            .buffer = (Buffer) {
                .buffer = NULL,
                .size = 0,
                .capacity = 0
            }
        };
        open_editor(&editor);
        // print buffer content
        printf("%s\n", editor.buffer.buffer);

    } else {
        fprintf(stderr, "Error: Unsupported file type\n");
        return 1;
    }

    return 0;
}