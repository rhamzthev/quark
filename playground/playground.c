#include "playground.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <libgen.h>

FileContent* load_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        endwin();
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(1);
    }

    FileContent *content = malloc(sizeof(FileContent));
    content->lines = malloc(MAX_LINES * sizeof(char*));
    content->line_count = 0;
    content->scroll_position = 0;

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file) && content->line_count < MAX_LINES) {
        // Remove trailing newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }

        content->lines[content->line_count] = strdup(line);
        content->line_count++;
    }

    fclose(file);
    return content;
}

void draw_scrollbar(int total_lines, int visible_lines, int scroll_position) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    // Calculate scrollbar dimensions
    int scrollbar_height = (visible_lines * max_y) / total_lines;
    if (scrollbar_height < 1) scrollbar_height = 1;
    
    // Calculate scrollbar position
    int scrollbar_pos = (scroll_position * (max_y - scrollbar_height)) / (total_lines - visible_lines);
    if (scrollbar_pos < 0) scrollbar_pos = 0;
    
    // Draw scrollbar track
    attron(COLOR_PAIR(1));
    for (int y = 0; y < max_y; y++) {
        mvaddch(y, max_x - SCROLLBAR_WIDTH, ACS_VLINE);
    }
    
    // Draw scrollbar thumb
    attron(A_REVERSE);
    for (int y = scrollbar_pos; y < scrollbar_pos + scrollbar_height && y < max_y; y++) {
        mvaddch(y, max_x - SCROLLBAR_WIDTH, ' ');
    }
    attroff(A_REVERSE);
    attroff(COLOR_PAIR(1));
}

void display_file_content(FileContent *content) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    
    int editor_width = max_x - FILETREE_WIDTH - SCROLLBAR_WIDTH - 1;
    int display_start = content->scroll_position;
    int display_end = MIN(content->scroll_position + max_y, content->line_count);

    // Clear editor area
    for (int y = 0; y < max_y; y++) {
        for (int x = FILETREE_WIDTH + 1; x < max_x - SCROLLBAR_WIDTH; x++) {
            mvaddch(y, x, ' ');
        }
    }

    // Display file content
    for (int i = display_start; i < display_end; i++) {
        mvprintw(i - display_start, FILETREE_WIDTH + 1, "%.*s", 
                 editor_width, content->lines[i]);
    }

    // Update scrollbar
    draw_scrollbar(content->line_count, max_y, content->scroll_position);
}

void draw_layout(FileContent *content, FileTree *tree) {
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    // Draw vertical line separator
    attron(COLOR_PAIR(1));
    for (int y = 0; y < max_y; y++) {
        mvaddch(y, FILETREE_WIDTH, ACS_VLINE);
    }
    attroff(COLOR_PAIR(1));

    // Draw file tree
    if (tree && tree->root) {
        int current_y = 0;
        draw_tree_node(tree->root, 1, 0, &current_y, 0);
    }

    // Display file content
    if (content != NULL) {
        display_file_content(content);
    } else {
        mvprintw(1, FILETREE_WIDTH + 2, "No file opened");
    }

    refresh();
}

void init_screen() {
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
}

TreeNode* create_node(const char *name, int is_directory) {
    TreeNode *node = malloc(sizeof(TreeNode));
    node->name = strdup(name);
    node->is_directory = is_directory;
    node->is_expanded = 0;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    node->parent = NULL;  // Add this line
    return node;
}

void add_child(TreeNode *parent, TreeNode *child) {
    if (parent->child_count >= parent->child_capacity) {
        int new_capacity = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
        parent->children = realloc(parent->children, new_capacity * sizeof(TreeNode*));
        parent->child_capacity = new_capacity;
    }
    parent->children[parent->child_count++] = child;
    child->parent = parent;  // Add this line
}

TreeNode* build_tree(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return NULL;
    }

    char *name = strrchr(path, '/');
    name = name ? name + 1 : (char*)path;
    TreeNode *node = create_node(name, S_ISDIR(st.st_mode));

    if (node->is_directory) {
        DIR *dir = opendir(path);
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue;
                }
                char full_path[PATH_MAX];
                snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
                TreeNode *child = build_tree(full_path);
                if (child) {
                    add_child(node, child);
                }
            }
            closedir(dir);
        }
    }
    return node;
}

void draw_tree_node(TreeNode *node, int x, int y, int *current_y, int depth) {
    if (*current_y >= LINES) return;

    // Draw the node
    mvprintw(*current_y, x + depth * 2, "%s %s", 
             node->is_directory ? (node->is_expanded ? "[-]" : "[+]") : "   ",
             node->name);
    
    (*current_y)++;

    // Draw children if expanded
    if (node->is_directory && node->is_expanded) {
        for (int i = 0; i < node->child_count; i++) {
            draw_tree_node(node->children[i], x, y, current_y, depth + 1);
        }
    }
}

char* get_absolute_path(const char *path) {
    char *abs_path = realpath(path, NULL);
    if (!abs_path) {
        fprintf(stderr, "Error: Cannot resolve path '%s'\n", path);
        exit(1);
    }
    return abs_path;
}

TreeNode* build_tree_contents(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        return NULL;
    }

    // Create a dummy root node to hold contents
    TreeNode *root = create_node(".", 1);
    root->is_expanded = 1;  // Always show contents

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[PATH_MAX];   
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0) {
            TreeNode *node = create_node(entry->d_name, S_ISDIR(st.st_mode));
            if (node->is_directory) {
                // For directories, we'll load their contents when expanded
                node->is_expanded = 0;
            }
            add_child(root, node);
        }
    }
    closedir(dir);
    return root;
}

TreeNode* find_node_at_y(TreeNode *root, int target_y, int *current_y, int depth) {
    if (!root || *current_y >= LINES) return NULL;

    // Check if current node is at target_y
    if (*current_y == target_y) {
        return root;
    }
    (*current_y)++;

    // Check children if directory is expanded
    if (root->is_directory && root->is_expanded) {
        for (int i = 0; i < root->child_count; i++) {
            TreeNode *found = find_node_at_y(root->children[i], target_y, current_y, depth + 1);
            if (found) return found;
        }
    }
    return NULL;
}

char* get_node_path(TreeNode *node) {
    // Allocate space for path
    char *path = malloc(PATH_MAX);
    path[0] = '\0';

    // Build path from root to node
    TreeNode *current = node;
    while (current) {
        char temp[PATH_MAX];
        snprintf(temp, sizeof(temp), "/%s%s", current->name, path);
        strcpy(path, temp);
        current = current->parent;  // Note: You'll need to add parent pointer to TreeNode
    }

    return path;
}

void free_file_content(FileContent *content) {
    if (!content) return;
    
    for (int i = 0; i < content->line_count; i++) {
        free(content->lines[i]);
    }
    free(content->lines);
    free(content);
}

void free_file_tree(FileTree *tree) {
    if (!tree) return;
    free_tree_node(tree->root);
}

void free_tree_node(TreeNode *node) {
    if (!node) return;

    for (int i = 0; i < node->child_count; i++) {
        free_tree_node(node->children[i]);
    }

    free(node->name);
    free(node->children);
    free(node);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path>\n", argv[0]);
        return 1;
    }

    char *abs_path = realpath(argv[1], NULL);
    if (!abs_path) {
        fprintf(stderr, "Error: Cannot resolve path '%s'\n", argv[1]);
        return 1;
    }

    struct stat st;
    if (stat(abs_path, &st) != 0) {
        fprintf(stderr, "Error: Cannot access '%s'\n", abs_path);
        free(abs_path);
        return 1;
    }

    // Initialize content and tree
    FileContent *content = NULL;
    FileTree tree = {0};

    // Handle directory vs file
    if (S_ISDIR(st.st_mode)) {
        tree.root = build_tree_contents(abs_path);
    } else {
        content = load_file(abs_path);
        char *dir_path = strdup(abs_path);
        dir_path = dirname(dir_path);
        tree.root = build_tree_contents(dir_path);
        free(dir_path);
    }

    free(abs_path);

    // Initialize screen
    init_screen();
    mousemask(ALL_MOUSE_EVENTS, NULL);
    mouseinterval(0);
    
    // Initial draw
    draw_layout(content, &tree);
    
    // Main event loop
    MEVENT event;
    int ch;
    while ((ch = getch()) != 'q') {
        if (ch == KEY_MOUSE && getmouse(&event) == OK) {
            if (event.x < FILETREE_WIDTH) {
                // Find clicked node
                int current_y = 0;
                TreeNode *clicked = find_node_at_y(tree.root, event.y, &current_y, 0);
                
                if (clicked) {
                    if (clicked->is_directory) {
                        // Toggle directory expansion
                        clicked->is_expanded = !clicked->is_expanded;
                        if (clicked->is_expanded && clicked->child_count == 0) {
                            // Load directory contents when expanded
                            char *full_path = get_node_path(clicked);
                            if (full_path) {
                                TreeNode *contents = build_tree_contents(full_path);
                                if (contents) {
                                    clicked->children = contents->children;
                                    clicked->child_count = contents->child_count;
                                    clicked->child_capacity = contents->child_capacity;
                                    free(contents->name);
                                    free(contents);
                                }
                                free(full_path);
                            }
                        }
                    } else {
                        // Load file content
                        char *file_path = get_node_path(clicked);
                        if (file_path) {
                            if (content) {
                                free_file_content(content);
                            }
                            content = load_file(file_path);
                            free(file_path);
                        }
                    }
                }
            }
        } else {
            // Handle keyboard navigation
            switch (ch) {
                case KEY_UP:
                    if (content && content->scroll_position > 0) {
                        content->scroll_position--;
                    }
                    break;
                case KEY_DOWN:
                    if (content && content->scroll_position < content->line_count - 1) {
                        content->scroll_position++;
                    }
                    break;
                case KEY_PPAGE: // Page Up
                    if (content) {
                        content->scroll_position -= LINES;
                        if (content->scroll_position < 0) {
                            content->scroll_position = 0;
                        }
                    }
                    break;
                case KEY_NPAGE: // Page Down
                    if (content) {
                        content->scroll_position += LINES;
                        if (content->scroll_position > content->line_count - 1) {
                            content->scroll_position = content->line_count - 1;
                        }
                    }
                    break;
            }
        }
        
        // Redraw screen
        draw_layout(content, &tree);
    }

    // Cleanup
    if (content) {
        free_file_content(content);
    }
    free_file_tree(&tree);
    endwin();
    return 0;
}